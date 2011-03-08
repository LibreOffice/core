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
#include "precompiled_l10ntools.hxx"
#include "export.hxx"
#include "utf8conv.hxx"
#include <tools/datetime.hxx>
#include <tools/isofallback.hxx>
#include <stdio.h>
#include <osl/time.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <sal/macros.h>
#include <iostream>
#include <iomanip>
#include <tools/urlobj.hxx>
#include <time.h>
#include <stdlib.h>

using namespace std;
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
ByteString Export::sLanguages;
ByteString Export::sForcedLanguages;
//ByteString Export::sIsoCode99;
/*****************************************************************************/

void Export::DumpExportList( ByteString& sListName , ExportList& aList ){
    printf( "%s\n", sListName.GetBuffer() );
    ByteString l("");
    ExportListEntry* aEntry;
    for( unsigned int x = 0; x < aList.size() ; x++ ){
        aEntry = (ExportListEntry*) aList[ x ];
        Export::DumpMap( l , *aEntry );
    }
    printf("\n");
}
ByteString Export::DumpMap( ByteString& sMapName , ByteStringHashMap& aMap ){
    ByteStringHashMap::const_iterator idbg;
    ByteString sReturn;

    if( sMapName.Len() )
        printf("MapName %s\n", sMapName.GetBuffer());
    if( aMap.size() < 1 ) return ByteString();
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg ){
        ByteString a( idbg->first );
        ByteString b( idbg->second );
        printf("[%s]= %s",a.GetBuffer(),b.GetBuffer());
        printf("\n");
    }
    printf("\n");
    return sReturn;
}
/*****************************************************************************/
void Export::SetLanguages( std::vector<ByteString> val ){
/*****************************************************************************/
    aLanguages = val;
    isInitialized = true;
}
/*****************************************************************************/
std::vector<ByteString> Export::GetLanguages(){
/*****************************************************************************/
    return aLanguages;
}
/*****************************************************************************/
std::vector<ByteString> Export::GetForcedLanguages(){
/*****************************************************************************/
    return aForcedLanguages;
}
std::vector<ByteString> Export::aLanguages       = std::vector<ByteString>();
std::vector<ByteString> Export::aForcedLanguages = std::vector<ByteString>();


/*****************************************************************************/
void Export::QuotHTMLXRM( ByteString &rString )
/*****************************************************************************/
{
    ByteString sReturn;
    for ( USHORT i = 0; i < rString.Len(); i++ ) {
        ByteString sTemp = rString.Copy( i );
        if ( sTemp.Search( "<Arg n=" ) == 0 ) {
            while ( i < rString.Len() && rString.GetChar( i ) != '>' ) {
                 sReturn += rString.GetChar( i );
                i++;
            }
            if ( rString.GetChar( i ) == '>' ) {
                sReturn += ">";
                i++;
            }
        }

        if ( i < rString.Len()) {
            switch ( rString.GetChar( i )) {
                case '<':
                    if( i+2 < rString.Len() &&
                        (rString.GetChar( i+1 ) == 'b' || rString.GetChar( i+1 ) == 'B') &&
                        rString.GetChar( i+2 ) == '>' )
                    {
                           sReturn +="<b>";
                           i += 2;
                    }
                    else if( i+3 < rString.Len() &&
                             rString.GetChar( i+1 ) == '/' &&
                             (rString.GetChar( i+2 ) == 'b' || rString.GetChar( i+2 ) == 'B') &&
                             rString.GetChar( i+3 ) == '>' )
                    {
                           sReturn +="</b>";
                           i += 3;
                    }
                    else
                        sReturn += "&lt;";
                break;

                case '>':
                    sReturn += "&gt;";
                break;

                case '\"':
                    sReturn += "&quot;";
                break;

                case '\'':
                    sReturn += "&apos;";
                break;

                case '&':
                    if ((( i + 4 ) < rString.Len()) &&
                        ( rString.Copy( i, 5 ) == "&amp;" ))
                            sReturn += rString.GetChar( i );
                    else
                        sReturn += "&amp;";
                break;

                default:
                    sReturn += rString.GetChar( i );
                break;
            }
        }
    }
    rString = sReturn;
}
/*****************************************************************************/
void Export::QuotHTML( ByteString &rString )
/*****************************************************************************/
{
    ByteString sReturn;
    for ( USHORT i = 0; i < rString.Len(); i++ ) {
        ByteString sTemp = rString.Copy( i );
        if ( sTemp.Search( "<Arg n=" ) == 0 ) {
            while ( i < rString.Len() && rString.GetChar( i ) != '>' ) {
                 sReturn += rString.GetChar( i );
                i++;
            }
            if ( rString.GetChar( i ) == '>' ) {
                sReturn += ">";
                i++;
            }
        }
        if ( i < rString.Len()) {
            switch ( rString.GetChar( i )) {
                case '<':
                    sReturn += "&lt;";
                break;

                case '>':
                    sReturn += "&gt;";
                break;

                case '\"':
                    sReturn += "&quot;";
                break;

                case '\'':
                    sReturn += "&apos;";
                break;

                case '&':
                    if ((( i + 4 ) < rString.Len()) &&
                        ( rString.Copy( i, 5 ) == "&amp;" ))
                            sReturn += rString.GetChar( i );
                    else
                        sReturn += "&amp;";
                break;

                default:
                    sReturn += rString.GetChar( i );
                break;
            }
        }
    }
    rString = sReturn;
}

