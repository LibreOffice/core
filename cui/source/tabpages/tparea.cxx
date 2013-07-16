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

#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>

#if defined (UNX) || defined (ICC) || defined(WNT)
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
#include "paragrph.hrc"

// static ----------------------------------------------------------------

static sal_uInt16 pAreaRanges[] =
{
    XATTR_GRADIENTSTEPCOUNT,
    XATTR_GRADIENTSTEPCOUNT,
    SID_ATTR_FILL_STYLE,
    SID_ATTR_FILL_BITMAP,
    0
};

static sal_uInt16 pTransparenceRanges[] =
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

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransOffHdl_Impl)
{
    // disable all other controls
    ActivateLinear(sal_False);
    ActivateGradient(sal_False);

    // Preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );

    InvalidatePreview( sal_False );

    return( 0L );
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransLinearHdl_Impl)
{
    // enable linear, disable other
    ActivateLinear(sal_True);
    ActivateGradient(sal_False);

    // preview
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    ModifyTransparentHdl_Impl (NULL);

    return( 0L );
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransGradientHdl_Impl)
{
    // enable gradient, disable other
    ActivateLinear(sal_False);
    ActivateGradient(sal_True);

    // preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    ModifiedTrgrHdl_Impl (NULL);

    return( 0L );
}

void SvxTransparenceTabPage::ActivateLinear(sal_Bool bActivate)
{
    m_pMtrTransparent->Enable(bActivate);
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ModifyTransparentHdl_Impl)
{
    sal_uInt16 nPos = (sal_uInt16)m_pMtrTransparent->GetValue();
    XFillTransparenceItem aItem(nPos);
    rXFSet.Put(XFillTransparenceItem(aItem));

    // preview
    InvalidatePreview();

    return 0L;
}

IMPL_LINK(SvxTransparenceTabPage, ModifiedTrgrHdl_Impl, void *, pControl)
{
    if(pControl == m_pLbTrgrGradientType || pControl == this)
    {
        XGradientStyle eXGS = (XGradientStyle)m_pLbTrgrGradientType->GetSelectEntryPos();
        SetControlState_Impl( eXGS );
    }

    // preview
    sal_uInt8 nStartCol = (sal_uInt8)(((sal_uInt16)m_pMtrTrgrStartValue->GetValue() * 255) / 100);
    sal_uInt8 nEndCol = (sal_uInt8)(((sal_uInt16)m_pMtrTrgrEndValue->GetValue() * 255) / 100);
    XGradient aTmpGradient(
                Color(nStartCol, nStartCol, nStartCol),
                Color(nEndCol, nEndCol, nEndCol),
                (XGradientStyle)m_pLbTrgrGradientType->GetSelectEntryPos(),
                (sal_uInt16)m_pMtrTrgrAngle->GetValue() * 10,
                (sal_uInt16)m_pMtrTrgrCenterX->GetValue(),
                (sal_uInt16)m_pMtrTrgrCenterY->GetValue(),
                (sal_uInt16)m_pMtrTrgrBorder->GetValue(),
                100, 100);

    XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aTmpGradient);
    rXFSet.Put ( aItem );

    InvalidatePreview();

    return( 0L );
}

void SvxTransparenceTabPage::ActivateGradient(sal_Bool bActivate)
{
    m_pGridGradient->Enable(bActivate);

    if(bActivate)
    {
        XGradientStyle eXGS = (XGradientStyle)m_pLbTrgrGradientType->GetSelectEntryPos();
        SetControlState_Impl( eXGS );
    }
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ChangeTrgrTypeHdl_Impl)
{
    return( 0L );
}

void SvxTransparenceTabPage::SetControlState_Impl(XGradientStyle eXGS)
{
    switch(eXGS)
    {
        case XGRAD_LINEAR:
        case XGRAD_AXIAL:
            m_pFtTrgrCenterX->Disable();
            m_pMtrTrgrCenterX->Disable();
            m_pFtTrgrCenterY->Disable();
            m_pMtrTrgrCenterY->Disable();
            m_pFtTrgrAngle->Enable();
            m_pMtrTrgrAngle->Enable();
            break;

        case XGRAD_RADIAL:
            m_pFtTrgrCenterX->Enable();
            m_pMtrTrgrCenterX->Enable();
            m_pFtTrgrCenterY->Enable();
            m_pMtrTrgrCenterY->Enable();
            m_pFtTrgrAngle->Disable();
            m_pMtrTrgrAngle->Disable();
            break;

        case XGRAD_ELLIPTICAL:
            m_pFtTrgrCenterX->Enable();
            m_pMtrTrgrCenterX->Enable();
            m_pFtTrgrCenterY->Enable();
            m_pMtrTrgrCenterY->Enable();
            m_pFtTrgrAngle->Enable();
            m_pMtrTrgrAngle->Enable();
            break;

        case XGRAD_SQUARE:
        case XGRAD_RECT:
            m_pFtTrgrCenterX->Enable();
            m_pMtrTrgrCenterX->Enable();
            m_pFtTrgrCenterY->Enable();
            m_pMtrTrgrCenterY->Enable();
            m_pFtTrgrAngle->Enable();
            m_pMtrTrgrAngle->Enable();
            break;
    }
}

