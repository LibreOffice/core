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
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/lstbox.hxx>

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

ScCondFormatList::ScCondFormatList(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle | WB_DIALOGCONTROL)
    , mbHasScrollBar(false)
    , mbFrozen(false)
    , mbNewEntry(false)
    , mpScrollBar(VclPtr<ScrollBar>::Create(this, WB_VERT ))
    , mpDoc(nullptr)
    , mpDialogParent(nullptr)
{
    mpScrollBar->SetScrollHdl( LINK( this, ScCondFormatList, ScrollHdl ) );
    mpScrollBar->EnableDrag();
    SetControlBackground( GetSettings().GetStyleSettings().GetWindowColor() );
    SetBackground(GetControlBackground());
}

ScCondFormatList::~ScCondFormatList()
{
    disposeOnce();
}

void ScCondFormatList::dispose()
{
    Freeze();
    mpDialogParent.clear();
    mpScrollBar.disposeAndClear();
    for (auto& rxEntry : maEntries)
        rxEntry.disposeAndClear();
    maEntries.clear();
    Control::dispose();
}

void ScCondFormatList::init(ScDocument* pDoc, ScCondFormatDlg* pDialogParent,
        const ScConditionalFormat* pFormat, const ScRangeList& rRanges,
        const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType)
{
    mpDialogParent = pDialogParent;
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
                    {
                        const ScCondFormatEntry* pConditionEntry = static_cast<const ScCondFormatEntry*>( pEntry );
                        if(pConditionEntry->GetOperation() != ScConditionMode::Direct)
                            maEntries.push_back(VclPtr<ScConditionFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos, pConditionEntry ) );
                        else
                            maEntries.push_back(VclPtr<ScFormulaFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos, pConditionEntry ) );

                    }
                    break;
                case ScFormatEntry::Type::Colorscale:
                    {
                        const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>( pEntry );
                        if( pColorScale->size() == 2 )
                            maEntries.push_back(VclPtr<ScColorScale2FrmtEntry>::Create( this, mpDoc, maPos, pColorScale ) );
                        else
                            maEntries.push_back(VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos, pColorScale ) );
                    }
                    break;
                case ScFormatEntry::Type::Databar:
                    maEntries.push_back(VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos, static_cast<const ScDataBarFormat*>( pEntry ) ) );
                    break;
                case ScFormatEntry::Type::Iconset:
                    maEntries.push_back(VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos, static_cast<const ScIconSetFormat*>( pEntry ) ) );
                    break;
                case ScFormatEntry::Type::Date:
                    maEntries.push_back(VclPtr<ScDateFrmtEntry>::Create( this, mpDoc, static_cast<const ScCondDateFormatEntry*>( pEntry ) ) );
                    break;
            }
        }
        if(nCount)
            EntrySelectHdl(*maEntries[0].get());
    }
    else
    {
        switch(eType)
        {
            case condformat::dialog::CONDITION:
                maEntries.push_back(VclPtr<ScConditionFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos ));
                break;
            case condformat::dialog::COLORSCALE:
                maEntries.push_back(VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATABAR:
                maEntries.push_back(VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos ));
                break;
            case condformat::dialog::ICONSET:
                maEntries.push_back(VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATE:
                maEntries.push_back(VclPtr<ScDateFrmtEntry>::Create( this, mpDoc ));
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

VCL_BUILDER_FACTORY_CONSTRUCTOR(ScCondFormatList, 0)

Size ScCondFormatList::GetOptimalSize() const
{
    return LogicToPixel(Size(300, 185), MapMode(MapUnit::MapAppFont));
}

void ScCondFormatList::Resize()
{
    Control::Resize();
    RecalcAll();
}

void ScCondFormatList::queue_resize(StateChangedType eReason)
{
    Control::queue_resize(eReason);
    if (!mpDialogParent) //detects that this is during dispose
        return;
    RecalcAll();
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

    sal_Int32 nTotalHeight = 0;
    sal_Int32 nIndex = 1;
    for (const auto& item : maEntries)
    {
        if (!item)
            continue;
        nTotalHeight += item->GetSizePixel().Height();
        item->SetIndex(nIndex);
        ++nIndex;
    }

    Size aCtrlSize = GetOutputSize();
    long nSrcBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if(nTotalHeight > GetSizePixel().Height())
    {
        mbHasScrollBar = true;
        mpScrollBar->SetPosSizePixel(Point(aCtrlSize.Width() -nSrcBarSize, 0),
                Size(nSrcBarSize, aCtrlSize.Height()) );
        mpScrollBar->SetRangeMax(nTotalHeight);
        mpScrollBar->SetVisibleSize(aCtrlSize.Height());
        mpScrollBar->Show();
    }
    else
    {
        mbHasScrollBar = false;
        mpScrollBar->Hide();
    }

    Point aPoint(0,-1*mpScrollBar->GetThumbPos());
    for (const auto& item : maEntries)
    {
        if (!item)
            continue;
        item->SetPosPixel(aPoint);
        Size aSize = item->GetSizePixel();
        if(mbHasScrollBar)
            aSize.setWidth( aCtrlSize.Width() - nSrcBarSize );
        else
            aSize.setWidth( aCtrlSize.Width() );
        item->SetSizePixel(aSize);

        aPoint.AdjustY(item->GetSizePixel().Height() );
    }
}

void ScCondFormatList::DoScroll(long nDelta)
{
    Point aNewPoint = mpScrollBar->GetPosPixel();
    tools::Rectangle aRect(Point(), GetOutputSize());
    aRect.AdjustRight( -(mpScrollBar->GetSizePixel().Width()) );
    Scroll( 0, -nDelta, aRect );
    mpScrollBar->SetPosPixel(aNewPoint);
}

IMPL_LINK(ScCondFormatList, ColFormatTypeHdl, ListBox&, rBox, void)
{
    EntryContainer::iterator itr = std::find_if(maEntries.begin(), maEntries.end(),
        [](const VclPtr<ScCondFrmtEntry>& widget) { return widget->IsSelected(); });
    if(itr == maEntries.end())
        return;

    sal_Int32 nPos = rBox.GetSelectedEntryPos();
    switch(nPos)
    {
        case 0:
            if((*itr)->GetType() == condformat::entry::COLORSCALE2)
                return;

            Freeze();
            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale2FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::COLORSCALE3)
                return;

            Freeze();
            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::DATABAR)
                return;

            Freeze();
            itr->disposeAndClear();
            *itr = VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::ICONSET)
                return;

            Freeze();
            itr->disposeAndClear();
            *itr = VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        default:
            break;
    }
    mpDialogParent->InvalidateRefData();
    (*itr)->SetActive();
    Thaw();
    RecalcAll();
}

