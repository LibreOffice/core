/*************************************************************************
 *
 *  $RCSfile: mnemonic.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:54:30 $
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

#define _SV_MNEMONIC_CXX

#include <string.h>
#include <svapp.hxx>
#include <settings.hxx>
#include <mnemonic.hxx>

#include <unohelp.hxx>

#ifndef _COM_SUN_STAR_I18N_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#endif

using namespace ::com::sun::star;


// =======================================================================

MnemonicGenerator::MnemonicGenerator()
{
    memset( maMnemonics, 1, sizeof( maMnemonics ) );
}

// -----------------------------------------------------------------------

USHORT MnemonicGenerator::ImplGetMnemonicIndex( sal_Unicode c )
{
    static USHORT const aImplMnemonicRangeTab[MNEMONIC_RANGES*2] =
    {
        MNEMONIC_RANGE_1_START, MNEMONIC_RANGE_1_END,
        MNEMONIC_RANGE_2_START, MNEMONIC_RANGE_2_END,
        MNEMONIC_RANGE_3_START, MNEMONIC_RANGE_3_END,
        MNEMONIC_RANGE_4_START, MNEMONIC_RANGE_4_END
    };

    USHORT nMnemonicIndex = 0;
    for ( USHORT i = 0; i < MNEMONIC_RANGES; i++ )
    {
        if ( (c >= aImplMnemonicRangeTab[i*2]) &&
             (c <= aImplMnemonicRangeTab[i*2+1]) )
            return nMnemonicIndex+c-aImplMnemonicRangeTab[i*2];

        nMnemonicIndex += aImplMnemonicRangeTab[i*2+1]-aImplMnemonicRangeTab[i*2];
    }

    return MNEMONIC_INDEX_NOTFOUND;
}

// -----------------------------------------------------------------------

sal_Unicode MnemonicGenerator::ImplFindMnemonic( const XubString& rKey )
{
    xub_StrLen nIndex = 0;
    while ( (nIndex = rKey.Search( MNEMONIC_CHAR, nIndex )) != STRING_NOTFOUND )
    {
        sal_Unicode cMnemonic = rKey.GetChar( nIndex+1 );
        if ( cMnemonic != MNEMONIC_CHAR )
            return cMnemonic;
        nIndex += 2;
    }

    return 0;
}

// -----------------------------------------------------------------------

void MnemonicGenerator::RegisterMnemonic( const XubString& rKey )
{
    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    // Don't crash even when we don't have access to i18n service
    if ( !xCharClass.is() )
        return;

    XubString aKey = xCharClass->toUpper( rKey, 0, rKey.Len(), rLocale );

    // If we find a Mnemonic, set the flag. In other case count the
    // characters, because we need this to set most as possible
    // Mnemonics
    sal_Unicode cMnemonic = ImplFindMnemonic( aKey );
    if ( cMnemonic )
    {
        USHORT nMnemonicIndex = ImplGetMnemonicIndex( cMnemonic );
        if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
            maMnemonics[nMnemonicIndex] = 0;
    }
    else
    {
        xub_StrLen nIndex = 0;
        xub_StrLen nLen = aKey.Len();
        while ( nIndex < nLen )
        {
            sal_Unicode c = aKey.GetChar( nIndex );

            USHORT nMnemonicIndex = ImplGetMnemonicIndex( c );
            if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
            {
                if ( maMnemonics[nMnemonicIndex] && (maMnemonics[nMnemonicIndex] < 0xFF) )
                    maMnemonics[nMnemonicIndex]++;
            }

            nIndex++;
        }
    }
}

// -----------------------------------------------------------------------

BOOL MnemonicGenerator::CreateMnemonic( XubString& rKey )
{
    if ( !rKey.Len() || ImplFindMnemonic( rKey ) )
        return FALSE;

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    XubString aKey = xCharClass->toUpper( rKey, 0, rKey.Len(), rLocale );

    BOOL bChanged = FALSE;
    xub_StrLen nLen = aKey.Len();

    BOOL bCJK = FALSE;
    switch( Application::GetSettings().GetUILanguage() )
    {
        case LANGUAGE_JAPANESE:
        case LANGUAGE_CHINESE_TRADITIONAL:
        case LANGUAGE_CHINESE_SIMPLIFIED:
        case LANGUAGE_CHINESE_HONGKONG:
        case LANGUAGE_CHINESE_SINGAPORE:
        case LANGUAGE_CHINESE_MACAU:
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
            bCJK = TRUE;
            break;
        default:
            break;
    }
    // #107889# in CJK versions ALL strings (even those that contain latin characters)
    // will get mnemonics in the form: xyz (M)
    // thus steps 1) and 2) are skipped for CJK locales

    // #110720#, avoid CJK-style mnemonics for latin-only strings that do not contain useful mnemonic chars
    if( bCJK )
    {
        BOOL bLatinOnly = TRUE;
        BOOL bMnemonicIndexFound = FALSE;
        sal_Unicode     c;
        xub_StrLen      nIndex;

        for( nIndex=0; nIndex < nLen; nIndex++ )
        {
            c = aKey.GetChar( nIndex );
            if ( ((c >= 0x3000) && (c <= 0xD7FF)) ||    // cjk
                 ((c >= 0xFF61) && (c <= 0xFFDC)) )     // halfwidth forms
            {
                bLatinOnly = FALSE;
                break;
            }
            if( ImplGetMnemonicIndex( c ) != MNEMONIC_INDEX_NOTFOUND )
                bMnemonicIndexFound = TRUE;
        }
        if( bLatinOnly && !bMnemonicIndexFound )
            return FALSE;
    }


    int             nCJK = 0;
    USHORT          nMnemonicIndex;
    sal_Unicode     c;
    xub_StrLen      nIndex = 0;
    if( !bCJK )
    {
        // 1) first try the first character of a word
        do
        {
            c = aKey.GetChar( nIndex );

            if ( nCJK != 2 )
            {
                if ( ((c >= 0x3000) && (c <= 0xD7FF)) ||    // cjk
                    ((c >= 0xFF61) && (c <= 0xFFDC)) )     // halfwidth forms
                    nCJK = 1;
                else if ( ((c >= 0x0030) && (c <= 0x0039)) || // digits
                        ((c >= 0x0041) && (c <= 0x005A)) || // latin capitals
                        ((c >= 0x0061) && (c <= 0x007A)) || // latin small
                        ((c >= 0x0370) && (c <= 0x037F)) || // greek numeral signs
                        ((c >= 0x0400) && (c <= 0x04FF)) )  // cyrillic
                    nCJK = 2;
            }

            nMnemonicIndex = ImplGetMnemonicIndex( c );
            if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
            {
                if ( maMnemonics[nMnemonicIndex] )
                {
                    maMnemonics[nMnemonicIndex] = 0;
                    rKey.Insert( MNEMONIC_CHAR, nIndex );
                    bChanged = TRUE;
                    break;
                }
            }

            // Search for next word
            do
            {
                nIndex++;
                c = aKey.GetChar( nIndex );
                if ( c == ' ' )
                    break;
            }
            while ( nIndex < nLen );
            nIndex++;
        }
        while ( nIndex < nLen );

        // 2) search for a unique/uncommon character
        if ( !bChanged )
        {
            USHORT      nBestCount = 0xFFFF;
            USHORT      nBestMnemonicIndex;
            xub_StrLen  nBestIndex;
            nIndex = 0;
            do
            {
                c = aKey.GetChar( nIndex );
                nMnemonicIndex = ImplGetMnemonicIndex( c );
                if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
                {
                    if ( maMnemonics[nMnemonicIndex] )
                    {
                        if ( maMnemonics[nMnemonicIndex] < nBestCount )
                        {
                            nBestCount = maMnemonics[nMnemonicIndex];
                            nBestIndex = nIndex;
                            nBestMnemonicIndex = nMnemonicIndex;
                            if ( nBestCount == 2 )
                                break;
                        }
                    }
                }

                nIndex++;
            }
            while ( nIndex < nLen );

            if ( nBestCount != 0xFFFF )
            {
                maMnemonics[nBestMnemonicIndex] = 0;
                rKey.Insert( MNEMONIC_CHAR, nBestIndex );
                bChanged = TRUE;
            }
        }
    }
    else
        nCJK = 1;

    // 3) Add English Mnemonic for CJK Text
    if ( !bChanged && (nCJK == 1) && rKey.Len() )
    {
        // Append Ascii Mnemonic
        for ( c = MNEMONIC_RANGE_2_START; c <= MNEMONIC_RANGE_2_END; c++ )
        {
            nMnemonicIndex = ImplGetMnemonicIndex( c );
            if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
            {
                if ( maMnemonics[nMnemonicIndex] )
                {
                    maMnemonics[nMnemonicIndex] = 0;
                    UniString aStr( '(' );
                    aStr += MNEMONIC_CHAR;
                    aStr += c;
                    aStr += ')';
                    nIndex = rKey.Len();
                    if( nIndex >= 2 )
                    {
                        static sal_Unicode cGreaterGreater[] = { 0xFF1E, 0xFF1E };
                        if ( rKey.EqualsAscii( ">>", nIndex-2, 2 ) ||
                            rKey.Equals( cGreaterGreater, nIndex-2, 2 ) )
                            nIndex -= 2;
                    }
                    if( nIndex >= 3 )
                    {
                        static sal_Unicode cDotDotDot[] = { 0xFF0E, 0xFF0E, 0xFF0E };
                        if ( rKey.EqualsAscii( "...", nIndex-3, 3 ) ||
                            rKey.Equals( cDotDotDot, nIndex-3, 3 ) )
                            nIndex -= 3;
                    }
                    if( nIndex >= 1)
                    {
                        sal_Unicode cLastChar = rKey.GetChar( nIndex-1 );
                        if ( (cLastChar == ':') || (cLastChar == 0xFF1A) ||
                            (cLastChar == '.') || (cLastChar == 0xFF0E) ||
                            (cLastChar == '?') || (cLastChar == 0xFF1F) ||
                            (cLastChar == ' ') )
                            nIndex--;
                    }
                    rKey.Insert( aStr, nIndex );
                    bChanged = TRUE;
                    break;
                }
            }
        }
    }

    if( ! bChanged )
    {
        /*
         *  #97809# if all else fails use the first character of a word
         *  anyway and live with duplicate mnemonics
         */
        nIndex = 0;
        do
        {
            c = aKey.GetChar( nIndex );

            nMnemonicIndex = ImplGetMnemonicIndex( c );
            if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
            {
                maMnemonics[nMnemonicIndex] = 0;
                rKey.Insert( MNEMONIC_CHAR, nIndex );
                bChanged = TRUE;
                break;
            }

            // Search for next word
            do
            {
                nIndex++;
                c = aKey.GetChar( nIndex );
                if ( c == ' ' )
                    break;
            }
            while ( nIndex < nLen );
            nIndex++;
        }
        while ( nIndex < nLen );
    }

    return bChanged;
}

// -----------------------------------------------------------------------

uno::Reference< i18n::XCharacterClassification > MnemonicGenerator::GetCharClass()
{
    if ( !xCharClass.is() )
        xCharClass = vcl::unohelper::CreateCharacterClassification();
    return xCharClass;
}
