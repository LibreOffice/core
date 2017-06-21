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
#include <vcl/builderfactory.hxx>

SvxRelativeField::SvxRelativeField(
        vcl::Window *const pParent, WinBits const nBits, FieldUnit const eUnit)
    : MetricField( pParent, nBits)
    , nRelMin(0)
    , nRelMax(0)
    , bRelativeMode(false)
    , bRelative(false)
    , bNegativeEnabled(false)

{
    SetUnit(eUnit);
    SetDecimalDigits( 2 );
    SetMin( 0 );
    SetMax( 9999 );
}

VCL_BUILDER_DECL_FACTORY(SvxRelativeField)
{
    OUString const custom(VclBuilder::extractCustomProperty(rMap));
    FieldUnit const eUnit(VclBuilder::detectUnit(custom));
    rRet = VclPtr<SvxRelativeField>::Create(pParent,
                                            WB_BORDER | WB_SPIN | WB_REPEAT |
                                            WB_LEFT | WB_GROUP,
                                            eUnit);
}

void SvxRelativeField::Modify()
{
    MetricField::Modify();

    if ( bRelativeMode )
    {
        OUString  aStr = GetText();
        bool      bNewMode = bRelative;

        if ( bRelative )
        {
            const sal_Unicode* pStr = aStr.getStr();

            while ( *pStr )
            {
                if( ( ( *pStr < '0' ) || ( *pStr > '9' ) ) &&
                    ( *pStr != '%' ) )
                {
                    bNewMode = false;
                    break;
                }
                pStr++;
            }
        }
        else
        {
            if ( aStr.indexOf( "%" ) != -1 )
                bNewMode = true;
        }

        if ( bNewMode != bRelative )
            SetRelative( bNewMode );

        MetricField::Modify();
    }
}


void SvxRelativeField::EnableRelativeMode( sal_uInt16 nMin, sal_uInt16 nMax )
{
    bRelativeMode = true;
    nRelMin       = nMin;
    nRelMax       = nMax;
    SetUnit( FUNIT_CM );
}


void SvxRelativeField::SetRelative( bool bNewRelative )
{
    Selection aSelection = GetSelection();
    OUString aStr = GetText();

    if ( bNewRelative )
    {
        bRelative = true;
        SetDecimalDigits( 0 );
        SetMin( nRelMin );
        SetMax( nRelMax );
        SetUnit( FUNIT_PERCENT );
    }
    else
    {
        bRelative = false;
        SetDecimalDigits( 2 );
        SetMin( bNegativeEnabled ? -9999 : 0 );
        SetMax( 9999 );
        SetUnit( FUNIT_CM );
    }

    SetText( aStr );
    SetSelection( aSelection );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
