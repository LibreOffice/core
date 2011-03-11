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
#include "precompiled_vcl.hxx"

#include <string.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/mnemonic.hxx>

#include <vcl/unohelp.hxx>
#include <com/sun/star/i18n/XCharacterClassification.hpp>

using namespace ::com::sun::star;


// =======================================================================

MnemonicGenerator::MnemonicGenerator()
{
    memset( maMnemonics, 1, sizeof( maMnemonics ) );
}

// -----------------------------------------------------------------------

sal_uInt16 MnemonicGenerator::ImplGetMnemonicIndex( sal_Unicode c )
{
    static sal_uInt16 const aImplMnemonicRangeTab[MNEMONIC_RANGES*2] =
    {
        MNEMONIC_RANGE_1_START, MNEMONIC_RANGE_1_END,
        MNEMONIC_RANGE_2_START, MNEMONIC_RANGE_2_END,
        MNEMONIC_RANGE_3_START, MNEMONIC_RANGE_3_END,
        MNEMONIC_RANGE_4_START, MNEMONIC_RANGE_4_END
    };

    sal_uInt16 nMnemonicIndex = 0;
    for ( sal_uInt16 i = 0; i < MNEMONIC_RANGES; i++ )
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
        sal_uInt16 nMnemonicIndex = ImplGetMnemonicIndex( cMnemonic );
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

            sal_uInt16 nMnemonicIndex = ImplGetMnemonicIndex( c );
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

sal_Bool MnemonicGenerator::CreateMnemonic( XubString& rKey )
{
    if ( !rKey.Len() || ImplFindMnemonic( rKey ) )
        return sal_False;

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    // Don't crash even when we don't have access to i18n service
    if ( !xCharClass.is() )
        return sal_False;

    XubString aKey = xCharClass->toUpper( rKey, 0, rKey.Len(), rLocale );

    sal_Bool bChanged = sal_False;
    xub_StrLen nLen = aKey.Len();

    sal_Bool bCJK = sal_False;
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
            bCJK = sal_True;
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
        sal_Bool bLatinOnly = sal_True;
        sal_Bool bMnemonicIndexFound = sal_False;
        sal_Unicode     c;
        xub_StrLen      nIndex;

        for( nIndex=0; nIndex < nLen; nIndex++ )
        {
            c = aKey.GetChar( nIndex );
            if ( ((c >= 0x3000) && (c <= 0xD7FF)) ||    // cjk
                 ((c >= 0xFF61) && (c <= 0xFFDC)) )     // halfwidth forms
            {
                bLatinOnly = sal_False;
                break;
            }
            if( ImplGetMnemonicIndex( c ) != MNEMONIC_INDEX_NOTFOUND )
                bMnemonicIndexFound = sal_True;
        }
        if( bLatinOnly && !bMnemonicIndexFound )
            return sal_False;
    }


    int             nCJK = 0;
    sal_uInt16          nMnemonicIndex;
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
                    bChanged = sal_True;
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
            sal_uInt16      nBestCount = 0xFFFF;
            sal_uInt16      nBestMnemonicIndex = 0;
            xub_StrLen  nBestIndex = 0;
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
                bChanged = sal_True;
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
                    bChanged = sal_True;
                    break;
                }
            }
        }
    }

// #i87415# Duplicates mnemonics are bad for consistent keyboard accessibility
// It's probably better to not have mnemonics for some widgets, than to have ambiguous ones.
//    if( ! bChanged )
//    {
//        /*
//         *  #97809# if all else fails use the first character of a word
//         *  anyway and live with duplicate mnemonics
//         */
//        nIndex = 0;
//        do
//        {
//            c = aKey.GetChar( nIndex );
//
//            nMnemonicIndex = ImplGetMnemonicIndex( c );
//            if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
//            {
//                maMnemonics[nMnemonicIndex] = 0;
//                rKey.Insert( MNEMONIC_CHAR, nIndex );
//                bChanged = sal_True;
//                break;
//            }
//
//            // Search for next word
//            do
//            {
//                nIndex++;
//                c = aKey.GetChar( nIndex );
//                if ( c == ' ' )
//                    break;
//            }
//            while ( nIndex < nLen );
//            nIndex++;
//        }
//        while ( nIndex < nLen );
//    }

    return bChanged;
}

// -----------------------------------------------------------------------

uno::Reference< i18n::XCharacterClassification > MnemonicGenerator::GetCharClass()
{
    if ( !mxCharClass.is() )
        mxCharClass = vcl::unohelper::CreateCharacterClassification();
    return mxCharClass;
}

// -----------------------------------------------------------------------

String MnemonicGenerator::EraseAllMnemonicChars( const String& rStr )
{
    String      aStr = rStr;
    xub_StrLen  nLen = aStr.Len();
    xub_StrLen  i    = 0;

    while ( i < nLen )
    {
        if ( aStr.GetChar( i ) == '~' )
        {
            // check for CJK-style mnemonic
            if( i > 0 && (i+2) < nLen )
            {
                sal_Unicode c = aStr.GetChar(i+1);
                if( aStr.GetChar( i-1 ) == '(' &&
                    aStr.GetChar( i+2 ) == ')' &&
                    c >= MNEMONIC_RANGE_2_START && c <= MNEMONIC_RANGE_2_END )
                {
                    aStr.Erase( i-1, 4 );
                    nLen -= 4;
                    i--;
                    continue;
                }
            }

            // remove standard mnemonics
            aStr.Erase( i, 1 );
            nLen--;
        }
        else
            i++;
    }

    return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
