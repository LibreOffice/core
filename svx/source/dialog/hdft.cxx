/*************************************************************************
 *
 *  $RCSfile: hdft.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:08 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <vcl/graph.hxx>
#pragma hdrstop

#include "dialogs.hrc"
#include "hdft.hrc"

#define ITEMID_PAGE     0
#define ITEMID_SIZE     0
#define ITEMID_ULSPACE  0
#define ITEMID_LRSPACE  0
#define ITEMID_PAPERBIN 0
#define ITEMID_SETITEM  0
#define ITEMID_BRUSH    0
#define ITEMID_BOX      0
#define ITEMID_BOXINFO  0
#define ITEMID_SHADOW   0

#define _SVX_HDFT_CXX

#include "hdft.hxx"
#include "pageitem.hxx"
#include "bbdlg.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"
#include "htmlmode.hxx"

#include "brshitem.hxx"
#include "lrspitem.hxx"
#include "ulspitem.hxx"
#include "shaditem.hxx"
#include "sizeitem.hxx"
#include "boxitem.hxx"

// static ----------------------------------------------------------------

static const long MINBODY = 284;            // 0,5cm in twips aufgerundet

// default distance to Header or footer
static const long DEF_DIST_WRITER = 500;    // 5mm (Writer)
static const long DEF_DIST_CALC = 250;      // 2,5mm (Calc)

static USHORT pRanges[] =
{
    SID_ATTR_BRUSH,          SID_ATTR_BRUSH,
    SID_ATTR_BORDER_OUTER,   SID_ATTR_BORDER_OUTER,
    SID_ATTR_BORDER_INNER,   SID_ATTR_BORDER_INNER,
    SID_ATTR_BORDER_SHADOW,  SID_ATTR_BORDER_SHADOW,
    SID_ATTR_LRSPACE,        SID_ATTR_LRSPACE,
    SID_ATTR_ULSPACE,        SID_ATTR_ULSPACE,
    SID_ATTR_PAGE_SIZE,      SID_ATTR_PAGE_SIZE,
    SID_ATTR_PAGE_HEADERSET, SID_ATTR_PAGE_HEADERSET,
    SID_ATTR_PAGE_FOOTERSET, SID_ATTR_PAGE_FOOTERSET,
    SID_ATTR_PAGE_ON,        SID_ATTR_PAGE_ON,
    SID_ATTR_PAGE_DYNAMIC,   SID_ATTR_PAGE_DYNAMIC,
    SID_ATTR_PAGE_SHARED,    SID_ATTR_PAGE_SHARED,
    0
};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

// gibt den Bereich der Which-Werte zurueck


USHORT* SvxHeaderPage::GetRanges()
{
    return pRanges;
}

//------------------------------------------------------------------------

SfxTabPage* SvxHeaderPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxHeaderPage( pParent, rSet );
}

//------------------------------------------------------------------------

USHORT* SvxFooterPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxFooterPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxFooterPage( pParent, rSet );
}

// -----------------------------------------------------------------------

SvxHeaderPage::SvxHeaderPage( Window* pParent, const SfxItemSet& rAttr ) :

    SvxHFPage( pParent, RID_SVXPAGE_HEADER, rAttr, SID_ATTR_PAGE_HEADERSET )

{
}

// -----------------------------------------------------------------------

SvxFooterPage::SvxFooterPage( Window* pParent, const SfxItemSet& rAttr ) :

    SvxHFPage( pParent, RID_SVXPAGE_FOOTER, rAttr, SID_ATTR_PAGE_FOOTERSET )

{
}

// -----------------------------------------------------------------------

SvxHFPage::SvxHFPage( Window* pParent, USHORT nResId, const SfxItemSet& rAttr, USHORT nSetId ) :

    SfxTabPage( pParent, ResId( nResId, DIALOG_MGR() ), rAttr ),

    aTurnOnBox      ( this, ResId( CB_TURNON ) ),
    aDistFT         ( this, ResId( FT_DIST ) ),
    aDistEdit       ( this, ResId( ED_DIST ) ),
    aHeightFT       ( this, ResId( FT_HEIGHT ) ),
    aHeightEdit     ( this, ResId( ED_HEIGHT ) ),
    aHeightDynBtn   ( this, ResId( CB_HEIGHT_DYN ) ),
    aLMLbl          ( this, ResId( FT_LMARGIN ) ),
    aLMEdit         ( this, ResId( ED_LMARGIN ) ),
    aRMLbl          ( this, ResId( FT_RMARGIN ) ),
    aRMEdit         ( this, ResId( ED_RMARGIN ) ),
    aCntSharedBox   ( this, ResId( CB_SHARED ) ),
    aFrm            ( this, ResId( GB_FRAME ) ),
    aBspWin         ( this, ResId( WN_BSP ) ),
    aBspFrm         ( this, ResId( GB_BSP ) ),
    aBackgroundBtn  ( this, ResId( BTN_EXTRAS ) ),

    pBBSet                      ( NULL ),
    nId                         ( nSetId ),
    bDisableQueryBox            ( FALSE ),
    bEnableBackgroundSelector   ( TRUE )

{
    InitHandler();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    FreeResource();

    // Metrik einstellen
    FieldUnit eFUnit;
    GET_MODULE_FIELDUNIT( eFUnit );
    SetFieldUnit( aDistEdit, eFUnit );
    SetFieldUnit( aHeightEdit, eFUnit );
    SetFieldUnit( aLMEdit, eFUnit );
    SetFieldUnit( aRMEdit, eFUnit );
}

// -----------------------------------------------------------------------

SvxHFPage::~SvxHFPage()
{
    delete pBBSet;
}

// -----------------------------------------------------------------------

BOOL SvxHFPage::FillItemSet( SfxItemSet& rSet )
{
    const USHORT        nWSize      = GetWhich( SID_ATTR_PAGE_SIZE );
    const USHORT        nWLRSpace   = GetWhich( SID_ATTR_LRSPACE );
    const USHORT        nWULSpace   = GetWhich( SID_ATTR_ULSPACE );
    const USHORT        nWOn        = GetWhich( SID_ATTR_PAGE_ON );
    const USHORT        nWDynamic   = GetWhich( SID_ATTR_PAGE_DYNAMIC );
    const USHORT        nWShared    = GetWhich( SID_ATTR_PAGE_SHARED );
    const USHORT        nWBrush     = GetWhich( SID_ATTR_BRUSH );
    const USHORT        nWBox       = GetWhich( SID_ATTR_BORDER_OUTER );
    const USHORT        nWBoxInfo   = GetWhich( SID_ATTR_BORDER_INNER );
    const USHORT        nWShadow    = GetWhich( SID_ATTR_BORDER_SHADOW );
#ifdef SINIX
    USHORT      aWhichTab[23];
    aWhichTab[0]    =   aWhichTab[1]    = nWSize;
    aWhichTab[2]    =   aWhichTab[3]    = nWLRSpace;
    aWhichTab[4]    =   aWhichTab[5]    = nWULSpace;
    aWhichTab[6]    =   aWhichTab[7]    = nWOn;
    aWhichTab[8]    =   aWhichTab[9]    = nWDynamic;
    aWhichTab[10]   =   aWhichTab[11]   = nWShared;
    aWhichTab[12]   =   aWhichTab[13]   = nWBrush;
    aWhichTab[14]   =   aWhichTab[15]   = nWBoxInfo;
    aWhichTab[16]   =   aWhichTab[17]   = nWBox;
    aWhichTab[18]   =   aWhichTab[19]   = nWShadow;
    aWhichTab[20]   =   0;
#else
    const USHORT        aWhichTab[] = { nWSize,     nWSize,
                                        nWLRSpace,  nWLRSpace,
                                        nWULSpace,  nWULSpace,
                                        nWOn,       nWOn,
                                        nWDynamic,  nWDynamic,
                                        nWShared,   nWShared,
                                        nWBrush,    nWBrush,
                                        nWBoxInfo,  nWBoxInfo,
                                        nWBox,      nWBox,
                                        nWShadow,   nWShadow,
                                        0 };
#endif
    const SfxItemSet&   rOldSet     = GetItemSet();
    SfxItemPool*        pPool       = rOldSet.GetPool();
    DBG_ASSERT( pPool, "no pool :-(" );
    SfxMapUnit          eUnit       = pPool->GetMetric( nWSize );
    SfxItemSet          aSet        ( *pPool, aWhichTab );

    //--------------------------------------------------------------------

    aSet.Put( SfxBoolItem( nWOn,      aTurnOnBox.IsChecked() ) );
    aSet.Put( SfxBoolItem( nWDynamic, aHeightDynBtn.IsChecked() ) );
    aSet.Put( SfxBoolItem( nWShared,  aCntSharedBox.IsChecked() ) );

    // Groesse
    SvxSizeItem aSizeItem( (const SvxSizeItem&)rOldSet.Get( nWSize ) );
    Size        aSize( aSizeItem.GetSize() );
    long        nDist = GetCoreValue( aDistEdit, eUnit );
    long        nH    = GetCoreValue( aHeightEdit, eUnit );

    // fixe Hoehe?
//  if ( !aHeightDynBtn.IsChecked() )
        nH += nDist; // dann Abstand dazu addieren
    aSize.Height() = nH;
    aSizeItem.SetSize( aSize );
    aSet.Put( aSizeItem );

    // Raender
    SvxLRSpaceItem aLR( nWLRSpace );
    aLR.SetLeft( (USHORT)GetCoreValue( aLMEdit, eUnit ) );
    aLR.SetRight( (USHORT)GetCoreValue( aRMEdit, eUnit ) );
    aSet.Put( aLR );

    SvxULSpaceItem aUL( nWULSpace );
    if ( nId == SID_ATTR_PAGE_HEADERSET )
        aUL.SetLower( (USHORT)nDist );
    else
        aUL.SetUpper( (USHORT)nDist );
    aSet.Put( aUL );

    // Hintergrund und Umrandung?
    if ( pBBSet )
        aSet.Put( *pBBSet );
    else
    {
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( GetWhich( nId ), FALSE, &pItem ) )
        {
            pSet = &( (SvxSetItem*)pItem )->GetItemSet();

            if ( pSet->GetItemState( nWBrush ) == SFX_ITEM_SET )
                aSet.Put( (const SvxBrushItem&)pSet->Get( nWBrush ) );
            if ( pSet->GetItemState( nWBoxInfo ) == SFX_ITEM_SET )
                aSet.Put( (const SvxBoxInfoItem&)pSet->Get( nWBoxInfo ) );
            if ( pSet->GetItemState( nWBox ) == SFX_ITEM_SET )
                aSet.Put( (const SvxBoxItem&)pSet->Get( nWBox ) );
            if ( pSet->GetItemState( nWShadow ) == SFX_ITEM_SET )
                aSet.Put( (const SvxShadowItem&)pSet->Get( nWShadow ) );
        }
    }

    // Das SetItem wegschreiben
    SvxSetItem aSetItem( GetWhich( nId ), aSet );
    rSet.Put( aSetItem );

    return TRUE;
}

// -----------------------------------------------------------------------
void SvxHFPage::Reset( const SfxItemSet& rSet )
{
    ActivatePage( rSet );
    ResetBackground_Impl( rSet );

    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_PAGE_SIZE ) );

    // Kopf-/Fusszeilen-Attribute auswerten
    //
    const SvxSetItem* pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich(nId), FALSE,
                                            (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get(GetWhich(SID_ATTR_PAGE_ON));

        aTurnOnBox.Check(rHeaderOn.GetValue());

        if ( rHeaderOn.GetValue() )
        {
            const SfxBoolItem& rDynamic =
                (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_DYNAMIC ) );
            const SfxBoolItem& rShared =
                (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SHARED ) );
            const SvxSizeItem& rSize =
                (const SvxSizeItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL =
                (const SvxULSpaceItem&)rHeaderSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR =
                (const SvxLRSpaceItem&)rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );

            if ( nId == SID_ATTR_PAGE_HEADERSET )
            {   // Kopfzeile
                SetMetricValue( aDistEdit, rUL.GetLower(), eUnit );
                SetMetricValue( aHeightEdit, rSize.GetSize().Height() - rUL.GetLower(), eUnit );
            }
            else
            {   // Fusszeile
                SetMetricValue( aDistEdit, rUL.GetUpper(), eUnit );
                SetMetricValue( aHeightEdit, rSize.GetSize().Height() - rUL.GetUpper(), eUnit );
            }

            aHeightDynBtn.Check(rDynamic.GetValue());
            SetMetricValue( aLMEdit, rLR.GetLeft(), eUnit );
            SetMetricValue( aRMEdit, rLR.GetRight(), eUnit );
            aCntSharedBox.Check(rShared.GetValue());
        }
        else
            pSetItem = 0;
    }
    else
    {
        // defaults for distance and height
        long nDefaultDist = DEF_DIST_WRITER;
        const SfxPoolItem* pExt1 = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
        const SfxPoolItem* pExt2 = GetItem( rSet, SID_ATTR_PAGE_EXT2 );

        if ( pExt1 && pExt1->ISA(SfxBoolItem) && pExt2 && pExt2->ISA(SfxBoolItem) )
            nDefaultDist = DEF_DIST_CALC;

        SetMetricValue( aDistEdit, nDefaultDist, SFX_MAPUNIT_100TH_MM );
        SetMetricValue( aHeightEdit, 500, SFX_MAPUNIT_100TH_MM );
    }

    if ( !pSetItem )
    {
        aTurnOnBox.Check( FALSE );
        aHeightDynBtn.Check( TRUE );
        aCntSharedBox.Check( TRUE );
    }

    TurnOnHdl(0);

    aTurnOnBox.SaveValue();
    aDistEdit.SaveValue();
    aHeightEdit.SaveValue();
    aHeightDynBtn.SaveValue();
    aLMEdit.SaveValue();
    aRMEdit.SaveValue();
    aCntSharedBox.SaveValue();
    RangeHdl( 0 );

    USHORT nHtmlMode = 0;
    const SfxPoolItem* pItem = 0;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        if(nHtmlMode && HTMLMODE_ON)
        {
            aCntSharedBox.Hide();
            aBackgroundBtn.Hide();
            Size aFrmSize = aBspFrm.GetSizePixel();
            long nDiff = aFrmSize.Height() -  aBspWin.GetSizePixel().Height();
            aFrmSize.Height() = aFrm.GetSizePixel().Height();
            aBspFrm.SetSizePixel(aFrmSize);
            aFrmSize.Height() -= nDiff;
            aBspWin.SetSizePixel(aFrmSize);

        }
    }

}

/*--------------------------------------------------------------------
    Beschreibung:   Handler initialisieren
 --------------------------------------------------------------------*/

