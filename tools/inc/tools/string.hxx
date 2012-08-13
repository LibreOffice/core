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
#define STRING_MATCH       ((xub_StrLen)0x7FFFFFFF)
#define STRING_LEN         ((xub_StrLen)0x7FFFFFFF)
#define STRING_MAXLEN      ((xub_StrLen)0x7FFFFFFF)
#else
#define STRING_NOTFOUND    ((xub_StrLen)0xFFFF)
#define STRING_MATCH       ((xub_StrLen)0xFFFF)
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

class TOOLS_DLLPUBLIC UniString
{
private:
    UniStringData*      mpData;

    TOOLS_DLLPRIVATE inline void ImplCopyData();
    TOOLS_DLLPRIVATE inline sal_Unicode * ImplCopyStringData(sal_Unicode *);

                        UniString( const int* pDummy );    // not implemented: to prevent UniString( NULL )
                        UniString(int); // not implemented; to detect misuses of
                                        // UniString(sal_Unicode)
    void                Assign(int); // not implemented; to detect misuses of
                                     // Assign(sal_Unicode)
    void                operator =(int); // not implemented; to detect misuses
                                         // of operator =(sal_Unicode)
    void                Append(int); // not implemented; to detect misuses of
                                     // Append(sal_Unicode)
    void                operator +=(int); // not implemented; to detect misuses
                                          // of operator +=(sal_Unicode)

    //detect and reject use of RTL_CONSTASCII_STRINGPARAM instead of RTL_CONSTASCII_USTRINGPARAM
    TOOLS_DLLPRIVATE UniString( const sal_Char*, sal_Int32 );

    //detect and reject wrong way to attempt to create a UniString from a substring of
    //a rtl::OString
    TOOLS_DLLPRIVATE UniString(const rtl::OString& rByteStr, xub_StrLen nPos, xub_StrLen nLen,
    sal_uInt32       nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS);

    //no longer implemented
    TOOLS_DLLPRIVATE UniString( const rtl::OString& rByteStr,
                                   rtl_TextEncoding eTextEncoding,
                                   sal_uInt32 nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS );
    TOOLS_DLLPRIVATE UniString( const sal_Unicode* pCharStr );
    TOOLS_DLLPRIVATE UniString( const sal_Unicode* pCharStr, xub_StrLen nLen );

public:
                        UniString();
                        UniString( const ResId& rResId );
                        UniString( const UniString& rStr );
                        UniString( const UniString& rStr, xub_StrLen nPos, xub_StrLen nLen );
                        UniString( const rtl::OUString& rStr );
                        UniString( sal_Unicode c );
                        UniString(char c); // ...but allow "UniString('a')"
                        UniString( const sal_Char* pByteStr,
                                   rtl_TextEncoding eTextEncoding,
                                   sal_uInt32 nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS );
                        UniString( const sal_Char* pByteStr, xub_StrLen nLen,
                                   rtl_TextEncoding eTextEncoding,
                                   sal_uInt32 nCvtFlags = BYTESTRING_TO_UNISTRING_CVTFLAGS );
                        ~UniString();

    operator rtl::OUString () const
    {
        return rtl::OUString( rtl_uStringBuffer_refReturn(
                                reinterpret_cast<rtl_uString*>(mpData)), SAL_NO_ACQUIRE );
    }

    static UniString    CreateFromInt32( sal_Int32 n, sal_Int16 nRadix = 10 );
    static const UniString& EmptyString();
    sal_Int32           ToInt32() const;
    sal_Int64           ToInt64() const;

