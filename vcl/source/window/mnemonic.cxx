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

sal_Unicode MnemonicGenerator::ImplFindMnemonic( const OUString& rKey )
{
    sal_Int32 nIndex = 0;
    while ( (nIndex = rKey.indexOf( MNEMONIC_CHAR, nIndex )) != -1 )
    {
        sal_Unicode cMnemonic = rKey[ nIndex+1 ];
        if ( cMnemonic != MNEMONIC_CHAR )
            return cMnemonic;
        nIndex += 2;
    }

    return 0;
}

// -----------------------------------------------------------------------

void MnemonicGenerator::RegisterMnemonic( const OUString& rKey )
{
    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    // Don't crash even when we don't have access to i18n service
    if ( !xCharClass.is() )
        return;

    OUString aKey = xCharClass->toUpper( rKey, 0, rKey.getLength(), rLocale );

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
        sal_Int32 nIndex = 0;
        sal_Int32 nLen = aKey.getLength();
        while ( nIndex < nLen )
        {
            sal_Unicode c = aKey[ nIndex ];

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

OUString MnemonicGenerator::CreateMnemonic( const OUString& _rKey )
{
    if ( _rKey.isEmpty() || ImplFindMnemonic( _rKey ) )
        return _rKey;

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    // Don't crash even when we don't have access to i18n service
    if ( !xCharClass.is() )
        return _rKey;

    OUString aKey = xCharClass->toUpper( _rKey, 0, _rKey.getLength(), rLocale );

    sal_Bool bChanged = sal_False;
    sal_Int32 nLen = aKey.getLength();

    bool bCJK = MsLangId::isCJK(Application::GetSettings().GetUILanguageTag().getLanguageType());

    // #107889# in CJK versions ALL strings (even those that contain latin characters)
    // will get mnemonics in the form: xyz (M)
    // thus steps 1) and 2) are skipped for CJK locales

    // #110720#, avoid CJK-style mnemonics for latin-only strings that do not contain useful mnemonic chars
    if( bCJK )
    {
        bool bLatinOnly = true;
        bool bMnemonicIndexFound = false;
        sal_Unicode     c;
        sal_Int32       nIndex;

        for( nIndex=0; nIndex < nLen; nIndex++ )
        {
            c = aKey[ nIndex ];
            if ( ((c >= 0x3000) && (c <= 0xD7FF)) ||    // cjk
                 ((c >= 0xFF61) && (c <= 0xFFDC)) )     // halfwidth forms
            {
                bLatinOnly = false;
                break;
            }
            if( ImplGetMnemonicIndex( c ) != MNEMONIC_INDEX_NOTFOUND )
                bMnemonicIndexFound = true;
        }
        if( bLatinOnly && !bMnemonicIndexFound )
            return _rKey;
    }

    OUString        rKey(_rKey);
    int             nCJK = 0;
    sal_uInt16      nMnemonicIndex;
    sal_Unicode     c;
    sal_Int32       nIndex = 0;
    if( !bCJK )
    {
        // 1) first try the first character of a word
        do
        {
            c = aKey[ nIndex ];

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
                    rKey = rKey.replaceAt( nIndex, 0, OUString(MNEMONIC_CHAR) );
                    bChanged = sal_True;
                    break;
                }
            }

            // Search for next word
            do
            {
                nIndex++;
                c = aKey[ nIndex ];
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
            sal_Int32       nBestIndex = 0;
            nIndex = 0;
            do
            {
                c = aKey[ nIndex ];
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
                rKey = rKey.replaceAt( nBestIndex, 0, OUString(MNEMONIC_CHAR) );
                bChanged = sal_True;
            }
        }
    }
    else
        nCJK = 1;

    // 3) Add English Mnemonic for CJK Text
    if ( !bChanged && (nCJK == 1) && !rKey.isEmpty() )
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
                    OUString aStr = OUStringBuffer().
                        append('(').append(MNEMONIC_CHAR).append(c).
                        append(')').makeStringAndClear();
                    nIndex = rKey.getLength();
                    if( nIndex >= 2 )
                    {
                        if ( ( rKey[nIndex-2] == '>'    && rKey[nIndex-1] == '>' ) ||
                             ( rKey[nIndex-2] == 0xFF1E && rKey[nIndex-1] == 0xFF1E ) )
                            nIndex -= 2;
                    }
                    if( nIndex >= 3 )
                    {
                        if ( ( rKey[nIndex-3] == '.'    && rKey[nIndex-2] == '.'    && rKey[nIndex-1] == '.' ) ||
                             ( rKey[nIndex-3] == 0xFF0E && rKey[nIndex-2] == 0xFF0E && rKey[nIndex-1] == 0xFF0E ) )
                            nIndex -= 3;
                    }
                    if( nIndex >= 1)
                    {
                        sal_Unicode cLastChar = rKey[ nIndex-1 ];
                        if ( (cLastChar == ':') || (cLastChar == 0xFF1A) ||
                            (cLastChar == '.') || (cLastChar == 0xFF0E) ||
                            (cLastChar == '?') || (cLastChar == 0xFF1F) ||
                            (cLastChar == ' ') )
                            nIndex--;
                    }
                    rKey = rKey.replaceAt( nIndex, 0, aStr );
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

    return rKey;
}

// -----------------------------------------------------------------------

uno::Reference< i18n::XCharacterClassification > MnemonicGenerator::GetCharClass()
{
    if ( !mxCharClass.is() )
        mxCharClass = vcl::unohelper::CreateCharacterClassification();
    return mxCharClass;
}

// -----------------------------------------------------------------------

OUString MnemonicGenerator::EraseAllMnemonicChars( const OUString& rStr )
{
    OUString    aStr = rStr;
    sal_Int32   nLen = aStr.getLength();
    sal_Int32   i    = 0;

    while ( i < nLen )
    {
        if ( aStr[ i ] == '~' )
        {
            // check for CJK-style mnemonic
            if( i > 0 && (i+2) < nLen )
            {
                sal_Unicode c = aStr[i+1];
                if( aStr[ i-1 ] == '(' &&
                    aStr[ i+2 ] == ')' &&
                    c >= MNEMONIC_RANGE_2_START && c <= MNEMONIC_RANGE_2_END )
                {
                    aStr = aStr.replaceAt( i-1, 4, "" );
                    nLen -= 4;
                    i--;
                    continue;
                }
            }

            // remove standard mnemonics
            aStr = aStr.replaceAt( i, 1, "" );
            nLen--;
        }
        else
            i++;
    }

    return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
