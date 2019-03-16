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

#include <sal/config.h>
#include <rtl/ustring.hxx>

#include <cassert>

/*
 *   ToDo:
 *      - cleanup of process status things
 *      - cleanup of process spawning
 *      - cleanup of resource transfer
 */

#if defined(__sun)
  // The procfs may only be used without LFS in 32bits.
# ifdef _FILE_OFFSET_BITS
#   undef   _FILE_OFFSET_BITS
# endif
#endif

#if defined(FREEBSD) || defined(NETBSD) || defined(DRAGONFLY)
#include <machine/param.h>
#endif

#include "system.hxx"
#include "unixerrnostring.hxx"
#if defined(__sun)
# include <sys/procfs.h>
#endif
#include <osl/diagnose.h>
#include <osl/mutex.h>
#include <osl/process.h>
#include <osl/conditn.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <osl/file.hxx>
#include <osl/signal.h>
#include <rtl/alloc.h>
#include <sal/log.hxx>

#include <grp.h>

#include "createfilehandlefromfd.hxx"
#include "file_url.hxx"
#include "readwrite_helper.hxx"
#include "sockimpl.hxx"
#include "secimpl.hxx"

#define MAX_ARGS        255
#define MAX_ENVS        255

namespace
{

struct oslProcessImpl {
    pid_t           m_pid;
    oslCondition    m_terminated;
    int             m_status;
    oslProcessImpl* m_pnext;
};

struct ProcessData
{
    const sal_Char*  m_pszArgs[MAX_ARGS + 1];
    const sal_Char*  m_pszDir;
    sal_Char*        m_pszEnv[MAX_ENVS + 1];
    uid_t            m_uid;
    gid_t            m_gid;
    sal_Char*        m_name;
    oslCondition     m_started;
    oslProcessImpl*  m_pProcImpl;
    oslFileHandle    *m_pInputWrite;
    oslFileHandle    *m_pOutputRead;
    oslFileHandle    *m_pErrorRead;
};

static oslProcessImpl* ChildList;
static oslMutex        ChildListMutex;

} //Anonymous namespace

static oslProcessError osl_psz_executeProcess(sal_Char *pszImageName,
                                                sal_Char *pszArguments[],
                                                oslProcessOption Options,
                                                oslSecurity Security,
                                                sal_Char *pszDirectory,
                                                sal_Char *pszEnvironments[],
                                                oslProcess *pProcess,
                                                oslFileHandle *pInputWrite,
                                                oslFileHandle *pOutputRead,
                                                oslFileHandle *pErrorRead );