void Export::RemoveUTF8ByteOrderMarker( ByteString &rString ){
    if( hasUTF8ByteOrderMarker( rString ) )
        rString.Erase( 0 , 3 );
}

bool Export::hasUTF8ByteOrderMarker( const ByteString &rString ){
    // Byte order marker signature

    const unsigned char c1 =  0xEF;
    const unsigned char c2 =  0xBB;
    const unsigned char c3 =  0xBF;

    const char bom[ 3 ] = { c1 , c2 , c3 };

    return      rString.Len() >= 3 &&
                rString.GetChar( 0 ) == bom[ 0 ] &&
                rString.GetChar( 1 ) == bom[ 1 ] &&
                rString.GetChar( 2 ) == bom[ 2 ] ;
}
bool Export::fileHasUTF8ByteOrderMarker( const ByteString &rString ){
    SvFileStream aFileIn( String( rString , RTL_TEXTENCODING_ASCII_US ) , STREAM_READ );
    ByteString sLine;
    if( !aFileIn.IsEof() ) {
        aFileIn.ReadLine( sLine );
        if( aFileIn.IsOpen() ) aFileIn.Close();
        return hasUTF8ByteOrderMarker( sLine );
    }
    if( aFileIn.IsOpen() ) aFileIn.Close();
    return false;
}
void Export::RemoveUTF8ByteOrderMarkerFromFile( const ByteString &rFilename ){
    SvFileStream aFileIn( String( rFilename , RTL_TEXTENCODING_ASCII_US ) , STREAM_READ );
    ByteString sLine;
    if( !aFileIn.IsEof() ) {
        aFileIn.ReadLine( sLine );
        // Test header
        if( hasUTF8ByteOrderMarker( sLine ) ){
            DirEntry aTempFile = Export::GetTempFile();
            ByteString sTempFile = ByteString( aTempFile.GetFull() , RTL_TEXTENCODING_ASCII_US );
            SvFileStream aNewFile( String( sTempFile , RTL_TEXTENCODING_ASCII_US ) , STREAM_WRITE );
            // Remove header
            RemoveUTF8ByteOrderMarker( sLine );
            aNewFile.WriteLine( sLine );
            // Copy the rest
            while( !aFileIn.IsEof() ){
                aFileIn.ReadLine( sLine );
                aNewFile.WriteLine( sLine );
            }
            if( aFileIn.IsOpen() ) aFileIn.Close();
            if( aNewFile.IsOpen() ) aNewFile.Close();
            DirEntry aEntry( rFilename.GetBuffer() );
            aEntry.Kill();
            DirEntry( sTempFile ).MoveTo( DirEntry( rFilename.GetBuffer() ) );
        }
    }
    if( aFileIn.IsOpen() ) aFileIn.Close();
}

