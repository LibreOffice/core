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

#include <PivotCalcFieldDialog.hxx>
#include <dbfunc.hxx>
#include <document.hxx>
#include <dbdocfun.hxx>
#include <global.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <dputil.hxx>
#include <dpsave.hxx>
#include <compiler.hxx>
#include <tokenarray.hxx>

#include <o3tl/string_view.hxx>
#include <formula/errorcodes.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/TreeView.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

using namespace css::uno;
using namespace css::sheet;

ScPivotCalcFieldDlg::ScPivotCalcFieldDlg(weld::Window* pParent, ScViewData& rData,
                                         const ScDPObject* pDPObj)
    : GenericDialogController(pParent, u"modules/scalc/ui/pivotcalcfielddialog.ui"_ustr,
                              u"PivotCalcField"_ustr)
    , mrViewData(rData)
    , mrDocument(mrViewData.GetDocument())
    , maPivotTableObject(*pDPObj)
    , mxCalcNames(m_xBuilder->weld_combo_box(u"calcfieldscb"_ustr))
    , mxCalculation(m_xBuilder->weld_entry(u"calcentry"_ustr))
    , mxBtnAdd(m_xBuilder->weld_button(u"addbtn"_ustr))
    , mxBtnMod(m_xBuilder->weld_button(u"modbtn"_ustr))
    , mxBtnDel(m_xBuilder->weld_button(u"deletebtn"_ustr))
    , mxFieldList(m_xBuilder->weld_tree_view(u"listbox-fields"_ustr))
    , mxBtnInsert(m_xBuilder->weld_button(u"insertbtn"_ustr))
    , mxBtnOK(m_xBuilder->weld_button(u"ok"_ustr))
    , mxBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
{
    Init();
}

ScPivotCalcFieldDlg::~ScPivotCalcFieldDlg()
{
    if (m_xInfoBox)
        m_xInfoBox->response(RET_OK);
}

void ScPivotCalcFieldDlg::Init()
{
    // Buttons
    mxBtnCancel->connect_clicked(LINK(this, ScPivotCalcFieldDlg, CancelClicked));
    mxBtnOK->connect_clicked(LINK(this, ScPivotCalcFieldDlg, OKClicked));
    mxBtnInsert->connect_clicked(LINK(this, ScPivotCalcFieldDlg, InsertClicked));
    mxBtnAdd->connect_clicked(LINK(this, ScPivotCalcFieldDlg, AddModClicked));
    mxBtnDel->connect_clicked(LINK(this, ScPivotCalcFieldDlg, DeleteClicked));
    mxFieldList->connect_row_activated(LINK(this, ScPivotCalcFieldDlg, DoubleClickHdl));
    mxFieldList->connect_selection_changed(LINK(this, ScPivotCalcFieldDlg, FieldListSelected));
    mxCalcNames->connect_changed(LINK(this, ScPivotCalcFieldDlg, CalcFieldNameSelected));
    mxCalculation->connect_changed(LINK(this, ScPivotCalcFieldDlg, CalcEntryChanged));

    // Initialize Data
    maPivotTableObject.FillOldParam(maPivotParameters);
    maPivotTableObject.FillLabelData(maPivotParameters);
    // Fill the list view and combobox with existing field names
    FillAllFields();
    // Init new dummy field name
    AddDummyNewField();
    aStrAdd = mxBtnAdd->get_label();
    aStrModify = mxBtnMod->get_label();
}

void ScPivotCalcFieldDlg::ErrorBox(const OUString& rString)
{
    if (m_xInfoBox)
        m_xInfoBox->response(RET_OK);

    m_xInfoBox = std::shared_ptr<weld::MessageDialog>(Application::CreateMessageDialog(
        m_xDialog.get(), VclMessageType::Warning, VclButtonsType::Ok, rString));

    m_xInfoBox->runAsync(m_xInfoBox, [this](sal_Int32) { m_xInfoBox = nullptr; });
}

