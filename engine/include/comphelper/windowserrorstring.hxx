/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_WINDOWSERRORSTRING_HXX
#define INCLUDED_COMPHELPER_WINDOWSERRORSTRING_HXX

#include <prewin.h>
#include <postwin.h>
#include <rtl/ustring.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

// The functions here are inline because sal uses them too, and sal cannot link to comphelper.

namespace comphelper
{
namespace detail
{
#define COMPHELPER_ERROR_NAME(code)                                                                \
    case code:                                                                                     \
        return #code

// The symbolic name of a Win32 error code, like "ERROR_ACCESS_DENIED", or nullptr for a code that
// is not listed. Windows has no call that gives the name. The list holds the codes that the calls
// whose failures we log are known to return: file access, processes and threads, windows and
// menus, the clipboard, sockets, and the general ones.
inline const char* Win32ErrorName(DWORD nErrorCode)
{
    switch (nErrorCode)
    {
        COMPHELPER_ERROR_NAME(ERROR_SUCCESS);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_FUNCTION);
        COMPHELPER_ERROR_NAME(ERROR_FILE_NOT_FOUND);
        COMPHELPER_ERROR_NAME(ERROR_PATH_NOT_FOUND);
        COMPHELPER_ERROR_NAME(ERROR_TOO_MANY_OPEN_FILES);
        COMPHELPER_ERROR_NAME(ERROR_ACCESS_DENIED);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_HANDLE);
        COMPHELPER_ERROR_NAME(ERROR_NOT_ENOUGH_MEMORY);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_DATA);
        COMPHELPER_ERROR_NAME(ERROR_OUTOFMEMORY);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_DRIVE);
        COMPHELPER_ERROR_NAME(ERROR_NO_MORE_FILES);
        COMPHELPER_ERROR_NAME(ERROR_WRITE_PROTECT);
        COMPHELPER_ERROR_NAME(ERROR_NOT_READY);
        COMPHELPER_ERROR_NAME(ERROR_BAD_LENGTH);
        COMPHELPER_ERROR_NAME(ERROR_SHARING_VIOLATION);
        COMPHELPER_ERROR_NAME(ERROR_LOCK_VIOLATION);
        COMPHELPER_ERROR_NAME(ERROR_HANDLE_EOF);
        COMPHELPER_ERROR_NAME(ERROR_NOT_SUPPORTED);
        COMPHELPER_ERROR_NAME(ERROR_BAD_NETPATH);
        COMPHELPER_ERROR_NAME(ERROR_NETWORK_ACCESS_DENIED);
        COMPHELPER_ERROR_NAME(ERROR_BAD_NET_NAME);
        COMPHELPER_ERROR_NAME(ERROR_FILE_EXISTS);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_PARAMETER);
        COMPHELPER_ERROR_NAME(ERROR_BROKEN_PIPE);
        COMPHELPER_ERROR_NAME(ERROR_DISK_FULL);
        COMPHELPER_ERROR_NAME(ERROR_CALL_NOT_IMPLEMENTED);
        COMPHELPER_ERROR_NAME(ERROR_INSUFFICIENT_BUFFER);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_NAME);
        COMPHELPER_ERROR_NAME(ERROR_MOD_NOT_FOUND);
        COMPHELPER_ERROR_NAME(ERROR_PROC_NOT_FOUND);
        COMPHELPER_ERROR_NAME(ERROR_ALREADY_EXISTS);
        COMPHELPER_ERROR_NAME(ERROR_BAD_EXE_FORMAT);
        COMPHELPER_ERROR_NAME(ERROR_FILENAME_EXCED_RANGE);
        COMPHELPER_ERROR_NAME(ERROR_MORE_DATA);
        COMPHELPER_ERROR_NAME(ERROR_NO_MORE_ITEMS);
        COMPHELPER_ERROR_NAME(ERROR_DIRECTORY);
        COMPHELPER_ERROR_NAME(ERROR_ELEVATION_REQUIRED);
        COMPHELPER_ERROR_NAME(ERROR_OPERATION_ABORTED);
        COMPHELPER_ERROR_NAME(ERROR_IO_PENDING);
        COMPHELPER_ERROR_NAME(ERROR_NOACCESS);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_FLAGS);
        COMPHELPER_ERROR_NAME(ERROR_NO_TOKEN);
        COMPHELPER_ERROR_NAME(ERROR_NOT_FOUND);
        COMPHELPER_ERROR_NAME(ERROR_LOGON_FAILURE);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_WINDOW_HANDLE);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_MENU_HANDLE);
        COMPHELPER_ERROR_NAME(ERROR_TLW_WITH_WSCHILD);
        COMPHELPER_ERROR_NAME(ERROR_CANNOT_FIND_WND_CLASS);
        COMPHELPER_ERROR_NAME(ERROR_CLASS_DOES_NOT_EXIST);
        COMPHELPER_ERROR_NAME(ERROR_CLIPBOARD_NOT_OPEN);
        COMPHELPER_ERROR_NAME(ERROR_NO_SYSTEM_RESOURCES);
        COMPHELPER_ERROR_NAME(ERROR_TIMEOUT);
        COMPHELPER_ERROR_NAME(ERROR_NOT_ENOUGH_QUOTA);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_PIXEL_FORMAT);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_WINDOW_STYLE);
        COMPHELPER_ERROR_NAME(ERROR_INVALID_STATE);
        COMPHELPER_ERROR_NAME(WSAEINTR);
        COMPHELPER_ERROR_NAME(WSAEBADF);
        COMPHELPER_ERROR_NAME(WSAEACCES);
        COMPHELPER_ERROR_NAME(WSAEFAULT);
        COMPHELPER_ERROR_NAME(WSAEINVAL);
        COMPHELPER_ERROR_NAME(WSAEMFILE);
        COMPHELPER_ERROR_NAME(WSAEWOULDBLOCK);
        COMPHELPER_ERROR_NAME(WSAEINPROGRESS);
        COMPHELPER_ERROR_NAME(WSAEALREADY);
        COMPHELPER_ERROR_NAME(WSAENOTSOCK);
        COMPHELPER_ERROR_NAME(WSAEPROTONOSUPPORT);
        COMPHELPER_ERROR_NAME(WSAESOCKTNOSUPPORT);
        COMPHELPER_ERROR_NAME(WSAEAFNOSUPPORT);
        COMPHELPER_ERROR_NAME(WSAEADDRINUSE);
        COMPHELPER_ERROR_NAME(WSAEADDRNOTAVAIL);
        COMPHELPER_ERROR_NAME(WSAENETDOWN);
        COMPHELPER_ERROR_NAME(WSAENETUNREACH);
        COMPHELPER_ERROR_NAME(WSAECONNABORTED);
        COMPHELPER_ERROR_NAME(WSAECONNRESET);
        COMPHELPER_ERROR_NAME(WSAENOBUFS);
        COMPHELPER_ERROR_NAME(WSAENOTCONN);
        COMPHELPER_ERROR_NAME(WSAETIMEDOUT);
        COMPHELPER_ERROR_NAME(WSAECONNREFUSED);
        COMPHELPER_ERROR_NAME(WSAEHOSTUNREACH);
        COMPHELPER_ERROR_NAME(WSASYSNOTREADY);
        COMPHELPER_ERROR_NAME(WSAVERNOTSUPPORTED);
        COMPHELPER_ERROR_NAME(WSANOTINITIALISED);
        COMPHELPER_ERROR_NAME(WSAEPROVIDERFAILEDINIT);
        default:
            return nullptr;
    }
}