bool Export::CopyFile( const ByteString& source , const ByteString& dest )
{
    const int BUFFERSIZE    = 8192;
    char buf[ BUFFERSIZE ];

    FILE* IN_FILE = fopen( source.GetBuffer() , "r" );
    if( IN_FILE == NULL )
    {
        cerr << "Export::CopyFile WARNING: Could not open " << source.GetBuffer() << "\n";
        return false;
    }

    FILE* OUT_FILE = fopen( dest.GetBuffer() , "w" );
    if( OUT_FILE == NULL )
    {
        cerr << "Export::CopyFile WARNING: Could not open/create " << dest.GetBuffer() << " for writing\n";
        fclose( IN_FILE );
        return false;
    }

    while( fgets( buf , BUFFERSIZE , IN_FILE ) != NULL )
    {
        if( fputs( buf , OUT_FILE ) == EOF )
        {
            cerr << "Export::CopyFile WARNING: Write problems " << source.GetBuffer() << "\n";
            fclose( IN_FILE );
            fclose( OUT_FILE );
            return false;
        }
    }
    if( ferror( IN_FILE ) )
    {
        cerr << "Export::CopyFile WARNING: Read problems " << dest.GetBuffer() << "\n";
        fclose( IN_FILE );
        fclose( OUT_FILE );
        return false;
    }
    fclose ( IN_FILE );
    fclose ( OUT_FILE );

    return true;
}

/*****************************************************************************/
void Export::UnquotHTML( ByteString &rString )
/*****************************************************************************/
{
    ByteString sReturn;
    while ( rString.Len()) {
        if ( rString.Copy( 0, 5 ) == "&amp;" ) {
            sReturn += "&";
            rString.Erase( 0, 5 );
        }
        else if ( rString.Copy( 0, 4 ) == "&lt;" ) {
            sReturn += "<";
            rString.Erase( 0, 4 );
        }
        else if ( rString.Copy( 0, 4 ) == "&gt;" ) {
            sReturn += ">";
            rString.Erase( 0, 4 );
        }
        else if ( rString.Copy( 0, 6 ) == "&quot;" ) {
            sReturn += "\"";
            rString.Erase( 0, 6 );
        }
        else if ( rString.Copy( 0, 6 ) == "&apos;" ) {
            sReturn += "\'";
            rString.Erase( 0, 6 );
        }
        else {
            sReturn += rString.GetChar( 0 );
            rString.Erase( 0, 1 );
        }
    }
    rString = sReturn;
}
bool Export::isSourceLanguage( const ByteString &sLanguage )
{
    return !isAllowed( sLanguage );
}
bool Export::isAllowed( const ByteString &sLanguage ){
    return ! ( sLanguage.EqualsIgnoreCaseAscii("en-US") );
}
/*****************************************************************************/
bool Export::LanguageAllowed( const ByteString &nLanguage )
/*****************************************************************************/
{
    return std::find( aLanguages.begin() , aLanguages.end() , nLanguage ) != aLanguages.end();
}

bool Export::isInitialized = false;

/*****************************************************************************/
void Export::InitLanguages( bool bMergeMode ){
/*****************************************************************************/
    if( !isInitialized ){
        ByteString sTmp;
        ByteStringBoolHashMap aEnvLangs;
        for ( USHORT x = 0; x < sLanguages.GetTokenCount( ',' ); x++ ){
            sTmp = sLanguages.GetToken( x, ',' ).GetToken( 0, '=' );
            sTmp.EraseLeadingAndTrailingChars();
            if( bMergeMode && !isAllowed( sTmp ) ){}
            else if( !( (sTmp.GetChar(0)=='x' || sTmp.GetChar(0)=='X') && sTmp.GetChar(1)=='-' ) ){
                aLanguages.push_back( sTmp );
            }
        }
        InitForcedLanguages( bMergeMode );
        isInitialized = true;
    }
}
/*****************************************************************************/
void Export::InitForcedLanguages( bool bMergeMode ){
/*****************************************************************************/
    ByteString sTmp;
    ByteStringBoolHashMap aEnvLangs;
    for ( USHORT x = 0; x < sForcedLanguages.GetTokenCount( ',' ); x++ ){
        sTmp = sForcedLanguages.GetToken( x, ',' ).GetToken( 0, '=' );
        sTmp.EraseLeadingAndTrailingChars();
        if( bMergeMode && isAllowed( sTmp ) ){}
        else if( !( (sTmp.GetChar(0)=='x' || sTmp.GetChar(0)=='X') && sTmp.GetChar(1)=='-' ) )
            aForcedLanguages.push_back( sTmp );
    }
}