bool ScPivotCalcFieldDlg::IsExistingField(std::u16string_view rName) const
{
    for (const auto& pLabelData : maPivotParameters.maLabelArray)
    {
        if (!pLabelData->mbCalculatedField && pLabelData->maName == rName)
            return true;
    }
    return false;
}

std::shared_ptr<ScTokenArray> ScPivotCalcFieldDlg::ValidateFormula(const OUString& rFormula,
                                                                   FormulaError* pError)
{
    ScAddress aAddr(ScAddress::INITIALIZE_INVALID);
    ScCompiler aComp(mrDocument, aAddr, mrDocument.GetGrammar(), true, false);
    for (const auto& pLabels : maPivotParameters.maLabelArray)
    {
        if (!pLabels->mbDataLayout)
            aComp.SetAvailablePivotFields(pLabels->maName);
    }

    std::shared_ptr<ScTokenArray> pArrayRef(aComp.CompileString(rFormula));
    if (pArrayRef && pArrayRef->GetCodeError() == FormulaError::NONE && pArrayRef->GetLen())
        aComp.CompileTokenArray(); // Generate RPN tokens

    // Check for errors from either CompileString or CompileTokenArray
    // TODO: more warnings can be useful to the user, e.g. use of undefined fields,
    // circular references, etc., but for now just check for errors and empty result.
    if (!pArrayRef || pArrayRef->GetCodeError() != FormulaError::NONE || !pArrayRef->GetLen())
    {
        if (pError)
            *pError = (pArrayRef && pArrayRef->GetCodeError() != FormulaError::NONE)
                          ? pArrayRef->GetCodeError()
                          : FormulaError::NoCode;
        pArrayRef.reset();
        return nullptr;
    }

    return pArrayRef;
}

void ScPivotCalcFieldDlg::FillAllFields()
{
    mxFieldList->clear();
    mxCalcNames->clear();
    mxBtnInsert->set_sensitive(false);

    for (std::unique_ptr<ScDPLabelData> const& pLabelData : maPivotParameters.maLabelArray)
    {
        OUString aColId = OUString::number(pLabelData->mnCol);

        if (pLabelData->mnOriginalDim < 0 && !pLabelData->mbDataLayout)
            mxFieldList->append(aColId, pLabelData->maName);

        if (pLabelData->mbCalculatedField)
            mxCalcNames->append(aColId, pLabelData->maName);
    }
}

void ScPivotCalcFieldDlg::AddDummyNewField()
{
    OUString aBaseName = ScResId(STR_PIVOT_FIELD);
    SCCOL nCol = 1;
    bool bFound = true;
    while (bFound)
    {
        bFound = false;
        OUString aCandidate = aBaseName + OUString::number(nCol);
        for (const auto& pLabelData : maPivotParameters.maLabelArray)
        {
            if (pLabelData->maName.equalsIgnoreAsciiCase(aCandidate))
            {
                bFound = true;
                nCol++;
                break;
            }
        }
    }
    mxCalculation->set_text(u"= 0"_ustr);
    mxCalcNames->set_entry_text(aBaseName + OUString::number(nCol));
    mxCalcNames->select_entry_region(0, -1);
    mxBtnDel->set_sensitive(false);
}

void ScPivotCalcFieldDlg::InsertSelectedField()
{
    int nEntry = mxFieldList->get_cursor_index();
    if (nEntry == -1)
        return;
    OUString aFieldName = mxFieldList->get_text(nEntry);
    if (aFieldName.indexOf(' ') >= 0)
        aFieldName = "'" + aFieldName + "'";

    // If the formula is just a placeholder like "= 0", "=0", "0", replace it
    OUString aCurrent = mxCalculation->get_text().trim();
    if (aCurrent.startsWith("="))
        aCurrent = OUString(o3tl::trim(aCurrent.subView(1)));
    if (aCurrent == "0")
        mxCalculation->set_text(u"= "_ustr + aFieldName);
    else
        mxCalculation->set_text(mxCalculation->get_text() + u" "_ustr + aFieldName);
    mxCalculation->grab_focus();
    mxCalculation->set_position(-1);
}

