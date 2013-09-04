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

#include "svx/relfld.hxx"
#include "vcl/builder.hxx"

// -----------------------------------------------------------------------

SvxRelativeField::SvxRelativeField( Window* pParent, WinBits nBits) :
    MetricField( pParent, nBits)
{
    bNegativeEnabled = sal_False;
    bRelativeMode = sal_False;
    bRelative     = sal_False;

    SetDecimalDigits( 2 );
    SetMin( 0 );
    SetMax( 9999 );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxRelativeField(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxRelativeField(pParent, WB_BORDER | WB_SPIN | WB_REPEAT | WB_LEFT | WB_GROUP);
}
// -----------------------------------------------------------------------

void SvxRelativeField::Modify()
{
    MetricField::Modify();

    if ( bRelativeMode )
    {
        OUString  aStr = GetText();
        sal_Bool    bNewMode = bRelative;

        if ( bRelative )
        {
            const sal_Unicode* pStr = aStr.getStr();

            while ( *pStr )
            {
                if( ( ( *pStr < sal_Unicode( '0' ) ) || ( *pStr > sal_Unicode( '9' ) ) ) &&
                    ( *pStr != sal_Unicode( '%' ) ) )
                {
                    bNewMode = sal_False;
                    break;
                }
                pStr++;
            }
        }
        else
        {
            if ( aStr.indexOf( "%" ) != -1 )
                bNewMode = sal_True;
        }

        if ( bNewMode != bRelative )
            SetRelative( bNewMode );

        MetricField::Modify();
    }
}

// -----------------------------------------------------------------------

void SvxRelativeField::EnableRelativeMode( sal_uInt16 nMin,
                                           sal_uInt16 nMax, sal_uInt16 nStep )
{
    bRelativeMode = sal_True;
    nRelMin       = nMin;
    nRelMax       = nMax;
    nRelStep      = nStep;
    SetUnit( FUNIT_CM );
}

// -----------------------------------------------------------------------

void SvxRelativeField::SetRelative( sal_Bool bNewRelative )
{
    Selection aSelection = GetSelection();
    OUString aStr = GetText();

    if ( bNewRelative )
    {
        bRelative = sal_True;
        SetDecimalDigits( 0 );
        SetMin( nRelMin );
        SetMax( nRelMax );
        SetCustomUnitText( OUString('%') );
        SetUnit( FUNIT_CUSTOM );
    }
    else
    {
        bRelative = sal_False;
        SetDecimalDigits( 2 );
        SetMin( bNegativeEnabled ? -9999 : 0 );
        SetMax( 9999 );
        SetUnit( FUNIT_CM );
    }

    SetText( aStr );
    SetSelection( aSelection );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
