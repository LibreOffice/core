/*************************************************************************
 *
 *  $RCSfile: export2.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:10:46 $
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
#include <tools/l2txtenc.hxx>
#include <bootstrp/appdef.hxx>

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
            delete [] pStringList->GetObject( i );
        }
        delete pStringList;
    }
    if ( pFilterList ) {
        // delete existing res. of type FilterList
        for ( ULONG i = 0; i < pFilterList->Count(); i++ ) {
            delete [] pFilterList->GetObject( i );
        }
        delete pFilterList;
    }
    if ( pItemList ) {
        // delete existing res. of type ItemList
        for ( ULONG i = 0; i < pItemList->Count(); i++ ) {
            delete [] pItemList->GetObject( i );
        }
        delete pItemList;
    }
    if ( pUIEntries ) {
        // delete existing res. of type UIEntries
        for ( ULONG i = 0; i < pUIEntries->Count(); i++ ) {
            delete [] pUIEntries->GetObject( i );
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
USHORT Export::LangId[ LANGUAGES ] =
/*****************************************************************************/
{
    // translation table: Index <=> LangId
    COMMENT,
    ENGLISH_US,
    PORTUGUESE,
    GERMAN_DE,
    RUSSIAN,
    GREEK,
    DUTCH,
    FRENCH,
    SPANISH,
    FINNISH,
    HUNGARIAN,
    ITALIAN,
    CZECH,
    SLOVAK,
    ENGLISH,
    DANISH,
    SWEDISH,
    NORWEGIAN,
    POLISH,
    GERMAN,
    PORTUGUESE_BRAZILIAN,
    JAPANESE,
    KOREAN,
    CHINESE_SIMPLIFIED,
    CHINESE_TRADITIONAL,
    TURKISH,
    ARABIC,
    HEBREW,
    CATALAN,
    THAI,
    HINDI,
    ESTONIAN,
    EXTERN
};


/*****************************************************************************/
USHORT Export::GetLangIndex( USHORT nLangId )
/*****************************************************************************/
{
    for ( USHORT i = 0; i < LANGUAGES; i++ )
        if ( nLangId == LangId[ i ] )
            return i;
    return 0xFFFF;
}

/*****************************************************************************/
CharSet Export::GetCharSet( USHORT nLangId )
/*****************************************************************************/
{
    return Langcode2TextEncoding( nLangId );
}

/*****************************************************************************/
USHORT Export::GetLangByIsoLang( const ByteString &rIsoLang )
/*****************************************************************************/
{
    ByteString sLang( rIsoLang );

    sLang.ToUpperAscii();

    if ( sLang == ByteString( COMMENT_ISO ).ToUpperAscii())
        return COMMENT;
    else if ( sLang == ByteString( ENGLISH_US_ISO ).ToUpperAscii())
        return ENGLISH_US;
    else if ( sLang == ByteString( PORTUGUESE_ISO ).ToUpperAscii())
        return PORTUGUESE;
    else if ( sLang == ByteString( RUSSIAN_ISO ).ToUpperAscii())
        return RUSSIAN;
    else if ( sLang == ByteString( GREEK_ISO ).ToUpperAscii())
        return GREEK;
    else if ( sLang == ByteString( DUTCH_ISO ).ToUpperAscii())
        return DUTCH;
    else if ( sLang == ByteString( FRENCH_ISO ).ToUpperAscii())
        return FRENCH;
    else if ( sLang == ByteString( SPANISH_ISO ).ToUpperAscii())
        return SPANISH;
    else if ( sLang == ByteString( FINNISH_ISO ).ToUpperAscii())
        return FINNISH;
    else if ( sLang == ByteString( HUNGARIAN_ISO ).ToUpperAscii())
        return HUNGARIAN;
    else if ( sLang == ByteString( ITALIAN_ISO ).ToUpperAscii())
        return ITALIAN;
    else if ( sLang == ByteString( CZECH_ISO ).ToUpperAscii())
        return CZECH;
    else if ( sLang == ByteString( SLOVAK_ISO ).ToUpperAscii())
        return SLOVAK;
    else if ( sLang == ByteString( ENGLISH_ISO ).ToUpperAscii())
        return ENGLISH;
    else if ( sLang == ByteString( DANISH_ISO ).ToUpperAscii())
        return DANISH;
    else if ( sLang == ByteString( SWEDISH_ISO ).ToUpperAscii())
        return SWEDISH;
    else if ( sLang == ByteString( NORWEGIAN_ISO ).ToUpperAscii())
        return NORWEGIAN;
    else if ( sLang == ByteString( POLISH_ISO ).ToUpperAscii())
        return POLISH;
    else if ( sLang == ByteString( GERMAN_ISO ).ToUpperAscii())
        return GERMAN;
    else if ( sLang == ByteString( PORTUGUESE_BRAZILIAN_ISO ).ToUpperAscii())
        return PORTUGUESE_BRAZILIAN;
    else if ( sLang == ByteString( JAPANESE_ISO ).ToUpperAscii())
        return JAPANESE;
    else if ( sLang == ByteString( KOREAN_ISO ).ToUpperAscii())
        return KOREAN;
    else if ( sLang == ByteString( CHINESE_SIMPLIFIED_ISO ).ToUpperAscii())
        return CHINESE_SIMPLIFIED;
    else if ( sLang == ByteString( CHINESE_TRADITIONAL_ISO ).ToUpperAscii())
        return CHINESE_TRADITIONAL;
    else if ( sLang == ByteString( TURKISH_ISO ).ToUpperAscii())
        return TURKISH;
    else if ( sLang == ByteString( ARABIC_ISO ).ToUpperAscii())
        return ARABIC;
    else if ( sLang == ByteString( HEBREW_ISO ).ToUpperAscii())
        return HEBREW;
    else if ( sLang == ByteString( CATALAN_ISO ).ToUpperAscii())
        return CATALAN;
    else if ( sLang == ByteString( THAI_ISO ).ToUpperAscii())
        return THAI;
    else if ( sLang == ByteString( HINDI_ISO ).ToUpperAscii())
        return HINDI;
    else if ( sLang == ByteString( ESTONIAN_ISO ).ToUpperAscii())
        return ESTONIAN;
    else if ( sLang == ByteString( sIsoCode99 ).ToUpperAscii())
        return EXTERN;

    return 0xFFFF;
}

