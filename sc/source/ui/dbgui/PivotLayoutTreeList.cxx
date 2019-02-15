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

#include <vcl/builderfactory.hxx>
#include <vcl/treelistentry.hxx>
#include <pivot.hxx>
#include <scabstdlg.hxx>

VCL_BUILDER_FACTORY_ARGS(ScPivotLayoutTreeList,
                         WB_BORDER | WB_TABSTOP | WB_CLIPCHILDREN)

ScPivotLayoutTreeList::ScPivotLayoutTreeList(vcl::Window* pParent, WinBits nBits) :
    ScPivotLayoutTreeListBase(pParent, nBits)
{
    SetForceMakeVisible(true);
}

ScPivotLayoutTreeList::~ScPivotLayoutTreeList()
{}

void ScPivotLayoutTreeList::Setup(ScPivotLayoutDialog* pParent, SvPivotTreeListType eType)
{
    mpParent = pParent;
    meType = eType;
}

bool ScPivotLayoutTreeList::DoubleClickHdl()
{
    SvTreeListEntry* pEntry = GetCurEntry();
    if (!pEntry)
        return false;

    ScItemValue* pCurrentItemValue = static_cast<ScItemValue*>(pEntry->GetUserData());
    ScPivotFuncData& rCurrentFunctionData = pCurrentItemValue->maFunctionData;

    if (mpParent->IsDataElement(rCurrentFunctionData.mnCol))
        return false;

    SCCOL nCurrentColumn = rCurrentFunctionData.mnCol;
    ScDPLabelData& rCurrentLabelData = mpParent->GetLabelData(nCurrentColumn);

    ScAbstractDialogFactory* pFactory = ScAbstractDialogFactory::Create();

    std::vector<ScDPName> aDataFieldNames;
    mpParent->PushDataFieldNames(aDataFieldNames);

    ScopedVclPtr<AbstractScDPSubtotalDlg> pDialog(
        pFactory->CreateScDPSubtotalDlg(GetFrameWeld(), mpParent->maPivotTableObject, rCurrentLabelData, rCurrentFunctionData, aDataFieldNames));

    if (pDialog->Execute() == RET_OK)
    {
        pDialog->FillLabelData(rCurrentLabelData);
        rCurrentFunctionData.mnFuncMask = pDialog->GetFuncMask();
    }

    return true;
}

void ScPivotLayoutTreeList::FillFields(ScPivotFieldVector& rFieldVector)
{
    Clear();
    maItemValues.clear();

    for (ScPivotField& rField : rFieldVector)
    {
        OUString aLabel = mpParent->GetItem( rField.nCol )->maName;
        ScItemValue* pItemValue = new ScItemValue( aLabel, rField.nCol, rField.nFuncMask );
        maItemValues.push_back(std::unique_ptr<ScItemValue>(pItemValue));
        InsertEntry(pItemValue->maName, nullptr, false, TREELIST_APPEND, pItemValue);
    }
}

void ScPivotLayoutTreeList::InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget)
{
    ScItemValue* pItemValue = static_cast<ScItemValue*>(pSource->GetUserData());
    ScItemValue* pOriginalItemValue = pItemValue->mpOriginalItemValue;

    // Don't allow to add "Data" element to page fields
    if(meType == PAGE_LIST && mpParent->IsDataElement(pItemValue->maFunctionData.mnCol))
        return;

    mpParent->ItemInserted(pOriginalItemValue, meType);

    sal_uLong nPosition = (pTarget == nullptr) ? TREELIST_APPEND : GetModel()->GetAbsPos(pTarget) + 1;
    InsertEntryForItem(pOriginalItemValue, nPosition);
}

void ScPivotLayoutTreeList::InsertEntryForItem(const ScItemValue* pItemValue, sal_uLong nPosition)
{
    ScItemValue *pListItemValue = new ScItemValue(pItemValue);
    maItemValues.push_back(std::unique_ptr<ScItemValue>(pListItemValue));
    OUString rName = pListItemValue->maName;
    InsertEntry(rName, nullptr, false, nPosition, pListItemValue);
}

void ScPivotLayoutTreeList::KeyInput(const KeyEvent& rKeyEvent)
{
    vcl::KeyCode aCode = rKeyEvent.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    if (nCode == KEY_DELETE)
    {
        const SvTreeListEntry* pEntry = GetCurEntry();
        if (pEntry)
            GetModel()->Remove(pEntry);
        return;
    }
    SvTreeListBox::KeyInput(rKeyEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
