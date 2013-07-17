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

#define t_print printf

#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <osl/module.hxx>
#include <sal/macros.h>

#if ( defined WNT )                     // Windows
#   include <windows.h>
#   include <tchar.h>
#else
#include <unistd.h>
#endif

#include "rtl/allocator.hxx"

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

#if defined(WNT)
    const rtl::OUString EXECUTABLE_NAME ("osl_process_child.exe");
#else
    const rtl::OUString EXECUTABLE_NAME ("osl_process_child");
#endif

//########################################
using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    t_print("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", aString.getStr( ) );
}

/** get binary Path.
*/
inline ::rtl::OUString getExecutablePath( void )
{
    ::rtl::OUString dirPath;
    osl::Module::getUrlFromAddress( ( void* ) &getExecutablePath, dirPath );
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') );
    dirPath = dirPath.copy( 0, dirPath.lastIndexOf('/') + 1);
    dirPath += rtl::OUString("Executable");
    return dirPath;
}

//rtl::OUString CWD = getExecutablePath();

//########################################
class Test_osl_joinProcess : public CppUnit::TestFixture
{
    const OUString join_param_;
    const OUString wait_time_;
    OUString suCWD;
    OUString suExecutableFileURL;

    rtl_uString* parameters_[2];
    int          parameters_count_;

public:

    Test_osl_joinProcess() :
        join_param_(OUString("-join")),
        wait_time_(OUString("1")),
        parameters_count_(2)
    {
        parameters_[0] = join_param_.pData;
        parameters_[1] = wait_time_.pData;
        suCWD = getExecutablePath();
        suExecutableFileURL = suCWD;
        suExecutableFileURL += rtl::OUString("/");
        suExecutableFileURL += EXECUTABLE_NAME;
    }

    /*-------------------------------------
        Start a process and join with this
        process specify a timeout so that
        osl_joinProcessWithTimeout returns
        osl_Process_E_TimedOut
     -------------------------------------*/

    void osl_joinProcessWithTimeout_timeout_failure()
    {
        oslProcess process;
        oslSecurity security = osl_getCurrentSecurity();
        oslProcessError osl_error = osl_executeProcess(
            suExecutableFileURL.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            security,
            suCWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );

        TimeValue timeout;
        timeout.Seconds = 1;
        timeout.Nanosec = 0;

        osl_error = osl_joinProcessWithTimeout(process, &timeout);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_joinProcessWithTimeout returned without timeout failure",
            osl_Process_E_TimedOut, osl_error
        );

        osl_error = osl_terminateProcess(process);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_terminateProcess failed",
            osl_Process_E_None, osl_error
        );

        osl_freeProcessHandle(process);
    }

    /*-------------------------------------
        Start a process and join with this
        process specify a timeout so that
        osl_joinProcessWithTimeout returns
        osl_Process_E_None
     -------------------------------------*/

    void osl_joinProcessWithTimeout_without_timeout_failure()
    {
        oslProcess process;
        oslSecurity security = osl_getCurrentSecurity ();
        oslProcessError osl_error = osl_executeProcess(
            suExecutableFileURL.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            security,
            suCWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );

        TimeValue timeout;
        timeout.Seconds = 10;
        timeout.Nanosec = 0;

        osl_error = osl_joinProcessWithTimeout(process, &timeout);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_joinProcessWithTimeout returned with failure",
            osl_Process_E_None, osl_error
        );

        osl_freeProcessHandle(process);
    }

     /*-------------------------------------
        Start a process and join with this
        process specify an infinite timeout
     -------------------------------------*/

    void osl_joinProcessWithTimeout_infinite()
    {
        oslProcess process;
        oslSecurity security = osl_getCurrentSecurity ();
        oslProcessError osl_error = osl_executeProcess(
            suExecutableFileURL.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            security,
            suCWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_createProcess failed",
            osl_Process_E_None, osl_error
        );

        osl_error = osl_joinProcessWithTimeout(process, NULL);

        CPPUNIT_ASSERT_EQUAL_MESSAGE
        (
            "osl_joinProcessWithTimeout returned with failure",
            osl_Process_E_None, osl_error
        );

        osl_freeProcessHandle(process);
    }

     /*-------------------------------------
        Start a process and join with this
        process using osl_joinProcess
     -------------------------------------*/

     void osl_joinProcess()
    {
        oslProcess process;
        oslSecurity security = osl_getCurrentSecurity ();
        oslProcessError osl_error = osl_executeProcess(
            suExecutableFileURL.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            security,
            suCWD.pData,
            NULL,
            0,
            &process);
        osl_freeSecurityHandle(security);

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

    CPPUNIT_TEST_SUITE(Test_osl_joinProcess);
    CPPUNIT_TEST(osl_joinProcessWithTimeout_timeout_failure);
    CPPUNIT_TEST(osl_joinProcessWithTimeout_without_timeout_failure);
    CPPUNIT_TEST(osl_joinProcessWithTimeout_infinite);
    CPPUNIT_TEST(osl_joinProcess);
    CPPUNIT_TEST_SUITE_END();
};

