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
#include <reffact.hxx>
#include <svtools/treelistentry.hxx>

#include "rangeutl.hxx"
#include "uiitems.hxx"
#include "dputil.hxx"

#include <vector>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

using namespace css::uno;
using namespace css::sheet;

ScItemValue::ScItemValue(OUString const & aName, SCCOL nColumn, sal_uInt16 nFunctionMask) :
    maName(aName),
    maFunctionData(nColumn, nFunctionMask),
    mpOriginalItemValue(this)
{}

ScItemValue::ScItemValue(ScItemValue* pInputItemValue) :
    maName(pInputItemValue->maName),
    maFunctionData(pInputItemValue->maFunctionData),
    mpOriginalItemValue(this)
{}

ScItemValue::~ScItemValue()
{}

namespace
{

ScRange lclGetRangeForNamedRange(OUString const & aName, ScDocument* pDocument)
{
    ScRange aInvalidRange(ScAddress::INITIALIZE_INVALID);
    ScRangeName* pRangeName = pDocument->GetRangeName();
    if (pRangeName == NULL)
        return aInvalidRange;

    const ScRangeData* pData = pRangeName->findByUpperName(aName.toAsciiUpperCase());
    if (pData == NULL)
        return aInvalidRange;

    ScRange aRange;
    if (pData->IsReference(aRange))
        return aRange;

    return aInvalidRange;
}

}