    UniString&          Assign( const UniString& rStr );
    UniString&          Assign( const rtl::OUString& rStr );
    UniString&          Assign( const sal_Unicode* pCharStr );
    UniString&          Assign( const sal_Unicode* pCharStr, xub_StrLen nLen );
    UniString&          Assign( sal_Unicode c );
    inline UniString & Assign(char c) // ...but allow "Assign('a')"
        { return Assign(static_cast< sal_Unicode >(c)); }
    UniString&          AssignAscii( const sal_Char* pAsciiStr );
    UniString&          AssignAscii( const sal_Char* pAsciiStr, xub_StrLen nLen );
    UniString&          operator =( const UniString& rStr )
                            { return Assign( rStr ); }
    UniString&          operator =( const rtl::OUString& rStr )
                            { return Assign( rStr ); }
    UniString&          operator =( const sal_Unicode* pCharStr )
                            { return Assign( pCharStr ); }
    UniString&          operator =( sal_Unicode c )
                            { return Assign( c ); }
    inline UniString & operator =(char c) // ...but allow "= 'a'"
        { return operator =(static_cast< sal_Unicode >(c)); }

    UniString&          Append( const UniString& rStr );
    UniString&          Append( const sal_Unicode* pCharStr );
    UniString&          Append( const sal_Unicode* pCharStr, xub_StrLen nLen );
    UniString&          Append( sal_Unicode c );
    inline UniString & Append(char c) // ...but allow "Append('a')"
        { return Append(static_cast< sal_Unicode >(c)); }
    UniString&          AppendAscii( const sal_Char* pAsciiStr );
    UniString&          AppendAscii( const sal_Char* pAsciiStr, xub_StrLen nLen );
    UniString&          operator +=( const UniString& rStr )
                            { return Append( rStr ); }
    UniString&          operator +=( const rtl::OUString& rStr )
                            { return Append( UniString(rStr) ); }
    UniString&          operator +=( const sal_Unicode* pCharStr )
                            { return Append( pCharStr ); }
    UniString&          operator +=( sal_Unicode c )
                            { return Append( c ); }
    inline UniString & operator +=(char c) // ...but allow "+= 'a'"
        { return operator +=(static_cast< sal_Unicode >(c)); }

    void                SetChar( xub_StrLen nIndex, sal_Unicode c );
    sal_Unicode         GetChar( xub_StrLen nIndex ) const
                            { return mpData->maStr[nIndex]; }

    xub_StrLen          Len() const { return (xub_StrLen)mpData->mnLen; }

    UniString&          Insert( const UniString& rStr, xub_StrLen nIndex = STRING_LEN );
    UniString&          Insert( const UniString& rStr, xub_StrLen nPos, xub_StrLen nLen,
                                xub_StrLen nIndex = STRING_LEN );
    UniString&          Insert( sal_Unicode c, xub_StrLen nIndex = STRING_LEN );
    UniString&          InsertAscii( const sal_Char* pAsciiStr, xub_StrLen nIndex = STRING_LEN );
    UniString&          Replace( xub_StrLen nIndex, xub_StrLen nLen, const UniString& rStr );
    UniString&          ReplaceAscii( xub_StrLen nIndex, xub_StrLen nLen,
                                      const sal_Char* pAsciiStr, xub_StrLen nStrLen = STRING_LEN );
    UniString&          Erase( xub_StrLen nIndex = 0, xub_StrLen nCount = STRING_LEN );
    UniString           Copy( xub_StrLen nIndex = 0, xub_StrLen nCount = STRING_LEN ) const;

    UniString&          Fill( xub_StrLen nCount, sal_Unicode cFillChar = ' ' );
    UniString&          Expand( xub_StrLen nCount, sal_Unicode cExpandChar = ' ' );

    UniString&          ToLowerAscii();
    UniString&          ToUpperAscii();

