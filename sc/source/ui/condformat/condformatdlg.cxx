/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <condformatdlg.hxx>

#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>

#include <anyrefdg.hxx>
#include <document.hxx>
#include <conditio.hxx>
#include <tabvwsh.hxx>
#include <colorscale.hxx>
#include <reffact.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <condformatdlgentry.hxx>
#include <condformatdlgdata.hxx>

ScCondFormatList::ScCondFormatList(ScCondFormatDlg* pDialogParent,
                                   ScDocument& rDoc,
                                   std::unique_ptr<weld::ScrolledWindow> xWindow,
                                   std::unique_ptr<weld::Grid> xGrid)
    : mxScrollWindow(std::move(xWindow))
    , mxGrid(std::move(xGrid))
    , mbFrozen(false)
    , mbNewEntry(false)
    , mrDoc(rDoc)
    , mpDialogParent(pDialogParent)
{
    mxScrollWindow->set_size_request(mxScrollWindow->get_approximate_digit_width() * 85,
                                     mxScrollWindow->get_text_height() * 23);
    mxGrid->set_stack_background();
}

weld::Window* ScCondFormatList::GetFrameWeld()
{
    return mpDialogParent->getDialog();
}

ScCondFormatList::~ScCondFormatList()
{
    Freeze();
}

void ScCondFormatList::init(
        const ScConditionalFormat* pFormat, const ScRangeList& rRanges,
        const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType)
{
    maPos = rPos;
    maRanges = rRanges;

    Freeze();

    if(pFormat)
    {
        size_t nCount = pFormat->size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const ScFormatEntry* pEntry = pFormat->GetEntry(nIndex);
            switch(pEntry->GetType())
            {
                case ScFormatEntry::Type::Condition:
                case ScFormatEntry::Type::ExtCondition:
                    {
                        const ScCondFormatEntry* pConditionEntry = static_cast<const ScCondFormatEntry*>( pEntry );
                        if(pConditionEntry->GetOperation() != ScConditionMode::Direct)
                            maEntries.emplace_back(new ScConditionFrmtEntry( this, mrDoc, mpDialogParent, maPos, pConditionEntry ) );
                        else
                            maEntries.emplace_back(new ScFormulaFrmtEntry( this, mrDoc, mpDialogParent, maPos, pConditionEntry ) );

                    }
                    break;
                case ScFormatEntry::Type::Colorscale:
                    {
                        const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>( pEntry );
                        if( pColorScale->size() == 2 )
                            maEntries.emplace_back(new ScColorScale2FrmtEntry( this, mrDoc, maPos, pColorScale ) );
                        else
                            maEntries.emplace_back(new ScColorScale3FrmtEntry( this, mrDoc, maPos, pColorScale ) );
                    }
                    break;
                case ScFormatEntry::Type::Databar:
                    maEntries.emplace_back(new ScDataBarFrmtEntry( this, mrDoc, maPos, static_cast<const ScDataBarFormat*>( pEntry ) ) );
                    break;
                case ScFormatEntry::Type::Iconset:
                    maEntries.emplace_back(new ScIconSetFrmtEntry( this, mrDoc, maPos, static_cast<const ScIconSetFormat*>( pEntry ) ) );
                    break;
                case ScFormatEntry::Type::Date:
                    maEntries.emplace_back(new ScDateFrmtEntry( this, mrDoc, static_cast<const ScCondDateFormatEntry*>( pEntry ) ) );
                    break;
            }
        }
        if(nCount)
            EntrySelectHdl(*maEntries[0]);
    }
    else
    {
        switch(eType)
        {
            case condformat::dialog::CONDITION:
                maEntries.emplace_back(new ScConditionFrmtEntry( this, mrDoc, mpDialogParent, maPos ));
                break;
            case condformat::dialog::COLORSCALE:
                maEntries.emplace_back(new ScColorScale3FrmtEntry( this, mrDoc, maPos ));
                break;
            case condformat::dialog::DATABAR:
                maEntries.emplace_back(new ScDataBarFrmtEntry( this, mrDoc, maPos ));
                break;
            case condformat::dialog::ICONSET:
                maEntries.emplace_back(new ScIconSetFrmtEntry( this, mrDoc, maPos ));
                break;
            case condformat::dialog::DATE:
                maEntries.emplace_back(new ScDateFrmtEntry( this, mrDoc ));
                break;
            case condformat::dialog::NONE:
                break;
        }
        mbNewEntry = true;
    }
    Thaw();
    RecalcAll();
    if (!maEntries.empty())
    {
        (*maEntries.begin())->SetActive();
        mpDialogParent->OnSelectionChange(0, maEntries.size());
    }

    RecalcAll();
}

