/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <string.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/mnemonic.hxx>

#include <vcl/unohelp.hxx>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <i18nlangtag/mslangid.hxx>

using namespace ::com::sun::star;




MnemonicGenerator::MnemonicGenerator()
{
    memset( maMnemonics, 1, sizeof( maMnemonics ) );
}



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



void MnemonicGenerator::RegisterMnemonic( const OUString& rKey )
{
    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    
    if ( !xCharClass.is() )
        return;

    OUString aKey = xCharClass->toUpper( rKey, 0, rKey.getLength(), rLocale );

    
    
    
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



OUString MnemonicGenerator::CreateMnemonic( const OUString& _rKey )
{
    if ( _rKey.isEmpty() || ImplFindMnemonic( _rKey ) )
        return _rKey;

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    
    if ( !xCharClass.is() )
        return _rKey;

    OUString aKey = xCharClass->toUpper( _rKey, 0, _rKey.getLength(), rLocale );

    bool bChanged = false;
    sal_Int32 nLen = aKey.getLength();

    bool bCJK = MsLangId::isCJK(Application::GetSettings().GetUILanguageTag().getLanguageType());

    
    
    

    
    if( bCJK )
    {
        bool bLatinOnly = true;
        bool bMnemonicIndexFound = false;
        sal_Unicode     c;
        sal_Int32       nIndex;

        for( nIndex=0; nIndex < nLen; nIndex++ )
        {
            c = aKey[ nIndex ];
            if ( ((c >= 0x3000) && (c <= 0xD7FF)) ||    
                 ((c >= 0xFF61) && (c <= 0xFFDC)) )     
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
        
        do
        {
            c = aKey[ nIndex ];

            if ( nCJK != 2 )
            {
                if ( ((c >= 0x3000) && (c <= 0xD7FF)) ||    
                    ((c >= 0xFF61) && (c <= 0xFFDC)) )     
                    nCJK = 1;
                else if ( ((c >= 0x0030) && (c <= 0x0039)) || 
                        ((c >= 0x0041) && (c <= 0x005A)) || 
                        ((c >= 0x0061) && (c <= 0x007A)) || 
                        ((c >= 0x0370) && (c <= 0x037F)) || 
                        ((c >= 0x0400) && (c <= 0x04FF)) )  
                    nCJK = 2;
            }

            nMnemonicIndex = ImplGetMnemonicIndex( c );
            if ( nMnemonicIndex != MNEMONIC_INDEX_NOTFOUND )
            {
                if ( maMnemonics[nMnemonicIndex] )
                {
                    maMnemonics[nMnemonicIndex] = 0;
                    rKey = rKey.replaceAt( nIndex, 0, OUString(MNEMONIC_CHAR) );
                    bChanged = true;
                    break;
                }
            }

            
            nIndex++;
            while ( nIndex < nLen )
            {
                c = aKey[ nIndex ];
                if ( c == ' ' )
                    break;
                nIndex++;
            }
            nIndex++;
        }
        while ( nIndex < nLen );

        
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
                bChanged = true;
            }
        }
    }
    else
        nCJK = 1;

    
    if ( !bChanged && (nCJK == 1) && !rKey.isEmpty() )
    {
        
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
                    bChanged = true;
                    break;
                }
            }
        }
    }













//








//














    return rKey;
}



uno::Reference< i18n::XCharacterClassification > MnemonicGenerator::GetCharClass()
{
    if ( !mxCharClass.is() )
        mxCharClass = vcl::unohelper::CreateCharacterClassification();
    return mxCharClass;
}



OUString MnemonicGenerator::EraseAllMnemonicChars( const OUString& rStr )
{
    OUString    aStr = rStr;
    sal_Int32   nLen = aStr.getLength();
    sal_Int32   i    = 0;

    while ( i < nLen )
    {
        if ( aStr[ i ] == '~' )
        {
            
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

            
            aStr = aStr.replaceAt( i, 1, "" );
            nLen--;
        }
        else
            i++;
    }

    return aStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
