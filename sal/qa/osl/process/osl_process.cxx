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

#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>

#include <osl/module.hxx>
#include <sal/macros.h>

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/valgrind.h>
#elif !defined _WIN32
#define RUNNING_ON_VALGRIND false
#endif

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#ifdef UNX
#if defined( MACOSX )
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
# else
    extern char** environ;
# endif
#endif

using namespace osl;

/** get binary Path.
*/
static OUString getExecutablePath()
{
    OUString dirPath;
    osl::Module::getUrlFromAddress(
        reinterpret_cast<oslGenericFunction>(&getExecutablePath), dirPath);
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') );
    dirPath = OUString::Concat(dirPath.subView( 0, dirPath.lastIndexOf('/') + 1)) +
        "Executable";
    return dirPath;
}

#if !defined _WIN32

namespace {

class exclude
{
public:

    explicit exclude(const std::vector<OString>& exclude_list)
    {
        for (auto& exclude_list_item : exclude_list)
            exclude_list_.push_back(env_var_name(exclude_list_item));
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

        if (pos_equal_sign != -1)
            return env_var.copy(0, pos_equal_sign);

        return OString();
    }

private:
    std::vector<OString> exclude_list_;
};

    void tidy_container(std::vector<OString> &env_container)
    {
        //sort them because there are no guarantees to ordering
        std::sort(env_container.begin(), env_container.end());
        if (RUNNING_ON_VALGRIND)
        {
            std::erase_if(
                    env_container,
                    [](OString const & s) {
                        return s.startsWith("LD_PRELOAD=")
                            || s.startsWith("VALGRIND_LIB="); });
        }
    }
}

    static void read_parent_environment(std::vector<OString>* env_container)
    {
        for (int i = 0; environ[i] != nullptr; i++)
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
    static const int parameters_count_ = 2;
    OUString    suCWD;
    OUString    suExecutableFileURL;

public:

    // ctor
    Test_osl_executeProcess() :
        env_param_(u"-env"_ustr), suCWD(getExecutablePath())
    {
        parameters_[0] = env_param_.pData;

#if defined(_WIN32)
        suExecutableFileURL = suCWD + "/" "osl_process_child.exe";
#else
        suExecutableFileURL = suCWD + "/" "osl_process_child";
#endif
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

#if !defined _WIN32

    void read_child_environment(std::vector<OString>* env_container)
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
        std::vector<OString> parent_env;
        read_parent_environment(&parent_env);

        std::vector<OString> child_env;
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
    bool compare_merged_environments(const std::vector<OString>& different_env_vars)
    {
        std::vector<OString> parent_env;
        read_parent_environment(&parent_env);

        for (auto& env : parent_env)
            std::cout << "initially parent env: " << env << "\n";

        //remove the environment variables that we have changed
        //in the child environment from the read parent environment
        std::erase_if(parent_env, exclude(different_env_vars));

        for (auto& env : parent_env)
            std::cout << "stripped parent env: " << env << "\n";

        //read the child environment and exclude the variables that
        //are different
        std::vector<OString> child_env;
        read_child_environment(&child_env);

        for (auto& env : child_env)
            std::cout << "initial child env: " << env << "\n";
        //partition the child environment into the variables that
        //are different to the parent environment (they come first)
        //and the variables that should be equal between parent
        //and child environment
        auto iter_logical_end =
            std::stable_partition(child_env.begin(), child_env.end(), exclude(different_env_vars));

        std::vector<OString> different_child_env_vars(child_env.begin(), iter_logical_end);
        child_env.erase(child_env.begin(), iter_logical_end);

        for (auto& env : child_env)
            std::cout << "stripped child env: " << env << "\n";

        bool common_env_size_equals    = (parent_env.size() == child_env.size());
        bool common_env_content_equals = std::equal(child_env.begin(), child_env.end(), parent_env.begin());

        for (auto& env_var : different_env_vars)
            std::cout << "different should be: " << env_var << "\n";

        for (auto& env_var : different_child_env_vars)
            std::cout << "different are: " << env_var << "\n";

        bool different_env_size_equals    = (different_child_env_vars.size() == different_env_vars.size());
        bool different_env_content_equals =
            std::equal(different_env_vars.begin(), different_env_vars.end(), different_child_env_vars.begin());

        return (common_env_size_equals && common_env_content_equals &&
                different_env_size_equals && different_env_content_equals);
    }

    // test that parent and child process have the
    // same environment when osl_executeProcess will
    // be called without setting new environment
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
        OUString env1(u"" ENV1 ""_ustr);
        OUString env2(u"" ENV2 ""_ustr);
        OUString env3(u"" ENV3 ""_ustr);
        OUString env4(u"" ENV4 ""_ustr);

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

        std::vector<OString> different_child_env_vars
        {
            ENV1 ""_ostr,
            ENV2 ""_ostr,
            ENV4 ""_ostr
        };

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_execProc_merged_child_environment",
            compare_merged_environments(different_child_env_vars)
        );
    }

#endif

    void osl_execProc_test_batch()
    {
        oslProcess process;
#if defined(_WIN32)
        OUString suBatch = suCWD + "/batch.bat";
#else
        OUString suBatch = suCWD + "/batch.sh";
#endif
        oslProcessError osl_error = osl_executeProcess(
            suBatch.pData,
            nullptr,
            0,
            osl_Process_HIDDEN,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