void SvxHFPage::InitHandler()
{
    aTurnOnBox.SetClickHdl(LINK(this,   SvxHFPage, TurnOnHdl));
    aDistEdit.SetModifyHdl(LINK(this,   SvxHFPage, DistModify));
    aDistEdit.SetLoseFocusHdl(LINK(this, SvxHFPage, RangeHdl));

    aHeightEdit.SetModifyHdl(LINK(this,     SvxHFPage, HeightModify));
    aHeightEdit.SetLoseFocusHdl(LINK(this,SvxHFPage,RangeHdl));

    aLMEdit.SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    aLMEdit.SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeHdl));
    aRMEdit.SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    aRMEdit.SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeHdl));
    aBackgroundBtn.SetClickHdl(LINK(this,SvxHFPage, BackgroundHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:   Ein/aus
 --------------------------------------------------------------------*/

IMPL_LINK( SvxHFPage, TurnOnHdl, CheckBox *, pBox )
{
    if ( aTurnOnBox.IsChecked() )
    {
        aDistFT.Enable();
        aDistEdit.Enable();
        aHeightFT.Enable();
        aHeightEdit.Enable();
        aHeightDynBtn.Enable();
        aLMLbl.Enable();
        aLMEdit.Enable();
        aRMLbl.Enable();
        aRMEdit.Enable();

        USHORT nUsage = aBspWin.GetUsage();

        if( nUsage == SVX_PAGE_RIGHT || nUsage == SVX_PAGE_LEFT )
            aCntSharedBox.Disable();
        else
            aCntSharedBox.Enable();
        aBackgroundBtn.Enable();
    }
    else
    {
        BOOL bDelete = TRUE;

        if ( !bDisableQueryBox && pBox && aTurnOnBox.GetSavedValue() == TRUE )
            bDelete = ( QueryBox( this, ResId( RID_SVXQBX_DELETE_HEADFOOT, DIALOG_MGR() ) ).Execute() == RET_YES );

        if ( bDelete )
        {
            aDistFT.Disable();
            aDistEdit.Disable();
            aHeightFT.Disable();
            aHeightEdit.Disable();
            aHeightDynBtn.Disable();

            aLMLbl.Disable();
            aLMEdit.Disable();
            aRMLbl.Disable();
            aRMEdit.Disable();

            aCntSharedBox.Disable();
            aBackgroundBtn.Disable();
        }
        else
            aTurnOnBox.Check();
    }
    UpdateExample();
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Abstand im Bsp Modifizieren
 --------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SvxHFPage, DistModify, MetricField *, EMPTYARG )
{
    UpdateExample();
    return 0;
}
IMPL_LINK_INLINE_END( SvxHFPage, DistModify, MetricField *, EMPTYARG )

IMPL_LINK_INLINE_START( SvxHFPage, HeightModify, MetricField *, EMPTYARG )
{
    UpdateExample();

    return 0;
}
IMPL_LINK_INLINE_END( SvxHFPage, HeightModify, MetricField *, EMPTYARG )

/*--------------------------------------------------------------------
    Beschreibung: Raender einstellen
 --------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SvxHFPage, BorderModify, MetricField *, EMPTYARG )
{
    UpdateExample();
    return 0;
}
IMPL_LINK_INLINE_END( SvxHFPage, BorderModify, MetricField *, EMPTYARG )

/*--------------------------------------------------------------------
    Beschreibung:   Hintergrund
 --------------------------------------------------------------------*/

IMPL_LINK( SvxHFPage, BackgroundHdl, Button *, EMPTYARG )
{
    if ( !pBBSet )
    {
        // nur die n"otigen Items f"uer Umrandung und Hintergrund benutzen
        USHORT nBrush = GetWhich( SID_ATTR_BRUSH );
        USHORT nOuter = GetWhich( SID_ATTR_BORDER_OUTER );
        USHORT nInner = GetWhich( SID_ATTR_BORDER_INNER );
        USHORT nShadow = GetWhich( SID_ATTR_BORDER_SHADOW );

        // einen leeren Set erzeugen
        pBBSet = new SfxItemSet( *GetItemSet().GetPool(), nBrush, nBrush,
                                 nOuter, nOuter, nInner, nInner,
                                 nShadow, nShadow, 0 );
        const SfxPoolItem* pItem;

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( GetWhich( nId ), FALSE, &pItem ) )
            // wenn es schon einen gesetzen Set gibt, dann diesen benutzen
            pBBSet->Put( ( (SvxSetItem*)pItem)->GetItemSet() );

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( nInner, FALSE, &pItem ) )
            // das gesetze InfoItem wird immer ben"otigt
            pBBSet->Put( *pItem );
    }

    SvxBorderBackgroundDlg* pDlg =
        new SvxBorderBackgroundDlg( this, *pBBSet, bEnableBackgroundSelector );

    if ( pDlg->Execute() == RET_OK && pDlg->GetOutputItemSet() )
    {
        SfxItemIter aIter( *pDlg->GetOutputItemSet() );
        const SfxPoolItem* pItem = aIter.FirstItem();

        while ( pItem )
        {
            if ( !IsInvalidItem( pItem ) )
                pBBSet->Put( *pItem );
            pItem = aIter.NextItem();
        }

        //----------------------------------------------------------------

        USHORT nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( pBBSet->GetItemState( nWhich ) == SFX_ITEM_SET )
        {
            const SvxBrushItem& rItem = (const SvxBrushItem&)pBBSet->Get( nWhich );
            if ( nId == SID_ATTR_PAGE_HEADERSET )
                aBspWin.SetHdColor( rItem.GetColor() );
            else
                aBspWin.SetFtColor( rItem.GetColor() );
        }

        //----------------------------------------------------------------

        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( pBBSet->GetItemState( nWhich ) == SFX_ITEM_SET )
        {
            const SvxBoxItem& rItem = (const SvxBoxItem&)pBBSet->Get( nWhich );

            if ( nId == SID_ATTR_PAGE_HEADERSET )
                aBspWin.SetHdBorder( rItem );
            else
                aBspWin.SetFtBorder( rItem );
        }

        UpdateExample();
    }
    delete pDlg;
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Bsp
 --------------------------------------------------------------------*/

void SvxHFPage::UpdateExample()
{
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        aBspWin.SetHeader( aTurnOnBox.IsChecked() );
        aBspWin.SetHdHeight( GetCoreValue( aHeightEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetHdDist( GetCoreValue( aDistEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetHdLeft( GetCoreValue( aLMEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetHdRight( GetCoreValue( aRMEdit, SFX_MAPUNIT_TWIP ) );
    }
    else
    {
        aBspWin.SetFooter( aTurnOnBox.IsChecked() );
        aBspWin.SetFtHeight( GetCoreValue( aHeightEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetFtDist( GetCoreValue( aDistEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetFtLeft( GetCoreValue( aLMEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetFtRight( GetCoreValue( aRMEdit, SFX_MAPUNIT_TWIP ) );
    }
    aBspWin.Invalidate();
}

/*--------------------------------------------------------------------
    Beschreibung: Hintergrund im Beispiel setzen
 --------------------------------------------------------------------*/

void SvxHFPage::ResetBackground_Impl( const SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_PAGE_HEADERSET );

    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, FALSE );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem = (const SvxBrushItem&)rTmpSet.Get( nWhich );
                aBspWin.SetHdColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetHdBorder( rItem );
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE_FOOTERSET );

    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, FALSE );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem = (const SvxBrushItem&)rTmpSet.Get( nWhich );
                aBspWin.SetFtColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetFtBorder( rItem );
            }
        }
    }
    nWhich = GetWhich( SID_ATTR_BRUSH );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxBrushItem& rItem = (const SvxBrushItem&)rSet.Get( nWhich );
        aBspWin.SetColor( rItem.GetColor() );
        const Graphic* pGrf = rItem.GetGraphic();

        if ( pGrf )
        {
            Bitmap aBitmap = pGrf->GetBitmap();
            aBspWin.SetBitmap( &aBitmap );
        }
        else
            aBspWin.SetBitmap( NULL );
    }
    nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxBoxItem& rItem = (const SvxBoxItem&)rSet.Get( nWhich );
        aBspWin.SetBorder( rItem );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SvxHFPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        // linken und rechten Rand einstellen
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)*pItem;

        aBspWin.SetLeft( rLRSpace.GetLeft() );
        aBspWin.SetRight( rLRSpace.GetRight() );
    }
    else
    {
        aBspWin.SetLeft( 0 );
        aBspWin.SetRight( 0 );
    }

    pItem = GetItem( rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        // oberen und unteren Rand einstellen
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)*pItem;

        aBspWin.SetTop( rULSpace.GetUpper() );
        aBspWin.SetBottom( rULSpace.GetLower() );
    }
    else
    {
        aBspWin.SetTop( 0 );
        aBspWin.SetBottom( 0 );
    }

    USHORT nUsage = SVX_PAGE_ALL;
    pItem = GetItem( rSet, SID_ATTR_PAGE );

    if ( pItem )
        nUsage = ( (const SvxPageItem*)pItem )->GetPageUsage();

    aBspWin.SetUsage( nUsage );

    if ( SVX_PAGE_RIGHT == nUsage || SVX_PAGE_LEFT == nUsage )
        aCntSharedBox.Disable();
    else
        aCntSharedBox.Enable();
    pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
    {
        // Orientation und Size aus dem PageItem
        const SvxSizeItem& rSize = (const SvxSizeItem&)*pItem;
        // die Groesse ist ggf. schon geswappt (Querformat)
        aBspWin.SetSize( rSize.GetSize() );
    }

    // Kopfzeilen-Attribute auswerten
    const SvxSetItem* pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                                            FALSE,
                                            (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SfxBoolItem& rDynamic = (const SfxBoolItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_DYNAMIC ) );
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rHeaderSet.Get( GetWhich(SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            long nDist = rUL.GetLower();

            aBspWin.SetHdHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetHdDist( nDist );
            aBspWin.SetHdLeft( rLR.GetLeft() );
            aBspWin.SetHdRight( rLR.GetRight() );
            aBspWin.SetHeader( TRUE );
        }
        else
            pSetItem = 0;
    }

    if ( !pSetItem )
    {
        aBspWin.SetHeader( FALSE );

        if ( SID_ATTR_PAGE_HEADERSET == nId )
            aCntSharedBox.Disable();
    }
    pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                                            FALSE,
                                            (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn =
            (const SfxBoolItem&)rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rFooterOn.GetValue() )
        {
            const SfxBoolItem& rDynamic = (const SfxBoolItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_DYNAMIC ) );
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            long nDist = rUL.GetUpper();

            aBspWin.SetFtHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetFtDist( nDist );
            aBspWin.SetFtLeft( rLR.GetLeft() );
            aBspWin.SetFtRight( rLR.GetRight() );
            aBspWin.SetFooter( TRUE );
        }
        else
            pSetItem = 0;
    }

    if ( !pSetItem )
    {
        aBspWin.SetFooter( FALSE );

        if ( SID_ATTR_PAGE_FOOTERSET == nId )
            aCntSharedBox.Disable();
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );

    if ( pItem && pItem->ISA(SfxBoolItem) )
    {
        aBspWin.SetTable( TRUE );
        aBspWin.SetHorz( ( (SfxBoolItem*)pItem )->GetValue() );
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );

    if ( pItem && pItem->ISA(SfxBoolItem) )
    {
        aBspWin.SetTable( TRUE );
        aBspWin.SetVert( ( (SfxBoolItem*)pItem )->GetValue() );
    }
    ResetBackground_Impl( rSet );
    RangeHdl( 0 );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SvxHFPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Berech
 --------------------------------------------------------------------*/