    StringCompare       CompareTo( const UniString& rStr,
                                   xub_StrLen nLen = STRING_LEN ) const;
    StringCompare       CompareToAscii( const sal_Char* pAsciiStr,
                                        xub_StrLen nLen = STRING_LEN ) const;
    StringCompare       CompareIgnoreCaseToAscii( const UniString& rStr,
                                                  xub_StrLen nLen = STRING_LEN ) const;
    StringCompare       CompareIgnoreCaseToAscii( const sal_Char* pAsciiStr,
                                                  xub_StrLen nLen = STRING_LEN ) const;
    sal_Bool            Equals( const UniString& rStr ) const;
    sal_Bool            EqualsAscii( const sal_Char* pAsciiStr ) const;
    sal_Bool            EqualsIgnoreCaseAscii( const UniString& rStr ) const;
    sal_Bool            EqualsIgnoreCaseAscii( const sal_Unicode* pCharStr ) const;
    sal_Bool            EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr ) const;
    sal_Bool            Equals( const UniString& rStr,
                                xub_StrLen nIndex, xub_StrLen nLen ) const;
    sal_Bool            Equals( const sal_Unicode* pCharStr,
                                xub_StrLen nIndex, xub_StrLen nLen ) const;
    sal_Bool            EqualsAscii( const sal_Char* pAsciiStr,
                                     xub_StrLen nIndex, xub_StrLen nLen ) const;
    sal_Bool            EqualsIgnoreCaseAscii( const UniString& rStr,
                                               xub_StrLen nIndex, xub_StrLen nLen ) const;
    sal_Bool            EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr,
                                               xub_StrLen nIndex, xub_StrLen nLen ) const;

    xub_StrLen          Match( const UniString& rStr ) const;

    xub_StrLen          Search( sal_Unicode c, xub_StrLen nIndex = 0 ) const;
    xub_StrLen          Search( const UniString& rStr, xub_StrLen nIndex = 0 ) const;
    xub_StrLen          Search( const sal_Unicode* pCharStr, xub_StrLen nIndex = 0 ) const;
    xub_StrLen          SearchAscii( const sal_Char* pAsciiStr, xub_StrLen nIndex = 0 ) const;
    xub_StrLen          SearchBackward( sal_Unicode c, xub_StrLen nIndex = STRING_LEN ) const;
    xub_StrLen          SearchChar( const sal_Unicode* pChars, xub_StrLen nIndex = 0 ) const;

    xub_StrLen          SearchAndReplace( sal_Unicode c, sal_Unicode cRep,
                                          xub_StrLen nIndex = 0 );
    xub_StrLen          SearchAndReplace( const UniString& rStr, const UniString& rRepStr,
                                          xub_StrLen nIndex = 0 );
    xub_StrLen          SearchAndReplaceAscii( const sal_Char* pAsciiStr, const UniString& rRepStr,
                                               xub_StrLen nIndex = 0 );
    void                SearchAndReplaceAll( sal_Unicode c, sal_Unicode cRep );
    void                SearchAndReplaceAll( const UniString& rStr, const UniString& rRepStr );
    void                SearchAndReplaceAllAscii( const sal_Char* pAsciiStr, const UniString& rRepStr );

    void                SetToken( xub_StrLen nToken, sal_Unicode cTok, const UniString& rStr,
                                  xub_StrLen nIndex = 0 );
    UniString           GetToken( xub_StrLen nToken, sal_Unicode cTok, xub_StrLen& rIndex ) const;
    UniString           GetToken( xub_StrLen nToken, sal_Unicode cTok = ';' ) const;

    const sal_Unicode*  GetBuffer() const { return mpData->maStr; }
    sal_Unicode*        GetBufferAccess();
    void                ReleaseBufferAccess( xub_StrLen nLen = STRING_LEN );
    sal_Unicode*        AllocBuffer( xub_StrLen nLen );

    friend sal_Bool     operator == ( const UniString& rStr1,   const UniString& rStr2 )
                            { return rStr1.Equals( rStr2 ); }
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

inline UniString UniString::Copy( xub_StrLen nIndex, xub_StrLen nCount ) const
{
    return UniString( *this, nIndex, nCount );
}

inline UniString UniString::GetToken( xub_StrLen nToken, sal_Unicode cTok ) const
{
    xub_StrLen nTempPos = 0;
    return GetToken( nToken, cTok, nTempPos );
}

template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, UniString const & string)
{
    return stream <<
        rtl::OUStringToOString(string, RTL_TEXTENCODING_UTF8).getStr();
        // best effort; potentially loses data due to conversion failures
        // (stray surrogate halves) and embedded null characters
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
