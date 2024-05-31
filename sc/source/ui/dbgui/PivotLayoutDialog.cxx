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

#include <PivotLayoutTreeList.hxx>
#include <PivotLayoutDialog.hxx>
#include <reffact.hxx>

#include <rangeutl.hxx>
#include <uiitems.hxx>
#include <dputil.hxx>
#include <dbdocfun.hxx>
#include <dpsave.hxx>
#include <dpshttab.hxx>
#include <scmod.hxx>

#include <memory>
#include <utility>
#include <vector>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

using namespace css::uno;
using namespace css::sheet;

ScItemValue::ScItemValue(OUString aName, SCCOL nColumn, PivotFunc nFunctionMask) :
    maName(std::move(aName)),
    maFunctionData(nColumn, nFunctionMask),
    mpOriginalItemValue(this)
{}

ScItemValue::ScItemValue(const ScItemValue* pInputItemValue) :
    maName(pInputItemValue->maName),
    maFunctionData(pInputItemValue->maFunctionData),
    mpOriginalItemValue(this)
{}

ScItemValue::~ScItemValue()
{}

namespace
{

ScRange lclGetRangeForNamedRange(OUString const & aName, const ScDocument& rDocument)
{
    ScRange aInvalidRange(ScAddress::INITIALIZE_INVALID);
    ScRangeName* pRangeName = rDocument.GetRangeName();
    if (pRangeName == nullptr)
        return aInvalidRange;

    const ScRangeData* pData = pRangeName->findByUpperName(aName.toAsciiUpperCase());
    if (pData == nullptr)
        return aInvalidRange;

    ScRange aRange;
    if (pData->IsReference(aRange))
        return aRange;

    return aInvalidRange;
}

}