void ScPivotCalcFieldDlg::RemoveDeletedFieldsFromCache()
{
    for (const OUString& rName : maDeletedFieldNames)
        maPivotTableObject.RemoveCalculatedFieldFromCache(rName);
    maDeletedFieldNames.clear();
}

void ScPivotCalcFieldDlg::CompileCalcFieldFormulas()
{
    for (const auto& pLabelData : maPivotParameters.maLabelArray)
    {
        if (!pLabelData->mbCalculatedField || !pLabelData->maCalculation)
            continue;

        // Use pre-compiled token array from DoAddMod if available,
        // otherwise compile now (for fields that existed before the dialog opened).
        std::shared_ptr<ScTokenArray> pArrayRef = pLabelData->mpCompiledFormula;
        if (!pArrayRef)
            pArrayRef = ValidateFormula(pLabelData->maCalculation.value());
        if (pArrayRef)
        {
            maPivotTableObject.InsertCalculatedFieldToCache(pLabelData->mnCol, pLabelData->maName,
                                                            pArrayRef);
        }
    }
}

void ScPivotCalcFieldDlg::ApplyChanges()
{
    // Remove deleted calculated fields from the cache first, before
    // re-inserting the surviving fields with CompileCalcFieldFormulas.
    RemoveDeletedFieldsFromCache();

    // Compile calculated field formulas and register them in the cache before
    // GetSource(), so that new dimensions are available for ConvertOrientation.
    CompileCalcFieldFormulas();

    ScDPSaveData aSaveData;
    ApplySaveData(aSaveData);
    ApplyLabelData(aSaveData);

    // Find the existing pivot table in the document
    ScDPObject* pOldObj = mrDocument.GetDPAtCursor(maPivotParameters.nCol, maPivotParameters.nRow,
                                                   maPivotParameters.nTab);
    if (!pOldObj)
        return;

    // Preserve custom grouping and calculated field data from the existing
    // pivot table (same pattern as ScDBFunc::MakePivotTable in dbfunc3.cxx).
    if (!aSaveData.GetExistingDimensionData() || !aSaveData.GetExistingDimCalcData())
    {
        const ScDPSaveData* pOldSaveData = pOldObj->GetSaveData();
        if (pOldSaveData)
        {
            if (!aSaveData.GetExistingDimensionData())
            {
                const ScDPDimensionSaveData* pDimSave = pOldSaveData->GetExistingDimensionData();
                aSaveData.SetDimensionData(pDimSave);
            }
            if (!aSaveData.GetExistingDimCalcData())
            {
                const ScDPDimCalcSaveData* pDimCalcData = pOldSaveData->GetExistingDimCalcData();
                aSaveData.SetDimCalcData(pDimCalcData);
            }
        }
    }

    // Build new DPObject with the updated save data
    ScDPObject aNewObj(maPivotTableObject);
    aNewObj.SetSaveData(aSaveData);

    // Update directly without undo recording — calculated field changes
    // modify the cache which cannot be undone (same behaviour as Excel).
    ScDBDocFunc aFunc(*mrViewData.GetDocShell());
    aFunc.DataPilotUpdate(pOldObj, &aNewObj, false, false);
    mrViewData.GetView()->CursorPosChanged();
}

