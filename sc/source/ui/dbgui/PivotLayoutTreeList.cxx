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

#include "PivotLayoutTreeList.hxx"
#include "PivotLayoutDialog.hxx"

#include <vcl/builderfactory.hxx>
#include <svtools/treelistentry.hxx>
#include "pivot.hxx"
#include "scabstdlg.hxx"

using namespace std;

VCL_BUILDER_FACTORY_ARGS(ScPivotLayoutTreeList,
                         WB_BORDER | WB_TABSTOP | WB_CLIPCHILDREN |
                         WB_FORCE_MAKEVISIBLE)

ScPivotLayoutTreeList::ScPivotLayoutTreeList(vcl::Window* pParent, WinBits nBits) :
    ScPivotLayoutTreeListBase(pParent, nBits)
{}

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

    vector<ScDPName> aDataFieldNames;
    mpParent->PushDataFieldNames(aDataFieldNames);

    boost::scoped_ptr<AbstractScDPSubtotalDlg> pDialog(
        pFactory->CreateScDPSubtotalDlg(this, mpParent->maPivotTableObject, rCurrentLabelData, rCurrentFunctionData, aDataFieldNames, true));

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

    ScPivotFieldVector::iterator it;
    for (it = rFieldVector.begin(); it != rFieldVector.end(); ++it)
    {
        ScPivotField& rField = *it;
        ScItemValue* pItemValue = mpParent->GetItem(rField.nCol);
        InsertEntry(pItemValue->maName, NULL, false, TREELIST_APPEND, pItemValue);
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

    sal_uLong nPosition = (pTarget == NULL) ? TREELIST_APPEND : GetModel()->GetAbsPos(pTarget) + 1;
    InsertEntryForItem(pOriginalItemValue, nPosition);
}

void ScPivotLayoutTreeList::InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition)
{
    OUString rName = pItemValue->maName;
    InsertEntry(rName, NULL, false, nPosition, pItemValue);
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