//#########################################################

typedef std::vector<std::string, rtl::Allocator<std::string> >  string_container_t;
typedef string_container_t::const_iterator string_container_const_iter_t;
typedef string_container_t::iterator       string_container_iter_t;

//#########################################################
class exclude : public std::unary_function<std::string, bool>
{
public:
    //------------------------------------------------
    exclude(const string_container_t& exclude_list)
    {
        string_container_const_iter_t iter     = exclude_list.begin();
        string_container_const_iter_t iter_end = exclude_list.end();
        for (/**/; iter != iter_end; ++iter)
            exclude_list_.push_back(env_var_name(*iter));
    }

    //------------------------------------------------
    bool operator() (const std::string& env_var) const
    {
        return (exclude_list_.end() !=
                std::find(
                    exclude_list_.begin(),
                    exclude_list_.end(),
                    env_var_name(env_var)));
    }

private:
    //-------------------------------------------------
    // extract the name from an environment variable
    // that is given in the form "NAME=VALUE"
    std::string env_var_name(const std::string& env_var) const
    {
        std::string::size_type pos_equal_sign =
            env_var.find_first_of("=");

        if (std::string::npos != pos_equal_sign)
            return std::string(env_var, 0, pos_equal_sign);

        return std::string();
    }

private:
    string_container_t exclude_list_;
};

namespace
{
    class starts_with
        : public std::unary_function<const std::string&, bool>
    {
    private:
        const std::string m_rString;
    public:
        starts_with(const char *pString) : m_rString(pString) {}
        bool operator()(const std::string &rEntry) const
        {
            return rEntry.find(m_rString) == 0;
        }
    };

    void tidy_container(string_container_t &env_container)
    {
        //sort them because there are no guarantees to ordering
        std::sort(env_container.begin(), env_container.end());
        //remove LD_PRELOAD because valgrind injects that into the
        //parent process
        env_container.erase(std::remove_if(env_container.begin(), env_container.end(),
            starts_with("LD_PRELOAD=")), env_container.end());
    }
}

#ifdef WNT
    void read_parent_environment(string_container_t* env_container)
    {
        LPTSTR env = reinterpret_cast<LPTSTR>(GetEnvironmentStrings());
        LPTSTR p   = env;

        while (size_t l = _tcslen(p))
        {
            env_container->push_back(std::string(p));
            p += l + 1;
        }
        FreeEnvironmentStrings(env);
        tidy_container(*env_container);
    }
#else
    void read_parent_environment(string_container_t* env_container)
    {
        for (int i = 0; NULL != environ[i]; i++)
            env_container->push_back(std::string(environ[i]));
        tidy_container(*env_container);
    }
#endif

