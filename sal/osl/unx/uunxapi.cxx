/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <string_view>

#include <config_features.h>

#include "uunxapi.hxx"
#include "system.hxx"
#include "unixerrnostring.hxx"
#include <limits.h>
#include <rtl/ustring.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

OString osl::OUStringToOString(std::u16string_view s)
{
    return rtl::OUStringToOString(s, osl_getThreadTextEncoding());
}

#if HAVE_FEATURE_MACOSX_SANDBOX

#include <Foundation/Foundation.h>
#include <Security/Security.h>
#include <mach-o/dyld.h>

static NSUserDefaults *userDefaults = NULL;
static bool isSandboxed = false;

static void do_once()
{
    SecCodeRef code;
    OSStatus rc = SecCodeCopySelf(kSecCSDefaultFlags, &code);

    SecStaticCodeRef staticCode;
    if (rc == errSecSuccess)
        rc = SecCodeCopyStaticCode(code, kSecCSDefaultFlags, &staticCode);

    CFDictionaryRef signingInformation;
    if (rc == errSecSuccess)
        rc = SecCodeCopySigningInformation(staticCode, kSecCSRequirementInformation, &signingInformation);

    CFDictionaryRef entitlements = NULL;
    if (rc == errSecSuccess)
        entitlements = (CFDictionaryRef) CFDictionaryGetValue(signingInformation, kSecCodeInfoEntitlementsDict);

    if (entitlements != NULL)
        if (CFDictionaryGetValue(entitlements, CFSTR("com.apple.security.app-sandbox")) != NULL)
            isSandboxed = true;

    if (isSandboxed)
        userDefaults = [NSUserDefaults standardUserDefaults];
}

typedef struct {
    NSURL *scopeURL;
    NSAutoreleasePool *pool;
} accessFilePathState;

static accessFilePathState *
prepare_to_access_file_path( const char *cpFilePath )
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, &do_once);
    NSURL *fileURL = nil;
    NSData *data = nil;
    BOOL stale;
    accessFilePathState *state;

    if (!isSandboxed)
        return NULL;

    // If malloc() fails we are screwed anyway
    state = (accessFilePathState*) malloc(sizeof(accessFilePathState));

    state->pool = [[NSAutoreleasePool alloc] init];
    state->scopeURL = nil;

    if (userDefaults != nil)
        fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:cpFilePath]];

    if (fileURL != nil)
        data = [userDefaults dataForKey:[@"bookmarkFor:" stringByAppendingString:[fileURL absoluteString]]];

    if (data != nil)
        state->scopeURL = [NSURL URLByResolvingBookmarkData:data
                                                    options:NSURLBookmarkResolutionWithSecurityScope
                                              relativeToURL:nil
                                        bookmarkDataIsStale:&stale
                                                      error:nil];
    if (state->scopeURL != nil)
        [state->scopeURL startAccessingSecurityScopedResource];

    return state;
}

static void
done_accessing_file_path( const char * /*cpFilePath*/, accessFilePathState *state )
{
    if (!isSandboxed)
        return;

    int saved_errno = errno;

    if (state->scopeURL != nil)
        [state->scopeURL stopAccessingSecurityScopedResource];
    [state->pool release];
    free(state);

    errno = saved_errno;
}

#else

typedef void accessFilePathState;

#define prepare_to_access_file_path( cpFilePath ) nullptr

#define done_accessing_file_path( cpFilePath, state ) ((void) cpFilePath, (void) state)

#endif

#ifdef MACOSX
/*
 * Helper function for resolving Mac native alias files (not the same as unix alias files)
 * and to return the resolved alias as OString
 */
static OString macxp_resolveAliasAndConvert(OString const & p)
{
    char path[PATH_MAX];
    if (p.getLength() < PATH_MAX)
    {
        strcpy(path, p.getStr());
        macxp_resolveAlias(path, PATH_MAX);
        return path;
    }
    return p;
}
#endif /* MACOSX */