IMPL_LINK( SvxHFPage, RangeHdl, Edit *, EMPTYARG )
{
    long nHHeight = aBspWin.GetHdHeight();
    long nHDist   = aBspWin.GetHdDist();

    long nFHeight = aBspWin.GetFtHeight();
    long nFDist   = aBspWin.GetFtDist();

    long nHeight = Max( (long)MINBODY,
        aHeightEdit.Denormalize( aHeightEdit.GetValue( FUNIT_TWIP ) ) );
    long nDist   = aTurnOnBox.IsChecked() ?
        aDistEdit.Denormalize( aDistEdit.GetValue( FUNIT_TWIP ) ) : 0;

    long nMin;
    long nMax;

    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        nHHeight = nHeight;
        nHDist   = nDist;
    }
    else
    {
        nFHeight = nHeight;
        nFDist   = nDist;
    }

    // Aktuelle Werte der Seitenraender
    long nBT = aBspWin.GetTop();
    long nBB = aBspWin.GetBottom();
    long nBL = aBspWin.GetLeft();
    long nBR = aBspWin.GetRight();

    long nH  = aBspWin.GetSize().Height();
    long nW  = aBspWin.GetSize().Width();

    // Grenzen
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        // Header
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nMax = Max( nH - nMin - nHDist - nFDist - nFHeight - nBB - nBT,
                    nMin );
        aHeightEdit.SetMax( aHeightEdit.Normalize( nMax ), FUNIT_TWIP );
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nDist = Max( nH - nMin - nHHeight - nFDist - nFHeight - nBB - nBT,
                     long(0) );
        aDistEdit.SetMax( aDistEdit.Normalize( nDist ), FUNIT_TWIP );
    }
    else
    {
        // Footer
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nMax = Max( nH - nMin - nFDist - nHDist - nHHeight - nBT - nBB,
                    nMin );
        aHeightEdit.SetMax( aHeightEdit.Normalize( nMax ), FUNIT_TWIP );
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nDist = Max( nH - nMin - nFHeight - nHDist - nHHeight - nBT - nBB,
                     long(0) );
        aDistEdit.SetMax( aDistEdit.Normalize( nDist ), FUNIT_TWIP );
    }

    // Einzuege beschraenken
    nMax = nW - nBL - nBR -
           aRMEdit.Denormalize( aRMEdit.GetValue( FUNIT_TWIP ) ) - MINBODY;
    aLMEdit.SetMax( aLMEdit.Normalize( nMax ), FUNIT_TWIP );

    nMax = nW - nBL - nBR -
           aLMEdit.Denormalize( aLMEdit.GetValue( FUNIT_TWIP ) ) - MINBODY;
    aRMEdit.SetMax( aLMEdit.Normalize( nMax ), FUNIT_TWIP );
    return 0;
}


