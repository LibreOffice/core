/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "TextColumnsPropertyPanel.hxx"

#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/itempool.hxx>
#include <svtools/unitconv.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpool.hxx>
#include <svl/eitem.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svdoattr.hxx>
#include <tools/fldunit.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace
{
MapUnit GetUnit(const SfxBindings* pBindings, sal_uInt16 nWhich)
{
    assert(pBindings);

    SfxObjectShell* pSh = nullptr;
    if (auto pShell = pBindings->GetDispatcher()->GetShell(0))
        pSh = pShell->GetObjectShell();
    if (!pSh)
        pSh = SfxObjectShell::Current();
    SfxItemPool& rPool = pSh ? pSh->GetPool() : SfxGetpApp()->GetPool();
    return rPool.GetMetric(nWhich);
}
}

namespace svx::sidebar
{
TextColumnsPropertyPanel::TextColumnsPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, u"TextColumnsPropertyPanel"_ustr,
                  u"svx/ui/sidebartextcolumnspanel.ui"_ustr)
    , mpBindings(pBindings)
    , m_xColumnsNumber(m_xBuilder->weld_spin_button(u"FLD_COL_NUMBER"_ustr))
    , m_xColumnsSpacing(
          m_xBuilder->weld_metric_spin_button(u"MTR_FLD_COL_SPACING"_ustr, FieldUnit::CM))
    , maColumnsNumberController(SID_ATTR_TEXTCOLUMNS_NUMBER, *pBindings, *this)
    , maColumnsSpacingController(SID_ATTR_TEXTCOLUMNS_SPACING, *pBindings, *this)
    , m_xFitWidth(m_xBuilder->weld_check_button(u"CBX_FIT_WIDTH"_ustr))
    , m_xFitHeight(m_xBuilder->weld_check_button(u"CBX_FIT_HEIGHT"_ustr))
    , m_xFitFrame(m_xBuilder->weld_check_button(u"CBX_FIT_FRAME"_ustr))
    , maFitController(SID_ATTR_TEXT_FITTOSIZE, *pBindings, *this)
    , maFitWidthController(SID_ATTR_TEXT_AUTOGROWWIDTH, *pBindings, *this)
    , maFitHeightController(SID_ATTR_TEXT_AUTOGROWHEIGHT, *pBindings, *this)
    , m_xPaddingLeft(m_xBuilder->weld_metric_spin_button(u"MTR_PADDING_LEFT"_ustr, FieldUnit::CM))
    , m_xPaddingRight(m_xBuilder->weld_metric_spin_button(u"MTR_PADDING_RIGHT"_ustr, FieldUnit::CM))
    , m_xPaddingTop(m_xBuilder->weld_metric_spin_button(u"MTR_PADDING_TOP"_ustr, FieldUnit::CM))
    , m_xPaddingBottom(
          m_xBuilder->weld_metric_spin_button(u"MTR_PADDING_BOTTOM"_ustr, FieldUnit::CM))
    , maLeftDistController(SID_ATTR_TEXT_LEFTDIST, *pBindings, *this)
    , maRightDistController(SID_ATTR_TEXT_RIGHTDIST, *pBindings, *this)
    , maUpperDistController(SID_ATTR_TEXT_UPPERDIST, *pBindings, *this)
    , maLowerDistController(SID_ATTR_TEXT_LOWERDIST, *pBindings, *this)

{
    m_xColumnsNumber->connect_value_changed(
        LINK(this, TextColumnsPropertyPanel, ModifyColumnsNumberHdl));
    m_xColumnsSpacing->connect_value_changed(
        LINK(this, TextColumnsPropertyPanel, ModifyColumnsSpacingHdl));
    m_xFitWidth->connect_toggled(LINK(this, TextColumnsPropertyPanel, ClickFitOptionHdl));
    m_xFitHeight->connect_toggled(LINK(this, TextColumnsPropertyPanel, ClickFitOptionHdl));
    m_xFitFrame->connect_toggled(LINK(this, TextColumnsPropertyPanel, ClickFitOptionHdl));
    m_xPaddingLeft->connect_value_changed(LINK(this, TextColumnsPropertyPanel, ModifyPaddingHdl));
    m_xPaddingRight->connect_value_changed(LINK(this, TextColumnsPropertyPanel, ModifyPaddingHdl));
    m_xPaddingTop->connect_value_changed(LINK(this, TextColumnsPropertyPanel, ModifyPaddingHdl));
    m_xPaddingBottom->connect_value_changed(LINK(this, TextColumnsPropertyPanel, ModifyPaddingHdl));
}

