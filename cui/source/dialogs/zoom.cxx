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

#include <osl/diagnose.h>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <sfx2/objsh.hxx>
#include <zoom.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/viewlayoutitem.hxx>
#include <svx/zoom_def.hxx>

namespace
{
const sal_uInt16 SPECIAL_FACTOR = 0xFFFF;

} // anonymous namespace

sal_uInt16 SvxZoomDialog::GetFactor() const
{
    if (m_x100Btn->get_active())
        return 100;

    if (m_xUserBtn->get_active())
        return static_cast<sal_uInt16>(m_xUserEdit->get_value(FieldUnit::PERCENT));
    else
        return SPECIAL_FACTOR;
}

void SvxZoomDialog::SetFactor(sal_uInt16 nNewFactor, ZoomButtonId nButtonId)
{
    m_xUserEdit->set_sensitive(false);

    if (nButtonId == ZoomButtonId::NONE)
    {
        if (nNewFactor == 100)
        {
            m_x100Btn->set_active(true);
            m_x100Btn->grab_focus();
        }
        else
        {
            m_xUserBtn->set_active(true);
            m_xUserEdit->set_sensitive(true);
            m_xUserEdit->set_value(nNewFactor, FieldUnit::PERCENT);
            m_xUserEdit->grab_focus();
        }
    }
    else
    {
        m_xUserEdit->set_value(nNewFactor, FieldUnit::PERCENT);
        switch (nButtonId)
        {
            case ZoomButtonId::OPTIMAL:
            {
                m_xOptimalBtn->set_active(true);
                m_xOptimalBtn->grab_focus();
                break;
            }
            case ZoomButtonId::PAGEWIDTH:
            {
                m_xPageWidthBtn->set_active(true);
                m_xPageWidthBtn->grab_focus();
                break;
            }
            case ZoomButtonId::WHOLEPAGE:
            {
                m_xWholePageBtn->set_active(true);
                m_xWholePageBtn->grab_focus();
                break;
            }
            default:
                break;
        }
    }
}

void SvxZoomDialog::HideButton(ZoomButtonId nButtonId)
{
    switch (nButtonId)
    {
        case ZoomButtonId::OPTIMAL:
            m_xOptimalBtn->hide();
            break;

        case ZoomButtonId::PAGEWIDTH:
            m_xPageWidthBtn->hide();
            break;

        case ZoomButtonId::WHOLEPAGE:
            m_xWholePageBtn->hide();
            break;

        default:
            OSL_FAIL("Wrong button number!");
    }
}

void SvxZoomDialog::SetLimits(sal_uInt16 nMin, sal_uInt16 nMax)
{
    DBG_ASSERT(nMin < nMax, "invalid limits");
    m_xUserEdit->set_range(nMin, nMax, FieldUnit::PERCENT);
}

const SfxItemSet* SvxZoomDialog::GetOutputItemSet() const { return m_pOutSet.get(); }

