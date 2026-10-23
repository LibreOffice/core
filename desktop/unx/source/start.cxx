/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_java.h>

#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <comphelper/hash.hxx>
#include <desktop/exithelper.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.h>
#include <rtl/process.h>
#include <rtl/ustrbuf.h>
#include <rtl/ustring.hxx>
#include <sal/main.h>

#include "args.hxx"
#include "pagein.hxx"
#include "splashx.h"

#define PIPEDEFAULTPATH      "/tmp"
#define PIPEALTERNATEPATH    "/var/tmp"

/* Easier conversions: char * to rtl_uString */
static rtl_uString *charp_to_ustr(const char *pStr)
{
    rtl_uString *pOut = nullptr;

    rtl_string2UString(&pOut, pStr, strlen(pStr), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);

    return pOut;
}

namespace {

class ChildInfo
{
public:
    int status_fd;
    oslProcess child;

    ChildInfo() = default;
    ChildInfo(ChildInfo&&) = default;
    ChildInfo(const ChildInfo&) = delete;
    ChildInfo& operator=(const ChildInfo&) = delete;
    ~ChildInfo();
};

ChildInfo::~ChildInfo()
{
    close(status_fd);
    osl_freeProcessHandle(child);
}
} // namespace

static int
child_info_get_status_fd(const ChildInfo& rInfo)
{
    return rInfo.status_fd;
}

static ChildInfo child_spawn(Args* args, bool bAllArgs, bool bWithStatus)
{
    rtl_uString* pTmp = nullptr;
    int status_pipe[2];

    /* create pipe */
    if (pipe(status_pipe) < 0)
    {
        fprintf(stderr, "ERROR: no file handles\n");
        exit(1);
    }
    ChildInfo info;
    info.status_fd = status_pipe[0];

    /* application name */
    const OUString sApp = u"file://" + args->sAppPath + u"soffice.bin";

    /* copy args */
    sal_uInt32 nArgs = bAllArgs ? args->aArgs.size() : args->nArgsEnv;
    std::vector<rtl_uString*> aArguments;
    for (sal_uInt32 i = 0; i < nArgs; ++i)
        aArguments.push_back(args->aArgs.at(i));

    if(bWithStatus)
    {
        char buffer[64];

        /* add the pipe arg */
        snprintf(buffer, 63, "--splash-pipe=%d", status_pipe[1]);
        rtl_uString_newFromAscii( &pTmp, buffer );
        aArguments.push_back(pTmp);
    }

    /* start the main process */
    oslProcessError nError
        = osl_executeProcess(sApp.pData, aArguments.data(), aArguments.size(), osl_Process_NORMAL,
                             nullptr, nullptr, nullptr, 0, &info.child);

    if (pTmp)
        rtl_uString_release(pTmp);

    if (nError != osl_Process_E_None)
    {
        fprintf(stderr, "ERROR %d forking process\n", nError);
        _exit (1);
    }

    close( status_pipe[1] );

    return info;
}

static bool child_exited_wait(ChildInfo& rInfo, bool bShortWait)
{
    TimeValue t = { 0, 250 /* ms */ * 1000 * 1000 };
    if (!bShortWait)
        t.Seconds = 1024;

    return osl_joinProcessWithTimeout(rInfo.child, &t) != osl_Process_E_TimedOut;
}

static int child_get_exit_code(ChildInfo& rInfo)
{
    oslProcessInfo inf;

    inf.Code = -1;
    inf.Size = sizeof(inf);

    if (osl_getProcessInfo(rInfo.child, osl_Process_EXITCODE, &inf) != osl_Process_E_None)
    {
        fprintf(stderr, "Warning: failed to fetch libreoffice exit status\n");
        return -1;
    }

    return inf.Code;
}

namespace {

typedef enum { ProgressContinue, ProgressRestart, ProgressExit } ProgressStatus;

} // namespace