void ScCondFormatList::SetRange(const ScRangeList& rRange)
{
    maRanges = rRange;
}

std::unique_ptr<ScConditionalFormat> ScCondFormatList::GetConditionalFormat() const
{
    if(maEntries.empty())
        return nullptr;

    std::unique_ptr<ScConditionalFormat> pFormat(new ScConditionalFormat(0, mrDoc));
    pFormat->SetRange(maRanges);

    for(auto & rEntry: maEntries)
    {
        // tdf#119178: Sometimes initial apply-to range (the one this dialog
        // was opened with) is different from the final apply-to range
        // (as edited by the user)

        // If this format entry is new, take top-left corner of the final range
        // and use it to create the initial entry (token array therein, if applicable)
        if (mbNewEntry)
            rEntry->SetPos(maRanges.GetTopLeftCorner());
        // else do nothing: setting new position when editing recompiles formulas
        // in entries and nobody wants that

        ScFormatEntry* pEntry = rEntry->GetEntry();
        if(pEntry)
            pFormat->AddEntry(pEntry);
    }

    return pFormat;
}

void ScCondFormatList::RecalcAll()
{
    if (mbFrozen)
        return;

    int nWheelScroll = SAL_MAX_INT32;

    sal_Int32 nIndex = 1;
    for (const auto& item : maEntries)
    {
        if (!item)
            continue;
        item->SetIndex(nIndex);
        item->set_grid_top_attach(nIndex - 1);
        nWheelScroll = std::min(nWheelScroll, item->get_preferred_height());
        ++nIndex;
    }

    if (nWheelScroll != SAL_MAX_INT32)
    {
        // tdf#118482 set a scroll step of the height of a collapsed entry
        mxScrollWindow->vadjustment_set_step_increment(nWheelScroll);
    }
}

IMPL_LINK(ScCondFormatList, ColFormatTypeHdl, weld::ComboBox&, rBox, void)
{
    Application::PostUserEvent(LINK(this, ScCondFormatList, AfterColFormatTypeHdl), &rBox);
}

IMPL_LINK(ScCondFormatList, AfterColFormatTypeHdl, void*, p, void)
{
    weld::ComboBox* pBox = static_cast<weld::ComboBox*>(p);
    EntryContainer::iterator itr = std::find_if(maEntries.begin(), maEntries.end(),
        [](const std::unique_ptr<ScCondFrmtEntry>& widget) { return widget->IsSelected(); });
    if(itr == maEntries.end())
        return;

    sal_Int32 nPos = pBox->get_active();
    switch(nPos)
    {
        case 0:
            if((*itr)->GetType() == condformat::entry::COLORSCALE2)
                return;

            Freeze();
            itr->reset(new ScColorScale2FrmtEntry(this, mrDoc, maPos));
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::COLORSCALE3)
                return;

            Freeze();
            itr->reset(new ScColorScale3FrmtEntry(this, mrDoc, maPos));
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::DATABAR)
                return;

            Freeze();
            itr->reset(new ScDataBarFrmtEntry(this, mrDoc, maPos));
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::ICONSET)
                return;

            Freeze();
            itr->reset(new ScIconSetFrmtEntry(this, mrDoc, maPos));
            break;
        default:
            break;
    }
    mpDialogParent->InvalidateRefData();
    (*itr)->SetActive();
    Thaw();
    RecalcAll();
}