ScPivotLayoutDialog::ScPivotLayoutDialog(
                            SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow, vcl::Window* pParent,
                            ScViewData* pViewData, const ScDPObject* pPivotTableObject, bool bNewPivotTable) :
    ScAnyRefDlg           (pSfxBindings, pChildWindow, pParent, "PivotTableLayout", "modules/scalc/ui/pivottablelayoutdialog.ui"),
    maPivotTableObject    (*pPivotTableObject),
    mpPreviouslyFocusedListBox(NULL),
    mpCurrentlyFocusedListBox(NULL),
    mpViewData            (pViewData),
    mpDocument            (pViewData->GetDocument()),
    mbNewPivotTable       (bNewPivotTable),
    mpActiveEdit          (NULL),
    maAddressDetails      (mpDocument->GetAddressConvention(), 0, 0),
    mbDialogLostFocus     (false)
{
    Link aLink;

    get(mpListBoxField,    "listbox-fields");
    get(mpListBoxPage,     "listbox-page");
    get(mpListBoxColumn,   "listbox-column");
    get(mpListBoxRow,      "listbox-row");
    get(mpListBoxData,     "listbox-data");

    get(mpCheckIgnoreEmptyRows,     "check-ignore-empty-rows");
    get(mpCheckTotalColumns,        "check-total-columns");
    get(mpCheckAddFilter,           "check-add-filter");
    get(mpCheckIdentifyCategories,  "check-identify-categories");
    get(mpCheckTotalRows,           "check-total-rows");
    get(mpCheckDrillToDetail,       "check-drill-to-details");

    get(mpBtnOK,  "ok");
    get(mpBtnCancel,  "cancel");

    get(mpSourceRadioNamedRange, "source-radio-named-range");
    get(mpSourceRadioSelection,  "source-radio-selection");
    get(mpSourceListBox,         "source-list");
    get(mpSourceEdit,            "source-edit");
    get(mpSourceButton,          "source-button");

    get(mpDestinationRadioNewSheet,   "destination-radio-new-sheet");
    get(mpDestinationRadioNamedRange, "destination-radio-named-range");
    get(mpDestinationRadioSelection,  "destination-radio-selection");
    get(mpDestinationListBox,         "destination-list");
    get(mpDestinationEdit,            "destination-edit");
    get(mpDestinationButton,          "destination-button");

    // Source UI
    aLink = LINK(this, ScPivotLayoutDialog, ToggleSource);
    mpSourceRadioNamedRange->SetToggleHdl(aLink);
    mpSourceRadioSelection->SetToggleHdl(aLink);

    mpSourceEdit->SetReferences(this, mpSourceRadioSelection);
    mpSourceButton->SetReferences(this, mpSourceEdit);

    aLink = LINK(this, ScPivotLayoutDialog, GetFocusHandler);
    mpSourceEdit->SetGetFocusHdl(aLink);
    mpSourceButton->SetGetFocusHdl(aLink);

    aLink = LINK(this, ScPivotLayoutDialog, LoseFocusHandler);
    mpSourceEdit->SetLoseFocusHdl(aLink);
    mpSourceButton->SetLoseFocusHdl(aLink);

    mpSourceEdit->SetModifyHdl(LINK(this, ScPivotLayoutDialog, SourceEditModified));
    mpSourceListBox->SetSelectHdl(LINK(this, ScPivotLayoutDialog, SourceEditModified));

    // Destination UI
    aLink = LINK(this, ScPivotLayoutDialog, ToggleDestination);
    mpDestinationRadioNewSheet->SetToggleHdl(aLink);
    mpDestinationRadioNamedRange->SetToggleHdl(aLink);
    mpDestinationRadioSelection->SetToggleHdl(aLink);

    mpDestinationEdit->SetReferences(this, mpDestinationRadioNewSheet);
    mpDestinationButton->SetReferences(this, mpDestinationEdit);

    aLink = LINK(this, ScPivotLayoutDialog, GetFocusHandler);
    mpDestinationEdit->SetGetFocusHdl(aLink);
    mpDestinationButton->SetGetFocusHdl(aLink);

    aLink = LINK(this, ScPivotLayoutDialog, LoseFocusHandler);
    mpDestinationEdit->SetLoseFocusHdl(aLink);
    mpDestinationButton->SetLoseFocusHdl(aLink);

    // Buttons
    mpBtnCancel->SetClickHdl(LINK(this, ScPivotLayoutDialog, CancelClicked));
    mpBtnOK->SetClickHdl(LINK(this, ScPivotLayoutDialog, OKClicked));

    // Initialize Data
    maPivotTableObject.FillOldParam(maPivotParameters);
    maPivotTableObject.FillLabelData(maPivotParameters);

    mpListBoxField->Setup (this);
    mpListBoxPage->Setup  (this, ScPivotLayoutTreeList::PAGE_LIST);
    mpListBoxColumn->Setup(this, ScPivotLayoutTreeList::COLUMN_LIST);
    mpListBoxRow->Setup   (this, ScPivotLayoutTreeList::ROW_LIST);
    mpListBoxData->Setup  (this);

    FillValuesToListBoxes();

    // Initialize Options
    const ScDPSaveData* pSaveData = maPivotTableObject.GetSaveData();
    if (pSaveData == NULL)
    {
        mpCheckAddFilter->Check(false);
        mpCheckDrillToDetail->Check(false);
    }
    else
    {
        mpCheckAddFilter->Check(pSaveData->GetFilterButton());
        mpCheckDrillToDetail->Check(pSaveData->GetDrillDown());
    }

    mpCheckIgnoreEmptyRows->Check(maPivotParameters.bIgnoreEmptyRows);
    mpCheckIdentifyCategories->Check(maPivotParameters.bDetectCategories);
    mpCheckTotalColumns->Check(maPivotParameters.bMakeTotalCol);
    mpCheckTotalRows->Check(maPivotParameters.bMakeTotalRow);

    SetupSource();
    SetupDestination();
}

ScPivotLayoutDialog::~ScPivotLayoutDialog()
{}

