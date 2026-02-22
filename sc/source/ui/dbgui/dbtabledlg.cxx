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

#include <memory>
#include <sal/config.h>

#include <cassert>

#include <comphelper/string.hxx>
#include <unotools/charclass.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weld.hxx>
#include <o3tl/string_view.hxx>

#include <reffact.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <dbtabledlg.hxx>
#include <dbdocfun.hxx>
#include <tablestyle.hxx>

ScDbTableDlg::ScDbTableDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                           ScViewData& rViewData)
    : ScAnyRefDlgController(pB, pCW, pParent, u"modules/scalc/ui/definetablerangedialog.ui"_ustr,
                            u"tablerangedialog"_ustr)
    , m_rViewData(rViewData)
    , rDoc(rViewData.GetDocument())
    , bRefInputMode(true)
    , aAddrDetails(rDoc.GetAddressConvention(), 0, 0)
    , m_xAssignFrame(m_xBuilder->weld_frame(u"rangeframe"_ustr))
    , m_xEdAssign(new formula::RefEdit(m_xBuilder->weld_entry(u"assign"_ustr)))
    , m_xRbAssign(new formula::RefButton(m_xBuilder->weld_button(u"assgnrb"_ustr)))
    , m_xOptions(m_xBuilder->weld_widget(u"options"_ustr))
    , m_xBtnHeader(m_xBuilder->weld_check_button(u"bheaders"_ustr))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
{
    m_xEdAssign->SetReferences(this, m_xAssignFrame.get());
    m_xRbAssign->SetReferences(this, m_xEdAssign.get());

    Init();
}

ScDbTableDlg::~ScDbTableDlg()
{
    if (m_xInfoBox)
        m_xInfoBox->response(RET_OK);
}

void ScDbTableDlg::Init()
{
    m_xBtnHeader->set_active(true); // Default: with column headers

    m_xBtnOk->connect_clicked(LINK(this, ScDbTableDlg, OkBtnHdl));
    m_xBtnCancel->connect_clicked(LINK(this, ScDbTableDlg, CancelBtnHdl));
    m_xEdAssign->SetModifyHdl(LINK(this, ScDbTableDlg, AssModifyHdl));

    SCCOL nStartCol, nEndCol = 0;
    SCROW nStartRow, nEndRow = 0;
    SCTAB nStartTab, nEndTab = 0;

    m_rViewData.GetSimpleArea(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
    theCurArea = ScRange(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
    OUString theAreaStr = theCurArea.Format(rDoc, ScRefFlags::RANGE_ABS_3D, aAddrDetails);
    m_xEdAssign->SetText(theAreaStr);

    bInvalid = false;
}

void ScDbTableDlg::SetReference(const ScRange& rRef, ScDocument& rDocP)
{
    if (!m_xEdAssign->GetWidget()->get_sensitive())
        return;

    if (rRef.aStart != rRef.aEnd)
        RefInputStart(m_xEdAssign.get());

    theCurArea = rRef;

    OUString aRefStr(theCurArea.Format(rDocP, ScRefFlags::RANGE_ABS_3D, aAddrDetails));
    m_xEdAssign->SetRefString(aRefStr);
    m_xOptions->set_sensitive(true);
}

void ScDbTableDlg::Close() { DoClose(ScTableLayoutWrapper::GetChildWindowId()); }

void ScDbTableDlg::SetActive()
{
    m_xEdAssign->GrabFocus();
    RefInputDone();
}

bool ScDbTableDlg::IsRefInputMode() const { return bRefInputMode; }

void ScDbTableDlg::ErrorBox(const OUString& rString)
{
    if (m_xInfoBox)
        m_xInfoBox->response(RET_OK);

    m_xInfoBox = std::shared_ptr<weld::MessageDialog>(Application::CreateMessageDialog(
        m_xDialog.get(), VclMessageType::Warning, VclButtonsType::Ok, rString));

    m_xInfoBox->runAsync(m_xInfoBox, [this](sal_Int32) { m_xInfoBox = nullptr; });
}

IMPL_LINK_NOARG(ScDbTableDlg, OkBtnHdl, weld::Button&, void)
{
    bInvalid = false;

    OUString aNewArea = m_xEdAssign->GetText();

    if (aNewArea.isEmpty())
        return;

    ScRange aTmpRange;
    if (aTmpRange.ParseAny(aNewArea, rDoc, aAddrDetails) & ScRefFlags::VALID)
    {
        if (aTmpRange.aStart.Row() == aTmpRange.aEnd.Row()) // minimum 2 row height needs
            aTmpRange.aEnd.IncRow();
        theCurArea = aTmpRange;

        std::vector<const ScDBData*> aDBData = rDoc.GetAllNamedDBsInArea(
            theCurArea.aStart.Col(), theCurArea.aStart.Row(), theCurArea.aEnd.Col(),
            theCurArea.aEnd.Row(), theCurArea.aStart.Tab());

        for (const ScDBData* pDBData : aDBData)
        {
            if (pDBData->GetTableStyleInfo())
            {
                bInvalid = true;
                theCurArea = ScRange();
                break;
            }
        }

        if (bInvalid)
        {
            ErrorBox(ScResId(STR_INVALIDTABLE));
            m_xEdAssign->SelectAll();
            m_xEdAssign->GrabFocus();
            return;
        }

        //  insert new area
        OUString aTable = ScResId(STR_CALC_TABLE);
        tools::Long nCount = 0;
        const ScDBData* pDummy = nullptr;
        ScDBCollection::NamedDBs& rDBs = rDoc.GetDBCollection()->getNamedDBs();
        OUString aNewName;
        do
        {
            ++nCount;
            aNewName = aTable + OUString::number(nCount);
            pDummy = rDBs.findByUpperName(ScGlobal::getCharClass().uppercase(aNewName));
        } while (pDummy);

        OUString aDefault = u""_ustr;
        if (const ScTableStyles* pTableStyles = rDoc.GetTableStyles())
        {
            const ScTableStyle* pTableStyle
                = pTableStyles->GetTableStyle(u"TableStyleMedium2"_ustr);
            if (pTableStyle)
                aDefault = u"TableStyleMedium2"_ustr;
        }

        ScDBDocFunc aFunc(*m_rViewData.GetDocShell());
        bInvalid = !aFunc.AddDBTable(aNewName, ScRange(theCurArea.aStart, theCurArea.aEnd),
                                     m_xBtnHeader->get_active(), true, false, aDefault);

        m_xEdAssign->SetText(u""_ustr);
        m_xBtnHeader->set_active(true);
        theCurArea = ScRange();
    }
    else
    {
        ErrorBox(ScResId(STR_ERR_INVALID_AREA));
        m_xEdAssign->SelectAll();
        m_xEdAssign->GrabFocus();
        bInvalid = true;
    }

    if (!bInvalid)
        response(RET_OK);
}

IMPL_LINK_NOARG(ScDbTableDlg, CancelBtnHdl, weld::Button&, void) { response(RET_CANCEL); }

IMPL_LINK_NOARG(ScDbTableDlg, AssModifyHdl, formula::RefEdit&, void)
{
    ScRange aTmpRange;
    OUString aText = m_xEdAssign->GetText();
    if (aTmpRange.ParseAny(aText, rDoc, aAddrDetails) & ScRefFlags::VALID)
        theCurArea = aTmpRange;

    if (!aText.isEmpty())
    {
        m_xBtnHeader->set_sensitive(true);
    }
    else
    {
        m_xBtnHeader->set_sensitive(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
