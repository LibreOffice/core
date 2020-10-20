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

#include <svx/svxids.hrc>

#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbckit.hxx>
#include <svx/sdshtitm.hxx>
#include <svx/xfltrit.hxx>
#include <cuitabarea.hxx>
#include <svl/intitem.hxx>

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

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransOffHdl_Impl, weld::ToggleButton&, void)
{
    // disable all other controls
    ActivateLinear(false);
    ActivateGradient(false);

    // Preview
    rXFSet.ClearItem(XATTR_FILLTRANSPARENCE);
    rXFSet.ClearItem(XATTR_FILLFLOATTRANSPARENCE);
    m_aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    m_aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );

    InvalidatePreview(false);
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransLinearHdl_Impl, weld::ToggleButton&, void)
{
    // enable linear, disable other
    ActivateLinear(true);
    ActivateGradient(false);

    // preview
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    ModifyTransparentHdl_Impl(*m_xMtrTransparent);
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ClickTransGradientHdl_Impl, weld::ToggleButton&, void)
{
    // enable gradient, disable other
    ActivateLinear(false);
    ActivateGradient(true);

    // preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    ModifiedTrgrHdl_Impl(nullptr);
}

SvxTransparenceTabPage::~SvxTransparenceTabPage()
{
}

void SvxTransparenceTabPage::ActivateLinear(bool bActivate)
{
    m_xMtrTransparent->set_sensitive(bActivate);
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ModifyTransparentHdl_Impl, weld::MetricSpinButton&, void)
{
    sal_uInt16 nPos = m_xMtrTransparent->get_value(FieldUnit::PERCENT);
    rXFSet.Put(XFillTransparenceItem(nPos));

    // preview
    InvalidatePreview();
}

IMPL_LINK(SvxTransparenceTabPage, ModifiedTrgrListBoxHdl_Impl, weld::ComboBox&, rListBox, void)
{
    ModifiedTrgrHdl_Impl(&rListBox);
}

IMPL_LINK_NOARG(SvxTransparenceTabPage, ModifiedTrgrEditHdl_Impl, weld::MetricSpinButton&, void)
{
    ModifiedTrgrHdl_Impl(nullptr);
}

void SvxTransparenceTabPage::ModifiedTrgrHdl_Impl(const weld::ComboBox* pControl)
{
    if (pControl == m_xLbTrgrGradientType.get())
    {
        css::awt::GradientStyle eXGS = static_cast<css::awt::GradientStyle>(m_xLbTrgrGradientType->get_active());
        SetControlState_Impl( eXGS );
    }

    // preview
    sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_xMtrTrgrStartValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_xMtrTrgrEndValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
    XGradient aTmpGradient(
                Color(nStartCol, nStartCol, nStartCol),
                Color(nEndCol, nEndCol, nEndCol),
                static_cast<css::awt::GradientStyle>(m_xLbTrgrGradientType->get_active()),
                Degree10(static_cast<sal_Int16>(m_xMtrTrgrAngle->get_value(FieldUnit::DEGREE)) * 10),
                static_cast<sal_uInt16>(m_xMtrTrgrCenterX->get_value(FieldUnit::PERCENT)),
                static_cast<sal_uInt16>(m_xMtrTrgrCenterY->get_value(FieldUnit::PERCENT)),
                static_cast<sal_uInt16>(m_xMtrTrgrBorder->get_value(FieldUnit::PERCENT)),
                100, 100);

    XFillFloatTransparenceItem aItem( aTmpGradient);
    rXFSet.Put ( aItem );

    InvalidatePreview();
}

void SvxTransparenceTabPage::ActivateGradient(bool bActivate)
{
    m_xGridGradient->set_sensitive(bActivate);

    if (bActivate)
    {
        css::awt::GradientStyle eXGS = static_cast<css::awt::GradientStyle>(m_xLbTrgrGradientType->get_active());
        SetControlState_Impl( eXGS );
    }
}