extern "C" {

static void ChildStatusProc(void *pData)
{
    osl_setThreadName("osl_executeProcess");

    pid_t pid = -1;
    int   status = 0;
    int   channel[2] = { -1, -1 };
    ProcessData  data;
    ProcessData *pdata;
    int     stdOutput[2] = { -1, -1 }, stdInput[2] = { -1, -1 }, stdError[2] = { -1, -1 };

    pdata = static_cast<ProcessData *>(pData);

    /* make a copy of our data, because forking will only copy
       our local stack of the thread, so the process data will not be accessible
       in our child process */
    memcpy(&data, pData, sizeof(data));

#ifdef NO_CHILD_PROCESSES
#define fork() (errno = EINVAL, -1)
#endif
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, channel) == -1)
    {
        status = errno;
        SAL_WARN("sal.osl", "executeProcess socketpair() errno " << status);
    }

    (void) fcntl(channel[0], F_SETFD, FD_CLOEXEC);
    (void) fcntl(channel[1], F_SETFD, FD_CLOEXEC);

    /* Create redirected IO pipes */
    if ( status == 0 && data.m_pInputWrite && pipe( stdInput ) == -1 )
    {
        status = errno;
        assert(status != 0);
        SAL_WARN("sal.osl", "executeProcess pipe(stdInput) errno " << status);
    }

    if ( status == 0 && data.m_pOutputRead && pipe( stdOutput ) == -1 )
    {
        status = errno;
        assert(status != 0);
        SAL_WARN("sal.osl", "executeProcess pipe(stdOutput) errno " << status);
    }

    if ( status == 0 && data.m_pErrorRead && pipe( stdError ) == -1 )
    {
        status = errno;
        assert(status != 0);
        SAL_WARN("sal.osl", "executeProcess pipe(stdError) errno " << status);
    }

    if ( (status == 0) && ((pid = fork()) == 0) )
    {
        /* Child */
        int chstatus = 0;
        int errno_copy;

        if (channel[0] != -1) close(channel[0]);

        if ((data.m_uid != uid_t(-1)) && ((data.m_uid != getuid()) || (data.m_gid != getgid())))
        {
            OSL_ASSERT(geteuid() == 0);     /* must be root */

            if (! INIT_GROUPS(data.m_name, data.m_gid) || (setuid(data.m_uid) != 0))
                SAL_WARN("sal.osl", "Failed to change uid and guid: " << UnixErrnoString(errno));

            const OUString envVar("HOME");
            osl_clearEnvironment(envVar.pData);
        }

        if (data.m_pszDir)
            chstatus = chdir(data.m_pszDir);

        if (chstatus == 0 && ((data.m_uid == uid_t(-1)) || ((data.m_uid == getuid()) && (data.m_gid == getgid()))))
        {
            int i;
            for (i = 0; data.m_pszEnv[i] != nullptr; i++)
            {
                if (strchr(data.m_pszEnv[i], '=') == nullptr)
                {
                    unsetenv(data.m_pszEnv[i]); /*TODO: check error return*/
                }
                else
                {
                    putenv(data.m_pszEnv[i]); /*TODO: check error return*/
                }
            }

            /* Connect std IO to pipe ends */

            /* Write end of stdInput not used in child process */
            if (stdInput[1] != -1) close( stdInput[1] );

            /* Read end of stdOutput not used in child process */
            if (stdOutput[0] != -1) close( stdOutput[0] );

            /* Read end of stdError not used in child process */
            if (stdError[0] != -1) close( stdError[0] );

            /* Redirect pipe ends to std IO */

            if ( stdInput[0] != STDIN_FILENO )
            {
                dup2( stdInput[0], STDIN_FILENO );
                if (stdInput[0] != -1) close( stdInput[0] );
            }

            if ( stdOutput[1] != STDOUT_FILENO )
            {
                dup2( stdOutput[1], STDOUT_FILENO );
                if (stdOutput[1] != -1) close( stdOutput[1] );
            }

            if ( stdError[1] != STDERR_FILENO )
            {
                dup2( stdError[1], STDERR_FILENO );
                if (stdError[1] != -1) close( stdError[1] );
            }

            // No need to check the return value of execv. If we return from
            // it, an error has occurred.
            execv(data.m_pszArgs[0], const_cast<char **>(data.m_pszArgs));
        }

        SAL_WARN("sal.osl", "ChildStatusProc : Failed to exec <" << data.m_pszArgs[0] << ">: " << UnixErrnoString(errno));

        /* if we reach here, something went wrong */
        errno_copy = errno;
        if ( !safeWrite(channel[1], &errno_copy, sizeof(errno_copy)) )
            SAL_WARN("sal.osl", "sendFdPipe : sending failed:  " << UnixErrnoString(errno));

        if ( channel[1] != -1 )
            close(channel[1]);

        _exit(255);
    }
    else
    {   /* Parent  */
        int i = -1;
        if (channel[1] != -1) close(channel[1]);

        /* Close unused pipe ends */
        if (stdInput[0] != -1) close( stdInput[0] );
        if (stdOutput[1] != -1) close( stdOutput[1] );
        if (stdError[1] != -1) close( stdError[1] );

        if (pid > 0)
        {
            while ((i = read(channel[0], &status, sizeof(status))) < 0)
            {
                if (errno != EINTR)
                    break;
            }
        }

        if (channel[0] != -1) close(channel[0]);

        if ((pid > 0) && (i == 0))
        {
            pid_t   child_pid;
            osl_acquireMutex(ChildListMutex);

            pdata->m_pProcImpl->m_pid = pid;
            pdata->m_pProcImpl->m_pnext = ChildList;
            ChildList = pdata->m_pProcImpl;

            /* Store used pipe ends in data structure */

            if ( pdata->m_pInputWrite )
                *(pdata->m_pInputWrite) = osl::detail::createFileHandleFromFD( stdInput[1] );

            if ( pdata->m_pOutputRead )
                *(pdata->m_pOutputRead) = osl::detail::createFileHandleFromFD( stdOutput[0] );

            if ( pdata->m_pErrorRead )
                *(pdata->m_pErrorRead) = osl::detail::createFileHandleFromFD( stdError[0] );

            osl_releaseMutex(ChildListMutex);

            osl_setCondition(pdata->m_started);

            do
            {
                child_pid = waitpid(pid, &status, 0);
            } while ( 0 > child_pid && EINTR == errno );

            if ( child_pid < 0)
            {
                SAL_WARN("sal.osl", "Failed to wait for child process: " << UnixErrnoString(errno));

                /*
                We got another error than EINTR. Anyway we have to wake up the
                waiting thread under any circumstances */

                child_pid = pid;
            }

            if ( child_pid > 0 )
            {
                oslProcessImpl* pChild;

                osl_acquireMutex(ChildListMutex);

                pChild = ChildList;

                /* check if it is one of our child processes */
                while (pChild != nullptr)
                {
                    if (pChild->m_pid == child_pid)
                    {
                        if (WIFEXITED(status))
                            pChild->m_status = WEXITSTATUS(status);
                        else if (WIFSIGNALED(status))
                            pChild->m_status = 128 + WTERMSIG(status);
                        else
                            pChild->m_status = -1;

                        osl_setCondition(pChild->m_terminated);
                    }

                    pChild = pChild->m_pnext;
                }

                osl_releaseMutex(ChildListMutex);
            }
        }
        else
        {
            SAL_WARN("sal.osl", "ChildStatusProc : starting '" << data.m_pszArgs[0] << "' failed");
            SAL_WARN("sal.osl", "Failed to launch child process, child reports " << UnixErrnoString(status));

            /* Close pipe ends */
            if ( pdata->m_pInputWrite )
                *pdata->m_pInputWrite = nullptr;

            if ( pdata->m_pOutputRead )
                *pdata->m_pOutputRead = nullptr;

            if ( pdata->m_pErrorRead )
                *pdata->m_pErrorRead = nullptr;

            if (stdInput[1] != -1) close( stdInput[1] );
            if (stdOutput[0] != -1) close( stdOutput[0] );
            if (stdError[0] != -1) close( stdError[0] );

            /* if pid > 0 then a process was created, even if it later failed
               e.g. bash searching for a command to execute, and we still
               need to clean it up to avoid "defunct" processes */
            if (pid > 0)
            {
                pid_t child_pid;
                do
                {
                    child_pid = waitpid(pid, &status, 0);
                } while ( 0 > child_pid && EINTR == errno );
            }

            /* notify (and unblock) parent thread */
            osl_setCondition(pdata->m_started);
        }
    }
}

}

