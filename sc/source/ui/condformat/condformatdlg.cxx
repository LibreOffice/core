/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "condformatdlg.hxx"
#include "condformatdlg.hrc"

#include <vcl/vclevent.hxx>
#include <svl/style.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <libxml/tree.h>

#include "anyrefdg.hxx"
#include "document.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"
#include "tabvwsh.hxx"
#include "colorscale.hxx"
#include "colorformat.hxx"
#include "reffact.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "condformatdlgentry.hxx"
#include "condformatdlgitem.hxx"

#include "globstr.hrc"

ScCondFormatList::ScCondFormatList(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle | WB_DIALOGCONTROL)
    , mbHasScrollBar(false)
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
    mpDialogParent.clear();
    mpScrollBar.disposeAndClear();
    for (auto it = maEntries.begin(); it != maEntries.end(); ++it)
        it->disposeAndClear();
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

    if(pFormat)
    {
        size_t nCount = pFormat->size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const ScFormatEntry* pEntry = pFormat->GetEntry(nIndex);
            switch(pEntry->GetType())
            {
                case condformat::CONDITION:
                    {
                        const ScCondFormatEntry* pConditionEntry = static_cast<const ScCondFormatEntry*>( pEntry );
                        if(pConditionEntry->GetOperation() != SC_COND_DIRECT)
                            maEntries.push_back(VclPtr<ScConditionFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos, pConditionEntry ) );
                        else
                            maEntries.push_back(VclPtr<ScFormulaFrmtEntry>::Create( this, mpDoc, pDialogParent, maPos, pConditionEntry ) );

                    }
                    break;
                case condformat::COLORSCALE:
                    {
                        const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>( pEntry );
                        if( pColorScale->size() == 2 )
                            maEntries.push_back(VclPtr<ScColorScale2FrmtEntry>::Create( this, mpDoc, maPos, pColorScale ) );
                        else
                            maEntries.push_back(VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos, pColorScale ) );
                    }
                    break;
                case condformat::DATABAR:
                    maEntries.push_back(VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos, static_cast<const ScDataBarFormat*>( pEntry ) ) );
                    break;
                case condformat::ICONSET:
                    maEntries.push_back(VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos, static_cast<const ScIconSetFormat*>( pEntry ) ) );
                    break;
                case condformat::DATE:
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
    }
    RecalcAll();
    if (!maEntries.empty())
        (*maEntries.begin())->SetActive();

    RecalcAll();
}

VCL_BUILDER_DECL_FACTORY(ScCondFormatList)
{
    WinBits nWinBits = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<ScCondFormatList>::Create(pParent, nWinBits);
}

Size ScCondFormatList::GetOptimalSize() const
{
    return LogicToPixel(Size(290, 185), MAP_APPFONT);
}

void ScCondFormatList::Resize()
{
    Control::Resize();
    RecalcAll();
}

ScConditionalFormat* ScCondFormatList::GetConditionalFormat() const
{
    if(maEntries.empty())
        return nullptr;

    ScConditionalFormat* pFormat = new ScConditionalFormat(0, mpDoc);
    pFormat->SetRange(maRanges);

    for(EntryContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        ScFormatEntry* pEntry = (*itr)->GetEntry();
        if(pEntry)
            pFormat->AddEntry(pEntry);
    }

    return pFormat;
}

void ScCondFormatList::RecalcAll()
{
    sal_Int32 nTotalHeight = 0;
    sal_Int32 nIndex = 1;
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        nTotalHeight += (*itr)->GetSizePixel().Height();
        (*itr)->SetIndex( nIndex );
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
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        (*itr)->SetPosPixel(aPoint);
        Size aSize = (*itr)->GetSizePixel();
        if(mbHasScrollBar)
            aSize.Width() = aCtrlSize.Width() - nSrcBarSize;
        else
            aSize.Width() = aCtrlSize.Width();
        (*itr)->SetSizePixel(aSize);

        aPoint.Y() += (*itr)->GetSizePixel().Height();
    }
}

void ScCondFormatList::DoScroll(long nDelta)
{
    Point aNewPoint = mpScrollBar->GetPosPixel();
    Rectangle aRect(Point(), GetOutputSize());
    aRect.Right() -= mpScrollBar->GetSizePixel().Width();
    Scroll( 0, -nDelta, aRect );
    mpScrollBar->SetPosPixel(aNewPoint);
}

IMPL_LINK_TYPED(ScCondFormatList, ColFormatTypeHdl, ListBox&, rBox, void)
{
    EntryContainer::iterator itr = maEntries.begin();
    for(; itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return;

    sal_Int32 nPos = rBox.GetSelectEntryPos();
    switch(nPos)
    {
        case 0:
            if((*itr)->GetType() == condformat::entry::COLORSCALE2)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale2FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::COLORSCALE3)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::DATABAR)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::ICONSET)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        default:
            break;
    }
    mpDialogParent->InvalidateRefData();
    (*itr)->SetActive();
    RecalcAll();
}