/* Path of the application, with trailing slash. */
static OUString get_app_path(const char *pAppExec)
{
    char pRealPath[PATH_MAX];
    char* dummy;

    char *pOrigPath = strdup(pAppExec);
    char *pPath = dirname(pOrigPath);

    dummy = realpath(pPath, pRealPath);
    (void)dummy;
    OUString sResult(charp_to_ustr(pRealPath), SAL_NO_ACQUIRE);
    free(pOrigPath);

    const sal_Int32 len = sResult.getLength();
    if (len > 0 && sResult[len - 1] != u'/')
        sResult += u"/";

    return sResult;
}

/* Compute the OOo md5 hash from 'pText' */
static rtl_uString *get_md5hash(rtl_uString *pText)
{
    rtl_uString *pResult = nullptr;
    sal_Int32 nCapacity = 100;

    if ( !pText )
        return nullptr;

    unsigned char* pData = reinterpret_cast<unsigned char*>(rtl_uString_getStr(pText));
    sal_uInt32 nSize = rtl_uString_getLength(pText) * sizeof(sal_Unicode);
    if (!pData)
        return nullptr;

    const std::vector<unsigned char> md5_buf =
        comphelper::Hash::calculateHash(pData, nSize, comphelper::HashType::MD5);

    /* create hex-value string from the MD5 value to keep
       the string size minimal */
    rtl_uString_new_WithLength(&pResult, nCapacity);
    for (unsigned char b : md5_buf)
    {
        char val[3];
        snprintf(val, 3, "%x", b); /* sic! we ignore some of the 0's */

        rtl_uStringbuffer_insert_ascii(&pResult, &nCapacity, rtl_uString_getLength(pResult),
                val, strlen(val));
    }

    return pResult;
}

/* Construct the pipe name */
static OUString get_pipe_path(std::u16string_view sAppPath)
{
    rtl_uString* pUserInstallation = nullptr;
    rtl_uString *pBasePath = nullptr, *pAbsUserInstallation = nullptr;

    /* setup bootstrap filename */
    const OUString sPath = u"file://"_ustr + sAppPath + SAL_CONFIGFILE("bootstrap");

    /* read userinstallation value */
    rtlBootstrapHandle handle = rtl_bootstrap_args_open(sPath.pData);

    static constexpr OUString sUserInstallationName = u"UserInstallation"_ustr;
    rtl_bootstrap_get_from_handle(handle, sUserInstallationName.pData, &pUserInstallation, nullptr);

    rtl_bootstrap_args_close(handle);

    /* turn it into an absolute path - unwinding symlinks etc. */
    if (osl_getProcessWorkingDir(&pBasePath) ||
        osl_getAbsoluteFileURL(pBasePath, pUserInstallation, &pAbsUserInstallation))
        rtl_uString_newFromString(&pAbsUserInstallation, pUserInstallation);

    /* create the pipe name */
    rtl_uString* pMd5hash = get_md5hash(pAbsUserInstallation);
    if (!pMd5hash)
        rtl_uString_new(&pMd5hash);

    OUString sResult;
    if (access(PIPEDEFAULTPATH, W_OK) == 0)
    {
        sResult = PIPEDEFAULTPATH;
    }
    else if (access(PIPEALTERNATEPATH, W_OK) == 0)
    {
        sResult = PIPEALTERNATEPATH;
    }
    else
    {
        fprintf(stderr, "ERROR: no valid pipe path found.\n");
        exit(1);
    }

    const OUString sMd5hash(pMd5hash);
    sResult += u"/OSL_PIPE_" + OUString::number(static_cast<int>(getuid())) + u"_SingleOfficeIPC_" + sMd5hash;

    /* cleanup */
    rtl_uString_release(pMd5hash);

    if (pBasePath)
        rtl_uString_release(pBasePath);

    rtl_uString_release(pUserInstallation);
    rtl_uString_release(pAbsUserInstallation);

    return sResult;
}

