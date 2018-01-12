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

#include <stdlib.h>

#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>

#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/xflbckit.hxx>
#include <svx/svdattr.hxx>
#include <svx/xtable.hxx>
#include <svx/xlineit0.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>
#include <dlgname.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>

#include <sfx2/opengrf.hxx>
#include <vcl/layout.hxx>

using namespace com::sun::star;

const sal_uInt16 SvxTransparenceTabPage::pTransparenceRanges[] =
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

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransOffHdl_Impl, Button*, void)
{
    // disable all other controls
    ActivateLinear(false);
    ActivateGradient(false);

    // Preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );

    InvalidatePreview( false );
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransLinearHdl_Impl, Button*, void)
{
    // enable linear, disable other
    ActivateLinear(true);
    ActivateGradient(false);

    // preview
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    ModifyTransparentHdl_Impl (*m_pMtrTransparent);
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransGradientHdl_Impl, Button*, void)
{
    // enable gradient, disable other
    ActivateLinear(false);
    ActivateGradient(true);

    // preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    ModifiedTrgrHdl_Impl (nullptr);
}

SvxTransparenceTabPage::~SvxTransparenceTabPage()
{
    disposeOnce();
}

void SvxTransparenceTabPage::dispose()
{
    m_pRbtTransOff.clear();
    m_pRbtTransLinear.clear();
    m_pRbtTransGradient.clear();
    m_pMtrTransparent.clear();
    m_pGridGradient.clear();
    m_pLbTrgrGradientType.clear();
    m_pFtTrgrCenterX.clear();
    m_pMtrTrgrCenterX.clear();
    m_pFtTrgrCenterY.clear();
    m_pMtrTrgrCenterY.clear();
    m_pFtTrgrAngle.clear();
    m_pMtrTrgrAngle.clear();
    m_pMtrTrgrBorder.clear();
    m_pMtrTrgrStartValue.clear();
    m_pMtrTrgrEndValue.clear();
    m_pCtlBitmapPreview.clear();
    m_pCtlXRectPreview.clear();
    SvxTabPage::dispose();
}

void SvxTransparenceTabPage::ActivateLinear(bool bActivate)
{
    m_pMtrTransparent->Enable(bActivate);
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ModifyTransparentHdl_Impl, Edit&, void)
{
    sal_uInt16 nPos = static_cast<sal_uInt16>(m_pMtrTransparent->GetValue());
    rXFSet.Put(XFillTransparenceItem(nPos));

    // preview
    InvalidatePreview();
}

IMPL_LINK(SvxTransparenceTabPage, ModifiedTrgrListBoxHdl_Impl, ListBox&, rListBox, void)
{
    ModifiedTrgrHdl_Impl(&rListBox);
}

IMPL_LINK(SvxTransparenceTabPage, ModifiedTrgrEditHdl_Impl, Edit&, rBox, void)
{
    ModifiedTrgrHdl_Impl(&rBox);
}

void SvxTransparenceTabPage::ModifiedTrgrHdl_Impl(void const * pControl)
{
    if(pControl == m_pLbTrgrGradientType || pControl == this)
    {
        css::awt::GradientStyle eXGS = (css::awt::GradientStyle)m_pLbTrgrGradientType->GetSelectedEntryPos();
        SetControlState_Impl( eXGS );
    }

    // preview
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_pMtrTrgrStartValue->GetValue()) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_pMtrTrgrEndValue->GetValue()) * 255) / 100);
    XGradient aTmpGradient(
                Color(nStartCol, nStartCol, nStartCol),
                Color(nEndCol, nEndCol, nEndCol),
                (css::awt::GradientStyle)m_pLbTrgrGradientType->GetSelectedEntryPos(),
                static_cast<sal_uInt16>(m_pMtrTrgrAngle->GetValue()) * 10,
                static_cast<sal_uInt16>(m_pMtrTrgrCenterX->GetValue()),
                static_cast<sal_uInt16>(m_pMtrTrgrCenterY->GetValue()),
                static_cast<sal_uInt16>(m_pMtrTrgrBorder->GetValue()),
                100, 100);

    XFillFloatTransparenceItem aItem( aTmpGradient);
    rXFSet.Put ( aItem );

    InvalidatePreview();
}

void SvxTransparenceTabPage::ActivateGradient(bool bActivate)
{
    m_pGridGradient->Enable(bActivate);

    if(bActivate)
    {
        css::awt::GradientStyle eXGS = (css::awt::GradientStyle)m_pLbTrgrGradientType->GetSelectedEntryPos();
        SetControlState_Impl( eXGS );
    }
}