ScPivotLayoutDialog::ScPivotLayoutDialog(
                            SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow, weld::Window* pParent,
                            ScViewData* pViewData, const ScDPObject* pPivotTableObject, bool bNewPivotTable)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent, "modules/scalc/ui/pivottablelayoutdialog.ui", "PivotTableLayout")
    , maPivotTableObject(*pPivotTableObject)
    , mpPreviouslyFocusedListBox(nullptr)
    , mpViewData(pViewData)
    , mrDocument(pViewData->GetDocument())
    , mbNewPivotTable(bNewPivotTable)
    , maAddressDetails(mrDocument.GetAddressConvention(), 0, 0)
    , mbDialogLostFocus(false)
    , mpActiveEdit(nullptr)
    , mxListBoxField(new ScPivotLayoutTreeListLabel(m_xBuilder->weld_tree_view("listbox-fields")))
    , mxListBoxPage(new ScPivotLayoutTreeList(m_xBuilder->weld_tree_view("listbox-page")))
    , mxListBoxColumn(new ScPivotLayoutTreeList(m_xBuilder->weld_tree_view("listbox-column")))
    , mxListBoxRow(new ScPivotLayoutTreeList(m_xBuilder->weld_tree_view("listbox-row")))
    , mxListBoxData(new ScPivotLayoutTreeListData(m_xBuilder->weld_tree_view("listbox-data")))
    , mxCheckIgnoreEmptyRows(m_xBuilder->weld_check_button("check-ignore-empty-rows"))
    , mxCheckTotalColumns(m_xBuilder->weld_check_button("check-total-columns"))
    , mxCheckAddFilter(m_xBuilder->weld_check_button("check-add-filter"))
    , mxCheckIdentifyCategories(m_xBuilder->weld_check_button("check-identify-categories"))
    , mxCheckTotalRows(m_xBuilder->weld_check_button("check-total-rows"))
    , mxCheckDrillToDetail(m_xBuilder->weld_check_button("check-drill-to-details"))
    , mxCheckExpandCollapse(m_xBuilder->weld_check_button("check-show-expand-collapse"))
    , mxSourceRadioNamedRange(m_xBuilder->weld_radio_button("source-radio-named-range"))
    , mxSourceRadioSelection(m_xBuilder->weld_radio_button("source-radio-selection"))
    , mxSourceListBox(m_xBuilder->weld_combo_box("source-list"))
    , mxSourceEdit(new formula::RefEdit(m_xBuilder->weld_entry("source-edit")))
    , mxSourceButton(new formula::RefButton(m_xBuilder->weld_button("source-button")))
    , mxDestinationRadioNewSheet(m_xBuilder->weld_radio_button("destination-radio-new-sheet"))
    , mxDestinationRadioNamedRange(m_xBuilder->weld_radio_button("destination-radio-named-range"))
    , mxDestinationRadioSelection(m_xBuilder->weld_radio_button("destination-radio-selection"))
    , mxDestinationListBox(m_xBuilder->weld_combo_box("destination-list"))
    , mxDestinationEdit(new formula::RefEdit(m_xBuilder->weld_entry("destination-edit")))
    , mxDestinationButton(new formula::RefButton(m_xBuilder->weld_button("destination-button")))
    , mxBtnOK(m_xBuilder->weld_button("ok"))
    , mxBtnCancel(m_xBuilder->weld_button("cancel"))
    , mxSourceFrame(m_xBuilder->weld_frame("frame2"))
    , mxSourceLabel(mxSourceFrame->weld_label_widget())
    , mxDestFrame(m_xBuilder->weld_frame("frame1"))
    , mxDestLabel(mxDestFrame->weld_label_widget())
    , mxOptions(m_xBuilder->weld_expander("options"))
    , mxMore(m_xBuilder->weld_expander("more"))
{
    // Source UI
    Link<weld::Toggleable&,void> aLink2 = LINK(this, ScPivotLayoutDialog, ToggleSource);
    mxSourceRadioNamedRange->connect_toggled(aLink2);
    mxSourceRadioSelection->connect_toggled(aLink2);

    mxSourceEdit->SetReferences(this, mxSourceLabel.get());
    mxSourceButton->SetReferences(this, mxSourceEdit.get());

    Link<formula::RefEdit&,void> aEditLink = LINK(this, ScPivotLayoutDialog, GetEditFocusHandler);
    mxDestinationEdit->SetGetFocusHdl(aEditLink);
    mxSourceEdit->SetGetFocusHdl(aEditLink);

    aEditLink = LINK(this, ScPivotLayoutDialog, LoseEditFocusHandler);
    mxDestinationEdit->SetLoseFocusHdl(aEditLink);
    mxSourceEdit->SetLoseFocusHdl(aEditLink);

    mxSourceEdit->SetModifyHdl(LINK(this, ScPivotLayoutDialog, SourceEditModified));
    mxSourceListBox->connect_changed(LINK(this, ScPivotLayoutDialog, SourceListSelected));

    // Destination UI
    aLink2 = LINK(this, ScPivotLayoutDialog, ToggleDestination);
    mxDestinationRadioNewSheet->connect_toggled(aLink2);
    mxDestinationRadioNamedRange->connect_toggled(aLink2);
    mxDestinationRadioSelection->connect_toggled(aLink2);

    mxDestinationEdit->SetReferences(this, mxDestLabel.get());
    mxDestinationButton->SetReferences(this, mxDestinationEdit.get());

    Link<formula::RefButton&,void> aButtonLink = LINK(this, ScPivotLayoutDialog, GetButtonFocusHandler);
    mxSourceButton->SetGetFocusHdl(aButtonLink);
    mxDestinationButton->SetGetFocusHdl(aButtonLink);

    aButtonLink = LINK(this, ScPivotLayoutDialog, LoseButtonFocusHandler);
    mxSourceButton->SetLoseFocusHdl(aButtonLink);
    mxDestinationButton->SetLoseFocusHdl(aButtonLink);

    // Buttons
    mxBtnCancel->connect_clicked(LINK(this, ScPivotLayoutDialog, CancelClicked));
    mxBtnOK->connect_clicked(LINK(this, ScPivotLayoutDialog, OKClicked));

    // Initialize Data
    maPivotTableObject.FillOldParam(maPivotParameters);
    maPivotTableObject.FillLabelData(maPivotParameters);

    mxListBoxField->Setup (this);
    mxListBoxPage->Setup  (this, ScPivotLayoutTreeList::PAGE_LIST);
    mxListBoxColumn->Setup(this, ScPivotLayoutTreeList::COLUMN_LIST);
    mxListBoxRow->Setup   (this, ScPivotLayoutTreeList::ROW_LIST);
    mxListBoxData->Setup  (this);

    FillValuesToListBoxes();

    // Initialize Options
    const ScDPSaveData* pSaveData = maPivotTableObject.GetSaveData();
    if (pSaveData == nullptr)
    {
        mxCheckAddFilter->set_active(false);
        mxCheckDrillToDetail->set_active(false);
        mxCheckExpandCollapse->set_active(false);
    }
    else
    {
        mxCheckAddFilter->set_active(pSaveData->GetFilterButton());
        mxCheckDrillToDetail->set_active(pSaveData->GetDrillDown());
        mxCheckExpandCollapse->set_active(pSaveData->GetExpandCollapse());
    }

    mxCheckIgnoreEmptyRows->set_active(maPivotParameters.bIgnoreEmptyRows);
    mxCheckIdentifyCategories->set_active(maPivotParameters.bDetectCategories);
    mxCheckTotalColumns->set_active(maPivotParameters.bMakeTotalCol);
    mxCheckTotalRows->set_active(maPivotParameters.bMakeTotalRow);

    SetupSource();
    SetupDestination();
}