// The symbolic name of an HRESULT, like "E_INVALIDARG", or nullptr for a value that is not listed.
// The list holds the general values, and those of COM and OLE, storage, cryptography and
// certificates, DirectWrite, Direct2D and DXGI. Some Win32 calls of cryptography return such a
// value from GetLastError() too.
inline const char* HresultName(HRESULT hr)
{
    switch (hr)
    {
        COMPHELPER_ERROR_NAME(S_OK);
        COMPHELPER_ERROR_NAME(S_FALSE);
        COMPHELPER_ERROR_NAME(E_NOTIMPL);
        COMPHELPER_ERROR_NAME(E_NOINTERFACE);
        COMPHELPER_ERROR_NAME(E_POINTER);
        COMPHELPER_ERROR_NAME(E_ABORT);
        COMPHELPER_ERROR_NAME(E_FAIL);
        COMPHELPER_ERROR_NAME(E_UNEXPECTED);
        COMPHELPER_ERROR_NAME(E_ACCESSDENIED);
        COMPHELPER_ERROR_NAME(E_HANDLE);
        COMPHELPER_ERROR_NAME(E_OUTOFMEMORY);
        COMPHELPER_ERROR_NAME(E_INVALIDARG);
        COMPHELPER_ERROR_NAME(E_PENDING);
        COMPHELPER_ERROR_NAME(E_BOUNDS);
        COMPHELPER_ERROR_NAME(CO_E_NOTINITIALIZED);
        COMPHELPER_ERROR_NAME(CO_E_ALREADYINITIALIZED);
        COMPHELPER_ERROR_NAME(CO_E_CLASSSTRING);
        COMPHELPER_ERROR_NAME(CO_E_APPNOTFOUND);
        COMPHELPER_ERROR_NAME(CO_E_OBJNOTCONNECTED);
        COMPHELPER_ERROR_NAME(CO_E_SERVER_EXEC_FAILURE);
        COMPHELPER_ERROR_NAME(RPC_E_CALL_REJECTED);
        COMPHELPER_ERROR_NAME(RPC_E_DISCONNECTED);
        COMPHELPER_ERROR_NAME(RPC_E_SERVERFAULT);
        COMPHELPER_ERROR_NAME(RPC_E_CHANGED_MODE);
        COMPHELPER_ERROR_NAME(RPC_E_SERVERCALL_RETRYLATER);
        COMPHELPER_ERROR_NAME(RPC_E_WRONG_THREAD);
        COMPHELPER_ERROR_NAME(REGDB_E_READREGDB);
        COMPHELPER_ERROR_NAME(REGDB_E_CLASSNOTREG);
        COMPHELPER_ERROR_NAME(CLASS_E_NOAGGREGATION);
        COMPHELPER_ERROR_NAME(CLASS_E_CLASSNOTAVAILABLE);
        COMPHELPER_ERROR_NAME(DISP_E_UNKNOWNNAME);
        COMPHELPER_ERROR_NAME(DISP_E_MEMBERNOTFOUND);
        COMPHELPER_ERROR_NAME(DISP_E_PARAMNOTFOUND);
        COMPHELPER_ERROR_NAME(DISP_E_TYPEMISMATCH);
        COMPHELPER_ERROR_NAME(DISP_E_EXCEPTION);
        COMPHELPER_ERROR_NAME(DISP_E_BADPARAMCOUNT);
        COMPHELPER_ERROR_NAME(TYPE_E_ELEMENTNOTFOUND);
        COMPHELPER_ERROR_NAME(OLE_E_NOCONNECTION);
        COMPHELPER_ERROR_NAME(OLE_E_NOTRUNNING);
        COMPHELPER_ERROR_NAME(OLE_E_PROMPTSAVECANCELLED);
        COMPHELPER_ERROR_NAME(OLE_E_BLANK);
        COMPHELPER_ERROR_NAME(OLE_E_CLASSDIFF);
        COMPHELPER_ERROR_NAME(OLE_E_CANT_BINDTOSOURCE);
        COMPHELPER_ERROR_NAME(OLE_E_STATIC);
        COMPHELPER_ERROR_NAME(OLEOBJ_E_NOVERBS);
        COMPHELPER_ERROR_NAME(OLEOBJ_E_INVALIDVERB);
        COMPHELPER_ERROR_NAME(DV_E_FORMATETC);
        COMPHELPER_ERROR_NAME(DV_E_TYMED);
        COMPHELPER_ERROR_NAME(DV_E_CLIPFORMAT);
        COMPHELPER_ERROR_NAME(CLIPBRD_E_CANT_OPEN);
        COMPHELPER_ERROR_NAME(DRAGDROP_E_NOTREGISTERED);
        COMPHELPER_ERROR_NAME(DRAGDROP_E_ALREADYREGISTERED);
        COMPHELPER_ERROR_NAME(DRAGDROP_E_INVALIDHWND);
        COMPHELPER_ERROR_NAME(STG_E_INVALIDFUNCTION);
        COMPHELPER_ERROR_NAME(STG_E_FILENOTFOUND);
        COMPHELPER_ERROR_NAME(STG_E_ACCESSDENIED);
        COMPHELPER_ERROR_NAME(STG_E_SHAREVIOLATION);
        COMPHELPER_ERROR_NAME(STG_E_FILEALREADYEXISTS);
        COMPHELPER_ERROR_NAME(STG_E_MEDIUMFULL);
        COMPHELPER_ERROR_NAME(STG_E_INVALIDHEADER);
        COMPHELPER_ERROR_NAME(STG_E_INVALIDNAME);
        COMPHELPER_ERROR_NAME(NTE_BAD_UID);
        COMPHELPER_ERROR_NAME(NTE_BAD_HASH);
        COMPHELPER_ERROR_NAME(NTE_BAD_KEY);
        COMPHELPER_ERROR_NAME(NTE_BAD_LEN);
        COMPHELPER_ERROR_NAME(NTE_BAD_DATA);
        COMPHELPER_ERROR_NAME(NTE_BAD_SIGNATURE);
        COMPHELPER_ERROR_NAME(NTE_BAD_ALGID);
        COMPHELPER_ERROR_NAME(NTE_BAD_FLAGS);
        COMPHELPER_ERROR_NAME(NTE_BAD_TYPE);
        COMPHELPER_ERROR_NAME(NTE_NO_KEY);
        COMPHELPER_ERROR_NAME(NTE_NO_MEMORY);
        COMPHELPER_ERROR_NAME(NTE_PERM);
        COMPHELPER_ERROR_NAME(NTE_NOT_FOUND);
        COMPHELPER_ERROR_NAME(NTE_BAD_KEYSET);
        COMPHELPER_ERROR_NAME(NTE_PROV_TYPE_NOT_DEF);
        COMPHELPER_ERROR_NAME(NTE_KEYSET_NOT_DEF);
        COMPHELPER_ERROR_NAME(NTE_SILENT_CONTEXT);
        COMPHELPER_ERROR_NAME(NTE_INVALID_PARAMETER);
        COMPHELPER_ERROR_NAME(NTE_BUFFER_TOO_SMALL);
        COMPHELPER_ERROR_NAME(NTE_NOT_SUPPORTED);
        COMPHELPER_ERROR_NAME(SEC_E_INVALID_HANDLE);
        COMPHELPER_ERROR_NAME(SEC_E_UNSUPPORTED_FUNCTION);
        COMPHELPER_ERROR_NAME(SEC_E_INTERNAL_ERROR);
        COMPHELPER_ERROR_NAME(SEC_E_NO_CREDENTIALS);
        COMPHELPER_ERROR_NAME(SEC_E_WRONG_PRINCIPAL);
        COMPHELPER_ERROR_NAME(SEC_E_UNTRUSTED_ROOT);
        COMPHELPER_ERROR_NAME(SEC_E_CERT_EXPIRED);
        COMPHELPER_ERROR_NAME(CRYPT_E_MSG_ERROR);
        COMPHELPER_ERROR_NAME(CRYPT_E_UNKNOWN_ALGO);
        COMPHELPER_ERROR_NAME(CRYPT_E_OID_FORMAT);
        COMPHELPER_ERROR_NAME(CRYPT_E_INVALID_MSG_TYPE);
        COMPHELPER_ERROR_NAME(CRYPT_E_UNEXPECTED_ENCODING);
        COMPHELPER_ERROR_NAME(CRYPT_E_AUTH_ATTR_MISSING);
        COMPHELPER_ERROR_NAME(CRYPT_E_HASH_VALUE);
        COMPHELPER_ERROR_NAME(CRYPT_E_INVALID_INDEX);
        COMPHELPER_ERROR_NAME(CRYPT_E_RECIPIENT_NOT_FOUND);
        COMPHELPER_ERROR_NAME(CRYPT_E_SIGNER_NOT_FOUND);
        COMPHELPER_ERROR_NAME(CRYPT_E_ATTRIBUTES_MISSING);
        COMPHELPER_ERROR_NAME(CRYPT_E_STREAM_MSG_NOT_READY);
        COMPHELPER_ERROR_NAME(CRYPT_E_STREAM_INSUFFICIENT_DATA);
        COMPHELPER_ERROR_NAME(CRYPT_E_BAD_LEN);
        COMPHELPER_ERROR_NAME(CRYPT_E_BAD_ENCODE);
        COMPHELPER_ERROR_NAME(CRYPT_E_FILE_ERROR);
        COMPHELPER_ERROR_NAME(CRYPT_E_NOT_FOUND);
        COMPHELPER_ERROR_NAME(CRYPT_E_EXISTS);
        COMPHELPER_ERROR_NAME(CRYPT_E_NO_PROVIDER);
        COMPHELPER_ERROR_NAME(CRYPT_E_SELF_SIGNED);
        COMPHELPER_ERROR_NAME(CRYPT_E_NO_MATCH);
        COMPHELPER_ERROR_NAME(CRYPT_E_UNEXPECTED_MSG_TYPE);
        COMPHELPER_ERROR_NAME(CRYPT_E_NO_KEY_PROPERTY);
        COMPHELPER_ERROR_NAME(CRYPT_E_NO_DECRYPT_CERT);
        COMPHELPER_ERROR_NAME(CRYPT_E_BAD_MSG);
        COMPHELPER_ERROR_NAME(CRYPT_E_NO_SIGNER);
        COMPHELPER_ERROR_NAME(CRYPT_E_REVOKED);
        COMPHELPER_ERROR_NAME(CRYPT_E_NO_REVOCATION_CHECK);
        COMPHELPER_ERROR_NAME(CRYPT_E_REVOCATION_OFFLINE);
        COMPHELPER_ERROR_NAME(CRYPT_E_ASN1_ERROR);
        COMPHELPER_ERROR_NAME(CRYPT_E_ASN1_CORRUPT);
        COMPHELPER_ERROR_NAME(CRYPT_E_ASN1_BADTAG);
        COMPHELPER_ERROR_NAME(TRUST_E_BAD_DIGEST);
        COMPHELPER_ERROR_NAME(TRUST_E_NOSIGNATURE);
        COMPHELPER_ERROR_NAME(CERT_E_EXPIRED);
        COMPHELPER_ERROR_NAME(CERT_E_UNTRUSTEDROOT);
        COMPHELPER_ERROR_NAME(CERT_E_CHAINING);
        COMPHELPER_ERROR_NAME(CERT_E_WRONG_USAGE);
        COMPHELPER_ERROR_NAME(DWRITE_E_FILEFORMAT);
        COMPHELPER_ERROR_NAME(DWRITE_E_UNEXPECTED);
        COMPHELPER_ERROR_NAME(DWRITE_E_NOFONT);
        COMPHELPER_ERROR_NAME(DWRITE_E_FILENOTFOUND);
        COMPHELPER_ERROR_NAME(DWRITE_E_FILEACCESS);
        COMPHELPER_ERROR_NAME(DWRITE_E_FONTCOLLECTIONOBSOLETE);
        COMPHELPER_ERROR_NAME(DWRITE_E_ALREADYREGISTERED);
        COMPHELPER_ERROR_NAME(D2DERR_WRONG_STATE);
        COMPHELPER_ERROR_NAME(D2DERR_NOT_INITIALIZED);
        COMPHELPER_ERROR_NAME(D2DERR_UNSUPPORTED_OPERATION);
        COMPHELPER_ERROR_NAME(D2DERR_SCANNER_FAILED);
        COMPHELPER_ERROR_NAME(D2DERR_DISPLAY_STATE_INVALID);
        COMPHELPER_ERROR_NAME(D2DERR_ZERO_VECTOR);
        COMPHELPER_ERROR_NAME(D2DERR_INTERNAL_ERROR);
        COMPHELPER_ERROR_NAME(D2DERR_DISPLAY_FORMAT_NOT_SUPPORTED);
        COMPHELPER_ERROR_NAME(D2DERR_INVALID_CALL);
        COMPHELPER_ERROR_NAME(D2DERR_NO_HARDWARE_DEVICE);
        COMPHELPER_ERROR_NAME(D2DERR_RECREATE_TARGET);
        COMPHELPER_ERROR_NAME(D2DERR_UNSUPPORTED_VERSION);
        COMPHELPER_ERROR_NAME(D2DERR_BAD_NUMBER);
        COMPHELPER_ERROR_NAME(DXGI_ERROR_INVALID_CALL);
        COMPHELPER_ERROR_NAME(DXGI_ERROR_UNSUPPORTED);
        COMPHELPER_ERROR_NAME(DXGI_ERROR_DEVICE_REMOVED);
        COMPHELPER_ERROR_NAME(DXGI_ERROR_DEVICE_HUNG);
        COMPHELPER_ERROR_NAME(DXGI_ERROR_DEVICE_RESET);
        default:
            return nullptr;
    }
}

