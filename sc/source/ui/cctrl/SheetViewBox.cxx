/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SheetViewBox.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/weld/Builder.hxx>

#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <docsh.hxx>
#include <viewdata.hxx>

#include <SheetViewManager.hxx>
#include <SheetView.hxx>
#include <SheetViewTypes.hxx>

SheetViewBox::SheetViewBox(vcl::Window* pParent)
    : InterimItemWindow(pParent, u"modules/scalc/ui/sheetviewbox.ui"_ustr, u"SheetViewBox"_ustr,
                        true, reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
    , m_xWidget(m_xBuilder->weld_combo_box(u"sheetview_combobox"_ustr))
{
    m_xWidget->connect_changed(LINK(this, SheetViewBox, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, SheetViewBox, KeyInputHdl));
    SetSizePixel(m_xWidget->get_preferred_size());
}

void SheetViewBox::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

SheetViewBox::~SheetViewBox() { disposeOnce(); }

void SheetViewBox::GetFocus()
{
    if (m_xWidget)
        m_xWidget->grab_focus();
    InterimItemWindow::GetFocus();
}

void SheetViewBox::Update(sc::SheetViewID nSelectedID)
{
    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return;

    m_xWidget->clear();
    m_xWidget->freeze();

    OUString sActiveID = OUString::number(sc::DefaultSheetViewID);
    m_xWidget->append(sActiveID, sc::SheetViewManager::defaultViewName());

    auto pSheetManager = pViewData->GetCurrentSheetViewManager();

    if (pSheetManager)
    {
        sc::SheetViewID nSheetViewID = 0;
        for (auto const& pSheetView : pSheetManager->getSheetViews())
        {
            if (pSheetView)
            {
                OUString sID = OUString::number(nSheetViewID);
                if (nSheetViewID == nSelectedID)
                    sActiveID = sID;
                m_xWidget->append(sID, pSheetView->GetName());
            }
            nSheetViewID++;
        }
    }
    m_xWidget->thaw();

    m_xWidget->set_active_id(sActiveID);
}

IMPL_STATIC_LINK(SheetViewBox, SelectHdl, weld::ComboBox&, rComboBox, void)
{
    auto* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    SfxDispatcher* pDispatcher = pViewFrame->GetBindings().GetDispatcher();
    if (!pDispatcher)
        return;

    const OUString sValue = rComboBox.get_active_id();
    SfxInt32Item aItem(FID_CURRENT_SHEET_VIEW, sValue.toInt32());
    pDispatcher->ExecuteList(FID_CURRENT_SHEET_VIEW, SfxCallMode::RECORD, { &aItem });

    pViewFrame->GetWindow().GrabFocus();
}

IMPL_LINK(SheetViewBox, KeyInputHdl, const KeyEvent&, rKEvt, bool) { return ChildKeyInput(rKEvt); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