oslProcessError SAL_CALL osl_executeProcess_WithRedirectedIO(
                                            rtl_uString *ustrImageName,
                                            rtl_uString *ustrArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *ustrWorkDir,
                                            rtl_uString *ustrEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslProcess *pProcess,
                                            oslFileHandle   *pInputWrite,
                                            oslFileHandle   *pOutputRead,
                                            oslFileHandle   *pErrorRead
                                            )
{
    OUString image;
    if (ustrImageName == nullptr)
    {
        if (nArguments == 0)
        {
            return osl_Process_E_InvalidError;
        }
        image = OUString::unacquired(ustrArguments);
    }
    else
    {
        osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
            OUString::unacquired(&ustrImageName), image);
        if (e != osl::FileBase::E_None)
        {
            SAL_INFO(
                "sal.osl",
                "getSystemPathFromFileURL("
                    << OUString::unacquired(&ustrImageName)
                    << ") failed with " << e);
            return osl_Process_E_Unknown;
        }
    }

    if ((Options & osl_Process_SEARCHPATH) != 0)
    {
        OUString path;
        if (osl::detail::find_in_PATH(image, path))
        {
            image = path;
        }
    }

    oslProcessError Error;
    sal_Char* pszWorkDir=nullptr;
    sal_Char** pArguments=nullptr;
    sal_Char** pEnvironment=nullptr;
    unsigned int idx;

    char szImagePath[PATH_MAX] = "";
    if (!image.isEmpty()
        && (UnicodeToText(
                szImagePath, SAL_N_ELEMENTS(szImagePath), image.getStr(),
                image.getLength())
            == 0))
    {
        int e = errno;
        SAL_INFO("sal.osl", "UnicodeToText(" << image << ") failed with " << e);
        return osl_Process_E_Unknown;
    }

    char szWorkDir[PATH_MAX] = "";
    if ( ustrWorkDir != nullptr && ustrWorkDir->length )
    {
        oslFileError e = FileURLToPath( szWorkDir, PATH_MAX, ustrWorkDir );
        if (e != osl_File_E_None)
        {
            SAL_INFO(
                "sal.osl",
                "FileURLToPath(" << OUString::unacquired(&ustrWorkDir)
                    << ") failed with " << e);
            return osl_Process_E_Unknown;
        }
        pszWorkDir = szWorkDir;
    }

    if ( pArguments == nullptr && nArguments > 0 )
    {
        pArguments = static_cast<sal_Char**>(malloc( ( nArguments + 2 ) * sizeof(sal_Char*) ));
    }

    for ( idx = 0 ; idx < nArguments ; ++idx )
    {
        rtl_String* strArg =nullptr;

        rtl_uString2String( &strArg,
                            rtl_uString_getStr(ustrArguments[idx]),
                            rtl_uString_getLength(ustrArguments[idx]),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pArguments[idx]=strdup(rtl_string_getStr(strArg));
        rtl_string_release(strArg);
        pArguments[idx+1]=nullptr;
    }

    for ( idx = 0 ; idx < nEnvironmentVars ; ++idx )
    {
        rtl_String* strEnv=nullptr;

        if ( pEnvironment == nullptr )
        {
            pEnvironment = static_cast<sal_Char**>(malloc( ( nEnvironmentVars + 2 ) * sizeof(sal_Char*) ));
        }

        rtl_uString2String( &strEnv,
                            rtl_uString_getStr(ustrEnvironment[idx]),
                            rtl_uString_getLength(ustrEnvironment[idx]),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pEnvironment[idx]=strdup(rtl_string_getStr(strEnv));
        rtl_string_release(strEnv);
        pEnvironment[idx+1]=nullptr;
    }

    Error = osl_psz_executeProcess(szImagePath,
                                   pArguments,
                                   Options,
                                   Security,
                                   pszWorkDir,
                                   pEnvironment,
                                   pProcess,
                                   pInputWrite,
                                   pOutputRead,
                                   pErrorRead
                                   );

    if ( pArguments != nullptr )
    {
        for ( idx = 0 ; idx < nArguments ; ++idx )
        {
            if ( pArguments[idx] != nullptr )
            {
                free(pArguments[idx]);
            }
        }
        free(pArguments);
    }

    if ( pEnvironment != nullptr )
    {
        for ( idx = 0 ; idx < nEnvironmentVars ; ++idx )
        {
            if ( pEnvironment[idx] != nullptr )
            {
                free(pEnvironment[idx]);
            }
        }
        free(pEnvironment);
    }

    return Error;
}

oslProcessError SAL_CALL osl_executeProcess(
                                            rtl_uString *ustrImageName,
                                            rtl_uString *ustrArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *ustrWorkDir,
                                            rtl_uString *ustrEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslProcess *pProcess
                                            )
{
    return osl_executeProcess_WithRedirectedIO(
        ustrImageName,
        ustrArguments,
        nArguments,
        Options,
        Security,
        ustrWorkDir,
        ustrEnvironment,
        nEnvironmentVars,
        pProcess,
        nullptr,
        nullptr,
        nullptr
        );
}

oslProcessError osl_psz_executeProcess(sal_Char *pszImageName,
                                                sal_Char *pszArguments[],
                                                oslProcessOption Options,
                                                oslSecurity Security,
                                                sal_Char *pszDirectory,
                                                sal_Char *pszEnvironments[],
                                                oslProcess *pProcess,
                                                oslFileHandle   *pInputWrite,
                                                oslFileHandle   *pOutputRead,
                                                oslFileHandle   *pErrorRead
                                                )
{
    int i;
    ProcessData Data;
    oslThread hThread;

    memset(&Data,0,sizeof(ProcessData));
    Data.m_pInputWrite = pInputWrite;
    Data.m_pOutputRead = pOutputRead;
    Data.m_pErrorRead = pErrorRead;

    OSL_ASSERT(pszImageName != nullptr);

    if ( pszImageName == nullptr )
    {
        return osl_Process_E_NotFound;
    }

    Data.m_pszArgs[0] = strdup(pszImageName);
    Data.m_pszArgs[1] = nullptr;

    if ( pszArguments != nullptr )
    {
        for (i = 0; ((i + 2) < MAX_ARGS) && (pszArguments[i] != nullptr); i++)
            Data.m_pszArgs[i+1] = strdup(pszArguments[i]);
        Data.m_pszArgs[i+2] = nullptr;
    }

    Data.m_pszDir  = (pszDirectory != nullptr) ? strdup(pszDirectory) : nullptr;

    if (pszEnvironments != nullptr)
    {
        for (i = 0; ((i + 1) < MAX_ENVS) &&  (pszEnvironments[i] != nullptr); i++)
            Data.m_pszEnv[i] = strdup(pszEnvironments[i]);
        Data.m_pszEnv[i+1] = nullptr;
    }
    else
         Data.m_pszEnv[0] = nullptr;

    if (Security != nullptr)
    {
        Data.m_uid  = static_cast<oslSecurityImpl*>(Security)->m_pPasswd.pw_uid;
        Data.m_gid  = static_cast<oslSecurityImpl*>(Security)->m_pPasswd.pw_gid;
        Data.m_name = static_cast<oslSecurityImpl*>(Security)->m_pPasswd.pw_name;
    }
    else
        Data.m_uid = uid_t(-1);

    Data.m_pProcImpl = static_cast<oslProcessImpl*>(malloc(sizeof(oslProcessImpl)));
    Data.m_pProcImpl->m_pid = 0;
    Data.m_pProcImpl->m_terminated = osl_createCondition();
    Data.m_pProcImpl->m_pnext = nullptr;

    if (ChildListMutex == nullptr)
        ChildListMutex = osl_createMutex();

    Data.m_started = osl_createCondition();

    hThread = osl_createThread(ChildStatusProc, &Data);

    if (hThread != nullptr)
    {
        osl_waitCondition(Data.m_started, nullptr);
    }
    osl_destroyCondition(Data.m_started);

    for (i = 0; Data.m_pszArgs[i] != nullptr; i++)
          free(const_cast<char *>(Data.m_pszArgs[i]));

    for (i = 0; Data.m_pszEnv[i] != nullptr; i++)
          free(Data.m_pszEnv[i]);

    if ( Data.m_pszDir != nullptr )
    {
        free(const_cast<char *>(Data.m_pszDir));
    }

    osl_destroyThread(hThread);

    if (Data.m_pProcImpl->m_pid != 0)
    {
        assert(hThread != nullptr);

        *pProcess = Data.m_pProcImpl;

        if (Options & osl_Process_WAIT)
            osl_joinProcess(*pProcess);

        return osl_Process_E_None;
    }

    osl_destroyCondition(Data.m_pProcImpl->m_terminated);
    free(Data.m_pProcImpl);

    return osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process)
{
    if (Process == nullptr)
        return osl_Process_E_Unknown;

    if (kill(static_cast<oslProcessImpl*>(Process)->m_pid, SIGKILL) != 0)
    {
        switch (errno)
        {
            case EPERM:
                return osl_Process_E_NoPermission;

            case ESRCH:
                return osl_Process_E_NotFound;

            default:
                return osl_Process_E_Unknown;
        }
    }

    return osl_Process_E_None;
}

oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident)
{
    oslProcessImpl *pProcImpl;

    if (kill(Ident, 0) != -1)
    {
        oslProcessImpl* pChild;

        if (ChildListMutex == nullptr)
            ChildListMutex = osl_createMutex();

        osl_acquireMutex(ChildListMutex);

        pChild = ChildList;

        /* check if it is one of our child processes */
        while (pChild != nullptr)
        {
            if (Ident == static_cast<sal_uInt32>(pChild->m_pid))
                break;

            pChild = pChild->m_pnext;
        }

        pProcImpl = static_cast<oslProcessImpl*>(malloc(sizeof(oslProcessImpl)));
        pProcImpl->m_pid        = Ident;
        pProcImpl->m_terminated = osl_createCondition();

        if (pChild != nullptr)
        {
            /* process is a child so insert into list */
            pProcImpl->m_pnext = pChild->m_pnext;
            pChild->m_pnext = pProcImpl;

            pProcImpl->m_status = pChild->m_status;

            if (osl_checkCondition(pChild->m_terminated))
                osl_setCondition(pProcImpl->m_terminated);
        }
        else
            pProcImpl->m_pnext = nullptr;

        osl_releaseMutex(ChildListMutex);
    }
    else
        pProcImpl = nullptr;

    return pProcImpl;
}