/*****************************************************************************/
ByteString Export::GetFallbackLanguage( const ByteString nLanguage )
/*****************************************************************************/
{
    ByteString sFallback=nLanguage;
    GetIsoFallback( sFallback );
    return sFallback;
}

void Export::replaceEncoding( ByteString& rString )
{
// &#x2122; -> \u2122

    for( xub_StrLen idx = 0; idx <= rString.Len()-8 ; idx++ )
    {
        if( rString.GetChar( idx )   == '&' &&
            rString.GetChar( idx+1 ) == '#' &&
            rString.GetChar( idx+2 ) == 'x' &&
            rString.GetChar( idx+7 ) == ';' )
        {
            ByteString sTmp = rString.Copy( 0 , idx );
            sTmp.Append( "\\u" );
            sTmp.Append( rString.GetChar( idx+3 ) );
            sTmp.Append( rString.GetChar( idx+4 ) );
            sTmp.Append( rString.GetChar( idx+5 ) );
            sTmp.Append( rString.GetChar( idx+6 ) );
            sTmp.Append( rString.Copy( idx+8 , rString.Len() ) );
            rString = sTmp;
         }
    }
}

/*****************************************************************************/
void Export::FillInFallbacks( ResData *pResData )
/*****************************************************************************/
{
    ByteString sCur;
    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];
        if( isAllowed( sCur )  ){
            ByteString nFallbackIndex = GetFallbackLanguage( sCur );
            if( nFallbackIndex.Len() ){
                if ( !pResData->sText[ sCur ].Len())
                    pResData->sText[ sCur ] =
                        pResData->sText[ nFallbackIndex ];

                if ( !pResData->sHelpText[ sCur ].Len())
                    pResData->sHelpText[ sCur ] =
                        pResData->sHelpText[ nFallbackIndex ];

                if ( !pResData->sQuickHelpText[ sCur ].Len())
                    pResData->sQuickHelpText[ sCur ] =
                        pResData->sQuickHelpText[ nFallbackIndex ];

                if ( !pResData->sTitle[ sCur ].Len())
                    pResData->sTitle[ sCur ] =
                        pResData->sTitle[ nFallbackIndex ];

                if ( pResData->pStringList )
                    FillInListFallbacks(
                        pResData->pStringList, sCur, nFallbackIndex );

                if ( pResData->pPairedList )
                    FillInListFallbacks(
                        pResData->pPairedList, sCur, nFallbackIndex );

                if ( pResData->pFilterList )
                    FillInListFallbacks(
                        pResData->pFilterList, sCur, nFallbackIndex );

                if ( pResData->pItemList )
                    FillInListFallbacks(
                        pResData->pItemList, sCur, nFallbackIndex );

                if ( pResData->pUIEntries )
                    FillInListFallbacks(
                        pResData->pUIEntries, sCur, nFallbackIndex );
            }
        }
    }
}

/*****************************************************************************/
void Export::FillInListFallbacks(
    ExportList *pList, const ByteString &nSource, const ByteString &nFallback )
/*****************************************************************************/
{

    for ( size_t i = 0; i < pList->size(); i++ ) {
        ExportListEntry *pEntry = (*pList)[ i ];
        if ( !( *pEntry )[ nSource ].Len()){
             ( *pEntry )[ nSource ] = ( *pEntry )[ nFallback ];
            ByteString x = ( *pEntry )[ nSource ];
            ByteString y = ( *pEntry )[ nFallback ];
        }
    }
}

