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

#include "optctl.hxx"
#include <svl/ctloptions.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>

// class SvxCTLOptionsPage -----------------------------------------------------

IMPL_LINK_NOARG(SvxCTLOptionsPage, SequenceCheckingCB_Hdl, Button*, void)
{
    bool bIsSequenceChecking = m_pSequenceCheckingCB->IsChecked();
    m_pRestrictedCB->Enable( bIsSequenceChecking );
    m_pTypeReplaceCB->Enable( bIsSequenceChecking );
    // #i48117#: by default restricted and type&replace have to be switched on
    if(bIsSequenceChecking)
    {
        m_pTypeReplaceCB->Check();
        m_pRestrictedCB->Check();
    }
}

SvxCTLOptionsPage::SvxCTLOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, "OptCTLPage", "cui/ui/optctlpage.ui", &rSet  )

{
    get( m_pSequenceCheckingCB, "sequencechecking");
    get( m_pRestrictedCB, "restricted");
    get( m_pTypeReplaceCB, "typeandreplace");

    get( m_pMovementLogicalRB, "movementlogical");
    get( m_pMovementVisualRB, "movementvisual");

    get( m_pNumeralsLB, "numerals");

    m_pSequenceCheckingCB->SetClickHdl( LINK( this, SvxCTLOptionsPage, SequenceCheckingCB_Hdl ) );

    m_pNumeralsLB->SetDropDownLineCount( m_pNumeralsLB->GetEntryCount() );
}

SvxCTLOptionsPage::~SvxCTLOptionsPage()
{
    disposeOnce();
}

void SvxCTLOptionsPage::dispose()
{
    m_pSequenceCheckingCB.clear();
    m_pRestrictedCB.clear();
    m_pTypeReplaceCB.clear();
    m_pMovementLogicalRB.clear();
    m_pMovementVisualRB.clear();
    m_pNumeralsLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxCTLOptionsPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxCTLOptionsPage>::Create( pParent.pParent, *rAttrSet );
}

bool SvxCTLOptionsPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;
    SvtCTLOptions aCTLOptions;

    // Sequence checking
    bool bChecked = m_pSequenceCheckingCB->IsChecked();
    if ( m_pSequenceCheckingCB->IsValueChangedFromSaved() )
    {
        aCTLOptions.SetCTLSequenceChecking( bChecked );
        bModified = true;
    }

    bChecked = m_pRestrictedCB->IsChecked();
    if( m_pRestrictedCB->IsValueChangedFromSaved() )
    {
        aCTLOptions.SetCTLSequenceCheckingRestricted( bChecked );
        bModified = true;
    }
    bChecked = m_pTypeReplaceCB->IsChecked();
    if( m_pTypeReplaceCB->IsValueChangedFromSaved())
    {
        aCTLOptions.SetCTLSequenceCheckingTypeAndReplace(bChecked);
        bModified = true;
    }

    bool bLogicalChecked = m_pMovementLogicalRB->IsChecked();
    if ( m_pMovementLogicalRB->IsValueChangedFromSaved() ||
         m_pMovementVisualRB->IsValueChangedFromSaved() )
    {
        SvtCTLOptions::CursorMovement eMovement =
            bLogicalChecked ? SvtCTLOptions::MOVEMENT_LOGICAL : SvtCTLOptions::MOVEMENT_VISUAL;
        aCTLOptions.SetCTLCursorMovement( eMovement );
        bModified = true;
    }

    if ( m_pNumeralsLB->IsValueChangedFromSaved() )
    {
        const sal_Int32 nPos = m_pNumeralsLB->GetSelectedEntryPos();
        aCTLOptions.SetCTLTextNumerals( static_cast<SvtCTLOptions::TextNumerals>(nPos) );
        bModified = true;
    }

    return bModified;
}

void SvxCTLOptionsPage::Reset( const SfxItemSet* )
{
    SvtCTLOptions aCTLOptions;

    m_pSequenceCheckingCB->Check( aCTLOptions.IsCTLSequenceChecking() );
    m_pRestrictedCB->Check( aCTLOptions.IsCTLSequenceCheckingRestricted() );
    m_pTypeReplaceCB->Check( aCTLOptions.IsCTLSequenceCheckingTypeAndReplace() );

    SvtCTLOptions::CursorMovement eMovement = aCTLOptions.GetCTLCursorMovement();
    switch ( eMovement )
    {
        case SvtCTLOptions::MOVEMENT_LOGICAL :
            m_pMovementLogicalRB->Check();
            break;

        case SvtCTLOptions::MOVEMENT_VISUAL :
            m_pMovementVisualRB->Check();
            break;

        default:
            SAL_WARN( "cui.options", "SvxCTLOptionsPage::Reset(): invalid movement enum" );
    }

    sal_uInt16 nPos = static_cast<sal_uInt16>(aCTLOptions.GetCTLTextNumerals());
    DBG_ASSERT( nPos < m_pNumeralsLB->GetEntryCount(), "SvxCTLOptionsPage::Reset(): invalid numerals enum" );
    m_pNumeralsLB->SelectEntryPos( nPos );

    m_pSequenceCheckingCB->SaveValue();
    m_pRestrictedCB->SaveValue();
    m_pTypeReplaceCB->SaveValue();
    m_pMovementLogicalRB->SaveValue();
    m_pMovementVisualRB->SaveValue();
    m_pNumeralsLB->SaveValue();

    bool bIsSequenceChecking = m_pSequenceCheckingCB->IsChecked();
    m_pRestrictedCB->Enable( bIsSequenceChecking );
    m_pTypeReplaceCB->Enable( bIsSequenceChecking );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