void SAL_CALL osl_freeProcessHandle(oslProcess Process)
{
    if (Process != nullptr)
    {
        oslProcessImpl *pChild, *pPrev = nullptr;

        OSL_ASSERT(ChildListMutex != nullptr);

        if ( ChildListMutex == nullptr )
        {
            return;
        }

        osl_acquireMutex(ChildListMutex);

        pChild = ChildList;

        /* remove process from child list */
        while (pChild != nullptr)
        {
            if (pChild == static_cast<oslProcessImpl*>(Process))
            {
                if (pPrev != nullptr)
                    pPrev->m_pnext = pChild->m_pnext;
                else
                    ChildList = pChild->m_pnext;

                break;
            }

            pPrev  = pChild;
            pChild = pChild->m_pnext;
        }

        osl_releaseMutex(ChildListMutex);

        osl_destroyCondition(static_cast<oslProcessImpl*>(Process)->m_terminated);

        free(Process);
    }
}

#if defined(LINUX)
struct osl_procStat
{
   /* from 'stat' */
    pid_t pid;                /* pid */
    char command[16];         /* 'argv[0]' */ /* mfe: it all right char comm[16] in kernel! */
    char state;               /* state (running, stopped, ...) */
    pid_t ppid;               /* parent pid */
    pid_t pgrp;               /* parent group */
    int session;              /* session ID */
    int tty;                  /* no of tty */
    pid_t tpgid;              /* group of process owning the tty */
    unsigned long flags;      /* flags dunno */
    unsigned long minflt;     /* minor page faults */
    unsigned long cminflt;    /* minor page faults with children */
    unsigned long majflt;     /* major page faults */
    unsigned long cmajflt;    /* major page faults with children */
    unsigned long utime;      /* no of jiffies in user mode */
    unsigned long stime;      /* no of jiffies in kernel mode */
    unsigned long cutime;     /* no of jiffies in user mode with children */
    unsigned long cstime;     /* no of jiffies in kernel mode with children */
    unsigned long priority;   /* nice value + 15 (kernel scheduling prio)*/
    long nice;                /* nice value */
    long timeout;             /* no of jiffies of next process timeout */
    long itrealvalue;         /* no jiffies before next SIGALRM */
    unsigned long starttime;  /* process started this no of jiffies after boot */
    unsigned long vsize;      /* virtual memory size (in bytes) */
    long rss;                 /* resident set size (in pages) */
    unsigned long rss_rlim;   /* rss limit (in bytes) */
    unsigned long startcode;   /* address above program text can run */
    unsigned long endcode;    /* address below program text can run */
    unsigned long startstack; /* address of start of stack */
    unsigned long kstkesp;    /* current value of 'esp' (stack pointer) */
    unsigned long kstkeip;    /* current value of 'eip' (instruction pointer) */
    /* mfe: Linux > 2.1.7x have more signals (88) */
    char signal[24];          /* pending signals */
    char blocked[24];         /* blocked signals */
    char sigignore[24];       /* ignored signals */
    char sigcatch[24];        /* caught signals */
    unsigned long wchan;      /* 'channel' the process is waiting in */
    unsigned long nswap;      /* ? */
    unsigned long cnswap;     /* ? */

