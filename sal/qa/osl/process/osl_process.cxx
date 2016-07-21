/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifdef IOS
#define CPPUNIT_PLUGIN_EXPORTED_NAME cppunitTest_osl_process
#endif

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <osl/module.hxx>
#include <sal/macros.h>

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/valgrind.h>
#else
#define RUNNING_ON_VALGRIND false
#endif

#if defined(_WIN32)                     // Windows
#   include <windows.h>
#   include <tchar.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>

#ifdef UNX
#if defined( MACOSX )
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
# else
    extern char** environ;
# endif
#endif

#if defined(_WIN32)
    const rtl::OUStringLiteral EXECUTABLE_NAME ("osl_process_child.exe");
#else
    const rtl::OUStringLiteral EXECUTABLE_NAME ("osl_process_child");
#endif

using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

/** get binary Path.
*/
inline ::rtl::OUString getExecutablePath()
{
    ::rtl::OUString dirPath;
    osl::Module::getUrlFromAddress(
        reinterpret_cast<oslGenericFunction>(&getExecutablePath), dirPath);
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') );
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') + 1);
    dirPath += "Executable";
    return dirPath;
}

//rtl::OUString CWD = getExecutablePath();

typedef std::vector<OString> string_container_t;

class exclude : public std::unary_function<OString, bool>
{
public:

    explicit exclude(const string_container_t& exclude_list)
    {
        string_container_t::const_iterator iter     = exclude_list.begin();
        string_container_t::const_iterator iter_end = exclude_list.end();
        for (/**/; iter != iter_end; ++iter)
            exclude_list_.push_back(env_var_name(*iter));
    }

    bool operator() (const OString& env_var) const
    {
        return (exclude_list_.end() !=
                std::find(
                    exclude_list_.begin(),
                    exclude_list_.end(),
                    env_var_name(env_var)));
    }

private:

    // extract the name from an environment variable
    // that is given in the form "NAME=VALUE"
    static OString env_var_name(const OString& env_var)
    {
        sal_Int32 pos_equal_sign =
            env_var.indexOf('=');

        if (-1 != pos_equal_sign)
            return env_var.copy(0, pos_equal_sign);

        return OString();
    }

private:
    string_container_t exclude_list_;
};

namespace
{
    void tidy_container(string_container_t &env_container)
    {
        //sort them because there are no guarantees to ordering
        std::sort(env_container.begin(), env_container.end());
        if (RUNNING_ON_VALGRIND)
        {
            env_container.erase(
                std::remove_if(
                    env_container.begin(), env_container.end(),
                    [](OString const & s) {
                        return s.startsWith("LD_PRELOAD=")
                            || s.startsWith("VALGRIND_LIB="); }),
                env_container.end());
        }
    }
}

#ifdef _WIN32
    void read_parent_environment(string_container_t* env_container)
    {
        LPTSTR env = reinterpret_cast<LPTSTR>(GetEnvironmentStrings());
        LPTSTR p   = env;

        while (size_t l = _tcslen(p))
        {
            env_container->push_back(OString(p));
            p += l + 1;
        }
        FreeEnvironmentStrings(env);
        tidy_container(*env_container);
    }
#else
    void read_parent_environment(string_container_t* env_container)
    {
        for (int i = 0; nullptr != environ[i]; i++)
            env_container->push_back(OString(environ[i]));
        tidy_container(*env_container);
    }
#endif

class Test_osl_executeProcess : public CppUnit::TestFixture
{
    const OUString env_param_;

    OUString     temp_file_url_;
    OUString     temp_file_path_;
    rtl_uString* parameters_[2];
    int          parameters_count_;
    OUString    suCWD;
    OUString    suExecutableFileURL;

public:

    // ctor
    Test_osl_executeProcess() :
        env_param_(OUString("-env")),
        parameters_count_(2)
    {
        parameters_[0] = env_param_.pData;
        suCWD = getExecutablePath();
        suExecutableFileURL = suCWD + "/" + EXECUTABLE_NAME;
    }

    virtual void setUp() override
    {
        temp_file_path_ = create_temp_file(temp_file_url_);
        parameters_[1]  = temp_file_path_.pData;
    }

    virtual void tearDown() override
    {
        osl::File::remove(temp_file_url_);
    }

    OUString create_temp_file(OUString &temp_file_url)
    {
        FileBase::RC rc = FileBase::createTempFile(nullptr, nullptr, &temp_file_url);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("createTempFile failed", FileBase::E_None, rc);

        OUString temp_file_path;
        rc = FileBase::getSystemPathFromFileURL(temp_file_url, temp_file_path);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getSystemPathFromFileURL failed", FileBase::E_None, rc);

        return temp_file_path;
    }

    void read_child_environment(string_container_t* env_container)
    {
        OString temp_file_name = OUStringToOString(OUString(
            parameters_[1]), osl_getThreadTextEncoding());
        std::ifstream file(temp_file_name.getStr());

        CPPUNIT_ASSERT_MESSAGE
        (
            "I/O error, cannot open child environment file",
            file.is_open()
        );

        std::string line;
        line.reserve(1024);
        while (std::getline(file, line, '\0'))
            env_container->push_back(OString(line.c_str()));
        tidy_container(*env_container);
    }