SvxTransparenceTabPage::SvxTransparenceTabPage(Window* pParent, const SfxItemSet& rInAttrs)
:   SvxTabPage          ( pParent,
                          "TransparencyTabPage",
                          "cui/ui/transparencytabpage.ui",
                          rInAttrs),
    rOutAttrs           ( rInAttrs ),
    eRP                 ( RP_LT ),
    nPageType           (0),
    nDlgType            (0),
    bBitmap             ( sal_False ),
    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    get(m_pRbtTransOff,"RBT_TRANS_OFF");
    get(m_pRbtTransLinear,"RBT_TRANS_LINEAR");
    get(m_pRbtTransGradient,"RBT_TRANS_GRADIENT");

    get(m_pMtrTransparent,"MTR_TRANSPARENT");

    get(m_pGridGradient,"gridGradient");
    get(m_pLbTrgrGradientType,"LB_TRGR_GRADIENT_TYPES");
    get(m_pFtTrgrCenterX,"FT_TRGR_CENTER_X");
    get(m_pMtrTrgrCenterX,"MTR_TRGR_CENTER_X");
    get(m_pFtTrgrCenterY,"FT_TRGR_CENTER_Y");
    get(m_pMtrTrgrCenterY,"MTR_TRGR_CENTER_Y");
    get(m_pFtTrgrAngle,"FT_TRGR_ANGLE"),
    get(m_pMtrTrgrAngle,"MTR_TRGR_ANGLE");
    get(m_pMtrTrgrBorder,"MTR_TRGR_BORDER");
    get(m_pMtrTrgrStartValue,"MTR_TRGR_START_VALUE");
    get(m_pMtrTrgrEndValue,"MTR_TRGR_END_VALUE");

    get(m_pCtlBitmapPreview,"CTL_BITMAP_PREVIEW");
    get(m_pCtlXRectPreview,"CTL_TRANS_PREVIEW");

    // main selection
    m_pRbtTransOff->SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransOffHdl_Impl));
    m_pRbtTransLinear->SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransLinearHdl_Impl));
    m_pRbtTransGradient->SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransGradientHdl_Impl));

    // linear transparency
    m_pMtrTransparent->SetValue( 50 );
    m_pMtrTransparent->SetModifyHdl(LINK(this, SvxTransparenceTabPage, ModifyTransparentHdl_Impl));

    // gradient transparency
    m_pMtrTrgrEndValue->SetValue( 100 );
    m_pMtrTrgrStartValue->SetValue( 0 );
    m_pLbTrgrGradientType->SetSelectHdl(LINK(this, SvxTransparenceTabPage, ChangeTrgrTypeHdl_Impl));
    Link aLink = LINK( this, SvxTransparenceTabPage, ModifiedTrgrHdl_Impl);
    m_pLbTrgrGradientType->SetSelectHdl( aLink );
    m_pMtrTrgrCenterX->SetModifyHdl( aLink );
    m_pMtrTrgrCenterY->SetModifyHdl( aLink );
    m_pMtrTrgrAngle->SetModifyHdl( aLink );
    m_pMtrTrgrBorder->SetModifyHdl( aLink );
    m_pMtrTrgrStartValue->SetModifyHdl( aLink );
    m_pMtrTrgrEndValue->SetModifyHdl( aLink );

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

sal_uInt16* SvxTransparenceTabPage::GetRanges()
{
    return(pTransparenceRanges);
}

sal_Bool SvxTransparenceTabPage::FillItemSet(SfxItemSet& rAttrs)
{
    const SfxPoolItem* pGradientItem = NULL;
    const SfxPoolItem* pLinearItem = NULL;
    SfxItemState eStateGradient(rOutAttrs.GetItemState(XATTR_FILLFLOATTRANSPARENCE, sal_True, &pGradientItem));
    SfxItemState eStateLinear(rOutAttrs.GetItemState(XATTR_FILLTRANSPARENCE, sal_True, &pLinearItem));
    sal_Bool bGradActive = (eStateGradient == SFX_ITEM_SET && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled());
    sal_Bool bLinearActive = (eStateLinear == SFX_ITEM_SET && ((XFillTransparenceItem*)pLinearItem)->GetValue() != 0);

    // #103765#
    sal_Bool bGradUsed = (eStateGradient == SFX_ITEM_DONTCARE);
    sal_Bool bLinearUsed = (eStateLinear == SFX_ITEM_DONTCARE);

    sal_Bool bModified(sal_False);
    sal_Bool bSwitchOffLinear(sal_False);
    sal_Bool bSwitchOffGradient(sal_False);

    if(m_pMtrTransparent->IsEnabled())
    {
        // linear transparence
        sal_uInt16 nPos = (sal_uInt16)m_pMtrTransparent->GetValue();
        if(nPos != (sal_uInt16)m_pMtrTransparent->GetSavedValue().toInt32() || !bLinearActive)
        {
            XFillTransparenceItem aItem(nPos);
            SdrShadowTransparenceItem aShadowItem(nPos);
            const SfxPoolItem* pOld = GetOldItem(rAttrs, XATTR_FILLTRANSPARENCE);
            if(!pOld || !(*(const XFillTransparenceItem*)pOld == aItem) || !bLinearActive)
            {
                rAttrs.Put(aItem);
                rAttrs.Put(aShadowItem);
                bModified = sal_True;
                bSwitchOffGradient = sal_True;
            }
        }
    }
    else if(m_pLbTrgrGradientType->IsEnabled())
    {
        // transparence gradient, fill ItemSet from values
        if(!bGradActive
            || (XGradientStyle)m_pLbTrgrGradientType->GetSelectEntryPos() != (XGradientStyle)m_pLbTrgrGradientType->GetSavedValue()
            || (sal_uInt16)m_pMtrTrgrAngle->GetValue() != (sal_uInt16)m_pMtrTrgrAngle->GetSavedValue().toInt32()
            || (sal_uInt16)m_pMtrTrgrCenterX->GetValue() != (sal_uInt16)m_pMtrTrgrCenterX->GetSavedValue().toInt32()
            || (sal_uInt16)m_pMtrTrgrCenterY->GetValue() != (sal_uInt16)m_pMtrTrgrCenterY->GetSavedValue().toInt32()
            || (sal_uInt16)m_pMtrTrgrBorder->GetValue() != (sal_uInt16)m_pMtrTrgrBorder->GetSavedValue().toInt32()
            || (sal_uInt16)m_pMtrTrgrStartValue->GetValue() != (sal_uInt16)m_pMtrTrgrStartValue->GetSavedValue().toInt32()
            || (sal_uInt16)m_pMtrTrgrEndValue->GetValue() != (sal_uInt16)m_pMtrTrgrEndValue->GetSavedValue().toInt32() )
        {
            sal_uInt8 nStartCol = (sal_uInt8)(((sal_uInt16)m_pMtrTrgrStartValue->GetValue() * 255) / 100);
            sal_uInt8 nEndCol = (sal_uInt8)(((sal_uInt16)m_pMtrTrgrEndValue->GetValue() * 255) / 100);
            XGradient aTmpGradient(
                        Color(nStartCol, nStartCol, nStartCol),
                        Color(nEndCol, nEndCol, nEndCol),
                        (XGradientStyle)m_pLbTrgrGradientType->GetSelectEntryPos(),
                        (sal_uInt16)m_pMtrTrgrAngle->GetValue() * 10,
                        (sal_uInt16)m_pMtrTrgrCenterX->GetValue(),
                        (sal_uInt16)m_pMtrTrgrCenterY->GetValue(),
                        (sal_uInt16)m_pMtrTrgrBorder->GetValue(),
                        100, 100);

            XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aTmpGradient);
            const SfxPoolItem* pOld = GetOldItem(rAttrs, XATTR_FILLFLOATTRANSPARENCE);

            if(!pOld || !(*(const XFillFloatTransparenceItem*)pOld == aItem) || !bGradActive)
            {
                rAttrs.Put(aItem);
                bModified = sal_True;
                bSwitchOffLinear = sal_True;
            }
        }
    }
    else
    {
        // no transparence
        bSwitchOffGradient = sal_True;
        bSwitchOffLinear = sal_True;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffGradient && (bGradActive || bGradUsed))
    {
        Color aColor(COL_BLACK);
        XGradient aGrad(aColor, Color(COL_WHITE));
        aGrad.SetStartIntens(100);
        aGrad.SetEndIntens(100);
        XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aGrad);
        aItem.SetEnabled(sal_False);
        rAttrs.Put(aItem);
        bModified = sal_True;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffLinear && (bLinearActive || bLinearUsed))
    {
        XFillTransparenceItem aItem(0);
        SdrShadowTransparenceItem aShadowItem(0);
        rAttrs.Put(aItem);
        rAttrs.Put(aShadowItem);
        bModified = sal_True;
    }
    rAttrs.Put (CntUInt16Item(SID_PAGE_TYPE,nPageType));
    return bModified;
}

