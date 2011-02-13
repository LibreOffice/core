/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TPAREA_CXX

#if defined (UNX) || defined (WTC) || defined (ICC) || defined(WNT)
#include <stdlib.h>
#endif

#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <cuires.hrc>
#include "tabarea.hrc"
#include <svx/xflbckit.hxx>
#include <svx/svdattr.hxx>
#include <svx/xtable.hxx>
#include <svx/xlineit0.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "dlgname.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>

#define DLGWIN this->GetParent()->GetParent()

// static ----------------------------------------------------------------

static USHORT pAreaRanges[] =
{
    XATTR_GRADIENTSTEPCOUNT,
    XATTR_GRADIENTSTEPCOUNT,
    SID_ATTR_FILL_STYLE,
    SID_ATTR_FILL_BITMAP,
    0
};

static USHORT pTransparenceRanges[] =
{
    XATTR_FILLTRANSPARENCE,
    XATTR_FILLTRANSPARENCE,
    SDRATTR_SHADOWTRANSPARENCE,
    SDRATTR_SHADOWTRANSPARENCE,
    XATTR_FILLFLOATTRANSPARENCE,
    XATTR_FILLFLOATTRANSPARENCE,
    0
};

/*************************************************************************
|*
|*  Dialog for transparence
|*
\************************************************************************/

IMPL_LINK(SvxTransparenceTabPage, ClickTransOffHdl_Impl, void *, EMPTYARG)
{
    // disable all other controls
    ActivateLinear(FALSE);
    ActivateGradient(FALSE);

    // Preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );

    InvalidatePreview( FALSE );

    return( 0L );
}

IMPL_LINK(SvxTransparenceTabPage, ClickTransLinearHdl_Impl, void *, EMPTYARG)
{
    // enable linear, disable other
    ActivateLinear(TRUE);
    ActivateGradient(FALSE);

    // preview
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    ModifyTransparentHdl_Impl (NULL);

    return( 0L );
}

IMPL_LINK(SvxTransparenceTabPage, ClickTransGradientHdl_Impl, void *, EMPTYARG)
{
    // enable gradient, disable other
    ActivateLinear(FALSE);
    ActivateGradient(TRUE);

    // preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    ModifiedTrgrHdl_Impl (NULL);

    return( 0L );
}

void SvxTransparenceTabPage::ActivateLinear(BOOL bActivate)
{
    aMtrTransparent.Enable(bActivate);
}

IMPL_LINK(SvxTransparenceTabPage, ModifyTransparentHdl_Impl, void*, EMPTYARG)
{
    UINT16 nPos = (UINT16)aMtrTransparent.GetValue();
    XFillTransparenceItem aItem(nPos);
    rXFSet.Put(XFillTransparenceItem(aItem));

    // preview
    InvalidatePreview();

    return 0L;
}

IMPL_LINK(SvxTransparenceTabPage, ModifiedTrgrHdl_Impl, void *, pControl)
{
    if(pControl == &aLbTrgrGradientType || pControl == this)
    {
        XGradientStyle eXGS = (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos();
        SetControlState_Impl( eXGS );
    }

    // preview
    UINT8 nStartCol = (UINT8)(((UINT16)aMtrTrgrStartValue.GetValue() * 255) / 100);
    UINT8 nEndCol = (UINT8)(((UINT16)aMtrTrgrEndValue.GetValue() * 255) / 100);
    XGradient aTmpGradient(
                Color(nStartCol, nStartCol, nStartCol),
                Color(nEndCol, nEndCol, nEndCol),
                (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos(),
                (UINT16)aMtrTrgrAngle.GetValue() * 10,
                (UINT16)aMtrTrgrCenterX.GetValue(),
                (UINT16)aMtrTrgrCenterY.GetValue(),
                (UINT16)aMtrTrgrBorder.GetValue(),
                100, 100);

    String aString;
    XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aTmpGradient);
    rXFSet.Put ( aItem );

    InvalidatePreview();

    return( 0L );
}

void SvxTransparenceTabPage::ActivateGradient(BOOL bActivate)
{
    aFtTrgrType.Enable(bActivate);
    aLbTrgrGradientType.Enable(bActivate);
    aFtTrgrCenterX.Enable(bActivate);
    aMtrTrgrCenterX.Enable(bActivate);
    aFtTrgrCenterY.Enable(bActivate);
    aMtrTrgrCenterY.Enable(bActivate);
    aFtTrgrAngle.Enable(bActivate);
    aMtrTrgrAngle.Enable(bActivate);
    aFtTrgrBorder.Enable(bActivate);
    aMtrTrgrBorder.Enable(bActivate);
    aFtTrgrStartValue.Enable(bActivate);
    aMtrTrgrStartValue.Enable(bActivate);
    aFtTrgrEndValue.Enable(bActivate);
    aMtrTrgrEndValue.Enable(bActivate);

    if(bActivate)
    {
        XGradientStyle eXGS = (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos();
        SetControlState_Impl( eXGS );
    }
}

IMPL_LINK(SvxTransparenceTabPage, ChangeTrgrTypeHdl_Impl, void *, EMPTYARG)
{
    return( 0L );
}

void SvxTransparenceTabPage::SetControlState_Impl(XGradientStyle eXGS)
{
    switch(eXGS)
    {
        case XGRAD_LINEAR:
        case XGRAD_AXIAL:
            aFtTrgrCenterX.Disable();
            aMtrTrgrCenterX.Disable();
            aFtTrgrCenterY.Disable();
            aMtrTrgrCenterY.Disable();
            aFtTrgrAngle.Enable();
            aMtrTrgrAngle.Enable();
            break;

        case XGRAD_RADIAL:
            aFtTrgrCenterX.Enable();
            aMtrTrgrCenterX.Enable();
            aFtTrgrCenterY.Enable();
            aMtrTrgrCenterY.Enable();
            aFtTrgrAngle.Disable();
            aMtrTrgrAngle.Disable();
            break;

        case XGRAD_ELLIPTICAL:
            aFtTrgrCenterX.Enable();
            aMtrTrgrCenterX.Enable();
            aFtTrgrCenterY.Enable();
            aMtrTrgrCenterY.Enable();
            aFtTrgrAngle.Enable();
            aMtrTrgrAngle.Enable();
            break;

        case XGRAD_SQUARE:
        case XGRAD_RECT:
            aFtTrgrCenterX.Enable();
            aMtrTrgrCenterX.Enable();
            aFtTrgrCenterY.Enable();
            aMtrTrgrCenterY.Enable();
            aFtTrgrAngle.Enable();
            aMtrTrgrAngle.Enable();
            break;
    }
}

SvxTransparenceTabPage::SvxTransparenceTabPage(Window* pParent, const SfxItemSet& rInAttrs)
:   SvxTabPage          ( pParent, CUI_RES( RID_SVXPAGE_TRANSPARENCE ), rInAttrs),
    rOutAttrs           ( rInAttrs ),
    nPageType(0),
    nDlgType(0),

    aFlProp             ( this, CUI_RES( FL_PROP ) ),
    aRbtTransOff        ( this, CUI_RES( RBT_TRANS_OFF ) ),
    aRbtTransLinear     ( this, CUI_RES( RBT_TRANS_LINEAR ) ),
    aRbtTransGradient   ( this, CUI_RES( RBT_TRANS_GRADIENT ) ),

    aMtrTransparent     ( this, CUI_RES( MTR_TRANSPARENT ) ),

    aFtTrgrType         ( this, CUI_RES( FT_TRGR_TYPE ) ),
    aLbTrgrGradientType ( this, CUI_RES( LB_TRGR_GRADIENT_TYPES ) ),
    aFtTrgrCenterX      ( this, CUI_RES( FT_TRGR_CENTER_X ) ),
    aMtrTrgrCenterX     ( this, CUI_RES( MTR_TRGR_CENTER_X ) ),
    aFtTrgrCenterY      ( this, CUI_RES( FT_TRGR_CENTER_Y ) ),
    aMtrTrgrCenterY     ( this, CUI_RES( MTR_TRGR_CENTER_Y ) ),
    aFtTrgrAngle        ( this, CUI_RES( FT_TRGR_ANGLE ) ),
    aMtrTrgrAngle       ( this, CUI_RES( MTR_TRGR_ANGLE ) ),
    aFtTrgrBorder       ( this, CUI_RES( FT_TRGR_BORDER ) ),
    aMtrTrgrBorder      ( this, CUI_RES( MTR_TRGR_BORDER ) ),
    aFtTrgrStartValue   ( this, CUI_RES( FT_TRGR_START_VALUE ) ),
    aMtrTrgrStartValue  ( this, CUI_RES( MTR_TRGR_START_VALUE ) ),
    aFtTrgrEndValue     ( this, CUI_RES( FT_TRGR_END_VALUE ) ),
    aMtrTrgrEndValue    ( this, CUI_RES( MTR_TRGR_END_VALUE ) ),

    aCtlBitmapPreview   ( this, CUI_RES( CTL_BITMAP_PREVIEW ) ),
    aCtlXRectPreview    ( this, CUI_RES( CTL_TRANS_PREVIEW ) ),
    bBitmap             ( FALSE ),
    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    FreeResource();

    // main selection
    aRbtTransOff.SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransOffHdl_Impl));
    aRbtTransLinear.SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransLinearHdl_Impl));
    aRbtTransGradient.SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransGradientHdl_Impl));

    // linear transparency
    aMtrTransparent.SetValue( 50 );
    aMtrTransparent.SetModifyHdl(LINK(this, SvxTransparenceTabPage, ModifyTransparentHdl_Impl));

    // gradient transparency
    aMtrTrgrEndValue.SetValue( 100 );
    aMtrTrgrStartValue.SetValue( 0 );
    aLbTrgrGradientType.SetSelectHdl(LINK(this, SvxTransparenceTabPage, ChangeTrgrTypeHdl_Impl));
    Link aLink = LINK( this, SvxTransparenceTabPage, ModifiedTrgrHdl_Impl);
    aLbTrgrGradientType.SetSelectHdl( aLink );
    aMtrTrgrCenterX.SetModifyHdl( aLink );
    aMtrTrgrCenterY.SetModifyHdl( aLink );
    aMtrTrgrAngle.SetModifyHdl( aLink );
    aMtrTrgrBorder.SetModifyHdl( aLink );
    aMtrTrgrStartValue.SetModifyHdl( aLink );
    aMtrTrgrEndValue.SetModifyHdl( aLink );

    // this page needs ExchangeSupport
    SetExchangeSupport();
}