#undef COMPHELPER_ERROR_NAME

// The text that the system has for an error code, in the language of the user, or an empty string
// when it has none.
inline OUString WindowsErrorText(DWORD nErrorCode)
{
    LPWSTR pMsgBuf;

    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr,
                       nErrorCode,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       reinterpret_cast<LPWSTR>(&pMsgBuf),
                       0,
                       nullptr) == 0)
        return OUString();

    OUString result(o3tl::toU(pMsgBuf));
    result.endsWith("\r\n", &result);

    HeapFree(GetProcessHeap(), 0, pMsgBuf);

    return result;
}

// An error code as a number: a Win32 error code in decimal, as the documentation lists them, and
// an HRESULT or any other large value in hex.
inline OUString WindowsErrorNumber(DWORD nErrorCode)
{
    if (nErrorCode <= 0xFFFF)
        return OUString::number(nErrorCode);
    return "0x" + OUString::number(nErrorCode, 16);
}

// An error code that has no name: its number, followed by the text of the system in parentheses
// when there is one.
inline OUString WindowsErrorNumberAndText(DWORD nErrorCode)
{
    OUString sText = WindowsErrorText(nErrorCode);
    if (sText.isEmpty())
        return WindowsErrorNumber(nErrorCode);
    return WindowsErrorNumber(nErrorCode) + " (" + sText + ")";
}

