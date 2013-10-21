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

#ifndef _STRING_HXX
#define _STRING_HXX

#include <tools/solar.h>
#include <osl/thread.h>
#include <rtl/textenc.h>
#include <rtl/textcvt.h>
#include <rtl/ustrbuf.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "tools/toolsdllapi.h"
#include "tools/lineend.hxx"

/*******************************************************************************
 *
 * THIS CODE IS DEPRECATED.  DO NOT USE IT IN ANY NEW CODE.
 *
 * Use the string classes in rtl/ustring.hxx and rtl/ustrbuf.hxx (and
 * rtl/string.hxx and rtl/strbuf.hxx for byte-sized strings) instead.  If you
 * feel functionality missing from those string classes, please request
 * improvements on discuss@openoffice.org.
 *
 * There will not be any fixes to the code here.
 ******************************************************************************/

class ResId;
class String;
class UniString;

#define BYTESTRING_TO_UNISTRING_CVTFLAGS    (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |\
                                             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |\
                                             RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT)

// CharSet

#ifndef ENUM_CHARSET_DECLARED
#define ENUM_CHARSET_DECLARED

typedef rtl_TextEncoding CharSet;

#endif

// String-Types

#ifdef STRING32
#define STRING_NOTFOUND    ((xub_StrLen)0x7FFFFFFF)
#define STRING_LEN         ((xub_StrLen)0x7FFFFFFF)
#define STRING_MAXLEN      ((xub_StrLen)0x7FFFFFFF)
#else
#define STRING_NOTFOUND    ((xub_StrLen)0xFFFF)
#define STRING_LEN         ((xub_StrLen)0xFFFF)
#define STRING_MAXLEN      ((xub_StrLen)0xFFFF)
#endif

enum StringCompare { COMPARE_LESS = -1, COMPARE_EQUAL = 0, COMPARE_GREATER = 1 };

//Internal String data

// Data used for the management of this String
// use only for debugging purposes (never assign to String directly!)

#ifdef SAL_W32
#pragma pack(push, 4)
#endif

typedef struct _UniStringData
{
    sal_Int32           mnRefCount;     // reference counter
    sal_Int32           mnLen;          // Length of the String
    sal_Unicode         maStr[1];       // CharArray (String)
} UniStringData;

#ifdef SAL_W32
#pragma pack(pop)
#endif

// UniString

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED UniString
{
private:
    UniStringData*      mpData;

    TOOLS_DLLPRIVATE inline void ImplCopyData();
    TOOLS_DLLPRIVATE inline sal_Unicode * ImplCopyStringData(sal_Unicode *);

    StringCompare       CompareTo( const UniString& rStr,
                                   xub_StrLen nLen = STRING_LEN ) const;

    UniString&          Assign( const OUString& rStr );
    UniString&          Append( const UniString& rStr );

                        UniString( const int* pDummy );    // not implemented: to prevent UniString( NULL )
                        UniString(int); // not implemented; to detect misuses of
                                        // UniString(sal_Unicode)
    void                operator =(int); // not implemented; to detect misuses
                                         // of operator =(sal_Unicode)
    void                Append(int); // not implemented; to detect misuses of
                                     // Append(sal_Unicode)

    //detect and reject use of RTL_CONSTASCII_STRINGPARAM instead of RTL_CONSTASCII_USTRINGPARAM
    TOOLS_DLLPRIVATE UniString( const sal_Char*, sal_Int32 );

    //detect and reject wrong way to attempt to create a UniString from a substring of
    //a OString
    TOOLS_DLLPRIVATE UniString(const OString& rByteStr, xub_StrLen nPos, xub_StrLen nLen,
    sal_uInt32       nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS);

    //no longer implemented
    TOOLS_DLLPRIVATE UniString( const OString& rByteStr,
                                   rtl_TextEncoding eTextEncoding,
                                   sal_uInt32 nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS );
    TOOLS_DLLPRIVATE UniString( const sal_Unicode* pCharStr );
    TOOLS_DLLPRIVATE UniString( const sal_Unicode* pCharStr, xub_StrLen nLen );
    TOOLS_DLLPRIVATE UniString( sal_Unicode c );
public:
                        UniString();
                        UniString( const ResId& rResId );
                        UniString( const UniString& rStr );
                        UniString( const UniString& rStr, xub_StrLen nPos, xub_StrLen nLen );
                        UniString( const OUString& rStr );
                        UniString(char c); // ...but allow "UniString('a')"
                        UniString( const sal_Char* pByteStr,
                                   rtl_TextEncoding eTextEncoding,
                                   sal_uInt32 nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS );
                        UniString( const sal_Char* pByteStr, xub_StrLen nLen,
                                   rtl_TextEncoding eTextEncoding,
                                   sal_uInt32 nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS );
                        ~UniString();

    operator OUString () const
    {
        return OUString( rtl_uStringBuffer_refReturn(
                                reinterpret_cast<rtl_uString*>(mpData)), SAL_NO_ACQUIRE );
    }

#ifdef RTL_FAST_STRING
    template< typename T1, typename T2 >
    UniString( const rtl::OUStringConcat< T1, T2 >& concat )
        : mpData(NULL) { Assign( OUString( concat )); }
    template< typename T1, typename T2 >
    UniString&          operator =( const rtl::OUStringConcat< T1, T2 >& concat )
                            { return Assign( OUString( concat )); }
    template< typename T1, typename T2 >
    UniString&          operator +=( const rtl::OUStringConcat< T1, T2 >& concat )
                            { return Append( UniString( concat ) ); }
#endif

    UniString&          operator =( const OUString& rStr )
                            { return Assign( rStr ); }

    UniString&          operator +=( const OUString& rStr )
                            { return Append( UniString(rStr) ); }

    xub_StrLen          Len() const { return (xub_StrLen)mpData->mnLen; }

    UniString           Copy( xub_StrLen nIndex = 0, xub_StrLen nCount = STRING_LEN ) const
        { return UniString( *this, nIndex, nCount ); }

    const sal_Unicode*  GetBuffer() const { return mpData->maStr; }

    friend sal_Bool     operator == ( const UniString& rStr1,   const UniString& rStr2 );
    friend sal_Bool     operator != ( const UniString& rStr1,   const UniString& rStr2 )
                            { return !(operator == ( rStr1, rStr2 )); }
    friend sal_Bool     operator <  ( const UniString& rStr1,   const UniString& rStr2 )
                            { return (rStr1.CompareTo( rStr2 ) == COMPARE_LESS); }
    friend sal_Bool     operator >  ( const UniString& rStr1,   const UniString& rStr2 )
                            { return (rStr1.CompareTo( rStr2 ) == COMPARE_GREATER); }
    friend sal_Bool     operator <= ( const UniString& rStr1,   const UniString& rStr2 )
                            { return !(operator > ( rStr1, rStr2 )); }
    friend sal_Bool     operator >= ( const UniString& rStr1,   const UniString& rStr2 )
                            { return !(operator < ( rStr1, rStr2 )); }
};