IMPL_LINK(ScCondFormatList, TypeListHdl, weld::ComboBox&, rBox, void)
{
    //Resolves: fdo#79021 At this point we are still inside the ListBox Select.
    //If we call maEntries.replace here then the pBox will be deleted before it
    //has finished Select and will crash on accessing its deleted this. So Post
    //to do the real work after the Select has completed
    Application::PostUserEvent(LINK(this, ScCondFormatList, AfterTypeListHdl), &rBox);
}

IMPL_LINK(ScCondFormatList, AfterTypeListHdl, void*, p, void)
{
    weld::ComboBox* pBox = static_cast<weld::ComboBox*>(p);
    EntryContainer::iterator itr = std::find_if(maEntries.begin(), maEntries.end(),
        [](const std::unique_ptr<ScCondFrmtEntry>& widget) { return widget->IsSelected(); });
    if(itr == maEntries.end())
        return;

    sal_Int32 nPos = pBox->get_active();
    switch(nPos)
    {
        case 0:
            switch((*itr)->GetType())
            {
                case condformat::entry::FORMULA:
                case condformat::entry::CONDITION:
                case condformat::entry::DATE:
                    break;
                case condformat::entry::COLORSCALE2:
                case condformat::entry::COLORSCALE3:
                case condformat::entry::DATABAR:
                case condformat::entry::ICONSET:
                    return;
            }
            Freeze();
            itr->reset(new ScColorScale3FrmtEntry(this, mrDoc, maPos));
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::CONDITION)
                return;

            Freeze();
            itr->reset(new ScConditionFrmtEntry(this, mrDoc, mpDialogParent, maPos));
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::FORMULA)
                return;

            Freeze();
            itr->reset(new ScFormulaFrmtEntry(this, mrDoc, mpDialogParent, maPos));
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::DATE)
                return;

            Freeze();
            itr->reset(new ScDateFrmtEntry( this, mrDoc ));
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;

    }
    Thaw();
    RecalcAll();
}

