/*************************************************************************
 *
 *  $RCSfile: export2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:26 $
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
    for ( ULONG i = 0; i < LANGUAGES; i++ )
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
        case RUSSIAN: return RTL_TEXTENCODING_MS_1257;
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