ScPivotLayoutDialog::~ScPivotLayoutDialog()
{
}

void ScPivotLayoutDialog::SetupSource()
{
    mxSourceListBox->clear();

    ScRange aSourceRange;
    OUString sSourceNamedRangeName;

    if (maPivotTableObject.GetSheetDesc())
    {
        const ScSheetSourceDesc* pSheetSourceDesc = maPivotTableObject.GetSheetDesc();
        aSourceRange = pSheetSourceDesc->GetSourceRange();

        if(!aSourceRange.IsValid())
        {
            // Source is probably a DB Range
            mxSourceRadioNamedRange->set_sensitive(false);
            mxSourceRadioSelection->set_sensitive(false);
            ToggleSource();
            return;
        }
        else
        {
            OUString aSourceRangeName = aSourceRange.Format(mrDocument, ScRefFlags::RANGE_ABS_3D, maAddressDetails);
            mxSourceEdit->SetText(aSourceRangeName);
        }
    }
    else
    {
        mxSourceRadioNamedRange->set_sensitive(false);
        mxSourceRadioSelection->set_sensitive(false);
        ToggleSource();
        return;
    }

    // Setup Named Ranges
    bool bIsNamedRange = false;

    ScAreaNameIterator aIterator(mrDocument);
    OUString aEachName;
    ScRange aEachRange;

    while (aIterator.Next(aEachName, aEachRange))
    {
        if (!aIterator.WasDBName())
        {
            mxSourceListBox->append_text(aEachName);
            if (aEachRange == aSourceRange)
            {
                sSourceNamedRangeName = aEachName;
                bIsNamedRange = true;
            }
        }
    }

    bool bSourceBoxHasEntries = mxSourceListBox->get_count() > 0;

    if (bIsNamedRange)
    {
        mxSourceListBox->set_active_text(sSourceNamedRangeName);
        mxSourceRadioNamedRange->set_active(true);
    }
    else
    {
        // If entries - select first entry
        mxSourceListBox->set_active(bSourceBoxHasEntries ? 0 : -1);
        mxSourceRadioSelection->set_active(true);
    }

    // If no entries disable the radio button.
    if (!bSourceBoxHasEntries)
        mxSourceRadioNamedRange->set_sensitive(false);

    ToggleSource();
}

void ScPivotLayoutDialog::SetupDestination()
{
    mxDestinationListBox->clear();

    // Fill up named ranges
    ScAreaNameIterator aIterator(mrDocument);
    OUString aName;
    ScRange aRange;

    while (aIterator.Next(aName, aRange))
    {
        if (!aIterator.WasDBName())
        {
            mxDestinationListBox->append_text(aName);
        }
    }

    // If entries - select first entry, otherwise disable the radio button.
    if (mxDestinationListBox->get_count() > 0)
        mxDestinationListBox->set_active(0);
    else
        mxDestinationRadioNamedRange->set_sensitive(false);

    //
    if (mbNewPivotTable)
    {
        mxDestinationRadioNewSheet->set_active(true);
    }
    else
    {
        if (maPivotParameters.nTab != MAXTAB + 1)
        {
            ScAddress aAddress(maPivotParameters.nCol, maPivotParameters.nRow, maPivotParameters.nTab);
            OUString aAddressString = aAddress.Format(ScRefFlags::ADDR_ABS_3D, &mrDocument, maAddressDetails);
            mxDestinationEdit->SetText(aAddressString);
            mxDestinationRadioSelection->set_active(true);
        }
    }

    ToggleDestination();
}

