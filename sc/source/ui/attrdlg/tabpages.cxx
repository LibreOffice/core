/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



#include "global.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include "attrdlg.hrc"
#include "tabpages.hxx"

// STATIC DATA -----------------------------------------------------------

static sal_uInt16 pProtectionRanges[] =
{
    SID_SCATTR_PROTECTION,
    SID_SCATTR_PROTECTION,
    0
};

//========================================================================
// Zellschutz-Tabpage:
//========================================================================

ScTabPageProtection::ScTabPageProtection( Window*           pParent,
                                          const SfxItemSet& rCoreAttrs )
    :   SfxTabPage          ( pParent,
                              ScResId( RID_SCPAGE_PROTECTION ),
                              rCoreAttrs ),
        //
        aFlProtect          ( this, ScResId( FL_PROTECTION      ) ),
        aBtnHideCell        ( this, ScResId( BTN_HIDE_ALL       ) ),
        aBtnProtect         ( this, ScResId( BTN_PROTECTED      ) ),
        aBtnHideFormula     ( this, ScResId( BTN_HIDE_FORMULAR  ) ),
        aTxtHint            ( this, ScResId( FT_HINT            ) ),
        aFlPrint            ( this, ScResId( FL_PRINT           ) ),
        aBtnHidePrint       ( this, ScResId( BTN_HIDE_PRINT     ) ),
        aTxtHint2           ( this, ScResId( FT_HINT2           ) )
{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    //  States werden in Reset gesetzt
    bTriEnabled = bDontCare = bProtect = bHideForm = bHideCell = bHidePrint = sal_False;

    aBtnProtect.SetClickHdl(     LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    aBtnHideCell.SetClickHdl(    LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    aBtnHideFormula.SetClickHdl( LINK( this, ScTabPageProtection, ButtonClickHdl ) );
    aBtnHidePrint.SetClickHdl(   LINK( this, ScTabPageProtection, ButtonClickHdl ) );

    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScTabPageProtection::~ScTabPageProtection()
{
}

//------------------------------------------------------------------------

sal_uInt16* __EXPORT ScTabPageProtection::GetRanges()
{
    return pProtectionRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTabPageProtection::Create( Window*           pParent,
                                                  const SfxItemSet& rAttrSet )
{
    return ( new ScTabPageProtection( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

void __EXPORT ScTabPageProtection::Reset( const SfxItemSet& rCoreAttrs )
{
    //  Variablen initialisieren

    sal_uInt16 nWhich = GetWhich( SID_SCATTR_PROTECTION );
    const ScProtectionAttr* pProtAttr = NULL;
    SfxItemState eItemState = rCoreAttrs.GetItemState( nWhich, sal_False,
                                          (const SfxPoolItem**)&pProtAttr );

    // handelt es sich um ein Default-Item?
    if ( eItemState == SFX_ITEM_DEFAULT )
        pProtAttr = (const ScProtectionAttr*)&(rCoreAttrs.Get(nWhich));
    // bei SFX_ITEM_DONTCARE auf 0 lassen

    bTriEnabled = ( pProtAttr == NULL );                // TriState, wenn DontCare
    bDontCare = bTriEnabled;
    if (bTriEnabled)
    {
        //  Defaults, die erscheinen wenn ein TriState weggeklickt wird:
        //  (weil alles zusammen ein Attribut ist, kann auch nur alles zusammen
        //  auf DontCare stehen - #38543#)
        bProtect = sal_True;
        bHideForm = bHideCell = bHidePrint = sal_False;
    }
    else
    {
        bProtect = pProtAttr->GetProtection();
        bHideCell = pProtAttr->GetHideCell();
        bHideForm = pProtAttr->GetHideFormula();
        bHidePrint = pProtAttr->GetHidePrint();
    }

    //  Controls initialisieren

    aBtnProtect     .EnableTriState( bTriEnabled );
    aBtnHideCell    .EnableTriState( bTriEnabled );
    aBtnHideFormula .EnableTriState( bTriEnabled );
    aBtnHidePrint   .EnableTriState( bTriEnabled );

    UpdateButtons();
}

// -----------------------------------------------------------------------

sal_Bool __EXPORT ScTabPageProtection::FillItemSet( SfxItemSet& rCoreAttrs )
{
    sal_Bool                bAttrsChanged   = sal_False;
    sal_uInt16              nWhich          = GetWhich( SID_SCATTR_PROTECTION );
    const SfxPoolItem*  pOldItem        = GetOldItem( rCoreAttrs, SID_SCATTR_PROTECTION );
    const SfxItemSet&   rOldSet         = GetItemSet();
    SfxItemState        eItemState      = rOldSet.GetItemState( nWhich, sal_False );
    ScProtectionAttr    aProtAttr;

    if ( !bDontCare )
    {
        aProtAttr.SetProtection( bProtect );
        aProtAttr.SetHideCell( bHideCell );
        aProtAttr.SetHideFormula( bHideForm );
        aProtAttr.SetHidePrint( bHidePrint );

        if ( bTriEnabled )
            bAttrsChanged = sal_True;                   // DontCare -> richtiger Wert
        else
            bAttrsChanged = !pOldItem || !( aProtAttr == *(const ScProtectionAttr*)pOldItem );
    }

    //--------------------------------------------------

    if ( bAttrsChanged )
        rCoreAttrs.Put( aProtAttr );
    else if ( eItemState == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    return bAttrsChanged;
}

//------------------------------------------------------------------------

int __EXPORT ScTabPageProtection::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pSetP )
        FillItemSet( *pSetP );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

IMPL_LINK( ScTabPageProtection, ButtonClickHdl, TriStateBox*, pBox )
{
    TriState eState = pBox->GetState();
    if ( eState == STATE_DONTKNOW )
        bDontCare = sal_True;                           // alles zusammen auf DontCare
    else
    {
        bDontCare = sal_False;                          // DontCare ueberall aus
        sal_Bool bOn = ( eState == STATE_CHECK );       // ausgewaehlter Wert

        if ( pBox == &aBtnProtect )
            bProtect = bOn;
        else if ( pBox == &aBtnHideCell )
            bHideCell = bOn;
        else if ( pBox == &aBtnHideFormula )
            bHideForm = bOn;
        else if ( pBox == &aBtnHidePrint )
            bHidePrint = bOn;
        else
        {
            DBG_ERRORFILE("falscher Button");
        }
    }

    UpdateButtons();        // TriState und Enable-Logik

    return 0;
}

//------------------------------------------------------------------------

void ScTabPageProtection::UpdateButtons()
{
    if ( bDontCare )
    {
        aBtnProtect.SetState( STATE_DONTKNOW );
        aBtnHideCell.SetState( STATE_DONTKNOW );
        aBtnHideFormula.SetState( STATE_DONTKNOW );
        aBtnHidePrint.SetState( STATE_DONTKNOW );
    }
    else
    {
        aBtnProtect.SetState( bProtect ? STATE_CHECK : STATE_NOCHECK );
        aBtnHideCell.SetState( bHideCell ? STATE_CHECK : STATE_NOCHECK );
        aBtnHideFormula.SetState( bHideForm ? STATE_CHECK : STATE_NOCHECK );
        aBtnHidePrint.SetState( bHidePrint ? STATE_CHECK : STATE_NOCHECK );
    }

    sal_Bool bEnable = ( aBtnHideCell.GetState() != STATE_CHECK );
    {
        aBtnProtect.Enable( bEnable );
        aBtnHideFormula.Enable( bEnable );
    }
}