void ScPivotLayoutDialog::SetupSource()
{
    mpSourceListBox->Clear();

    ScRange aSourceRange;
    OUString sSourceNamedRangeName;

    if (maPivotTableObject.GetSheetDesc())
    {
        const ScSheetSourceDesc* pSheetSourceDesc = maPivotTableObject.GetSheetDesc();
        aSourceRange = pSheetSourceDesc->GetSourceRange();

        if(!aSourceRange.IsValid())
        {
            // Source is probably a DB Range
            mpSourceRadioNamedRange->Disable();
            mpSourceRadioSelection->Disable();
            ToggleSource(NULL);
            return;
        }
        else
        {
            OUString aSourceRangeName = aSourceRange.Format(SCR_ABS_3D, mpDocument, maAddressDetails);
            mpSourceEdit->SetText(aSourceRangeName);
        }
    }
    else
    {
        mpSourceRadioNamedRange->Disable();
        mpSourceRadioSelection->Disable();
        ToggleSource(NULL);
        return;
    }

    // Setup Named Ranges
    bool bIsNamedRange = false;

    ScAreaNameIterator aIterator(mpDocument);
    OUString aEachName;
    ScRange aEachRange;

    while (aIterator.Next(aEachName, aEachRange))
    {
        if (!aIterator.WasDBName())
        {
            mpSourceListBox->InsertEntry(aEachName);
            if (aEachRange == aSourceRange)
            {
                sSourceNamedRangeName = aEachName;
                bIsNamedRange = true;
            }
        }
    }

    if (bIsNamedRange)
    {
        mpSourceListBox->SelectEntry(sSourceNamedRangeName, true);
        mpSourceRadioNamedRange->Check(true);
    }
    else
    {
        mpSourceListBox->SelectEntryPos(0, true);
        mpSourceRadioSelection->Check(true);
    }

    // If entries - select first entry, otherwise disable the radio button.
    if (mpSourceListBox->GetEntryCount() <= 0)
        mpSourceRadioNamedRange->Disable();

    ToggleSource(NULL);
}

void ScPivotLayoutDialog::SetupDestination()
{
    mpDestinationListBox->Clear();

    // Fill up named ranges
    ScAreaNameIterator aIterator(mpDocument);
    OUString aName;
    ScRange aRange;

    while (aIterator.Next(aName, aRange))
    {
        if (!aIterator.WasDBName())
        {
            mpDestinationListBox->InsertEntry(aName);
        }
    }

    // If entries - select first entry, otherwise disable the radio button.
    if (mpDestinationListBox->GetEntryCount() > 0)
        mpDestinationListBox->SelectEntryPos(0, true);
    else
        mpDestinationRadioNamedRange->Disable();

    //
    if (mbNewPivotTable)
    {
        mpDestinationRadioNewSheet->Check(true);
    }
    else
    {
        if (maPivotParameters.nTab != MAXTAB + 1)
        {
            ScAddress aAddress(maPivotParameters.nCol, maPivotParameters.nRow, maPivotParameters.nTab);
            OUString aAddressString = aAddress.Format(SCA_VALID | SCA_TAB_3D | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE, mpDocument, maAddressDetails);
            mpDestinationEdit->SetText(aAddressString);
            mpDestinationRadioSelection->Check(true);
        }
    }

    ToggleDestination(NULL);
}

void ScPivotLayoutDialog::FillValuesToListBoxes()
{
    mpListBoxField->FillLabelFields(maPivotParameters.maLabelArray);
    mpListBoxData->FillDataField(maPivotParameters.maDataFields);
    mpListBoxColumn->FillFields(maPivotParameters.maColFields);
    mpListBoxRow->FillFields(maPivotParameters.maRowFields);
    mpListBoxPage->FillFields(maPivotParameters.maPageFields);
}

void ScPivotLayoutDialog::SetActive()
{
    if (mbDialogLostFocus)
    {
        mbDialogLostFocus = false;
        if(mpActiveEdit != NULL)
        {
            mpActiveEdit->GrabFocus();
            if (mpActiveEdit == mpSourceEdit)
                UpdateSourceRange();
        }
    }
    else
    {
        GrabFocus();
    }

    RefInputDone();
}

void ScPivotLayoutDialog::SetReference(const ScRange& rReferenceRange, ScDocument* pDocument)
{
    if (!mbDialogLostFocus)
        return;

    if (mpActiveEdit == NULL)
        return;

    if (rReferenceRange.aStart != rReferenceRange.aEnd)
        RefInputStart(mpActiveEdit);

    OUString aReferenceString = rReferenceRange.Format(SCR_ABS_3D, pDocument, maAddressDetails);

    if (mpActiveEdit == mpSourceEdit)
    {
        mpSourceEdit->SetRefString(aReferenceString);
    }
    else if (mpActiveEdit == mpDestinationEdit)
    {
        mpDestinationEdit->SetRefString(aReferenceString);
    }
}