void SvxTransparenceTabPage::Reset(const SfxItemSet& rAttrs)
{
    const SfxPoolItem* pGradientItem = NULL;
    SfxItemState eStateGradient(rAttrs.GetItemState(XATTR_FILLFLOATTRANSPARENCE, sal_True, &pGradientItem));
    if(!pGradientItem)
        pGradientItem = &rAttrs.Get(XATTR_FILLFLOATTRANSPARENCE);
    sal_Bool bGradActive = (eStateGradient == SFX_ITEM_SET && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled());

    const SfxPoolItem* pLinearItem = NULL;
    SfxItemState eStateLinear(rAttrs.GetItemState(XATTR_FILLTRANSPARENCE, sal_True, &pLinearItem));
    if(!pLinearItem)
        pLinearItem = &rAttrs.Get(XATTR_FILLTRANSPARENCE);
    sal_Bool bLinearActive = (eStateLinear == SFX_ITEM_SET && ((XFillTransparenceItem*)pLinearItem)->GetValue() != 0);

    // transparence gradient
    const XGradient& rGradient = ((XFillFloatTransparenceItem*)pGradientItem)->GetGradientValue();
    XGradientStyle eXGS(rGradient.GetGradientStyle());
    m_pLbTrgrGradientType->SelectEntryPos(sal::static_int_cast< sal_uInt16 >(eXGS));
    m_pMtrTrgrAngle->SetValue(rGradient.GetAngle() / 10);
    m_pMtrTrgrBorder->SetValue(rGradient.GetBorder());
    m_pMtrTrgrCenterX->SetValue(rGradient.GetXOffset());
    m_pMtrTrgrCenterY->SetValue(rGradient.GetYOffset());
    m_pMtrTrgrStartValue->SetValue((sal_uInt16)((((sal_uInt16)rGradient.GetStartColor().GetRed() + 1) * 100) / 255));
    m_pMtrTrgrEndValue->SetValue((sal_uInt16)((((sal_uInt16)rGradient.GetEndColor().GetRed() + 1) * 100) / 255));

    // linear transparence
    sal_uInt16 nTransp = ((XFillTransparenceItem*)pLinearItem)->GetValue();
    m_pMtrTransparent->SetValue(bLinearActive ? nTransp : 50);
    ModifyTransparentHdl_Impl(NULL);

    // select the correct radio button
    if(bGradActive)
    {
        // transparence gradient, set controls appropriate to item
        m_pRbtTransGradient->Check();
        ClickTransGradientHdl_Impl(NULL);
    }
    else if(bLinearActive)
    {
        // linear transparence
        m_pRbtTransLinear->Check();
        ClickTransLinearHdl_Impl(NULL);
    }
    else
    {
        // no transparence
        m_pRbtTransOff->Check();
        ClickTransOffHdl_Impl(NULL);
        ModifiedTrgrHdl_Impl(NULL);
    }

    // save values
    m_pMtrTransparent->SaveValue();
    m_pLbTrgrGradientType->SaveValue();
    m_pMtrTrgrCenterX->SaveValue();
    m_pMtrTrgrCenterY->SaveValue();
    m_pMtrTrgrAngle->SaveValue();
    m_pMtrTrgrBorder->SaveValue();
    m_pMtrTrgrStartValue->SaveValue();
    m_pMtrTrgrEndValue->SaveValue();

    sal_Bool bActive = InitPreview ( rAttrs );
    InvalidatePreview ( bActive );
}