    // environment of the child process that was
    // started. The child process writes his
    // environment into a file
    void compare_environments()
    {
        string_container_t parent_env;
        read_parent_environment(&parent_env);

        string_container_t child_env;
        read_child_environment(&child_env);

        OString msg(
            OString::number(parent_env.size()) + "/"
            + OString::number(child_env.size()));
        auto min = std::min(parent_env.size(), child_env.size());
        for (decltype(min) i = 0; i != min; ++i) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                msg.getStr(), parent_env[i], child_env[i]);
        }
        if (parent_env.size() != child_env.size()) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                (parent_env.size() >= child_env.size()
                 ? parent_env.back() : child_env.back()).getStr(),
                parent_env.size(), child_env.size());
        }
    }

    // compare the equal environment parts and the
    // different part of the child environment
    bool compare_merged_environments(const string_container_t& different_env_vars)
    {
        string_container_t parent_env;
        read_parent_environment(&parent_env);

        for (string_container_t::const_iterator iter = parent_env.begin(), end = parent_env.end(); iter != end; ++iter)
            std::cout << "initially parent env: " << *iter << "\n";

        //remove the environment variables that we have changed
        //in the child environment from the read parent environment
        parent_env.erase(
            std::remove_if(parent_env.begin(), parent_env.end(), exclude(different_env_vars)),
            parent_env.end());

        for (string_container_t::const_iterator iter = parent_env.begin(), end = parent_env.end(); iter != end; ++iter)
            std::cout << "stripped parent env: " << *iter << "\n";

        //read the child environment and exclude the variables that
        //are different
        string_container_t child_env;
        read_child_environment(&child_env);

        for (string_container_t::const_iterator iter = child_env.begin(), end = child_env.end(); iter != end; ++iter)
            std::cout << "initial child env: " << *iter << "\n";
        //partition the child environment into the variables that
        //are different to the parent environment (they come first)
        //and the variables that should be equal between parent
        //and child environment
        string_container_t::iterator iter_logical_end =
            std::stable_partition(child_env.begin(), child_env.end(), exclude(different_env_vars));

        string_container_t different_child_env_vars(child_env.begin(), iter_logical_end);
        child_env.erase(child_env.begin(), iter_logical_end);

        for (string_container_t::const_iterator iter = child_env.begin(), end = child_env.end(); iter != end; ++iter)
            std::cout << "stripped child env: " << *iter << "\n";

        bool common_env_size_equals    = (parent_env.size() == child_env.size());
        bool common_env_content_equals = std::equal(child_env.begin(), child_env.end(), parent_env.begin());

        for (string_container_t::const_iterator iter = different_env_vars.begin(), end = different_env_vars.end(); iter != end; ++iter)
            std::cout << "different should be: " << *iter << "\n";

        for (string_container_t::const_iterator iter = different_child_env_vars.begin(), end = different_child_env_vars.end(); iter != end; ++iter)
            std::cout << "different are: " << *iter << "\n";

        bool different_env_size_equals    = (different_child_env_vars.size() == different_env_vars.size());
        bool different_env_content_equals =
            std::equal(different_env_vars.begin(), different_env_vars.end(), different_child_env_vars.begin());

        return (common_env_size_equals && common_env_content_equals &&
                different_env_size_equals && different_env_content_equals);
    }

    // test that parent and child process have the
    // same environment when osl_executeProcess will
    // be called with out setting new environment
    // variables
   void osl_execProc_parent_equals_child_environment()
    {
        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            suExecutableFileURL.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            nullptr,
            suCWD.pData,
            nullptr,
            0,
            &process);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None, osl_error
        );

        osl_freeProcessHandle(process);

        compare_environments();
    }

    #define ENV1 "PAT=a:\\"
    #define ENV2 "PATHb=b:\\"
    #define ENV3 "Patha=c:\\"
    #define ENV4 "Patha=d:\\"

    void osl_execProc_merged_child_environment()
    {
        rtl_uString* child_env[4];
        OUString env1(ENV1);
        OUString env2(ENV2);
        OUString env3(ENV3);
        OUString env4(ENV4);

        child_env[0] = env1.pData;
        child_env[1] = env2.pData;
        child_env[2] = env3.pData;
        child_env[3] = env4.pData;

        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            suExecutableFileURL.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            nullptr,
            suCWD.pData,
            child_env,
            SAL_N_ELEMENTS(child_env),
            &process);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None, osl_error
        );

        osl_freeProcessHandle(process);

        string_container_t different_child_env_vars;
        different_child_env_vars.push_back(ENV1);
        different_child_env_vars.push_back(ENV2);
        different_child_env_vars.push_back(ENV4);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_execProc_merged_child_environment",
            compare_merged_environments(different_child_env_vars)
        );
    }

    void osl_execProc_test_batch()
    {
        oslProcess process;
#if defined(_WIN32)
        rtl::OUString suBatch = suCWD + "/batch.bat";
#else
        rtl::OUString suBatch = suCWD + "/batch.sh";
#endif
        oslProcessError osl_error = osl_executeProcess(
            suBatch.pData,
            nullptr,
            0,
            osl_Process_NORMAL,
            nullptr,
            suCWD.pData,
            nullptr,
            0,
            &process);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None, osl_error
        );

        osl_freeProcessHandle(process);
    }

    CPPUNIT_TEST_SUITE(Test_osl_executeProcess);
    //TODO: Repair these (at least under Windows)
#if !defined(_WIN32)
    CPPUNIT_TEST(osl_execProc_parent_equals_child_environment);
    CPPUNIT_TEST(osl_execProc_merged_child_environment);
#endif
    CPPUNIT_TEST(osl_execProc_test_batch);
    ///TODO: Repair test (or tested function ;-) - test fails.
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test_osl_executeProcess);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