void SvxTransparenceTabPage::Construct()
{
}

SfxTabPage* SvxTransparenceTabPage::Create(Window* pWindow, const SfxItemSet& rAttrs)
{
    return(new SvxTransparenceTabPage(pWindow, rAttrs));
}

UINT16* SvxTransparenceTabPage::GetRanges()
{
    return(pTransparenceRanges);
}

BOOL SvxTransparenceTabPage::FillItemSet(SfxItemSet& rAttrs)
{
    const SfxPoolItem* pGradientItem = NULL;
    const SfxPoolItem* pLinearItem = NULL;
    SfxItemState eStateGradient(rOutAttrs.GetItemState(XATTR_FILLFLOATTRANSPARENCE, TRUE, &pGradientItem));
    SfxItemState eStateLinear(rOutAttrs.GetItemState(XATTR_FILLTRANSPARENCE, TRUE, &pLinearItem));
    BOOL bGradActive = (eStateGradient == SFX_ITEM_SET && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled());
    BOOL bLinearActive = (eStateLinear == SFX_ITEM_SET && ((XFillTransparenceItem*)pLinearItem)->GetValue() != 0);

    // #103765#
    BOOL bGradUsed = (eStateGradient == SFX_ITEM_DONTCARE);
    BOOL bLinearUsed = (eStateLinear == SFX_ITEM_DONTCARE);

    BOOL bModified(FALSE);
    BOOL bSwitchOffLinear(FALSE);
    BOOL bSwitchOffGradient(FALSE);

    if(aMtrTransparent.IsEnabled())
    {
        // linear transparence
        UINT16 nPos = (UINT16)aMtrTransparent.GetValue();
        if(nPos != (UINT16)aMtrTransparent.GetSavedValue().ToInt32() || !bLinearActive)
        {
            XFillTransparenceItem aItem(nPos);
            SdrShadowTransparenceItem aShadowItem(nPos);
            const SfxPoolItem* pOld = GetOldItem(rAttrs, XATTR_FILLTRANSPARENCE);
            if(!pOld || !(*(const XFillTransparenceItem*)pOld == aItem) || !bLinearActive)
            {
                rAttrs.Put(aItem);
                rAttrs.Put(aShadowItem);
                bModified = TRUE;
                bSwitchOffGradient = TRUE;
            }
        }
    }
    else if(aLbTrgrGradientType.IsEnabled())
    {
        // transparence gradient, fill ItemSet from values
        if(!bGradActive
            || (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos() != (XGradientStyle)aLbTrgrGradientType.GetSavedValue()
            || (UINT16)aMtrTrgrAngle.GetValue() != (UINT16)aMtrTrgrAngle.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrCenterX.GetValue() != (UINT16)aMtrTrgrCenterX.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrCenterY.GetValue() != (UINT16)aMtrTrgrCenterY.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrBorder.GetValue() != (UINT16)aMtrTrgrBorder.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrStartValue.GetValue() != (UINT16)aMtrTrgrStartValue.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrEndValue.GetValue() != (UINT16)aMtrTrgrEndValue.GetSavedValue().ToInt32() )
        {
            UINT8 nStartCol = (UINT8)(((UINT16)aMtrTrgrStartValue.GetValue() * 255) / 100);
            UINT8 nEndCol = (UINT8)(((UINT16)aMtrTrgrEndValue.GetValue() * 255) / 100);
            XGradient aTmpGradient(
                        Color(nStartCol, nStartCol, nStartCol),
                        Color(nEndCol, nEndCol, nEndCol),
                        (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos(),
                        (UINT16)aMtrTrgrAngle.GetValue() * 10,
                        (UINT16)aMtrTrgrCenterX.GetValue(),
                        (UINT16)aMtrTrgrCenterY.GetValue(),
                        (UINT16)aMtrTrgrBorder.GetValue(),
                        100, 100);

            String aString;
            XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aTmpGradient);
            const SfxPoolItem* pOld = GetOldItem(rAttrs, XATTR_FILLFLOATTRANSPARENCE);

            if(!pOld || !(*(const XFillFloatTransparenceItem*)pOld == aItem) || !bGradActive)
            {
                rAttrs.Put(aItem);
                bModified = TRUE;
                bSwitchOffLinear = TRUE;
            }
        }
    }
    else
    {
        // no transparence
        bSwitchOffGradient = TRUE;
        bSwitchOffLinear = TRUE;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffGradient && (bGradActive || bGradUsed))
    {
        Color aColor(COL_BLACK);
        XGradient aGrad(aColor, Color(COL_WHITE));
        aGrad.SetStartIntens(100);
        aGrad.SetEndIntens(100);
        String aString;
        XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aGrad);
        aItem.SetEnabled(FALSE);
        rAttrs.Put(aItem);
        bModified = TRUE;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffLinear && (bLinearActive || bLinearUsed))
    {
        XFillTransparenceItem aItem(0);
        SdrShadowTransparenceItem aShadowItem(0);
        rAttrs.Put(aItem);
        rAttrs.Put(aShadowItem);
        bModified = TRUE;
    }
    rAttrs.Put (CntUInt16Item(SID_PAGE_TYPE,nPageType));
    return bModified;
}

void SvxTransparenceTabPage::Reset(const SfxItemSet& rAttrs)
{
    const SfxPoolItem* pGradientItem = NULL;
    SfxItemState eStateGradient(rAttrs.GetItemState(XATTR_FILLFLOATTRANSPARENCE, TRUE, &pGradientItem));
    if(!pGradientItem)
        pGradientItem = &rAttrs.Get(XATTR_FILLFLOATTRANSPARENCE);
    BOOL bGradActive = (eStateGradient == SFX_ITEM_SET && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled());

    const SfxPoolItem* pLinearItem = NULL;
    SfxItemState eStateLinear(rAttrs.GetItemState(XATTR_FILLTRANSPARENCE, TRUE, &pLinearItem));
    if(!pLinearItem)
        pLinearItem = &rAttrs.Get(XATTR_FILLTRANSPARENCE);
    BOOL bLinearActive = (eStateLinear == SFX_ITEM_SET && ((XFillTransparenceItem*)pLinearItem)->GetValue() != 0);

    // transparence gradient
    const XGradient& rGradient = ((XFillFloatTransparenceItem*)pGradientItem)->GetGradientValue();
    XGradientStyle eXGS(rGradient.GetGradientStyle());
    aLbTrgrGradientType.SelectEntryPos(sal::static_int_cast< USHORT >(eXGS));
    aMtrTrgrAngle.SetValue(rGradient.GetAngle() / 10);
    aMtrTrgrBorder.SetValue(rGradient.GetBorder());
    aMtrTrgrCenterX.SetValue(rGradient.GetXOffset());
    aMtrTrgrCenterY.SetValue(rGradient.GetYOffset());
    aMtrTrgrStartValue.SetValue((UINT16)((((UINT16)rGradient.GetStartColor().GetRed() + 1) * 100) / 255));
    aMtrTrgrEndValue.SetValue((UINT16)((((UINT16)rGradient.GetEndColor().GetRed() + 1) * 100) / 255));

    // linear transparence
    UINT16 nTransp = ((XFillTransparenceItem*)pLinearItem)->GetValue();
    aMtrTransparent.SetValue(bLinearActive ? nTransp : 50);
    ModifyTransparentHdl_Impl(NULL);

    // select the correct radio button
    if(bGradActive)
    {
        // transparence gradient, set controls appropriate to item
        aRbtTransGradient.Check();
        ClickTransGradientHdl_Impl(NULL);
    }
    else if(bLinearActive)
    {
        // linear transparence
        aRbtTransLinear.Check();
        ClickTransLinearHdl_Impl(NULL);
    }
    else
    {
        // no transparence
        aRbtTransOff.Check();
        ClickTransOffHdl_Impl(NULL);
        ModifiedTrgrHdl_Impl(NULL);
    }

    // save values
    aMtrTransparent.SaveValue();
    aLbTrgrGradientType.SaveValue();
    aMtrTrgrCenterX.SaveValue();
    aMtrTrgrCenterY.SaveValue();
    aMtrTrgrAngle.SaveValue();
    aMtrTrgrBorder.SaveValue();
    aMtrTrgrStartValue.SaveValue();
    aMtrTrgrEndValue.SaveValue();

    BOOL bActive = InitPreview ( rAttrs );
    InvalidatePreview ( bActive );
}

void SvxTransparenceTabPage::ActivatePage(const SfxItemSet& rSet)
{
    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,CntUInt16Item,SID_PAGE_TYPE,sal_False);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());

    if(nDlgType == 0) // Flaechen-Dialog
        nPageType = PT_TRANSPARENCE;

    InitPreview ( rSet );
}

int SvxTransparenceTabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet( *_pSet );
    return(LEAVE_PAGE);
}

void SvxTransparenceTabPage::PointChanged(Window* , RECT_POINT eRcPt)
{
    eRP = eRcPt;
}

//
// Preview-Methods
//
BOOL SvxTransparenceTabPage::InitPreview ( const SfxItemSet& rSet )
{
    // set transparencetyp for preview
    if ( aRbtTransOff.IsChecked() )
    {
        ClickTransOffHdl_Impl(NULL);
    } else if ( aRbtTransLinear.IsChecked() )
    {
        ClickTransLinearHdl_Impl(NULL);
    } else if ( aRbtTransGradient.IsChecked() )
    {
        ClickTransGradientHdl_Impl(NULL);
    }

    // Get fillstyle for preview
    rXFSet.Put ( ( XFillStyleItem& )    rSet.Get(XATTR_FILLSTYLE) );
    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillGradientItem& ) rSet.Get(XATTR_FILLGRADIENT) );
    rXFSet.Put ( ( XFillHatchItem& )    rSet.Get(XATTR_FILLHATCH) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    rXFSet.Put ( ( XFillBitmapItem& )   rSet.Get(XATTR_FILLBITMAP) );

    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );

    bBitmap = ( ( ( XFillStyleItem& )rSet.Get(XATTR_FILLSTYLE) ).GetValue() == XFILL_BITMAP );

    // show the right preview window
    if ( bBitmap )
    {
        aCtlBitmapPreview.Show();
        aCtlXRectPreview.Hide();
    }
    else
    {
        aCtlBitmapPreview.Hide();
        aCtlXRectPreview.Show();
    }

    return !aRbtTransOff.IsChecked();
}

void SvxTransparenceTabPage::InvalidatePreview (BOOL bEnable)
{
    if ( bBitmap )
    {
        if ( bEnable )
        {
            aCtlBitmapPreview.Enable();
            aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );
        }
        else
            aCtlBitmapPreview.Disable();
        aCtlBitmapPreview.Invalidate();
    }
    else
    {
        if ( bEnable )
        {
            aCtlXRectPreview.Enable();
            aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
        }
        else
            aCtlXRectPreview.Disable();
        aCtlXRectPreview.Invalidate();
    }
}

void SvxTransparenceTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,sal_False);

    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    Construct();
}
/*************************************************************************
|*
|*  Dialog to modify fill-attributes
|*
\************************************************************************/

SvxAreaTabPage::SvxAreaTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage          ( pParent, CUI_RES( RID_SVXPAGE_AREA ), rInAttrs ),

    aFlProp             ( this, CUI_RES( FL_PROP ) ),
    aTypeLB             ( this, CUI_RES( LB_AREA_TYPE ) ),

    aLbColor            ( this, CUI_RES( LB_COLOR ) ),
    aLbGradient         ( this, CUI_RES( LB_GRADIENT ) ),
    aLbHatching         ( this, CUI_RES( LB_HATCHING ) ),
    aLbBitmap           ( this, CUI_RES( LB_BITMAP ) ),
    aCtlBitmapPreview   ( this, CUI_RES( CTL_BITMAP_PREVIEW ) ),

    aTsbStepCount       ( this, CUI_RES( TSB_STEPCOUNT ) ),
    aFlStepCount        ( this, CUI_RES( FL_STEPCOUNT ) ),
    aNumFldStepCount    ( this, CUI_RES( NUM_FLD_STEPCOUNT ) ),

    aCbxHatchBckgrd     ( this, CUI_RES( CB_HATCHBCKGRD ) ),
    aLbHatchBckgrdColor ( this, CUI_RES( LB_HATCHBCKGRDCOLOR ) ),

    aFlSize             ( this, CUI_RES( FL_SIZE ) ),
    aTsbOriginal        ( this, CUI_RES( TSB_ORIGINAL ) ),
    aTsbScale           ( this, CUI_RES( TSB_SCALE ) ),
    aFtXSize            ( this, CUI_RES( FT_X_SIZE ) ),
    aMtrFldXSize        ( this, CUI_RES( MTR_FLD_X_SIZE ) ),
    aFtYSize            ( this, CUI_RES( FT_Y_SIZE ) ),
    aMtrFldYSize        ( this, CUI_RES( MTR_FLD_Y_SIZE ) ),
    aFlPosition         ( this, CUI_RES( FL_POSITION ) ),
    aCtlPosition        ( this, CUI_RES( CTL_POSITION ), RP_RM, 110, 80, CS_RECT ),
    aFtXOffset          ( this, CUI_RES( FT_X_OFFSET ) ),
    aMtrFldXOffset      ( this, CUI_RES( MTR_FLD_X_OFFSET ) ),
    aFtYOffset          ( this, CUI_RES( FT_Y_OFFSET ) ),
    aMtrFldYOffset      ( this, CUI_RES( MTR_FLD_Y_OFFSET ) ),
    aTsbTile            ( this, CUI_RES( TSB_TILE ) ),
    aTsbStretch         ( this, CUI_RES( TSB_STRETCH ) ),
    aFlOffset           ( this, CUI_RES( FL_OFFSET ) ),
    aRbtRow             ( this, CUI_RES( RBT_ROW ) ),
    aRbtColumn          ( this, CUI_RES( RBT_COLUMN ) ),
    aMtrFldOffset       ( this, CUI_RES( MTR_FLD_OFFSET ) ),

    aCtlXRectPreview    ( this, CUI_RES( CTL_COLOR_PREVIEW ) ),

    rOutAttrs           ( rInAttrs ),

    pColorTab( NULL ),
    pGradientList( NULL ),
    pHatchingList( NULL ),
    pBitmapList( NULL ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    FreeResource();

    // Gruppen, die sich ueberlagern
    aLbBitmap.Hide();
    aCtlBitmapPreview.Hide();

    aFlStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aFlSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aFlOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aFlPosition.Hide();
    // Controls for Hatch-Background
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();


    aTsbOriginal.EnableTriState( FALSE );


    // this page needs ExchangeSupport
    SetExchangeSupport();

    // set Metrics
    eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
        break;
        default: ;//prevent warning
    }
    SetFieldUnit( aMtrFldXSize, eFUnit, TRUE );
    SetFieldUnit( aMtrFldYSize, eFUnit, TRUE );

    // get PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( XATTR_FILLBMP_SIZEX );

    // Setzen Output-Devices
    rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
    rXFSet.Put( XFillColorItem( String(), COL_BLACK ) );
    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );

    aLbColor.SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyColorHdl_Impl ) );
    aLbHatchBckgrdColor.SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyHatchBckgrdColorHdl_Impl ) );
    aCbxHatchBckgrd.SetToggleHdl( LINK( this, SvxAreaTabPage, ToggleHatchBckgrdColorHdl_Impl ) );

    aLbGradient.SetSelectHdl(
        LINK( this, SvxAreaTabPage, ModifyGradientHdl_Impl ) );
    aLbHatching.SetSelectHdl(
        LINK( this, SvxAreaTabPage, ModifyHatchingHdl_Impl ) );
    aLbBitmap.SetSelectHdl(
        LINK( this, SvxAreaTabPage, ModifyBitmapHdl_Impl ) );

    aTsbStepCount.SetClickHdl(
        LINK( this, SvxAreaTabPage, ModifyStepCountHdl_Impl ) );
    aNumFldStepCount.SetModifyHdl(
        LINK( this, SvxAreaTabPage, ModifyStepCountHdl_Impl ) );

    Link aLink( LINK( this, SvxAreaTabPage, ModifyTileHdl_Impl ) );
    aTsbTile.SetClickHdl( aLink );
    aTsbStretch.SetClickHdl( aLink );
    aTsbOriginal.SetClickHdl( aLink );
    aMtrFldXSize.SetModifyHdl( aLink );
    aMtrFldYSize.SetModifyHdl( aLink );
    aRbtRow.SetClickHdl( aLink );
    aRbtColumn.SetClickHdl( aLink );
    aMtrFldOffset.SetModifyHdl( aLink );
    aMtrFldXOffset.SetModifyHdl( aLink );
    aMtrFldYOffset.SetModifyHdl( aLink );
    aTsbScale.SetClickHdl( LINK( this, SvxAreaTabPage, ClickScaleHdl_Impl ) );

    aTypeLB.SetSelectHdl( LINK( this, SvxAreaTabPage, SelectDialogTypeHdl_Impl ) );

    // #i76307# always paint the preview in LTR, because this is what the document does
    aCtlXRectPreview.EnableRTL(FALSE);
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::Construct()
{
    // fill colortables / lists
    aLbColor.Fill( pColorTab );
    aLbHatchBckgrdColor.Fill ( pColorTab );

    aLbGradient.Fill( pGradientList );
    aLbHatching.Fill( pHatchingList );
    aLbBitmap.Fill( pBitmapList );
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    USHORT nCount;
    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&rSet,pPosItem,SfxUInt16Item,SID_TABPAGE_POS,sal_False);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pPosItem)
        SetPos(pPosItem->GetValue());
    if( nDlgType == 0 ) // Flaechen-Dialog
    {
        *pbAreaTP = TRUE;

        if( pColorTab )
        {
            USHORT _nPos = 0;
            // Bitmapliste
            if( *pnBitmapListState )
            {
                if( *pnBitmapListState & CT_CHANGED )
                    pBitmapList = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewBitmapList();

                _nPos = aLbBitmap.GetSelectEntryPos();

                aLbBitmap.Clear();
                aLbBitmap.Fill( pBitmapList );
                nCount = aLbBitmap.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    aLbBitmap.SelectEntryPos( 0 );
                else
                    aLbBitmap.SelectEntryPos( _nPos );
                ModifyBitmapHdl_Impl( this );
            }
            // hatch-liste
            if( *pnHatchingListState )
            {
                if( *pnHatchingListState & CT_CHANGED )
                    pHatchingList = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewHatchingList();

                _nPos = aLbHatching.GetSelectEntryPos();

                aLbHatching.Clear();
                aLbHatching.Fill( pHatchingList );
                nCount = aLbHatching.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    aLbHatching.SelectEntryPos( 0 );
                else
                    aLbHatching.SelectEntryPos( _nPos );
                ModifyHatchingHdl_Impl( this );

                ModifyHatchBckgrdColorHdl_Impl( this );
            }
            // gradient-liste
            if( *pnGradientListState )
            {
                if( *pnGradientListState & CT_CHANGED )
                    pGradientList = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewGradientList();

                _nPos = aLbGradient.GetSelectEntryPos();

                aLbGradient.Clear();
                aLbGradient.Fill( pGradientList );
                nCount = aLbGradient.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    aLbGradient.SelectEntryPos( 0 );
                else
                    aLbGradient.SelectEntryPos( _nPos );
                ModifyGradientHdl_Impl( this );
            }
            // ColorTable
            if( *pnColorTableState )
            {
                if( *pnColorTableState & CT_CHANGED )
                    pColorTab = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewColorTable();
                // aLbColor
                _nPos = aLbColor.GetSelectEntryPos();
                aLbColor.Clear();
                aLbColor.Fill( pColorTab );
                nCount = aLbColor.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    aLbColor.SelectEntryPos( 0 );
                else
                    aLbColor.SelectEntryPos( _nPos );

                ModifyColorHdl_Impl( this );

                // Backgroundcolor of hatch
                _nPos = aLbHatchBckgrdColor.GetSelectEntryPos();
                aLbHatchBckgrdColor.Clear();
                aLbHatchBckgrdColor.Fill( pColorTab );
                nCount = aLbHatchBckgrdColor.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    aLbHatchBckgrdColor.SelectEntryPos( 0 );
                else
                    aLbHatchBckgrdColor.SelectEntryPos( _nPos );

                ModifyHatchBckgrdColorHdl_Impl( this );
            }

            // evaluate if any other Tabpage set another filltype
            if( aTypeLB.GetSelectEntryPos() > XFILL_NONE)
            {
                switch( nPageType )
                {
                    case PT_GRADIENT:
                        aTypeLB.SelectEntryPos( XFILL_GRADIENT );
                        aLbGradient.SelectEntryPos( _nPos );
                        ClickGradientHdl_Impl( this );
                    break;

                    case PT_HATCH:
                        aTypeLB.SelectEntryPos( XFILL_HATCH );
                        aLbHatching.SelectEntryPos( _nPos );
                        ClickHatchingHdl_Impl( this );
                    break;

                    case PT_BITMAP:
                        aTypeLB.SelectEntryPos( XFILL_BITMAP );
                        aLbBitmap.SelectEntryPos( _nPos );
                        ClickBitmapHdl_Impl( this );
                    break;

                    case PT_COLOR:
                        aTypeLB.SelectEntryPos( XFILL_SOLID );
                        aLbColor.SelectEntryPos( _nPos );
                        aLbHatchBckgrdColor.SelectEntryPos( _nPos );
                        ClickColorHdl_Impl( this );
                    break;
                }
            }
            nPageType = PT_AREA;
        }
    }
}