void SvxTransparenceTabPage::ActivatePage(const SfxItemSet& rSet)
{
    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,CntUInt16Item,SID_PAGE_TYPE,sal_False);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());

    if(nDlgType == 0) // area dialog
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
sal_Bool SvxTransparenceTabPage::InitPreview ( const SfxItemSet& rSet )
{
    // set transparencetyp for preview
    if ( m_pRbtTransOff->IsChecked() )
    {
        ClickTransOffHdl_Impl(NULL);
    } else if ( m_pRbtTransLinear->IsChecked() )
    {
        ClickTransLinearHdl_Impl(NULL);
    } else if ( m_pRbtTransGradient->IsChecked() )
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

    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );

    bBitmap = ( ( ( XFillStyleItem& )rSet.Get(XATTR_FILLSTYLE) ).GetValue() == XFILL_BITMAP );

    // show the right preview window
    if ( bBitmap )
    {
        m_pCtlBitmapPreview->Show();
        m_pCtlXRectPreview->Hide();
    }
    else
    {
        m_pCtlBitmapPreview->Hide();
        m_pCtlXRectPreview->Show();
    }

    return !m_pRbtTransOff->IsChecked();
}

void SvxTransparenceTabPage::InvalidatePreview (sal_Bool bEnable)
{
    if ( bBitmap )
    {
        if ( bEnable )
        {
            m_pCtlBitmapPreview->Enable();
            m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );
        }
        else
            m_pCtlBitmapPreview->Disable();
        m_pCtlBitmapPreview->Invalidate();
    }
    else
    {
        if ( bEnable )
        {
            m_pCtlXRectPreview->Enable();
            m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
        }
        else
            m_pCtlXRectPreview->Disable();
        m_pCtlXRectPreview->Invalidate();
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
    eRP( RP_LT ),

    pColorList( NULL ),
    pGradientList( NULL ),
    pHatchingList( NULL ),
    pBitmapList( NULL ),

    pnColorListState( 0 ),
    pnBitmapListState( 0 ),
    pnGradientListState( 0 ),
    pnHatchingListState( 0 ),

    nPageType( 0 ),
    nDlgType( 0 ),
    nPos( LISTBOX_ENTRY_NOTFOUND ),

    pbAreaTP( 0 ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    FreeResource();

    String accName = String(CUI_RES(STR_EXAMPLE));
    aCtlXRectPreview.SetAccessibleName(accName);
    aCtlBitmapPreview.SetAccessibleName(accName);

    // groups that overlay each other
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


    aTsbOriginal.EnableTriState( sal_False );


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
    SetFieldUnit( aMtrFldXSize, eFUnit, sal_True );
    SetFieldUnit( aMtrFldYSize, eFUnit, sal_True );

    // get PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( XATTR_FILLBMP_SIZEX );

    // setting the output device
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
    aCtlXRectPreview.EnableRTL(sal_False);

    aNumFldStepCount.SetAccessibleRelationLabeledBy( &aTsbStepCount );
    aCtlPosition.SetAccessibleRelationMemberOf( &aFlPosition );
    aLbHatchBckgrdColor.SetAccessibleRelationLabeledBy( &aCbxHatchBckgrd );
    aLbHatchBckgrdColor.SetAccessibleName(aCbxHatchBckgrd.GetText());

    aLbColor.SetAccessibleRelationMemberOf( &aFlProp );
    aMtrFldOffset.SetAccessibleRelationLabeledBy(&aFlOffset);
    aMtrFldOffset.SetAccessibleName(aFlOffset.GetText());

    //fdo#61241 lock down size of this tab page until it's
    //converted to .ui (remember to use some sizegroups
    //that take into account hidden frames)
    Size aSize(GetSizePixel());
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::Construct()
{
    // fill colortables / lists
    aLbColor.Fill( pColorList );
    aLbHatchBckgrdColor.Fill ( pColorList );

    aLbGradient.Fill( pGradientList );
    aLbHatching.Fill( pHatchingList );
    aLbBitmap.Fill( pBitmapList );
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_uInt16 nCount;
    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&rSet,pPosItem,SfxUInt16Item,SID_TABPAGE_POS,sal_False);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pPosItem)
        SetPos(pPosItem->GetValue());
    if( nDlgType == 0 ) // area dialog
    {
        *pbAreaTP = sal_True;

        if( pColorList.is() )
        {
            sal_uInt16 _nPos = 0;

            if( *pnBitmapListState )
            {
                if( *pnBitmapListState & CT_CHANGED )
                    pBitmapList = ( (SvxAreaTabDialog*) GetParentDialog() )->
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

            if( *pnHatchingListState )
            {
                if( *pnHatchingListState & CT_CHANGED )
                    pHatchingList = ( (SvxAreaTabDialog*) GetParentDialog() )->
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

            if( *pnGradientListState )
            {
                if( *pnGradientListState & CT_CHANGED )
                    pGradientList = ( (SvxAreaTabDialog*) GetParentDialog() )->
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

            if( *pnColorListState )
            {
                if( *pnColorListState & CT_CHANGED )
                    pColorList = ( (SvxAreaTabDialog*) GetParentDialog() )->
                                            GetNewColorList();
                // aLbColor
                _nPos = aLbColor.GetSelectEntryPos();
                aLbColor.Clear();
                aLbColor.Fill( pColorList );
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
                aLbHatchBckgrdColor.Fill( pColorList );
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
                        ClickGradientHdl_Impl();
                    break;

                    case PT_HATCH:
                        aTypeLB.SelectEntryPos( XFILL_HATCH );
                        aLbHatching.SelectEntryPos( _nPos );
                        ClickHatchingHdl_Impl();
                    break;

                    case PT_BITMAP:
                        aTypeLB.SelectEntryPos( XFILL_BITMAP );
                        aLbBitmap.SelectEntryPos( _nPos );
                        ClickBitmapHdl_Impl();
                    break;

                    case PT_COLOR:
                        aTypeLB.SelectEntryPos( XFILL_SOLID );
                        aLbColor.SelectEntryPos( _nPos );
                        aLbHatchBckgrdColor.SelectEntryPos( _nPos );
                        ClickColorHdl_Impl();
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
    if( nDlgType == 0 ) // area dialog
    {
        sal_uInt16 nPosOrig = nPos;
        XFillStyle eStyle = (XFillStyle) aTypeLB.GetSelectEntryPos();
        switch( eStyle )
        {
            case XFILL_GRADIENT:
            {
                        nPageType = PT_GRADIENT;
                        nPos = aLbGradient.GetSelectEntryPos();
                        if( nPosOrig != nPos )
                            *pnGradientListState |= CT_MODIFIED;
            }
            break;
            case XFILL_HATCH:
            {
                nPageType = PT_HATCH;
                nPos = aLbHatching.GetSelectEntryPos();
                if( nPosOrig != nPos )
                    *pnHatchingListState |= CT_MODIFIED;
            }
            break;
            case XFILL_BITMAP:
            {
                nPageType = PT_BITMAP;
                nPos = aLbBitmap.GetSelectEntryPos();
                if( nPosOrig != nPos )
                    *pnBitmapListState |= CT_MODIFIED;
            }
            break;
            case XFILL_SOLID:
            {
                nPageType = PT_COLOR;
                nPos = aLbColor.GetSelectEntryPos();
                if( nPosOrig != nPos )
                    *pnColorListState |= CT_MODIFIED;
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

sal_Bool SvxAreaTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    sal_uInt16  _nPos;
    sal_Bool    bModified = sal_False;

    if( nDlgType != 0 || *pbAreaTP )
    {
        const SfxPoolItem* pOld = NULL;
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
                        bModified = sal_True;
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
                         bModified = sal_True;
                     }
                 }
                 // NEW
                 if( (eSavedStyle != eStyle) &&
                     ( bModified ||
                       SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), sal_True ) ) )
                 {
                     XFillStyleItem aStyleItem( XFILL_SOLID );
                     pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                     if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                     {
                         rAttrs.Put( aStyleItem );
                         bModified = sal_True;
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
                        bModified = sal_True;
                    }
                }
                // NEW
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), sal_True ) ) )
                {
                    XFillStyleItem aStyleItem( XFILL_GRADIENT );
                    pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                    {
                        rAttrs.Put( aStyleItem );
                        bModified = sal_True;
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
                        bModified = sal_True;
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
                        bModified = sal_True;
                    }
                }
                // NEW
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), sal_True ) ) )
                {
                    XFillStyleItem aStyleItem( XFILL_HATCH );
                    pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                    {
                        rAttrs.Put( aStyleItem );
                        bModified = sal_True;
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
                    const XBitmapEntry* pXBitmapEntry = pBitmapList->GetBitmap(nPos);
                    const String aString(aLbBitmap.GetSelectEntry());
                    const XFillBitmapItem aFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject());
                    pOld = GetOldItem( rAttrs, XATTR_FILLBITMAP );
                    if ( !pOld || !( *(const XFillBitmapItem*)pOld == aFillBitmapItem ) )
                    {
                        rAttrs.Put( aFillBitmapItem );
                        bModified = sal_True;
                    }
                }
                // NEW
                if( (eSavedStyle != eStyle) &&
                    ( bModified ||
                      SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), sal_True ) ) )
                {
                    XFillStyleItem aStyleItem( XFILL_BITMAP );
                    pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                    if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                    {
                        rAttrs.Put( aStyleItem );
                        bModified = sal_True;
                    }
               }
           }
           break;
       }

        // step size
        if( aTsbStepCount.IsEnabled() )
        {
            sal_uInt16 nValue = 0;
            sal_Bool   bValueModified = sal_False;
            TriState eState = aTsbStepCount.GetState();
            if( eState == STATE_CHECK )
            {
                if( eState != aTsbStepCount.GetSavedValue() )
                    bValueModified = sal_True;
            }
            else
            {
                // condition != Disabled ?
                if( !aNumFldStepCount.GetText().isEmpty() )
                {
                    nValue = (sal_uInt16) aNumFldStepCount.GetValue();
                    if( nValue != (sal_uInt16) aNumFldStepCount.GetSavedValue().toInt32() )
                        bValueModified = sal_True;
                }
            }
            if( bValueModified )
            {
                XGradientStepCountItem aFillBitmapItem( nValue );
                pOld = GetOldItem( rAttrs, XATTR_GRADIENTSTEPCOUNT );
                if ( !pOld || !( *(const XGradientStepCountItem*)pOld == aFillBitmapItem ) )
                {
                    rAttrs.Put( aFillBitmapItem );
                    bModified = sal_True;
                }
            }
        }

        if( aTsbTile.IsEnabled() )
        {
            TriState eState = aTsbTile.GetState();
            if( eState != aTsbTile.GetSavedValue() )
            {
                XFillBmpTileItem aFillBmpTileItem(
                    sal::static_int_cast< sal_Bool >( eState ) );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILE );
                if ( !pOld || !( *(const XFillBmpTileItem*)pOld == aFillBmpTileItem ) )
                {
                    rAttrs.Put( aFillBmpTileItem );
                    bModified = sal_True;
                }
            }
        }

        if( aTsbStretch.IsEnabled() )
        {
            TriState eState = aTsbStretch.GetState();
            if( eState != aTsbStretch.GetSavedValue() )
            {
                XFillBmpStretchItem aFillBmpStretchItem(
                    sal::static_int_cast< sal_Bool >( eState ) );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_STRETCH );
                if ( !pOld || !( *(const XFillBmpStretchItem*)pOld == aFillBmpStretchItem ) )
                {
                    rAttrs.Put( aFillBmpStretchItem );
                    bModified = sal_True;
                }
            }
        }

        // Original size (in the UI) is used as follows:
        // Controls are disabled, but have to be set.
        // SizeX = 0; SizeY = 0; Log = sal_True

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
                pItem = new XFillBmpSizeLogItem( sal_True );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZELOG );
                if ( !pOld || !( *(const XFillBmpSizeLogItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = sal_True;
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
                    // Percentage values are set negatively, so that
                    // they aren't scaled; this is considered in the item.
                    pItem = new XFillBmpSizeXItem( -labs( static_cast<long>(aMtrFldXSize.GetValue()) ) );
                }
            }
            else if( aTsbOriginal.IsEnabled() &&
                     aTsbOriginal.GetState() == STATE_CHECK &&
                     !aMtrFldXSize.GetSavedValue().isEmpty() )
                pItem = new XFillBmpSizeXItem( 0 );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZEX );
                if ( !pOld || !( *(const XFillBmpSizeXItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = sal_True;
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
                    // Percentage values are set negatively, so that
                    // they aren't scaled by the MetricItem;
                    // this is considered in the item.
                    pItem = new XFillBmpSizeYItem( -labs( static_cast<long>(aMtrFldYSize.GetValue()) ) );
                }
            }
            else if( aTsbOriginal.IsEnabled() &&
                     aTsbOriginal.GetState() == STATE_CHECK &&
                     !aMtrFldYSize.GetSavedValue().isEmpty() )
                pItem = new XFillBmpSizeYItem( 0 );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZEY );
                if ( !pOld || !( *(const XFillBmpSizeYItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = sal_True;
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
                    XFillBmpTileOffsetXItem aFillBmpTileOffsetXItem( (sal_uInt16) aMtrFldOffset.GetValue() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILEOFFSETX );
                    if ( !pOld || !( *(const XFillBmpTileOffsetXItem*)pOld == aFillBmpTileOffsetXItem ) )
                    {
                        rAttrs.Put( aFillBmpTileOffsetXItem );
                        rAttrs.Put( XFillBmpTileOffsetYItem( 0 ) );
                        bModified = sal_True;
                    }
                }
                else if( aRbtColumn.IsChecked() )
                {
                    XFillBmpTileOffsetYItem aFillBmpTileOffsetYItem( (sal_uInt16) aMtrFldOffset.GetValue() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILEOFFSETY );
                    if ( !pOld || !( *(const XFillBmpTileOffsetYItem*)pOld == aFillBmpTileOffsetYItem ) )
                    {
                        rAttrs.Put( aFillBmpTileOffsetYItem );
                        rAttrs.Put( XFillBmpTileOffsetXItem( 0 ) );
                        bModified = sal_True;
                    }
                }
            }
        }

        //aCtlPosition
        if( aCtlPosition.IsEnabled() )
        {
            sal_Bool bPut = sal_False;
            RECT_POINT _eRP = aCtlPosition.GetActualRP();

            if( SFX_ITEM_DONTCARE == rOutAttrs.GetItemState( XATTR_FILLBMP_POS ) )
                bPut = sal_True;
            else
            {
                RECT_POINT eValue = ( ( const XFillBmpPosItem& ) rOutAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
                if( eValue != _eRP )
                    bPut = sal_True;
            }
            if( bPut )
            {
                XFillBmpPosItem aFillBmpPosItem( _eRP );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POS );
                if ( !pOld || !( *(const XFillBmpPosItem*)pOld == aFillBmpPosItem ) )
                {
                    rAttrs.Put( aFillBmpPosItem );
                    bModified = sal_True;
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
                XFillBmpPosOffsetXItem aFillBmpPosOffsetXItem( (sal_uInt16) aMtrFldXOffset.GetValue() );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POSOFFSETX );
                if ( !pOld || !( *(const XFillBmpPosOffsetXItem*)pOld == aFillBmpPosOffsetXItem ) )
                {
                    rAttrs.Put( aFillBmpPosOffsetXItem );
                    bModified = sal_True;
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
                XFillBmpPosOffsetYItem aFillBmpPosOffsetYItem( (sal_uInt16) aMtrFldYOffset.GetValue() );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POSOFFSETY );
                if ( !pOld || !( *(const XFillBmpPosOffsetYItem*)pOld == aFillBmpPosOffsetYItem ) )
                {
                    rAttrs.Put( aFillBmpPosOffsetYItem );
                    bModified = sal_True;
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
        aTypeLB.SelectEntryPos( sal::static_int_cast< sal_uInt16 >( eXFS ) );
        switch( eXFS )
        {
            case XFILL_NONE:
                ClickInvisibleHdl_Impl();
            break;

            case XFILL_SOLID:
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rAttrs.Get( XATTR_FILLCOLOR ) );

                    aLbColor.SelectEntry( aColorItem.GetColorValue() );
                    aLbHatchBckgrdColor.SelectEntry( aColorItem.GetColorValue() );
                }
                ClickColorHdl_Impl();

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
                ClickGradientHdl_Impl();
            break;

            case XFILL_HATCH:
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    aLbHatching.SelectEntry( ( ( const XFillHatchItem& )
                                    rAttrs.Get( XATTR_FILLHATCH ) ).GetName() );
                }
                ClickHatchingHdl_Impl();

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
                ClickBitmapHdl_Impl();
            }
            break;

            default:
            break;
        }
    }
    else
    {
        // make all LBs not accessible
        aLbColor.Hide();
        aLbGradient.Hide();
        aLbHatching.Hide();
        aLbBitmap.Hide();
        aCtlBitmapPreview.Hide();
        aLbColor.Disable();
        aLbColor.Show();

        // so that Reset() also works correctly with Back
        aTypeLB.SetNoSelection();
    }

    // step size
    if( ( rAttrs.GetItemState( XATTR_GRADIENTSTEPCOUNT ) != SFX_ITEM_DONTCARE ) ||
        ( rAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE ) )
    {
        aTsbStepCount.EnableTriState( sal_False );
        sal_uInt16 nValue = ( ( const XGradientStepCountItem& ) rAttrs.Get( XATTR_GRADIENTSTEPCOUNT ) ).GetValue();
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

    // attributes for the bitmap filling

    if( rAttrs.GetItemState( XATTR_FILLBMP_TILE ) != SFX_ITEM_DONTCARE )
    {
        aTsbTile.EnableTriState( sal_False );

        if( ( ( const XFillBmpTileItem& ) rAttrs.Get( XATTR_FILLBMP_TILE ) ).GetValue() )
            aTsbTile.SetState( STATE_CHECK );
        else
            aTsbTile.SetState( STATE_NOCHECK );
    }
    else
        aTsbTile.SetState( STATE_DONTKNOW );

    if( rAttrs.GetItemState( XATTR_FILLBMP_STRETCH ) != SFX_ITEM_DONTCARE )
    {
        aTsbStretch.EnableTriState( sal_False );

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
        aTsbScale.EnableTriState( sal_False );

        if( ( ( const XFillBmpSizeLogItem& ) rAttrs.Get( XATTR_FILLBMP_SIZELOG ) ).GetValue() )
            aTsbScale.SetState( STATE_NOCHECK );
        else
            aTsbScale.SetState( STATE_CHECK );

        ClickScaleHdl_Impl( NULL );
    }
    else
        aTsbScale.SetState( STATE_DONTKNOW );


    // determine status for the original size
    TriState eOriginal = STATE_NOCHECK;

    //aMtrFldXSize
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZEX ) != SFX_ITEM_DONTCARE )
    {
        sal_Int32 nValue = ( ( const XFillBmpSizeXItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        if( aTsbScale.GetState() == STATE_CHECK )
        {
            // If there's a percentage value in the item,
            // it is negative because of the MetricItems.
            aMtrFldXSize.SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( aMtrFldXSize, nValue, ePoolUnit );
        aMtrFldXSize.SaveValue();

        if( nValue == 0 )
        {
            eOriginal = STATE_CHECK;
            // value would be too small otherwise when turning off the original size
            // (performance problem)
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
        sal_Int32 nValue = ( ( const XFillBmpSizeYItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEY ) ).GetValue();
        if( aTsbScale.GetState() == STATE_CHECK )
        {
            // If there's a percentage value in the item,
            // it is negative because of the MetricItems.
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
        sal_uInt16 nValue = ( ( const XFillBmpTileOffsetXItem& ) rAttrs.Get( XATTR_FILLBMP_TILEOFFSETX ) ).GetValue();
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
        sal_Int32 nValue = ( ( const XFillBmpPosOffsetXItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETX ) ).GetValue();
        aMtrFldXOffset.SetValue( nValue );
    }
    else
        aMtrFldXOffset.SetText( String() );

    //aMtrFldYOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_POSOFFSETY ) != SFX_ITEM_DONTCARE )
    {
        sal_Int32 nValue = ( ( const XFillBmpPosOffsetYItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETY ) ).GetValue();
        aMtrFldYOffset.SetValue( nValue );
    }
    else
        aMtrFldYOffset.SetText( String() );

    // not earlier so that tile and stretch are considered
    if( aTypeLB.GetSelectEntryPos() == XFILL_BITMAP )
        ClickBitmapHdl_Impl();

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

sal_uInt16* SvxAreaTabPage::GetRanges()
{
    return( pAreaRanges );
}

//------------------------------------------------------------------------
IMPL_LINK_NOARG(SvxAreaTabPage, SelectDialogTypeHdl_Impl)
{
    switch( (XFillStyle)aTypeLB.GetSelectEntryPos() )
    {
        case XFILL_NONE: ClickInvisibleHdl_Impl(); break;
        case XFILL_SOLID: ClickColorHdl_Impl(); break;
        case XFILL_GRADIENT: ClickGradientHdl_Impl(); break;
        case XFILL_HATCH: ClickHatchingHdl_Impl(); break;
        case XFILL_BITMAP: ClickBitmapHdl_Impl(); break;
    }

    return 0;
}

void SvxAreaTabPage::ClickInvisibleHdl_Impl()
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
}

//------------------------------------------------------------------------

void SvxAreaTabPage::ClickColorHdl_Impl()
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

    // set table text
    OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString += ": ";
    INetURLObject   aURL( pColorList->GetPath() );

    aURL.Append( pColorList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += aURL.getBase().copy( 0, 15 );
        aString += "...";
    }
    else
        aString += aURL.getBase();

    ModifyColorHdl_Impl( this );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyColorHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = aLbColor.GetSelectEntryPos();
    aLbHatchBckgrdColor.SelectEntryPos( _nPos );
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        rXFSet.Put( XFillColorItem( String(),
                                    aLbColor.GetSelectEntryColor() ) );
    }
    // NEW
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), sal_True, &pPoolItem ) )
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

