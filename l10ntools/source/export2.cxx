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

#include "export.hxx"
#include <stdio.h>
#include <osl/time.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <sal/macros.h>
#include <iostream>
#include <iomanip>
#include <tools/urlobj.hxx>
#include <comphelper/string.hxx>
#include <time.h>
#include <stdlib.h>

using namespace std;
using comphelper::string::getToken;
//
// class ResData();
//

/*****************************************************************************/
ResData::~ResData()
/*****************************************************************************/
{
    if ( pStringList ) {
        // delete existing res. of type StringList
        for ( size_t i = 0; i < pStringList->size(); i++ ) {
            ExportListEntry* test = (*pStringList)[ i ];
            if( test != NULL ) delete test;
        }
        delete pStringList;
    }
    if ( pFilterList ) {
        // delete existing res. of type FilterList
        for ( size_t i = 0; i < pFilterList->size(); i++ ) {
            ExportListEntry* test = (*pFilterList)[ i ];
            delete test;
        }
        delete pFilterList;
    }
    if ( pItemList ) {
        // delete existing res. of type ItemList
        for ( size_t i = 0; i < pItemList->size(); i++ ) {
            ExportListEntry* test = (*pItemList)[ i ];
            delete test;
        }
        delete pItemList;
    }
    if ( pUIEntries ) {
        // delete existing res. of type UIEntries
        for ( size_t i = 0; i < pUIEntries->size(); i++ ) {
            ExportListEntry* test = (*pUIEntries)[ i ];
            delete test;
        }
        delete pUIEntries;
    }
}

//
// class Export
//

/*****************************************************************************/
rtl::OString Export::sLanguages;
rtl::OString Export::sForcedLanguages;
/*****************************************************************************/

void Export::DumpExportList(const rtl::OString& rListName, ExportList& aList)
{
    printf( "%s\n", rListName.getStr() );
    rtl::OString l;
    ExportListEntry* aEntry;
    for( unsigned int x = 0; x < aList.size() ; ++x )
    {
        aEntry = (ExportListEntry*) aList[ x ];
        Export::DumpMap( l , *aEntry );
    }
    printf("\n");
}

void Export::DumpMap(const rtl::OString& rMapName,
    OStringHashMap& aMap)
{
    if( rMapName.getLength() )
        printf("MapName %s\n", rMapName.getStr());
    if( aMap.size() < 1 )
        return;
    for(OStringHashMap::const_iterator idbg = aMap.begin(); idbg != aMap.end(); ++idbg)
    {
        ByteString a( idbg->first );
        ByteString b( idbg->second );
        printf("[%s]= %s",a.GetBuffer(),b.GetBuffer());
        printf("\n");
    }
    printf("\n");
    return;
}

/*****************************************************************************/
void Export::SetLanguages( std::vector<rtl::OString> val ){
/*****************************************************************************/
    aLanguages = val;
    isInitialized = true;
}
/*****************************************************************************/
std::vector<rtl::OString> Export::GetLanguages(){
/*****************************************************************************/
    return aLanguages;
}
/*****************************************************************************/
std::vector<rtl::OString> Export::GetForcedLanguages(){
/*****************************************************************************/
    return aForcedLanguages;
}
std::vector<rtl::OString> Export::aLanguages       = std::vector<rtl::OString>();
std::vector<rtl::OString> Export::aForcedLanguages = std::vector<rtl::OString>();

/*****************************************************************************/
rtl::OString Export::QuoteHTML( rtl::OString const &rString )
/*****************************************************************************/
{
    rtl::OStringBuffer sReturn;
    for ( sal_Int32 i = 0; i < rString.getLength(); i++ ) {
        rtl::OString sTemp = rString.copy( i );
        if ( sTemp.match( "<Arg n=" ) ) {
            while ( i < rString.getLength() && rString[i] != '>' ) {
                 sReturn.append(rString[i]);
                i++;
            }
            if ( rString[i] == '>' ) {
                sReturn.append('>');
                i++;
            }
        }
        if ( i < rString.getLength()) {
            switch ( rString[i]) {
                case '<':
                    sReturn.append("&lt;");
                break;

                case '>':
                    sReturn.append("&gt;");
                break;

                case '\"':
                    sReturn.append("&quot;");
                break;

                case '\'':
                    sReturn.append("&apos;");
                break;

                case '&':
                    if ((( i + 4 ) < rString.getLength()) &&
                        ( rString.copy( i, 5 ) == "&amp;" ))
                            sReturn.append(rString[i]);
                    else
                        sReturn.append("&amp;");
                break;

                default:
                    sReturn.append(rString[i]);
                break;
            }
        }
    }
    return sReturn.makeStringAndClear();
}

