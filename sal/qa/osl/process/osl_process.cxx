/*************************************************************************
 *
 *  $RCSfile: osl_process.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:41:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <cppunit/simpleheader.hxx>
#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <unistd.h>
#include <signal.h>

#ifdef WNT
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <tchar.h>
#endif

#include "internal/allocator.hxx"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>

#ifdef WNT
    const rtl::OUString EXECUTABLE_NAME = rtl::OUString::createFromAscii("osl_process_child.exe");
#else
    const rtl::OUString EXECUTABLE_NAME = rtl::OUString::createFromAscii("osl_process_child");
#endif

const rtl::OUString CWD = rtl::OUString::createFromAscii(".");

//########################################
std::string OUString_to_std_string(const rtl::OUString& oustr)
{
    rtl::OString ostr = rtl::OUStringToOString(oustr, osl_getThreadTextEncoding());
    return std::string(ostr.getStr());
}

//########################################
using namespace osl;
using namespace rtl;

//########################################
class Test_osl_joinProcess : public CppUnit::TestFixture
{
    const OUString join_param_;
    const OUString wait_time_;

    rtl_uString* parameters_[2];
    int          parameters_count_;

public:

    Test_osl_joinProcess() :
        join_param_(OUString::createFromAscii("-join")),
        wait_time_(OUString::createFromAscii("1")),
        parameters_count_(2)
    {
        parameters_[0] = join_param_.pData;
        parameters_[1] = wait_time_.pData;
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
        oslProcessError osl_error = osl_executeProcess(
            EXECUTABLE_NAME.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        TimeValue timeout;
        timeout.Seconds = 1;
        timeout.Nanosec = 0;

        osl_error = osl_joinProcessWithTimeout(process, &timeout);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcessWithTimeout returned without timeout failure",
            osl_Process_E_TimedOut == osl_error
        );

        osl_error = osl_terminateProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_terminateProcess failed",
            osl_error == osl_Process_E_None
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
        oslProcessError osl_error = osl_executeProcess(
            EXECUTABLE_NAME.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        TimeValue timeout;
        timeout.Seconds = 10;
        timeout.Nanosec = 0;

        osl_error = osl_joinProcessWithTimeout(process, &timeout);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcessWithTimeout returned with failure",
            osl_Process_E_None == osl_error
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
        oslProcessError osl_error = osl_executeProcess(
            EXECUTABLE_NAME.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = osl_joinProcessWithTimeout(process, NULL);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcessWithTimeout returned with failure",
            osl_Process_E_None == osl_error
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
        oslProcessError osl_error = osl_executeProcess(
            EXECUTABLE_NAME.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            osl_getCurrentSecurity(),
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None == osl_error
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

typedef std::vector<std::string, sal::Allocator<std::string> >  string_container_t;
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

//#########################################################
class Test_osl_executeProcess : public CppUnit::TestFixture
{
    const OUString env_param_;

    OUString     temp_file_path_;
    rtl_uString* parameters_[2];
    int          parameters_count_;

public:

    //------------------------------------------------
    // ctor
    Test_osl_executeProcess() :
        env_param_(OUString::createFromAscii("-env")),
        parameters_count_(2)
    {
        parameters_[0] = env_param_.pData;
    }

    //------------------------------------------------
    virtual void setUp()
    {
        temp_file_path_ = create_temp_file();
        parameters_[1]  = temp_file_path_.pData;
    }

    //------------------------------------------------
    OUString create_temp_file()
    {
        OUString temp_file_url;
        FileBase::RC rc = FileBase::createTempFile(0, 0, &temp_file_url);
        CPPUNIT_ASSERT_MESSAGE("createTempFile failed", FileBase::E_None == rc);

        OUString temp_file_path;
        rc = FileBase::getSystemPathFromFileURL(temp_file_url, temp_file_path);
        CPPUNIT_ASSERT_MESSAGE("getSystemPathFromFileURL failed", FileBase::E_None == rc);

        return temp_file_path;
    }

    //------------------------------------------------
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
    }
#else
    extern char** environ;
    void read_parent_environment(string_container_t* env_container)
    {
        for (int i = 0; NULL != environ[i]; i++)
            env_container->push_back(std::string(environ[i]));
    }
#endif

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
        while (std::getline(file, line))
            env_container->push_back(line);
    }

    //------------------------------------------------
    void dump_env(const string_container_t& env, OUString file_name)
    {
        OString fname = OUStringToOString(file_name, osl_getThreadTextEncoding());
        std::ofstream file(fname.getStr());
        std::ostream_iterator<std::string> oi(file, "\n");
        std::copy(env.begin(), env.end(), oi);
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

        //remove the environment variables that we have changed
        //in the child environment from the read parent environment
        parent_env.erase(
            std::remove_if(parent_env.begin(), parent_env.end(), exclude(different_env_vars)),
            parent_env.end());

        //read the child environment and exclude the variables that
        //are different
        string_container_t child_env;
        read_child_environment(&child_env);

        //partition the child environment into the variables that
        //are different to the parent environment (they come first)
        //and the variables that should be equal between parent
        //and child environment
        string_container_iter_t iter_logical_end =
            std::stable_partition(child_env.begin(), child_env.end(), exclude(different_env_vars));

        string_container_t different_child_env_vars(child_env.begin(), iter_logical_end);
        child_env.erase(child_env.begin(), iter_logical_end);

        bool common_env_size_equals    = (parent_env.size() == child_env.size());
        bool common_env_content_equals = std::equal(child_env.begin(), child_env.end(), parent_env.begin());

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
            EXECUTABLE_NAME.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            NULL,
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None == osl_error
        );

        osl_freeProcessHandle(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "Parent an child environment not equal",
            compare_environments()
        );
    }

    //------------------------------------------------
    #define ENV1 "PAT=a:\\"
    #define ENV2 "PATH=b:\\"
    #define ENV3 "Patha=c:\\"
    #define ENV4 "Patha=d:\\"

    void osl_execProc_merged_child_environment()
    {
        rtl_uString* child_env[4];
        OUString env1 = OUString::createFromAscii(ENV1);
        OUString env2 = OUString::createFromAscii(ENV2);
        OUString env3 = OUString::createFromAscii(ENV3);
        OUString env4 = OUString::createFromAscii(ENV4);

        child_env[0] = env1.pData;
        child_env[1] = env2.pData;
        child_env[2] = env3.pData;
        child_env[3] = env4.pData;

        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            EXECUTABLE_NAME.pData,
            parameters_,
            parameters_count_,
            osl_Process_NORMAL,
            NULL,
            CWD.pData,
            child_env,
            sizeof(child_env)/sizeof(child_env[0]),
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None == osl_error
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
        rtl::OUString batch = rtl::OUString::createFromAscii("batch.bat");
        oslProcessError osl_error = osl_executeProcess(
            batch.pData,
            NULL,
            0,
            osl_Process_NORMAL,
            NULL,
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None == osl_error
        );

        osl_freeProcessHandle(process);
    }

    void osl_execProc_exe_name_in_argument_list()
    {
        rtl_uString* params[3];

        params[0] = EXECUTABLE_NAME.pData;
        params[1] = env_param_.pData;
        params[2] = temp_file_path_.pData;

        oslProcess process;
        oslProcessError osl_error = osl_executeProcess(
            NULL,
            params,
            3,
            osl_Process_NORMAL,
            NULL,
            CWD.pData,
            NULL,
            0,
            &process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_createProcess failed",
            osl_error == osl_Process_E_None
        );

        osl_error = ::osl_joinProcess(process);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_joinProcess returned with failure",
            osl_Process_E_None == osl_error
        );

        osl_freeProcessHandle(process);
    }

    CPPUNIT_TEST_SUITE(Test_osl_executeProcess);
    CPPUNIT_TEST(osl_execProc_parent_equals_child_environment);
    CPPUNIT_TEST(osl_execProc_merged_child_environment);
    CPPUNIT_TEST(osl_execProc_test_batch);
    CPPUNIT_TEST(osl_execProc_exe_name_in_argument_list);
    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test_osl_joinProcess,    "Test_osl_joinProcess");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test_osl_executeProcess, "Test_osl_executeProcess");

NOADDITIONAL;