template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, UniString const & string)
{
    return stream <<
        OUStringToOString(string, RTL_TEXTENCODING_UTF8).getStr();
        // best effort; potentially loses data due to conversion failures
        // (stray surrogate halves) and embedded null characters
}

#ifdef RTL_FAST_STRING
namespace rtl
{
template<>
struct ToStringHelper< UniString >
    {
    static int length( const UniString& s ) { return s.Len(); }
    static sal_Unicode* addData( sal_Unicode* buffer, const UniString& s ) { return addDataHelper( buffer, s.GetBuffer(), s.Len()); }
    static const bool allowOStringConcat = false;
    static const bool allowOUStringConcat = true;
    };
}

#endif

// some compare operators, so that conversions from String to OUString don't
// have to insert conversions all over the place
inline bool operator==(UniString const& rLeft, OUString const& rRight)
{
    return OUString(rLeft) == rRight;
}

inline bool operator==(OUString const& rLeft, UniString const& rRight)
{
    return rLeft == OUString(rRight);
}

inline bool operator!=(UniString const& rLeft, OUString const& rRight)
{
    return OUString(rLeft) != rRight;
}

inline bool operator!=(OUString const& rLeft, UniString const& rRight)
{
    return rLeft != OUString(rRight);
}

#ifdef RTL_FAST_STRING
// The above operators make comparisons involving fast string concatenation ambiguous, so provide explicit overloads.
template< typename T1, typename T2 >
inline bool operator==( const rtl::OUStringConcat< T1, T2 >& concat, const OUString& str )
{
    return OUString( concat ) == str;
}

template< typename T1, typename T2 >
inline bool operator!=( const rtl::OUStringConcat< T1, T2 >& concat, const OUString& str )
{
    return OUString( concat ) == str;
}

template< typename T1, typename T2 >
inline bool operator==( const OUString& str, const rtl::OUStringConcat< T1, T2 >& concat )
{
    return str == OUString( concat );
}

template< typename T1, typename T2 >
inline bool operator!=( const OUString& str, const rtl::OUStringConcat< T1, T2 >& concat )
{
    return str != OUString( concat );
}

template< typename T1, typename T2 >
inline bool operator==( const rtl::OUStringConcat< T1, T2 >& concat, const UniString& str )
{
    return UniString( concat ) == str;
}

template< typename T1, typename T2 >
inline bool operator!=( const rtl::OUStringConcat< T1, T2 >& concat, const UniString& str )
{
    return UniString( concat ) == str;
}

template< typename T1, typename T2 >
inline bool operator==( const UniString& str, const rtl::OUStringConcat< T1, T2 >& concat )
{
    return str == UniString( concat );
}

template< typename T1, typename T2 >
inline bool operator!=( const UniString& str, const rtl::OUStringConcat< T1, T2 >& concat )
{
    return str != UniString( concat );
}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