void SvxTransparenceTabPage::SetControlState_Impl(css::awt::GradientStyle eXGS)
{
    switch(eXGS)
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            m_pFtTrgrCenterX->Disable();
            m_pMtrTrgrCenterX->Disable();
            m_pFtTrgrCenterY->Disable();
            m_pMtrTrgrCenterY->Disable();
            m_pFtTrgrAngle->Enable();
            m_pMtrTrgrAngle->Enable();
            break;

        case css::awt::GradientStyle_RADIAL:
            m_pFtTrgrCenterX->Enable();
            m_pMtrTrgrCenterX->Enable();
            m_pFtTrgrCenterY->Enable();
            m_pMtrTrgrCenterY->Enable();
            m_pFtTrgrAngle->Disable();
            m_pMtrTrgrAngle->Disable();
            break;

        case css::awt::GradientStyle_ELLIPTICAL:
            m_pFtTrgrCenterX->Enable();
            m_pMtrTrgrCenterX->Enable();
            m_pFtTrgrCenterY->Enable();
            m_pMtrTrgrCenterY->Enable();
            m_pFtTrgrAngle->Enable();
            m_pMtrTrgrAngle->Enable();
            break;

        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            m_pFtTrgrCenterX->Enable();
            m_pMtrTrgrCenterX->Enable();
            m_pFtTrgrCenterY->Enable();
            m_pMtrTrgrCenterY->Enable();
            m_pFtTrgrAngle->Enable();
            m_pMtrTrgrAngle->Enable();
            break;
        default:
            break;
    }
}

SvxTransparenceTabPage::SvxTransparenceTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
:   SvxTabPage          ( pParent,
                          "TransparencyTabPage",
                          "cui/ui/transparencytabpage.ui",
                          rInAttrs),
    rOutAttrs           ( rInAttrs ),
    nPageType           ( PageType::Area ),
    nDlgType            (0),
    bBitmap             ( false ),
    aXFillAttr          ( rInAttrs.GetPool() ),
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
    get(m_pFtTrgrAngle,"FT_TRGR_ANGLE");
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
    Link<Edit&,void> aLink = LINK( this, SvxTransparenceTabPage, ModifiedTrgrEditHdl_Impl);
    m_pLbTrgrGradientType->SetSelectHdl( LINK( this, SvxTransparenceTabPage, ModifiedTrgrListBoxHdl_Impl) );
    m_pMtrTrgrCenterX->SetModifyHdl( aLink );
    m_pMtrTrgrCenterY->SetModifyHdl( aLink );
    m_pMtrTrgrAngle->SetModifyHdl( aLink );
    m_pMtrTrgrBorder->SetModifyHdl( aLink );
    m_pMtrTrgrStartValue->SetModifyHdl( aLink );
    m_pMtrTrgrEndValue->SetModifyHdl( aLink );

    // this page needs ExchangeSupport
    SetExchangeSupport();
}

VclPtr<SfxTabPage> SvxTransparenceTabPage::Create(vcl::Window* pWindow, const SfxItemSet* rAttrs)
{
    return VclPtr<SvxTransparenceTabPage>::Create(pWindow, *rAttrs);
}

