/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 */

#include <memory>
#include <PivotLayoutTreeList.hxx>
#include <PivotLayoutDialog.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <pivot.hxx>

#include <globstr.hrc>
#include <scresid.hxx>

ScPivotLayoutTreeList::ScPivotLayoutTreeList(std::unique_ptr<weld::TreeView> xControl)
    : ScPivotLayoutTreeListBase(std::move(xControl))
{
    mxControl->connect_key_press(LINK(this, ScPivotLayoutTreeList, KeyInputHdl));
    mxControl->connect_row_activated(LINK(this, ScPivotLayoutTreeList, DoubleClickHdl));
}

ScPivotLayoutTreeList::~ScPivotLayoutTreeList()
{
    if (mpSubtotalDlg)
    {
        mpSubtotalDlg->Response(RET_CANCEL);
        mpSubtotalDlg.reset();
    }

    if (m_xInfoBox)
    {
        m_xInfoBox->response(RET_OK);
    }
}

void ScPivotLayoutTreeList::Setup(ScPivotLayoutDialog* pParent, SvPivotTreeListType eType)
{
    mpParent = pParent;
    meType = eType;
}

IMPL_LINK_NOARG(ScPivotLayoutTreeList, DoubleClickHdl, weld::TreeView&, bool)
{
    int nEntry = mxControl->get_cursor_index();
    if (nEntry == -1)
        return true;

    ScItemValue* pCurrentItemValue = weld::fromId<ScItemValue*>(mxControl->get_id(nEntry));
    ScPivotFuncData& rCurrentFunctionData = pCurrentItemValue->maFunctionData;
    SCCOL nCurrentColumn = rCurrentFunctionData.mnCol;

    if (mpParent->IsDataElement(nCurrentColumn))
        return true;

    ScDPLabelData& rCurrentLabelData = mpParent->GetLabelData(nCurrentColumn);

    ScAbstractDialogFactory* pFactory = ScAbstractDialogFactory::Create();

    maDataFieldNames.clear();
    mpParent->PushDataFieldNames(maDataFieldNames);

    mpSubtotalDlg = pFactory->CreateScDPSubtotalDlg(mxControl.get(), mpParent->maPivotTableObject,
                                                    rCurrentLabelData, rCurrentFunctionData,
                                                    maDataFieldNames);

    mpSubtotalDlg->StartExecuteAsync([this, pCurrentItemValue, nCurrentColumn](int nResult) {
        if (nResult == RET_OK)
        {
            mpSubtotalDlg->FillLabelData(mpParent->GetLabelData(nCurrentColumn));
            pCurrentItemValue->maFunctionData.mnFuncMask = mpSubtotalDlg->GetFuncMask();
        }

        mpSubtotalDlg.disposeAndClear();
    });

    return true;
}

void ScPivotLayoutTreeList::FillFields(ScPivotFieldVector& rFieldVector)
{
    mxControl->clear();
    maItemValues.clear();

    for (const ScPivotField& rField : rFieldVector)
    {
        OUString aLabel = mpParent->GetItem(rField.nCol)->maName;
        ScItemValue* pItemValue = new ScItemValue(aLabel, rField.nCol, rField.nFuncMask);
        maItemValues.push_back(std::unique_ptr<ScItemValue>(pItemValue));
        OUString sId(weld::toId(pItemValue));
        mxControl->append(sId, pItemValue->maName);
    }
}

void ScPivotLayoutTreeList::InsertEntryForSourceTarget(weld::TreeView& rSource, int nTarget)
{
    ScItemValue* pItemValue = weld::fromId<ScItemValue*>(rSource.get_selected_id());
    ScItemValue* pOriginalItemValue = pItemValue->mpOriginalItemValue;

    // Don't allow to add "Data" element to page fields
    if (meType == PAGE_LIST && mpParent->IsDataElement(pItemValue->maFunctionData.mnCol))
        return;

    // Don't allow to add "Calculated Fields" element to non-data fields
    if (mpParent->IsCalculatedElement(pItemValue->maFunctionData.mnCol))
    {
        if (m_xInfoBox)
            m_xInfoBox->response(RET_OK);

        m_xInfoBox = std::shared_ptr<weld::MessageDialog>(Application::CreateMessageDialog(
            mxControl.get(), VclMessageType::Warning, VclButtonsType::Ok,
            ScResId(STR_PIVOT_CALC_FIELD_DRAG)));

        m_xInfoBox->runAsync(m_xInfoBox, [this](sal_Int32) { m_xInfoBox = nullptr; });
        return;
    }

    mpParent->ItemInserted(pOriginalItemValue, meType);

    InsertEntryForItem(pOriginalItemValue, nTarget);
}

void ScPivotLayoutTreeList::InsertEntryForItem(const ScItemValue* pItemValue, int nPosition)
{
    ScItemValue* pListItemValue = new ScItemValue(pItemValue);
    maItemValues.push_back(std::unique_ptr<ScItemValue>(pListItemValue));
    OUString sName = pListItemValue->maName;
    OUString sId(weld::toId(pListItemValue));
    mxControl->insert(nullptr, nPosition, &sName, &sId, nullptr, nullptr, false, nullptr);
}

IMPL_LINK(ScPivotLayoutTreeList, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    vcl::KeyCode aCode = rKeyEvent.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    if (nCode == KEY_DELETE)
    {
        const int nEntry = mxControl->get_cursor_index();
        if (nEntry != -1)
            mxControl->remove(nEntry);
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