    /* from 'status' */
    int ruid;                 /* real uid */
    int euid;                 /* effective uid */
    int suid;                 /* saved uid */
    int fuid;                 /* file access uid */
    int rgid;                 /* real gid */
    int egid;                 /* effective gid */
    int sgid;                 /* saved gid */
    int fgid;                 /* file access gid */
    unsigned long vm_size;    /* like vsize but on kb */
    unsigned long vm_lock;    /* locked pages in kb */
    unsigned long vm_rss;     /* like rss but in kb */
    unsigned long vm_data;    /* data size */
    unsigned long vm_stack;   /* stack size */
    unsigned long vm_exe;     /* executable size */
    unsigned long vm_lib;     /* library size */
};

static bool osl_getProcStat(pid_t pid, struct osl_procStat* procstat)
{
    int fd = 0;
    bool bRet = false;
    char name[PATH_MAX + 1];
    snprintf(name, sizeof(name), "/proc/%u/stat", pid);

    if ((fd = open(name,O_RDONLY)) >=0 )
    {
        char* tmp=nullptr;
        char prstatbuf[512];
        memset(prstatbuf,0,512);
        bRet = safeRead(fd, prstatbuf, 511);

        close(fd);

        if (!bRet)
            return false;

        tmp = strrchr(prstatbuf, ')');
        if(tmp)
        {
            *tmp = '\0';

            memset(procstat->command, 0, sizeof(procstat->command));

            sscanf(prstatbuf, "%d (%15c", &procstat->pid, procstat->command);
            sscanf(tmp + 2,
               "%c"
               "%i %i %i %i %i"
               "%lu %lu %lu %lu %lu"
               "%lu %lu %lu %lu"
               "%lu %li %li %li"
               "%lu %lu %li %lu"
               "%lu %lu %lu %lu %lu"
               "%23s %23s %23s %23s"
               "%lu %lu %lu",
               &procstat->state,
               &procstat->ppid,      &procstat->pgrp,    &procstat->session,    &procstat->tty,         &procstat->tpgid,
               &procstat->flags,     &procstat->minflt,  &procstat->cminflt,    &procstat->majflt,      &procstat->cmajflt,
               &procstat->utime,     &procstat->stime,   &procstat->cutime,     &procstat->cstime,
               &procstat->priority,  &procstat->nice,    &procstat->timeout,    &procstat->itrealvalue,
               &procstat->starttime, &procstat->vsize,   &procstat->rss,        &procstat->rss_rlim,
               &procstat->startcode, &procstat->endcode, &procstat->startstack, &procstat->kstkesp,     &procstat->kstkeip,
               procstat->signal,     procstat->blocked,  procstat->sigignore,   procstat->sigcatch,
               &procstat->wchan,     &procstat->nswap,   &procstat->cnswap
            );
        }
        else
        {
            bRet = false;
        }
    }
    return bRet;
}

