/*************************************************************************
 *
 *  $RCSfile: mnemonic.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2000-10-29 17:21:28 $
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

ImplMnemonicGenerator::ImplMnemonicGenerator()
{
    memset( maMnemonics, 1, sizeof( maMnemonics ) );
}

// -----------------------------------------------------------------------

USHORT ImplMnemonicGenerator::ImplGetMnemonicIndex( sal_Unicode c )
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

sal_Unicode ImplMnemonicGenerator::ImplFindMnemonic( const XubString& rKey )
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

void ImplMnemonicGenerator::RegisterMnemonic( const XubString& rKey )
{
    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetLocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

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

BOOL ImplMnemonicGenerator::CreateMnemonic( XubString& rKey )
{
    if ( !rKey.Len() || ImplFindMnemonic( rKey ) )
        return FALSE;

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetLocale();
    uno::Reference < i18n::XCharacterClassification > xCharClass = GetCharClass();

    XubString aKey = xCharClass->toUpper( rKey, 0, rKey.Len(), rLocale );

    BOOL bChanged = FALSE;
    xub_StrLen nLen = aKey.Len();

    // 1) Anfangsbuchstaben werden bevorzugt
    USHORT          nMnemonicIndex;
    sal_Unicode     c;
    xub_StrLen      nIndex = 0;
    do
    {
        c = aKey.GetChar( nIndex );
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

    // 2) Eindeutiger/seltender Buchstabe enthalten?
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

    return bChanged;
}

uno::Reference< i18n::XCharacterClassification > ImplMnemonicGenerator::GetCharClass()
{
    if ( !xCharClass.is() )
        xCharClass = vcl::unohelper::CreateCharacterClassification();
    return xCharClass;
}


