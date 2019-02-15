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
#include <PivotLayoutTreeListData.hxx>
#include <PivotLayoutDialog.hxx>

#include <vcl/builderfactory.hxx>
#include <vcl/treelistentry.hxx>
#include <pivot.hxx>
#include <scabstdlg.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

VCL_BUILDER_FACTORY_ARGS(ScPivotLayoutTreeListData,
                         WB_BORDER | WB_TABSTOP | WB_CLIPCHILDREN)

namespace
{

OUString lclGetFunctionMaskName(const PivotFunc nFunctionMask)
{
    const char* pStrId = nullptr;
    switch (nFunctionMask)
    {
        case PivotFunc::Sum:        pStrId = STR_FUN_TEXT_SUM;      break;
        case PivotFunc::Count:      pStrId = STR_FUN_TEXT_COUNT;    break;
        case PivotFunc::Average:    pStrId = STR_FUN_TEXT_AVG;      break;
        case PivotFunc::Median:     pStrId = STR_FUN_TEXT_MEDIAN;   break;
        case PivotFunc::Max:        pStrId = STR_FUN_TEXT_MAX;      break;
        case PivotFunc::Min:        pStrId = STR_FUN_TEXT_MIN;      break;
        case PivotFunc::Product:    pStrId = STR_FUN_TEXT_PRODUCT;  break;
        case PivotFunc::CountNum:   pStrId = STR_FUN_TEXT_COUNT;    break;
        case PivotFunc::StdDev:     pStrId = STR_FUN_TEXT_STDDEV;   break;
        case PivotFunc::StdDevP:    pStrId = STR_FUN_TEXT_STDDEV;   break;
        case PivotFunc::StdVar:     pStrId = STR_FUN_TEXT_VAR;      break;
        case PivotFunc::StdVarP:    pStrId = STR_FUN_TEXT_VAR;      break;
        default:
            assert(false);
            break;
    }
    if (pStrId)
        return ScResId(pStrId);
    else
        return OUString();
}

OUString lclCreateDataItemName(const PivotFunc nFunctionMask, const OUString& rName, const sal_uInt8 nDuplicationCount)
{
    OUString aBuffer = lclGetFunctionMaskName(nFunctionMask) + " - " + rName;
    if(nDuplicationCount > 0)
    {
        aBuffer += " " + OUString::number(nDuplicationCount);
    }
    return aBuffer;
}

} // anonymous namespace

ScPivotLayoutTreeListData::ScPivotLayoutTreeListData(vcl::Window* pParent, WinBits nBits) :
    ScPivotLayoutTreeListBase(pParent, nBits, DATA_LIST)
{
    SetForceMakeVisible(true);
}

ScPivotLayoutTreeListData::~ScPivotLayoutTreeListData()
{}

bool ScPivotLayoutTreeListData::DoubleClickHdl()
{
    ScItemValue* pCurrentItemValue = static_cast<ScItemValue*>(GetCurEntry()->GetUserData());
    ScPivotFuncData& rCurrentFunctionData = pCurrentItemValue->maFunctionData;

    SCCOL nCurrentColumn = rCurrentFunctionData.mnCol;
    ScDPLabelData& rCurrentLabelData = mpParent->GetLabelData(nCurrentColumn);

    ScAbstractDialogFactory* pFactory = ScAbstractDialogFactory::Create();

    ScopedVclPtr<AbstractScDPFunctionDlg> pDialog(
        pFactory->CreateScDPFunctionDlg(GetFrameWeld(), mpParent->GetLabelDataVector(), rCurrentLabelData, rCurrentFunctionData));

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

    for (ScPivotField& rField : rDataFields)
    {
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

        maDataItemValues.push_back(std::unique_ptr<ScItemValue>(pItemValue));
        InsertEntry(sDataItemName, nullptr, false, TREELIST_APPEND, pItemValue);
    }
}

void ScPivotLayoutTreeListData::PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames)
{
    SvTreeListEntry* pLoopEntry;
    for (pLoopEntry = First(); pLoopEntry != nullptr; pLoopEntry = Next(pLoopEntry))
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

        rDataFieldNames.emplace_back(rLabelData.maName, sLayoutName, rLabelData.mnDupCount);
    }
}

void ScPivotLayoutTreeListData::InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget)
{
    ScItemValue* pItemValue = static_cast<ScItemValue*>(pSource->GetUserData());

    if (mpParent->IsDataElement(pItemValue->maFunctionData.mnCol))
        return;

    if (HasEntry(pSource))
    {
        OUString rText = GetEntryText(pSource);
        GetModel()->Remove(pSource);
        sal_uLong nPosition = (pTarget == nullptr) ? TREELIST_APPEND : GetModel()->GetAbsPos(pTarget) + 1;
        InsertEntry(rText, nullptr, false, nPosition, pItemValue);
    }
    else
    {
        sal_uLong nPosition = (pTarget == nullptr) ? TREELIST_APPEND : GetModel()->GetAbsPos(pTarget) + 1;
        InsertEntryForItem(pItemValue->mpOriginalItemValue, nPosition);
    }
}

void ScPivotLayoutTreeListData::InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition)
{
    ScItemValue* pDataItemValue = new ScItemValue(pItemValue);
    pDataItemValue->mpOriginalItemValue = pItemValue;
    maDataItemValues.push_back(std::unique_ptr<ScItemValue>(pDataItemValue));

    ScPivotFuncData& rFunctionData = pDataItemValue->maFunctionData;

    if (rFunctionData.mnFuncMask == PivotFunc::NONE ||
        rFunctionData.mnFuncMask == PivotFunc::Auto)
    {
        rFunctionData.mnFuncMask = PivotFunc::Sum;
    }

    AdjustDuplicateCount(pDataItemValue);

    OUString sDataName = lclCreateDataItemName(
                            rFunctionData.mnFuncMask,
                            pDataItemValue->maName,
                            rFunctionData.mnDupCount);

    InsertEntry(sDataName, nullptr, false, nPosition, pDataItemValue);
}

void ScPivotLayoutTreeListData::AdjustDuplicateCount(ScItemValue* pInputItemValue)
{
    ScPivotFuncData& rInputFunctionData = pInputItemValue->maFunctionData;

    bool bFoundDuplicate = false;

    rInputFunctionData.mnDupCount = 0;
    sal_uInt8 nMaxDuplicateCount = 0;

    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != nullptr; pEachEntry = Next(pEachEntry))
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
