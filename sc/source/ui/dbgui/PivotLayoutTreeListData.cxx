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

#include "PivotLayoutTreeListData.hxx"
#include "PivotLayoutDialog.hxx"

#include <vcl/builderfactory.hxx>
#include <svtools/treelistentry.hxx>
#include "pivot.hxx"
#include "scabstdlg.hxx"

using namespace std;

VCL_BUILDER_FACTORY_ARGS(ScPivotLayoutTreeListData,
                         WB_BORDER | WB_TABSTOP | WB_CLIPCHILDREN |
                         WB_FORCE_MAKEVISIBLE)

namespace
{

OUString lclGetFunctionMaskName(const sal_uInt16 nFunctionMask)
{
    switch (nFunctionMask)
    {
        case PIVOT_FUNC_SUM:       return OUString("Sum");
        case PIVOT_FUNC_COUNT:     return OUString("Count");
        case PIVOT_FUNC_AVERAGE:   return OUString("Mean");
        case PIVOT_FUNC_MAX:       return OUString("Max");
        case PIVOT_FUNC_MIN:       return OUString("Min");
        case PIVOT_FUNC_PRODUCT:   return OUString("Product");
        case PIVOT_FUNC_COUNT_NUM: return OUString("Count");
        case PIVOT_FUNC_STD_DEV:   return OUString("StDev");
        case PIVOT_FUNC_STD_DEVP:  return OUString("StDevP");
        case PIVOT_FUNC_STD_VAR:   return OUString("Var");
        case PIVOT_FUNC_STD_VARP:  return OUString("VarP");
        default:
            break;
    }
    return OUString();
}

OUString lclCreateDataItemName(const sal_uInt16 nFunctionMask, const OUString& rName, const sal_uInt8 nDuplicationCount)
{
    OUStringBuffer aBuffer;
    aBuffer.append(lclGetFunctionMaskName(nFunctionMask));
    aBuffer.append(" - ");
    aBuffer.append(rName);
    if(nDuplicationCount > 0)
    {
        aBuffer.append(" ");
        aBuffer.append(OUString::number(nDuplicationCount));
    }

    return aBuffer.makeStringAndClear();
}

} // anonymous namespace

ScPivotLayoutTreeListData::ScPivotLayoutTreeListData(vcl::Window* pParent, WinBits nBits) :
    ScPivotLayoutTreeListBase(pParent, nBits, DATA_LIST)
{}

ScPivotLayoutTreeListData::~ScPivotLayoutTreeListData()
{}

bool ScPivotLayoutTreeListData::DoubleClickHdl()
{
    ScItemValue* pCurrentItemValue = static_cast<ScItemValue*>(GetCurEntry()->GetUserData());
    ScPivotFuncData& rCurrentFunctionData = pCurrentItemValue->maFunctionData;

    SCCOL nCurrentColumn = rCurrentFunctionData.mnCol;
    ScDPLabelData& rCurrentLabelData = mpParent->GetLabelData(nCurrentColumn);

    ScAbstractDialogFactory* pFactory = ScAbstractDialogFactory::Create();

    std::unique_ptr<AbstractScDPFunctionDlg> pDialog(
        pFactory->CreateScDPFunctionDlg(this, mpParent->GetLabelDataVector(), rCurrentLabelData, rCurrentFunctionData));

    if (pDialog->Execute() == RET_OK)
    {
        rCurrentFunctionData.mnFuncMask = pDialog->GetFuncMask();
        rCurrentLabelData.mnFuncMask = pDialog->GetFuncMask();

        rCurrentFunctionData.maFieldRef = pDialog->GetFieldRef();

        ScDPLabelData& rDFData = mpParent->GetLabelData(rCurrentFunctionData.mnCol);

        AdjustDuplicateCount(pCurrentItemValue);

        OUString sDataItemName = lclCreateDataItemName(
                                    rCurrentFunctionData.mnFuncMask,
                                    rDFData.maName,
                                    rCurrentFunctionData.mnDupCount);

        SetEntryText(GetCurEntry(), sDataItemName);
    }

    return true;
}

void ScPivotLayoutTreeListData::FillDataField(ScPivotFieldVector& rDataFields)
{
    Clear();
    maDataItemValues.clear();

    ScPivotFieldVector::iterator it;
    for (it = rDataFields.begin(); it != rDataFields.end(); ++it)
    {
        ScPivotField& rField = *it;

        if (rField.nCol == PIVOT_DATA_FIELD)
            continue;

        SCCOL nColumn;
        if (rField.mnOriginalDim >= 0)
            nColumn = rField.mnOriginalDim;
        else
            nColumn = rField.nCol;

        ScItemValue* pOriginalItemValue = mpParent->GetItem(nColumn);
        ScItemValue* pItemValue = new ScItemValue(pOriginalItemValue->maName, nColumn, rField.nFuncMask);

        pItemValue->mpOriginalItemValue = pOriginalItemValue;
        pItemValue->maFunctionData.mnOriginalDim = rField.mnOriginalDim;
        pItemValue->maFunctionData.maFieldRef = rField.maFieldRef;

        AdjustDuplicateCount(pItemValue);
        OUString sDataItemName = lclCreateDataItemName(pItemValue->maFunctionData.mnFuncMask,
                                                       pItemValue->maName,
                                                       pItemValue->maFunctionData.mnDupCount);

        maDataItemValues.push_back(pItemValue);
        InsertEntry(sDataItemName, NULL, false, TREELIST_APPEND, pItemValue);
    }
}