/* Get fd of the pipe of the already running OOo. */
static int connect_pipe(const OUString& sPipePath)
{
    int fd;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return fd;

    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);

    addr.sun_family = AF_UNIX;
    const OString sPipeStr = rtl::OUStringToOString(sPipePath, osl_getThreadTextEncoding());
    strncpy(addr.sun_path, sPipeStr.getStr(), sizeof(addr.sun_path) - 1);

/* cut / paste from osl's pipe.c */
#if defined(FREEBSD)
    size_t len = SUN_LEN(&addr);
#else
    size_t len = sizeof(addr);
#endif

    if (connect(fd, reinterpret_cast<struct sockaddr *>(&addr), len) < 0)
    {
        close(fd);
        fd = -1;
    }
    return fd;
}

/* Escape: "," -> "\\,", "\0" -> "\\0", "\\" -> "\\\\" */
static rtl_uString *escape_path(rtl_uString const *pToEscape)
{
    rtl_uString *pBuffer = nullptr;
    sal_Int32 nCapacity = 1000;
    sal_Int32 nEscapeLength = rtl_uString_getLength(pToEscape);

    rtl_uString_new_WithLength(&pBuffer, nCapacity);

    for (sal_Int32 i = 0; i < nEscapeLength; ++i)
    {
        sal_Unicode c = pToEscape->buffer[i];
        switch (c)
        {
            case '\0':
                rtl_uStringbuffer_insert_ascii(&pBuffer, &nCapacity,
                        rtl_uString_getLength(pBuffer),
                        RTL_CONSTASCII_STRINGPARAM("\\0"));
                break;
            case ',':
                rtl_uStringbuffer_insert_ascii(&pBuffer, &nCapacity,
                        rtl_uString_getLength(pBuffer),
                        RTL_CONSTASCII_STRINGPARAM("\\,"));
                break;
            case '\\':
                rtl_uStringbuffer_insert_ascii(&pBuffer, &nCapacity,
                        rtl_uString_getLength(pBuffer),
                        RTL_CONSTASCII_STRINGPARAM("\\\\"));
                break;
            default:
                rtl_uStringbuffer_insert(&pBuffer, &nCapacity,
                        rtl_uString_getLength(pBuffer),
                        &c, 1);
        }
    }

    return pBuffer;
}