/*****************************************************************************/
ByteString Export::GetTimeStamp()
/*****************************************************************************/
{
//  return "xx.xx.xx";
    char buf[20];
    Time aTime;

    snprintf(buf, sizeof(buf), "%8d %02d:%02d:%02d", int(Date().GetDate()),
        int(aTime.GetHour()), int(aTime.GetMin()), int(aTime.GetSec()));
    return ByteString(buf);
}

/*****************************************************************************/
BOOL Export::ConvertLineEnds(
    ByteString sSource, ByteString sDestination )
/*****************************************************************************/
{
    String sSourceFile( sSource, RTL_TEXTENCODING_ASCII_US );
    String sDestinationFile( sDestination, RTL_TEXTENCODING_ASCII_US );

    SvFileStream aSource( sSourceFile, STREAM_READ );
    if ( !aSource.IsOpen())
        return FALSE;
    SvFileStream aDestination( sDestinationFile, STREAM_STD_WRITE | STREAM_TRUNC );
    if ( !aDestination.IsOpen()) {
        aSource.Close();
        return FALSE;
    }

    ByteString sLine;

    while ( !aSource.IsEof()) {
        aSource.ReadLine( sLine );
        if ( !aSource.IsEof()) {
            sLine.EraseAllChars( '\r' );
            aDestination.WriteLine( sLine );
        }
        else
            aDestination.WriteByteString( sLine );
    }

    aSource.Close();
    aDestination.Close();

    return TRUE;
}

/*****************************************************************************/
ByteString Export::GetNativeFile( ByteString sSource )
/*****************************************************************************/
{
    DirEntry aTemp( GetTempFile());
    ByteString sReturn( aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US );

    for ( USHORT i = 0; i < 10; i++ )
        if ( ConvertLineEnds( sSource, sReturn ))
            return sReturn;

    return "";
}

const char* Export::GetEnv( const char *pVar )
{
        char *pRet = getenv( pVar );
        if ( !pRet )
            pRet = 0;
        return pRet;
}


int Export::getCurrentDirectory( rtl::OUString& base_fqurl_out, rtl::OUString& base_out )
{
    DirEntry aDir(".");
    aDir.ToAbs();
    base_out = rtl::OUString( aDir.GetFull() );
    return osl::File::getFileURLFromSystemPath( base_out , base_fqurl_out );
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

void Export::getRandomName( const ByteString& sPrefix , ByteString& sRandStr , const ByteString& sPostfix )
{
    static const char LETTERS[]        = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    static const int  COUNT_OF_LETTERS = SAL_N_ELEMENTS(LETTERS) - 1;
    sRandStr.Append( sPrefix );

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

    for (i = 0; i < RAND_NAME_LENGTH; i++)
    {
        buffer[i] = LETTERS[v % COUNT_OF_LETTERS];
        v        /= COUNT_OF_LETTERS;
    }

    sRandStr.Append( buffer , RAND_NAME_LENGTH );
    sRandStr.Append( sPostfix );
}

void Export::getRandomName( ByteString& sRandStr )
{
    const ByteString sEmpty;
    getRandomName( sEmpty , sRandStr , sEmpty );
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
    ByteString sTmp( sDecodedStr , RTL_TEXTENCODING_UTF8 );

#if defined(WNT) || defined(OS2)
    sTmp.SearchAndReplace("file:///","");
    sTmp.SearchAndReplaceAll('/','\\');
#else
    // Set file permission to 644
     const sal_uInt64 nPerm = osl_File_Attribute_OwnRead | osl_File_Attribute_OwnWrite |
                             osl_File_Attribute_GrpRead | osl_File_Attribute_OthRead ;

    nRC = osl::File::setAttributes( *sTempFilename , nPerm );
    if( nRC ) printf(" osl::File::setAttributes RC = %d",nRC);

    sTmp.SearchAndReplace("file://","");
#endif
    DirEntry aDirEntry( sTmp );
    delete sTempFilename;
    return aDirEntry;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