IMPL_LINK_NOARG( ScCondFormatList, AddBtnHdl, weld::Button&, void )
{
    Freeze();
    maEntries.emplace_back(new ScConditionFrmtEntry(this, mrDoc, mpDialogParent, maPos));
    for(auto& rxEntry : maEntries)
    {
        rxEntry->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    maEntries.back()->SetActive();
    mpDialogParent->OnSelectionChange(maEntries.size() - 1, maEntries.size());
    Thaw();
    RecalcAll();
}

IMPL_LINK_NOARG( ScCondFormatList, RemoveBtnHdl, weld::Button&, void )
{
    Freeze();
    auto itr = std::find_if(maEntries.begin(), maEntries.end(),
        [](const std::unique_ptr<ScCondFrmtEntry>& widget) { return widget->IsSelected(); });
    if (itr != maEntries.end())
    {
        maEntries.erase(itr);
    }
    mpDialogParent->InvalidateRefData();
    mpDialogParent->OnSelectionChange(0, maEntries.size(), false);
    Thaw();
    RecalcAll();
}

IMPL_LINK_NOARG(ScCondFormatList, UpBtnHdl, weld::Button&, void)
{
    Freeze();
    size_t index = 0;
    for (size_t i = 0; i < maEntries.size(); i++)
    {
        auto& widget = maEntries[i];
        if (widget->IsSelected() && i > 0)
        {
            std::swap(maEntries[i], maEntries[i - 1]);
            index = i - 1;
            break;
        }
    }
    mpDialogParent->InvalidateRefData();
    mpDialogParent->OnSelectionChange(index, maEntries.size());
    Thaw();
    RecalcAll();
}

IMPL_LINK_NOARG(ScCondFormatList, DownBtnHdl, weld::Button&, void)
{
    Freeze();
    size_t index = 0;
    for (size_t i = 0; i < maEntries.size(); i++)
    {
        auto& widget = maEntries[i];
        if (widget->IsSelected())
        {
            index = i;
            if (i < maEntries.size()-1)
            {
                std::swap(maEntries[i], maEntries[i + 1]);
                index = i + 1;
                break;
            }
        }
    }
    mpDialogParent->InvalidateRefData();
    mpDialogParent->OnSelectionChange(index, maEntries.size());
    Thaw();
    RecalcAll();
}

IMPL_LINK( ScCondFormatList, EntrySelectHdl, ScCondFrmtEntry&, rEntry, void )
{
    if(rEntry.IsSelected())
        return;

    Freeze();
    size_t index = 0;
    for(size_t i = 0; i < maEntries.size(); i++)
    {
        if (maEntries[i].get() == &rEntry)
        {
            index = i;
        }
        maEntries[i]->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    mpDialogParent->OnSelectionChange(index, maEntries.size());
    rEntry.SetActive();
    Thaw();
    RecalcAll();
}

static bool isLOKMobilePhone()
{
    SfxViewShell* pCurrent = SfxViewShell::Current();
    return pCurrent && pCurrent->isLOKMobilePhone();
}

ScCondFormatDlg::ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW,
    weld::Window* pParent, ScViewData& rViewData,
    const std::shared_ptr<ScCondFormatDlgData>& rItem)
        : ScAnyRefDlgController(pB, pCW, pParent,
                        isLOKMobilePhone()?u"modules/scalc/ui/conditionalformatdialogmobile.ui"_ustr:u"modules/scalc/ui/conditionalformatdialog.ui"_ustr,
                        u"ConditionalFormatDialog"_ustr)
    , mrViewData(rViewData)
    // previous version based on SfxPoolItem used SfxPoolItem::Clone here, so make a copy
    // using copy constructor
    , mpDlgData(std::make_shared<ScCondFormatDlgData>(*rItem))
    , mpLastEdit(nullptr)
    , mxBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , mxBtnAdd(m_xBuilder->weld_button(u"add"_ustr))
    , mxBtnRemove(m_xBuilder->weld_button(u"delete"_ustr))
    , mxBtnUp(m_xBuilder->weld_button(u"up"_ustr))
    , mxBtnDown(m_xBuilder->weld_button(u"down"_ustr))
    , mxBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
    , mxFtRange(m_xBuilder->weld_label(u"ftassign"_ustr))
    , mxEdRange(new formula::RefEdit(m_xBuilder->weld_entry(u"edassign"_ustr)))
    , mxRbRange(new formula::RefButton(m_xBuilder->weld_button(u"rbassign"_ustr)))
    , mxCondFormList(new ScCondFormatList(this, mrViewData.GetDocument(), m_xBuilder->weld_scrolled_window(u"listwindow"_ustr),
                                          m_xBuilder->weld_grid(u"list"_ustr)))
{
    mxEdRange->SetReferences(this, mxFtRange.get());
    mxRbRange->SetReferences(this, mxEdRange.get());

    ScConditionalFormat* pFormat = nullptr;
    mnKey = mpDlgData->GetIndex();
    if (mpDlgData->IsManaged() && mpDlgData->GetConditionalFormatList())
    {
        pFormat = mpDlgData->GetConditionalFormatList()->GetFormat(mnKey);
    }
    else if (!mpDlgData->IsManaged())
    {
        ScDocument& rDoc = mrViewData.GetDocument();
        pFormat = rDoc.GetCondFormList(mrViewData.GetTabNo())->GetFormat ( mnKey );
    }

    ScRangeList aRange;
    if (pFormat)
    {
        aRange = pFormat->GetRange();
    }
    else
    {
        // this is for adding a new entry
        mrViewData.GetMarkData().FillRangeListWithMarks(&aRange, false);
        if(aRange.empty())
        {
            ScAddress aPos(mrViewData.GetCurX(), mrViewData.GetCurY(), mrViewData.GetTabNo());
            aRange.push_back(ScRange(aPos));
        }
        mnKey = 0;
    }
    maPos = aRange.GetTopLeftCorner();

    mxCondFormList->init(pFormat, aRange, maPos, mpDlgData->GetDialogType());

    mxBtnOk->connect_clicked(LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mxBtnAdd->connect_clicked( LINK( mxCondFormList.get(), ScCondFormatList, AddBtnHdl ) );
    mxBtnRemove->connect_clicked( LINK( mxCondFormList.get(), ScCondFormatList, RemoveBtnHdl ) );
    mxBtnUp->connect_clicked(LINK(mxCondFormList.get(), ScCondFormatList, UpBtnHdl));
    mxBtnDown->connect_clicked(LINK(mxCondFormList.get(), ScCondFormatList, DownBtnHdl));
    mxBtnCancel->connect_clicked( LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mxEdRange->SetModifyHdl( LINK( this, ScCondFormatDlg, EdRangeModifyHdl ) );
    mxEdRange->SetGetFocusHdl( LINK( this, ScCondFormatDlg, RangeGetFocusHdl ) );

    OUString aRangeString;
    const ScDocument& rDoc = rViewData.GetDocument();
    aRange.Format(aRangeString, ScRefFlags::VALID, rDoc, rDoc.GetAddressConvention());
    mxEdRange->SetText(aRangeString);

    msBaseTitle = m_xDialog->get_title();
    updateTitle();
}

void ScCondFormatDlg::updateTitle()
{
    OUString aTitle = msBaseTitle + " " + mxEdRange->GetText();

    m_xDialog->set_title(aTitle);
}

ScCondFormatDlg::~ScCondFormatDlg()
{
}

void ScCondFormatDlg::SetActive()
{
    if(mpLastEdit)
        mpLastEdit->GrabFocus();
    else
        mxEdRange->GrabFocus();

    RefInputDone();
}

void ScCondFormatDlg::RefInputDone( bool bForced )
{
    ScAnyRefDlgController::RefInputDone(bForced);

    // ScAnyRefModalDlg::RefInputDone resets the title back
    // to its original state.
    // I.e. if we open the dialog normally, and then click into the sheet
    // to modify the selection, the title is updated such that the range
    // is only a single cell (e.g. $A$1), after which the dialog switches
    // into the RefInput mode. During the RefInput mode the title is updated
    // as expected, however at the end RefInputDone overwrites the title
    // with the initial (now incorrect) single cell range. Hence we correct
    // it here.
    updateTitle();
}

bool ScCondFormatDlg::IsTableLocked() const
{
    return !mpLastEdit || mpLastEdit == mxEdRange.get();
}

bool ScCondFormatDlg::IsRefInputMode() const
{
    return mxEdRange->GetWidget()->get_sensitive();
}

void ScCondFormatDlg::SetReference(const ScRange& rRef, ScDocument&)
{
    formula::RefEdit* pEdit = mpLastEdit;
    if (!mpLastEdit)
        pEdit = mxEdRange.get();

    if (!pEdit->GetWidget()->get_sensitive())
        return;

    if(rRef.aStart != rRef.aEnd)
        RefInputStart(pEdit);

    ScRefFlags nFlags;
    if (mpLastEdit && mpLastEdit != mxEdRange.get())
        nFlags = ScRefFlags::RANGE_ABS_3D;
    else
        nFlags = ScRefFlags::RANGE_ABS;

    const ScDocument& rDoc = mrViewData.GetDocument();
    OUString aRefStr(rRef.Format(rDoc, nFlags,
        ScAddress::Details(rDoc.GetAddressConvention(), 0, 0)));
    if (pEdit != mxEdRange.get())
    {
        Selection sel = pEdit->GetSelection();
        sel.Normalize();            // in case of RTL selection
        sel.Max() = sel.Min() + aRefStr.getLength();
        pEdit->GetWidget()->replace_selection(aRefStr);
        pEdit->SetSelection(sel); // to replace it again with next drag event
    }
    else
        pEdit->SetRefString( aRefStr );
    updateTitle();
}

std::unique_ptr<ScConditionalFormat> ScCondFormatDlg::GetConditionalFormat() const
{
    OUString aRangeStr = mxEdRange->GetText();
    if(aRangeStr.isEmpty())
        return nullptr;

    ScRangeList aRange;
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mrViewData.GetDocument(),
        mrViewData.GetDocument().GetAddressConvention(), maPos.Tab());
    mxCondFormList->SetRange(aRange);
    std::unique_ptr<ScConditionalFormat> pFormat = mxCondFormList->GetConditionalFormat();

    if((nFlags & ScRefFlags::VALID) && !aRange.empty() && pFormat)
        pFormat->SetRange(aRange);
    else
        pFormat.reset();

    return pFormat;
}

void ScCondFormatDlg::InvalidateRefData()
{
    mpLastEdit = nullptr;
}

// Close the Conditional Format Dialog
//
void ScCondFormatDlg::Close()
{
    DoClose( ScCondFormatDlgWrapper::GetChildWindowId() );
}

// Occurs when the Conditional Format Dialog the OK button is pressed.
//
void ScCondFormatDlg::OkPressed()
{
    std::unique_ptr<ScConditionalFormat> pFormat = GetConditionalFormat();

    if (!mpDlgData->IsManaged())
    {
        if(pFormat)
        {
            auto& rRangeList = pFormat->GetRange();
            mrViewData.GetDocShell().GetDocFunc().ReplaceConditionalFormat(mnKey,
                    std::move(pFormat), maPos.Tab(), rRangeList);
        }
        else
            mrViewData.GetDocShell().GetDocFunc().ReplaceConditionalFormat(mnKey,
                    nullptr, maPos.Tab(), ScRangeList());
    }
    else
    {
        ScConditionalFormatList* pList = mpDlgData->GetConditionalFormatList();
        sal_uInt32 nKey = mnKey;
        if (mnKey == 0)
        {
            nKey = pList->getMaxKey() + 1;
        }

        pList->erase(nKey);
        if (pFormat)
        {
            pFormat->SetKey(nKey);
            pList->InsertNew(std::move(pFormat));
        }

        // provide needed DialogData
        mrViewData.GetViewShell()->setScCondFormatDlgData(mpDlgData);
        SetDispatcherLock( false );

        // Queue message to open Conditional Format Manager Dialog
        GetBindings().GetDispatcher()->Execute( SID_OPENDLG_CONDFRMT_MANAGER,
                                            SfxCallMode::ASYNCHRON );
    }
    m_xDialog->response(RET_OK);
}

// Occurs when the Conditional Format Dialog is cancelled.
//
void ScCondFormatDlg::CancelPressed()
{
    if ( mpDlgData->IsManaged() )
    {
        // provide needed DialogData
        mrViewData.GetViewShell()->setScCondFormatDlgData(mpDlgData);
        SetDispatcherLock( false );

        // Queue message to open Conditional Format Manager Dialog
        GetBindings().GetDispatcher()->Execute( SID_OPENDLG_CONDFRMT_MANAGER,
                                            SfxCallMode::ASYNCHRON );
    }
    m_xDialog->response(RET_CANCEL);
}

void ScCondFormatDlg::OnSelectionChange(size_t nIndex, size_t nSize, bool bSelected)
{
    if (nSize <= 1 || !bSelected)
    {
        mxBtnUp->set_sensitive(false);
        mxBtnDown->set_sensitive(false);
    }
    else
    {
        mxBtnUp->set_sensitive(nIndex != 0);
        mxBtnDown->set_sensitive(nIndex < nSize - 1);
    }
}

IMPL_LINK(ScCondFormatDlg, EdRangeModifyHdl, formula::RefEdit&, rEdit, void)
{
    OUString aRangeStr = rEdit.GetText();
    ScRangeList aRange;
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mrViewData.GetDocument(),
        mrViewData.GetDocument().GetAddressConvention());
    if(nFlags & ScRefFlags::VALID)
    {
        rEdit.GetWidget()->set_message_type(weld::EntryMessageType::Normal);
        mxBtnOk->set_sensitive(true);
    }
    else
    {
        rEdit.GetWidget()->set_message_type(weld::EntryMessageType::Error);
        mxBtnOk->set_sensitive(false);
    }

    updateTitle();
}

IMPL_LINK(ScCondFormatDlg, RangeGetFocusHdl, formula::RefEdit&, rControl, void)
{
    mpLastEdit = &rControl;
}

IMPL_LINK( ScCondFormatDlg, BtnPressedHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnOk.get())
        OkPressed();
    else if (&rBtn == mxBtnCancel.get())
        CancelPressed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