void SvxAreaTabPage::ClickGradientHdl_Impl()
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

    // set table text
    OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString += ": ";
    INetURLObject   aURL( pGradientList->GetPath() );

    aURL.Append( pGradientList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += aURL.getBase().copy( 0, 15 );
        aString += "...";
    }
    else
        aString += aURL.getBase();

    ModifyGradientHdl_Impl( this );
    ModifyStepCountHdl_Impl( &aTsbStepCount );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyGradientHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = aLbGradient.GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // fill ItemSet and pass it on to aCtlXRectPreview
        XGradientEntry* pEntry = pGradientList->GetGradient( _nPos );

        rXFSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
        rXFSet.Put( XFillGradientItem( String(),
                                       pEntry->GetGradient() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), sal_True, &pPoolItem ) )
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

void SvxAreaTabPage::ClickHatchingHdl_Impl()
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

    // set table text
    OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString += ": ";
    INetURLObject   aURL( pHatchingList->GetPath() );

    aURL.Append( pHatchingList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += aURL.getBase().copy( 0, 15 );
        aString += "...";
    }
    else
        aString += aURL.getBase();

    ModifyHatchingHdl_Impl( this );
    ModifyHatchBckgrdColorHdl_Impl( this );
    ToggleHatchBckgrdColorHdl_Impl( this );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyHatchingHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = aLbHatching.GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // fill ItemSet and pass it on to aCtlXRectPreview
        XHatchEntry* pEntry = pHatchingList->GetHatch( _nPos );

        rXFSet.Put( XFillStyleItem( XFILL_HATCH ) );
        rXFSet.Put( XFillHatchItem( String(), pEntry->GetHatch() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), sal_True, &pPoolItem ) )
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

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyHatchBckgrdColorHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = aLbHatchBckgrdColor.GetSelectEntryPos();
    aLbColor.SelectEntryPos( _nPos );
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillColorItem( String(),
                                    aLbHatchBckgrdColor.GetSelectEntryColor() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), sal_True, &pPoolItem ) )
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