void SvxTransparenceTabPage::SetControlState_Impl(css::awt::GradientStyle eXGS)
{
    switch(eXGS)
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            m_xFtTrgrCenterX->set_sensitive(false);
            m_xMtrTrgrCenterX->set_sensitive(false);
            m_xFtTrgrCenterY->set_sensitive(false);
            m_xMtrTrgrCenterY->set_sensitive(false);
            m_xFtTrgrAngle->set_sensitive(true);
            m_xMtrTrgrAngle->set_sensitive(true);
            break;

        case css::awt::GradientStyle_RADIAL:
            m_xFtTrgrCenterX->set_sensitive(true);
            m_xMtrTrgrCenterX->set_sensitive(true);
            m_xFtTrgrCenterY->set_sensitive(true);
            m_xMtrTrgrCenterY->set_sensitive(true);
            m_xFtTrgrAngle->set_sensitive(false);
            m_xMtrTrgrAngle->set_sensitive(false);
            break;

        case css::awt::GradientStyle_ELLIPTICAL:
        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            m_xFtTrgrCenterX->set_sensitive(true);
            m_xMtrTrgrCenterX->set_sensitive(true);
            m_xFtTrgrCenterY->set_sensitive(true);
            m_xMtrTrgrCenterY->set_sensitive(true);
            m_xFtTrgrAngle->set_sensitive(true);
            m_xMtrTrgrAngle->set_sensitive(true);
            break;
        default:
            break;
    }
}

SvxTransparenceTabPage::SvxTransparenceTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "cui/ui/transparencytabpage.ui", "TransparencyTabPage", &rInAttrs)
    , rOutAttrs(rInAttrs)
    , nPageType(PageType::Area)
    , nDlgType(0)
    , bBitmap(false)
    , aXFillAttr(rInAttrs.GetPool())
    , rXFSet(aXFillAttr.GetItemSet())
    , m_xRbtTransOff(m_xBuilder->weld_radio_button("RBT_TRANS_OFF"))
    , m_xRbtTransLinear(m_xBuilder->weld_radio_button("RBT_TRANS_LINEAR"))
    , m_xRbtTransGradient(m_xBuilder->weld_radio_button("RBT_TRANS_GRADIENT"))
    , m_xMtrTransparent(m_xBuilder->weld_metric_spin_button("MTR_TRANSPARENT", FieldUnit::PERCENT))
    , m_xGridGradient(m_xBuilder->weld_widget("gridGradient"))
    , m_xLbTrgrGradientType(m_xBuilder->weld_combo_box("LB_TRGR_GRADIENT_TYPES"))
    , m_xFtTrgrCenterX(m_xBuilder->weld_label("FT_TRGR_CENTER_X"))
    , m_xMtrTrgrCenterX(m_xBuilder->weld_metric_spin_button("MTR_TRGR_CENTER_X", FieldUnit::PERCENT))
    , m_xFtTrgrCenterY(m_xBuilder->weld_label("FT_TRGR_CENTER_Y"))
    , m_xMtrTrgrCenterY(m_xBuilder->weld_metric_spin_button("MTR_TRGR_CENTER_Y", FieldUnit::PERCENT))
    , m_xFtTrgrAngle(m_xBuilder->weld_label("FT_TRGR_ANGLE"))
    , m_xMtrTrgrAngle(m_xBuilder->weld_metric_spin_button("MTR_TRGR_ANGLE", FieldUnit::DEGREE))
    , m_xMtrTrgrBorder(m_xBuilder->weld_metric_spin_button("MTR_TRGR_BORDER", FieldUnit::PERCENT))
    , m_xMtrTrgrStartValue(m_xBuilder->weld_metric_spin_button("MTR_TRGR_START_VALUE", FieldUnit::PERCENT))
    , m_xMtrTrgrEndValue(m_xBuilder->weld_metric_spin_button("MTR_TRGR_END_VALUE", FieldUnit::PERCENT))
    , m_xCtlBitmapBorder(m_xBuilder->weld_widget("bitmap_border"))
    , m_xCtlXRectBorder(m_xBuilder->weld_widget("trans_border"))
    , m_xCtlBitmapPreview(new weld::CustomWeld(*m_xBuilder, "CTL_BITMAP_PREVIEW", m_aCtlBitmapPreview))
    , m_xCtlXRectPreview(new weld::CustomWeld(*m_xBuilder, "CTL_TRANS_PREVIEW", m_aCtlXRectPreview))
{
    // main selection
    m_xRbtTransOff->connect_toggled(LINK(this, SvxTransparenceTabPage, ClickTransOffHdl_Impl));
    m_xRbtTransLinear->connect_toggled(LINK(this, SvxTransparenceTabPage, ClickTransLinearHdl_Impl));
    m_xRbtTransGradient->connect_toggled(LINK(this, SvxTransparenceTabPage, ClickTransGradientHdl_Impl));

    // linear transparency
    m_xMtrTransparent->set_value(50, FieldUnit::PERCENT);
    m_xMtrTransparent->connect_value_changed(LINK(this, SvxTransparenceTabPage, ModifyTransparentHdl_Impl));

    // gradient transparency
    m_xMtrTrgrEndValue->set_value(100, FieldUnit::PERCENT);
    m_xMtrTrgrStartValue->set_value(0, FieldUnit::PERCENT);
    Link<weld::MetricSpinButton&,void> aLink = LINK( this, SvxTransparenceTabPage, ModifiedTrgrEditHdl_Impl);
    m_xLbTrgrGradientType->connect_changed(LINK(this, SvxTransparenceTabPage, ModifiedTrgrListBoxHdl_Impl));
    m_xMtrTrgrCenterX->connect_value_changed( aLink );
    m_xMtrTrgrCenterY->connect_value_changed( aLink );
    m_xMtrTrgrAngle->connect_value_changed( aLink );
    m_xMtrTrgrBorder->connect_value_changed( aLink );
    m_xMtrTrgrStartValue->connect_value_changed( aLink );
    m_xMtrTrgrEndValue->connect_value_changed( aLink );

    // this page needs ExchangeSupport
    SetExchangeSupport();
}