static bool osl_getProcStatus(pid_t pid, struct osl_procStat* procstat)
{
    int fd = 0;
    char name[PATH_MAX + 1];
    bool bRet = false;

    snprintf(name, sizeof(name), "/proc/%u/status", pid);

    if ((fd = open(name,O_RDONLY)) >=0 )
    {
        char* tmp=nullptr;
        char prstatusbuf[512];
        memset(prstatusbuf,0,512);
        bRet = safeRead(fd, prstatusbuf, 511);

        close(fd);

        if (!bRet)
            return false;

        tmp = strstr(prstatusbuf,"Uid:");
        if(tmp)
        {
            sscanf(tmp,"Uid:\t%d\t%d\t%d\t%d",
                   &procstat->ruid, &procstat->euid, &procstat->suid, &procstat->fuid
                );
        }

        tmp = strstr(prstatusbuf,"Gid:");
        if(tmp)
        {
            sscanf(tmp,"Gid:\t%d\t%d\t%d\t%d",
                   &procstat->rgid, &procstat->egid, &procstat->sgid, &procstat->fgid
                );
        }

        tmp = strstr(prstatusbuf,"VmSize:");
        if(tmp)
        {
            sscanf(tmp,
                   "VmSize: %lu kB\n"
                   "VmLck: %lu kB\n"
                   "VmRSS: %lu kB\n"
                   "VmData: %lu kB\n"
                   "VmStk: %lu kB\n"
                   "VmExe: %lu kB\n"
                   "VmLib: %lu kB\n",
                   &procstat->vm_size, &procstat->vm_lock, &procstat->vm_rss, &procstat->vm_data,
                   &procstat->vm_stack, &procstat->vm_exe, &procstat->vm_lib
                );
        }

        tmp = strstr(prstatusbuf,"SigPnd:");
        if(tmp)
        {
            sscanf(tmp, "SigPnd: %23s SigBlk: %23s SigIgn: %23s %*s %23s",
                   procstat->signal, procstat->blocked, procstat->sigignore, procstat->sigcatch
                );
        }
    }
    return bRet;
}