IMPL_LINK(ScCondFormatList, TypeListHdl, ListBox&, rBox, void)
{
    //Resolves: fdo#79021 At this point we are still inside the ListBox Select.
    //If we call maEntries.replace here then the pBox will be deleted before it
    //has finished Select and will crash on accessing its deleted this. So Post
    //to do the real work after the Select has completed
    Application::PostUserEvent(LINK(this, ScCondFormatList, AfterTypeListHdl), &rBox, true);
}

IMPL_LINK(ScCondFormatList, AfterTypeListHdl, void*, p, void)
{
    ListBox* pBox = static_cast<ListBox*>(p);
    EntryContainer::iterator itr = std::find_if(maEntries.begin(), maEntries.end(),
        [](const VclPtr<ScCondFrmtEntry>& widget) { return widget->IsSelected(); });
    if(itr == maEntries.end())
        return;

    sal_Int32 nPos = pBox->GetSelectedEntryPos();
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
            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create(this, mpDoc, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::CONDITION)
                return;

            Freeze();
            itr->disposeAndClear();
            *itr = VclPtr<ScConditionFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::FORMULA)
                return;

            Freeze();
            itr->disposeAndClear();
            *itr = VclPtr<ScFormulaFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::DATE)
                return;

            Freeze();
            itr->disposeAndClear();
            *itr = VclPtr<ScDateFrmtEntry>::Create( this, mpDoc );
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;

    }
    Thaw();
    RecalcAll();
}