IMPL_LINK_TYPED(ScCondFormatList, TypeListHdl, ListBox&, rBox, void)
{
    //Resolves: fdo#79021 At this point we are still inside the ListBox Select.
    //If we call maEntries.replace here then the pBox will be deleted before it
    //has finished Select and will crash on accessing its deleted this. So Post
    //to do the real work after the Select has completed
    Application::PostUserEvent(LINK(this, ScCondFormatList, AfterTypeListHdl), &rBox, true);
}

IMPL_LINK_TYPED(ScCondFormatList, AfterTypeListHdl, void*, p, void)
{
    ListBox* pBox = static_cast<ListBox*>(p);
    EntryContainer::iterator itr = maEntries.begin();
    for(; itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return;

    sal_Int32 nPos = pBox->GetSelectEntryPos();
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
            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create(this, mpDoc, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::CONDITION)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScConditionFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::FORMULA)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScFormulaFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::DATE)
                return;

            itr->disposeAndClear();
            *itr = VclPtr<ScDateFrmtEntry>::Create( this, mpDoc );
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;

    }
    RecalcAll();
}

IMPL_LINK_NOARG_TYPED( ScCondFormatList, AddBtnHdl, Button*, void )
{
    VclPtr<ScCondFrmtEntry> pNewEntry = VclPtr<ScConditionFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
    maEntries.push_back( pNewEntry );
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        (*itr)->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    pNewEntry->SetActive();
    RecalcAll();
}

IMPL_LINK_NOARG_TYPED( ScCondFormatList, RemoveBtnHdl, Button*, void )
{
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
        {
            itr->disposeAndClear();
            maEntries.erase(itr);
            break;
        }
    }
    mpDialogParent->InvalidateRefData();
    RecalcAll();
}

IMPL_LINK_TYPED( ScCondFormatList, EntrySelectHdl, ScCondFrmtEntry&, rEntry, void )
{
    if(rEntry.IsSelected())
        return;

    //A child has focus, but we will hide that, so regrab to whatever new thing gets
    //shown instead of leaving it stuck in the inaccessible hidden element
    bool bReGrabFocus = HasChildPathFocus();
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        (*itr)->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    rEntry.SetActive();
    RecalcAll();
    if (bReGrabFocus)
        GrabFocus();
}

IMPL_LINK_NOARG_TYPED( ScCondFormatList, ScrollHdl, ScrollBar*, void )
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
            ScRange* pRange = new ScRange(aPos);
            aRange.push_back(pRange);
        }
        mnKey = 0;
    }
    maPos = aRange.GetTopLeftCorner();

    get(mpCondFormList, "list");
    mpCondFormList->init(mpViewData->GetDocument(), this, pFormat, aRange, maPos, mpDlgItem->GetDialogType());

    mpBtnOk->SetClickHdl(LINK(this, ScCondFormatDlg, BtnPressedHdl ) );
    mpBtnAdd->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, AddBtnHdl ) );
    mpBtnRemove->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, RemoveBtnHdl ) );
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
    if (mpLastEdit && mpLastEdit != mpEdRange)
        return false;

    return true;
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
        pEdit->SetRefString( aRefStr );
        updateTitle();
    }
}

ScConditionalFormat* ScCondFormatDlg::GetConditionalFormat() const
{
    OUString aRangeStr = mpEdRange->GetText();
    if(aRangeStr.isEmpty())
        return nullptr;

    ScRangeList aRange;
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        ScRefFlags::VALID, mpViewData->GetDocument()->GetAddressConvention(), maPos.Tab());
    ScConditionalFormat* pFormat = mpCondFormList->GetConditionalFormat();

    if((nFlags & ScRefFlags::VALID) && !aRange.empty() && pFormat)
        pFormat->SetRange(aRange);
    else
    {
        delete pFormat;
        pFormat = nullptr;
    }

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
    ScConditionalFormat* pFormat = GetConditionalFormat();

    if (!mpDlgItem->IsManaged())
    {
        if(pFormat)
            mpViewData->GetDocShell()->GetDocFunc().ReplaceConditionalFormat(mnKey,
                    pFormat, maPos.Tab(), pFormat->GetRange());
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
            pList->InsertNew(pFormat);
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

IMPL_LINK_TYPED( ScCondFormatDlg, EdRangeModifyHdl, Edit&, rEdit, void )
{
    OUString aRangeStr = rEdit.GetText();
    ScRangeList aRange;
    ScRefFlags nFlags = aRange.Parse(aRangeStr, mpViewData->GetDocument(),
        ScRefFlags::VALID, mpViewData->GetDocument()->GetAddressConvention());
    if(nFlags & ScRefFlags::VALID)
        rEdit.SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    else
        rEdit.SetControlBackground(COL_LIGHTRED);

    updateTitle();
}

IMPL_LINK_TYPED( ScCondFormatDlg, RangeGetFocusHdl, Control&, rControl, void )
{
    mpLastEdit = static_cast<formula::RefEdit*>(&rControl);
}

IMPL_LINK_TYPED( ScCondFormatDlg, BtnPressedHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnOk)
        OkPressed();
    else if (pBtn == mpBtnCancel)
        CancelPressed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