/*****************************************************************************/
ByteString Export::GetIsoLangByIndex( USHORT nIndex )
/*****************************************************************************/
{
    switch ( nIndex ) {
        case COMMENT_INDEX: return COMMENT_ISO;
        case ENGLISH_US_INDEX: return ENGLISH_US_ISO;
        case PORTUGUESE_INDEX: return PORTUGUESE_ISO;
        case RUSSIAN_INDEX: return RUSSIAN_ISO;
        case GREEK_INDEX: return GREEK_ISO;
        case DUTCH_INDEX: return DUTCH_ISO;
        case FRENCH_INDEX: return FRENCH_ISO;
        case SPANISH_INDEX: return SPANISH_ISO;
        case FINNISH_INDEX: return FINNISH_ISO;
        case HUNGARIAN_INDEX: return HUNGARIAN_ISO;
        case ITALIAN_INDEX: return ITALIAN_ISO;
        case CZECH_INDEX: return CZECH_ISO;
        case SLOVAK_INDEX: return SLOVAK_ISO;
        case ENGLISH_INDEX: return ENGLISH_ISO;
        case DANISH_INDEX: return DANISH_ISO;
        case SWEDISH_INDEX: return SWEDISH_ISO;
        case NORWEGIAN_INDEX: return NORWEGIAN_ISO;
        case POLISH_INDEX: return POLISH_ISO;
        case GERMAN_INDEX: return GERMAN_ISO;
        case PORTUGUESE_BRAZILIAN_INDEX: return PORTUGUESE_BRAZILIAN_ISO;
        case JAPANESE_INDEX: return JAPANESE_ISO;
        case KOREAN_INDEX: return KOREAN_ISO;
        case CHINESE_SIMPLIFIED_INDEX: return CHINESE_SIMPLIFIED_ISO;
        case CHINESE_TRADITIONAL_INDEX: return CHINESE_TRADITIONAL_ISO;
        case TURKISH_INDEX: return TURKISH_ISO;
        case ARABIC_INDEX: return ARABIC_ISO;
        case HEBREW_INDEX: return HEBREW_ISO;
        case CATALAN_INDEX: return CATALAN_ISO;
        case THAI_INDEX: return THAI_ISO;
        case HINDI_INDEX: return HINDI_ISO;
        case ESTONIAN_INDEX: return ESTONIAN_ISO;
        case EXTERN_INDEX: return sIsoCode99;
    }
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
const ByteString Export::LangName[ LANGUAGES ] =
/*****************************************************************************/
{
    "language_user1",
    "english_us",
    "portuguese",
    "german_de",
    "russian",
    "greek",
    "dutch",
    "french",
    "spanish",
    "finnish",
    "hungarian",
    "italian",
    "czech",
    "slovak",
    "english",
    "danish",
    "swedish",
    "norwegian",
    "polish",
    "german",
    "portuguese_brazilian",
    "japanese",
    "korean",
    "chinese_simplified",
    "chinese_traditional",
    "turkish",
    "arabic",
    "hebrew",
    "catalan",
    "thai",
    "hindi",
    "estonian",
    "extern"
};

/*****************************************************************************/
BOOL Export::LanguageAllowed( USHORT nLanguage )
/*****************************************************************************/
{
    if ( !sLanguages.Len() && ( nLanguage != 99 ))
        return TRUE;

    for ( USHORT i = 0; i < sLanguages.GetTokenCount( ',' ); i++ )
        if ( nLanguage ==
            sLanguages.GetToken( i, ',' ).GetToken( 0, '=' ).ToInt32())
            return TRUE;

    return FALSE;
}

/*****************************************************************************/
USHORT Export::GetFallbackLanguage( USHORT nLanguage )
/*****************************************************************************/
{
    for ( USHORT i = 0; i < sLanguages.GetTokenCount( ',' ); i++ )
        if ( nLanguage ==
            sLanguages.GetToken( i, ',' ).GetToken( 0, '=' ).ToInt32())
        {
            if ( sLanguages.GetToken( i, ',' ).GetTokenCount( '=' ) > 1 )
                return
                    (USHORT)( sLanguages.GetToken( i, ',' ).GetToken( 1, '=' ).ToInt32());
            else
                return nLanguage;
        }

    return nLanguage;
}

/*****************************************************************************/
void Export::FillInFallbacks( ResData *pResData )
/*****************************************************************************/
{
    for ( USHORT i = 0; i < LANGUAGES; i++ ) {
        if (( i != GERMAN_INDEX ) && ( i != ENGLISH_INDEX )) {
            USHORT nFallbackIndex =
                GetLangIndex( GetFallbackLanguage( LangId[ i ] ));
            if (( nFallbackIndex < LANGUAGES)  && ( i != nFallbackIndex )) {
                CharSet eSource =
                    Export::GetCharSet( Export::LangId[ nFallbackIndex ] );
                CharSet eDest =
                    Export::GetCharSet( Export::LangId[ i ] );

                if ( !pResData->sText[ i ].Len())
                    pResData->sText[ i ] = UTF8Converter::ConvertFromUTF8(
                        UTF8Converter::ConvertToUTF8(
                        pResData->sText[ nFallbackIndex ], eSource ), eDest );
                if ( !pResData->sHelpText[ i ].Len())
                    pResData->sHelpText[ i ] = UTF8Converter::ConvertFromUTF8(
                        UTF8Converter::ConvertToUTF8(
                        pResData->sHelpText[ nFallbackIndex ], eSource ), eDest );
                if ( !pResData->sQuickHelpText[ i ].Len())
                    pResData->sQuickHelpText[ i ] = UTF8Converter::ConvertFromUTF8(
                        UTF8Converter::ConvertToUTF8(
                        pResData->sQuickHelpText[ nFallbackIndex ], eSource ), eDest );
                if ( !pResData->sTitle[ i ].Len())
                    pResData->sTitle[ i ] = UTF8Converter::ConvertFromUTF8(
                        UTF8Converter::ConvertToUTF8(
                        pResData->sTitle[ nFallbackIndex ], eSource ), eDest );

                if ( pResData->pStringList )
                    FillInListFallbacks(
                        pResData->pStringList, i, nFallbackIndex );
                if ( pResData->pFilterList )
                    FillInListFallbacks(
                        pResData->pFilterList, i, nFallbackIndex );
                if ( pResData->pItemList )
                    FillInListFallbacks(
                        pResData->pItemList, i, nFallbackIndex );
                if ( pResData->pUIEntries )
                    FillInListFallbacks(
                        pResData->pUIEntries, i, nFallbackIndex );
            }
        }
    }
}

/*****************************************************************************/
void Export::FillInListFallbacks(
    ExportList *pList, USHORT nSource, USHORT nFallback )
/*****************************************************************************/
{
    CharSet eSource =
        Export::GetCharSet( Export::LangId[ nFallback ] );
    CharSet eDest =
        Export::GetCharSet( Export::LangId[ nSource ] );

    for ( ULONG i = 0; i < pList->Count(); i++ ) {
        ExportListEntry *pEntry = pList->GetObject( i );
        if ( !( *pEntry )[ nSource ].Len())
             ( *pEntry )[ nSource ] = UTF8Converter::ConvertFromUTF8(
                    UTF8Converter::ConvertToUTF8(
                    ( *pEntry )[ nFallback ], eSource ), eDest );
    }
}

/*****************************************************************************/
ByteString Export::GetTimeStamp()
/*****************************************************************************/
{
//  return "xx.xx.xx";

       Time aTime;
    ByteString sTimeStamp( ByteString::CreateFromInt64( Date().GetDate()));
    sTimeStamp += " ";
    sTimeStamp += ByteString::CreateFromInt32( aTime.GetHour());
    sTimeStamp += ":";
    sTimeStamp += ByteString::CreateFromInt32( aTime.GetMin());
    sTimeStamp += ":";
    sTimeStamp += ByteString::CreateFromInt32( aTime.GetSec());
    return sTimeStamp;
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
    DirEntry aTemp( sTempDir );
    return aTemp.TempName();
}
