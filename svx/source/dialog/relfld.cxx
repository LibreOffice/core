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

#include <svx/relfld.hxx>
#include <vcl/builderfactory.hxx>

SvxRelativeField::SvxRelativeField(
        vcl::Window *const pParent, WinBits const nBits, FieldUnit const eUnit)
    : MetricField( pParent, nBits)
    , bRelative(false)
{
    SetUnit(eUnit);
    SetDecimalDigits( 2 );
    SetMin( 0 );
    SetMax( 9999 );
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSvxRelativeField(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    OUString const custom(BuilderUtils::extractCustomProperty(rMap));
    FieldUnit const eUnit(BuilderUtils::detectUnit(custom));
    rRet = VclPtr<SvxRelativeField>::Create(pParent,
                                            WB_BORDER | WB_SPIN | WB_REPEAT |
                                            WB_LEFT | WB_GROUP,
                                            eUnit);
}

void SvxRelativeField::SetRelative( bool bNewRelative )
{
    Selection aSelection = GetSelection();
    OUString aStr = GetText();

    if ( bNewRelative )
    {
        bRelative = true;
        SetDecimalDigits( 0 );
        SetMin( 0 );
        SetMax( 0 );
        SetUnit( FUNIT_PERCENT );
    }
    else
    {
        bRelative = false;
        SetDecimalDigits( 2 );
        SetMin( 0 );
        SetMax( 9999 );
        SetUnit( FUNIT_CM );
    }

    SetText( aStr );
    SetSelection( aSelection );
}

RelativeField::RelativeField(weld::MetricSpinButton* pControl)
    : m_xSpinButton(pControl)
    , nRelMin(0)
    , nRelMax(0)
    , bRelativeMode(false)
    , bRelative(false)
    , bNegativeEnabled(false)

{
    weld::SpinButton& rSpinButton = m_xSpinButton->get_widget();
    rSpinButton.connect_changed(LINK(this, RelativeField, ModifyHdl));
}

IMPL_LINK_NOARG(RelativeField, ModifyHdl, weld::Entry&, void)
{
    if (bRelativeMode)
    {
        OUString  aStr = m_xSpinButton->get_text();
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
    }
}

void RelativeField::EnableRelativeMode(sal_uInt16 nMin, sal_uInt16 nMax)
{
    bRelativeMode = true;
    nRelMin       = nMin;
    nRelMax       = nMax;
    m_xSpinButton->set_unit(FUNIT_CM);
}

void RelativeField::SetRelative( bool bNewRelative )
{
    weld::SpinButton& rSpinButton = m_xSpinButton->get_widget();

    int nStartPos, nEndPos;
    rSpinButton.get_selection_bounds(nStartPos, nEndPos);
    OUString aStr = rSpinButton.get_text();

    if ( bNewRelative )
    {
        bRelative = true;
        m_xSpinButton->set_digits(0);
        m_xSpinButton->set_range(nRelMin, nRelMax, FUNIT_NONE);
        m_xSpinButton->set_unit(FUNIT_PERCENT);
    }
    else
    {
        bRelative = false;
        m_xSpinButton->set_digits(2);
        m_xSpinButton->set_range(bNegativeEnabled ? -9999 : 0, 9999, FUNIT_NONE);
        m_xSpinButton->set_unit(FUNIT_CM);
    }

    rSpinButton.set_text(aStr);
    rSpinButton.select_region(nStartPos, nEndPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