void Export::RemoveUTF8ByteOrderMarker( rtl::OString &rString )
{
    if( hasUTF8ByteOrderMarker( rString ) )
        rString = rString.copy(3);
}

bool Export::hasUTF8ByteOrderMarker( const rtl::OString &rString )
{
    return rString.getLength() >= 3 && rString[0] == '\xEF' &&
           rString[1] == '\xBB' && rString[2] == '\xBF' ;
}

bool Export::fileHasUTF8ByteOrderMarker(const rtl::OString &rString)
{
    SvFileStream aFileIn(rtl::OStringToOUString(rString, RTL_TEXTENCODING_ASCII_US), STREAM_READ);
    rtl::OString sLine;
    if( !aFileIn.IsEof() )
    {
        aFileIn.ReadLine( sLine );
        if( aFileIn.IsOpen() )
            aFileIn.Close();
        return hasUTF8ByteOrderMarker( sLine );
    }
    if( aFileIn.IsOpen() ) aFileIn.Close();
    return false;
}

void Export::RemoveUTF8ByteOrderMarkerFromFile(const rtl::OString &rFilename)
{
    SvFileStream aFileIn(rtl::OStringToOUString(rFilename , RTL_TEXTENCODING_ASCII_US) , STREAM_READ );
    rtl::OString sLine;
    if( !aFileIn.IsEof() )
    {
        aFileIn.ReadLine( sLine );
        // Test header
        if( hasUTF8ByteOrderMarker( sLine ) )
        {
            DirEntry aTempFile = Export::GetTempFile();
            rtl::OString sTempFile = rtl::OUStringToOString(aTempFile.GetFull() , RTL_TEXTENCODING_ASCII_US);
            SvFileStream aNewFile(rtl::OStringToOUString(sTempFile , RTL_TEXTENCODING_ASCII_US) , STREAM_WRITE);
            // Remove header
            RemoveUTF8ByteOrderMarker( sLine );
            aNewFile.WriteLine( sLine );
            // Copy the rest
            while( !aFileIn.IsEof() )
            {
                aFileIn.ReadLine( sLine );
                aNewFile.WriteLine( sLine );
            }
            if( aFileIn.IsOpen() ) aFileIn.Close();
            if( aNewFile.IsOpen() ) aNewFile.Close();
            DirEntry aEntry( rFilename.getStr() );
            aEntry.Kill();
            DirEntry( sTempFile ).MoveTo( DirEntry( rFilename.getStr() ) );
        }
    }
    if( aFileIn.IsOpen() )
        aFileIn.Close();
}

bool Export::CopyFile(const rtl::OString& rSource, const rtl::OString& rDest)
{
    const int BUFFERSIZE    = 8192;
    char buf[ BUFFERSIZE ];

    FILE* IN_FILE = fopen( rSource.getStr() , "r" );
    if( IN_FILE == NULL )
    {
        cerr << "Export::CopyFile WARNING: Could not open " << rSource.getStr() << "\n";
        return false;
    }

    FILE* OUT_FILE = fopen( rDest.getStr() , "w" );
    if( OUT_FILE == NULL )
    {
        cerr << "Export::CopyFile WARNING: Could not open/create " << rDest.getStr() << " for writing\n";
        fclose( IN_FILE );
        return false;
    }

    while( fgets( buf , BUFFERSIZE , IN_FILE ) != NULL )
    {
        if( fputs( buf , OUT_FILE ) == EOF )
        {
            cerr << "Export::CopyFile WARNING: Write problems " << rSource.getStr() << "\n";
            fclose( IN_FILE );
            fclose( OUT_FILE );
            return false;
        }
    }
    if( ferror( IN_FILE ) )
    {
        cerr << "Export::CopyFile WARNING: Read problems " << rDest.getStr() << "\n";
        fclose( IN_FILE );
        fclose( OUT_FILE );
        return false;
    }
    fclose ( IN_FILE );
    fclose ( OUT_FILE );

    return true;
}

