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
            if (rVariant.bstrVal == nullptr)
                stream << "(null)";
            else
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
            stream << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(
                rVariant.bstrVal);
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

inline std::string DMPAPER_to_string(int dmpaper)
{
    switch (dmpaper)
    {
        case DMPAPER_LETTER:
            return "LETTER";
        case DMPAPER_LETTERSMALL:
            return "LETTERSMALL";
        case DMPAPER_TABLOID:
            return "TABLOID";
        case DMPAPER_LEDGER:
            return "LEDGER";
        case DMPAPER_LEGAL:
            return "LEGAL";
        case DMPAPER_STATEMENT:
            return "STATEMENT";
        case DMPAPER_EXECUTIVE:
            return "EXECUTIVE";
        case DMPAPER_A3:
            return "A3";
        case DMPAPER_A4:
            return "A4";
        case DMPAPER_A4SMALL:
            return "A4SMALL";
        case DMPAPER_A5:
            return "A5";
        case DMPAPER_B4:
            return "B4";
        case DMPAPER_B5:
            return "B5";
        case DMPAPER_FOLIO:
            return "FOLIO";
        case DMPAPER_QUARTO:
            return "QUARTO";
        case DMPAPER_10X14:
            return "10X14";
        case DMPAPER_11X17:
            return "11X17";
        case DMPAPER_NOTE:
            return "NOTE";
        case DMPAPER_ENV_9:
            return "ENV_9";
        case DMPAPER_ENV_10:
            return "ENV_10";
        case DMPAPER_ENV_11:
            return "ENV_11";
        case DMPAPER_ENV_12:
            return "ENV_12";
        case DMPAPER_ENV_14:
            return "ENV_14";
        case DMPAPER_CSHEET:
            return "CSHEET";
        case DMPAPER_DSHEET:
            return "DSHEET";
        case DMPAPER_ESHEET:
            return "ESHEET";
        case DMPAPER_ENV_DL:
            return "ENV_DL";
        case DMPAPER_ENV_C5:
            return "ENV_C5";
        case DMPAPER_ENV_C3:
            return "ENV_C3";
        case DMPAPER_ENV_C4:
            return "ENV_C4";
        case DMPAPER_ENV_C6:
            return "ENV_C6";
        case DMPAPER_ENV_C65:
            return "ENV_C65";
        case DMPAPER_ENV_B4:
            return "ENV_B4";
        case DMPAPER_ENV_B5:
            return "ENV_B5";
        case DMPAPER_ENV_B6:
            return "ENV_B6";
        case DMPAPER_ENV_ITALY:
            return "ENV_ITALY";
        case DMPAPER_ENV_MONARCH:
            return "ENV_MONARCH";
        case DMPAPER_ENV_PERSONAL:
            return "ENV_PERSONAL";
        case DMPAPER_FANFOLD_US:
            return "FANFOLD_US";
        case DMPAPER_FANFOLD_STD_GERMAN:
            return "FANFOLD_STD_GERMAN";
        case DMPAPER_FANFOLD_LGL_GERMAN:
            return "FANFOLD_LGL_GERMAN";
        case DMPAPER_ISO_B4:
            return "ISO_B4";
        case DMPAPER_JAPANESE_POSTCARD:
            return "JAPANESE_POSTCARD";
        case DMPAPER_9X11:
            return "9X11";
        case DMPAPER_10X11:
            return "10X11";
        case DMPAPER_15X11:
            return "15X11";
        case DMPAPER_ENV_INVITE:
            return "ENV_INVITE";
        case DMPAPER_RESERVED_48:
            return "RESERVED_48";
        case DMPAPER_RESERVED_49:
            return "RESERVED_49";
        case DMPAPER_LETTER_EXTRA:
            return "LETTER_EXTRA";
        case DMPAPER_LEGAL_EXTRA:
            return "LEGAL_EXTRA";
        case DMPAPER_TABLOID_EXTRA:
            return "TABLOID_EXTRA";
        case DMPAPER_A4_EXTRA:
            return "A4_EXTRA";
        case DMPAPER_LETTER_TRANSVERSE:
            return "LETTER_TRANSVERSE";
        case DMPAPER_A4_TRANSVERSE:
            return "A4_TRANSVERSE";
        case DMPAPER_LETTER_EXTRA_TRANSVERSE:
            return "LETTER_EXTRA_TRANSVERSE";
        case DMPAPER_A_PLUS:
            return "A_PLUS";
        case DMPAPER_B_PLUS:
            return "B_PLUS";
        case DMPAPER_LETTER_PLUS:
            return "LETTER_PLUS";
        case DMPAPER_A4_PLUS:
            return "A4_PLUS";
        case DMPAPER_A5_TRANSVERSE:
            return "A5_TRANSVERSE";
        case DMPAPER_B5_TRANSVERSE:
            return "B5_TRANSVERSE";
        case DMPAPER_A3_EXTRA:
            return "A3_EXTRA";
        case DMPAPER_A5_EXTRA:
            return "A5_EXTRA";
        case DMPAPER_B5_EXTRA:
            return "B5_EXTRA";
        case DMPAPER_A2:
            return "A2";
        case DMPAPER_A3_TRANSVERSE:
            return "A3_TRANSVERSE";
        case DMPAPER_A3_EXTRA_TRANSVERSE:
            return "A3_EXTRA_TRANSVERSE";
        case DMPAPER_DBL_JAPANESE_POSTCARD:
            return "DBL_JAPANESE_POSTCARD";
        case DMPAPER_A6:
            return "A6";
        case DMPAPER_JENV_KAKU2:
            return "JENV_KAKU2";
        case DMPAPER_JENV_KAKU3:
            return "JENV_KAKU3";
        case DMPAPER_JENV_CHOU3:
            return "JENV_CHOU3";
        case DMPAPER_JENV_CHOU4:
            return "JENV_CHOU4";
        case DMPAPER_LETTER_ROTATED:
            return "LETTER_ROTATED";
        case DMPAPER_A3_ROTATED:
            return "A3_ROTATED";
        case DMPAPER_A4_ROTATED:
            return "A4_ROTATED";
        case DMPAPER_A5_ROTATED:
            return "A5_ROTATED";
        case DMPAPER_B4_JIS_ROTATED:
            return "B4_JIS_ROTATED";
        case DMPAPER_B5_JIS_ROTATED:
            return "B5_JIS_ROTATED";
        case DMPAPER_JAPANESE_POSTCARD_ROTATED:
            return "JAPANESE_POSTCARD_ROTATED";
        case DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED:
            return "DBL_JAPANESE_POSTCARD_ROTATED";
        case DMPAPER_A6_ROTATED:
            return "A6_ROTATED";
        case DMPAPER_JENV_KAKU2_ROTATED:
            return "JENV_KAKU2_ROTATED";
        case DMPAPER_JENV_KAKU3_ROTATED:
            return "JENV_KAKU3_ROTATED";
        case DMPAPER_JENV_CHOU3_ROTATED:
            return "JENV_CHOU3_ROTATED";
        case DMPAPER_JENV_CHOU4_ROTATED:
            return "JENV_CHOU4_ROTATED";
        case DMPAPER_B6_JIS:
            return "B6_JIS";
        case DMPAPER_B6_JIS_ROTATED:
            return "B6_JIS_ROTATED";
        case DMPAPER_12X11:
            return "12X11";
        case DMPAPER_JENV_YOU4:
            return "JENV_YOU4";
        case DMPAPER_JENV_YOU4_ROTATED:
            return "JENV_YOU4_ROTATED";
        case DMPAPER_P16K:
            return "P16K";
        case DMPAPER_P32K:
            return "P32K";
        case DMPAPER_P32KBIG:
            return "P32KBIG";
        case DMPAPER_PENV_1:
            return "PENV_1";
        case DMPAPER_PENV_2:
            return "PENV_2";
        case DMPAPER_PENV_3:
            return "PENV_3";
        case DMPAPER_PENV_4:
            return "PENV_4";
        case DMPAPER_PENV_5:
            return "PENV_5";
        case DMPAPER_PENV_6:
            return "PENV_6";
        case DMPAPER_PENV_7:
            return "PENV_7";
        case DMPAPER_PENV_8:
            return "PENV_8";
        case DMPAPER_PENV_9:
            return "PENV_9";
        case DMPAPER_PENV_10:
            return "PENV_10";
        case DMPAPER_P16K_ROTATED:
            return "P16K_ROTATED";
        case DMPAPER_P32K_ROTATED:
            return "P32K_ROTATED";
        case DMPAPER_P32KBIG_ROTATED:
            return "P32KBIG_ROTATED";
        case DMPAPER_PENV_1_ROTATED:
            return "PENV_1_ROTATED";
        case DMPAPER_PENV_2_ROTATED:
            return "PENV_2_ROTATED";
        case DMPAPER_PENV_3_ROTATED:
            return "PENV_3_ROTATED";
        case DMPAPER_PENV_4_ROTATED:
            return "PENV_4_ROTATED";
        case DMPAPER_PENV_5_ROTATED:
            return "PENV_5_ROTATED";
        case DMPAPER_PENV_6_ROTATED:
            return "PENV_6_ROTATED";
        case DMPAPER_PENV_7_ROTATED:
            return "PENV_7_ROTATED";
        case DMPAPER_PENV_8_ROTATED:
            return "PENV_8_ROTATED";
        case DMPAPER_PENV_9_ROTATED:
            return "PENV_9_ROTATED";
        case DMPAPER_PENV_10_ROTATED:
            return "PENV_10_ROTATED";
        default:
            return "?" + std::to_string(dmpaper);
    }
}

inline std::string DC_PAPERSIZE_array_to_string(POINT* pPaperSizes, DWORD nCount)
{
    std::string result;

    for (DWORD i = 0; i < nCount; i++)
    {
        if (i > 0)
            result += ", ";

        result += std::to_string(std::lround(pPaperSizes[i].x / 10.0)) + "x"
                  + std::to_string(std::lround(pPaperSizes[i].y / 10.0));

#if 0
        // WIP. Printer::GetPaperName() should really be inline in <i18nutil/paper.hxx> or
        // something, so that it can be used anywhere. We can't depend on vcl in this file as we
        // might be included in modules that precede vcl.
        PaperInfo paperInfo(pPaperSizes[i].x * 10, pPaperSizes[i].y * 10);
        paperInfo.doSloppyFit(true);
        if (paperInfo.getPaper() != PAPER_USER)
            result += "(" + std::string(Printer::GetPaperName(paperInfo.getPaper()).toUtf8().getStr()) + ")";
#endif
    }
    return result;
}

#endif // INCLUDED_COMPHELPER_WINDOWSDEBUGOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