std::unique_ptr<SfxTabPage> SvxTransparenceTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs)
{
    return std::make_unique<SvxTransparenceTabPage>(pPage, pController, *rAttrs);
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

    if (m_xMtrTransparent->get_sensitive())
    {
        // linear transparence
        sal_uInt16 nPos = m_xMtrTransparent->get_value(FieldUnit::PERCENT);
        if (m_xMtrTransparent->get_value_changed_from_saved() || !bLinearActive)
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
    else if (m_xGridGradient->get_sensitive())
    {
        // transparence gradient, fill ItemSet from values
        if (!bGradActive
            || m_xLbTrgrGradientType->get_value_changed_from_saved()
            || m_xMtrTrgrAngle->get_value_changed_from_saved()
            || m_xMtrTrgrCenterX->get_value_changed_from_saved()
            || m_xMtrTrgrCenterY->get_value_changed_from_saved()
            || m_xMtrTrgrBorder->get_value_changed_from_saved()
            || m_xMtrTrgrStartValue->get_value_changed_from_saved()
            || m_xMtrTrgrEndValue->get_value_changed_from_saved())
        {
            sal_uInt8 nStartCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_xMtrTrgrStartValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
            sal_uInt8 nEndCol = static_cast<sal_uInt8>((static_cast<sal_uInt16>(m_xMtrTrgrEndValue->get_value(FieldUnit::PERCENT)) * 255) / 100);
            XGradient aTmpGradient(
                        Color(nStartCol, nStartCol, nStartCol),
                        Color(nEndCol, nEndCol, nEndCol),
                        static_cast<css::awt::GradientStyle>(m_xLbTrgrGradientType->get_active()),
                        Degree10(static_cast<sal_Int16>(m_xMtrTrgrAngle->get_value(FieldUnit::DEGREE)) * 10),
                        static_cast<sal_uInt16>(m_xMtrTrgrCenterX->get_value(FieldUnit::PERCENT)),
                        static_cast<sal_uInt16>(m_xMtrTrgrCenterY->get_value(FieldUnit::PERCENT)),
                        static_cast<sal_uInt16>(m_xMtrTrgrBorder->get_value(FieldUnit::PERCENT)),
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
        XGradient aGrad(COL_BLACK, COL_WHITE);
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
    rAttrs->Put(CntUInt16Item(SID_PAGE_TYPE, static_cast<sal_uInt16>(nPageType)));
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
    m_xLbTrgrGradientType->set_active(sal::static_int_cast< sal_Int32 >(eXGS));
    m_xMtrTrgrAngle->set_value(rGradient.GetAngle().get() / 10, FieldUnit::DEGREE);
    m_xMtrTrgrBorder->set_value(rGradient.GetBorder(), FieldUnit::PERCENT);
    m_xMtrTrgrCenterX->set_value(rGradient.GetXOffset(), FieldUnit::PERCENT);
    m_xMtrTrgrCenterY->set_value(rGradient.GetYOffset(), FieldUnit::PERCENT);
    m_xMtrTrgrStartValue->set_value(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(rGradient.GetStartColor().GetRed()) + 1) * 100) / 255), FieldUnit::PERCENT);
    m_xMtrTrgrEndValue->set_value(static_cast<sal_uInt16>(((static_cast<sal_uInt16>(rGradient.GetEndColor().GetRed()) + 1) * 100) / 255), FieldUnit::PERCENT);

    // linear transparence
    sal_uInt16 nTransp = static_cast<const XFillTransparenceItem*>(pLinearItem)->GetValue();
    m_xMtrTransparent->set_value(bLinearActive ? nTransp : 50, FieldUnit::PERCENT);
    ModifyTransparentHdl_Impl(*m_xMtrTransparent);

    // select the correct radio button
    if(bGradActive)
    {
        // transparence gradient, set controls appropriate to item
        m_xRbtTransGradient->set_active(true);
        ClickTransGradientHdl_Impl(*m_xRbtTransGradient);
    }
    else if(bLinearActive)
    {
        // linear transparence
        m_xRbtTransLinear->set_active(true);
        ClickTransLinearHdl_Impl(*m_xRbtTransLinear);
    }
    else
    {
        // no transparence
        m_xRbtTransOff->set_active(true);
        ClickTransOffHdl_Impl(*m_xRbtTransOff);
        ModifiedTrgrHdl_Impl(nullptr);
    }

    // save values
    ChangesApplied();
    bool bActive = InitPreview ( *rAttrs );
    InvalidatePreview ( bActive );
}

void SvxTransparenceTabPage::ChangesApplied()
{
    m_xMtrTransparent->save_value();
    m_xLbTrgrGradientType->save_value();
    m_xMtrTrgrCenterX->save_value();
    m_xMtrTrgrCenterY->save_value();
    m_xMtrTrgrAngle->save_value();
    m_xMtrTrgrBorder->save_value();
    m_xMtrTrgrStartValue->save_value();
    m_xMtrTrgrEndValue->save_value();
}

void SvxTransparenceTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const CntUInt16Item* pPageTypeItem = rSet.GetItem<CntUInt16Item>(SID_PAGE_TYPE, false);
    if (pPageTypeItem)
        SetPageType(static_cast<PageType>(pPageTypeItem->GetValue()));

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

// Preview-Methods

bool SvxTransparenceTabPage::InitPreview(const SfxItemSet& rSet)
{
    // set transparencetype for preview
    if (m_xRbtTransOff->get_active())
    {
        ClickTransOffHdl_Impl(*m_xRbtTransOff);
    }
    else if (m_xRbtTransLinear->get_active())
    {
        ClickTransLinearHdl_Impl(*m_xRbtTransLinear);
    }
    else if (m_xRbtTransGradient->get_active())
    {
        ClickTransGradientHdl_Impl(*m_xRbtTransGradient);
    }

    // Get fillstyle for preview
    rXFSet.Put ( rSet.Get(XATTR_FILLSTYLE) );
    rXFSet.Put ( rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( rSet.Get(XATTR_FILLGRADIENT) );
    rXFSet.Put ( rSet.Get(XATTR_FILLHATCH) );
    rXFSet.Put ( rSet.Get(XATTR_FILLBACKGROUND) );
    rXFSet.Put ( rSet.Get(XATTR_FILLBITMAP) );

    m_aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
    m_aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );

    bBitmap = rSet.Get(XATTR_FILLSTYLE).GetValue() == drawing::FillStyle_BITMAP;

    // show the right preview window
    if ( bBitmap )
    {
        m_xCtlBitmapBorder->show();
        m_xCtlXRectBorder->hide();
    }
    else
    {
        m_xCtlBitmapBorder->hide();
        m_xCtlXRectBorder->show();
    }

    return !m_xRbtTransOff->get_active();
}

void SvxTransparenceTabPage::InvalidatePreview (bool bEnable)
{
    if ( bBitmap )
    {
        if ( bEnable )
        {
            m_xCtlBitmapPreview->set_sensitive(true);
            m_aCtlBitmapPreview.SetAttributes( aXFillAttr.GetItemSet() );
        }
        else
            m_xCtlBitmapPreview->set_sensitive(false);
        m_xCtlBitmapPreview->queue_draw();
    }
    else
    {
        if ( bEnable )
        {
            m_xCtlXRectPreview->set_sensitive(true);
            m_aCtlXRectPreview.SetAttributes( aXFillAttr.GetItemSet() );
        }
        else
            m_xCtlXRectPreview->set_sensitive(false);
        m_xCtlXRectPreview->queue_draw();
    }
}

void SvxTransparenceTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pPageTypeItem = aSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pDlgTypeItem = aSet.GetItem<SfxUInt16Item>(SID_DLG_TYPE, false);

    if (pPageTypeItem)
        SetPageType(static_cast<PageType>(pPageTypeItem->GetValue()));
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