void ScPivotCalcFieldDlg::ApplySaveData(ScDPSaveData& rSaveData)
{
    // Preserve existing pivot table settings
    const ScDPSaveData* pOldSaveData = maPivotTableObject.GetSaveData();
    if (pOldSaveData)
    {
        rSaveData.SetIgnoreEmptyRows(pOldSaveData->GetIgnoreEmptyRows());
        rSaveData.SetRepeatIfEmpty(pOldSaveData->GetRepeatIfEmpty());
        rSaveData.SetColumnGrand(pOldSaveData->GetColumnGrand());
        rSaveData.SetRowGrand(pOldSaveData->GetRowGrand());
        rSaveData.SetFilterButton(pOldSaveData->GetFilterButton());
        rSaveData.SetDrillDown(pOldSaveData->GetDrillDown());
        rSaveData.SetExpandCollapse(pOldSaveData->GetExpandCollapse());
    }

    Reference<XDimensionsSupplier> xSource = maPivotTableObject.GetSource();

    ScDPObject::ConvertOrientation(rSaveData, maPivotParameters.maPageFields,
                                   DataPilotFieldOrientation_PAGE, xSource,
                                   maPivotParameters.maLabelArray);

    ScDPObject::ConvertOrientation(rSaveData, maPivotParameters.maColFields,
                                   DataPilotFieldOrientation_COLUMN, xSource,
                                   maPivotParameters.maLabelArray);

    ScDPObject::ConvertOrientation(rSaveData, maPivotParameters.maRowFields,
                                   DataPilotFieldOrientation_ROW, xSource,
                                   maPivotParameters.maLabelArray);

    ScDPObject::ConvertOrientation(rSaveData, maPivotParameters.maDataFields,
                                   DataPilotFieldOrientation_DATA, xSource,
                                   maPivotParameters.maLabelArray, &maPivotParameters.maColFields,
                                   &maPivotParameters.maRowFields, &maPivotParameters.maPageFields);
}

void ScPivotCalcFieldDlg::ApplyLabelData(const ScDPSaveData& rSaveData)
{
    for (std::unique_ptr<ScDPLabelData> const& pLabelData : maPivotParameters.maLabelArray)
    {
        OUString aUnoName
            = ScDPUtil::createDuplicateDimensionName(pLabelData->maName, pLabelData->mnDupCount);

        ScDPSaveDimension* pSaveDimensions = rSaveData.GetExistingDimensionByName(aUnoName);
        if (pSaveDimensions == nullptr)
            continue;

        pSaveDimensions->SetUsedHierarchy(pLabelData->mnUsedHier);
        pSaveDimensions->SetShowEmpty(pLabelData->mbShowAll);
        pSaveDimensions->SetRepeatItemLabels(pLabelData->mbRepeatItemLabels);
        pSaveDimensions->SetSortInfo(&pLabelData->maSortInfo);
        pSaveDimensions->SetLayoutInfo(&pLabelData->maLayoutInfo);
        pSaveDimensions->SetAutoShowInfo(&pLabelData->maShowInfo);

        bool bManualSort = (pLabelData->maSortInfo.Mode == DataPilotFieldSortMode::MANUAL);

        for (ScDPLabelData::Member const& rLabelMember : pLabelData->maMembers)
        {
            ScDPSaveMember* pMember = pSaveDimensions->GetMemberByName(rLabelMember.maName);

            if (bManualSort || !rLabelMember.mbVisible || !rLabelMember.mbShowDetails)
            {
                pMember->SetIsVisible(rLabelMember.mbVisible);
                pMember->SetShowDetails(rLabelMember.mbShowDetails);
            }
        }
    }
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, CancelClicked, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, OKClicked, weld::Button&, void)
{
    // Reject names that conflict with existing non-calculated fields
    OUString aFieldName = mxCalcNames->get_active_text();
    if (!aFieldName.isEmpty() && IsExistingField(aFieldName))
    {
        ErrorBox(ScResId(STR_PIVOT_CALC_FIELD_NAME_ERROR));
        return;
    }

    if (mxBtnAdd->get_sensitive())
    {
        if (!DoAddMod())
            return;
    }

    ApplyChanges();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, InsertClicked, weld::Button&, void) { InsertSelectedField(); }

