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

#include <attrib.hxx>
#include <sc.hrc>

#include <tabpages.hxx>
#include <osl/diagnose.h>

const sal_uInt16 ScTabPageProtection::pProtectionRanges[] =
{
    SID_SCATTR_PROTECTION,
    SID_SCATTR_PROTECTION,
    0
};

// Zellschutz-Tabpage:

ScTabPageProtection::ScTabPageProtection(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "modules/scalc/ui/cellprotectionpage.ui", "CellProtectionPage", &rCoreAttrs)
    , m_xBtnHideCell(m_xBuilder->weld_check_button("checkHideAll"))
    , m_xBtnProtect(m_xBuilder->weld_check_button("checkProtected"))
    , m_xBtnHideFormula(m_xBuilder->weld_check_button("checkHideFormula"))
    , m_xBtnHidePrint(m_xBuilder->weld_check_button("checkHidePrinting"))
{
    // This Page need ExchangeSupport
    SetExchangeSupport();

    //  States will be set in Reset
    bTriEnabled = bDontCare = bProtect = bHideForm = bHideCell = bHidePrint = false;

    m_xBtnProtect->connect_toggled(LINK(this, ScTabPageProtection, ButtonClickHdl));
    m_xBtnHideCell->connect_toggled(LINK(this, ScTabPageProtection, ButtonClickHdl));
    m_xBtnHideFormula->connect_toggled(LINK(this, ScTabPageProtection, ButtonClickHdl));
    m_xBtnHidePrint->connect_toggled(LINK(this, ScTabPageProtection, ButtonClickHdl));
}

ScTabPageProtection::~ScTabPageProtection()
{
    disposeOnce();
}

VclPtr<SfxTabPage> ScTabPageProtection::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<ScTabPageProtection>::Create(pParent, *rAttrSet);
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
    if (bTriEnabled)
    {
        m_xBtnProtect->set_state(TRISTATE_INDET);
        m_xBtnHideCell->set_state(TRISTATE_INDET);
        m_xBtnHideFormula->set_state(TRISTATE_INDET);
        m_xBtnHidePrint->set_state(TRISTATE_INDET);
    }

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
            bAttrsChanged = !pOldItem || aProtAttr != *static_cast<const ScProtectionAttr*>(pOldItem);
    }

    if ( bAttrsChanged )
        rCoreAttrs->Put( aProtAttr );
    else if ( eItemState == SfxItemState::DEFAULT )
        rCoreAttrs->ClearItem( nWhich );

    return bAttrsChanged;
}

DeactivateRC ScTabPageProtection::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( pSetP );

    return DeactivateRC::LeavePage;
}

IMPL_LINK(ScTabPageProtection, ButtonClickHdl, weld::ToggleButton&, rBox, void)
{
    TriState eState = rBox.get_state();
    if (eState == TRISTATE_INDET)
        bDontCare = true;                           // everything combined at DontCare
    else
    {
        bDontCare = false;                          // DontCare from everywhere
        bool bOn = eState == TRISTATE_TRUE;         // from a selected value

        if (&rBox == m_xBtnProtect.get())
            bProtect = bOn;
        else if (&rBox == m_xBtnHideCell.get())
            bHideCell = bOn;
        else if (&rBox == m_xBtnHideFormula.get())
            bHideForm = bOn;
        else if (&rBox == m_xBtnHidePrint.get())
            bHidePrint = bOn;
        else
        {
            OSL_FAIL("Wrong button");
        }
    }

    UpdateButtons();        // TriState and Logic-Enable
}

void ScTabPageProtection::UpdateButtons()
{
    if (bDontCare)
    {
        m_xBtnProtect->set_state(TRISTATE_INDET);
        m_xBtnHideCell->set_state(TRISTATE_INDET);
        m_xBtnHideFormula->set_state(TRISTATE_INDET);
        m_xBtnHidePrint->set_state(TRISTATE_INDET);
    }
    else
    {
        m_xBtnProtect->set_state(bProtect ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xBtnHideCell->set_state(bHideCell ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xBtnHideFormula->set_state(bHideForm ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xBtnHidePrint->set_state(bHidePrint ? TRISTATE_TRUE : TRISTATE_FALSE);
    }

    bool bEnable = (m_xBtnHideCell->get_state() != TRISTATE_TRUE);
    {
        m_xBtnProtect->set_sensitive(bEnable);
        m_xBtnHideFormula->set_sensitive(bEnable);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
