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
#include <dialmgr.hxx>
#include <cuires.hrc>
#include <svl/ctloptions.hxx>

// class SvxCTLOptionsPage -----------------------------------------------------

IMPL_LINK_NOARG(SvxCTLOptionsPage, SequenceCheckingCB_Hdl)
{
    sal_Bool bIsSequenceChecking = m_pSequenceCheckingCB->IsChecked();
    m_pRestrictedCB->Enable( bIsSequenceChecking );
    m_pTypeReplaceCB->Enable( bIsSequenceChecking );
    // #i48117#: by default restricted and type&replace have to be switched on
    if(bIsSequenceChecking)
    {
        m_pTypeReplaceCB->Check( true );
        m_pRestrictedCB->Check( true );
    }
    return 0;
}

SvxCTLOptionsPage::SvxCTLOptionsPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, "OptCTLPage", "cui/ui/optctlpage.ui", rSet  )

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
}

SfxTabPage* SvxCTLOptionsPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxCTLOptionsPage( pParent, rAttrSet );
}

sal_Bool SvxCTLOptionsPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bModified = sal_False;
    SvtCTLOptions aCTLOptions;

    // Sequence checking
    sal_Bool bChecked = m_pSequenceCheckingCB->IsChecked();
    if ( bChecked != m_pSequenceCheckingCB->GetSavedValue() )
    {
        aCTLOptions.SetCTLSequenceChecking( bChecked );
        bModified = sal_True;
    }

    bChecked = m_pRestrictedCB->IsChecked();
    if( bChecked != m_pRestrictedCB->GetSavedValue() )
    {
        aCTLOptions.SetCTLSequenceCheckingRestricted( bChecked );
        bModified = sal_True;
    }
    bChecked = m_pTypeReplaceCB->IsChecked();
    if( bChecked != m_pTypeReplaceCB->GetSavedValue())
    {
        aCTLOptions.SetCTLSequenceCheckingTypeAndReplace(bChecked);
        bModified = sal_True;
    }

    bool bLogicalChecked = m_pMovementLogicalRB->IsChecked();
    bool bVisualChecked = m_pMovementVisualRB->IsChecked();
    if ( bLogicalChecked != m_pMovementLogicalRB->GetSavedValue() ||
         bVisualChecked != m_pMovementVisualRB->GetSavedValue() )
    {
        SvtCTLOptions::CursorMovement eMovement =
            bLogicalChecked ? SvtCTLOptions::MOVEMENT_LOGICAL : SvtCTLOptions::MOVEMENT_VISUAL;
        aCTLOptions.SetCTLCursorMovement( eMovement );
        bModified = sal_True;
    }

    sal_uInt16 nPos = m_pNumeralsLB->GetSelectEntryPos();
    if ( nPos != m_pNumeralsLB->GetSavedValue() )
    {
        aCTLOptions.SetCTLTextNumerals( (SvtCTLOptions::TextNumerals)nPos );
        bModified = sal_True;
    }

    return bModified;
}

void SvxCTLOptionsPage::Reset( const SfxItemSet& )
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

    sal_uInt16 nPos = (sal_uInt16)aCTLOptions.GetCTLTextNumerals();
    DBG_ASSERT( nPos < m_pNumeralsLB->GetEntryCount(), "SvxCTLOptionsPage::Reset(): invalid numerals enum" );
    m_pNumeralsLB->SelectEntryPos( nPos );

    m_pSequenceCheckingCB->SaveValue();
    m_pRestrictedCB->SaveValue();
    m_pTypeReplaceCB->SaveValue();
    m_pMovementLogicalRB->SaveValue();
    m_pMovementVisualRB->SaveValue();
    m_pNumeralsLB->SaveValue();

    sal_Bool bIsSequenceChecking = m_pSequenceCheckingCB->IsChecked();
    m_pRestrictedCB->Enable( bIsSequenceChecking );
    m_pTypeReplaceCB->Enable( bIsSequenceChecking );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