bool ScPivotCalcFieldDlg::DoAddMod()
{
    // Validate the formula before adding or modifying
    FormulaError eError = FormulaError::NONE;
    OUString aCalculation = mxCalculation->get_text();
    std::shared_ptr<ScTokenArray> pTokenArray = ValidateFormula(aCalculation, &eError);
    if (!pTokenArray)
    {
        ErrorBox(ScGlobal::GetLongErrorString(eError));
        return false;
    }

    bool bModify = (mxBtnAdd->get_label() == aStrModify);
    OUString aFieldName = mxCalcNames->get_active_text();
    if (bModify)
    {
        // Modify existing calculated field: find it by name and update its formula
        for (auto& pLabelData : maPivotParameters.maLabelArray)
        {
            if (pLabelData->mbCalculatedField && pLabelData->maName == aFieldName)
            {
                pLabelData->maCalculation = aCalculation;
                pLabelData->mpCompiledFormula = pTokenArray;
                break;
            }
        }
    }
    else
    {
        // Find DataLayout position — insert new field just before it
        size_t nInsertPos = maPivotParameters.maLabelArray.size();
        for (size_t i = 0; i < maPivotParameters.maLabelArray.size(); ++i)
        {
            if (maPivotParameters.maLabelArray[i]->mbDataLayout)
            {
                nInsertPos = i;
                break;
            }
        }

        // Create new ScDPLabelData for the calculated field
        auto pNewLabel = std::make_unique<ScDPLabelData>();
        pNewLabel->maName = aFieldName;
        pNewLabel->mnCol = static_cast<SCCOL>(nInsertPos);
        pNewLabel->mnOriginalDim = -1;
        pNewLabel->mbCalculatedField = true;
        pNewLabel->maCalculation = aCalculation;
        pNewLabel->mpCompiledFormula = pTokenArray;

        // Insert before DataLayout
        maPivotParameters.maLabelArray.insert(maPivotParameters.maLabelArray.begin() + nInsertPos,
                                              std::move(pNewLabel));

        // Update mnCol for all entries after the insertion point (they shifted by 1)
        for (size_t i = nInsertPos + 1; i < maPivotParameters.maLabelArray.size(); ++i)
            maPivotParameters.maLabelArray[i]->mnCol = static_cast<SCCOL>(i);

        // Update nCol references in pivot field vectors for the shifted dimensions
        auto updateFieldRefs = [nInsertPos](ScPivotFieldVector& rFields) {
            for (auto& rField : rFields)
            {
                if (rField.nCol >= static_cast<SCCOL>(nInsertPos))
                    ++rField.nCol;
            }
        };
        updateFieldRefs(maPivotParameters.maPageFields);
        updateFieldRefs(maPivotParameters.maColFields);
        updateFieldRefs(maPivotParameters.maRowFields);
        updateFieldRefs(maPivotParameters.maDataFields);

        // Add new field to the data fields (calculated fields go to the Values area)
        ScPivotField aNewDataField(static_cast<SCCOL>(nInsertPos));
        aNewDataField.nFuncMask = PivotFunc::Sum;
        maPivotParameters.maDataFields.push_back(aNewDataField);

        // If this name was previously deleted in this session, un-track it
        // so we don't remove it from the cache on OK.
        std::erase(maDeletedFieldNames, aFieldName);
    }

    FillAllFields();

    // Select the field we just added/modified in the combo box
    int nIdx = mxCalcNames->find_text(aFieldName);
    if (nIdx != -1)
        mxCalcNames->set_active(nIdx);

    mxBtnAdd->set_sensitive(false);
    mxBtnDel->set_sensitive(true);
    mxBtnOK->set_can_focus(true);
    return true;
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, AddModClicked, weld::Button&, void) { DoAddMod(); }

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, DeleteClicked, weld::Button&, void)
{
    OUString aFieldName = mxCalcNames->get_active_text();
    if (aFieldName.isEmpty())
        return;

    // Find the calculated field in maLabelArray
    size_t nRemovePos = 0;
    bool bFound = false;
    for (size_t i = 0; i < maPivotParameters.maLabelArray.size(); ++i)
    {
        auto& pLabelData = maPivotParameters.maLabelArray[i];
        if (pLabelData->mbCalculatedField && pLabelData->maName == aFieldName)
        {
            nRemovePos = i;
            bFound = true;
            break;
        }
    }

    if (!bFound)
        return;

    // Track this deletion so we can remove it from the cache on OK
    maDeletedFieldNames.push_back(aFieldName);

    // Remove from maLabelArray
    maPivotParameters.maLabelArray.erase(maPivotParameters.maLabelArray.begin() + nRemovePos);

    // Update mnCol for all entries after the removal point (they shifted down by 1)
    for (size_t i = nRemovePos; i < maPivotParameters.maLabelArray.size(); ++i)
        maPivotParameters.maLabelArray[i]->mnCol = static_cast<SCCOL>(i);

    // Remove from maDataFields and update nCol references in pivot field vectors
    auto removeAndUpdate = [nRemovePos](ScPivotFieldVector& rFields) {
        std::erase_if(rFields, [nRemovePos](const ScPivotField& rField) {
            return rField.nCol == static_cast<SCCOL>(nRemovePos);
        });
        for (auto& rField : rFields)
        {
            if (rField.nCol > static_cast<SCCOL>(nRemovePos))
                --rField.nCol;
        }
    };
    removeAndUpdate(maPivotParameters.maPageFields);
    removeAndUpdate(maPivotParameters.maColFields);
    removeAndUpdate(maPivotParameters.maRowFields);
    removeAndUpdate(maPivotParameters.maDataFields);

    FillAllFields();

    // Clear the combo box and formula entry, disable Add/Del
    mxCalcNames->set_entry_text(u""_ustr);
    mxCalculation->set_text(u"= 0"_ustr);
    mxBtnAdd->set_sensitive(false);
    mxBtnDel->set_sensitive(false);
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, DoubleClickHdl, weld::TreeView&, bool)
{
    InsertSelectedField();
    return true;
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, FieldListSelected, weld::TreeView&, void)
{
    mxBtnInsert->set_sensitive(mxFieldList->get_cursor_index() != -1);
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, CalcFieldNameSelected, weld::ComboBox&, void)
{
    OUString theName = mxCalcNames->get_active_text();
    if (theName.isEmpty())
    {
        if (mxBtnAdd->get_label() != aStrAdd)
            mxBtnAdd->set_label(aStrAdd);
        mxBtnAdd->set_sensitive(false);
        mxBtnDel->set_sensitive(false);
    }
    else
    {
        int nIdx = mxCalcNames->find_text(theName);
        if (nIdx != -1)
        {
            if (mxBtnAdd->get_label() != aStrModify)
                mxBtnAdd->set_label(aStrModify);
            mxBtnAdd->set_sensitive(true);
            mxBtnDel->set_sensitive(true);

            SCCOL nCol = static_cast<SCCOL>(mxCalcNames->get_id(nIdx).toInt32());
            ScDPLabelData& rLabelData = *maPivotParameters.maLabelArray[nCol];
            if (rLabelData.maCalculation)
            {
                OUString aFormula = rLabelData.maCalculation.value().trim();
                mxCalculation->set_text(aFormula);
            }
        }
        else
        {
            if (mxBtnAdd->get_label() != aStrAdd)
                mxBtnAdd->set_label(aStrAdd);

            mxBtnDel->set_sensitive(false);
            mxBtnAdd->set_sensitive(!IsExistingField(theName));
        }
    }
}

IMPL_LINK_NOARG(ScPivotCalcFieldDlg, CalcEntryChanged, weld::Entry&, void)
{
    OUString aName = mxCalcNames->get_active_text();
    if (!aName.isEmpty() && !IsExistingField(aName))
        mxBtnAdd->set_sensitive(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