bool ScPivotLayoutDialog::IsRefInputMode() const
{
    return mbDialogLostFocus;
}

void ScPivotLayoutDialog::ItemInserted(ScItemValue* pItemValue, ScPivotLayoutTreeList::SvPivotTreeListType eType)
{
    if (pItemValue == NULL)
        return;

    switch (eType)
    {
        case ScPivotLayoutTreeList::ROW_LIST:
        case ScPivotLayoutTreeList::COLUMN_LIST:
        case ScPivotLayoutTreeList::PAGE_LIST:
        {
            mpListBoxRow->RemoveEntryForItem(pItemValue);
            mpListBoxColumn->RemoveEntryForItem(pItemValue);
            mpListBoxPage->RemoveEntryForItem(pItemValue);
        }
        case ScPivotLayoutTreeList::LABEL_LIST:
        {
            mpListBoxRow->RemoveEntryForItem(pItemValue);
            mpListBoxColumn->RemoveEntryForItem(pItemValue);
            mpListBoxPage->RemoveEntryForItem(pItemValue);
            mpListBoxData->RemoveEntryForItem(pItemValue);
        }
        break;
        default:
            break;
    }
}

void ScPivotLayoutDialog::UpdateSourceRange()
{
    ScSheetSourceDesc aSourceSheet = *maPivotTableObject.GetSheetDesc();

    if (mpSourceRadioNamedRange->IsChecked())
    {
        OUString aEntryString = mpSourceListBox->GetSelectEntry();
        ScRange aSourceRange = lclGetRangeForNamedRange(aEntryString, mpDocument);
        if (!aSourceRange.IsValid() || aSourceSheet.GetSourceRange() == aSourceRange)
            return;
        aSourceSheet.SetRangeName(aEntryString);
    }
    else if (mpSourceRadioSelection->IsChecked())
    {
        OUString aSourceString = mpSourceEdit->GetText();
        ScRange aSourceRange;
        sal_uInt16 nResult = aSourceRange.Parse(aSourceString, mpDocument, maAddressDetails);

        bool bIsValid = (nResult & SCA_VALID) == SCA_VALID; // aSourceString is valid

        mpSourceEdit->SetRefValid(true);

        if (bIsValid)
        {
            ScRefAddress aStart;
            ScRefAddress aEnd;

            ConvertDoubleRef(mpDocument, aSourceString, 1, aStart, aEnd, maAddressDetails);
            aSourceRange.aStart = aStart.GetAddress();
            aSourceRange.aEnd = aEnd.GetAddress();
        }
        else
        {
            aSourceRange = lclGetRangeForNamedRange(aSourceString, mpDocument);
        }

        if (!aSourceRange.IsValid())
        {
            mpSourceEdit->SetRefValid(false);
            return;
        }

        if (aSourceSheet.GetSourceRange() == aSourceRange)
                return;

        aSourceSheet.SetSourceRange(aSourceRange);
        if (aSourceSheet.CheckSourceRange() != 0)
        {
            mpSourceEdit->SetRefValid(false);
            return;
        }
    }
    else
    {
        return;
    }

    maPivotTableObject.SetSheetDesc(aSourceSheet);
    maPivotTableObject.FillOldParam(maPivotParameters);
    maPivotTableObject.FillLabelData(maPivotParameters);

    FillValuesToListBoxes();
}