void ScPivotLayoutTreeListData::PushDataFieldNames(vector<ScDPName>& rDataFieldNames)
{
    SvTreeListEntry* pLoopEntry;
    for (pLoopEntry = First(); pLoopEntry != NULL; pLoopEntry = Next(pLoopEntry))
    {
        ScItemValue* pEachItemValue = static_cast<ScItemValue*>(pLoopEntry->GetUserData());
        SCCOL nColumn = pEachItemValue->maFunctionData.mnCol;

        ScDPLabelData& rLabelData = mpParent->GetLabelData(nColumn);

        if (rLabelData.maName.isEmpty())
            continue;

        OUString sLayoutName = rLabelData.maLayoutName;
        if (sLayoutName.isEmpty())
        {
            sLayoutName = lclCreateDataItemName(
                            pEachItemValue->maFunctionData.mnFuncMask,
                            pEachItemValue->maName,
                            pEachItemValue->maFunctionData.mnDupCount);
        }

        rDataFieldNames.push_back(ScDPName(rLabelData.maName, sLayoutName, rLabelData.mnDupCount));
    }
}

void ScPivotLayoutTreeListData::InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget)
{
    ScItemValue* pItemValue = static_cast<ScItemValue*>(pSource->GetUserData());

    if(mpParent->IsDataElement(pItemValue->maFunctionData.mnCol))
        return;

    if (HasEntry(pSource))
    {
        OUString rText = GetEntryText(pSource);
        GetModel()->Remove(pSource);
        sal_uLong nPosition = (pTarget == NULL) ? TREELIST_APPEND : GetModel()->GetAbsPos(pTarget) + 1;
        InsertEntry(rText, NULL, false, nPosition, pItemValue);
    }
    else
    {
        sal_uLong nPosition = (pTarget == NULL) ? TREELIST_APPEND : GetModel()->GetAbsPos(pTarget) + 1;
        InsertEntryForItem(pItemValue->mpOriginalItemValue, nPosition);
    }
}

void ScPivotLayoutTreeListData::InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition)
{
    ScItemValue* pDataItemValue = new ScItemValue(pItemValue);
    pDataItemValue->mpOriginalItemValue = pItemValue;
    maDataItemValues.push_back(pDataItemValue);

    ScPivotFuncData& rFunctionData = pDataItemValue->maFunctionData;

    if (rFunctionData.mnFuncMask == PIVOT_FUNC_NONE ||
        rFunctionData.mnFuncMask == PIVOT_FUNC_AUTO)
    {
        rFunctionData.mnFuncMask = PIVOT_FUNC_SUM;
    }

    AdjustDuplicateCount(pDataItemValue);

    OUString sDataName = lclCreateDataItemName(
                            rFunctionData.mnFuncMask,
                            pDataItemValue->maName,
                            rFunctionData.mnDupCount);

    InsertEntry(sDataName, NULL, false, nPosition, pDataItemValue);
}

void ScPivotLayoutTreeListData::AdjustDuplicateCount(ScItemValue* pInputItemValue)
{
    ScPivotFuncData& rInputFunctionData = pInputItemValue->maFunctionData;

    bool bFoundDuplicate = false;

    rInputFunctionData.mnDupCount = 0;
    sal_uInt8 nMaxDuplicateCount = 0;

    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != NULL; pEachEntry = Next(pEachEntry))
    {
        ScItemValue* pItemValue = static_cast<ScItemValue*>(pEachEntry->GetUserData());
        if (pItemValue == pInputItemValue)
            continue;

        ScPivotFuncData& rFunctionData = pItemValue->maFunctionData;

        if (rFunctionData.mnCol      == rInputFunctionData.mnCol &&
            rFunctionData.mnFuncMask == rInputFunctionData.mnFuncMask)
        {
            bFoundDuplicate = true;
            if(rFunctionData.mnDupCount > nMaxDuplicateCount)
                nMaxDuplicateCount = rFunctionData.mnDupCount;
        }
    }

    if(bFoundDuplicate)
    {
        rInputFunctionData.mnDupCount = nMaxDuplicateCount + 1;
    }
}

void ScPivotLayoutTreeListData::KeyInput(const KeyEvent& rKeyEvent)
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
