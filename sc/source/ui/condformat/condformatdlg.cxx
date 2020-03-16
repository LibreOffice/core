/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>
#include <condformatdlg.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/lokhelper.hxx>
#include <vcl/settings.hxx>
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
#include <condformatdlgitem.hxx>

ScCondFormatList::ScCondFormatList(ScCondFormatDlg* pDialogParent,
                                   std::unique_ptr<weld::ScrolledWindow> xWindow,
                                   std::unique_ptr<weld::Container> xGrid)
    : mxScrollWindow(std::move(xWindow))
    , mxGrid(std::move(xGrid))
    , mbFrozen(false)
    , mbNewEntry(false)
    , mpDoc(nullptr)
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

void ScCondFormatList::init(ScDocument* pDoc,
        const ScConditionalFormat* pFormat, const ScRangeList& rRanges,
        const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType)
{
    mpDoc = pDoc;
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
                            maEntries.emplace_back(new ScConditionFrmtEntry( this, mpDoc, mpDialogParent, maPos, pConditionEntry ) );
                        else
                            maEntries.emplace_back(new ScFormulaFrmtEntry( this, mpDoc, mpDialogParent, maPos, pConditionEntry ) );

                    }
                    break;
                case ScFormatEntry::Type::Colorscale:
                    {
                        const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>( pEntry );
                        if( pColorScale->size() == 2 )
                            maEntries.emplace_back(new ScColorScale2FrmtEntry( this, mpDoc, maPos, pColorScale ) );
                        else
                            maEntries.emplace_back(new ScColorScale3FrmtEntry( this, mpDoc, maPos, pColorScale ) );
                    }
                    break;
                case ScFormatEntry::Type::Databar:
                    maEntries.emplace_back(new ScDataBarFrmtEntry( this, mpDoc, maPos, static_cast<const ScDataBarFormat*>( pEntry ) ) );
                    break;
                case ScFormatEntry::Type::Iconset:
                    maEntries.emplace_back(new ScIconSetFrmtEntry( this, mpDoc, maPos, static_cast<const ScIconSetFormat*>( pEntry ) ) );
                    break;
                case ScFormatEntry::Type::Date:
                    maEntries.emplace_back(new ScDateFrmtEntry( this, mpDoc, static_cast<const ScCondDateFormatEntry*>( pEntry ) ) );
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
                maEntries.emplace_back(new ScConditionFrmtEntry( this, mpDoc, mpDialogParent, maPos ));
                break;
            case condformat::dialog::COLORSCALE:
                maEntries.emplace_back(new ScColorScale3FrmtEntry( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATABAR:
                maEntries.emplace_back(new ScDataBarFrmtEntry( this, mpDoc, maPos ));
                break;
            case condformat::dialog::ICONSET:
                maEntries.emplace_back(new ScIconSetFrmtEntry( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATE:
                maEntries.emplace_back(new ScDateFrmtEntry( this, mpDoc ));
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

    std::unique_ptr<ScConditionalFormat> pFormat(new ScConditionalFormat(0, mpDoc));
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

    sal_Int32 nIndex = 1;
    for (const auto& item : maEntries)
    {
        if (!item)
            continue;
        item->SetIndex(nIndex);
        item->set_grid_top_attach(nIndex - 1);
        ++nIndex;
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
            itr->reset(new ScColorScale2FrmtEntry(this, mpDoc, maPos));
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::COLORSCALE3)
                return;

            Freeze();
            itr->reset(new ScColorScale3FrmtEntry(this, mpDoc, maPos));
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::DATABAR)
                return;

            Freeze();
            itr->reset(new ScDataBarFrmtEntry(this, mpDoc, maPos));
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::ICONSET)
                return;

            Freeze();
            itr->reset(new ScIconSetFrmtEntry(this, mpDoc, maPos));
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
            itr->reset(new ScColorScale3FrmtEntry(this, mpDoc, maPos));
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::CONDITION)
                return;

            Freeze();
            itr->reset(new ScConditionFrmtEntry(this, mpDoc, mpDialogParent, maPos));
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::FORMULA)
                return;

            Freeze();
            itr->reset(new ScFormulaFrmtEntry(this, mpDoc, mpDialogParent, maPos));
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::DATE)
                return;

            Freeze();
            itr->reset(new ScDateFrmtEntry( this, mpDoc ));
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
    maEntries.emplace_back(new ScConditionFrmtEntry(this, mpDoc, mpDialogParent, maPos));
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

