/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include <MNSInclude.hxx>
#include "MTypeConverter.hxx"
#include <ucbhelper/content.hxx>

using namespace connectivity::mozab;

// -------------------------------------------------------------------------
void MTypeConverter::ouStringToNsString(::rtl::OUString const &ous, nsString &nss)
{
    // Convert to ::rtl::OString (utf-8 encoding).
    ::rtl::OString os(rtl::OUStringToOString(ous, RTL_TEXTENCODING_UTF8));

    const char *cs = os.getStr();
    PRUint32 csLen = os.getLength();

    NS_ConvertUTF8toUCS2 mozString(cs, csLen);
    //const PRUnichar* uniMozString = (const PRUnichar*) mozString;
    nss = mozString; // temp.
}
// -------------------------------------------------------------------------
::rtl::OUString MTypeConverter::nsACStringToOUString( const nsACString& _source )
{
    const char* buffer = _source.BeginReading();
    const char* bufferEnd = _source.EndReading();
    return ::rtl::OUString( buffer, static_cast<sal_Int32>(bufferEnd - buffer), RTL_TEXTENCODING_ASCII_US );
}
// -------------------------------------------------------------------------
::rtl::OString MTypeConverter::nsACStringToOString( const nsACString& _source )
{
    const char* buffer = _source.BeginReading();
    const char* bufferEnd = _source.EndReading();
    return ::rtl::OString( buffer, static_cast<sal_Int32>(bufferEnd - buffer) );
}
// -------------------------------------------------------------------------
void MTypeConverter::asciiOUStringToNsACString( const ::rtl::OUString& _asciiString, nsACString& _dest )
{
    ::rtl::OString sAsciiVersion( _asciiString.getStr(), _asciiString.getLength(), RTL_TEXTENCODING_ASCII_US );
    asciiToNsACString( sAsciiVersion.getStr(), _dest );
}
// -------------------------------------------------------------------------
void MTypeConverter::asciiToNsACString( const sal_Char* _asciiString, nsACString& _dest )
{
    _dest.Truncate();
    _dest.AppendASCII( _asciiString );
}
// -------------------------------------------------------------------------
void MTypeConverter::nsStringToOUString(nsString const &nss, ::rtl::OUString &ous)
{
    // Get clone of buffer.
    PRUnichar *uc = ToNewUnicode(nss);
    sal_Int32 nssLen = nss.Length();

    // TODO check if this is ok.
    // PRUnichar != sal_Unicode in mingw
    ::rtl::OUString _ous(reinterpret_cast_mingw_only<sal_Unicode *>(uc), nssLen);
    ous = _ous;

    nsMemory::Free(uc);
}
// -------------------------------------------------------------------------
void MTypeConverter::prUnicharToOUString(PRUnichar const *pru, ::rtl::OUString &ous)
{
    // TODO, specify length.
    // PRUnichar != sal_Unicode in mingw
    ::rtl::OUString _ous(reinterpret_cast_mingw_only<const sal_Unicode *>(pru));
    ous = _ous;
}
// -------------------------------------------------------------------------
char *MTypeConverter::ouStringToCCharStringUtf8(::rtl::OUString const &ous)
{
    // Convert to ::rtl::OString,
    ::rtl::OString os(rtl::OUStringToOString(ous, RTL_TEXTENCODING_UTF8));

    const char *cs = os.getStr();

    return(strdup(cs));
}
// -------------------------------------------------------------------------
char *MTypeConverter::ouStringToCCharStringAscii(::rtl::OUString const &ous)
{
    // Convert ::rtl::OUString to ::rtl::OString,
    ::rtl::OString os(rtl::OUStringToOString(ous, RTL_TEXTENCODING_ASCII_US));

    return(strdup(os.getStr()));
}
// -------------------------------------------------------------------------
char *MTypeConverter::nsStringToCCharStringAscii(nsString const &nss)
{
    char cs[1024];
    nss.ToCString(cs, 1024);

    return(strdup(cs));
}
// -------------------------------------------------------------------------
::std::string MTypeConverter::ouStringToStlString(::rtl::OUString const &ous)
{
    // Convert ::rtl::OUString to ::rtl::OString.
    ::rtl::OString os(rtl::OUStringToOString(ous,RTL_TEXTENCODING_ASCII_US));
    return( ::std::string(os.getStr()));
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