#endif

oslProcessError SAL_CALL osl_getProcessInfo(oslProcess Process, oslProcessData Fields, oslProcessInfo* pInfo)
{
    pid_t   pid;

    if (Process == nullptr)
        pid = getpid();
    else
        pid = static_cast<oslProcessImpl*>(Process)->m_pid;

    if (! pInfo || (pInfo->Size != sizeof(oslProcessInfo)))
        return osl_Process_E_Unknown;

    pInfo->Fields = 0;

    if (Fields & osl_Process_IDENTIFIER)
    {
        pInfo->Ident  = pid;
        pInfo->Fields |= osl_Process_IDENTIFIER;
    }

    if (Fields & osl_Process_EXITCODE)
    {
        if ((Process != nullptr) &&
            osl_checkCondition(static_cast<oslProcessImpl*>(Process)->m_terminated))
        {
            pInfo->Code = static_cast<oslProcessImpl*>(Process)->m_status;
            pInfo->Fields |= osl_Process_EXITCODE;
        }
    }

    if (Fields & (osl_Process_HEAPUSAGE | osl_Process_CPUTIMES))
    {

#if defined(__sun)

        int  fd;
        sal_Char name[PATH_MAX + 1];

        snprintf(name, sizeof(name), "/proc/%ld", (long)pid);

        if ((fd = open(name, O_RDONLY)) >= 0)
        {
            prstatus_t prstatus;

            if (ioctl(fd, PIOCSTATUS, &prstatus) >= 0)
            {
                if (Fields & osl_Process_CPUTIMES)
                {
                    pInfo->UserTime.Seconds   = prstatus.pr_utime.tv_sec;
                    pInfo->UserTime.Nanosec   = prstatus.pr_utime.tv_nsec;
                    pInfo->SystemTime.Seconds = prstatus.pr_stime.tv_sec;
                    pInfo->SystemTime.Nanosec = prstatus.pr_stime.tv_nsec;

                    pInfo->Fields |= osl_Process_CPUTIMES;
                }

                if (Fields & osl_Process_HEAPUSAGE)
                {
                    pInfo->HeapUsage = prstatus.pr_brksize;

                    pInfo->Fields |= osl_Process_HEAPUSAGE;
                }

                close(fd);

                return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
            }
            else
                close(fd);
        }

#elif defined(LINUX)

        if ( (Fields & osl_Process_CPUTIMES) || (Fields & osl_Process_HEAPUSAGE) )
        {
            struct osl_procStat procstat;
            memset(&procstat,0,sizeof(procstat));

            if ( (Fields & osl_Process_CPUTIMES) && osl_getProcStat(pid, &procstat) )
            {
                /*
                 *  mfe:
                 *  We calculate only time of the process proper.
                 *  Threads are processes, we do not consider their time here!
                 *  (For this, cutime and cstime should be used, it seems not
                 *   to work in 2.0.36)
                 */

                long clktck;
                unsigned long hz;
                unsigned long userseconds;
                unsigned long systemseconds;

                clktck = sysconf(_SC_CLK_TCK);
                if (clktck < 0) {
                    return osl_Process_E_Unknown;
                }
                hz = static_cast<unsigned long>(clktck);

                userseconds = procstat.utime/hz;
                systemseconds = procstat.stime/hz;

                pInfo->UserTime.Seconds   = userseconds;
                pInfo->UserTime.Nanosec   = procstat.utime - (userseconds * hz);
                pInfo->SystemTime.Seconds = systemseconds;
                pInfo->SystemTime.Nanosec = procstat.stime - (systemseconds * hz);

                pInfo->Fields |= osl_Process_CPUTIMES;
            }

            if ( (Fields & osl_Process_HEAPUSAGE) && osl_getProcStatus(pid, &procstat) )
            {
                /*
                 *  mfe:
                 *  vm_data (found in status) shows the size of the data segment
                 *  it a rough approximation of the core heap size
                 */
                pInfo->HeapUsage = procstat.vm_data*1024;

                pInfo->Fields |= osl_Process_HEAPUSAGE;
            }
        }

        return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
#endif

    }

    return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
}