ScCondFormatDlg::ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW,
    weld::Window* pParent, ScViewData* pViewData,
    const ScCondFormatDlgItem* pItem)
        : ScAnyRefDlgController(pB, pCW, pParent,
                        (comphelper::LibreOfficeKit::isMobilePhone(SfxLokHelper::getView())?OUString("modules/scalc/ui/conditionalformatdialogmobile.ui"):OUString("modules/scalc/ui/conditionalformatdialog.ui")),
                        "ConditionalFormatDialog")
    , mpViewData(pViewData)
    , mpDlgItem(static_cast<ScCondFormatDlgItem*>(pItem->Clone()))
    , mpLastEdit(nullptr)
    , mxBtnOk(m_xBuilder->weld_button("ok"))
    , mxBtnAdd(m_xBuilder->weld_button("add"))
    , mxBtnRemove(m_xBuilder->weld_button("delete"))
    , mxBtnUp(m_xBuilder->weld_button("up"))
    , mxBtnDown(m_xBuilder->weld_button("down"))
    , mxBtnCancel(m_xBuilder->weld_button("cancel"))
    , mxFtRange(m_xBuilder->weld_label("ftassign"))
    , mxEdRange(new formula::RefEdit(m_xBuilder->weld_entry("edassign")))
    , mxRbRange(new formula::RefButton(m_xBuilder->weld_button("rbassign")))
    , mxCondFormList(new ScCondFormatList(this, m_xBuilder->weld_scrolled_window("listwindow"),
                                          m_xBuilder->weld_container("list")))
{
    mxEdRange->SetReferences(this, mxFtRange.get());
    mxRbRange->SetReferences(this, mxEdRange.get());

    ScConditionalFormat* pFormat = nullptr;
    mnKey = mpDlgItem->GetIndex();
    if (mpDlgItem->IsManaged() && mpDlgItem->GetConditionalFormatList())
    {
        pFormat = mpDlgItem->GetConditionalFormatList()->GetFormat(mnKey);
    }
    else if (!mpDlgItem->IsManaged())
    {
        ScDocument* pDoc = mpViewData->GetDocument();
        pFormat = pDoc->GetCondFormList(mpViewData->GetTabNo())->GetFormat ( mnKey );
    }

    ScRangeList aRange;
    if (pFormat)
    {
        aRange = pFormat->GetRange();
    }
    else
    {
        // this is for adding a new entry
        mpViewData->GetMarkData().FillRangeListWithMarks(&aRange, false);
        if(aRange.empty())
        {
            ScAddress aPos(mpViewData->GetCurX(), mpViewData->GetCurY(), mpViewData->GetTabNo());
            aRange.push_back(ScRange(aPos));
        }
        mnKey = 0;
    }
    maPos = aRange.GetTopLeftCorner();

    mxCondFormList->init(mpViewData->GetDocument(), pFormat, aRange, maPos, mpDlgItem->GetDialogType());

    mxBtnOk->connect_clicked(LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mxBtnAdd->connect_clicked( LINK( mxCondFormList.get(), ScCondFormatList, AddBtnHdl ) );
    mxBtnRemove->connect_clicked( LINK( mxCondFormList.get(), ScCondFormatList, RemoveBtnHdl ) );
    mxBtnUp->connect_clicked(LINK(mxCondFormList.get(), ScCondFormatList, UpBtnHdl));
    mxBtnDown->connect_clicked(LINK(mxCondFormList.get(), ScCondFormatList, DownBtnHdl));
    mxBtnCancel->connect_clicked( LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mxEdRange->SetModifyHdl( LINK( this, ScCondFormatDlg, EdRangeModifyHdl ) );
    mxEdRange->SetGetFocusHdl( LINK( this, ScCondFormatDlg, RangeGetFocusHdl ) );

    OUString aRangeString;
    aRange.Format(aRangeString, ScRefFlags::VALID, pViewData->GetDocument(),
                    pViewData->GetDocument()->GetAddressConvention());
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

void ScCondFormatDlg::SetReference(const ScRange& rRef, ScDocument*)
{
    formula::RefEdit* pEdit = mpLastEdit;
    if (!mpLastEdit)
        pEdit = mxEdRange.get();

    if (pEdit->GetWidget()->get_sensitive())
    {
        if(rRef.aStart != rRef.aEnd)
            RefInputStart(pEdit);

        ScRefFlags nFlags;
        if (mpLastEdit && mpLastEdit != mxEdRange.get())
            nFlags = ScRefFlags::RANGE_ABS_3D;
        else
            nFlags = ScRefFlags::RANGE_ABS;

        OUString aRefStr(rRef.Format(nFlags, mpViewData->GetDocument(),
            ScAddress::Details(mpViewData->GetDocument()->GetAddressConvention(), 0, 0)));
        if (pEdit != mxEdRange.get())
        {
            Selection sel = pEdit->GetSelection();
            sel.Justify();            // in case of RTL selection
            sel.Max() = sel.Min() + aRefStr.getLength();
            pEdit->GetWidget()->replace_selection(aRefStr);
            pEdit->SetSelection(sel); // to replace it again with next drag event
        }
        else
            pEdit->SetRefString( aRefStr );
        updateTitle();
    }
}

std::unique_ptr<ScConditionalFormat> ScCondFormatDlg::GetConditionalFormat() const
{
    OUString aRangeStr = mxEdRange->GetText();
    if(aRangeStr.isEmpty())
        return nullptr;

    ScRangeList aRange;
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        mpViewData->GetDocument()->GetAddressConvention(), maPos.Tab());
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

    if (!mpDlgItem->IsManaged())
    {
        if(pFormat)
        {
            auto& rRangeList = pFormat->GetRange();
            mpViewData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(mnKey,
                    std::move(pFormat), maPos.Tab(), rRangeList);
        }
        else
            mpViewData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(mnKey,
                    nullptr, maPos.Tab(), ScRangeList());
    }
    else
    {
        ScConditionalFormatList* pList = mpDlgItem->GetConditionalFormatList();
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
        mpViewData->GetViewShell()->GetPool().Put(*mpDlgItem);

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
    if ( mpDlgItem->IsManaged() )
    {
        mpViewData->GetViewShell()->GetPool().Put(*mpDlgItem);
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
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        mpViewData->GetDocument()->GetAddressConvention());
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
