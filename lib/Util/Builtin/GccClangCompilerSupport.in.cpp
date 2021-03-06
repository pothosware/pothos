// Copyright (c) 2014-2021 Josh Blum
//                    2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "Util/Builtin/CompilerStdFlags.hpp"

#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Process.h>
#include <Poco/TemporaryFile.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Logger.h>
#include <fstream>
#include <iostream>

/***********************************************************************
 * gcc/clang compiler wrapper
 **********************************************************************/
class GccClangCompilerSupport : public Pothos::Util::Compiler
{
public:

    GccClangCompilerSupport(void)
    {
        return;
    }

    std::string compiler_executable(void) const
    {
        return "@CMAKE_CXX_COMPILER@";
    }

    bool test(void)
    {
        Poco::Process::Args args;
        args.push_back("--version");
        Poco::Process::Env env;
        Poco::Pipe outPipe;
        Poco::ProcessHandle ph(Poco::Process::launch(
            compiler_executable(), args, nullptr, &outPipe, &outPipe, env));
        return ph.wait() == 0;
    }

    std::string compileCppModule(const Pothos::Util::CompilerArgs &args);
};

std::string GccClangCompilerSupport::compileCppModule(const Pothos::Util::CompilerArgs &compilerArgs)
{
    //create args
    Poco::Process::Args args;

    //add libraries
    for (const auto &library : compilerArgs.libraries)
    {
        args.push_back(library);
    }

    //add compiler flags
    args.push_back(CPP_STD_FLAG);
#ifndef __GLIBCXX__
    args.push_back("-stdlib=libc++");
#endif
    args.push_back("-shared");
    args.push_back("-fPIC");
    for (const auto &flag : compilerArgs.flags)
    {
        args.push_back(flag);
    }

    //add include paths
    for (const auto &include : compilerArgs.includes)
    {
        args.push_back("-I");
        args.push_back(include);
    }

    //specify OSX sysroot when available
    #ifdef __APPLE__
    #cmakedefine CMAKE_OSX_SYSROOT
    #ifdef CMAKE_OSX_SYSROOT
        args.push_back("-isysroot");
        args.push_back("@CMAKE_OSX_SYSROOT@");
    #endif //CMAKE_OSX_SYSROOT
    #endif //__APPLE__

    //add compiler sources
    args.push_back("-x");
    args.push_back("c++");
    for (const auto &source : compilerArgs.sources)
    {
        args.push_back(source);
    }

    //create temp out file
    const auto outPath = this->createTempFile(Poco::SharedLibrary::suffix());
    args.push_back("-o");
    args.push_back(outPath);

    //launch
    Poco::Pipe inPipe, outPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        compiler_executable(), args, &inPipe, &outPipe, &outPipe, env));

    //read into output buffer until pipe is closed
    Poco::PipeInputStream is(outPipe);
    std::string outBuff;
    for (std::string line; std::getline(is, line);) outBuff += line+'\n';

    //handle error case
    if (ph.wait() != 0 or not Poco::File(outPath.c_str()).exists())
    {
        std::string cmdStr(compiler_executable());
        for (const auto &arg : args) cmdStr += " " + arg;
        poco_error_f2(Poco::Logger::get("Pothos.GccClangCompilerSupport.compileCppModule"), "\nCommand: %s\nMessage: %s", cmdStr, outBuff);
        throw Pothos::Exception("GccClangCompilerSupport::compileCppModule", outBuff);
    }

    //return output file path
    return outPath;
}

/***********************************************************************
 * factory and registration
 **********************************************************************/
Pothos::Util::Compiler::Sptr makeGccClangCompilerSupport(void)
{
    return Pothos::Util::Compiler::Sptr(new GccClangCompilerSupport());
}

pothos_static_block(pothosUtilRegisterGccClangCompilerSupport)
{
    Pothos::PluginRegistry::addCall("/util/compiler/gcc_clang", &makeGccClangCompilerSupport);
}
