/*************************************************************************
 *
 *  $RCSfile: tabpages.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mh $ $Date: 2001-10-23 11:02:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif


#include "global.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include "attrdlg.hrc"
#include "tabpages.hxx"

// STATIC DATA -----------------------------------------------------------

static USHORT pProtectionRanges[] =
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
        aBtnProtect         ( this, ScResId( BTN_PROTECTED      ) ),
        aBtnHideCell        ( this, ScResId( BTN_HIDE_ALL       ) ),
        aBtnHideFormula     ( this, ScResId( BTN_HIDE_FORMULAR  ) ),
        aTxtHint            ( this, ScResId( FT_HINT            ) ),
        aFlPrint            ( this, ScResId( FL_PRINT           ) ),
        aBtnHidePrint       ( this, ScResId( BTN_HIDE_PRINT     ) ),
        aTxtHint2           ( this, ScResId( FT_HINT2           ) )
{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    //  States werden in Reset gesetzt
    bTriEnabled = bDontCare = bProtect = bHideForm = bHideCell = bHidePrint = FALSE;

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

USHORT* __EXPORT ScTabPageProtection::GetRanges()
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

    USHORT nWhich = GetWhich( SID_SCATTR_PROTECTION );
    const ScProtectionAttr* pProtAttr = NULL;
    SfxItemState eItemState = rCoreAttrs.GetItemState( nWhich, FALSE,
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
        bProtect = TRUE;
        bHideForm = bHideCell = bHidePrint = FALSE;
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

BOOL __EXPORT ScTabPageProtection::FillItemSet( SfxItemSet& rCoreAttrs )
{
    BOOL                bAttrsChanged   = FALSE;
    USHORT              nWhich          = GetWhich( SID_SCATTR_PROTECTION );
    const SfxPoolItem*  pOldItem        = GetOldItem( rCoreAttrs, SID_SCATTR_PROTECTION );
    const SfxItemSet&   rOldSet         = GetItemSet();
    SfxItemState        eItemState      = rOldSet.GetItemState( nWhich, FALSE );
    ScProtectionAttr    aProtAttr;

    if ( !bDontCare )
    {
        aProtAttr.SetProtection( bProtect );
        aProtAttr.SetHideCell( bHideCell );
        aProtAttr.SetHideFormula( bHideForm );
        aProtAttr.SetHidePrint( bHidePrint );

        if ( bTriEnabled )
            bAttrsChanged = TRUE;                   // DontCare -> richtiger Wert
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

int __EXPORT ScTabPageProtection::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

IMPL_LINK( ScTabPageProtection, ButtonClickHdl, TriStateBox*, pBox )
{
    TriState eState = pBox->GetState();
    if ( eState == STATE_DONTKNOW )
        bDontCare = TRUE;                           // alles zusammen auf DontCare
    else
    {
        bDontCare = FALSE;                          // DontCare ueberall aus
        BOOL bOn = ( eState == STATE_CHECK );       // ausgewaehlter Wert

        if ( pBox == &aBtnProtect )
            bProtect = bOn;
        else if ( pBox == &aBtnHideCell )
            bHideCell = bOn;
        else if ( pBox == &aBtnHideFormula )
            bHideForm = bOn;
        else if ( pBox == &aBtnHidePrint )
            bHidePrint = bOn;
        else
            DBG_ERRORFILE("falscher Button");
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

    BOOL bEnable = ( aBtnHideCell.GetState() != STATE_CHECK );
    {
        aBtnProtect.Enable( bEnable );
        aBtnHideFormula.Enable( bEnable );
    }
}
