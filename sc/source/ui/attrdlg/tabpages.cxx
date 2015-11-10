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

#undef SC_DLLIMPLEMENTATION

#include "global.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include "tabpages.hxx"

// STATIC DATA -----------------------------------------------------------

const sal_uInt16 ScTabPageProtection::pProtectionRanges[] =
{
    SID_SCATTR_PROTECTION,
    SID_SCATTR_PROTECTION,
    0
};

// Zellschutz-Tabpage:

ScTabPageProtection::ScTabPageProtection(vcl::Window* pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "CellProtectionPage",
        "modules/scalc/ui/cellprotectionpage.ui", &rCoreAttrs)
{
    get(m_pBtnHideCell,"checkHideAll");
    get(m_pBtnProtect,"checkProtected");
    get(m_pBtnHideFormula,"checkHideFormula");
    get(m_pBtnHidePrint,"checkHidePrinting");

    // This Page need ExchangeSupport
    SetExchangeSupport();

    //  States will be set in Reset
    bTriEnabled = bDontCare = bProtect = bHideForm = bHideCell = bHidePrint = false;

    m_pBtnProtect->SetClickHdl(     LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    m_pBtnHideCell->SetClickHdl(    LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    m_pBtnHideFormula->SetClickHdl( LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    m_pBtnHidePrint->SetClickHdl(   LINK( this, ScTabPageProtection, ButtonClickHdl ) );
}

ScTabPageProtection::~ScTabPageProtection()
{
    disposeOnce();
}

void ScTabPageProtection::dispose()
{
    m_pBtnHideCell.clear();
    m_pBtnProtect.clear();
    m_pBtnHideFormula.clear();
    m_pBtnHidePrint.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> ScTabPageProtection::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<ScTabPageProtection>::Create( pParent, *rAttrSet );
}

void ScTabPageProtection::Reset( const SfxItemSet* rCoreAttrs )
{
    //  Initialize variables

    sal_uInt16 nWhich = GetWhich( SID_SCATTR_PROTECTION );
    const ScProtectionAttr* pProtAttr = nullptr;
    SfxItemState eItemState = rCoreAttrs->GetItemState( nWhich, false,
                                          reinterpret_cast<const SfxPoolItem**>(&pProtAttr) );

    // Is this a Default-Item?
    if ( eItemState == SfxItemState::DEFAULT )
        pProtAttr = static_cast<const ScProtectionAttr*>(&(rCoreAttrs->Get(nWhich)));
    // At SfxItemState::DONTCARE let to 0

    bTriEnabled = ( pProtAttr == nullptr );                // TriState, when DontCare
    bDontCare = bTriEnabled;
    if (bTriEnabled)
    {
       //  Defaults which appear when a TriState will be clicked away:
       //  (because everything combined is an attribute, and also only
       //   everything combined as DontCare can be available - #38543#)

        bProtect = true;
        bHideForm = bHideCell = bHidePrint = false;
    }
    else
    {
        bProtect = pProtAttr->GetProtection();
        bHideCell = pProtAttr->GetHideCell();
        bHideForm = pProtAttr->GetHideFormula();
        bHidePrint = pProtAttr->GetHidePrint();
    }

    //  Start Controls

    m_pBtnProtect->EnableTriState( bTriEnabled );
    m_pBtnHideCell->EnableTriState( bTriEnabled );
    m_pBtnHideFormula->EnableTriState( bTriEnabled );
    m_pBtnHidePrint->EnableTriState( bTriEnabled );

    UpdateButtons();
}

bool ScTabPageProtection::FillItemSet( SfxItemSet* rCoreAttrs )
{
    bool                bAttrsChanged   = false;
    sal_uInt16              nWhich          = GetWhich( SID_SCATTR_PROTECTION );
    const SfxPoolItem*  pOldItem        = GetOldItem( *rCoreAttrs, SID_SCATTR_PROTECTION );
    const SfxItemSet&   rOldSet         = GetItemSet();
    SfxItemState        eItemState      = rOldSet.GetItemState( nWhich, false );
    ScProtectionAttr    aProtAttr;

    if ( !bDontCare )
    {
        aProtAttr.SetProtection( bProtect );
        aProtAttr.SetHideCell( bHideCell );
        aProtAttr.SetHideFormula( bHideForm );
        aProtAttr.SetHidePrint( bHidePrint );

        if ( bTriEnabled )
            bAttrsChanged = true;                   // DontCare -> properly value
        else
            bAttrsChanged = !pOldItem || !( aProtAttr == *static_cast<const ScProtectionAttr*>(pOldItem) );
    }

    if ( bAttrsChanged )
        rCoreAttrs->Put( aProtAttr );
    else if ( eItemState == SfxItemState::DEFAULT )
        rCoreAttrs->ClearItem( nWhich );

    return bAttrsChanged;
}

SfxTabPage::sfxpg ScTabPageProtection::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( pSetP );

    return LEAVE_PAGE;
}

IMPL_LINK_TYPED( ScTabPageProtection, ButtonClickHdl, Button*, pBox, void )
{
    TriState eState = static_cast<TriStateBox*>(pBox)->GetState();
    if ( eState == TRISTATE_INDET )
        bDontCare = true;                           // everything combined at DontCare
    else
    {
        bDontCare = false;                          // DontCare from everywhere
        bool bOn = ( eState == TRISTATE_TRUE );       // from a selected value

        if ( pBox == m_pBtnProtect )
            bProtect = bOn;
        else if ( pBox == m_pBtnHideCell )
            bHideCell = bOn;
        else if ( pBox == m_pBtnHideFormula )
            bHideForm = bOn;
        else if ( pBox == m_pBtnHidePrint )
            bHidePrint = bOn;
        else
        {
            OSL_FAIL("falscher Button");
        }
    }

    UpdateButtons();        // TriState and Logic-Enable
}

void ScTabPageProtection::UpdateButtons()
{
    if ( bDontCare )
    {
        m_pBtnProtect->SetState( TRISTATE_INDET );
        m_pBtnHideCell->SetState( TRISTATE_INDET );
        m_pBtnHideFormula->SetState( TRISTATE_INDET );
        m_pBtnHidePrint->SetState( TRISTATE_INDET );
    }
    else
    {
        m_pBtnProtect->SetState( bProtect ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pBtnHideCell->SetState( bHideCell ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pBtnHideFormula->SetState( bHideForm ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pBtnHidePrint->SetState( bHidePrint ? TRISTATE_TRUE : TRISTATE_FALSE );
    }

    bool bEnable = ( m_pBtnHideCell->GetState() != TRISTATE_TRUE );
    {
        m_pBtnProtect->Enable( bEnable );
        m_pBtnHideFormula->Enable( bEnable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