//#########################################################
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

    //------------------------------------------------
    // ctor
    Test_osl_executeProcess() :
        env_param_(OUString("-env")),
        parameters_count_(2)
    {
        parameters_[0] = env_param_.pData;
        suCWD = getExecutablePath();
        suExecutableFileURL = suCWD;
        suExecutableFileURL += rtl::OUString("/");
        suExecutableFileURL += EXECUTABLE_NAME;
    }

    //------------------------------------------------
    virtual void setUp()
    {
        temp_file_path_ = create_temp_file(temp_file_url_);
        parameters_[1]  = temp_file_path_.pData;
    }

    virtual void tearDown()
    {
        osl::File::remove(temp_file_url_);
    }

    //------------------------------------------------
    OUString create_temp_file(OUString &temp_file_url)
    {
        FileBase::RC rc = FileBase::createTempFile(0, 0, &temp_file_url);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("createTempFile failed", FileBase::E_None, rc);

        OUString temp_file_path;
        rc = FileBase::getSystemPathFromFileURL(temp_file_url, temp_file_path);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getSystemPathFromFileURL failed", FileBase::E_None, rc);

        return temp_file_path;
    }

   //------------------------------------------------
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
            env_container->push_back(line);
        tidy_container(*env_container);
    }

    //------------------------------------------------
    // environment of the child process that was
    // started. The child process writes his
    // environment into a file
    bool compare_environments()
    {
        string_container_t parent_env;
        read_parent_environment(&parent_env);

        string_container_t child_env;
        read_child_environment(&child_env);

        return ((parent_env.size() == child_env.size()) &&
                (std::equal(child_env.begin(), child_env.end(), parent_env.begin())));
    }

    //------------------------------------------------
    // compare the equal environment parts and the
    // different part of the child environment
    bool compare_merged_environments(const string_container_t& different_env_vars)
    {
        string_container_t parent_env;
        read_parent_environment(&parent_env);

#if OSL_DEBUG_LEVEL > 1
        for (string_container_t::const_iterator iter = parent_env.begin(), end = parent_env.end(); iter != end; ++iter)
            std::cerr << "initially parent env: " << *iter << std::endl;
#endif

        //remove the environment variables that we have changed
        //in the child environment from the read parent environment
        parent_env.erase(
            std::remove_if(parent_env.begin(), parent_env.end(), exclude(different_env_vars)),
            parent_env.end());

#if OSL_DEBUG_LEVEL > 1
        for (string_container_t::const_iterator iter = parent_env.begin(), end = parent_env.end(); iter != end; ++iter)
            std::cerr << "stripped parent env: " << *iter << std::endl;
#endif

        //read the child environment and exclude the variables that
        //are different
        string_container_t child_env;
        read_child_environment(&child_env);

#if OSL_DEBUG_LEVEL > 1
        for (string_container_t::const_iterator iter = child_env.begin(), end = child_env.end(); iter != end; ++iter)
            std::cerr << "initial child env: " << *iter << std::endl;
#endif
        //partition the child environment into the variables that
        //are different to the parent environment (they come first)
        //and the variables that should be equal between parent
        //and child environment
        string_container_iter_t iter_logical_end =
            std::stable_partition(child_env.begin(), child_env.end(), exclude(different_env_vars));

        string_container_t different_child_env_vars(child_env.begin(), iter_logical_end);
        child_env.erase(child_env.begin(), iter_logical_end);

#if OSL_DEBUG_LEVEL > 1
        for (string_container_t::const_iterator iter = child_env.begin(), end = child_env.end(); iter != end; ++iter)
            std::cerr << "stripped child env: " << *iter << std::endl;
#endif

        bool common_env_size_equals    = (parent_env.size() == child_env.size());
        bool common_env_content_equals = std::equal(child_env.begin(), child_env.end(), parent_env.begin());

#if OSL_DEBUG_LEVEL > 1
        for (string_container_t::const_iterator iter = different_env_vars.begin(), end = different_env_vars.end(); iter != end; ++iter)
            std::cerr << "different should be: " << *iter << std::endl;
#endif


#if OSL_DEBUG_LEVEL > 1
        for (string_container_t::const_iterator iter = different_child_env_vars.begin(), end = different_child_env_vars.end(); iter != end; ++iter)
            std::cerr << "different are: " << *iter << std::endl;
#endif

        bool different_env_size_equals    = (different_child_env_vars.size() == different_env_vars.size());
        bool different_env_content_equals =
            std::equal(different_env_vars.begin(), different_env_vars.end(), different_child_env_vars.begin());

        return (common_env_size_equals && common_env_content_equals &&
                different_env_size_equals && different_env_content_equals);
    }

    //------------------------------------------------
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
            NULL,
            suCWD.pData,
            NULL,
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

        CPPUNIT_ASSERT_MESSAGE
        (
            "Parent and child environment not equal",
            compare_environments()
        );
    }

    //------------------------------------------------
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
            NULL,
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
#if defined(WNT)
        rtl::OUString suBatch = suCWD + rtl::OUString("/") + rtl::OUString("batch.bat");
#else
        rtl::OUString suBatch = suCWD + rtl::OUString("/") + rtl::OUString("batch.sh");
#endif
        oslProcessError osl_error = osl_executeProcess(
            suBatch.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            NULL,
            suCWD.pData,
            NULL,
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

    void osl_execProc_exe_name_in_argument_list()
    {
        rtl_uString* params[3];

        params[0] = suExecutableFileURL.pData;
        params[1] = env_param_.pData;
        params[2] = temp_file_path_.pData;
        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            NULL,
            params,
            3,
            osl_Process_NORMAL,
            NULL,
            suCWD.pData,
            NULL,
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
    //TODO: Repair these under windows.
#ifndef WNT
    CPPUNIT_TEST(osl_execProc_parent_equals_child_environment);
    CPPUNIT_TEST(osl_execProc_merged_child_environment);
#endif
    CPPUNIT_TEST(osl_execProc_test_batch);
    ///TODO: Repair test (or tested function ;-) - test fails.
    // CPPUNIT_TEST(osl_execProc_exe_name_in_argument_list);
    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test_osl_joinProcess,    "Test_osl_joinProcess");
CPPUNIT_TEST_SUITE_REGISTRATION(Test_osl_executeProcess);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
