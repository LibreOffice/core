/*************************************************************************
 *
 *  $RCSfile: export2.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:51:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "export.hxx"
#include "utf8conv.hxx"
#include <tools/datetime.hxx>
#include <bootstrp/appdef.hxx>
#include <tools/isofallback.hxx>
#include <stdio.h>
#include <osl/file.hxx>
#include <rtl/ustring.hxx>
//
// class ResData();
//

/*****************************************************************************/
ResData::~ResData()
/*****************************************************************************/
{
    if ( pStringList ) {
        // delete existing res. of type StringList
        for ( ULONG i = 0; i < pStringList->Count(); i++ ) {
            ExportListEntry* test = pStringList->GetObject( i );
            if( test != NULL ) delete test;
        }
        delete pStringList;
    }
    if ( pFilterList ) {
        // delete existing res. of type FilterList
        for ( ULONG i = 0; i < pFilterList->Count(); i++ ) {
            ExportListEntry* test = pFilterList->GetObject( i );
            delete test;
        }
        delete pFilterList;
    }
    if ( pItemList ) {
        // delete existing res. of type ItemList
        for ( ULONG i = 0; i < pItemList->Count(); i++ ) {
            ExportListEntry* test = pItemList->GetObject( i );
            delete test;
        }
        delete pItemList;
    }
    if ( pUIEntries ) {
        // delete existing res. of type UIEntries
        for ( ULONG i = 0; i < pUIEntries->Count(); i++ ) {
            ExportListEntry* test = pUIEntries->GetObject( i );
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
ByteString Export::sIsoCode99;
/*****************************************************************************/


/*****************************************************************************/
USHORT Export::GetLangIndex( USHORT nLangId )
/*****************************************************************************/
{
    // removeme
    return 0xFFFF;
}

/*****************************************************************************/
CharSet Export::GetCharSet( USHORT nLangId )
/*****************************************************************************/
{
    // removeme
    //return Langcode2TextEncoding( nLangId );
    return 0;
}

/*****************************************************************************/
USHORT Export::GetLangByIsoLang( const ByteString &rIsoLang )
/*****************************************************************************/
{
    ByteString sLang( rIsoLang );
    sLang.ToUpperAscii();
    return 0xFFFF;
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

std::vector<ByteString> Export::aLanguages = std::vector<ByteString>();

/*****************************************************************************/
ByteString Export::GetIsoLangByIndex( USHORT nIndex )
/*****************************************************************************/
{
// remove me
    return "";
}

/*****************************************************************************/
void Export::QuotHTML( ByteString &rString )
/*****************************************************************************/
{
    ByteString sReturn;
    BOOL bBreak = FALSE;
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
    ByteString sTmp;
    ByteStringBoolHashMap aEnvLangs;
    for ( USHORT x = 0; x < sLanguages.GetTokenCount( ',' ); x++ ){
        sTmp = sLanguages.GetToken( x, ',' ).GetToken( 0, '=' );
        sTmp.EraseLeadingAndTrailingChars();
        if( bMergeMode && ( sTmp.EqualsIgnoreCaseAscii("de") || sTmp.EqualsIgnoreCaseAscii("en-US") )){}
        else if( !( (sTmp.GetChar(0)=='x' || sTmp.GetChar(0)=='X') && sTmp.GetChar(1)=='-' ) )
            aLanguages.push_back( sTmp );
    }
    isInitialized = true;
}


/*****************************************************************************/
ByteString Export::GetFallbackLanguage( const ByteString nLanguage )
/*****************************************************************************/
{
    ByteString sFallback=nLanguage;
    GetIsoFallback( sFallback );
    return sFallback;
}

/*****************************************************************************/
void Export::FillInFallbacks( ResData *pResData )
/*****************************************************************************/
{
    ByteString sCur;
    for( long int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];
        if( !sCur.EqualsIgnoreCaseAscii("de") && !sCur.EqualsIgnoreCaseAscii("en-US") ){
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

    for ( ULONG i = 0; i < pList->Count(); i++ ) {
        ExportListEntry *pEntry = pList->GetObject( i );
        if ( !( *pEntry )[ nSource ].Len())
             ( *pEntry )[ nSource ] = ( *pEntry )[ nFallback ];
    }
}

/*****************************************************************************/
ByteString Export::GetTimeStamp()
/*****************************************************************************/
{
//  return "xx.xx.xx";
    char buf[20];
    Time aTime;

    snprintf(buf, sizeof(buf), "%8d %02d:%02d:%02d", Date().GetDate(),
        aTime.GetHour(), aTime.GetMin(), aTime.GetSec());
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

/*****************************************************************************/
DirEntry Export::GetTempFile()
/*****************************************************************************/
{
#ifdef WNT
    String sTempDir( GetEnv( "TEMP" ), RTL_TEXTENCODING_ASCII_US );
#else
//  String sTempDir( GetEnv( "HOME" ), RTL_TEXTENCODING_ASCII_US );
    String sTempDir( String::CreateFromAscii( "/tmp" ));
#endif

    rtl::OUString* sTempFilename = new rtl::OUString();
    int nRC = osl::FileBase::createTempFile( 0 , 0 , sTempFilename );
    if( nRC ) printf(" osl::FileBase::createTempFile RC = %d",nRC);
    ByteString sTmp( sTempFilename->getStr() , RTL_TEXTENCODING_UTF8 );
#ifdef WNT
    sTmp.SearchAndReplace("file:///","");
    sTmp.SearchAndReplaceAll('/','\\');
#else
    sTmp.SearchAndReplace("file://","");
#endif
    DirEntry aDirEntry( sTmp );
    delete sTempFilename;
    return aDirEntry;
}