/** Helper function for osl_joinProcessWithTimeout
 */

static bool is_timeout(const struct timeval* tend)
{
    struct timeval tcurrent;
    gettimeofday(&tcurrent, nullptr);
    return (tcurrent.tv_sec >= tend->tv_sec);
}

/* kill(pid, 0) is useful for checking if a
 process is still alive, but remember that
 kill even returns 0 if the process is already
 a zombie. */

static bool is_process_dead(pid_t pid)
{
    return ((kill(pid, 0) == -1) && (ESRCH == errno));
}

oslProcessError SAL_CALL osl_joinProcessWithTimeout(oslProcess Process, const TimeValue* pTimeout)
{
    oslProcessImpl* pChild    = ChildList;
    oslProcessError osl_error = osl_Process_E_None;

    OSL_PRECOND(Process, "osl_joinProcess: Invalid parameter");
    OSL_ASSERT(ChildListMutex);

    if (Process == nullptr || ChildListMutex == nullptr)
        return osl_Process_E_Unknown;

    osl_acquireMutex(ChildListMutex);

    /* check if process is a child of ours */
    while (pChild != nullptr)
    {
        if (pChild == static_cast<oslProcessImpl*>(Process))
            break;

        pChild = pChild->m_pnext;
    }

    osl_releaseMutex(ChildListMutex);

    if (pChild != nullptr)
    {
        oslConditionResult cond_res = osl_waitCondition(pChild->m_terminated, pTimeout);

        if (cond_res == osl_cond_result_timeout)
            osl_error = osl_Process_E_TimedOut;
        else if (cond_res != osl_cond_result_ok)
            osl_error = osl_Process_E_Unknown;
    }
    else /* alien process; StatusThread will not be able
               to set the condition terminated */
    {
        pid_t pid = static_cast<oslProcessImpl*>(Process)->m_pid;

        if (pTimeout)
        {
            bool timeout = false;
            struct timeval tend;

            gettimeofday(&tend, nullptr);

            tend.tv_sec += pTimeout->Seconds;

            while (!is_process_dead(pid) && !(timeout = is_timeout(&tend)))
                sleep(1);

            if (timeout)
                osl_error = osl_Process_E_TimedOut;
        }
        else /* infinite */
        {
            while (!is_process_dead(pid))
                sleep(1);
        }
    }
    return osl_error;
}

oslProcessError SAL_CALL osl_joinProcess(oslProcess Process)
{
    return osl_joinProcessWithTimeout(Process, nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
