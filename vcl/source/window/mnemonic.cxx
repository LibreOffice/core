/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <string.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/mnemonic.hxx>

#include <vcl/unohelp.hxx>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <i18nlangtag/mslangid.hxx>

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
    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
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

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    // Don't crash even when we don't have access to i18n service
    if ( !xCharClass.is() )
        return sal_False;

    XubString aKey = xCharClass->toUpper( rKey, 0, rKey.Len(), rLocale );

    sal_Bool bChanged = sal_False;
    xub_StrLen nLen = aKey.Len();

    bool bCJK = MsLangId::isCJK(Application::GetSettings().GetUILanguageTag().getLanguageType());

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
                    rtl::OUString aStr = rtl::OUStringBuffer().
                        append('(').append(MNEMONIC_CHAR).append(c).
                        append(')').makeStringAndClear();
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