TextColumnsPropertyPanel::~TextColumnsPropertyPanel()
{
    maColumnsSpacingController.dispose();
    maColumnsNumberController.dispose();

    m_xColumnsSpacing.reset();
    m_xColumnsNumber.reset();

    m_xFitWidth.reset();
    m_xFitHeight.reset();
    m_xFitFrame.reset();

    maFitController.dispose();
    maFitWidthController.dispose();
    maFitHeightController.dispose();

    maLeftDistController.dispose();
    maRightDistController.dispose();
    maUpperDistController.dispose();
    maLowerDistController.dispose();
}

IMPL_LINK_NOARG(TextColumnsPropertyPanel, ModifyColumnsNumberHdl, weld::SpinButton&, void)
{
    m_xColumnsSpacing->set_sensitive(m_xColumnsNumber->get_value() > 1);
    SfxInt16Item aItem(SDRATTR_TEXTCOLUMNS_NUMBER, m_xColumnsNumber->get_value());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_TEXTCOLUMNS_NUMBER, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK_NOARG(TextColumnsPropertyPanel, ModifyColumnsSpacingHdl, weld::MetricSpinButton&, void)
{
    const MapUnit aUnit = GetUnit(mpBindings, SDRATTR_TEXTCOLUMNS_SPACING);
    SdrMetricItem aItem(SDRATTR_TEXTCOLUMNS_SPACING, GetCoreValue(*m_xColumnsSpacing, aUnit));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_TEXTCOLUMNS_SPACING, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK(TextColumnsPropertyPanel, ClickFitOptionHdl, weld::Toggleable&, rButton, void)
{
    const bool bFitWidth = m_xFitWidth->get_active();
    const bool bFitHeight = m_xFitHeight->get_active();
    const bool bFitFrame = m_xFitFrame->get_active();

    if (&rButton == m_xFitFrame.get())
    {
        css::drawing::TextFitToSizeType eType = bFitFrame
                                                    ? css::drawing::TextFitToSizeType_PROPORTIONAL
                                                    : css::drawing::TextFitToSizeType_NONE;
        SdrTextFitToSizeTypeItem aItem(eType);
        mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_TEXT_FITTOSIZE, SfxCallMode::RECORD,
                                                 { &aItem });

        if (bFitFrame && m_xFitFrame->get_sensitive())
        {
            // Disable Fit Width / Fit Height when Fit to Frame is On
            m_xFitWidth->set_active(false);
            m_xFitHeight->set_active(false);
            m_xFitWidth->set_sensitive(false);
            m_xFitHeight->set_sensitive(false);
        }
        else
        {
            // Re-enable both options when Fit to Frame is Off
            m_xFitWidth->set_sensitive(true);
            m_xFitHeight->set_sensitive(true);
        }
    }
    else if (&rButton == m_xFitWidth.get() || &rButton == m_xFitHeight.get())
    {
        if (&rButton == m_xFitWidth.get())
        {
            SfxBoolItem aItem(SDRATTR_TEXT_AUTOGROWWIDTH, bFitWidth);
            mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_TEXT_AUTOGROWWIDTH,
                                                     SfxCallMode::RECORD, { &aItem });
        }
        else
        {
            SfxBoolItem aItem(SDRATTR_TEXT_AUTOGROWHEIGHT, bFitHeight);
            mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_TEXT_AUTOGROWHEIGHT,
                                                     SfxCallMode::RECORD, { &aItem });
        }

        if (bFitWidth || bFitHeight)
        {
            m_xFitFrame->set_active(false);
            m_xFitFrame->set_sensitive(false);
        }
        else
        {
            m_xFitFrame->set_sensitive(true);
        }
    }
}

IMPL_LINK(TextColumnsPropertyPanel, ModifyPaddingHdl, weld::MetricSpinButton&, rCtrl, void)
{
    const MapUnit aUnit = GetUnit(mpBindings, SDRATTR_TEXT_LEFTDIST);
    sal_uInt16 nSID = 0;
    sal_uInt16 nWhich = 0;

    if (&rCtrl == m_xPaddingLeft.get())
    {
        nSID = SID_ATTR_TEXT_LEFTDIST;
        nWhich = SDRATTR_TEXT_LEFTDIST;
    }
    else if (&rCtrl == m_xPaddingRight.get())
    {
        nSID = SID_ATTR_TEXT_RIGHTDIST;
        nWhich = SDRATTR_TEXT_RIGHTDIST;
    }
    else if (&rCtrl == m_xPaddingTop.get())
    {
        nSID = SID_ATTR_TEXT_UPPERDIST;
        nWhich = SDRATTR_TEXT_UPPERDIST;
    }
    else if (&rCtrl == m_xPaddingBottom.get())
    {
        nSID = SID_ATTR_TEXT_LOWERDIST;
        nWhich = SDRATTR_TEXT_LOWERDIST;
    }

    if (nSID && nWhich)
    {
        SdrMetricItem aItem(TypedWhichId<SdrMetricItem>(nWhich), GetCoreValue(rCtrl, aUnit));
        mpBindings->GetDispatcher()->ExecuteList(nSID, SfxCallMode::RECORD, { &aItem });
    }
}

void TextColumnsPropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
                                                const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_ATTR_TEXTCOLUMNS_NUMBER:
            if (eState >= SfxItemState::DEFAULT)
            {
                if (const auto pItem = dynamic_cast<const SfxInt16Item*>(pState))
                {
                    m_xColumnsNumber->set_value(pItem->GetValue());
                    m_xColumnsSpacing->set_sensitive(pItem->GetValue() > 1);
                }
            }
            break;
        case SID_ATTR_TEXTCOLUMNS_SPACING:
            if (eState >= SfxItemState::DEFAULT)
            {
                const MapUnit aUnit = GetUnit(mpBindings, SDRATTR_TEXTCOLUMNS_SPACING);
                if (const auto pItem = dynamic_cast<const SdrMetricItem*>(pState))
                    SetMetricValue(*m_xColumnsSpacing, pItem->GetValue(), aUnit);
            }
            break;
        case SID_ATTR_TEXT_FITTOSIZE:
            if (eState >= SfxItemState::DEFAULT)
            {
                if (const auto pFit = dynamic_cast<const SdrTextFitToSizeTypeItem*>(pState))
                {
                    bool bFrame
                        = (pFit->GetValue() == css::drawing::TextFitToSizeType_PROPORTIONAL);
                    m_xFitFrame->set_active(bFrame);
                    if (bFrame)
                    {
                        m_xFitWidth->set_active(false);
                        m_xFitHeight->set_active(false);
                    }
                }
            }
            break;

        case SID_ATTR_TEXT_AUTOGROWWIDTH:
            if (eState >= SfxItemState::DEFAULT)
                if (const auto pBool = dynamic_cast<const SfxBoolItem*>(pState))
                    m_xFitWidth->set_active(pBool->GetValue());
            break;

        case SID_ATTR_TEXT_AUTOGROWHEIGHT:
            if (eState >= SfxItemState::DEFAULT)
                if (const auto pBool = dynamic_cast<const SfxBoolItem*>(pState))
                    m_xFitHeight->set_active(pBool->GetValue());
            break;
        case SID_ATTR_TEXT_LEFTDIST:
            if (eState >= SfxItemState::DEFAULT)
                if (const auto pItem = dynamic_cast<const SdrMetricItem*>(pState))
                    SetMetricValue(*m_xPaddingLeft, pItem->GetValue(), GetUnit(mpBindings, nSID));
            break;
        case SID_ATTR_TEXT_RIGHTDIST:
            if (eState >= SfxItemState::DEFAULT)
                if (const auto pItem = dynamic_cast<const SdrMetricItem*>(pState))
                    SetMetricValue(*m_xPaddingRight, pItem->GetValue(), GetUnit(mpBindings, nSID));
            break;
        case SID_ATTR_TEXT_UPPERDIST:
            if (eState >= SfxItemState::DEFAULT)
                if (const auto pItem = dynamic_cast<const SdrMetricItem*>(pState))
                    SetMetricValue(*m_xPaddingTop, pItem->GetValue(), GetUnit(mpBindings, nSID));
            break;
        case SID_ATTR_TEXT_LOWERDIST:
            if (eState >= SfxItemState::DEFAULT)
                if (const auto pItem = dynamic_cast<const SdrMetricItem*>(pState))
                    SetMetricValue(*m_xPaddingBottom, pItem->GetValue(), GetUnit(mpBindings, nSID));
            break;
    }
    UpdateFitControls();
}

std::unique_ptr<PanelLayout> TextColumnsPropertyPanel::Create(weld::Widget* pParent,
                                                              SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to TextColumnsPropertyPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no SfxBindings given to TextColumnsPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<TextColumnsPropertyPanel>(pParent, pBindings);
}

void TextColumnsPropertyPanel::UpdateFitControls()
{
    const bool bFitFrame = m_xFitFrame->get_active();
    const bool bFitWidth = m_xFitWidth->get_active();
    const bool bFitHeight = m_xFitHeight->get_active();

    // fit frame disable width/height
    m_xFitWidth->set_sensitive(!bFitFrame);
    m_xFitHeight->set_sensitive(!bFitFrame);

    // width/height disable Fit Frame
    const bool bDisableFrame = (bFitWidth || bFitHeight);
    m_xFitFrame->set_sensitive(!bDisableFrame);

    if (bDisableFrame)
        m_xFitFrame->set_active(false);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
