/*************************************************************************
 *
 *  $RCSfile: export2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nf $ $Date: 2000-11-27 07:10:08 $
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
    HEBREW
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
    switch ( nLangId ) {
        case COMMENT: return RTL_TEXTENCODING_MS_1252;
        case ENGLISH_US: return RTL_TEXTENCODING_MS_1252;
        case PORTUGUESE: return RTL_TEXTENCODING_MS_1252;
        case RUSSIAN: return RTL_TEXTENCODING_MS_1251;
        case GREEK: return RTL_TEXTENCODING_MS_1253;
        case DUTCH: return RTL_TEXTENCODING_MS_1252;
        case FRENCH: return RTL_TEXTENCODING_MS_1252;
        case SPANISH: return RTL_TEXTENCODING_MS_1252;
        case FINNISH: return RTL_TEXTENCODING_MS_1252;
        case HUNGARIAN: return RTL_TEXTENCODING_MS_1250;
        case ITALIAN: return RTL_TEXTENCODING_MS_1252;
        case CZECH: return RTL_TEXTENCODING_MS_1250;
        case SLOVAK: return RTL_TEXTENCODING_MS_1250;
        case ENGLISH: return RTL_TEXTENCODING_MS_1252;
        case DANISH: return RTL_TEXTENCODING_MS_1252;
        case SWEDISH: return RTL_TEXTENCODING_MS_1252;
        case NORWEGIAN: return RTL_TEXTENCODING_MS_1252;
        case POLISH: return RTL_TEXTENCODING_MS_1250;
        case GERMAN: return RTL_TEXTENCODING_MS_1252;
        case PORTUGUESE_BRAZILIAN: return RTL_TEXTENCODING_MS_1252;
        case JAPANESE: return RTL_TEXTENCODING_MS_932;
        case KOREAN: return RTL_TEXTENCODING_MS_949;
        case CHINESE_SIMPLIFIED: return RTL_TEXTENCODING_MS_936;
        case CHINESE_TRADITIONAL: return RTL_TEXTENCODING_MS_950;
        case TURKISH: return RTL_TEXTENCODING_MS_1254;
        case ARABIC: return RTL_TEXTENCODING_MS_1256;
        case HEBREW: return RTL_TEXTENCODING_MS_1255;
    }
    return 0xFFFF;
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
    }
    return "";
}

/*****************************************************************************/
void Export::QuotHTML( ByteString &rString )
/*****************************************************************************/
{
    ByteString sReturn;
    for ( ULONG i = 0; i < rString.Len(); i++ ) {
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
    "hebrew"
};