// -----------------------------------------------------------------------

int SvxAreaTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( nDlgType == 0 ) // Flaechen-Dialog
    {
        XFillStyle eStyle = (XFillStyle) aTypeLB.GetSelectEntryPos();
        switch( eStyle )
        {
            case XFILL_GRADIENT:
            {
                        nPageType = PT_GRADIENT;
                        nPos = aLbGradient.GetSelectEntryPos();
            }
            break;
            case XFILL_HATCH:
            {
                nPageType = PT_HATCH;
                nPos = aLbHatching.GetSelectEntryPos();
            }
            break;
            case XFILL_BITMAP:
            {
                nPageType = PT_BITMAP;
                nPos = aLbBitmap.GetSelectEntryPos();
            }
            break;
            case XFILL_SOLID:
            {
                nPageType = PT_COLOR;
                nPos = aLbColor.GetSelectEntryPos();
            }
            break;
            default: ;//prevent warning
        }
    }

    if( _pSet )
        FillItemSet( *_pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

BOOL SvxAreaTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    const SfxPoolItem* pOld = NULL;
    USHORT  _nPos;
    BOOL    bModified = FALSE;

    if( nDlgType != 0 || *pbAreaTP )
    {
        XFillStyle eStyle = (XFillStyle) aTypeLB.GetSelectEntryPos();
        XFillStyle eSavedStyle = (XFillStyle) aTypeLB.GetSavedValue();
        switch( eStyle )
        {
            case XFILL_NONE:
            {
                if(  eSavedStyle != eStyle )
                {
                    XFillStyleItem aStyleItem( XFILL_NONE );
                    pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                    {
                        rAttrs.Put( aStyleItem );
                        bModified = TRUE;
                    }
                }
            }
            break;
            case XFILL_SOLID:
            {
                 _nPos = aLbColor.GetSelectEntryPos();
                 if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                     _nPos != aLbColor.GetSavedValue() )
                 {
                     XFillColorItem aItem( aLbColor.GetSelectEntry(),
                                           aLbColor.GetSelectEntryColor() );
                     pOld = GetOldItem( rAttrs, XATTR_FILLCOLOR );
                     if ( !pOld || !( *(const XFillColorItem*)pOld == aItem ) )
                     {
                         rAttrs.Put( aItem );
                         bModified = TRUE;
                     }
                 }
                 // NEU
                 if( (eSavedStyle != eStyle) &&
                     ( bModified ||
                       SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE ) ) )
                 {
                     XFillStyleItem aStyleItem( XFILL_SOLID );
                     pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                     if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                     {
                         rAttrs.Put( aStyleItem );
                         bModified = TRUE;
                     }
                 }
            }
            break;
            case XFILL_GRADIENT:
            {
                _nPos = aLbGradient.GetSelectEntryPos();
                if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                    _nPos != aLbGradient.GetSavedValue() )
                {
                    XGradient aGradient = pGradientList->GetGradient( _nPos )->GetGradient();
                    String aString = aLbGradient.GetSelectEntry();
                    XFillGradientItem aItem( aString, aGradient );
                    pOld = GetOldItem( rAttrs, XATTR_FILLGRADIENT );
                    if ( !pOld || !( *(const XFillGradientItem*)pOld == aItem ) )
                    {
                        rAttrs.Put( aItem );
                        bModified = TRUE;
                    }
                }
                // NEU
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), TRUE ) ) )
                {
                    XFillStyleItem aStyleItem( XFILL_GRADIENT );
                    pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                    {
                        rAttrs.Put( aStyleItem );
                        bModified = TRUE;
                    }
                }
            }
            break;
            case XFILL_HATCH:
            {
                _nPos = aLbHatching.GetSelectEntryPos();
                if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                   _nPos != aLbHatching.GetSavedValue() )
                {
                    XHatch aHatching = pHatchingList->GetHatch( _nPos )->GetHatch();
                    String aString = aLbHatching.GetSelectEntry();
                    XFillHatchItem aItem( aString, aHatching );
                    pOld = GetOldItem( rAttrs, XATTR_FILLHATCH );
                    if ( !pOld || !( *(const XFillHatchItem*)pOld == aItem ) )
                    {
                        rAttrs.Put( aItem );
                        bModified = TRUE;
                    }
                }
                XFillBackgroundItem aItem ( aCbxHatchBckgrd.IsChecked() );
                rAttrs.Put( aItem );
                nPos = aLbHatchBckgrdColor.GetSelectEntryPos();
                if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                     nPos != aLbHatchBckgrdColor.GetSavedValue() )
                {
                    XFillColorItem aFillColorItem( aLbHatchBckgrdColor.GetSelectEntry(),
                                          aLbHatchBckgrdColor.GetSelectEntryColor() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLCOLOR );
                    if ( !pOld || !( *(const XFillColorItem*)pOld == aFillColorItem ) )
                    {
                        rAttrs.Put( aFillColorItem );
                        bModified = TRUE;
                    }
                }
                // NEU
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), TRUE ) ) )
                {
                    XFillStyleItem aStyleItem( XFILL_HATCH );
                    pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                    {
                        rAttrs.Put( aStyleItem );
                        bModified = TRUE;
                    }
                }
            }
            break;
            case XFILL_BITMAP:
            {
                nPos = aLbBitmap.GetSelectEntryPos();
                if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                    nPos != aLbBitmap.GetSavedValue() )
                {
                    XOBitmap aXOBitmap = pBitmapList->GetBitmap( nPos )->GetXBitmap();
                    String aString = aLbBitmap.GetSelectEntry();
                    XFillBitmapItem aFillBitmapItem( aString, aXOBitmap );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBITMAP );
                    if ( !pOld || !( *(const XFillBitmapItem*)pOld == aFillBitmapItem ) )
                    {
                        rAttrs.Put( aFillBitmapItem );
                        bModified = TRUE;
                    }
                }
                // NEU
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), TRUE ) ) )
                {
                    XFillStyleItem aStyleItem( XFILL_BITMAP );
                    pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                    {
                        rAttrs.Put( aStyleItem );
                        bModified = TRUE;
                    }
               }
           }
           break;
       }

        // Schrittweite
        if( aTsbStepCount.IsEnabled() )
        {
            UINT16 nValue = 0;
            BOOL   bValueModified = FALSE;
            TriState eState = aTsbStepCount.GetState();
            if( eState == STATE_CHECK )
            {
                if( eState != aTsbStepCount.GetSavedValue() )
                    bValueModified = TRUE;
            }
            else
            {
                // Zustand != Disabled ?
                if( aNumFldStepCount.GetText().Len() > 0 )
                {
                    nValue = (UINT16) aNumFldStepCount.GetValue();
                    if( nValue != (UINT16) aNumFldStepCount.GetSavedValue().ToInt32() )
                        bValueModified = TRUE;
                }
            }
            if( bValueModified )
            {
                XGradientStepCountItem aFillBitmapItem( nValue );
                pOld = GetOldItem( rAttrs, XATTR_GRADIENTSTEPCOUNT );
                if ( !pOld || !( *(const XGradientStepCountItem*)pOld == aFillBitmapItem ) )
                {
                    rAttrs.Put( aFillBitmapItem );
                    bModified = TRUE;
                }
            }
        }

        // Kacheln
        if( aTsbTile.IsEnabled() )
        {
            TriState eState = aTsbTile.GetState();
            if( eState != aTsbTile.GetSavedValue() )
            {
                XFillBmpTileItem aFillBmpTileItem(
                    sal::static_int_cast< BOOL >( eState ) );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILE );
                if ( !pOld || !( *(const XFillBmpTileItem*)pOld == aFillBmpTileItem ) )
                {
                    rAttrs.Put( aFillBmpTileItem );
                    bModified = TRUE;
                }
            }
        }
        // Stretchen
        if( aTsbStretch.IsEnabled() )
        {
            TriState eState = aTsbStretch.GetState();
            if( eState != aTsbStretch.GetSavedValue() )
            {
                XFillBmpStretchItem aFillBmpStretchItem(
                    sal::static_int_cast< BOOL >( eState ) );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_STRETCH );
                if ( !pOld || !( *(const XFillBmpStretchItem*)pOld == aFillBmpStretchItem ) )
                {
                    rAttrs.Put( aFillBmpStretchItem );
                    bModified = TRUE;
                }
            }
        }

        // Originalgroesse (im UI) wird wie folgt benutzt:
        // Controls sind disabled, muessen aber gesetzt werden.
        // SizeX = 0; SizeY = 0; Log = TRUE

        //aTsbScale
        TriState eState = aTsbScale.GetState();
        if( eState != aTsbScale.GetSavedValue() ||
            ( !aTsbScale.IsEnabled() &&
              aTsbOriginal.IsEnabled() &&
              aTsbScale.GetSavedValue() != STATE_CHECK ) )
        {
            XFillBmpSizeLogItem* pItem = NULL;
            if( aTsbScale.IsEnabled() )
                pItem = new XFillBmpSizeLogItem( eState == STATE_NOCHECK );
            else if( aTsbOriginal.IsEnabled() && aTsbOriginal.GetState() == STATE_CHECK )
                pItem = new XFillBmpSizeLogItem( TRUE );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZELOG );
                if ( !pOld || !( *(const XFillBmpSizeLogItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = TRUE;
                }
                delete pItem;
            }
        }

        //aMtrFldXSize
        String aStr = aMtrFldXSize.GetText();
        {
            XFillBmpSizeXItem* pItem = NULL;
            TriState eScaleState = aTsbScale.GetState();

            if( aMtrFldXSize.IsEnabled() &&
                aStr.Len() > 0  &&
                aStr != aMtrFldXSize.GetSavedValue() )
            {
                if( eScaleState == STATE_NOCHECK )
                    pItem = new XFillBmpSizeXItem( GetCoreValue( aMtrFldXSize, ePoolUnit ) );
                else
                {
                    // Prozentwerte werden negativ gesetzt, damit
                    // diese nicht skaliert werden; dieses wird
                    // im Item beruecksichtigt
                    pItem = new XFillBmpSizeXItem( -labs( static_cast<long>(aMtrFldXSize.GetValue()) ) );
                }
            }
            else if( aTsbOriginal.IsEnabled() &&
                     aTsbOriginal.GetState() == STATE_CHECK &&
                     aMtrFldXSize.GetSavedValue().Len() > 0 )
                pItem = new XFillBmpSizeXItem( 0 );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZEX );
                if ( !pOld || !( *(const XFillBmpSizeXItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = TRUE;
                }
                delete pItem;
            }
        }

        //aMtrFldYSize
        aStr = aMtrFldYSize.GetText();
        {
            XFillBmpSizeYItem* pItem = NULL;
            TriState eScaleState = aTsbScale.GetState();

            if( aMtrFldYSize.IsEnabled() &&
                aStr.Len() > 0  &&
                aStr != aMtrFldYSize.GetSavedValue() )
            {
                if( eScaleState == STATE_NOCHECK )
                    pItem = new XFillBmpSizeYItem( GetCoreValue( aMtrFldYSize, ePoolUnit ) );
                else
                {
                    // Prozentwerte werden negativ gesetzt, damit
                    // diese vom MetricItem nicht skaliert werden;
                    // dieses wird im Item beruecksichtigt
                    pItem = new XFillBmpSizeYItem( -labs( static_cast<long>(aMtrFldYSize.GetValue()) ) );
                }
            }
            else if( aTsbOriginal.IsEnabled() &&
                     aTsbOriginal.GetState() == STATE_CHECK &&
                     aMtrFldYSize.GetSavedValue().Len() > 0 )
                pItem = new XFillBmpSizeYItem( 0 );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZEY );
                if ( !pOld || !( *(const XFillBmpSizeYItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = TRUE;
                }
                delete pItem;
            }
        }

        //aRbtRow
        //aRbtColumn
        //aMtrFldOffset
        if( aMtrFldOffset.IsEnabled() )
        {
            String aMtrString = aMtrFldOffset.GetText();
            if( ( aMtrString.Len() > 0  &&
                  aMtrString != aMtrFldOffset.GetSavedValue() ) ||
                  aRbtRow.GetSavedValue() != aRbtRow.IsChecked() ||
                  aRbtColumn.GetSavedValue() != aRbtColumn.IsChecked() )
            {
                if( aRbtRow.IsChecked() )
                {
                    XFillBmpTileOffsetXItem aFillBmpTileOffsetXItem( (UINT16) aMtrFldOffset.GetValue() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILEOFFSETX );
                    if ( !pOld || !( *(const XFillBmpTileOffsetXItem*)pOld == aFillBmpTileOffsetXItem ) )
                    {
                        rAttrs.Put( aFillBmpTileOffsetXItem );
                        rAttrs.Put( XFillBmpTileOffsetYItem( 0 ) );
                        bModified = TRUE;
                    }
                }
                else if( aRbtColumn.IsChecked() )
                {
                    XFillBmpTileOffsetYItem aFillBmpTileOffsetYItem( (UINT16) aMtrFldOffset.GetValue() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILEOFFSETY );
                    if ( !pOld || !( *(const XFillBmpTileOffsetYItem*)pOld == aFillBmpTileOffsetYItem ) )
                    {
                        rAttrs.Put( aFillBmpTileOffsetYItem );
                        rAttrs.Put( XFillBmpTileOffsetXItem( 0 ) );
                        bModified = TRUE;
                    }
                }
            }
        }

        //aCtlPosition
        if( aCtlPosition.IsEnabled() )
        {
            BOOL bPut = FALSE;
            RECT_POINT _eRP = aCtlPosition.GetActualRP();

            if( SFX_ITEM_DONTCARE == rOutAttrs.GetItemState( XATTR_FILLBMP_POS ) )
                bPut = TRUE;
            else
            {
                RECT_POINT eValue = ( ( const XFillBmpPosItem& ) rOutAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
                if( eValue != _eRP )
                    bPut = TRUE;
            }
            if( bPut )
            {
                XFillBmpPosItem aFillBmpPosItem( _eRP );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POS );
                if ( !pOld || !( *(const XFillBmpPosItem*)pOld == aFillBmpPosItem ) )
                {
                    rAttrs.Put( aFillBmpPosItem );
                    bModified = TRUE;
                }
            }
        }

        //aMtrFldXOffset
        if( aMtrFldXOffset.IsEnabled() )
        {
            String sMtrXOffset = aMtrFldXOffset.GetText();
            if( sMtrXOffset.Len() > 0  &&
                sMtrXOffset != aMtrFldXOffset.GetSavedValue() )
            {
                XFillBmpPosOffsetXItem aFillBmpPosOffsetXItem( (UINT16) aMtrFldXOffset.GetValue() );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POSOFFSETX );
                if ( !pOld || !( *(const XFillBmpPosOffsetXItem*)pOld == aFillBmpPosOffsetXItem ) )
                {
                    rAttrs.Put( aFillBmpPosOffsetXItem );
                    bModified = TRUE;
                }
            }
        }

        //aMtrFldYOffset
        if( aMtrFldYOffset.IsEnabled() )
        {
            String sMtrYOffset = aMtrFldYOffset.GetText();
            if( sMtrYOffset.Len() > 0  &&
                sMtrYOffset != aMtrFldYOffset.GetSavedValue() )
            {
                XFillBmpPosOffsetYItem aFillBmpPosOffsetYItem( (UINT16) aMtrFldYOffset.GetValue() );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POSOFFSETY );
                if ( !pOld || !( *(const XFillBmpPosOffsetYItem*)pOld == aFillBmpPosOffsetYItem ) )
                {
                    rAttrs.Put( aFillBmpPosOffsetYItem );
                    bModified = TRUE;
                }
            }
        }
        rAttrs.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
        rAttrs.Put (SfxUInt16Item(SID_TABPAGE_POS,nPos));
    }

    return( bModified );
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::Reset( const SfxItemSet& rAttrs )
{
    XFillStyle eXFS;
    if( rAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE )
    {
        eXFS = (XFillStyle) ( ( ( const XFillStyleItem& ) rAttrs.
                                Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
        aTypeLB.SelectEntryPos( sal::static_int_cast< USHORT >( eXFS ) );
        switch( eXFS )
        {
            case XFILL_NONE:
                ClickInvisibleHdl_Impl( this );
            break;

            case XFILL_SOLID:
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rAttrs.Get( XATTR_FILLCOLOR ) );

                    aLbColor.SelectEntry( aColorItem.GetColorValue() );
                    aLbHatchBckgrdColor.SelectEntry( aColorItem.GetColorValue() );
                }
                ClickColorHdl_Impl( this );

            break;

            case XFILL_GRADIENT:
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLGRADIENT ) )
                {
                    XFillGradientItem aGradientItem( ( ( const XFillGradientItem& )
                                            rAttrs.Get( XATTR_FILLGRADIENT ) ) );
                    String    aString( aGradientItem.GetName() );
                    XGradient aGradient( aGradientItem.GetGradientValue() );

                    aLbGradient.SelectEntryByList( pGradientList, aString, aGradient );
                }
                ClickGradientHdl_Impl( this );
            break;

            case XFILL_HATCH:
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    aLbHatching.SelectEntry( ( ( const XFillHatchItem& )
                                    rAttrs.Get( XATTR_FILLHATCH ) ).GetName() );
                }
                ClickHatchingHdl_Impl( this );

                if ( SFX_ITEM_DONTCARE != rAttrs.GetItemState ( XATTR_FILLBACKGROUND ) )
                {
                    aCbxHatchBckgrd.Check ( ( ( const XFillBackgroundItem& ) rAttrs.Get ( XATTR_FILLBACKGROUND ) ).GetValue() );
                }
                ToggleHatchBckgrdColorHdl_Impl( this );
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rAttrs.Get( XATTR_FILLCOLOR ) );

                    aLbColor.SelectEntry( aColorItem.GetColorValue() );
                    aLbHatchBckgrdColor.SelectEntry( aColorItem.GetColorValue() );
                }
            break;

            case XFILL_BITMAP:
            {
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLBITMAP ) )
                {
                    XFillBitmapItem aBitmapItem( ( const XFillBitmapItem& )
                                        rAttrs.Get( XATTR_FILLBITMAP ) );

                    String aString( aBitmapItem.GetName() );
                    aLbBitmap.SelectEntry( aString );
                }
                ClickBitmapHdl_Impl( this );
            }
            break;

            default:
            break;
        }
    }
    else
    {
        // Alle LBs nicht zug"anglich machen
        aLbColor.Hide();
        aLbGradient.Hide();
        aLbHatching.Hide();
        aLbBitmap.Hide();
        aCtlBitmapPreview.Hide();
        aLbColor.Disable();
        aLbColor.Show();

        // Damit Reset() auch mit Zurueck richtig funktioniert
        aTypeLB.SetNoSelection();
    }

    // Schrittweite
    if( ( rAttrs.GetItemState( XATTR_GRADIENTSTEPCOUNT ) != SFX_ITEM_DONTCARE ) ||
        ( rAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE ) )
    {
        aTsbStepCount.EnableTriState( FALSE );
        UINT16 nValue = ( ( const XGradientStepCountItem& ) rAttrs.Get( XATTR_GRADIENTSTEPCOUNT ) ).GetValue();
        if( nValue == 0 )
        {
            aTsbStepCount.SetState( STATE_CHECK );
            aNumFldStepCount.SetText( String() );
        }
        else
        {
            aTsbStepCount.SetState( STATE_NOCHECK );
            aNumFldStepCount.SetValue( nValue );
        }
        ModifyStepCountHdl_Impl( &aTsbStepCount );
    }
    else
    {
        aTsbStepCount.SetState( STATE_DONTKNOW );
        aNumFldStepCount.SetText( String() );
    }

    // Attribute fuer die Bitmap-Fuellung

    // Ist Kacheln gesetzt?
    if( rAttrs.GetItemState( XATTR_FILLBMP_TILE ) != SFX_ITEM_DONTCARE )
    {
        aTsbTile.EnableTriState( FALSE );

        if( ( ( const XFillBmpTileItem& ) rAttrs.Get( XATTR_FILLBMP_TILE ) ).GetValue() )
            aTsbTile.SetState( STATE_CHECK );
        else
            aTsbTile.SetState( STATE_NOCHECK );
    }
    else
        aTsbTile.SetState( STATE_DONTKNOW );

    // Ist Stretchen gesetzt?
    if( rAttrs.GetItemState( XATTR_FILLBMP_STRETCH ) != SFX_ITEM_DONTCARE )
    {
        aTsbStretch.EnableTriState( FALSE );

        if( ( ( const XFillBmpStretchItem& ) rAttrs.Get( XATTR_FILLBMP_STRETCH ) ).GetValue() )
            aTsbStretch.SetState( STATE_CHECK );
        else
            aTsbStretch.SetState( STATE_NOCHECK );
    }
    else
        aTsbStretch.SetState( STATE_DONTKNOW );


    //aTsbScale
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZELOG ) != SFX_ITEM_DONTCARE )
    {
        aTsbScale.EnableTriState( FALSE );

        if( ( ( const XFillBmpSizeLogItem& ) rAttrs.Get( XATTR_FILLBMP_SIZELOG ) ).GetValue() )
            aTsbScale.SetState( STATE_NOCHECK );
        else
            aTsbScale.SetState( STATE_CHECK );

        ClickScaleHdl_Impl( NULL );
    }
    else
        aTsbScale.SetState( STATE_DONTKNOW );


    // Status fuer Originalgroesse ermitteln
    TriState eOriginal = STATE_NOCHECK;

    //aMtrFldXSize
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZEX ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpSizeXItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        if( aTsbScale.GetState() == STATE_CHECK )
        {
            // Wenn im Item eine Prozentangabe steckt,
            // so ist diese wegen des MetricItems negativ
            aMtrFldXSize.SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( aMtrFldXSize, nValue, ePoolUnit );
        aMtrFldXSize.SaveValue();

        if( nValue == 0 )
        {
            eOriginal = STATE_CHECK;
            // Wert ist beim Ausschalten von Originalgroesse sonst zu klein
            // (Performance-Problem)
            aMtrFldXSize.SetValue( 100 );
        }
    }
    else
    {
        aMtrFldXSize.SetText( String() );
        aMtrFldXSize.SaveValue();
    }

    //aMtrFldYSize
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZEY ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpSizeYItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEY ) ).GetValue();
        if( aTsbScale.GetState() == STATE_CHECK )
        {
            // Wenn im Item eine Prozentangabe steckt,
            // so ist diese wegen des MetricItems negativ
            aMtrFldYSize.SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( aMtrFldYSize, nValue, ePoolUnit );
        aMtrFldYSize.SaveValue();

        if( nValue == 0 )
            aMtrFldYSize.SetValue( 100 ); //s.o.
        else
            eOriginal = STATE_NOCHECK;
    }
    else
    {
        aMtrFldYSize.SetText( String() );
        aMtrFldYSize.SaveValue();
        eOriginal = STATE_NOCHECK;
    }

    // aTsbOriginal
    aTsbOriginal.SetState( eOriginal );

    // #93372# Setting proper state after changing button
    ModifyTileHdl_Impl( NULL );

    //aRbtRow
    //aRbtColumn
    //aMtrFldOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_TILEOFFSETX ) != SFX_ITEM_DONTCARE )
    {
        UINT16 nValue = ( ( const XFillBmpTileOffsetXItem& ) rAttrs.Get( XATTR_FILLBMP_TILEOFFSETX ) ).GetValue();
        if( nValue > 0 )
        {
            aMtrFldOffset.SetValue( nValue );
            aRbtRow.Check();
        }
        else if( rAttrs.GetItemState( XATTR_FILLBMP_TILEOFFSETY ) != SFX_ITEM_DONTCARE )
        {
            nValue = ( ( const XFillBmpTileOffsetYItem& ) rAttrs.Get( XATTR_FILLBMP_TILEOFFSETY ) ).GetValue();
            if( nValue > 0 )
            {
                aMtrFldOffset.SetValue( nValue );
                aRbtColumn.Check();
            }
        }
        else
            aMtrFldOffset.SetValue( 0 );
    }
    else
        aMtrFldOffset.SetText( String() );


    //aCtlPosition
    if( rAttrs.GetItemState( XATTR_FILLBMP_POS ) != SFX_ITEM_DONTCARE )
    {
        RECT_POINT eValue = ( ( const XFillBmpPosItem& ) rAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
        aCtlPosition.SetActualRP( eValue );
    }
    else
        aCtlPosition.Reset();

    //aMtrFldXOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_POSOFFSETX ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpPosOffsetXItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETX ) ).GetValue();
        aMtrFldXOffset.SetValue( nValue );
    }
    else
        aMtrFldXOffset.SetText( String() );

    //aMtrFldYOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_POSOFFSETY ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpPosOffsetYItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETY ) ).GetValue();
        aMtrFldYOffset.SetValue( nValue );
    }
    else
        aMtrFldYOffset.SetText( String() );

    // Erst hier, damit Tile und Stretch mit beruecksichtigt wird
    if( aTypeLB.GetSelectEntryPos() == XFILL_BITMAP )
        ClickBitmapHdl_Impl( NULL );

    // Werte sichern
    aTypeLB.SaveValue();
    aLbColor.SaveValue();
    aLbGradient.SaveValue();
    aLbHatching.SaveValue();
    aLbHatchBckgrdColor.SaveValue();
    aLbBitmap.SaveValue();
    aTsbStepCount.SaveValue();
    aNumFldStepCount.SaveValue();
    aTsbTile.SaveValue();
    aTsbStretch.SaveValue();
    aTsbScale.SaveValue();
    aRbtRow.SaveValue();
    aRbtColumn.SaveValue();
    aMtrFldOffset.SaveValue();
    aMtrFldXOffset.SaveValue();
    aMtrFldYOffset.SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* SvxAreaTabPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxAreaTabPage( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxAreaTabPage::GetRanges()
{
    return( pAreaRanges );
}

//------------------------------------------------------------------------
IMPL_LINK( SvxAreaTabPage, SelectDialogTypeHdl_Impl, ListBox *, EMPTYARG )
{
    switch( (XFillStyle)aTypeLB.GetSelectEntryPos() )
    {
        case XFILL_NONE: ClickInvisibleHdl_Impl( this ); break;
        case XFILL_SOLID: ClickColorHdl_Impl( this ); break;
        case XFILL_GRADIENT: ClickGradientHdl_Impl( this ); break;
        case XFILL_HATCH: ClickHatchingHdl_Impl( this ); break;
        case XFILL_BITMAP: ClickBitmapHdl_Impl( this ); break;
    }

    return 0;
}

IMPL_LINK( SvxAreaTabPage, ClickInvisibleHdl_Impl, void *, EMPTYARG )
{
    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aFlSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aFlOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aFlPosition.Hide();

    aLbColor.Hide();
    aLbGradient.Hide();
    aLbHatching.Hide();
    aLbBitmap.Hide();
    aCtlXRectPreview.Hide();
    aCtlBitmapPreview.Hide();

    aFlStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    // Controls for Hatch-Background
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();

    rXFSet.Put( XFillStyleItem( XFILL_NONE ) );
    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );

    aCtlXRectPreview.Invalidate();
    aCtlBitmapPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickColorHdl_Impl, void *, EMPTYARG )
{
    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aFlSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aFlOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aFlPosition.Hide();

    aLbColor.Enable();
    aLbColor.Show();
    aLbGradient.Hide();
    aLbHatching.Hide();
    aLbBitmap.Hide();
    aCtlXRectPreview.Enable();
    aCtlXRectPreview.Show();
    aCtlBitmapPreview.Hide();

    aFlStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    // Controls for Hatch-Background
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();

    // Text der Tabelle setzen
    String          aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pColorTab->GetPath() );

    aURL.Append( pColorTab->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += String(aURL.getBase()).Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += String(aURL.getBase());

    ModifyColorHdl_Impl( this );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyColorHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT _nPos = aLbColor.GetSelectEntryPos();
    aLbHatchBckgrdColor.SelectEntryPos( _nPos );
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        rXFSet.Put( XFillColorItem( String(),
                                    aLbColor.GetSelectEntryColor() ) );
    }
    // NEU
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        Color aColor( ( ( const XFillColorItem* ) pPoolItem )->GetColorValue() );
        rXFSet.Put( XFillColorItem( String(), aColor ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}
//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickGradientHdl_Impl, void *, EMPTYARG )
{
    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aFlSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aFlOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aFlPosition.Hide();

    aLbColor.Hide();
    aLbGradient.Enable();
    aLbGradient.Show();
    aLbHatching.Hide();
    aLbBitmap.Hide();
    aCtlXRectPreview.Enable();
    aCtlXRectPreview.Show();
    aCtlBitmapPreview.Hide();

    aFlStepCount.Enable();
    aFlStepCount.Show();
    aTsbStepCount.Enable();
    aTsbStepCount.Show();
    aNumFldStepCount.Show();

    // Controls for Hatch-Background
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();

    // Text der Tabelle setzen
    String          aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pGradientList->GetPath() );

    aURL.Append( pGradientList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += String(aURL.getBase()).Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += String(aURL.getBase());

    ModifyGradientHdl_Impl( this );
    ModifyStepCountHdl_Impl( &aTsbStepCount );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyGradientHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT _nPos = aLbGradient.GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // ItemSet fuellen und an aCtlXRectPreview weiterleiten
        XGradientEntry* pEntry = pGradientList->GetGradient( _nPos );

        rXFSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
        rXFSet.Put( XFillGradientItem( String(),
                                       pEntry->GetGradient() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
        rXFSet.Put( XFillGradientItem( String(), ( ( const XFillGradientItem* ) pPoolItem )->GetGradientValue() ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickHatchingHdl_Impl, void *, EMPTYARG )
{
    aLbColor.Hide();
    aLbGradient.Hide();
    aLbHatching.Enable();
    aLbHatching.Show();
    aLbBitmap.Hide();
    aCtlXRectPreview.Enable();
    aCtlXRectPreview.Show();
    aCtlBitmapPreview.Hide();

    aFlStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aFlSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aFlOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aFlPosition.Hide();

    // Controls for Hatch-Background
    aCbxHatchBckgrd.Show();
    aLbHatchBckgrdColor.Show();
    aCbxHatchBckgrd.Enable();
    aLbHatchBckgrdColor.Enable();

    // Text der Tabelle setzen
    String          aString( CUI_RES( RID_SVXSTR_TABLE ) );     aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pHatchingList->GetPath() );

    aURL.Append( pHatchingList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += String(String(aURL.getBase()).Copy( 0, 15 ));
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += String(aURL.getBase());

    ModifyHatchingHdl_Impl( this );
    ModifyHatchBckgrdColorHdl_Impl( this );
    ToggleHatchBckgrdColorHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyHatchingHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT _nPos = aLbHatching.GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // ItemSet fuellen und an aCtlXRectPreview weiterleiten
        XHatchEntry* pEntry = pHatchingList->GetHatch( _nPos );

        rXFSet.Put( XFillStyleItem( XFILL_HATCH ) );
        rXFSet.Put( XFillHatchItem( String(), pEntry->GetHatch() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_HATCH ) );
        rXFSet.Put( XFillHatchItem( String(), ( ( const XFillHatchItem* ) pPoolItem )->GetHatchValue() ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyHatchBckgrdColorHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT _nPos = aLbHatchBckgrdColor.GetSelectEntryPos();
    aLbColor.SelectEntryPos( _nPos );
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillColorItem( String(),
                                    aLbHatchBckgrdColor.GetSelectEntryColor() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE, &pPoolItem ) )
    {
        Color aColor( ( ( const XFillColorItem* ) pPoolItem )->GetColorValue() );
        rXFSet.Put( XFillColorItem( String(), aColor ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ToggleHatchBckgrdColorHdl_Impl, void *, EMPTYARG )
{
    // switch on/off backgroundcolor for hatches
    aLbHatchBckgrdColor.Enable( aCbxHatchBckgrd.IsChecked() );

    XFillBackgroundItem aItem( aCbxHatchBckgrd.IsChecked() );
    rXFSet.Put ( aItem, XATTR_FILLBACKGROUND );

    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    if( aLbHatchBckgrdColor.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
    {
        if ( SFX_ITEM_SET == rOutAttrs.GetItemState( XATTR_FILLCOLOR ) )//>= SFX_ITEM_DEFAULT )
        {
            XFillColorItem aColorItem( (const XFillColorItem&)rOutAttrs.Get( XATTR_FILLCOLOR ) );
            aLbHatchBckgrdColor.SelectEntry( aColorItem.GetColorValue() );
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickBitmapHdl_Impl, void *, EMPTYARG )
{
    aLbColor.Hide();
    aLbGradient.Hide();
    aLbHatching.Hide();
    aLbBitmap.Enable();
    aLbBitmap.Show();
    aCtlBitmapPreview.Enable();
    aCtlBitmapPreview.Show();
    aCtlXRectPreview.Hide();
    aFlStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    aTsbTile.Enable();
    aTsbStretch.Enable();
    aTsbScale.Enable();
    aTsbOriginal.Enable();
    aFtXSize.Enable();
    aMtrFldXSize.Enable();
    aFtYSize.Enable();
    aMtrFldYSize.Enable();
    aFlSize.Enable();
    aCtlPosition.Enable();
    aFtXOffset.Enable();
    aMtrFldXOffset.Enable();
    aFtYOffset.Enable();
    aMtrFldYOffset.Enable();
    aFlPosition.Enable();
    aRbtRow.Enable();
    aRbtColumn.Enable();
    aMtrFldOffset.Enable();
    aFlOffset.Enable();

    // Controls for Hatch-Background
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();

    aTsbTile.Show();
    aTsbStretch.Show();
    aTsbScale.Show();
    aTsbOriginal.Show();
    aFtXSize.Show();
    aMtrFldXSize.Show();
    aFtYSize.Show();
    aMtrFldYSize.Show();
    aFlSize.Show();
    aCtlPosition.Show();
    aFtXOffset.Show();
    aMtrFldXOffset.Show();
    aFtYOffset.Show();
    aMtrFldYOffset.Show();
    aFlPosition.Show();
    aRbtRow.Show();
    aRbtColumn.Show();
    aMtrFldOffset.Show();
    aFlOffset.Show();

    // Text der Tabelle setzen
    String          aString( CUI_RES( RID_SVXSTR_TABLE ) );     aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pBitmapList->GetPath() );

    aURL.Append( pBitmapList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += String(aURL.getBase()).Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += String(aURL.getBase());

    ModifyBitmapHdl_Impl( this );
    ModifyTileHdl_Impl( &aTsbOriginal );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyBitmapHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT _nPos = aLbBitmap.GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // ItemSet fuellen und an aCtlXRectPreview weiterleiten
        XBitmapEntry* pEntry = pBitmapList->GetBitmap( _nPos );

        rXFSet.Put( XFillStyleItem( XFILL_BITMAP ) );
        rXFSet.Put( XFillBitmapItem( String(), pEntry->GetXBitmap() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_BITMAP ) );
        rXFSet.Put( XFillBitmapItem( String(), ( ( const XFillBitmapItem* ) pPoolItem )->GetBitmapValue() ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyStepCountHdl_Impl, void *, p )
{
    if( p == &aTsbStepCount )
    {
        if( aTsbStepCount.GetState() == STATE_NOCHECK )
        {
            if( aNumFldStepCount.GetText().Len() == 0 )
                aNumFldStepCount.SetText( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "64") ));

            aNumFldStepCount.Enable();
        }
        else
            aNumFldStepCount.Disable();
    }

    UINT16 nValue = 0;
    if( aTsbStepCount.GetState() != STATE_CHECK )
    {
        // Zustand != Disabled ?
        if( aNumFldStepCount.GetText().Len() > 0 )
            nValue = (UINT16) aNumFldStepCount.GetValue();
    }
    rXFSet.Put( XGradientStepCountItem( nValue ) );
    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyTileHdl_Impl, void *, EMPTYARG )
{
    TriState eState = aTsbTile.GetState();
    if( eState == STATE_CHECK )
    {
        aTsbStretch.Disable();
        aRbtRow.Enable();
        aRbtColumn.Enable();
        aMtrFldOffset.Enable();
        aFlOffset.Enable();

        aCtlPosition.Enable();
        aCtlPosition.Invalidate();
        aFtXOffset.Enable();
        aMtrFldXOffset.Enable();
        aFtYOffset.Enable();
        aMtrFldYOffset.Enable();
        aFlPosition.Enable();

        aTsbScale.Enable();
        aTsbOriginal.Enable();
        aFtXSize.Enable();
        aMtrFldXSize.Enable();
        aFtYSize.Enable();
        aMtrFldYSize.Enable();
        aFlSize.Enable();
    }
    else if( eState == STATE_NOCHECK )
    {
        aTsbStretch.Enable();
        aRbtRow.Disable();
        aRbtColumn.Disable();
        aMtrFldOffset.Disable();
        aFlOffset.Disable();

        aCtlPosition.Disable();
        aCtlPosition.Invalidate();
        aFtXOffset.Disable();
        aMtrFldXOffset.Disable();
        aFtYOffset.Disable();
        aMtrFldYOffset.Disable();
        aFlPosition.Disable();

        if( aTsbStretch.GetState() != STATE_NOCHECK )
        {
            aTsbScale.Disable();
            aTsbOriginal.Disable();
            aFtXSize.Disable();
            aMtrFldXSize.Disable();
            aFtYSize.Disable();
            aMtrFldYSize.Disable();
            aFlSize.Disable();
        }
        else
        {
            aTsbScale.Enable();
            aTsbOriginal.Enable();
            aFtXSize.Enable();
            aMtrFldXSize.Enable();
            aFtYSize.Enable();
            aMtrFldYSize.Enable();
            aFlSize.Enable();
        }
    }
    else
    {
        aTsbStretch.Disable();
        aRbtRow.Disable();
        aRbtColumn.Disable();
        aMtrFldOffset.Disable();
        aFlOffset.Disable();

        aCtlPosition.Disable();
        aCtlPosition.Invalidate();
        aFtXOffset.Disable();
        aMtrFldXOffset.Disable();
        aFtYOffset.Disable();
        aMtrFldYOffset.Disable();
        aFlPosition.Disable();

        aTsbScale.Disable();
        aTsbOriginal.Disable();
        aFtXSize.Disable();
        aMtrFldXSize.Disable();
        aFtYSize.Disable();
        aMtrFldYSize.Disable();
        aFlSize.Disable();
    }

    if( aTsbOriginal.GetState() == STATE_CHECK )
    {
        aMtrFldXSize.SetText( String() );
        aMtrFldYSize.SetText( String() );
        aFtXSize.Disable();
        aFtYSize.Disable();
        aMtrFldXSize.Disable();
        aMtrFldYSize.Disable();
        aTsbScale.Disable();
    }
    else
    {
        aMtrFldXSize.SetValue( aMtrFldXSize.GetValue() );
        aMtrFldYSize.SetValue( aMtrFldYSize.GetValue() );
    }

    rXFSet.Put( XFillBmpTileItem( sal::static_int_cast< BOOL >( eState ) ) );

    if( aTsbStretch.IsEnabled() )
        rXFSet.Put(
            XFillBmpStretchItem(
                sal::static_int_cast< BOOL >( aTsbStretch.GetState() ) ) );

    if( aTsbScale.IsEnabled() )
        rXFSet.Put( XFillBmpSizeLogItem( aTsbScale.GetState() == STATE_NOCHECK ) );

    if( aMtrFldXSize.IsEnabled() )
    {
        XFillBmpSizeXItem* pItem = NULL;
        TriState eScaleState = aTsbScale.GetState();

        if( eScaleState == STATE_NOCHECK )
            pItem = new XFillBmpSizeXItem( GetCoreValue( aMtrFldXSize, ePoolUnit ) );
        else
            pItem = new XFillBmpSizeXItem( -labs( static_cast<long>(aMtrFldXSize.GetValue()) ) );

        rXFSet.Put( *pItem );

        delete pItem;
    }
    else if( aTsbOriginal.IsEnabled() && aTsbOriginal.GetState() == STATE_CHECK )
    {
        // Originalgroesse -> Size == 0
        rXFSet.Put( XFillBmpSizeXItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( TRUE ) );
    }

    if( aMtrFldYSize.IsEnabled() )
    {
        XFillBmpSizeYItem* pItem = NULL;
        TriState eScaleState = aTsbScale.GetState();

        if( eScaleState == STATE_NOCHECK )
            pItem = new XFillBmpSizeYItem( GetCoreValue( aMtrFldYSize, ePoolUnit ) );
        else
            pItem = new XFillBmpSizeYItem( -labs( static_cast<long>(aMtrFldYSize.GetValue()) ) );

        rXFSet.Put( *pItem );

        delete pItem;
    }
    else if( aTsbOriginal.IsEnabled() && aTsbOriginal.GetState() == STATE_CHECK )
    {
        // Originalgroesse -> Size == 0
        rXFSet.Put( XFillBmpSizeYItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( TRUE ) );
    }

    if( aMtrFldOffset.IsEnabled() )
    {
        if( aRbtRow.IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (UINT16) aMtrFldOffset.GetValue() ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (UINT16) 0 ) );
        }
        else if( aRbtColumn.IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (UINT16) 0 ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (UINT16) aMtrFldOffset.GetValue() ) );
        }
    }

    if( aCtlPosition.IsEnabled() )
        rXFSet.Put( XFillBmpPosItem( aCtlPosition.GetActualRP() ) );

    if( aMtrFldXOffset.IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetXItem( (UINT16) aMtrFldXOffset.GetValue() ) );

    if( aMtrFldYOffset.IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetYItem( (UINT16) aMtrFldYOffset.GetValue() ) );


    aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickScaleHdl_Impl, void *, EMPTYARG )
{
    if( aTsbScale.GetState() == STATE_CHECK )
    {
        aMtrFldXSize.SetDecimalDigits( 0 );
        aMtrFldXSize.SetUnit( FUNIT_CUSTOM );
        aMtrFldXSize.SetValue( 100 );
        aMtrFldXSize.SetMax( 100 );
        aMtrFldXSize.SetLast( 100 );

        aMtrFldYSize.SetDecimalDigits( 0 );
        aMtrFldYSize.SetUnit( FUNIT_CUSTOM );
        aMtrFldYSize.SetValue( 100 );
        aMtrFldYSize.SetMax( 100 );
        aMtrFldYSize.SetLast( 100 );
    }
    else
    {
        aMtrFldXSize.SetDecimalDigits( 2 );
        aMtrFldXSize.SetUnit( eFUnit );
        aMtrFldXSize.SetValue( 100 );
        aMtrFldXSize.SetMax( 999900 );
        aMtrFldXSize.SetLast( 100000 );

        aMtrFldYSize.SetDecimalDigits( 2 );
        aMtrFldYSize.SetUnit( eFUnit );
        aMtrFldYSize.SetValue( 100 );
        aMtrFldYSize.SetMax( 999900 );
        aMtrFldYSize.SetLast( 100000 );
    }

    ModifyTileHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

void SvxAreaTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    eRP = eRcPt;

    // Ausrichtung der Bitmapfuellung
    ModifyTileHdl_Impl( pWindow );
}

void SvxAreaTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pColorTabItem,SvxColorTableItem,SID_COLOR_TABLE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pGradientListItem,SvxGradientListItem,SID_GRADIENT_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pHatchingListItem,SvxHatchListItem,SID_HATCH_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pBitmapListItem,SvxBitmapListItem,SID_BITMAP_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPosItem,SfxUInt16Item,SID_TABPAGE_POS,sal_False);

    if (pColorTabItem)
        SetColorTable(pColorTabItem->GetColorTable());
    if (pGradientListItem)
        SetGradientList(pGradientListItem->GetGradientList());
    if (pHatchingListItem)
        SetHatchingList(pHatchingListItem->GetHatchList());
    if (pBitmapListItem)
        SetBitmapList(pBitmapListItem->GetBitmapList());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    if (pPosItem)
        SetPos(pPosItem->GetValue());
    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