// The Win32 error code that an HRESULT holds, or the HRESULT itself when it holds none.
inline DWORD Win32ErrorCodeOfHRESULT(HRESULT hr)
{
    // See https://devblogs.microsoft.com/oldnewthing/20061103-07/?p=29133
    // Also https://social.msdn.microsoft.com/Forums/vstudio/en-US/c33d9a4a-1077-4efd-99e8-0c222743d2f8
    // (which refers to https://msdn.microsoft.com/en-us/library/aa382475)
    // explains why can't we just reinterpret_cast HRESULT to DWORD Win32 error:
    // we might actually have a Win32 error code converted using HRESULT_FROM_WIN32 macro

    DWORD nErrorCode = DWORD(hr);
    if (HRESULT(hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0) || hr == S_OK)
    {
        nErrorCode = HRESULT_CODE(hr);
        // https://msdn.microsoft.com/en-us/library/ms679360 mentions that the codes might have
        // high word bits set (e.g., bit 29 could be set if error comes from a 3rd-party library).
        // So try to restore the original error code to avoid wrong error messages
        DWORD nLastError = GetLastError();
        if ((nLastError & 0xFFFF) == nErrorCode)
            nErrorCode = nLastError;
    }
    return nErrorCode;
}

// The symbolic name of an HRESULT, or of the Win32 error code that it holds, or an empty string
// when neither has a known name.
inline OUString NameOfHRESULT(HRESULT hr)
{
    if (const char* pName = HresultName(hr))
        return OUString::createFromAscii(pName);

    const DWORD nErrorCode = Win32ErrorCodeOfHRESULT(hr);
    if (nErrorCode != DWORD(hr))
    {
        if (const char* pName = Win32ErrorName(nErrorCode))
            return "HRESULT_FROM_WIN32(" + OUString::createFromAscii(pName) + ")";
    }
    return OUString();
}
} // namespace detail