/* Send args to the LO instance (using the 'fd' file descriptor) */
static bool send_args(int fd, rtl_uString const *pCwdPath)
{
    rtl_uString *pBuffer = nullptr, *pTmp = nullptr;
    sal_Int32 nCapacity = 1000;
    rtl_String *pOut = nullptr;
    rtl_uString *pEscapedCwdPath = escape_path(pCwdPath);
    sal_uInt32 nArgCount = rtl_getAppCommandArgCount();

    rtl_uString_new_WithLength(&pBuffer, nCapacity);
    rtl_uString_new(&pTmp);

    rtl_uStringbuffer_insert_ascii(&pBuffer, &nCapacity,
            rtl_uString_getLength(pBuffer),
            RTL_CONSTASCII_STRINGPARAM("InternalIPC::Arguments"));

    if (rtl_uString_getLength(pEscapedCwdPath))
    {
        rtl_uStringbuffer_insert_ascii(&pBuffer, &nCapacity,
            rtl_uString_getLength(pBuffer),
            RTL_CONSTASCII_STRINGPARAM("1"));

        rtl_uStringbuffer_insert(&pBuffer, &nCapacity,
                    rtl_uString_getLength(pBuffer),
                    rtl_uString_getStr(pEscapedCwdPath),
                    rtl_uString_getLength(pEscapedCwdPath));
    }
    else
    {
        rtl_uStringbuffer_insert_ascii(&pBuffer, &nCapacity,
            rtl_uString_getLength(pBuffer),
            RTL_CONSTASCII_STRINGPARAM("0"));
    }

    for (sal_uInt32 nArg = 0; nArg < nArgCount; ++nArg)
    {
        rtl_uString *pEscapedTmp = nullptr;
        rtl_uStringbuffer_insert_ascii(&pBuffer, &nCapacity,
                rtl_uString_getLength(pBuffer),
                ",", 1);

        rtl_getAppCommandArg(nArg, &pTmp);

        pEscapedTmp = escape_path(pTmp);

        rtl_uStringbuffer_insert(&pBuffer, &nCapacity,
                rtl_uString_getLength(pBuffer),
                rtl_uString_getStr(pEscapedTmp),
                rtl_uString_getLength(pEscapedTmp));

        rtl_uString_release(pEscapedTmp);
    }

    if (!rtl_convertUStringToString(
             &pOut, rtl_uString_getStr(pBuffer),
             rtl_uString_getLength(pBuffer), RTL_TEXTENCODING_UTF8,
             (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
               | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        fprintf(stderr, "ERROR: cannot convert arguments to UTF-8\n");
        exit(1);
    }

    const size_t nLen = rtl_string_getLength(pOut) + 1;
    ssize_t n = write(fd, rtl_string_getStr(pOut), nLen);
    bool bResult = (n >= 0 && static_cast<size_t>(n) == nLen);

    if ( bResult )
    {
        char resp[SAL_N_ELEMENTS("InternalIPC::ProcessingDone")];
        n = read(fd, resp, SAL_N_ELEMENTS(resp));
        bResult = n == SAL_N_ELEMENTS(resp)
            && (memcmp(
                    resp, "InternalIPC::ProcessingDone",
                    SAL_N_ELEMENTS(resp))
                == 0);
    }

    /* cleanup */
    rtl_uString_release(pEscapedCwdPath);
    rtl_uString_release(pBuffer);
    rtl_uString_release(pTmp);
    rtl_string_release(pOut);

    return bResult;
}


#define BUFFER_LEN 255

/* Read the percent to show in splash. */
static ProgressStatus read_percent(const ChildInfo& rInfo, int *pPercent)
{
    static char pBuffer[BUFFER_LEN + 1];
    static char *pNext = pBuffer;
    static ssize_t nRead = 0;

    char c;

    /* from the last call */
    int nNotProcessed = nRead - (pNext - pBuffer);
    if (nNotProcessed >= BUFFER_LEN)
        return ProgressContinue;

    memmove(pBuffer, pNext, nNotProcessed);

    /* read data */
    ssize_t nThisRead = read(child_info_get_status_fd(rInfo),
                             pBuffer + nNotProcessed, BUFFER_LEN - nNotProcessed);

    if (nThisRead < 0)
    {
        if (errno == EINTR)
            return ProgressContinue;

        return ProgressExit;
    }

    nRead = nThisRead + nNotProcessed;
    pBuffer[nRead] = '\0';

    /* skip old data */
    char* pBegin = pBuffer;
    pNext = pBuffer;
    for (char* pIter = pBuffer; *pIter; ++pIter)
    {
        if (*pIter == '\n')
        {
            pBegin = pNext;
            pNext = pIter + 1;
        }
    }

    if (!strncasecmp(pBegin, "end", 3))
        return ProgressExit;
    else if (!strncasecmp(pBegin, "restart", 7))
        return ProgressRestart;
    else if (sscanf(pBegin, "%d%c", pPercent, &c) == 2 && c == '%')
        return ProgressContinue;

    /* unexpected - let's exit the splash to be safe */
    return ProgressExit;
}

/* Simple system check. */
static void system_checks(void)
{
#ifdef LINUX
    struct stat buf;

    /* check proc is mounted - lots of things fail otherwise */
    if (stat("/proc/version", &buf) != 0)
    {
        fprintf(stderr, "ERROR: /proc not mounted - LibreOffice is unlikely to work well if at all\n");
        exit(1);
    }
#endif
}

static void exec_pagein (Args *args)
{
    const OString sPath = rtl::OUStringToOString(args->sAppPath, osl_getThreadTextEncoding());
    pagein_execute(sPath.getStr(), "pagein-common");

    if (args->pPageinType)
        pagein_execute(sPath.getStr(), args->pPageinType);
}

#if HAVE_FEATURE_JAVA

static void extend_library_path(const char *new_element)
{
    static constexpr OUString sEnvName = u"LD_LIBRARY_PATH"_ustr;
    OUString sNewEnvVar = OUString::fromUtf8(new_element);

    rtl_uString* pOrigEnvVar = nullptr;
    osl_getEnvironment(sEnvName.pData, &pOrigEnvVar);
    if (pOrigEnvVar && pOrigEnvVar->length)
    {
        const OUString sOrigEnvVar(pOrigEnvVar);
        sNewEnvVar += u":" + sOrigEnvVar;
    }

    osl_setEnvironment(sEnvName.pData, sNewEnvVar.pData);

    if (pOrigEnvVar)
        rtl_uString_release(pOrigEnvVar);
}

static void exec_javaldx(Args *args)
{
    char newpath[4096];

    oslProcess javaldx = nullptr;
    oslFileHandle fileOut = nullptr;
    oslProcessError err;

    std::vector<rtl_uString*> aArguments;
    for (sal_uInt32 nArgs = 0; nArgs < args->nArgsEnv; ++nArgs)
        aArguments.push_back(args->aArgs.at(nArgs));

    /* Use absolute path to redirectrc */
    const OUString sRedirectRcArg
        = u"-env:INIFILENAME=vnd.sun.star.pathname:" + args->sAppPath + u"redirectrc";
    aArguments.push_back(sRedirectRcArg.pData);

    /* And also to javaldx */
    const OUString sApp = u"file://" + args->sAppPath + u"javaldx";
    err = osl_executeProcess_WithRedirectedIO(sApp.pData, aArguments.data(), aArguments.size(),
                                              osl_Process_NORMAL,
                                              nullptr, // security
                                              nullptr, // work dir
                                              nullptr, 0,
                                              &javaldx, // process handle
                                              nullptr,
                                              &fileOut,
                                              nullptr);

    if(err != osl_Process_E_None)
    {
        putenv(const_cast<char*>("javaldx_warn=failed to launch javaldx - java may not function correctly"));

        if (javaldx)
            osl_freeProcessHandle(javaldx);
        if (fileOut)
            osl_closeFile(fileOut);
        return;
    }
    else
    {
        char *chomp;
        sal_uInt64 bytes_read;

        /* Magically osl_readLine doesn't work with pipes with E_SPIPE - so be this lame instead: */
        while (osl_readFile (fileOut, newpath, SAL_N_ELEMENTS (newpath), &bytes_read) == osl_File_E_INTR);

        if (bytes_read <= 0)
        {
            putenv(const_cast<char*>("javaldx_warn=failed to read path from javaldx"));

            if (javaldx)
                osl_freeProcessHandle(javaldx);

            if (fileOut)
                osl_closeFile(fileOut);

            return;
        }

        newpath[bytes_read] = '\0';

        if ((chomp = strstr (newpath, "\n")))
            *chomp = '\0';
    }

    if (newpath[0] != '\0') {
        extend_library_path(newpath);
    }

    if (javaldx)
        osl_freeProcessHandle(javaldx);

    if (fileOut)
        osl_closeFile(fileOut);
}

#endif

// has to be a global :(
static oslProcess volatile g_pProcess = nullptr;

static void sigterm_handler(int /*ignored*/)
{
    if (g_pProcess) {
        bool SigTermSucceded = false;
        oslProcessInfo info;
        info.Size = sizeof(oslProcessInfo);

        // forward SIGTERM to soffice.bin and give it a chance to semi-gracefully exit
        // enough to remove named pipe
        if (osl_getProcessInfo(g_pProcess, osl_Process_IDENTIFIER, &info) == osl_Process_E_None) {
            TimeValue delay = { 1, 0 }; // 1 sec
            SigTermSucceded = kill(info.Ident, SIGTERM) == 0 &&
                              osl_joinProcessWithTimeout(g_pProcess, &delay) == osl_Process_E_None;
        }

        // didn't work, SIGKILL instead
        if (!SigTermSucceded) {
            osl_terminateProcess(g_pProcess); // uses SIGKILL to terminate soffice.bin
            osl_joinProcess(g_pProcess);
        }
    }

    _exit(255);
}


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    bool bSentArgs = false;
    int status = 0;
    struct sigaction sigpipe_action;
    struct sigaction sigterm_action;

    /* turn SIGPIPE into an error */
    memset(&sigpipe_action, 0, sizeof(struct sigaction));
    sigpipe_action.sa_handler = SIG_IGN;
    sigemptyset(&sigpipe_action.sa_mask);
    sigaction(SIGPIPE, &sigpipe_action, nullptr);
    memset(&sigterm_action, 0, sizeof(struct sigaction));
    sigterm_action.sa_handler = &sigterm_handler;
    sigemptyset(&sigterm_action.sa_mask);
    sigaction(SIGTERM, &sigterm_action, nullptr);

    Args* args = args_parse();
    args->sAppPath = get_app_path(argv[0]);

#ifndef ENABLE_QUICKSTART_LIBPNG
    /* we can't load and render it anyway */
    args->bInhibitSplash = true;
#endif

    const char* pUsePlugin = getenv("SAL_USE_VCLPLUGIN");
    if (pUsePlugin && !strcmp(pUsePlugin, "svp"))
        args->bInhibitSplash = true;

    if (!args->bInhibitPipe && !getenv("LIBO_FLATPAK"))
    {
        int fd = 0;
        const OUString sPipePath = get_pipe_path(args->sAppPath);
        if ((fd = connect_pipe(sPipePath)) >= 0)
        {
            // Wait for answer
            char resp[27]; // strlen("InternalIPC::SendArguments") + 1
            ssize_t n = read(fd, resp, SAL_N_ELEMENTS(resp));
            if (n == static_cast<ssize_t>(SAL_N_ELEMENTS(resp)) &&
                (memcmp(resp, "InternalIPC::SendArguments",
                        SAL_N_ELEMENTS(resp) - 1) == 0))
            {
                rtl_uString *pCwdPath = nullptr;
                osl_getProcessWorkingDir(&pCwdPath);

                // Then send args
                bSentArgs = send_args(fd, pCwdPath);
            }

            close(fd);
        }
    }

    if (!bSentArgs)
    {
        /* we have to prepare for, and exec the binary */
        int nPercent = 0;
        bool bAllArgs = true;
        bool bRestart;

        /* sanity check pieces */
        system_checks();

        /* load splash image and create window */
        struct splash* splash = nullptr;
        if (!args->bInhibitSplash)
            splash = splash_create(args->sAppPath.pData, argc, argv);

        /* pagein */
        if (!args->bInhibitPagein)
            exec_pagein(args);

        /* javaldx */
#if HAVE_FEATURE_JAVA
        if (!args->bInhibitJavaLdx)
            exec_javaldx (args);
#endif

        do
        {
            bRestart = false;

            /* fast updates if we have somewhere to update it to */
            bool bShortWait = splash != nullptr;

            /* Periodically update the splash & the percent according
               to what status_fd says, poll quickly only while starting */
            ChildInfo info = child_spawn(args, bAllArgs, bShortWait);
            g_pProcess = info.child;

            while (!child_exited_wait(info, bShortWait))
            {
                ProgressStatus eResult;

                splash_draw_progress(splash, nPercent);
                eResult = read_percent(info, &nPercent);

                if (eResult != ProgressContinue)
                {
                    splash_destroy(splash);
                    splash = nullptr;
                    bShortWait = false;
                }
            }

            status = child_get_exit_code(info);
            g_pProcess = nullptr; // reset

            switch (status)
            {
                case EXITHELPER_CRASH_WITH_RESTART: // re-start with just -env: parameters
                    bRestart = true;
                    bAllArgs = false;
                    break;
                case EXITHELPER_NORMAL_RESTART: // re-start with all arguments
                    bRestart = true;
                    bAllArgs = true;
                    break;
                default:
                    break;
            }
        } while (bRestart);
    }

    args_free(args);

    return status;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