void ScPivotLayoutDialog::FillValuesToListBoxes()
{
    mxListBoxField->FillLabelFields(maPivotParameters.maLabelArray);
    mxListBoxData->FillDataField(maPivotParameters.maDataFields);
    mxListBoxColumn->FillFields(maPivotParameters.maColFields);
    mxListBoxRow->FillFields(maPivotParameters.maRowFields);
    mxListBoxPage->FillFields(maPivotParameters.maPageFields);
}

void ScPivotLayoutDialog::SetActive()
{
    if (mbDialogLostFocus)
    {
        mbDialogLostFocus = false;
        if(mpActiveEdit != nullptr)
        {
            mpActiveEdit->GrabFocus();
            if (mpActiveEdit == mxSourceEdit.get())
                UpdateSourceRange();
        }
    }
    else
    {
        m_xDialog->grab_focus();
    }

    RefInputDone();
}

void ScPivotLayoutDialog::SetReference(const ScRange& rReferenceRange, ScDocument& rDocument)
{
    if (!mbDialogLostFocus)
        return;

    if (mpActiveEdit == nullptr)
        return;

    if (rReferenceRange.aStart != rReferenceRange.aEnd)
        RefInputStart(mpActiveEdit);

    OUString aReferenceString = rReferenceRange.Format(rDocument, ScRefFlags::RANGE_ABS_3D, maAddressDetails);

    if (mpActiveEdit == mxSourceEdit.get())
    {
        mxSourceEdit->SetRefString(aReferenceString);
    }
    else if (mpActiveEdit == mxDestinationEdit.get())
    {
        mxDestinationEdit->SetRefString(aReferenceString);
    }
}

bool ScPivotLayoutDialog::IsRefInputMode() const
{
    return mbDialogLostFocus;
}

void ScPivotLayoutDialog::ItemInserted(const ScItemValue* pItemValue, ScPivotLayoutTreeList::SvPivotTreeListType eType)
{
    if (pItemValue == nullptr)
        return;

    switch (eType)
    {
        case ScPivotLayoutTreeList::ROW_LIST:
        case ScPivotLayoutTreeList::COLUMN_LIST:
        case ScPivotLayoutTreeList::PAGE_LIST:
        {
            mxListBoxRow->RemoveEntryForItem(pItemValue);
            mxListBoxColumn->RemoveEntryForItem(pItemValue);
            mxListBoxPage->RemoveEntryForItem(pItemValue);
        }
        break;
        case ScPivotLayoutTreeList::LABEL_LIST:
        {
            mxListBoxRow->RemoveEntryForItem(pItemValue);
            mxListBoxColumn->RemoveEntryForItem(pItemValue);
            mxListBoxPage->RemoveEntryForItem(pItemValue);
            mxListBoxData->RemoveEntryForItem(pItemValue);
        }
        break;
        default:
            break;
    }
}

