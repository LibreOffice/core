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


#undef SC_DLLIMPLEMENTATION


#include "global.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include "tabpages.hxx"



static sal_uInt16 pProtectionRanges[] =
{
    SID_SCATTR_PROTECTION,
    SID_SCATTR_PROTECTION,
    0
};





ScTabPageProtection::ScTabPageProtection(Window* pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "CellProtectionPage",
        "modules/scalc/ui/cellprotectionpage.ui", rCoreAttrs)
{
    get(m_pBtnHideCell,"checkHideAll");
    get(m_pBtnProtect,"checkProtected");
    get(m_pBtnHideFormula,"checkHideFormula");
    get(m_pBtnHidePrint,"checkHidePrinting");

    
    SetExchangeSupport();

    
    bTriEnabled = bDontCare = bProtect = bHideForm = bHideCell = bHidePrint = false;

    m_pBtnProtect->SetClickHdl(     LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    m_pBtnHideCell->SetClickHdl(    LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    m_pBtnHideFormula->SetClickHdl( LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    m_pBtnHidePrint->SetClickHdl(   LINK( this, ScTabPageProtection, ButtonClickHdl ) );
}

sal_uInt16* ScTabPageProtection::GetRanges()
{
    return pProtectionRanges;
}



SfxTabPage* ScTabPageProtection::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new ScTabPageProtection( pParent, rAttrSet ) );
}



void ScTabPageProtection::Reset( const SfxItemSet& rCoreAttrs )
{
    

    sal_uInt16 nWhich = GetWhich( SID_SCATTR_PROTECTION );
    const ScProtectionAttr* pProtAttr = NULL;
    SfxItemState eItemState = rCoreAttrs.GetItemState( nWhich, false,
                                          (const SfxPoolItem**)&pProtAttr );

    
    if ( eItemState == SFX_ITEM_DEFAULT )
        pProtAttr = (const ScProtectionAttr*)&(rCoreAttrs.Get(nWhich));
    

    bTriEnabled = ( pProtAttr == NULL );                
    bDontCare = bTriEnabled;
    if (bTriEnabled)
    {
       
       
       

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

    

    m_pBtnProtect->EnableTriState( bTriEnabled );
    m_pBtnHideCell->EnableTriState( bTriEnabled );
    m_pBtnHideFormula->EnableTriState( bTriEnabled );
    m_pBtnHidePrint->EnableTriState( bTriEnabled );

    UpdateButtons();
}



sal_Bool ScTabPageProtection::FillItemSet( SfxItemSet& rCoreAttrs )
{
    sal_Bool                bAttrsChanged   = false;
    sal_uInt16              nWhich          = GetWhich( SID_SCATTR_PROTECTION );
    const SfxPoolItem*  pOldItem        = GetOldItem( rCoreAttrs, SID_SCATTR_PROTECTION );
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
            bAttrsChanged = sal_True;                   
        else
            bAttrsChanged = !pOldItem || !( aProtAttr == *(const ScProtectionAttr*)pOldItem );
    }

    

    if ( bAttrsChanged )
        rCoreAttrs.Put( aProtAttr );
    else if ( eItemState == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    return bAttrsChanged;
}



int ScTabPageProtection::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( *pSetP );

    return LEAVE_PAGE;
}



IMPL_LINK( ScTabPageProtection, ButtonClickHdl, TriStateBox*, pBox )
{
    TriState eState = pBox->GetState();
    if ( eState == STATE_DONTKNOW )
        bDontCare = true;                           
    else
    {
        bDontCare = false;                          
        sal_Bool bOn = ( eState == STATE_CHECK );       

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

    UpdateButtons();        

    return 0;
}



void ScTabPageProtection::UpdateButtons()
{
    if ( bDontCare )
    {
        m_pBtnProtect->SetState( STATE_DONTKNOW );
        m_pBtnHideCell->SetState( STATE_DONTKNOW );
        m_pBtnHideFormula->SetState( STATE_DONTKNOW );
        m_pBtnHidePrint->SetState( STATE_DONTKNOW );
    }
    else
    {
        m_pBtnProtect->SetState( bProtect ? STATE_CHECK : STATE_NOCHECK );
        m_pBtnHideCell->SetState( bHideCell ? STATE_CHECK : STATE_NOCHECK );
        m_pBtnHideFormula->SetState( bHideForm ? STATE_CHECK : STATE_NOCHECK );
        m_pBtnHidePrint->SetState( bHidePrint ? STATE_CHECK : STATE_NOCHECK );
    }

    sal_Bool bEnable = ( m_pBtnHideCell->GetState() != STATE_CHECK );
    {
        m_pBtnProtect->Enable( bEnable );
        m_pBtnHideFormula->Enable( bEnable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