/*****************************************************************************/
rtl::OString Export::UnquoteHTML( rtl::OString const &rString )
/*****************************************************************************/
{
    rtl::OStringBuffer sReturn;
    for (sal_Int32 i = 0; i != rString.getLength();) {
        if (rString.match("&amp;", i)) {
            sReturn.append('&');
            i += RTL_CONSTASCII_LENGTH("&amp;");
        } else if (rString.match("&lt;", i)) {
            sReturn.append('<');
            i += RTL_CONSTASCII_LENGTH("&lt;");
        } else if (rString.match("&gt;", i)) {
            sReturn.append('>');
            i += RTL_CONSTASCII_LENGTH("&gt;");
        } else if (rString.match("&quot;", i)) {
            sReturn.append('"');
            i += RTL_CONSTASCII_LENGTH("&quot;");
        } else if (rString.match("&apos;", i)) {
            sReturn.append('\'');
            i += RTL_CONSTASCII_LENGTH("&apos;");
        } else {
            sReturn.append(rString[i]);
            ++i;
        }
    }
    return sReturn.makeStringAndClear();
}

bool Export::isSourceLanguage(const rtl::OString &rLanguage)
{
    return !isAllowed(rLanguage);
}

bool Export::isAllowed(const rtl::OString &rLanguage)
{
    return !rLanguage.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US"));
}

bool Export::isInitialized = false;

/*****************************************************************************/
void Export::InitLanguages( bool bMergeMode ){
/*****************************************************************************/
    if( !isInitialized )
    {
        ByteString sTmp;
        OStringBoolHashMap aEnvLangs;

        sal_Int32 nIndex = 0;
        do
        {
            rtl::OString aToken = sLanguages.getToken(0, ',', nIndex);
            sTmp = getToken(aToken, 0, '=');
            sTmp = comphelper::string::strip(sTmp, ' ');
            if( bMergeMode && !isAllowed( sTmp ) ){}
            else if( !( (sTmp.GetChar(0)=='x' || sTmp.GetChar(0)=='X') && sTmp.GetChar(1)=='-' ) ){
                aLanguages.push_back( sTmp );
            }
        }
        while ( nIndex >= 0 );

        InitForcedLanguages( bMergeMode );
        isInitialized = true;
    }
}
/*****************************************************************************/
void Export::InitForcedLanguages( bool bMergeMode ){
/*****************************************************************************/
    ByteString sTmp;
    OStringBoolHashMap aEnvLangs;

    sal_Int32 nIndex = 0;
    do
    {
        rtl::OString aToken = sForcedLanguages.getToken(0, ',', nIndex);

        sTmp = getToken(aToken, 0, '=');
        sTmp = comphelper::string::strip(sTmp, ' ');
        if( bMergeMode && isAllowed( sTmp ) ){}
        else if( !( (sTmp.GetChar(0)=='x' || sTmp.GetChar(0)=='X') && sTmp.GetChar(1)=='-' ) )
            aForcedLanguages.push_back( sTmp );
    }
    while ( nIndex >= 0 );
}

rtl::OString Export::GetTimeStamp()
{
//  return "xx.xx.xx";
    char buf[20];
    Time aTime( Time::SYSTEM );

    snprintf(buf, sizeof(buf), "%8d %02d:%02d:%02d", int(Date( Date::SYSTEM).GetDate()),
        int(aTime.GetHour()), int(aTime.GetMin()), int(aTime.GetSec()));
    return rtl::OString(buf);
}

/*****************************************************************************/
sal_Bool Export::ConvertLineEnds(
    rtl::OString const & sSource, rtl::OString const & sDestination )
/*****************************************************************************/
{
    String sSourceFile( sSource, RTL_TEXTENCODING_ASCII_US );
    String sDestinationFile( sDestination, RTL_TEXTENCODING_ASCII_US );

    SvFileStream aSource( sSourceFile, STREAM_READ );
    if ( !aSource.IsOpen())
        return sal_False;
    SvFileStream aDestination( sDestinationFile, STREAM_STD_WRITE | STREAM_TRUNC );
    if ( !aDestination.IsOpen()) {
        aSource.Close();
        return sal_False;
    }

    rtl::OString sLine;

    while ( !aSource.IsEof())
    {
        aSource.ReadLine( sLine );
        if ( !aSource.IsEof())  //a complete line
        {
            sLine = comphelper::string::remove(sLine, '\r');
            aDestination.WriteLine( sLine );
        }
        else                    //a final incomplete line, just copy it as-is
            aDestination.Write( sLine.getStr(), sLine.getLength() );
    }

    aSource.Close();
    aDestination.Close();

    return sal_True;
}

