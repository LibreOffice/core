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

#include "WinImplHelper.hxx"

#include <vector>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;

const OUString TILDE( "~" );
const sal_Unicode   TILDE_SIGN = L'~';
const OUString AMPERSAND( "&" );
const sal_Unicode   AMPERSAND_SIGN = L'&';

// OS NAME          Platform                    Major   Minor

// Windows NT 3.51  VER_PLATFORM_WIN32_NT       3       51
// Windows NT 4.0   VER_PLATFORM_WIN32_NT       4       0
// Windows 2000     VER_PLATFORM_WIN32_NT       5       0
// Windows XP       VER_PLATFORM_WIN32_NT       5       1
// Windows Vista    VER_PLATFORM_WIN32_NT       6       0
// Windows 7        VER_PLATFORM_WIN32_NT       6       1
// Windows 95       VER_PLATFORM_WIN32_WINDOWS  4       0
// Windows 98       VER_PLATFORM_WIN32_WINDOWS  4       10
// Windows ME       VER_PLATFORM_WIN32_WINDOWS  4       90


static void Replace( const OUString& aLabel, sal_Unicode OldChar, sal_Unicode NewChar, OUStringBuffer& aBuffer )
{
    OSL_ASSERT( aLabel.getLength( ) );
    OSL_ASSERT( aBuffer.getCapacity( ) >= (aLabel.getLength( )) );

    sal_Int32 i = 0;
    const sal_Unicode* pCurrent  = aLabel.getStr( );
    const sal_Unicode* pNext     = aLabel.getStr( ) + 1;
    const sal_Unicode* pEnd      = aLabel.getStr( ) + aLabel.getLength( );

    while( pCurrent < pEnd )
    {
        OSL_ASSERT( pNext <= pEnd );
        OSL_ASSERT( (i >= 0) && (i < aBuffer.getCapacity( )) );

        if ( OldChar == *pCurrent )
        {
            if ( OldChar == *pNext )
            {
                // two OldChars in line will
                // be replaced by one
                // e.g. ~~ -> ~
                aBuffer.insert( i, *pCurrent );

                // skip the next one
                pCurrent++;
                pNext++;
            }
            else
            {
                // one OldChar will be replace
                // by NexChar
                aBuffer.insert( i, NewChar );
            }
        }
        else if ( *pCurrent == NewChar )
        {
            // a NewChar will be replaced by
             // two NewChars
             // e.g. & -> &&
            aBuffer.insert( i++, *pCurrent );
            aBuffer.insert( i, *pCurrent );
        }
        else
        {
            aBuffer.insert( i, *pCurrent );
        }

        pCurrent++;
        pNext++;
        i++;
    }
}

// converts a soffice label to a windows label
// the following rules for character replacements
// will be done:
// '~'  -> '&'
// '~~' -> '~'
// '&'  -> '&&'

OUString SOfficeToWindowsLabel( const OUString& aSOLabel )
{
    OUString aWinLabel = aSOLabel;

    if ( (aWinLabel.indexOf( TILDE ) > -1) || (aWinLabel.indexOf( AMPERSAND ) > -1) )
    {
        sal_Int32 nStrLen = aWinLabel.getLength( );

        // in the worst case the new string is
        // doubled in length, maybe some waste
        // of memory but how long is a label
        // normally(?)
        OUStringBuffer aBuffer( nStrLen * 2 );

        Replace( aWinLabel, TILDE_SIGN, AMPERSAND_SIGN, aBuffer );

        aWinLabel = aBuffer.makeStringAndClear( );
    }

    return aWinLabel;
}

// converts a windows label to a soffice label
// the following rules for character replacements
// will be done:
// '&'  -> '~'
// '&&' -> '&'
// '~'  -> '~~'

OUString WindowsToSOfficeLabel( const OUString& aWinLabel )
{
    OUString aSOLabel = aWinLabel;

    if ( (aSOLabel.indexOf( TILDE ) > -1) || (aSOLabel.indexOf( AMPERSAND ) > -1) )
    {
        sal_Int32 nStrLen = aSOLabel.getLength( );

        // in the worst case the new string is
        // doubled in length, maybe some waste
        // of memory but how long is a label
        // normally(?)
        OUStringBuffer aBuffer( nStrLen * 2 );

        Replace( aSOLabel, AMPERSAND_SIGN, TILDE_SIGN, aBuffer );

        aSOLabel = aBuffer.makeStringAndClear( );
    }

    return aSOLabel;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
