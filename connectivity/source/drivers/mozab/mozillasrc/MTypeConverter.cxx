/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    ::rtl::OString os(ous,ous.getLength(), RTL_TEXTENCODING_UTF8);

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
    ::rtl::OString os(ous,ous.getLength(), RTL_TEXTENCODING_UTF8);

    const char *cs = os.getStr();

    return(strdup(cs));
}
// -------------------------------------------------------------------------
char *MTypeConverter::ouStringToCCharStringAscii(::rtl::OUString const &ous)
{
    // Convert ::rtl::OUString to ::rtl::OString,
    ::rtl::OString os(ous,ous.getLength(), RTL_TEXTENCODING_ASCII_US);

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
    ::rtl::OString os(ous,ous.getLength(),RTL_TEXTENCODING_ASCII_US);
    return( ::std::string(os.getStr()));
}
#if 0
// -------------------------------------------------------------------------
::std::string MTypeConverter::nsStringToStlString(nsString const &nss)
{
    return( ::std::string(nss.GetBuffer()));
}
#endif
// -------------------------------------------------------------------------