int osl::access(const OString& pstrPath, int mode)
{
    OString fn = pstrPath;
#ifdef ANDROID
    if (fn == "/assets" || fn.startsWith("/assets/"))
    {
        struct stat stat;
        if (lo_apk_lstat(fn.getStr(), &stat) == -1)
            return -1;
        if (mode & W_OK)
        {
            errno = EACCES;
            return -1;
        }
        return 0;
    }
#endif

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    int result = ::access(fn.getStr(), mode);
    int saved_errno = errno;
    if (result == -1)
        SAL_INFO("sal.file", "access(" << fn << ",0" << std::oct << mode << std::dec << "): " << UnixErrnoString(saved_errno));
    else
        SAL_INFO("sal.file", "access(" << fn << ",0" << std::oct << mode << std::dec << "): OK");

    done_accessing_file_path(fn.getStr(), state);

    errno = saved_errno;

    return result;
}

namespace {

OString toOString(OString const & s) { return s; }

OString toOString(std::u16string_view s) { return osl::OUStringToOString(s); }

template<typename T> T fromOString(OString const &) = delete;

template<> OString fromOString(OString const & s) { return s; }

template<> OUString fromOString(OString const & s)
{ return OStringToOUString(s, osl_getThreadTextEncoding()); }

template<typename T> bool realpath_(const T& pstrFileName, T& ppstrResolvedName)
{
    OString fn = toOString(pstrFileName);
#if defined ANDROID || defined(EMSCRIPTEN)
#if defined ANDROID
    if (fn == "/assets" || fn.startsWith("/assets/"))
#else
    if (fn == "/instdir" || fn.startsWith("/instdir/"))
#endif
    {
        if (osl::access(fn, F_OK) == -1)
            return false;

        ppstrResolvedName = pstrFileName;

        return true;
    }
#endif // ANDROID || EMSCRIPTEN

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    char  rp[PATH_MAX];
    bool  bRet = realpath(fn.getStr(), rp);
    int   saved_errno = errno;
    if (!bRet)
        SAL_INFO("sal.file", "realpath(" << fn << "): " << UnixErrnoString(saved_errno));
    else
        SAL_INFO("sal.file", "realpath(" << fn << "): OK");

    done_accessing_file_path(fn.getStr(), state);

    if (bRet)
    {
        ppstrResolvedName = fromOString<T>(OString(rp));
    }

    errno = saved_errno;

    return bRet;
}

}

bool osl::realpath(const OUString& pustrFileName, OUString& ppustrResolvedName)
{
    return realpath_(pustrFileName, ppustrResolvedName);
}

bool osl::realpath(const OString& pstrFileName, OString& ppstrResolvedName)
{
    return realpath_(pstrFileName, ppstrResolvedName);
}

int stat_c(const char* cpPath, struct stat* buf)
{
#ifdef ANDROID
    if (strncmp(cpPath, "/assets", sizeof("/assets")-1) == 0 &&
        (cpPath[sizeof("/assets")-1] == '\0' ||
         cpPath[sizeof("/assets")-1] == '/'))
        return lo_apk_lstat(cpPath, buf);
#endif

    accessFilePathState *state = prepare_to_access_file_path(cpPath);

    int result = stat(cpPath, buf);
    int saved_errno = errno;
    if (result == -1)
        SAL_INFO("sal.file", "stat(" << cpPath << "): " << UnixErrnoString(saved_errno));
    else
        SAL_INFO("sal.file", "stat(" << cpPath << "): OK");

    done_accessing_file_path(cpPath, state);

    errno = saved_errno;

    return result;
}

int lstat_c(const char* cpPath, struct stat* buf)
{
#ifdef ANDROID
    if (strncmp(cpPath, "/assets", sizeof("/assets")-1) == 0 &&
        (cpPath[sizeof("/assets")-1] == '\0' ||
         cpPath[sizeof("/assets")-1] == '/'))
        return lo_apk_lstat(cpPath, buf);
#endif

    accessFilePathState *state = prepare_to_access_file_path(cpPath);

    int result = lstat(cpPath, buf);
    int saved_errno = errno;
    if (result == -1)
        SAL_INFO("sal.file", "lstat(" << cpPath << "): " << UnixErrnoString(saved_errno));
    else
        SAL_INFO("sal.file", "lstat(" << cpPath << "): OK");

    done_accessing_file_path(cpPath, state);

    errno = saved_errno;

    return result;
}