SvxZoomDialog::SvxZoomDialog(weld::Window* pParent, const SfxItemSet& rCoreSet)
    : SfxDialogController(pParent, u"cui/ui/zoomdialog.ui"_ustr, u"ZoomDialog"_ustr)
    , m_rSet(rCoreSet)
    , m_bModified(false)
    , m_xOptimalBtn(m_xBuilder->weld_radio_button(u"optimal"_ustr))
    , m_xWholePageBtn(m_xBuilder->weld_radio_button(u"fitwandh"_ustr))
    , m_xPageWidthBtn(m_xBuilder->weld_radio_button(u"fitw"_ustr))
    , m_x100Btn(m_xBuilder->weld_radio_button(u"100pc"_ustr))
    , m_xUserBtn(m_xBuilder->weld_radio_button(u"variable"_ustr))
    , m_xUserEdit(m_xBuilder->weld_metric_spin_button(u"zoomsb"_ustr, FieldUnit::PERCENT))
    , m_xViewFrame(m_xBuilder->weld_widget(u"viewframe"_ustr))
    , m_xAutomaticBtn(m_xBuilder->weld_radio_button(u"automatic"_ustr))
    , m_xSingleBtn(m_xBuilder->weld_radio_button(u"singlepage"_ustr))
    , m_xColumnsBtn(m_xBuilder->weld_radio_button(u"columns"_ustr))
    , m_xColumnsEdit(m_xBuilder->weld_spin_button(u"columnssb"_ustr))
    , m_xBookModeChk(m_xBuilder->weld_check_button(u"bookmode"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
{
    Link<weld::Toggleable&, void> aLink = LINK(this, SvxZoomDialog, UserHdl);
    m_x100Btn->connect_toggled(aLink);
    m_xOptimalBtn->connect_toggled(aLink);
    m_xPageWidthBtn->connect_toggled(aLink);
    m_xWholePageBtn->connect_toggled(aLink);
    m_xUserBtn->connect_toggled(aLink);

    Link<weld::Toggleable&, void> aViewLayoutLink = LINK(this, SvxZoomDialog, ViewLayoutUserHdl);
    m_xAutomaticBtn->connect_toggled(aViewLayoutLink);
    m_xSingleBtn->connect_toggled(aViewLayoutLink);
    m_xColumnsBtn->connect_toggled(aViewLayoutLink);

    Link<weld::SpinButton&, void> aViewLayoutSpinLink
        = LINK(this, SvxZoomDialog, ViewLayoutSpinHdl);
    m_xColumnsEdit->connect_value_changed(aViewLayoutSpinLink);

    Link<weld::Toggleable&, void> aViewLayoutCheckLink
        = LINK(this, SvxZoomDialog, ViewLayoutCheckHdl);
    m_xBookModeChk->connect_toggled(aViewLayoutCheckLink);

    m_xOKBtn->connect_clicked(LINK(this, SvxZoomDialog, OKHdl));
    m_xUserEdit->connect_value_changed(LINK(this, SvxZoomDialog, SpinHdl));

    // default values
    sal_uInt16 nValue = 100;
    sal_uInt16 nMin = 10;
    sal_uInt16 nMax = 1000;

    // maybe get the old value first
    const SfxUInt16Item* pOldUserItem = nullptr;
    if (SfxObjectShell* pShell = SfxObjectShell::Current())
        pOldUserItem = pShell->GetItem(SID_ATTR_ZOOM_USER);

    if (pOldUserItem)
        nValue = pOldUserItem->GetValue();

    // initialize UserEdit
    if (nMin > nValue)
        nMin = nValue;
    if (nMax < nValue)
        nMax = nValue;

    SetLimits(nMin, nMax);
    m_xUserEdit->set_value(nValue, FieldUnit::PERCENT);

    const SfxPoolItem& rItem = m_rSet.Get(SID_ATTR_ZOOM);

    if (auto pZoomItem = dynamic_cast<const SvxZoomItem*>(&rItem))
    {
        const sal_uInt16 nZoom = pZoomItem->GetValue();
        const SvxZoomType eType = pZoomItem->GetType();
        const SvxZoomEnableFlags nValSet = pZoomItem->GetValueSet();
        ZoomButtonId nButtonId = ZoomButtonId::NONE;

        switch (eType)
        {
            case SvxZoomType::OPTIMAL:
                nButtonId = ZoomButtonId::OPTIMAL;
                break;
            case SvxZoomType::PAGEWIDTH:
                nButtonId = ZoomButtonId::PAGEWIDTH;
                break;
            case SvxZoomType::WHOLEPAGE:
                nButtonId = ZoomButtonId::WHOLEPAGE;
                break;
            case SvxZoomType::PERCENT:
                break;
            case SvxZoomType::PAGEWIDTH_NOBORDER:
                break;
        }

        if (!(SvxZoomEnableFlags::N100 & nValSet))
            m_x100Btn->set_sensitive(false);
        if (!(SvxZoomEnableFlags::OPTIMAL & nValSet))
            m_xOptimalBtn->set_sensitive(false);
        if (!(SvxZoomEnableFlags::PAGEWIDTH & nValSet))
            m_xPageWidthBtn->set_sensitive(false);
        if (!(SvxZoomEnableFlags::WHOLEPAGE & nValSet))
            m_xWholePageBtn->set_sensitive(false);

        SetFactor(nZoom, nButtonId);
    }
    else
    {
        const sal_uInt16 nZoom = static_cast<const SfxUInt16Item&>(rItem).GetValue();
        SetFactor(nZoom);
    }

    if (const SvxViewLayoutItem* pViewLayoutItem = m_rSet.GetItemIfSet(SID_ATTR_VIEWLAYOUT, false))
    {
        const sal_uInt16 nColumns = pViewLayoutItem->GetValue();
        const bool bBookMode = pViewLayoutItem->IsBookMode();

        if (0 == nColumns)
        {
            m_xAutomaticBtn->set_active(true);
            m_xColumnsEdit->set_value(2);
            m_xColumnsEdit->set_sensitive(false);
            m_xBookModeChk->set_sensitive(false);
        }
        else if (1 == nColumns)
        {
            m_xSingleBtn->set_active(true);
            m_xColumnsEdit->set_value(2);
            m_xColumnsEdit->set_sensitive(false);
            m_xBookModeChk->set_sensitive(false);
        }
        else
        {
            m_xColumnsBtn->set_active(true);
            if (!bBookMode)
            {
                m_xColumnsEdit->set_value(nColumns);
                if (nColumns % 2 != 0)
                    m_xBookModeChk->set_sensitive(false);
            }
            else
            {
                m_xColumnsEdit->set_value(nColumns);
                m_xBookModeChk->set_active(true);
            }
        }
    }
    else
    {
        // hide view layout related controls:
        m_xViewFrame->set_visible(false);
    }
}

IMPL_LINK_NOARG(SvxZoomDialog, UserHdl, weld::Toggleable&, void)
{
    m_bModified = true;

    if (m_xUserBtn->get_active())
    {
        m_xUserEdit->set_sensitive(true);
        m_xUserEdit->grab_focus();
    }
    else
    {
        m_xUserEdit->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxZoomDialog, SpinHdl, weld::MetricSpinButton&, void)
{
    if (!m_xUserBtn->get_active())
        return;

    m_bModified = true;
}

IMPL_LINK_NOARG(SvxZoomDialog, ViewLayoutUserHdl, weld::Toggleable&, void)
{
    m_bModified = true;

    if (m_xAutomaticBtn->get_active() || m_xSingleBtn->get_active())
    {
        m_xColumnsEdit->set_sensitive(false);
        m_xBookModeChk->set_sensitive(false);
    }
    else if (m_xColumnsBtn->get_active())
    {
        m_xColumnsEdit->set_sensitive(true);
        m_xColumnsEdit->grab_focus();
        if (m_xColumnsEdit->get_value() % 2 == 0)
            m_xBookModeChk->set_sensitive(true);
    }
}

IMPL_LINK_NOARG(SvxZoomDialog, ViewLayoutSpinHdl, weld::SpinButton&, void)
{
    if (!m_xColumnsBtn->get_active())
        return;

    if (m_xColumnsEdit->get_value() % 2 == 0)
    {
        m_xBookModeChk->set_sensitive(true);
    }
    else
    {
        m_xBookModeChk->set_active(false);
        m_xBookModeChk->set_sensitive(false);
    }

    m_bModified = true;
}

IMPL_LINK_NOARG(SvxZoomDialog, ViewLayoutCheckHdl, weld::Toggleable&, void)
{
    if (!m_xColumnsBtn->get_active())
        return;

    m_bModified = true;
}

IMPL_LINK_NOARG(SvxZoomDialog, OKHdl, weld::Button&, void)
{
    if (m_bModified)
    {
        SvxZoomItem aZoomItem(SvxZoomType::PERCENT, 0, SID_ATTR_ZOOM);
        SvxViewLayoutItem aViewLayoutItem(0, false, SID_ATTR_VIEWLAYOUT);

        sal_uInt16 nFactor = GetFactor();

        if (SPECIAL_FACTOR == nFactor)
        {
            if (m_xOptimalBtn->get_active())
                aZoomItem.SetType(SvxZoomType::OPTIMAL);
            else if (m_xPageWidthBtn->get_active())
                aZoomItem.SetType(SvxZoomType::PAGEWIDTH);
            else if (m_xWholePageBtn->get_active())
                aZoomItem.SetType(SvxZoomType::WHOLEPAGE);
        }
        else
        {
            aZoomItem.SetValue(nFactor);
        }

        if (m_xAutomaticBtn->get_active())
        {
            aViewLayoutItem.SetValue(0);
            aViewLayoutItem.SetBookMode(false);
        }
        if (m_xSingleBtn->get_active())
        {
            aViewLayoutItem.SetValue(1);
            aViewLayoutItem.SetBookMode(false);
        }
        else if (m_xColumnsBtn->get_active())
        {
            aViewLayoutItem.SetValue(static_cast<sal_uInt16>(m_xColumnsEdit->get_value()));
            aViewLayoutItem.SetBookMode(m_xBookModeChk->get_active());
        }

        m_pOutSet.reset(new SfxItemSet(m_rSet));
        m_pOutSet->Put(aZoomItem);

        // don't set attribute in case the whole viewlayout stuff is disabled:
        if (m_xViewFrame->get_sensitive())
            m_pOutSet->Put(aViewLayoutItem);

        // memorize value from the UserEdit beyond the dialog
        if (SfxObjectShell* pShell = SfxObjectShell::Current())
        {
            sal_uInt16 nZoomValue
                = static_cast<sal_uInt16>(m_xUserEdit->get_value(FieldUnit::PERCENT));
            pShell->PutItem(SfxUInt16Item(SID_ATTR_ZOOM_USER, nZoomValue));
        }
        m_xDialog->response(RET_OK);
    }
    else
        m_xDialog->response(RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