bool ScPivotLayoutDialog::ApplyChanges()
{
    ScDPSaveData aSaveData;
    ApplySaveData(aSaveData);
    ApplyLabelData(aSaveData);

    ScRange aDestinationRange;
    bool bToNewSheet = false;

    if (!GetDestination(aDestinationRange, bToNewSheet))
        return false;

    SetDispatcherLock(false);
    SwitchToDocument();

    sal_uInt16 nWhichPivot = SC_MOD()->GetPool().GetWhich(SID_PIVOT_TABLE);
    ScPivotItem aPivotItem(nWhichPivot, &aSaveData, &aDestinationRange, bToNewSheet);
    mpViewData->GetViewShell()->SetDialogDPObject(&maPivotTableObject);

    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();
    SfxCallMode nCallMode = SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD;
    const SfxPoolItem* pResult = pDispatcher->Execute(SID_PIVOT_TABLE, nCallMode, &aPivotItem, NULL, 0);

    if (pResult != NULL)
    {
        const SfxBoolItem* pItem = reinterpret_cast<const SfxBoolItem*>(pResult);
        if (pItem)
        {
            return pItem->GetValue();
        }
    }

    SetDispatcherLock(true);
    return true;
}

void ScPivotLayoutDialog::ApplySaveData(ScDPSaveData& rSaveData)
{
    rSaveData.SetIgnoreEmptyRows(mpCheckIgnoreEmptyRows->IsChecked());
    rSaveData.SetRepeatIfEmpty(mpCheckIdentifyCategories->IsChecked());
    rSaveData.SetColumnGrand(mpCheckTotalColumns->IsChecked());
    rSaveData.SetRowGrand(mpCheckTotalRows->IsChecked());
    rSaveData.SetFilterButton(mpCheckAddFilter->IsChecked());
    rSaveData.SetDrillDown(mpCheckDrillToDetail->IsChecked());

    Reference<XDimensionsSupplier> xSource = maPivotTableObject.GetSource();

    ScPivotFieldVector aPageFieldVector;
    mpListBoxPage->PushEntriesToPivotFieldVector(aPageFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aPageFieldVector, DataPilotFieldOrientation_PAGE,
                                   xSource, maPivotParameters.maLabelArray);

    ScPivotFieldVector aColFieldVector;
    mpListBoxColumn->PushEntriesToPivotFieldVector(aColFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aColFieldVector, DataPilotFieldOrientation_COLUMN,
                                   xSource, maPivotParameters.maLabelArray);

    ScPivotFieldVector aRowFieldVector;
    mpListBoxRow->PushEntriesToPivotFieldVector(aRowFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aRowFieldVector, DataPilotFieldOrientation_ROW,
                                   xSource, maPivotParameters.maLabelArray);

    ScPivotFieldVector aDataFieldVector;
    mpListBoxData->PushEntriesToPivotFieldVector(aDataFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aDataFieldVector, DataPilotFieldOrientation_DATA,
                                   xSource, maPivotParameters.maLabelArray,
                                   &aColFieldVector, &aRowFieldVector, &aPageFieldVector);
}

void ScPivotLayoutDialog::ApplyLabelData(ScDPSaveData& rSaveData)
{
    ScDPLabelDataVector::const_iterator it;
    ScDPLabelDataVector& rLabelDataVector = GetLabelDataVector();

    for (it = rLabelDataVector.begin(); it != rLabelDataVector.end(); ++it)
    {
        const ScDPLabelData& pLabelData = *it;

        OUString aUnoName = ScDPUtil::createDuplicateDimensionName(pLabelData.maName, pLabelData.mnDupCount);
        ScDPSaveDimension* pSaveDimensions = rSaveData.GetExistingDimensionByName(aUnoName);

        if (pSaveDimensions == NULL)
            continue;

        pSaveDimensions->SetUsedHierarchy(pLabelData.mnUsedHier);
        pSaveDimensions->SetShowEmpty(pLabelData.mbShowAll);
        pSaveDimensions->SetSortInfo(&pLabelData.maSortInfo);
        pSaveDimensions->SetLayoutInfo(&pLabelData.maLayoutInfo);
        pSaveDimensions->SetAutoShowInfo(&pLabelData.maShowInfo);

        bool bManualSort = (pLabelData.maSortInfo.Mode == DataPilotFieldSortMode::MANUAL);

        std::vector<ScDPLabelData::Member>::const_iterator itMember;
        for (itMember = pLabelData.maMembers.begin(); itMember != pLabelData.maMembers.end(); ++itMember)
        {
            const ScDPLabelData::Member& rLabelMember = *itMember;
            ScDPSaveMember* pMember = pSaveDimensions->GetMemberByName(rLabelMember.maName);

            if (bManualSort || !rLabelMember.mbVisible || !rLabelMember.mbShowDetails)
            {
                pMember->SetIsVisible(rLabelMember.mbVisible);
                pMember->SetShowDetails(rLabelMember.mbShowDetails);
            }
        }
    }
}