IMPL_LINK_NOARG(SvxAreaTabPage, ToggleHatchBckgrdColorHdl_Impl)
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

void SvxAreaTabPage::ClickBitmapHdl_Impl()
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

    // set table text
    OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString += ": ";
    INetURLObject   aURL( pBitmapList->GetPath() );

    aURL.Append( pBitmapList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().getLength() > 18 )
    {
        aString += aURL.getBase().copy( 0, 15 );
        aString += "...";
    }
    else
        aString += aURL.getBase();

    ModifyBitmapHdl_Impl( this );
    ModifyTileHdl_Impl( &aTsbOriginal );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyBitmapHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = aLbBitmap.GetSelectEntryPos();
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // fill ItemSet and pass it on to aCtlXRectPreview
        const XBitmapEntry* pEntry = pBitmapList->GetBitmap(_nPos);

        rXFSet.Put(XFillStyleItem(XFILL_BITMAP));
        rXFSet.Put(XFillBitmapItem(String(), pEntry->GetGraphicObject()));
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), sal_True, &pPoolItem ) )
    {
        rXFSet.Put(XFillStyleItem(XFILL_BITMAP));
        rXFSet.Put(XFillBitmapItem(String(), ((const XFillBitmapItem*)pPoolItem)->GetGraphicObject()));
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
            if( aNumFldStepCount.GetText().isEmpty() )
                aNumFldStepCount.SetText(OUString("64"));

            aNumFldStepCount.Enable();
        }
        else
            aNumFldStepCount.Disable();
    }

    sal_uInt16 nValue = 0;
    if( aTsbStepCount.GetState() != STATE_CHECK )
    {
        // condition != Disabled ?
        if( !aNumFldStepCount.GetText().isEmpty() )
            nValue = (sal_uInt16) aNumFldStepCount.GetValue();
    }
    rXFSet.Put( XGradientStepCountItem( nValue ) );
    aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyTileHdl_Impl)
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

    rXFSet.Put( XFillBmpTileItem( sal::static_int_cast< sal_Bool >( eState ) ) );

    if( aTsbStretch.IsEnabled() )
        rXFSet.Put(
            XFillBmpStretchItem(
                sal::static_int_cast< sal_Bool >( aTsbStretch.GetState() ) ) );

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
        // original size -> size == 0
        rXFSet.Put( XFillBmpSizeXItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( sal_True ) );
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
        // original size -> size == 0
        rXFSet.Put( XFillBmpSizeYItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( sal_True ) );
    }

    if( aMtrFldOffset.IsEnabled() )
    {
        if( aRbtRow.IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (sal_uInt16) aMtrFldOffset.GetValue() ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (sal_uInt16) 0 ) );
        }
        else if( aRbtColumn.IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (sal_uInt16) 0 ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (sal_uInt16) aMtrFldOffset.GetValue() ) );
        }
    }

    if( aCtlPosition.IsEnabled() )
        rXFSet.Put( XFillBmpPosItem( aCtlPosition.GetActualRP() ) );

    if( aMtrFldXOffset.IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetXItem( (sal_uInt16) aMtrFldXOffset.GetValue() ) );

    if( aMtrFldYOffset.IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetYItem( (sal_uInt16) aMtrFldYOffset.GetValue() ) );


    aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ClickScaleHdl_Impl)
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

    // alignment of the bitmap fill
    ModifyTileHdl_Impl( pWindow );
}

void SvxAreaTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pColorListItem,SvxColorListItem,SID_COLOR_TABLE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pGradientListItem,SvxGradientListItem,SID_GRADIENT_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pHatchingListItem,SvxHatchListItem,SID_HATCH_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pBitmapListItem,SvxBitmapListItem,SID_BITMAP_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPosItem,SfxUInt16Item,SID_TABPAGE_POS,sal_False);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
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