/*****************************************************************************/
rtl::OString Export::GetNativeFile( rtl::OString const & sSource )
/*****************************************************************************/
{
    DirEntry aTemp( GetTempFile());
    rtl::OString sReturn(rtl::OUStringToOString(aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US));

    for ( sal_uInt16 i = 0; i < 10; i++ )
        if ( ConvertLineEnds( sSource, sReturn ))
            return sReturn;

    return rtl::OString();
}

const char* Export::GetEnv( const char *pVar )
{
        char *pRet = getenv( pVar );
        if ( !pRet )
            pRet = 0;
        return pRet;
}

void Export::getCurrentDir( string& dir )
{
    char buffer[64000];
    if( getcwd( buffer , sizeof( buffer ) ) == 0 ){
        cerr << "Error: getcwd failed!\n";
        exit( -1 );
    }
    dir = string( buffer );
}


// Stolen from sal/osl/unx/tempfile.c

#define RAND_NAME_LENGTH 6

rtl::OString Export::getRandomName(const rtl::OString& rPrefix, const rtl::OString& rPostfix)
{
    static const char LETTERS[]        = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    static const int  COUNT_OF_LETTERS = SAL_N_ELEMENTS(LETTERS) - 1;

    rtl::OStringBuffer sRandStr(rPrefix);

    static sal_uInt64 value;
    char     buffer[RAND_NAME_LENGTH];

    TimeValue           tv;
    sal_uInt64          v;
    int                 i;

    osl_getSystemTime( &tv );
    oslProcessInfo proInfo;
    proInfo.Size = sizeof(oslProcessInfo);
    osl_getProcessInfo( 0 , osl_Process_IDENTIFIER , &proInfo );

    value += ((sal_uInt64) ( tv.Nanosec / 1000 ) << 16) ^ ( tv.Nanosec / 1000 ) ^ proInfo.Ident;

    v = value;

    for (i = 0; i < RAND_NAME_LENGTH; ++i)
    {
        buffer[i] = LETTERS[v % COUNT_OF_LETTERS];
        v        /= COUNT_OF_LETTERS;
    }

    sRandStr.append(buffer , RAND_NAME_LENGTH);
    sRandStr.append(rPostfix);
    return sRandStr.makeStringAndClear();
}

/*****************************************************************************/
DirEntry Export::GetTempFile()
/*****************************************************************************/
{
    rtl::OUString* sTempFilename = new rtl::OUString();

    // Create a temp file
    int nRC = osl::FileBase::createTempFile( 0 , 0 , sTempFilename );
    if( nRC ) printf(" osl::FileBase::createTempFile RC = %d",nRC);

    String strTmp( *sTempFilename  );

    INetURLObject::DecodeMechanism eMechanism = INetURLObject::DECODE_TO_IURI;
    String sDecodedStr = INetURLObject::decode( strTmp , '%' , eMechanism );
    rtl::OString sTmp(rtl::OUStringToOString(sDecodedStr , RTL_TEXTENCODING_UTF8));

#if defined(WNT)
    sTmp = comphelper::string::replace(sTmp,
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("file:///")),
        rtl::OString());
    sTmp = sTmp.replace('/', '\\');
#else
    // Set file permission to 644
    const sal_uInt64 nPerm = osl_File_Attribute_OwnRead | osl_File_Attribute_OwnWrite |
                             osl_File_Attribute_GrpRead | osl_File_Attribute_OthRead ;

    nRC = osl::File::setAttributes( *sTempFilename , nPerm );
    if( nRC ) printf(" osl::File::setAttributes RC = %d",nRC);

    sTmp = comphelper::string::replace(sTmp,
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("file://")),
        rtl::OString());
#endif
    DirEntry aDirEntry( sTmp );
    delete sTempFilename;
    return aDirEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