bool ScPivotLayoutDialog::GetDestination(ScRange& aDestinationRange, bool& bToNewSheet)
{
    bToNewSheet = false;

    if (mpDestinationRadioNamedRange->IsChecked())
    {
        OUString aName = mpDestinationListBox->GetSelectEntry();
        aDestinationRange = lclGetRangeForNamedRange(aName, mpDocument);
        if (!aDestinationRange.IsValid())
            return false;
    }
    else if (mpDestinationRadioSelection->IsChecked())
    {
        ScAddress aAddress;
        aAddress.Parse(mpDestinationEdit->GetText(), mpDocument, maAddressDetails);
        aDestinationRange = ScRange(aAddress);
    }
    else
    {
        bToNewSheet = true;
        aDestinationRange = ScRange(maPivotParameters.nCol, maPivotParameters.nRow, maPivotParameters.nTab);
    }
    return true;
}

ScItemValue* ScPivotLayoutDialog::GetItem(SCCOL nColumn)
{
    return mpListBoxField->GetItem(nColumn);
}

bool ScPivotLayoutDialog::IsDataElement(SCCOL nColumn)
{
    return mpListBoxField->IsDataElement(nColumn);
}

ScDPLabelData* ScPivotLayoutDialog::GetLabelData(SCCOL nColumn)
{
    return &maPivotParameters.maLabelArray[nColumn];
}

void ScPivotLayoutDialog::PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames)
{
    return mpListBoxData->PushDataFieldNames(rDataFieldNames);
}

IMPL_LINK( ScPivotLayoutDialog, OKClicked, PushButton*, /*pButton*/ )
{
    ApplyChanges();
    DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
    return 0;
}

IMPL_LINK( ScPivotLayoutDialog, CancelClicked, PushButton*, /*pButton*/ )
{
    DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
    return 0;
}

IMPL_LINK(ScPivotLayoutDialog, GetFocusHandler, Control*, pCtrl)
{
    mpActiveEdit = NULL;

    if (pCtrl == (Control*) mpSourceEdit  ||
        pCtrl == (Control*) mpSourceButton)
    {
        mpActiveEdit = mpSourceEdit;
    }
    else if (pCtrl == (Control*) mpDestinationEdit  ||
             pCtrl == (Control*) mpDestinationButton)
    {
        mpActiveEdit = mpDestinationEdit;
    }

    if (mpActiveEdit)
        mpActiveEdit->SetSelection(Selection(0, SELECTION_MAX));

    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, LoseFocusHandler)
{
    mbDialogLostFocus = !IsActive();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, SourceEditModified)
{
    UpdateSourceRange();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, ToggleSource)
{
    bool bNamedRange = mpSourceRadioNamedRange->IsChecked();
    bool bSelection = mpSourceRadioSelection->IsChecked();
    mpSourceListBox->Enable(bNamedRange);
    mpSourceButton->Enable(bSelection);
    mpSourceEdit->Enable(bSelection);
    UpdateSourceRange();
    return 0;
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, ToggleDestination)
{
    bool bNamedRange = mpDestinationRadioNamedRange->IsChecked();
    bool bSelection = mpDestinationRadioSelection->IsChecked();
    mpDestinationListBox->Enable(bNamedRange);
    mpDestinationButton->Enable(bSelection);
    mpDestinationEdit->Enable(bSelection);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