IMPL_LINK_NOARG( ScCondFormatList, AddBtnHdl, Button*, void )
{
    Freeze();
    VclPtr<ScCondFrmtEntry> pNewEntry = VclPtr<ScConditionFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
    maEntries.push_back( pNewEntry );
    for(auto& rxEntry : maEntries)
    {
        rxEntry->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    pNewEntry->SetActive();
    mpDialogParent->OnSelectionChange(maEntries.size() - 1, maEntries.size());
    Thaw();
    RecalcAll();
}

IMPL_LINK_NOARG( ScCondFormatList, RemoveBtnHdl, Button*, void )
{
    Freeze();
    auto itr = std::find_if(maEntries.begin(), maEntries.end(),
        [](const VclPtr<ScCondFrmtEntry>& widget) { return widget->IsSelected(); });
    if (itr != maEntries.end())
    {
        itr->disposeAndClear();
        maEntries.erase(itr);
    }
    mpDialogParent->InvalidateRefData();
    mpDialogParent->OnSelectionChange(0, maEntries.size(), false);
    Thaw();
    RecalcAll();
}

IMPL_LINK_NOARG(ScCondFormatList, UpBtnHdl, Button*, void)
{
    Freeze();
    size_t index = 0;
    for (size_t i = 0; i < maEntries.size(); i++)
    {
        auto widget = maEntries[i];
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

IMPL_LINK_NOARG(ScCondFormatList, DownBtnHdl, Button*, void)
{
    Freeze();
    size_t index = 0;
    for (size_t i = 0; i < maEntries.size(); i++)
    {
        auto widget = maEntries[i];
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
    //A child has focus, but we will hide that, so regrab to whatever new thing gets
    //shown instead of leaving it stuck in the inaccessible hidden element
    bool bReGrabFocus = HasChildPathFocus();
    size_t index = 0;
    for(size_t i = 0; i < maEntries.size(); i++)
    {
        if (maEntries[i] == &rEntry)
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
    if (bReGrabFocus)
        GrabFocus();
}

IMPL_LINK_NOARG( ScCondFormatList, ScrollHdl, ScrollBar*, void )
{
    DoScroll(mpScrollBar->GetDelta());
}

ScCondFormatDlg::ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW,
    vcl::Window* pParent, ScViewData* pViewData,
    const ScCondFormatDlgItem* pItem)
        : ScAnyRefDlg(pB, pCW, pParent, "ConditionalFormatDialog",
                        "modules/scalc/ui/conditionalformatdialog.ui")
    , mpViewData(pViewData)
    , mpLastEdit(nullptr)
    , mpDlgItem(static_cast<ScCondFormatDlgItem*>(pItem->Clone()))
{
    get(mpBtnOk, "ok");
    get(mpBtnAdd, "add");
    get(mpBtnUp, "up");
    get(mpBtnDown, "down");
    get(mpBtnRemove, "delete");
    get(mpBtnCancel, "cancel");

    get(mpFtRange, "ftassign");
    get(mpEdRange, "edassign");
    mpEdRange->SetReferences(this, mpFtRange);

    get(mpRbRange, "rbassign");
    mpRbRange->SetReferences(this, mpEdRange);

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

    get(mpCondFormList, "list");
    mpCondFormList->init(mpViewData->GetDocument(), this, pFormat, aRange, maPos, mpDlgItem->GetDialogType());

    // tdf#114603: enable setting the background to a different color;
    // relevant for GTK; see also #i75179#
    mpEdRange->SetForceControlBackground(true);

    mpBtnOk->SetClickHdl(LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mpBtnAdd->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, AddBtnHdl ) );
    mpBtnRemove->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, RemoveBtnHdl ) );
    mpBtnUp->SetClickHdl(LINK(mpCondFormList, ScCondFormatList, UpBtnHdl));
    mpBtnDown->SetClickHdl(LINK(mpCondFormList, ScCondFormatList, DownBtnHdl));
    mpBtnCancel->SetClickHdl( LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mpEdRange->SetModifyHdl( LINK( this, ScCondFormatDlg, EdRangeModifyHdl ) );
    mpEdRange->SetGetFocusHdl( LINK( this, ScCondFormatDlg, RangeGetFocusHdl ) );

    OUString aRangeString;
    aRange.Format(aRangeString, ScRefFlags::VALID, pViewData->GetDocument(),
                    pViewData->GetDocument()->GetAddressConvention());
    mpEdRange->SetText(aRangeString);

    msBaseTitle = GetText();
    updateTitle();
}

void ScCondFormatDlg::updateTitle()
{
    OUString aTitle = msBaseTitle + " " + mpEdRange->GetText();

    SetText(aTitle);
}

ScCondFormatDlg::~ScCondFormatDlg()
{
    disposeOnce();
}

void ScCondFormatDlg::dispose()
{
    mpBtnOk.clear();
    mpBtnAdd.clear();
    mpBtnRemove.clear();
    mpBtnUp.clear();
    mpBtnDown.clear();
    mpBtnCancel.clear();
    mpFtRange.clear();
    mpEdRange.clear();
    mpRbRange.clear();
    mpCondFormList.clear();
    mpLastEdit.clear();

    ScAnyRefDlg::dispose();
}

void ScCondFormatDlg::SetActive()
{
    if(mpLastEdit)
        mpLastEdit->GrabFocus();
    else
        mpEdRange->GrabFocus();

    RefInputDone();
}

void ScCondFormatDlg::RefInputDone( bool bForced )
{
    ScAnyRefDlg::RefInputDone(bForced);

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
    return !mpLastEdit || mpLastEdit == mpEdRange;
}

bool ScCondFormatDlg::IsRefInputMode() const
{
    return mpEdRange->IsEnabled();
}

void ScCondFormatDlg::SetReference(const ScRange& rRef, ScDocument*)
{
    formula::RefEdit* pEdit = mpLastEdit;
    if (!mpLastEdit)
        pEdit = mpEdRange;

    if( pEdit->IsEnabled() )
    {
        if(rRef.aStart != rRef.aEnd)
            RefInputStart(pEdit);

        ScRefFlags nFlags;
        if (mpLastEdit && mpLastEdit != mpEdRange)
            nFlags = ScRefFlags::RANGE_ABS_3D;
        else
            nFlags = ScRefFlags::RANGE_ABS;

        OUString aRefStr(rRef.Format(nFlags, mpViewData->GetDocument(),
            ScAddress::Details(mpViewData->GetDocument()->GetAddressConvention(), 0, 0)));
        if (pEdit != mpEdRange)
        {
            Selection sel = pEdit->GetSelection();
            sel.Justify();            // in case of RtL selection
            sel.Max() = sel.Min() + aRefStr.getLength();
            pEdit->ReplaceSelected(aRefStr);
            pEdit->SetSelection(sel); // to replace it again with next drag event
        }
        else
            pEdit->SetRefString( aRefStr );
        updateTitle();
    }
}

std::unique_ptr<ScConditionalFormat> ScCondFormatDlg::GetConditionalFormat() const
{
    OUString aRangeStr = mpEdRange->GetText();
    if(aRangeStr.isEmpty())
        return nullptr;

    ScRangeList aRange;
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        mpViewData->GetDocument()->GetAddressConvention(), maPos.Tab());
    mpCondFormList->SetRange(aRange);
    std::unique_ptr<ScConditionalFormat> pFormat = mpCondFormList->GetConditionalFormat();

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
bool ScCondFormatDlg::Close()
{
    return DoClose( ScCondFormatDlgWrapper::GetChildWindowId() );
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
    Close();
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
    Close();
}

void ScCondFormatDlg::OnSelectionChange(size_t nIndex, size_t nSize, bool bSelected)
{
    if (nSize <= 1 || !bSelected)
    {
        mpBtnUp->Enable(false);
        mpBtnDown->Enable(false);
    }
    else
    {
        mpBtnUp->Enable(nIndex != 0);
        mpBtnDown->Enable(nIndex < nSize - 1);
    }
}

IMPL_LINK( ScCondFormatDlg, EdRangeModifyHdl, Edit&, rEdit, void )
{
    OUString aRangeStr = rEdit.GetText();
    ScRangeList aRange;
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        mpViewData->GetDocument()->GetAddressConvention());
    if(nFlags & ScRefFlags::VALID)
    {
        rEdit.SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
        mpBtnOk->Enable(true);
    }
    else
    {
        rEdit.SetControlBackground(COL_LIGHTRED);
        mpBtnOk->Enable(false);
    }

    updateTitle();
}

IMPL_LINK( ScCondFormatDlg, RangeGetFocusHdl, Control&, rControl, void )
{
    mpLastEdit = static_cast<formula::RefEdit*>(&rControl);
}

IMPL_LINK( ScCondFormatDlg, BtnPressedHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnOk)
        OkPressed();
    else if (pBtn == mpBtnCancel)
        CancelPressed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
