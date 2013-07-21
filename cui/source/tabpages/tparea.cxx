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

    SvxTabPage( pParent,
                "AreaTabPage",
                "cui/ui/areatabpage.ui",
               rInAttrs ),
    rOutAttrs (rInAttrs ),
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
    get(m_pTypeLB,"LB_AREA_TYPE");
    get(m_pFillLB,"boxLB_FILL");
    get(m_pLbColor,"LB_COLOR");
    get(m_pLbGradient,"LB_GRADIENT");
    get(m_pLbHatching,"LB_HATCHING");
    get(m_pLbBitmap,"LB_BITMAP");
    get(m_pCtlBitmapPreview,"CTL_BITMAP_PREVIEW");

    get(m_pTsbStepCount,"TSB_STEPCOUNT");
    get(m_pFlStepCount,"FL_STEPCOUNT");
    get(m_pNumFldStepCount,"NUM_FLD_STEPCOUNT");

    get(m_pFlHatchBckgrd,"FL_HATCHCOLORS");
    get(m_pLbHatchBckgrdColor,"LB_HATCHBCKGRDCOLOR");
    get(m_pCbxHatchBckgrd,"CB_HATCHBCKGRD");

    get(m_pBxBitmap,"boxBITMAP");

    get(m_pFlSize,"FL_SIZE");
    get(m_pGridX_Y,"gridX_Y");
    get(m_pTsbOriginal,"TSB_ORIGINAL");
    get(m_pTsbScale,"TSB_SCALE");
    get(m_pFtXSize,"FT_X_SIZE");
    get(m_pMtrFldXSize,"MTR_FLD_X_SIZE");
    get(m_pFtYSize,"FT_Y_SIZE");
    get(m_pMtrFldYSize,"MTR_FLD_Y_SIZE");

    get(m_pFlPosition,"framePOSITION");
    get(m_pCtlPosition,"CTL_POSITION");
    get(m_pGridOffset,"gridOFFSET");
    get(m_pMtrFldXOffset,"MTR_FLD_X_OFFSET");
    get(m_pMtrFldYOffset,"MTR_FLD_Y_OFFSET");
    get(m_pBxTile,"boxTILE");
    get(m_pTsbTile,"TSB_TILE");
    get(m_pTsbStretch,"TSB_STRETCH");

    get(m_pFlOffset,"FL_OFFSET");
    get(m_pRbtRow,"RBT_ROW");
    get(m_pRbtColumn,"RBT_COLUMN");
    get(m_pMtrFldOffset,"MTR_FLD_OFFSET");

    get(m_pCtlXRectPreview,"CTL_COLOR_PREVIEW");

    // groups that overlay each other
    m_pLbBitmap->Hide();
    m_pCtlBitmapPreview->Hide();

    m_pFlStepCount->Hide();

    m_pBxBitmap->Hide();

    // Controls for Hatch-Background
    m_pFlHatchBckgrd->Hide();

    m_pTsbOriginal->EnableTriState( sal_False );

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
    SetFieldUnit( *m_pMtrFldXSize, eFUnit, sal_True );
    SetFieldUnit( *m_pMtrFldYSize, eFUnit, sal_True );

    // get PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( XATTR_FILLBMP_SIZEX );

    // setting the output device
    rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
    rXFSet.Put( XFillColorItem( String(), COL_BLACK ) );
    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );

    m_pLbColor->SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyColorHdl_Impl ) );
    m_pLbHatchBckgrdColor->SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyHatchBckgrdColorHdl_Impl ) );
    m_pCbxHatchBckgrd->SetToggleHdl( LINK( this, SvxAreaTabPage, ToggleHatchBckgrdColorHdl_Impl ) );

    m_pLbGradient->SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyGradientHdl_Impl ) );
    m_pLbHatching->SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyHatchingHdl_Impl ) );
    m_pLbBitmap->SetSelectHdl(   LINK( this, SvxAreaTabPage, ModifyBitmapHdl_Impl ) );

    m_pTsbStepCount->SetClickHdl( LINK( this, SvxAreaTabPage, ModifyStepCountHdl_Impl ) );
    m_pNumFldStepCount->SetModifyHdl( LINK( this, SvxAreaTabPage, ModifyStepCountHdl_Impl ) );

    Link aLink( LINK( this, SvxAreaTabPage, ModifyTileHdl_Impl ) );
    m_pTsbTile->SetClickHdl( aLink );
    m_pTsbStretch->SetClickHdl( aLink );
    m_pTsbOriginal->SetClickHdl( aLink );
    m_pMtrFldXSize->SetModifyHdl( aLink );
    m_pMtrFldYSize->SetModifyHdl( aLink );
    m_pRbtRow->SetClickHdl( aLink );
    m_pRbtColumn->SetClickHdl( aLink );
    m_pMtrFldOffset->SetModifyHdl( aLink );
    m_pMtrFldXOffset->SetModifyHdl( aLink );
    m_pMtrFldYOffset->SetModifyHdl( aLink );
    m_pTsbScale->SetClickHdl( LINK( this, SvxAreaTabPage, ClickScaleHdl_Impl ) );

    m_pTypeLB->SetSelectHdl( LINK( this, SvxAreaTabPage, SelectDialogTypeHdl_Impl ) );

    // #i76307# always paint the preview in LTR, because this is what the document does
    m_pCtlXRectPreview->EnableRTL(sal_False);

    // Calcualte size of dropdown listboxes
    Size aSize = LogicToPixel(Size(108, 103), MAP_APPFONT);

    m_pLbColor->set_width_request(aSize.Width());
    m_pLbColor->set_height_request(aSize.Height());
    //m_pLbColor->

    m_pLbGradient->set_width_request(aSize.Width());
    m_pLbGradient->set_height_request(aSize.Height());
    m_pLbHatching->set_width_request(aSize.Width());
    m_pLbHatching->set_height_request(aSize.Height());
    m_pLbBitmap->set_width_request(aSize.Width());
    m_pLbBitmap->set_height_request(aSize.Height());

    // Calculate size of display boxes
    Size aSize2 = LogicToPixel(Size(110, 42), MAP_APPFONT);
    m_pCtlBitmapPreview->set_width_request(aSize2.Width());
    m_pCtlBitmapPreview->set_height_request(aSize2.Height());
    m_pCtlXRectPreview->set_width_request(aSize2.Width());
    m_pCtlXRectPreview->set_height_request(aSize2.Height());
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::Construct()
{
    // fill colortables / lists
    m_pLbColor->Fill( pColorList );
    m_pLbHatchBckgrdColor->Fill ( pColorList );

    m_pLbGradient->Fill( pGradientList );
    m_pLbHatching->Fill( pHatchingList );
    m_pLbBitmap->Fill( pBitmapList );
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

                _nPos = m_pLbBitmap->GetSelectEntryPos();

                m_pLbBitmap->Clear();
                m_pLbBitmap->Fill( pBitmapList );
                nCount = m_pLbBitmap->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbBitmap->SelectEntryPos( 0 );
                else
                    m_pLbBitmap->SelectEntryPos( _nPos );
                ModifyBitmapHdl_Impl( this );
            }

            if( *pnHatchingListState )
            {
                if( *pnHatchingListState & CT_CHANGED )
                    pHatchingList = ( (SvxAreaTabDialog*) GetParentDialog() )->
                                            GetNewHatchingList();

                _nPos = m_pLbHatching->GetSelectEntryPos();

                m_pLbHatching->Clear();
                m_pLbHatching->Fill( pHatchingList );
                nCount = m_pLbHatching->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbHatching->SelectEntryPos( 0 );
                else
                    m_pLbHatching->SelectEntryPos( _nPos );
                ModifyHatchingHdl_Impl( this );

                ModifyHatchBckgrdColorHdl_Impl( this );
            }

            if( *pnGradientListState )
            {
                if( *pnGradientListState & CT_CHANGED )
                    pGradientList = ( (SvxAreaTabDialog*) GetParentDialog() )->
                                            GetNewGradientList();

                _nPos = m_pLbGradient->GetSelectEntryPos();

                m_pLbGradient->Clear();
                m_pLbGradient->Fill( pGradientList );
                nCount = m_pLbGradient->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbGradient->SelectEntryPos( 0 );
                else
                    m_pLbGradient->SelectEntryPos( _nPos );
                ModifyGradientHdl_Impl( this );
            }

            if( *pnColorListState )
            {
                if( *pnColorListState & CT_CHANGED )
                    pColorList = ( (SvxAreaTabDialog*) GetParentDialog() )->
                                            GetNewColorList();
                // aLbColor
                _nPos = m_pLbColor->GetSelectEntryPos();
                m_pLbColor->Clear();
                m_pLbColor->Fill( pColorList );
                nCount = m_pLbColor->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbColor->SelectEntryPos( 0 );
                else
                    m_pLbColor->SelectEntryPos( _nPos );

                ModifyColorHdl_Impl( this );

                // Backgroundcolor of hatch
                _nPos = m_pLbHatchBckgrdColor->GetSelectEntryPos();
                m_pLbHatchBckgrdColor->Clear();
                m_pLbHatchBckgrdColor->Fill( pColorList );
                nCount = m_pLbHatchBckgrdColor->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= _nPos )
                    m_pLbHatchBckgrdColor->SelectEntryPos( 0 );
                else
                    m_pLbHatchBckgrdColor->SelectEntryPos( _nPos );

                ModifyHatchBckgrdColorHdl_Impl( this );
            }

            // evaluate if any other Tabpage set another filltype
            if( m_pTypeLB->GetSelectEntryPos() > XFILL_NONE)
            {
                switch( nPageType )
                {
                    case PT_GRADIENT:
                        m_pTypeLB->SelectEntryPos( XFILL_GRADIENT );
                        m_pLbGradient->SelectEntryPos( _nPos );
                        ClickGradientHdl_Impl();
                    break;

                    case PT_HATCH:
                        m_pTypeLB->SelectEntryPos( XFILL_HATCH );
                        m_pLbHatching->SelectEntryPos( _nPos );
                        ClickHatchingHdl_Impl();
                    break;

                    case PT_BITMAP:
                        m_pTypeLB->SelectEntryPos( XFILL_BITMAP );
                        m_pLbBitmap->SelectEntryPos( _nPos );
                        ClickBitmapHdl_Impl();
                    break;

                    case PT_COLOR:
                        m_pTypeLB->SelectEntryPos( XFILL_SOLID );
                        m_pLbColor->SelectEntryPos( _nPos );
                        m_pLbHatchBckgrdColor->SelectEntryPos( _nPos );
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
        XFillStyle eStyle = (XFillStyle) m_pTypeLB->GetSelectEntryPos();
        switch( eStyle )
        {
            case XFILL_GRADIENT:
            {
                        nPageType = PT_GRADIENT;
                        nPos = m_pLbGradient->GetSelectEntryPos();
                        if( nPosOrig != nPos )
                            *pnGradientListState |= CT_MODIFIED;
            }
            break;
            case XFILL_HATCH:
            {
                nPageType = PT_HATCH;
                nPos = m_pLbHatching->GetSelectEntryPos();
                if( nPosOrig != nPos )
                    *pnHatchingListState |= CT_MODIFIED;
            }
            break;
            case XFILL_BITMAP:
            {
                nPageType = PT_BITMAP;
                nPos = m_pLbBitmap->GetSelectEntryPos();
                if( nPosOrig != nPos )
                    *pnBitmapListState |= CT_MODIFIED;
            }
            break;
            case XFILL_SOLID:
            {
                nPageType = PT_COLOR;
                nPos = m_pLbColor->GetSelectEntryPos();
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
        XFillStyle eStyle = (XFillStyle) m_pTypeLB->GetSelectEntryPos();
        XFillStyle eSavedStyle = (XFillStyle) m_pTypeLB->GetSavedValue();
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
                _nPos = m_pLbColor->GetSelectEntryPos();
                 if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                     _nPos != m_pLbColor->GetSavedValue() )
                 {
                     XFillColorItem aItem( m_pLbColor->GetSelectEntry(),
                                           m_pLbColor->GetSelectEntryColor() );
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
                _nPos = m_pLbGradient->GetSelectEntryPos();
                if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                    _nPos != m_pLbGradient->GetSavedValue() )
                {
                    XGradient aGradient = pGradientList->GetGradient( _nPos )->GetGradient();
                    String aString = m_pLbGradient->GetSelectEntry();
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
                _nPos = m_pLbHatching->GetSelectEntryPos();
                if( _nPos != LISTBOX_ENTRY_NOTFOUND &&
                   _nPos != m_pLbHatching->GetSavedValue() )
                {
                    XHatch aHatching = pHatchingList->GetHatch( _nPos )->GetHatch();
                    String aString = m_pLbHatching->GetSelectEntry();
                    XFillHatchItem aItem( aString, aHatching );
                    pOld = GetOldItem( rAttrs, XATTR_FILLHATCH );
                    if ( !pOld || !( *(const XFillHatchItem*)pOld == aItem ) )
                    {
                        rAttrs.Put( aItem );
                        bModified = sal_True;
                    }
                }
                XFillBackgroundItem aItem ( m_pCbxHatchBckgrd->IsChecked() );
                rAttrs.Put( aItem );
                nPos = m_pLbHatchBckgrdColor->GetSelectEntryPos();
                if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                     nPos != m_pLbHatchBckgrdColor->GetSavedValue() )
                {
                    XFillColorItem aFillColorItem( m_pLbHatchBckgrdColor->GetSelectEntry(),
                                          m_pLbHatchBckgrdColor->GetSelectEntryColor() );
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
                nPos = m_pLbBitmap->GetSelectEntryPos();
                if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                    nPos != m_pLbBitmap->GetSavedValue() )
                {
                    const XBitmapEntry* pXBitmapEntry = pBitmapList->GetBitmap(nPos);
                    const String aString(m_pLbBitmap->GetSelectEntry());
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
        if( m_pTsbStepCount->IsEnabled() )
        {
            sal_uInt16 nValue = 0;
            sal_Bool   bValueModified = sal_False;
            TriState eState = m_pTsbStepCount->GetState();
            if( eState == STATE_CHECK )
            {
                if( eState != m_pTsbStepCount->GetSavedValue() )
                    bValueModified = sal_True;
            }
            else
            {
                // condition != Disabled ?
                if( !m_pNumFldStepCount->GetText().isEmpty() )
                {
                    nValue = (sal_uInt16) m_pNumFldStepCount->GetValue();
                    if( nValue != (sal_uInt16) m_pNumFldStepCount->GetSavedValue().toInt32() )
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

        if( m_pTsbTile->IsEnabled() )
        {
            TriState eState = m_pTsbTile->GetState();
            if( eState != m_pTsbTile->GetSavedValue() )
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

        if( m_pTsbStretch->IsEnabled() )
        {
            TriState eState = m_pTsbStretch->GetState();
            if( eState != m_pTsbStretch->GetSavedValue() )
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
        TriState eState = m_pTsbScale->GetState();
        if( eState != m_pTsbScale->GetSavedValue() ||
            ( !m_pTsbScale->IsEnabled() &&
              m_pTsbOriginal->IsEnabled() &&
              m_pTsbScale->GetSavedValue() != STATE_CHECK ) )
        {
            XFillBmpSizeLogItem* pItem = NULL;
            if( m_pTsbScale->IsEnabled() )
                pItem = new XFillBmpSizeLogItem( eState == STATE_NOCHECK );
            else if( m_pTsbOriginal->IsEnabled() && m_pTsbOriginal->GetState() == STATE_CHECK )
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
        String aStr = m_pMtrFldXSize->GetText();
        {
            XFillBmpSizeXItem* pItem = NULL;
            TriState eScaleState = m_pTsbScale->GetState();

            if( m_pMtrFldXSize->IsEnabled() &&
                aStr.Len() > 0  &&
                aStr != m_pMtrFldXSize->GetSavedValue() )
            {
                if( eScaleState == STATE_NOCHECK )
                    pItem = new XFillBmpSizeXItem( GetCoreValue( *m_pMtrFldXSize, ePoolUnit ) );
                else
                {
                    // Percentage values are set negatively, so that
                    // they aren't scaled; this is considered in the item.
                    pItem = new XFillBmpSizeXItem( -labs( static_cast<long>(m_pMtrFldXSize->GetValue()) ) );
                }
            }
            else if( m_pTsbOriginal->IsEnabled() &&
                     m_pTsbOriginal->GetState() == STATE_CHECK &&
                     !m_pMtrFldXSize->GetSavedValue().isEmpty() )
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
        aStr = m_pMtrFldYSize->GetText();
        {
            XFillBmpSizeYItem* pItem = NULL;
            TriState eScaleState = m_pTsbScale->GetState();

            if( m_pMtrFldYSize->IsEnabled() &&
                aStr.Len() > 0  &&
                aStr != m_pMtrFldYSize->GetSavedValue() )
            {
                if( eScaleState == STATE_NOCHECK )
                    pItem = new XFillBmpSizeYItem( GetCoreValue( *m_pMtrFldYSize, ePoolUnit ) );
                else
                {
                    // Percentage values are set negatively, so that
                    // they aren't scaled by the MetricItem;
                    // this is considered in the item.
                    pItem = new XFillBmpSizeYItem( -labs( static_cast<long>(m_pMtrFldYSize->GetValue()) ) );
                }
            }
            else if( m_pTsbOriginal->IsEnabled() &&
                     m_pTsbOriginal->GetState() == STATE_CHECK &&
                     !m_pMtrFldYSize->GetSavedValue().isEmpty() )
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
        if( m_pMtrFldOffset->IsEnabled() )
        {
            String aMtrString = m_pMtrFldOffset->GetText();
            if( ( aMtrString.Len() > 0  &&
                  aMtrString != m_pMtrFldOffset->GetSavedValue() ) ||
                  m_pRbtRow->GetSavedValue() != m_pRbtRow->IsChecked() ||
                  m_pRbtColumn->GetSavedValue() != m_pRbtColumn->IsChecked() )
            {
                if( m_pRbtRow->IsChecked() )
                {
                    XFillBmpTileOffsetXItem aFillBmpTileOffsetXItem( (sal_uInt16) m_pMtrFldOffset->GetValue() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILEOFFSETX );
                    if ( !pOld || !( *(const XFillBmpTileOffsetXItem*)pOld == aFillBmpTileOffsetXItem ) )
                    {
                        rAttrs.Put( aFillBmpTileOffsetXItem );
                        rAttrs.Put( XFillBmpTileOffsetYItem( 0 ) );
                        bModified = sal_True;
                    }
                }
                else if( m_pRbtColumn->IsChecked() )
                {
                    XFillBmpTileOffsetYItem aFillBmpTileOffsetYItem( (sal_uInt16) m_pMtrFldOffset->GetValue() );
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
        if( m_pCtlPosition->IsEnabled() )
        {
            sal_Bool bPut = sal_False;
            RECT_POINT _eRP = m_pCtlPosition->GetActualRP();

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
        if( m_pMtrFldXOffset->IsEnabled() )
        {
            String sMtrXOffset = m_pMtrFldXOffset->GetText();
            if( sMtrXOffset.Len() > 0  &&
                sMtrXOffset != m_pMtrFldXOffset->GetSavedValue() )
            {
                XFillBmpPosOffsetXItem aFillBmpPosOffsetXItem( (sal_uInt16) m_pMtrFldXOffset->GetValue() );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POSOFFSETX );
                if ( !pOld || !( *(const XFillBmpPosOffsetXItem*)pOld == aFillBmpPosOffsetXItem ) )
                {
                    rAttrs.Put( aFillBmpPosOffsetXItem );
                    bModified = sal_True;
                }
            }
        }

        //aMtrFldYOffset
        if( m_pMtrFldYOffset->IsEnabled() )
        {
            String sMtrYOffset = m_pMtrFldYOffset->GetText();
            if( sMtrYOffset.Len() > 0  &&
                sMtrYOffset != m_pMtrFldYOffset->GetSavedValue() )
            {
                XFillBmpPosOffsetYItem aFillBmpPosOffsetYItem( (sal_uInt16) m_pMtrFldYOffset->GetValue() );
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
        m_pTypeLB->SelectEntryPos( sal::static_int_cast< sal_uInt16 >( eXFS ) );
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

                    m_pLbColor->SelectEntry( aColorItem.GetColorValue() );
                    m_pLbHatchBckgrdColor->SelectEntry( aColorItem.GetColorValue() );
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

                    m_pLbGradient->SelectEntryByList( pGradientList, aString, aGradient );
                }
                ClickGradientHdl_Impl();
            break;

            case XFILL_HATCH:
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    m_pLbHatching->SelectEntry( ( ( const XFillHatchItem& )
                                    rAttrs.Get( XATTR_FILLHATCH ) ).GetName() );
                }
                ClickHatchingHdl_Impl();

                if ( SFX_ITEM_DONTCARE != rAttrs.GetItemState ( XATTR_FILLBACKGROUND ) )
                {
                    m_pCbxHatchBckgrd->Check ( ( ( const XFillBackgroundItem& ) rAttrs.Get ( XATTR_FILLBACKGROUND ) ).GetValue() );
                }
                ToggleHatchBckgrdColorHdl_Impl( this );
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rAttrs.Get( XATTR_FILLCOLOR ) );

                    m_pLbColor->SelectEntry( aColorItem.GetColorValue() );
                    m_pLbHatchBckgrdColor->SelectEntry( aColorItem.GetColorValue() );
                }
            break;

            case XFILL_BITMAP:
            {
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLBITMAP ) )
                {
                    XFillBitmapItem aBitmapItem( ( const XFillBitmapItem& )
                                        rAttrs.Get( XATTR_FILLBITMAP ) );

                    String aString( aBitmapItem.GetName() );
                    m_pLbBitmap->SelectEntry( aString );
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
        m_pFillLB->Hide();

        m_pCtlBitmapPreview->Hide();
        m_pLbColor->Disable();
        m_pLbColor->Show();

        // so that Reset() also works correctly with Back
        m_pTypeLB->SetNoSelection();
    }

    // step size
    if( ( rAttrs.GetItemState( XATTR_GRADIENTSTEPCOUNT ) != SFX_ITEM_DONTCARE ) ||
        ( rAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE ) )
    {
        m_pTsbStepCount->EnableTriState( sal_False );
        sal_uInt16 nValue = ( ( const XGradientStepCountItem& ) rAttrs.Get( XATTR_GRADIENTSTEPCOUNT ) ).GetValue();
        if( nValue == 0 )
        {
            m_pTsbStepCount->SetState( STATE_CHECK );
            m_pNumFldStepCount->SetText( String() );
        }
        else
        {
            m_pTsbStepCount->SetState( STATE_NOCHECK );
            m_pNumFldStepCount->SetValue( nValue );
        }
        ModifyStepCountHdl_Impl( m_pTsbStepCount );
    }
    else
    {
        m_pTsbStepCount->SetState( STATE_DONTKNOW );
        m_pNumFldStepCount->SetText( String() );
    }

    // attributes for the bitmap filling

    if( rAttrs.GetItemState( XATTR_FILLBMP_TILE ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbTile->EnableTriState( sal_False );

        if( ( ( const XFillBmpTileItem& ) rAttrs.Get( XATTR_FILLBMP_TILE ) ).GetValue() )
            m_pTsbTile->SetState( STATE_CHECK );
        else
            m_pTsbTile->SetState( STATE_NOCHECK );
    }
    else
        m_pTsbTile->SetState( STATE_DONTKNOW );

    if( rAttrs.GetItemState( XATTR_FILLBMP_STRETCH ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbStretch->EnableTriState( sal_False );

        if( ( ( const XFillBmpStretchItem& ) rAttrs.Get( XATTR_FILLBMP_STRETCH ) ).GetValue() )
            m_pTsbStretch->SetState( STATE_CHECK );
        else
            m_pTsbStretch->SetState( STATE_NOCHECK );
    }
    else
        m_pTsbStretch->SetState( STATE_DONTKNOW );


    //aTsbScale
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZELOG ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbScale->EnableTriState( sal_False );

        if( ( ( const XFillBmpSizeLogItem& ) rAttrs.Get( XATTR_FILLBMP_SIZELOG ) ).GetValue() )
            m_pTsbScale->SetState( STATE_NOCHECK );
        else
            m_pTsbScale->SetState( STATE_CHECK );

        ClickScaleHdl_Impl( NULL );
    }
    else
        m_pTsbScale->SetState( STATE_DONTKNOW );


    // determine status for the original size
    TriState eOriginal = STATE_NOCHECK;

    //aMtrFldXSize
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZEX ) != SFX_ITEM_DONTCARE )
    {
        sal_Int32 nValue = ( ( const XFillBmpSizeXItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        if( m_pTsbScale->GetState() == STATE_CHECK )
        {
            // If there's a percentage value in the item,
            // it is negative because of the MetricItems.
            m_pMtrFldXSize->SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( *m_pMtrFldXSize, nValue, ePoolUnit );
        m_pMtrFldXSize->SaveValue();

        if( nValue == 0 )
        {
            eOriginal = STATE_CHECK;
            // value would be too small otherwise when turning off the original size
            // (performance problem)
            m_pMtrFldXSize->SetValue( 100 );
        }
    }
    else
    {
        m_pMtrFldXSize->SetText( String() );
        m_pMtrFldXSize->SaveValue();
    }

    //aMtrFldYSize
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZEY ) != SFX_ITEM_DONTCARE )
    {
        sal_Int32 nValue = ( ( const XFillBmpSizeYItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEY ) ).GetValue();
        if( m_pTsbScale->GetState() == STATE_CHECK )
        {
            // If there's a percentage value in the item,
            // it is negative because of the MetricItems.
            m_pMtrFldYSize->SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( *m_pMtrFldYSize, nValue, ePoolUnit );
        m_pMtrFldYSize->SaveValue();

        if( nValue == 0 )
            m_pMtrFldYSize->SetValue( 100 ); //s.o.
        else
            eOriginal = STATE_NOCHECK;
    }
    else
    {
        m_pMtrFldYSize->SetText( String() );
        m_pMtrFldYSize->SaveValue();
        eOriginal = STATE_NOCHECK;
    }

    // aTsbOriginal
    m_pTsbOriginal->SetState( eOriginal );

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
            m_pMtrFldOffset->SetValue( nValue );
            m_pRbtRow->Check();
        }
        else if( rAttrs.GetItemState( XATTR_FILLBMP_TILEOFFSETY ) != SFX_ITEM_DONTCARE )
        {
            nValue = ( ( const XFillBmpTileOffsetYItem& ) rAttrs.Get( XATTR_FILLBMP_TILEOFFSETY ) ).GetValue();
            if( nValue > 0 )
            {
                m_pMtrFldOffset->SetValue( nValue );
                m_pRbtColumn->Check();
            }
        }
        else
            m_pMtrFldOffset->SetValue( 0 );
    }
    else
        m_pMtrFldOffset->SetText( String() );


    //aCtlPosition
    if( rAttrs.GetItemState( XATTR_FILLBMP_POS ) != SFX_ITEM_DONTCARE )
    {
        RECT_POINT eValue = ( ( const XFillBmpPosItem& ) rAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
        m_pCtlPosition->SetActualRP( eValue );
    }
    else
        m_pCtlPosition->Reset();

    //aMtrFldXOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_POSOFFSETX ) != SFX_ITEM_DONTCARE )
    {
        sal_Int32 nValue = ( ( const XFillBmpPosOffsetXItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETX ) ).GetValue();
        m_pMtrFldXOffset->SetValue( nValue );
    }
    else
        m_pMtrFldXOffset->SetText( String() );

    //aMtrFldYOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_POSOFFSETY ) != SFX_ITEM_DONTCARE )
    {
        sal_Int32 nValue = ( ( const XFillBmpPosOffsetYItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETY ) ).GetValue();
        m_pMtrFldYOffset->SetValue( nValue );
    }
    else
        m_pMtrFldYOffset->SetText( String() );

    // not earlier so that tile and stretch are considered
    if( m_pTypeLB->GetSelectEntryPos() == XFILL_BITMAP )
        ClickBitmapHdl_Impl();

    m_pTypeLB->SaveValue();
    m_pLbColor->SaveValue();
    m_pLbGradient->SaveValue();
    m_pLbHatching->SaveValue();
    m_pLbHatchBckgrdColor->SaveValue();
    m_pLbBitmap->SaveValue();
    m_pTsbStepCount->SaveValue();
    m_pNumFldStepCount->SaveValue();
    m_pTsbTile->SaveValue();
    m_pTsbStretch->SaveValue();
    m_pTsbScale->SaveValue();
    m_pRbtRow->SaveValue();
    m_pRbtColumn->SaveValue();
    m_pMtrFldOffset->SaveValue();
    m_pMtrFldXOffset->SaveValue();
    m_pMtrFldYOffset->SaveValue();
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
    switch( (XFillStyle)m_pTypeLB->GetSelectEntryPos() )
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
    m_pBxBitmap->Hide();

    m_pFillLB->Hide();

    m_pCtlXRectPreview->Hide();
    m_pCtlBitmapPreview->Hide();

    m_pFlStepCount->Hide();

    // Controls for Hatch-Background
    m_pFlHatchBckgrd->Hide();

    rXFSet.Put( XFillStyleItem( XFILL_NONE ) );
    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );

    m_pCtlXRectPreview->Invalidate();
    m_pCtlBitmapPreview->Invalidate();
}

//------------------------------------------------------------------------

void SvxAreaTabPage::ClickColorHdl_Impl()
{
    m_pBxBitmap->Hide();

    m_pFillLB->Show();
    m_pLbColor->Enable();
    m_pLbColor->Show();
    m_pLbGradient->Hide();
    m_pLbHatching->Hide();
    m_pLbBitmap->Hide();
    m_pCtlXRectPreview->Enable();
    m_pCtlXRectPreview->Show();
    m_pCtlBitmapPreview->Hide();

    m_pFlStepCount->Hide();

    // Controls for Hatch-Background
    m_pFlHatchBckgrd->Hide();

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
    sal_uInt16 _nPos = m_pLbColor->GetSelectEntryPos();
    m_pLbHatchBckgrdColor->SelectEntryPos( _nPos );
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        rXFSet.Put( XFillColorItem( String(),
                                    m_pLbColor->GetSelectEntryColor() ) );
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

    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();

    return( 0L );
}
//------------------------------------------------------------------------

void SvxAreaTabPage::ClickGradientHdl_Impl()
{
    m_pBxBitmap->Hide();

    m_pFillLB->Show();
    m_pLbColor->Hide();
    m_pLbGradient->Enable();
    m_pLbGradient->Show();
    m_pLbHatching->Hide();
    m_pLbBitmap->Hide();
    m_pCtlXRectPreview->Enable();
    m_pCtlXRectPreview->Show();
    m_pCtlBitmapPreview->Hide();

    m_pFlStepCount->Enable();
    m_pFlStepCount->Show();
    m_pTsbStepCount->Enable();
    m_pTsbStepCount->Show();
    m_pNumFldStepCount->Show();

    // Controls for Hatch-Background
    m_pFlHatchBckgrd->Hide();

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
    ModifyStepCountHdl_Impl( m_pTsbStepCount );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyGradientHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = m_pLbGradient->GetSelectEntryPos();
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

    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

void SvxAreaTabPage::ClickHatchingHdl_Impl()
{
    m_pFillLB->Show();
    m_pLbColor->Hide();
    m_pLbGradient->Hide();
    m_pLbHatching->Enable();
    m_pLbHatching->Show();
    m_pLbBitmap->Hide();
    m_pCtlXRectPreview->Enable();
    m_pCtlXRectPreview->Show();
    m_pCtlBitmapPreview->Hide();

    m_pFlStepCount->Hide();

    m_pBxBitmap->Hide();

    // Controls for Hatch-Background
    m_pFlHatchBckgrd->Show();
    m_pCbxHatchBckgrd->Enable();
    m_pLbHatchBckgrdColor->Enable();

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
    sal_uInt16 _nPos = m_pLbHatching->GetSelectEntryPos();
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

    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyHatchBckgrdColorHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = m_pLbHatchBckgrdColor->GetSelectEntryPos();
    m_pLbColor->SelectEntryPos( _nPos );
    if( _nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillColorItem( String(), m_pLbHatchBckgrdColor->GetSelectEntryColor() ) );
    }
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), sal_True, &pPoolItem ) )
    {
        Color aColor( ( ( const XFillColorItem* ) pPoolItem )->GetColorValue() );
        rXFSet.Put( XFillColorItem( String(), aColor ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ToggleHatchBckgrdColorHdl_Impl)
{
    // switch on/off backgroundcolor for hatches
    m_pLbHatchBckgrdColor->Enable( m_pCbxHatchBckgrd->IsChecked() );

    XFillBackgroundItem aItem( m_pCbxHatchBckgrd->IsChecked() );
    rXFSet.Put ( aItem, XATTR_FILLBACKGROUND );

    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();

    if( m_pLbHatchBckgrdColor->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
    {
        if ( SFX_ITEM_SET == rOutAttrs.GetItemState( XATTR_FILLCOLOR ) )//>= SFX_ITEM_DEFAULT )
        {
            XFillColorItem aColorItem( (const XFillColorItem&)rOutAttrs.Get( XATTR_FILLCOLOR ) );
            m_pLbHatchBckgrdColor->SelectEntry( aColorItem.GetColorValue() );
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

void SvxAreaTabPage::ClickBitmapHdl_Impl()
{
    m_pFillLB->Show();
    m_pLbColor->Hide();
    m_pLbGradient->Hide();
    m_pLbHatching->Hide();
    m_pLbBitmap->Enable();
    m_pLbBitmap->Show();
    m_pCtlBitmapPreview->Enable();
    m_pCtlBitmapPreview->Show();
    m_pCtlXRectPreview->Hide();

    m_pFlStepCount->Hide();

    m_pBxTile->Enable();

    m_pFlSize->Enable();
    m_pFlPosition->Enable();
    m_pFlOffset->Enable();

    // Controls for Hatch-Background
    m_pFlHatchBckgrd->Hide();

    m_pBxBitmap->Show();

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
    ModifyTileHdl_Impl( m_pTsbOriginal );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyBitmapHdl_Impl)
{
    const SfxPoolItem* pPoolItem = NULL;
    sal_uInt16 _nPos = m_pLbBitmap->GetSelectEntryPos();
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

    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyStepCountHdl_Impl, void *, p )
{
    if( p == m_pTsbStepCount )
    {
        if( m_pTsbStepCount->GetState() == STATE_NOCHECK )
        {
            if( m_pNumFldStepCount->GetText().isEmpty() )
                m_pNumFldStepCount->SetText(OUString("64"));

            m_pNumFldStepCount->Enable();
        }
        else
            m_pNumFldStepCount->Disable();
    }

    sal_uInt16 nValue = 0;
    if( m_pTsbStepCount->GetState() != STATE_CHECK )
    {
        // condition != Disabled ?
        if( !m_pNumFldStepCount->GetText().isEmpty() )
            nValue = (sal_uInt16) m_pNumFldStepCount->GetValue();
    }
    rXFSet.Put( XGradientStepCountItem( nValue ) );
    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlXRectPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ModifyTileHdl_Impl)
{
    TriState eState = m_pTsbTile->GetState();
    if( eState == STATE_CHECK )
    {
        m_pTsbStretch->Disable();
        m_pFlOffset->Enable();

        m_pCtlPosition->Invalidate();
        m_pFlPosition->Enable();

        m_pFlSize->Enable();
    }
    else if( eState == STATE_NOCHECK )
    {
        m_pTsbStretch->Enable();
        m_pFlOffset->Disable();

        m_pCtlPosition->Invalidate();
        m_pFlPosition->Disable();

        if( m_pTsbStretch->GetState() != STATE_NOCHECK )
        {

            m_pFlSize->Disable();
        }
        else
        {
            m_pFlSize->Enable();
        }
    }
    else
    {
        m_pTsbStretch->Disable();
        m_pFlOffset->Disable();

        m_pCtlPosition->Invalidate();
        m_pFlPosition->Disable();

        m_pFlSize->Disable();
    }

    if( m_pTsbOriginal->GetState() == STATE_CHECK )
    {
        m_pMtrFldXSize->SetText( String() );
        m_pMtrFldYSize->SetText( String() );
        m_pGridX_Y->Disable();
        m_pTsbScale->Disable();
    }
    else
    {
        m_pMtrFldXSize->SetValue( m_pMtrFldXSize->GetValue() );
        m_pMtrFldYSize->SetValue( m_pMtrFldYSize->GetValue() );
    }

    rXFSet.Put( XFillBmpTileItem( sal::static_int_cast< sal_Bool >( eState ) ) );

    if( m_pTsbStretch->IsEnabled() )
        rXFSet.Put(
            XFillBmpStretchItem(
                sal::static_int_cast< sal_Bool >( m_pTsbStretch->GetState() ) ) );

    if( m_pTsbScale->IsEnabled() )
        rXFSet.Put( XFillBmpSizeLogItem( m_pTsbScale->GetState() == STATE_NOCHECK ) );

    if( m_pMtrFldXSize->IsEnabled() )
    {
        XFillBmpSizeXItem* pItem = NULL;
        TriState eScaleState = m_pTsbScale->GetState();

        if( eScaleState == STATE_NOCHECK )
            pItem = new XFillBmpSizeXItem( GetCoreValue( *m_pMtrFldXSize, ePoolUnit ) );
        else
            pItem = new XFillBmpSizeXItem( -labs( static_cast<long>(m_pMtrFldXSize->GetValue()) ) );

        rXFSet.Put( *pItem );

        delete pItem;
    }
    else if( m_pTsbOriginal->IsEnabled() && m_pTsbOriginal->GetState() == STATE_CHECK )
    {
        // original size -> size == 0
        rXFSet.Put( XFillBmpSizeXItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( sal_True ) );
    }

    if( m_pMtrFldYSize->IsEnabled() )
    {
        XFillBmpSizeYItem* pItem = NULL;
        TriState eScaleState = m_pTsbScale->GetState();

        if( eScaleState == STATE_NOCHECK )
            pItem = new XFillBmpSizeYItem( GetCoreValue( *m_pMtrFldYSize, ePoolUnit ) );
        else
            pItem = new XFillBmpSizeYItem( -labs( static_cast<long>(m_pMtrFldYSize->GetValue()) ) );

        rXFSet.Put( *pItem );

        delete pItem;
    }
    else if( m_pTsbOriginal->IsEnabled() && m_pTsbOriginal->GetState() == STATE_CHECK )
    {
        // original size -> size == 0
        rXFSet.Put( XFillBmpSizeYItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( sal_True ) );
    }

    if( m_pMtrFldOffset->IsEnabled() )
    {
        if( m_pRbtRow->IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (sal_uInt16) m_pMtrFldOffset->GetValue() ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (sal_uInt16) 0 ) );
        }
        else if( m_pRbtColumn->IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (sal_uInt16) 0 ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (sal_uInt16) m_pMtrFldOffset->GetValue() ) );
        }
    }

    if( m_pCtlPosition->IsEnabled() )
        rXFSet.Put( XFillBmpPosItem( m_pCtlPosition->GetActualRP() ) );

    if( m_pMtrFldXOffset->IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetXItem( (sal_uInt16) m_pMtrFldXOffset->GetValue() ) );

    if( m_pMtrFldYOffset->IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetYItem( (sal_uInt16) m_pMtrFldYOffset->GetValue() ) );


    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxAreaTabPage, ClickScaleHdl_Impl)
{
    if( m_pTsbScale->GetState() == STATE_CHECK )
    {
        m_pMtrFldXSize->SetDecimalDigits( 0 );
        m_pMtrFldXSize->SetUnit( FUNIT_CUSTOM );
        m_pMtrFldXSize->SetValue( 100 );
        m_pMtrFldXSize->SetMax( 100 );
        m_pMtrFldXSize->SetLast( 100 );

        m_pMtrFldYSize->SetDecimalDigits( 0 );
        m_pMtrFldYSize->SetUnit( FUNIT_CUSTOM );
        m_pMtrFldYSize->SetValue( 100 );
        m_pMtrFldYSize->SetMax( 100 );
        m_pMtrFldYSize->SetLast( 100 );
    }
    else
    {
        m_pMtrFldXSize->SetDecimalDigits( 2 );
        m_pMtrFldXSize->SetUnit( eFUnit );
        m_pMtrFldXSize->SetValue( 100 );
        m_pMtrFldXSize->SetMax( 999900 );
        m_pMtrFldXSize->SetLast( 100000 );

        m_pMtrFldYSize->SetDecimalDigits( 2 );
        m_pMtrFldYSize->SetUnit( eFUnit );
        m_pMtrFldYSize->SetValue( 100 );
        m_pMtrFldYSize->SetMax( 999900 );
        m_pMtrFldYSize->SetLast( 100000 );
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
