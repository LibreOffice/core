/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <memory>
#include <sal/config.h>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <reffact.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <dbtableresizedlg.hxx>
#include <dbdocfun.hxx>

ScDbTableResizeDlg::ScDbTableResizeDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                                       ScViewData& rViewData)
    : ScAnyRefDlgController(pB, pCW, pParent, u"modules/scalc/ui/resizetablerangedialog.ui"_ustr,
                            u"tableresizedialog"_ustr)
    , m_rViewData(rViewData)
    , rDoc(rViewData.GetDocument())
    , bRefInputMode(true)
    , aAddrDetails(rDoc.GetAddressConvention(), 0, 0)
    , m_xAssignFrame(m_xBuilder->weld_frame(u"rangeframe"_ustr))
    , m_xEdAssign(new formula::RefEdit(m_xBuilder->weld_entry(u"assign"_ustr)))
    , m_xRbAssign(new formula::RefButton(m_xBuilder->weld_button(u"assgnrb"_ustr)))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
{
    m_xEdAssign->SetReferences(this, m_xAssignFrame.get());
    m_xRbAssign->SetReferences(this, m_xEdAssign.get());

    Init();
}

ScDbTableResizeDlg::~ScDbTableResizeDlg()
{
    if (m_xInfoBox)
        m_xInfoBox->response(RET_OK);
}

void ScDbTableResizeDlg::Init()
{
    m_xBtnOk->connect_clicked(LINK(this, ScDbTableResizeDlg, OkBtnHdl));
    m_xBtnCancel->connect_clicked(LINK(this, ScDbTableResizeDlg, CancelBtnHdl));
    m_xEdAssign->SetModifyHdl(LINK(this, ScDbTableResizeDlg, AssModifyHdl));

    // Preset the entry with the Table's current range so it can be edited.
    const ScAddress aCursor = m_rViewData.GetCurPos();
    if (const ScDBData* pDBData = rDoc.GetTableDBAtCursor(aCursor.Col(), aCursor.Row(),
                                                          aCursor.Tab(), ScDBDataPortion::AREA))
        pDBData->GetArea(maOldArea);
    else
        maOldArea = ScRange(aCursor);

    theCurArea = maOldArea;
    OUString theAreaStr = maOldArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, aAddrDetails);
    m_xEdAssign->SetText(theAreaStr);
}

void ScDbTableResizeDlg::SetReference(const ScRange& rRef, ScDocument& rDocP)
{
    if (!m_xEdAssign->GetWidget()->get_sensitive())
        return;

    if (rRef.aStart != rRef.aEnd)
        RefInputStart(m_xEdAssign.get());

    theCurArea = rRef;

    OUString aRefStr(theCurArea.Format(rDocP, ScRefFlags::RANGE_ABS_3D, aAddrDetails));
    m_xEdAssign->SetRefString(aRefStr);
}

void ScDbTableResizeDlg::Close() { DoClose(ScTableResizeWrapper::GetChildWindowId()); }

void ScDbTableResizeDlg::SetActive()
{
    m_xEdAssign->GrabFocus();
    RefInputDone();
}

bool ScDbTableResizeDlg::IsRefInputMode() const { return bRefInputMode; }

void ScDbTableResizeDlg::ErrorBox(const OUString& rString)
{
    if (m_xInfoBox)
        m_xInfoBox->response(RET_OK);

    m_xInfoBox = std::shared_ptr<weld::MessageDialog>(Application::CreateMessageDialog(
        m_xDialog.get(), VclMessageType::Warning, VclButtonsType::Ok, rString));

    m_xInfoBox->runAsync(m_xInfoBox, [this](sal_Int32) { m_xInfoBox = nullptr; });
}

IMPL_LINK_NOARG(ScDbTableResizeDlg, OkBtnHdl, weld::Button&, void)
{
    OUString aNewArea = m_xEdAssign->GetText();
    if (aNewArea.isEmpty())
        return;

    ScRange aNewRange;
    if (!(aNewRange.ParseAny(aNewArea, rDoc, aAddrDetails) & ScRefFlags::VALID))
    {
        ErrorBox(ScResId(STR_ERR_INVALID_AREA));
        m_xEdAssign->SelectAll();
        m_xEdAssign->GrabFocus();
        return;
    }
    aNewRange.PutInOrder();

    // Locate the Table being resized by its (still unchanged) top-left cell.
    ScDocShell* pDocSh = m_rViewData.GetDocShell();
    ScDocument& rDocument = pDocSh->GetDocument();
    ScDBData* pDBData = rDocument.GetTableDBAtCursor(maOldArea.aStart.Col(), maOldArea.aStart.Row(),
                                                     maOldArea.aStart.Tab(), ScDBDataPortion::AREA);
    if (!pDBData)
    {
        ErrorBox(ScResId(STR_INVALIDTABLE));
        return;
    }

    ScRange aOldRange;
    pDBData->GetArea(aOldRange);

    // A Table resize never changes sheet, so interpret the entry on the Table's own sheet
    // (a bare "B2:F13" would otherwise parse onto the first sheet).
    aNewRange.aStart.SetTab(aOldRange.aStart.Tab());
    aNewRange.aEnd.SetTab(aOldRange.aStart.Tab());

    // Keep the header row in place, and overlap the original.
    const bool bAligns
        = aNewRange.aStart.Row() == aOldRange.aStart.Row() && aNewRange.Intersects(aOldRange);
    if (!bAligns)
    {
        ErrorBox(ScResId(STR_MSSG_TABLE_RESIZE_INVALID));
        m_xEdAssign->SelectAll();
        m_xEdAssign->GrabFocus();
        return;
    }

    // The Table must keep at least one data row below the header, and the Total Row on top of that.
    if (aNewRange.aEnd.Row() - aNewRange.aStart.Row() < pDBData->GetMinRowSpan())
    {
        ErrorBox(ScResId(STR_MSSG_TABLE_RESIZE_NO_DATA));
        m_xEdAssign->SelectAll();
        m_xEdAssign->GrabFocus();
        return;
    }

    // Commit through the shared helper, so the dialog and the drag-resize handle behave
    // identically (Total Row relocation, header-name generation, single undo).
    ScDBDocFunc(*pDocSh).ResizeTable(*pDBData, aNewRange);

    // A refused resize leaves the table unchanged (ResizeTable shows the error); keep the dialog
    // open by re-finding at the old top-left, which a successful resize keeps inside.
    if (const ScDBData* pAfter
        = rDocument.GetTableDBAtCursor(maOldArea.aStart.Col(), maOldArea.aStart.Row(),
                                       maOldArea.aStart.Tab(), ScDBDataPortion::AREA))
    {
        ScRange aResult;
        pAfter->GetArea(aResult);
        if (aResult == aOldRange && aResult != aNewRange)
        {
            m_xEdAssign->GrabFocus();
            return;
        }
    }

    response(RET_OK);
}

IMPL_LINK_NOARG(ScDbTableResizeDlg, CancelBtnHdl, weld::Button&, void) { response(RET_CANCEL); }

IMPL_LINK_NOARG(ScDbTableResizeDlg, AssModifyHdl, formula::RefEdit&, void)
{
    ScRange aTmpRange;
    OUString aText = m_xEdAssign->GetText();
    if (aTmpRange.ParseAny(aText, rDoc, aAddrDetails) & ScRefFlags::VALID)
        theCurArea = aTmpRange;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