bool SvxTransparenceTabPage::FillItemSet(SfxItemSet* rAttrs)
{
    const SfxPoolItem* pGradientItem = nullptr;
    const SfxPoolItem* pLinearItem = nullptr;
    SfxItemState eStateGradient(rOutAttrs.GetItemState(XATTR_FILLFLOATTRANSPARENCE, true, &pGradientItem));
    SfxItemState eStateLinear(rOutAttrs.GetItemState(XATTR_FILLTRANSPARENCE, true, &pLinearItem));
    bool bGradActive = (eStateGradient == SfxItemState::SET && static_cast<const XFillFloatTransparenceItem*>(pGradientItem)->IsEnabled());
    bool bLinearActive = (eStateLinear == SfxItemState::SET && static_cast<const XFillTransparenceItem*>(pLinearItem)->GetValue() != 0);

    bool bGradUsed = (eStateGradient == SfxItemState::DONTCARE);
    bool bLinearUsed = (eStateLinear == SfxItemState::DONTCARE);

    bool bModified(false);
    bool bSwitchOffLinear(false);
    bool bSwitchOffGradient(false);

    if(m_pMtrTransparent->IsEnabled())
    {
        // linear transparence
        sal_uInt16 nPos = static_cast<sal_uInt16>(m_pMtrTransparent->GetValue());
        if(m_pMtrTransparent->IsValueChangedFromSaved() || !bLinearActive)
        {
            XFillTransparenceItem aItem(nPos);
            SdrPercentItem aShadowItem(makeSdrShadowTransparenceItem(nPos));
            const SfxPoolItem* pOld = GetOldItem(*rAttrs, XATTR_FILLTRANSPARENCE);
            if(!pOld || !(*static_cast<const XFillTransparenceItem*>(pOld) == aItem) || !bLinearActive)
            {
                rAttrs->Put(aItem);
                rAttrs->Put(aShadowItem);
                bModified = true;
                bSwitchOffGradient = true;
            }
        }
    }
    else if(m_pLbTrgrGradientType->IsEnabled())
    {
        // transparence gradient, fill ItemSet from values
        if(!bGradActive
            || m_pLbTrgrGradientType->IsValueChangedFromSaved()
            || m_pMtrTrgrAngle->IsValueChangedFromSaved()
            || m_pMtrTrgrCenterX->IsValueChangedFromSaved()
            || m_pMtrTrgrCenterY->IsValueChangedFromSaved()
            || m_pMtrTrgrBorder->IsValueChangedFromSaved()
            || m_pMtrTrgrStartValue->IsValueChangedFromSaved()
            || m_pMtrTrgrEndValue->IsValueChangedFromSaved() )
        {
            sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_pMtrTrgrStartValue->GetValue()) * 255) / 100);
            sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_pMtrTrgrEndValue->GetValue()) * 255) / 100);
            XGradient aTmpGradient(
                        Color(nStartCol, nStartCol, nStartCol),
                        Color(nEndCol, nEndCol, nEndCol),
                        (css::awt::GradientStyle)m_pLbTrgrGradientType->GetSelectedEntryPos(),
                        static_cast<sal_uInt16>(m_pMtrTrgrAngle->GetValue()) * 10,
                        static_cast<sal_uInt16>(m_pMtrTrgrCenterX->GetValue()),
                        static_cast<sal_uInt16>(m_pMtrTrgrCenterY->GetValue()),
                        static_cast<sal_uInt16>(m_pMtrTrgrBorder->GetValue()),
                        100, 100);

            XFillFloatTransparenceItem aItem(aTmpGradient);
            const SfxPoolItem* pOld = GetOldItem(*rAttrs, XATTR_FILLFLOATTRANSPARENCE);

            if(!pOld || !(*static_cast<const XFillFloatTransparenceItem*>(pOld) == aItem) || !bGradActive)
            {
                rAttrs->Put(aItem);
                bModified = true;
                bSwitchOffLinear = true;
            }
        }
    }
    else
    {
        // no transparence
        bSwitchOffGradient = true;
        bSwitchOffLinear = true;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffGradient && (bGradActive || bGradUsed))
    {
        Color aColor(COL_BLACK);
        XGradient aGrad(aColor, Color(COL_WHITE));
        aGrad.SetStartIntens(100);
        aGrad.SetEndIntens(100);
        XFillFloatTransparenceItem aItem(aGrad);
        aItem.SetEnabled(false);
        rAttrs->Put(aItem);
        bModified = true;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffLinear && (bLinearActive || bLinearUsed))
    {
        XFillTransparenceItem aItem(0);
        SdrPercentItem aShadowItem(makeSdrShadowTransparenceItem(0));
        rAttrs->Put(aItem);
        rAttrs->Put(aShadowItem);
        bModified = true;
    }
    rAttrs->Put(CntUInt16Item(SID_PAGE_TYPE, (sal_uInt16)nPageType));
    return bModified;
}