// An error code from GetLastError() or WSAGetLastError(), for a developer to read in a log: its
// symbolic name, like "ERROR_ACCESS_DENIED". A code without a known name is given as a number,
// with the text of the system for it.
inline OUString WindowsErrorString(DWORD nErrorCode)
{
    if (const char* pName = detail::Win32ErrorName(nErrorCode))
        return OUString::createFromAscii(pName);
    // Some calls of cryptography set an HRESULT as their last error.
    if (nErrorCode > 0xFFFF)
    {
        if (const char* pName = detail::HresultName(HRESULT(nErrorCode)))
            return OUString::createFromAscii(pName);
    }
    return detail::WindowsErrorNumberAndText(nErrorCode);
}

// An HRESULT, for a developer to read in a log: its symbolic name, like "E_INVALIDARG". An HRESULT
// that holds a Win32 error code is given as "HRESULT_FROM_WIN32(" and the name of that code. A
// value without a known name is given as a number, with the text of the system for it.
inline OUString WindowsErrorStringFromHRESULT(HRESULT hr)
{
    OUString sName = detail::NameOfHRESULT(hr);
    if (!sName.isEmpty())
        return sName;
    return detail::WindowsErrorNumberAndText(detail::Win32ErrorCodeOfHRESULT(hr));
}

// An HRESULT, for a user to read in a message: the text of the system for it, in the language of
// the user, followed by the symbolic name or the number in parentheses.
inline OUString WindowsErrorMessageFromHRESULT(HRESULT hr)
{
    const DWORD nErrorCode = detail::Win32ErrorCodeOfHRESULT(hr);
    OUString sName = detail::NameOfHRESULT(hr);
    if (sName.isEmpty())
        sName = detail::WindowsErrorNumber(nErrorCode);

    OUString sText = detail::WindowsErrorText(nErrorCode);
    if (sText.isEmpty())
        return sName;
    return sText + " (" + sName + ")";
}

} // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