void ScPivotLayoutDialog::UpdateSourceRange()
{
    if (!maPivotTableObject.GetSheetDesc())
       return;

    ScSheetSourceDesc aSourceSheet = *maPivotTableObject.GetSheetDesc();

    if (mxSourceRadioNamedRange->get_active())
    {
        OUString aEntryString = mxSourceListBox->get_active_text();
        ScRange aSourceRange = lclGetRangeForNamedRange(aEntryString, mrDocument);
        if (!aSourceRange.IsValid() || aSourceSheet.GetSourceRange() == aSourceRange)
            return;
        aSourceSheet.SetRangeName(aEntryString);
    }
    else if (mxSourceRadioSelection->get_active())
    {
        OUString aSourceString = mxSourceEdit->GetText();
        ScRange aSourceRange;
        ScRefFlags nResult = aSourceRange.Parse(aSourceString, mrDocument, maAddressDetails);

        bool bIsValid = (nResult & ScRefFlags::VALID) == ScRefFlags::VALID; // aSourceString is valid

        mxSourceEdit->SetRefValid(true);

        if (bIsValid)
        {
            ScRefAddress aStart;
            ScRefAddress aEnd;

            ConvertDoubleRef(mrDocument, aSourceString, 1, aStart, aEnd, maAddressDetails);
            aSourceRange.aStart = aStart.GetAddress();
            aSourceRange.aEnd = aEnd.GetAddress();
        }
        else
        {
            aSourceRange = lclGetRangeForNamedRange(aSourceString, mrDocument);
        }

        if (!aSourceRange.IsValid())
        {
            mxSourceEdit->SetRefValid(false);
            return;
        }

        if (aSourceSheet.GetSourceRange() == aSourceRange)
                return;

        aSourceSheet.SetSourceRange(aSourceRange);
        if (aSourceSheet.CheckSourceRange())
        {
            mxSourceEdit->SetRefValid(false);
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

void ScPivotLayoutDialog::ApplyChanges()
{
    ScDPSaveData aSaveData;
    ApplySaveData(aSaveData);
    ApplyLabelData(aSaveData);

    ScDPObject *pOldDPObj = mrDocument.GetDPAtCursor( maPivotParameters.nCol, maPivotParameters.nRow, maPivotParameters.nTab);
    ScRange aDestinationRange;
    bool bToNewSheet = false;

    if (!GetDestination(aDestinationRange, bToNewSheet))
        return;

    SetDispatcherLock(false);
    SwitchToDocument();

    sal_uInt16 nWhichPivot = SC_MOD()->GetPool().GetWhich(SID_PIVOT_TABLE);
    ScPivotItem aPivotItem(nWhichPivot, &aSaveData, &aDestinationRange, bToNewSheet);
    mpViewData->GetViewShell()->SetDialogDPObject(std::make_unique<ScDPObject>(maPivotTableObject));


    SfxDispatcher* pDispatcher = GetBindings().GetDispatcher();
    SfxCallMode const nCallMode = SfxCallMode::SLOT | SfxCallMode::RECORD;
    const SfxPoolItem* pResult = pDispatcher->ExecuteList(SID_PIVOT_TABLE,
            nCallMode, { &aPivotItem });

    if (pResult != nullptr)
    {
        // existing pivot table might have moved to a new range or a new sheet
        if ( pOldDPObj != nullptr  )
        {
            const ScRange& rOldRange = pOldDPObj->GetOutRange();

            ScDPObject *pDPObj = nullptr;
            // FIXME: if the new range overlaps with the old one, the table actually doesn't move
            // and shouldn't therefore be deleted
            if ( ( ( rOldRange != aDestinationRange ) && !rOldRange.Contains( aDestinationRange ) )
                 || bToNewSheet )
            {
                pDPObj = mrDocument.GetDPAtCursor( maPivotParameters.nCol, maPivotParameters.nRow, maPivotParameters.nTab);
            }
            if (pDPObj)
            {
                ScDBDocFunc aFunc( *(mpViewData->GetDocShell() ));
                aFunc.RemovePivotTable( *pDPObj, true, false);
                mpViewData->GetView()->CursorPosChanged();
            }
        }
        return;
    }

    SetDispatcherLock(true);
}

void ScPivotLayoutDialog::ApplySaveData(ScDPSaveData& rSaveData)
{
    rSaveData.SetIgnoreEmptyRows(mxCheckIgnoreEmptyRows->get_active());
    rSaveData.SetRepeatIfEmpty(mxCheckIdentifyCategories->get_active());
    rSaveData.SetColumnGrand(mxCheckTotalColumns->get_active());
    rSaveData.SetRowGrand(mxCheckTotalRows->get_active());
    rSaveData.SetFilterButton(mxCheckAddFilter->get_active());
    rSaveData.SetDrillDown(mxCheckDrillToDetail->get_active());
    rSaveData.SetExpandCollapse(mxCheckExpandCollapse->get_active());

    Reference<XDimensionsSupplier> xSource = maPivotTableObject.GetSource();

    ScPivotFieldVector aPageFieldVector;
    mxListBoxPage->PushEntriesToPivotFieldVector(aPageFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aPageFieldVector, DataPilotFieldOrientation_PAGE,
                                   xSource, maPivotParameters.maLabelArray);

    ScPivotFieldVector aColFieldVector;
    mxListBoxColumn->PushEntriesToPivotFieldVector(aColFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aColFieldVector, DataPilotFieldOrientation_COLUMN,
                                   xSource, maPivotParameters.maLabelArray);

    ScPivotFieldVector aRowFieldVector;
    mxListBoxRow->PushEntriesToPivotFieldVector(aRowFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aRowFieldVector, DataPilotFieldOrientation_ROW,
                                   xSource, maPivotParameters.maLabelArray);

    ScPivotFieldVector aDataFieldVector;
    mxListBoxData->PushEntriesToPivotFieldVector(aDataFieldVector);
    ScDPObject::ConvertOrientation(rSaveData, aDataFieldVector, DataPilotFieldOrientation_DATA,
                                   xSource, maPivotParameters.maLabelArray,
                                   &aColFieldVector, &aRowFieldVector, &aPageFieldVector);
}

void ScPivotLayoutDialog::ApplyLabelData(const ScDPSaveData& rSaveData)
{
    ScDPLabelDataVector& rLabelDataVector = GetLabelDataVector();

    for (std::unique_ptr<ScDPLabelData> const & pLabelData : rLabelDataVector)
    {
        OUString aUnoName = ScDPUtil::createDuplicateDimensionName(pLabelData->maName, pLabelData->mnDupCount);
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

        for (ScDPLabelData::Member const & rLabelMember : pLabelData->maMembers)
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

bool ScPivotLayoutDialog::GetDestination(ScRange& aDestinationRange, bool& bToNewSheet)
{
    bToNewSheet = false;

    if (mxDestinationRadioNamedRange->get_active())
    {
        OUString aName = mxDestinationListBox->get_active_text();
        aDestinationRange = lclGetRangeForNamedRange(aName, mrDocument);
        if (!aDestinationRange.IsValid())
            return false;
    }
    else if (mxDestinationRadioSelection->get_active())
    {
        ScAddress aAddress;
        aAddress.Parse(mxDestinationEdit->GetText(), mrDocument, maAddressDetails);
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
    return mxListBoxField->GetItem(nColumn);
}

bool ScPivotLayoutDialog::IsDataElement(SCCOL nColumn)
{
    return mxListBoxField->IsDataElement(nColumn);
}

ScDPLabelData& ScPivotLayoutDialog::GetLabelData(SCCOL nColumn)
{
    return *maPivotParameters.maLabelArray[nColumn];
}

void ScPivotLayoutDialog::PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames)
{
    mxListBoxData->PushDataFieldNames(rDataFieldNames);
}

void ScPivotLayoutDialog::Close()
{
    DoClose(ScPivotLayoutWrapper::GetChildWindowId());
    SfxDialogController::Close();
}

IMPL_LINK_NOARG( ScPivotLayoutDialog, OKClicked, weld::Button&, void )
{
    /* tdf#137726 hide so it's not a candidate to be parent of any error
       messages that may appear because this dialog is going to disappear on
       response(RET_OK) and the error dialog is not run in its own event loop
       but instead async */
    m_xDialog->hide();

    ApplyChanges();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG( ScPivotLayoutDialog, CancelClicked, weld::Button&, void )
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK(ScPivotLayoutDialog, GetEditFocusHandler, formula::RefEdit&, rCtrl, void)
{
    mpActiveEdit = &rCtrl;
    mpActiveEdit->SelectAll();
}

IMPL_LINK(ScPivotLayoutDialog, GetButtonFocusHandler, formula::RefButton&, rCtrl, void)
{
    mpActiveEdit = nullptr;

    if (&rCtrl == mxSourceButton.get())
        mpActiveEdit = mxSourceEdit.get();
    else if (&rCtrl == mxDestinationButton.get())
        mpActiveEdit = mxDestinationEdit.get();

    if (mpActiveEdit)
        mpActiveEdit->SelectAll();
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, LoseEditFocusHandler, formula::RefEdit&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, LoseButtonFocusHandler, formula::RefButton&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, SourceListSelected, weld::ComboBox&, void)
{
    UpdateSourceRange();
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, SourceEditModified, formula::RefEdit&, void)
{
    UpdateSourceRange();
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, ToggleSource, weld::Toggleable&, void)
{
    ToggleSource();
}

void ScPivotLayoutDialog::ToggleSource()
{
    bool bNamedRange = mxSourceRadioNamedRange->get_active();
    bool bSelection = mxSourceRadioSelection->get_active();
    mxSourceListBox->set_sensitive(bNamedRange);
    mxSourceButton->GetWidget()->set_sensitive(bSelection);
    mxSourceEdit->GetWidget()->set_sensitive(bSelection);
    UpdateSourceRange();
}

IMPL_LINK_NOARG(ScPivotLayoutDialog, ToggleDestination, weld::Toggleable&, void)
{
    ToggleDestination();
}

void ScPivotLayoutDialog::ToggleDestination()
{
    bool bNamedRange = mxDestinationRadioNamedRange->get_active();
    bool bSelection = mxDestinationRadioSelection->get_active();
    mxDestinationListBox->set_sensitive(bNamedRange);
    mxDestinationButton->GetWidget()->set_sensitive(bSelection);
    mxDestinationEdit->GetWidget()->set_sensitive(bSelection);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
