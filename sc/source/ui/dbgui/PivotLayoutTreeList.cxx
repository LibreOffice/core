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

#include <svtools/treelistentry.hxx>
#include "pivot.hxx"
#include "scabstdlg.hxx"

using namespace std;

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeScPivotLayoutTreeList(Window *pParent, VclBuilder::stringmap& )
{
    return new ScPivotLayoutTreeList(pParent, WB_BORDER | WB_TABSTOP | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
}

ScPivotLayoutTreeList::ScPivotLayoutTreeList(Window* pParent, WinBits nBits) :
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
    ScItemValue* pCurrentItemValue = (ScItemValue*) GetCurEntry()->GetUserData();
    ScPivotFuncData& rCurrentFunctionData = pCurrentItemValue->maFunctionData;

    SCCOL nCurrentColumn = rCurrentFunctionData.mnCol;
    ScDPLabelData* pCurrentLabelData = mpParent->GetLabelData(nCurrentColumn);
    if (!pCurrentLabelData)
        return false;

    ScAbstractDialogFactory* pFactory = ScAbstractDialogFactory::Create();

    vector<ScDPName> aDataFieldNames;
    SvTreeListEntry* pLoopEntry;
    for (pLoopEntry = First(); pLoopEntry != NULL; pLoopEntry = Next(pLoopEntry))
    {
        ScItemValue* pEachItemValue = (ScItemValue*) pLoopEntry->GetUserData();
        SCCOL nColumn = pEachItemValue->maFunctionData.mnCol;

        ScDPLabelData* pDFData = mpParent->GetLabelData(nColumn);
        if (pDFData == NULL && pDFData->maName.isEmpty())
            continue;

        aDataFieldNames.push_back(ScDPName(pDFData->maName, pDFData->maLayoutName, pDFData->mnDupCount));
    }

    boost::scoped_ptr<AbstractScDPSubtotalDlg> pDialog(
        pFactory->CreateScDPSubtotalDlg(this, mpParent->maPivotTableObject, *pCurrentLabelData, rCurrentFunctionData, aDataFieldNames, true));

    if (pDialog->Execute() == RET_OK)
    {
        pDialog->FillLabelData(*pCurrentLabelData);
        rCurrentFunctionData.mnFuncMask = pCurrentLabelData->mnFuncMask;
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
        InsertEntry(pItemValue->maName, NULL, sal_False, TREELIST_APPEND, pItemValue);
    }
}

void ScPivotLayoutTreeList::InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget)
{
    ScItemValue* pItemValue = (ScItemValue*) pSource->GetUserData();
    ScItemValue* pOriginalItemValue = pItemValue->mpOriginalItemValue;

    // Don't allow to add "Data" element to page fields
    if(meType == PAGE_LIST && mpParent->IsDataItem(pItemValue->maFunctionData.mnCol))
        return;

    mpParent->ItemInserted(pOriginalItemValue, meType);

    sal_uLong nPosition = (pTarget == NULL) ? TREELIST_APPEND : GetModel()->GetAbsPos(pTarget) + 1;
    InsertEntryForItem(pOriginalItemValue, nPosition);
}

void ScPivotLayoutTreeList::InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition)
{
    OUString rName = pItemValue->maName;
    InsertEntry(rName, NULL, sal_False, nPosition, pItemValue);
}

void ScPivotLayoutTreeList::KeyInput(const KeyEvent& rKeyEvent)
{
    KeyCode aCode = rKeyEvent.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch (nCode)
    {
        case KEY_DELETE:
            GetModel()->Remove(GetCurEntry());
            return;
    }
    SvTreeListBox::KeyInput(rKeyEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