namespace {

template<typename T> int lstat_(const T& pstrPath, struct stat& buf)
{
    OString fn = toOString(pstrPath);

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    return lstat_c(fn.getStr(), &buf);
}

}

int osl::lstat(const OUString& pustrPath, struct stat& buf)
{
    return lstat_(pustrPath, buf);
}

int osl::lstat(const OString& pstrPath, struct stat& buf)
{
    return lstat_(pstrPath, buf);
}

int osl::mkdir(const OString& path, mode_t mode)
{
    accessFilePathState *state = prepare_to_access_file_path(path.getStr());

    int result = ::mkdir(path.getStr(), mode);
    int saved_errno = errno;
    if (result == -1)
        SAL_INFO("sal.file", "mkdir(" << path << ",0" << std::oct << mode << std::dec << "): " << UnixErrnoString(saved_errno));
    else
        SAL_INFO("sal.file", "mkdir(" << path << ",0" << std::oct << mode << std::dec << "): OK");

    done_accessing_file_path(path.getStr(), state);

    errno = saved_errno;

    return result;
}

int open_c(const OString& path, int oflag, int mode)
{
    accessFilePathState *state = prepare_to_access_file_path(path.getStr());

    int result = open(path.getStr(), oflag, mode);
    int saved_errno = errno;
    if (result == -1)
        SAL_INFO("sal.file", "open(" << path << ",0" << std::oct << oflag << ",0" << mode << std::dec << "): " << UnixErrnoString(saved_errno));
    else
        SAL_INFO("sal.file", "open(" << path << ",0" << std::oct << oflag << ",0" << mode << std::dec << ") => " << result);

#if HAVE_FEATURE_MACOSX_SANDBOX
    if (isSandboxed && result != -1 && (oflag & O_CREAT) && (oflag & O_EXCL))
    {
        // A new file was created. Check if it is outside the sandbox.
        // (In that case it must be one the user selected as export or
        // save destination in a file dialog, otherwise we wouldn't
        // have been able to create it.) Create and store a security
        // scoped bookmark for it so that we can access the file in
        // the future, too. (For the "Recent Files" functionality.)
        const char *sandbox = [NSHomeDirectory() UTF8String];
        if (!(strncmp(sandbox, path.getStr(), strlen(sandbox)) == 0 &&
              path[strlen(sandbox)] == '/'))
        {
            auto cpPath = path.getStr();
            NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:cpPath]];
            NSData *data = [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
                         includingResourceValuesForKeys:nil
                                          relativeToURL:nil
                                                  error:nil];
            if (data != NULL)
            {
                [userDefaults setObject:data
                                 forKey:[@"bookmarkFor:" stringByAppendingString:[url absoluteString]]];
            }
        }
    }
#endif

    done_accessing_file_path(path.getStr(), state);

    errno = saved_errno;

    return result;
}

int utime_c(const char *cpPath, struct utimbuf *times)
{
    accessFilePathState *state = prepare_to_access_file_path(cpPath);

    int result = utime(cpPath, times);

    done_accessing_file_path(cpPath, state);

    return result;
}

int ftruncate_with_name(int fd, sal_uInt64 uSize, const OString& path)
{
    /* When sandboxed on macOS, ftruncate(), even if it takes an
     * already open file descriptor which was returned from an open()
     * call already checked by the sandbox, still requires a security
     * scope bookmark for the file to be active in case the file is
     * one that the sandbox doesn't otherwise allow access to. Luckily
     * LibreOffice usually calls ftruncate() through the helpful C++
     * abstraction layer that keeps the pathname around.
     */

    OString fn(path);

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    int result = ftruncate(fd, uSize);
    int saved_errno = errno;
    if (result < 0)
        SAL_INFO("sal.file", "ftruncate(" << fd << "," << uSize << "): " << UnixErrnoString(saved_errno));
    else
        SAL_INFO("sal.file", "ftruncate(" << fd << "," << uSize << "): OK");

    done_accessing_file_path(fn.getStr(), state);

    errno = saved_errno;

    return result;
}