void SvxTransparenceTabPage::Reset(const SfxItemSet* rAttrs)
{
    const SfxPoolItem* pGradientItem = nullptr;
    SfxItemState eStateGradient(rAttrs->GetItemState(XATTR_FILLFLOATTRANSPARENCE, true, &pGradientItem));
    if(!pGradientItem)
        pGradientItem = &rAttrs->Get(XATTR_FILLFLOATTRANSPARENCE);
    bool bGradActive = (eStateGradient == SfxItemState::SET && static_cast<const XFillFloatTransparenceItem*>(pGradientItem)->IsEnabled());

    const SfxPoolItem* pLinearItem = nullptr;
    SfxItemState eStateLinear(rAttrs->GetItemState(XATTR_FILLTRANSPARENCE, true, &pLinearItem));
    if(!pLinearItem)
        pLinearItem = &rAttrs->Get(XATTR_FILLTRANSPARENCE);
    bool bLinearActive = (eStateLinear == SfxItemState::SET && static_cast<const XFillTransparenceItem*>(pLinearItem)->GetValue() != 0);

    // transparence gradient
    const XGradient& rGradient = static_cast<const XFillFloatTransparenceItem*>(pGradientItem)->GetGradientValue();
    css::awt::GradientStyle eXGS(rGradient.GetGradientStyle());
    m_pLbTrgrGradientType->SelectEntryPos(sal::static_int_cast< sal_Int32 >(eXGS));
    m_pMtrTrgrAngle->SetValue(rGradient.GetAngle() / 10);
    m_pMtrTrgrBorder->SetValue(rGradient.GetBorder());
    m_pMtrTrgrCenterX->SetValue(rGradient.GetXOffset());
    m_pMtrTrgrCenterY->SetValue(rGradient.GetYOffset());
    m_pMtrTrgrStartValue->SetValue(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(rGradient.GetStartColor().GetRed()) + 1) * 100) / 255));
    m_pMtrTrgrEndValue->SetValue(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(rGradient.GetEndColor().GetRed()) + 1) * 100) / 255));

    // linear transparence
    sal_uInt16 nTransp = static_cast<const XFillTransparenceItem*>(pLinearItem)->GetValue();
    m_pMtrTransparent->SetValue(bLinearActive ? nTransp : 50);
    ModifyTransparentHdl_Impl(*m_pMtrTransparent);

    // select the correct radio button
    if(bGradActive)
    {
        // transparence gradient, set controls appropriate to item
        m_pRbtTransGradient->Check();
        ClickTransGradientHdl_Impl(nullptr);
    }
    else if(bLinearActive)
    {
        // linear transparence
        m_pRbtTransLinear->Check();
        ClickTransLinearHdl_Impl(nullptr);
    }
    else
    {
        // no transparence
        m_pRbtTransOff->Check();
        ClickTransOffHdl_Impl(nullptr);
        ModifiedTrgrHdl_Impl(nullptr);
    }

    // save values
    ChangesApplied();
    bool bActive = InitPreview ( *rAttrs );
    InvalidatePreview ( bActive );
}

void SvxTransparenceTabPage::ChangesApplied()
{
    m_pMtrTransparent->SaveValue();
    m_pLbTrgrGradientType->SaveValue();
    m_pMtrTrgrCenterX->SaveValue();
    m_pMtrTrgrCenterY->SaveValue();
    m_pMtrTrgrAngle->SaveValue();
    m_pMtrTrgrBorder->SaveValue();
    m_pMtrTrgrStartValue->SaveValue();
    m_pMtrTrgrEndValue->SaveValue();
}

void SvxTransparenceTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const CntUInt16Item* pPageTypeItem = rSet.GetItem<CntUInt16Item>(SID_PAGE_TYPE, false);
    if (pPageTypeItem)
        SetPageType((PageType) pPageTypeItem->GetValue());

    if(nDlgType == 0) // area dialog
        nPageType = PageType::Transparence;

    InitPreview ( rSet );
}

DeactivateRC SvxTransparenceTabPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void SvxTransparenceTabPage::PointChanged(vcl::Window* , RectPoint )
{
}


// Preview-Methods

bool SvxTransparenceTabPage::InitPreview ( const SfxItemSet& rSet )
{
    // set transparencetype for preview
    if ( m_pRbtTransOff->IsChecked() )
    {
        ClickTransOffHdl_Impl(nullptr);
    } else if ( m_pRbtTransLinear->IsChecked() )
    {
        ClickTransLinearHdl_Impl(nullptr);
    } else if ( m_pRbtTransGradient->IsChecked() )
    {
        ClickTransGradientHdl_Impl(nullptr);
    }

    // Get fillstyle for preview
    rXFSet.Put ( rSet.Get(XATTR_FILLSTYLE) );
    rXFSet.Put ( rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( rSet.Get(XATTR_FILLGRADIENT) );
    rXFSet.Put ( rSet.Get(XATTR_FILLHATCH) );
    rXFSet.Put ( rSet.Get(XATTR_FILLBACKGROUND) );
    rXFSet.Put ( rSet.Get(XATTR_FILLBITMAP) );

    m_pCtlXRectPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlBitmapPreview->SetAttributes( aXFillAttr.GetItemSet() );

    bBitmap = rSet.Get(XATTR_FILLSTYLE).GetValue() == drawing::FillStyle_BITMAP;

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

void SvxTransparenceTabPage::InvalidatePreview (bool bEnable)
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

void SvxTransparenceTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pPageTypeItem = aSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pDlgTypeItem = aSet.GetItem<SfxUInt16Item>(SID_DLG_TYPE, false);

    if (pPageTypeItem)
        SetPageType((PageType) pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
