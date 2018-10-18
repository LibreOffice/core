/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_UNIXERRORSTRING_HXX
#define INCLUDED_COMPHELPER_UNIXERRORSTRING_HXX

#include <errno.h>
#include <cstring>

#include <rtl/ustring.hxx>

namespace
{
// Return the symbolic name of an errno value, like "ENOENT".

// Rationale why to use this and not strerror(): This is intended to be used in SAL_INFO() and
// SAL_WARN(). Such messages are intended to be read by developers, not end-users. Developers are
// (or should be) familiar with symbolic errno names in code anyway. strerror() is localized and the
// localised error strings might be less familiar to a developer that happens to run a localised
// environment.

inline OUString UnixErrnoString(int nErrno)
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
        case ENOTBLK:
            return "ENOTBLK";
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
        case ENOSTR:
            return "ENOSTR";
        case ENODATA:
            return "ENODATA";
        case ETIME:
            return "ETIME";
        case ENOSR:
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
            return OUString::number(nErrno) + " ("
                   + OUString(str, strlen(str), RTL_TEXTENCODING_UTF8) + ")";
    }
}

} // anonymous namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
