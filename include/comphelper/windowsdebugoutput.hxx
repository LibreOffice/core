/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* Debug output operators for Windows-specific types. For use in SAL_INFO(), SAL_WARN(), and
 * friends. The exact format of the generated output is not guaranteed to be stable or contain
 * complete information.
 */

#ifndef INCLUDED_COMPHELPER_WINDOWSDEBUGOUTPUT_HXX
#define INCLUDED_COMPHELPER_WINDOWSDEBUGOUTPUT_HXX

#include <codecvt>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#ifdef LIBO_INTERNAL_ONLY
#include <prewin.h>
#include <postwin.h>
#else
#include <windows.h>
#endif
#include <initguid.h>

namespace
{
DEFINE_GUID(IID_IdentityUnmarshal, 0x0000001B, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x46);
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const IID& rIid)
{
    LPOLESTR pRiid;
    if (StringFromIID(rIid, &pRiid) != S_OK)
        return stream << "?";

    stream << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
        std::wstring(pRiid));

    DWORD nSize;
    if (RegGetValueW(HKEY_CLASSES_ROOT, std::wstring(L"CLSID\\").append(pRiid).data(), nullptr,
                     RRF_RT_REG_SZ, nullptr, nullptr, &nSize)
        == ERROR_SUCCESS)
    {
        std::vector<wchar_t> sValue(nSize / 2);
        if (RegGetValueW(HKEY_CLASSES_ROOT, std::wstring(L"CLSID\\").append(pRiid).data(), nullptr,
                         RRF_RT_REG_SZ, nullptr, sValue.data(), &nSize)
            == ERROR_SUCCESS)
        {
            stream << "=\""
                   << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
                          std::wstring(sValue.data()))
                   << "\"";
        }
    }
    else if (RegGetValueW(HKEY_CLASSES_ROOT, std::wstring(L"Interface\\").append(pRiid).data(),
                          nullptr, RRF_RT_REG_SZ, nullptr, nullptr, &nSize)
             == ERROR_SUCCESS)
    {
        std::vector<wchar_t> sValue(nSize / 2);
        if (RegGetValueW(HKEY_CLASSES_ROOT, std::wstring(L"Interface\\").append(pRiid).data(),
                         nullptr, RRF_RT_REG_SZ, nullptr, sValue.data(), &nSize)
            == ERROR_SUCCESS)
        {
            stream << "=\""
                   << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
                          std::wstring(sValue.data()))
                   << "\"";
        }
    }
    else
    {
        // Special case well-known interfaces that pop up a lot, but which don't have their name in
        // the Registry.

        if (IsEqualIID(rIid, IID_IMarshal))
            stream << "=\"IMarshal\"";
        else if (IsEqualIID(rIid, IID_IMarshal2))
            stream << "=\"IMarshal2\"";
        else if (IsEqualIID(rIid, IID_INoMarshal))
            stream << "=\"INoMarshal\"";
        else if (IsEqualIID(rIid, IID_IdentityUnmarshal))
            stream << "=\"IdentityUnmarshal\"";
        else if (IsEqualIID(rIid, IID_IFastRundown))
            stream << "=\"IFastRundown\"";
        else if (IsEqualIID(rIid, IID_IStdMarshalInfo))
            stream << "=\"IStdMarshalInfo\"";
        else if (IsEqualIID(rIid, IID_IAgileObject))
            stream << "=\"IAgileObject\"";
        else if (IsEqualIID(rIid, IID_IExternalConnection))
            stream << "=\"IExternalConnection\"";
        else if (IsEqualIID(rIid, IID_ICallFactory))
            stream << "=\"ICallFactory\"";
    }

    CoTaskMemFree(pRiid);
    return stream;
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const VARIANT& rVariant)
{
    if (rVariant.vt & VT_VECTOR)
        stream << "VECTOR:";
    if (rVariant.vt & VT_ARRAY)
        stream << "ARRAY:";
    if (rVariant.vt & VT_BYREF)
        stream << "BYREF:";

    switch (rVariant.vt & ~(VT_VECTOR | VT_ARRAY | VT_BYREF))
    {
        case VT_EMPTY:
            stream << "EMPTY";
            break;
        case VT_NULL:
            stream << "NULL";
            break;
        case VT_I2:
            stream << "I2";
            break;
        case VT_I4:
            stream << "I4";
            break;
        case VT_R4:
            stream << "R4";
            break;
        case VT_R8:
            stream << "R8";
            break;
        case VT_CY:
            stream << "CY";
            break;
        case VT_DATE:
            stream << "DATE";
            break;
        case VT_BSTR:
            stream << "BSTR";
            break;
        case VT_DISPATCH:
            stream << "DISPATCH";
            break;
        case VT_ERROR:
            stream << "ERROR";
            break;
        case VT_BOOL:
            stream << "BOOL";
            break;
        case VT_VARIANT:
            stream << "VARIANT";
            break;
        case VT_UNKNOWN:
            stream << "UNKNOWN";
            break;
        case VT_DECIMAL:
            stream << "DECIMAL";
            break;
        case VT_I1:
            stream << "I1";
            break;
        case VT_UI1:
            stream << "UI1";
            break;
        case VT_UI2:
            stream << "UI2";
            break;
        case VT_UI4:
            stream << "UI4";
            break;
        case VT_I8:
            stream << "I8";
            break;
        case VT_UI8:
            stream << "UI8";
            break;
        case VT_INT:
            stream << "INT";
            break;
        case VT_UINT:
            stream << "UINT";
            break;
        case VT_VOID:
            stream << "VOID";
            break;
        case VT_HRESULT:
            stream << "HRESULT";
            break;
        case VT_PTR:
            stream << "PTR";
            break;
        case VT_SAFEARRAY:
            stream << "SAFEARRAY";
            break;
        case VT_CARRAY:
            stream << "CARRAY";
            break;
        case VT_USERDEFINED:
            stream << "USERDEFINED";
            break;
        case VT_LPSTR:
            stream << "LPSTR";
            break;
        case VT_LPWSTR:
            stream << "LPWSTR";
            break;
        case VT_RECORD:
            stream << "RECORD";
            break;
        case VT_INT_PTR:
            stream << "INT_PTR";
            break;
        case VT_UINT_PTR:
            stream << "UINT_PTR";
            break;
        case VT_FILETIME:
            stream << "FILETIME";
            break;
        case VT_BLOB:
            stream << "BLOB";
            break;
        case VT_STREAM:
            stream << "STREAM";
            break;
        case VT_STORAGE:
            stream << "STORAGE";
            break;
        case VT_STREAMED_OBJECT:
            stream << "STREAMED_OBJECT";
            break;
        case VT_STORED_OBJECT:
            stream << "STORED_OBJECT";
            break;
        case VT_BLOB_OBJECT:
            stream << "BLOB_OBJECT";
            break;
        case VT_CF:
            stream << "CF";
            break;
        case VT_CLSID:
            stream << "CLSID";
            break;
        case VT_VERSIONED_STREAM:
            stream << "VERSIONED_STREAM";
            break;
        case VT_BSTR_BLOB:
            stream << "BSTR_BLOB";
            break;
        default:
            stream << rVariant.vt;
            break;
    }
    if (rVariant.vt == VT_EMPTY || rVariant.vt == VT_NULL || rVariant.vt == VT_VOID)
        return stream;
    stream << ":";

    std::ios_base::fmtflags flags;
    std::streamsize width;
    charT fill;

    if (rVariant.vt & VT_BYREF)
    {
        stream << rVariant.byref << ":";
        if (rVariant.byref == nullptr)
            return stream;
        if ((rVariant.vt & VT_TYPEMASK) == VT_VOID || (rVariant.vt & VT_TYPEMASK) == VT_USERDEFINED)
            return stream;
        stream << ":";
        switch (rVariant.vt & VT_TYPEMASK)
        {
            case VT_I2:
                stream << *static_cast<short*>(rVariant.byref);
                break;
            case VT_I4:
                stream << *static_cast<int*>(rVariant.byref);
                break;
            case VT_R4:
                stream << *static_cast<float*>(rVariant.byref);
                break;
            case VT_R8:
                stream << *static_cast<double*>(rVariant.byref);
                break;
            case VT_CY:
                stream << static_cast<CY*>(rVariant.byref)->int64;
                break;
            case VT_DATE:
                stream << *static_cast<double*>(rVariant.byref);
                break; // FIXME
            case VT_BSTR:
                stream << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
                    *static_cast<OLECHAR**>(rVariant.byref));
                break;
            case VT_DISPATCH:
                stream << rVariant.byref;
                break;
            case VT_ERROR:
            case VT_HRESULT:
                flags = stream.flags();
                stream << std::hex << *static_cast<int*>(rVariant.byref);
                stream.setf(flags);
                break;
            case VT_BOOL:
                stream << (*static_cast<VARIANT_BOOL*>(rVariant.byref) ? "YES" : "NO");
                break;
            case VT_VARIANT:
                stream << *static_cast<VARIANT*>(rVariant.byref);
                break;
            case VT_UNKNOWN:
                stream << *static_cast<IUnknown**>(rVariant.byref);
                break;
            case VT_DECIMAL:
                flags = stream.flags();
                width = stream.width();
                fill = stream.fill();
                stream << std::hex << std::setw(8) << std::setfill('0')
                       << static_cast<DECIMAL*>(rVariant.byref)->Hi32;
                stream << std::setw(16) << static_cast<DECIMAL*>(rVariant.byref)->Lo64;
                stream.setf(flags);
                stream << std::setw(width) << std::setfill(fill);
                break;
            case VT_I1:
                stream << static_cast<int>(*static_cast<char*>(rVariant.byref));
                break;
            case VT_UI1:
                stream << static_cast<unsigned int>(*static_cast<unsigned char*>(rVariant.byref));
                break;
            case VT_UI2:
                stream << *static_cast<unsigned short*>(rVariant.byref);
                break;
            case VT_UI4:
                stream << *static_cast<unsigned int*>(rVariant.byref);
                break;
            case VT_I8:
                stream << *static_cast<long long*>(rVariant.byref);
                break;
            case VT_UI8:
                stream << *static_cast<unsigned long long*>(rVariant.byref);
                break;
            case VT_INT:
                stream << *static_cast<int*>(rVariant.byref);
                break;
            case VT_UINT:
                stream << *static_cast<unsigned int*>(rVariant.byref);
                break;
            case VT_INT_PTR:
                stream << *static_cast<intptr_t*>(rVariant.byref);
                break;
            case VT_UINT_PTR:
                stream << *static_cast<uintptr_t*>(rVariant.byref);
                break;
            case VT_PTR:
            case VT_CARRAY:
                stream << *static_cast<void**>(rVariant.byref);
                break;
            case VT_SAFEARRAY:
                break; // FIXME
            case VT_LPSTR:
                stream << *static_cast<char**>(rVariant.byref);
                break;
            case VT_LPWSTR:
                stream << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
                    std::wstring(*static_cast<wchar_t**>(rVariant.byref)));
                break;
            case VT_FILETIME:
                break; // FIXME
            case VT_BLOB:
                break; // FIXME
            case VT_STREAM:
                break; // FIXME
            case VT_STORAGE:
                break; // FIXME
            case VT_STREAMED_OBJECT:
                break; // FIXME
            case VT_STORED_OBJECT:
                break; // FIXME
            case VT_BLOB_OBJECT:
                break; // FIXME
            case VT_CF:
                break; // FIXME
            case VT_CLSID:
                stream << *static_cast<IID*>(rVariant.byref);
                break;
            case VT_VERSIONED_STREAM:
                break; // FIXME
            case VT_BSTR_BLOB:
                break; // FIXME
            default:
                stream << "?(" << (rVariant.vt & VT_TYPEMASK) << ")";
                break;
        }
        return stream;
    }

    switch (rVariant.vt & VT_TYPEMASK)
    {
        case VT_I2:
            stream << rVariant.iVal;
            break;
        case VT_I4:
            stream << rVariant.lVal;
            break;
        case VT_R4:
            stream << rVariant.fltVal;
            break;
        case VT_R8:
            stream << rVariant.dblVal;
            break;
        case VT_CY:
            stream << rVariant.cyVal.int64;
            break;
        case VT_DATE:
            stream << static_cast<double>(rVariant.date);
            break; // FIXME
        case VT_BSTR:
            stream << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
                rVariant.bstrVal);
            break;
        case VT_DISPATCH:
            stream << rVariant.pdispVal;
            break;
        case VT_ERROR:
        case VT_HRESULT:
            flags = stream.flags();
            stream << std::hex << rVariant.lVal;
            stream.setf(flags);
            break;
        case VT_BOOL:
            stream << (rVariant.boolVal ? "YES" : "NO");
            break;
        case VT_UNKNOWN:
            stream << rVariant.punkVal;
            break;
        case VT_DECIMAL:
            flags = stream.flags();
            width = stream.width();
            fill = stream.fill();
            stream << std::hex << std::setw(8) << std::setfill('0') << rVariant.decVal.Hi32;
            stream << std::setw(16) << rVariant.decVal.Lo64;
            stream.setf(flags);
            stream << std::setw(width) << std::setfill(fill);
            break;
        case VT_I1:
            stream << static_cast<int>(rVariant.bVal);
            break;
        case VT_UI1:
            stream << static_cast<unsigned int>(rVariant.bVal);
            break;
        case VT_UI2:
            stream << static_cast<unsigned short>(rVariant.iVal);
            break;
        case VT_UI4:
            stream << static_cast<unsigned int>(rVariant.lVal);
            break;
        case VT_I8:
            stream << rVariant.llVal;
            break;
        case VT_UI8:
            stream << static_cast<unsigned long long>(rVariant.llVal);
            break;
        case VT_INT:
            stream << rVariant.lVal;
            break;
        case VT_UINT:
            stream << static_cast<unsigned int>(rVariant.lVal);
            break;
        case VT_INT_PTR:
            stream << reinterpret_cast<intptr_t>(rVariant.plVal);
            break;
        case VT_UINT_PTR:
            stream << reinterpret_cast<uintptr_t>(rVariant.plVal);
            break;
        case VT_PTR:
        case VT_CARRAY:
            stream << rVariant.byref;
            break;
        case VT_SAFEARRAY:
            break; // FIXME
        case VT_LPSTR:
            stream << rVariant.bstrVal;
            break;
        case VT_LPWSTR:
            stream << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
                std::wstring(static_cast<wchar_t*>(rVariant.byref)));
            break;
        case VT_FILETIME:
            break; // FIXME
        case VT_BLOB:
            break; // FIXME
        case VT_STREAM:
            break; // FIXME
        case VT_STORAGE:
            break; // FIXME
        case VT_STREAMED_OBJECT:
            break; // FIXME
        case VT_STORED_OBJECT:
            break; // FIXME
        case VT_BLOB_OBJECT:
            break; // FIXME
        case VT_CF:
            break; // FIXME
        case VT_VERSIONED_STREAM:
            break; // FIXME
        case VT_BSTR_BLOB:
            break; // FIXME
        default:
            stream << "?(" << (rVariant.vt & VT_TYPEMASK) << ")";
            break;
    }
    return stream;
}

#endif // INCLUDED_COMPHELPER_WINDOWSDEBUGOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