std::string UnixErrnoString(int nErrno)
{
    // Errnos from <asm-generic/errno-base.h> and <asm-generic/errno.h> on Linux and <sys/errno.h>
    // on macOS.
    switch (nErrno)
    {
        case EPERM:
            return "EPERM";
        case ENOENT:
            return "ENOENT";
        case ESRCH:
            return "ESRCH";
        case EINTR:
            return "EINTR";
        case EIO:
            return "EIO";
        case ENXIO:
            return "ENXIO";
        case E2BIG:
            return "E2BIG";
        case ENOEXEC:
            return "ENOEXEC";
        case EBADF:
            return "EBADF";
        case ECHILD:
            return "ECHILD";
        case EAGAIN:
            return "EAGAIN";
        case ENOMEM:
            return "ENOMEM";
        case EACCES:
            return "EACCES";
        case EFAULT:
            return "EFAULT";
#ifdef ENOTBLK
        case ENOTBLK:
            return "ENOTBLK";
#endif
        case EBUSY:
            return "EBUSY";
        case EEXIST:
            return "EEXIST";
        case EXDEV:
            return "EXDEV";
        case ENODEV:
            return "ENODEV";
        case ENOTDIR:
            return "ENOTDIR";
        case EISDIR:
            return "EISDIR";
        case EINVAL:
            return "EINVAL";
        case ENFILE:
            return "ENFILE";
        case EMFILE:
            return "EMFILE";
        case ENOTTY:
            return "ENOTTY";
        case ETXTBSY:
            return "ETXTBSY";
        case EFBIG:
            return "EFBIG";
        case ENOSPC:
            return "ENOSPC";
        case ESPIPE:
            return "ESPIPE";
        case EROFS:
            return "EROFS";
        case EMLINK:
            return "EMLINK";
        case EPIPE:
            return "EPIPE";
        case EDOM:
            return "EDOM";
        case ERANGE:
            return "ERANGE";
        case EDEADLK:
            return "EDEADLK";
        case ENAMETOOLONG:
            return "ENAMETOOLONG";
        case ENOLCK:
            return "ENOLCK";
        case ENOSYS:
            return "ENOSYS";
        case ENOTEMPTY:
            return "ENOTEMPTY";
        case ELOOP:
            return "ELOOP";
        case ENOMSG:
            return "ENOMSG";
        case EIDRM:
            return "EIDRM";
#ifdef ECHRNG
        case ECHRNG:
            return "ECHRNG";
#endif
#ifdef EL2NSYNC
        case EL2NSYNC:
            return "EL2NSYNC";
#endif
#ifdef EL3HLT
        case EL3HLT:
            return "EL3HLT";
#endif
#ifdef EL3RST
        case EL3RST:
            return "EL3RST";
#endif
#ifdef ELNRNG
        case ELNRNG:
            return "ELNRNG";
#endif
#ifdef EUNATCH
        case EUNATCH:
            return "EUNATCH";
#endif
#ifdef ENOCSI
        case ENOCSI:
            return "ENOCSI";
#endif
#ifdef EL2HLT
        case EL2HLT:
            return "EL2HLT";
#endif
#ifdef EBADE
        case EBADE:
            return "EBADE";
#endif
#ifdef EBADR
        case EBADR:
            return "EBADR";
#endif
#ifdef EXFULL
        case EXFULL:
            return "EXFULL";
#endif
#ifdef ENOANO
        case ENOANO:
            return "ENOANO";
#endif
#ifdef EBADRQC
        case EBADRQC:
            return "EBADRQC";
#endif
#ifdef EBADSLT
        case EBADSLT:
            return "EBADSLT";
#endif
#ifdef EBFONT
        case EBFONT:
            return "EBFONT";
#endif
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-pragma"
#endif
#endif
        case ENOSTR:
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#endif
#endif
            return "ENOSTR";
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-pragma"
#endif
#endif
        case ENODATA:
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#endif
#endif
            return "ENODATA";
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-pragma"
#endif
#endif
        case ETIME:
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#endif
#endif
            return "ETIME";
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-pragma"
#endif
#endif
        case ENOSR:
#if defined __clang__
#if __has_warning("-Wdeprecated-pragma")
#pragma clang diagnostic push
#endif
#endif
            return "ENOSR";
#ifdef ENONET
        case ENONET:
            return "ENONET";
#endif
#ifdef ENOPKG
        case ENOPKG:
            return "ENOPKG";
#endif
#ifdef EREMOTE
        case EREMOTE:
            return "EREMOTE";
#endif
        case ENOLINK:
            return "ENOLINK";
#ifdef EADV
        case EADV:
            return "EADV";
#endif
#ifdef ESRMNT
        case ESRMNT:
            return "ESRMNT";
#endif
#ifdef ECOMM
        case ECOMM:
            return "ECOMM";
#endif
        case EPROTO:
            return "EPROTO";
        case EMULTIHOP:
            return "EMULTIHOP";
#ifdef EDOTDOT
        case EDOTDOT:
            return "EDOTDOT";
#endif
        case EBADMSG:
            return "EBADMSG";
        case EOVERFLOW:
            return "EOVERFLOW";
#ifdef ENOTUNIQ
        case ENOTUNIQ:
            return "ENOTUNIQ";
#endif
#ifdef EBADFD
        case EBADFD:
            return "EBADFD";
#endif
#ifdef EREMCHG
        case EREMCHG:
            return "EREMCHG";
#endif
#ifdef ELIBACC
        case ELIBACC:
            return "ELIBACC";
#endif
#ifdef ELIBBAD
        case ELIBBAD:
            return "ELIBBAD";
#endif
#ifdef ELIBSCN
        case ELIBSCN:
            return "ELIBSCN";
#endif
#ifdef ELIBMAX
        case ELIBMAX:
            return "ELIBMAX";
#endif
#ifdef ELIBEXEC
        case ELIBEXEC:
            return "ELIBEXEC";
#endif
        case EILSEQ:
            return "EILSEQ";
#ifdef ERESTART
        case ERESTART:
            return "ERESTART";
#endif
#ifdef ESTRPIPE
        case ESTRPIPE:
            return "ESTRPIPE";
#endif
#ifdef EUSERS
        case EUSERS:
            return "EUSERS";
#endif
        case ENOTSOCK:
            return "ENOTSOCK";
        case EDESTADDRREQ:
            return "EDESTADDRREQ";
        case EMSGSIZE:
            return "EMSGSIZE";
        case EPROTOTYPE:
            return "EPROTOTYPE";
        case ENOPROTOOPT:
            return "ENOPROTOOPT";
        case EPROTONOSUPPORT:
            return "EPROTONOSUPPORT";
#ifdef ESOCKTNOSUPPORT
        case ESOCKTNOSUPPORT:
            return "ESOCKTNOSUPPORT";
#endif
#ifdef EOPNOTSUPP
        case EOPNOTSUPP:
            return "EOPNOTSUPP";
#endif
        case EPFNOSUPPORT:
            return "EPFNOSUPPORT";
        case EAFNOSUPPORT:
            return "EAFNOSUPPORT";
        case EADDRINUSE:
            return "EADDRINUSE";
        case EADDRNOTAVAIL:
            return "EADDRNOTAVAIL";
        case ENETDOWN:
            return "ENETDOWN";
        case ENETUNREACH:
            return "ENETUNREACH";
        case ENETRESET:
            return "ENETRESET";
        case ECONNABORTED:
            return "ECONNABORTED";
        case ECONNRESET:
            return "ECONNRESET";
        case ENOBUFS:
            return "ENOBUFS";
        case EISCONN:
            return "EISCONN";
        case ENOTCONN:
            return "ENOTCONN";
#ifdef ESHUTDOWN
        case ESHUTDOWN:
            return "ESHUTDOWN";
#endif
#ifdef ETOOMANYREFS
        case ETOOMANYREFS:
            return "ETOOMANYREFS";
#endif
        case ETIMEDOUT:
            return "ETIMEDOUT";
        case ECONNREFUSED:
            return "ECONNREFUSED";
#ifdef EHOSTDOWN
        case EHOSTDOWN:
            return "EHOSTDOWN";
#endif
        case EHOSTUNREACH:
            return "EHOSTUNREACH";
        case EALREADY:
            return "EALREADY";
        case EINPROGRESS:
            return "EINPROGRESS";
        case ESTALE:
            return "ESTALE";
#ifdef EUCLEAN
        case EUCLEAN:
            return "EUCLEAN";
#endif
#ifdef ENOTNAM
        case ENOTNAM:
            return "ENOTNAM";
#endif
#ifdef ENAVAIL
        case ENAVAIL:
            return "ENAVAIL";
#endif
#ifdef EISNAM
        case EISNAM:
            return "EISNAM";
#endif
#ifdef EREMOTEIO
        case EREMOTEIO:
            return "EREMOTEIO";
#endif
        case EDQUOT:
            return "EDQUOT";
#ifdef ENOMEDIUM
        case ENOMEDIUM:
            return "ENOMEDIUM";
#endif
#ifdef EMEDIUMTYPE
        case EMEDIUMTYPE:
            return "EMEDIUMTYPE";
#endif
        case ECANCELED:
            return "ECANCELED";
#ifdef ENOKEY
        case ENOKEY:
            return "ENOKEY";
#endif
#ifdef EKEYEXPIRED
        case EKEYEXPIRED:
            return "EKEYEXPIRED";
#endif
#ifdef EKEYREVOKED
        case EKEYREVOKED:
            return "EKEYREVOKED";
#endif
#ifdef EKEYREJECTED
        case EKEYREJECTED:
            return "EKEYREJECTED";
#endif
#ifdef EOWNERDEAD
        case EOWNERDEAD:
            return "EOWNERDEAD";
#endif
#ifdef ENOTRECOVERABLE
        case ENOTRECOVERABLE:
            return "ENOTRECOVERABLE";
#endif
#ifdef ERFKILL
        case ERFKILL:
            return "ERFKILL";
#endif
#ifdef EHWPOISON
        case EHWPOISON:
            return "EHWPOISON";
#endif
#ifdef EPROCLIM
        case EPROCLIM:
            return "EPROCLIM";
#endif
#ifdef EBADRPC
        case EBADRPC:
            return "EBADRPC";
#endif
#ifdef ERPCMISMATCH
        case ERPCMISMATCH:
            return "ERPCMISMATCH";
#endif
#ifdef EPROGUNAVAIL
        case EPROGUNAVAIL:
            return "EPROGUNAVAIL";
#endif
#ifdef EPROGMISMATCH
        case EPROGMISMATCH:
            return "EPROGMISMATCH";
#endif
#ifdef EPROCUNAVAIL
        case EPROCUNAVAIL:
            return "EPROCUNAVAIL";
#endif
#ifdef EFTYPE
        case EFTYPE:
            return "EFTYPE";
#endif
#ifdef EAUTH
        case EAUTH:
            return "EAUTH";
#endif
#ifdef ENEEDAUTH
        case ENEEDAUTH:
            return "ENEEDAUTH";
#endif
#ifdef EPWROFF
        case EPWROFF:
            return "EPWROFF";
#endif
#ifdef EDEVERR
        case EDEVERR:
            return "EDEVERR";
#endif
#ifdef EBADEXEC
        case EBADEXEC:
            return "EBADEXEC";
#endif
#ifdef EBADARCH
        case EBADARCH:
            return "EBADARCH";
#endif
#ifdef ESHLIBVERS
        case ESHLIBVERS:
            return "ESHLIBVERS";
#endif
#ifdef EBADMACHO
        case EBADMACHO:
            return "EBADMACHO";
#endif
#ifdef ENOATTR
        case ENOATTR:
            return "ENOATTR";
#endif
#ifdef EQFULL
        case EQFULL:
            return "EQFULL";
#endif
        default:
            char* str = strerror(nErrno);
            return std::to_string(nErrno) + " (" + std::string(str) + ")";
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
