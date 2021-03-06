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
#include <string_view>

#include <PivotLayoutTreeListData.hxx>
#include <PivotLayoutDialog.hxx>

#include <vcl/event.hxx>
#include <pivot.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

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

OUString lclCreateDataItemName(const PivotFunc nFunctionMask, std::u16string_view rName, const sal_uInt8 nDuplicationCount)
{
    OUString aBuffer = lclGetFunctionMaskName(nFunctionMask) + " - " + rName;
    if(nDuplicationCount > 0)
    {
        aBuffer += " " + OUString::number(nDuplicationCount);
    }
    return aBuffer;
}

} // anonymous namespace

ScPivotLayoutTreeListData::ScPivotLayoutTreeListData(std::unique_ptr<weld::TreeView> xControl)
    : ScPivotLayoutTreeListBase(std::move(xControl))
{
    mxControl->connect_key_press(LINK(this, ScPivotLayoutTreeListData, KeyInputHdl));
    mxControl->connect_row_activated(LINK(this, ScPivotLayoutTreeListData, DoubleClickHdl));
}

ScPivotLayoutTreeListData::~ScPivotLayoutTreeListData()
{
    if (mpFunctionDlg)
    {
        mpFunctionDlg->Response(RET_CANCEL);
        mpFunctionDlg.clear();
    }
}

IMPL_LINK_NOARG(ScPivotLayoutTreeListData, DoubleClickHdl, weld::TreeView&, bool)
{
    int nEntry = mxControl->get_cursor_index();
    if (nEntry == -1)
        return true;

    ScItemValue* pCurrentItemValue = reinterpret_cast<ScItemValue*>(mxControl->get_id(nEntry).toInt64());
    ScPivotFuncData& rCurrentFunctionData = pCurrentItemValue->maFunctionData;

    SCCOL nCurrentColumn = rCurrentFunctionData.mnCol;
    ScDPLabelData& rCurrentLabelData = mpParent->GetLabelData(nCurrentColumn);

    ScAbstractDialogFactory* pFactory = ScAbstractDialogFactory::Create();

    mpFunctionDlg = pFactory->CreateScDPFunctionDlg(mxControl.get(), mpParent->GetLabelDataVector(), rCurrentLabelData, rCurrentFunctionData);

    mpFunctionDlg->StartExecuteAsync([this, pCurrentItemValue,
                                rCurrentLabelData, nEntry](int nResult) mutable {
        if (nResult == RET_OK)
        {
            ScPivotFuncData& rFunctionData = pCurrentItemValue->maFunctionData;
            rFunctionData.mnFuncMask = mpFunctionDlg->GetFuncMask();
            rCurrentLabelData.mnFuncMask = mpFunctionDlg->GetFuncMask();

            rFunctionData.maFieldRef = mpFunctionDlg->GetFieldRef();

            ScDPLabelData& rDFData = mpParent->GetLabelData(rFunctionData.mnCol);

            AdjustDuplicateCount(pCurrentItemValue);

            OUString sDataItemName = lclCreateDataItemName(
                                        rFunctionData.mnFuncMask,
                                        rDFData.maName,
                                        rFunctionData.mnDupCount);

            mxControl->set_text(nEntry, sDataItemName);
        }

        mpFunctionDlg->disposeOnce();
    });

    return true;
}

void ScPivotLayoutTreeListData::FillDataField(ScPivotFieldVector& rDataFields)
{
    mxControl->clear();
    maDataItemValues.clear();

    for (const ScPivotField& rField : rDataFields)
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
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pItemValue)));
        mxControl->append(sId, sDataItemName);
    }
}

void ScPivotLayoutTreeListData::PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames)
{
    std::unique_ptr<weld::TreeIter> xLoopEntry(mxControl->make_iterator());
    if (!mxControl->get_iter_first(*xLoopEntry))
        return;

    do
    {
        ScItemValue* pEachItemValue = reinterpret_cast<ScItemValue*>(mxControl->get_id(*xLoopEntry).toInt64());
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
    } while (mxControl->iter_next(*xLoopEntry));
}

void ScPivotLayoutTreeListData::InsertEntryForSourceTarget(weld::TreeView& rSource, int nTarget)
{
    if (rSource.count_selected_rows() <=0)
        return;

    ScItemValue* pItemValue = reinterpret_cast<ScItemValue*>(rSource.get_selected_id().toInt64());

    if (mpParent->IsDataElement(pItemValue->maFunctionData.mnCol))
        return;

    if (&rSource == mxControl.get())
    {
        OUString sText = mxControl->get_selected_text();
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pItemValue)));
        mxControl->remove_id(sId);
        mxControl->insert(nullptr, nTarget, &sText, &sId, nullptr, nullptr, false, nullptr);
    }
    else
    {
        InsertEntryForItem(pItemValue->mpOriginalItemValue, nTarget);
    }
}

void ScPivotLayoutTreeListData::InsertEntryForItem(ScItemValue* pItemValue, int nPosition)
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

    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pDataItemValue)));
    mxControl->insert(nullptr, nPosition, &sDataName, &sId, nullptr, nullptr, false, nullptr);
}

void ScPivotLayoutTreeListData::AdjustDuplicateCount(ScItemValue* pInputItemValue)
{
    ScPivotFuncData& rInputFunctionData = pInputItemValue->maFunctionData;

    bool bFoundDuplicate = false;

    rInputFunctionData.mnDupCount = 0;
    sal_uInt8 nMaxDuplicateCount = 0;

    std::unique_ptr<weld::TreeIter> xEachEntry(mxControl->make_iterator());
    if (!mxControl->get_iter_first(*xEachEntry))
        return;
    do
    {
        ScItemValue* pItemValue = reinterpret_cast<ScItemValue*>(mxControl->get_id(*xEachEntry).toInt64());
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
    } while (mxControl->iter_next(*xEachEntry));

    if(bFoundDuplicate)
    {
        rInputFunctionData.mnDupCount = nMaxDuplicateCount + 1;
    }
}

IMPL_LINK(ScPivotLayoutTreeListData, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    vcl::KeyCode aCode = rKeyEvent.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    if (nCode == KEY_DELETE)
    {
        int nEntry = mxControl->get_cursor_index();
        if (nEntry != -1)
            mxControl->remove(nEntry);
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
