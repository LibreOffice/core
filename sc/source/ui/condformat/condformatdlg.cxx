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

#include "globstr.hrc"

ScCondFormatList::ScCondFormatList(vcl::Window* pParent, WinBits nStyle)
    : Control(pParent, nStyle | WB_DIALOGCONTROL)
    , mbHasScrollBar(false)
    , mpScrollBar(VclPtr<ScrollBar>::Create(this, WB_VERT ))
    , mpDoc(NULL)
    , mpDialogParent(NULL)
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
            EntrySelectHdl(maEntries[0].get());
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

    return new ScCondFormatList(pParent, nWinBits);
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
        return NULL;

    ScConditionalFormat* pFormat = new ScConditionalFormat(0, mpDoc);
    for(EntryContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        ScFormatEntry* pEntry = (*itr)->GetEntry();
        if(pEntry)
            pFormat->AddEntry(pEntry);
    }

    pFormat->SetRange(maRanges);

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

IMPL_LINK(ScCondFormatList, ColFormatTypeHdl, ListBox*, pBox)
{
    EntryContainer::iterator itr = maEntries.begin();
    for(; itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return 0;

    sal_Int32 nPos = pBox->GetSelectEntryPos();
    switch(nPos)
    {
        case 0:
            if((*itr)->GetType() == condformat::entry::COLORSCALE2)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale2FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::COLORSCALE3)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::DATABAR)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScDataBarFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::ICONSET)
                return 0;

            itr->disposeAndClear();
            *itr = VclPtr<ScIconSetFrmtEntry>::Create( this, mpDoc, maPos );
            break;
        default:
            break;
    }
    mpDialogParent->InvalidateRefData();
    (*itr)->SetActive();
    RecalcAll();
    return 0;
}

IMPL_LINK(ScCondFormatList, TypeListHdl, ListBox*, pBox)
{
    //Resolves: fdo#79021 At this point we are still inside the ListBox Select.
    //If we call maEntries.replace here then the pBox will be deleted before it
    //has finished Select and will crash on accessing its deleted this. So Post
    //to do the real work after the Select has completed
    Application::PostUserEvent(LINK(this, ScCondFormatList, AfterTypeListHdl), pBox);
    return 0;
}

IMPL_LINK(ScCondFormatList, AfterTypeListHdl, ListBox*, pBox)
{
    EntryContainer::iterator itr = maEntries.begin();
    for(; itr != maEntries.end(); ++itr)
    {
        if((*itr)->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return 0;;

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
                    return 0;
            }
            *itr = VclPtr<ScColorScale3FrmtEntry>::Create(this, mpDoc, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 1:
            if((*itr)->GetType() == condformat::entry::CONDITION)
                return 0;

            *itr = VclPtr<ScConditionFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 2:
            if((*itr)->GetType() == condformat::entry::FORMULA)
                return 0;

            *itr = VclPtr<ScFormulaFrmtEntry>::Create(this, mpDoc, mpDialogParent, maPos);
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;
        case 3:
            if((*itr)->GetType() == condformat::entry::DATE)
                return 0;

            *itr = VclPtr<ScDateFrmtEntry>::Create( this, mpDoc );
            mpDialogParent->InvalidateRefData();
            (*itr)->SetActive();
            break;

    }
    RecalcAll();
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatList, AddBtnHdl )
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
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatList, RemoveBtnHdl )
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
    return 0;
}

IMPL_LINK( ScCondFormatList, EntrySelectHdl, ScCondFrmtEntry*, pEntry )
{
    if(pEntry->IsSelected())
        return 0;

    //A child has focus, but we will hide that, so regrab to whatever new thing gets
    //shown instead of leaving it stuck in the inaccessible hidden element
    bool bReGrabFocus = HasChildPathFocus();
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        (*itr)->SetInactive();
    }
    mpDialogParent->InvalidateRefData();
    pEntry->SetActive();
    RecalcAll();
    if (bReGrabFocus)
        GrabFocus();
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatList, ScrollHdl )
{
    DoScroll(mpScrollBar->GetDelta());
    return 0;
}

//ScCondFormatDlg

ScCondFormatDlg::ScCondFormatDlg(vcl::Window* pParent, ScDocument* pDoc,
    const ScConditionalFormat* pFormat, const ScRangeList& rRange,
    const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType)
    : ScAnyRefModalDlg(pParent, "ConditionalFormatDialog",
        "modules/scalc/ui/conditionalformatdialog.ui")
    , maPos(rPos)
    , mpDoc(pDoc)
    , mpLastEdit(NULL)
{
    get(mpBtnAdd, "add");
    get(mpBtnRemove, "delete");

    get(mpFtRange, "ftassign");
    get(mpEdRange, "edassign");
    mpEdRange->SetReferences(this, mpFtRange);

    get(mpRbRange, "rbassign");
    mpRbRange->SetReferences(this, mpEdRange);

    get(mpCondFormList, "list");
    mpCondFormList->init(pDoc, this, pFormat, rRange, rPos, eType);

    OUStringBuffer aTitle( GetText() );
    aTitle.append(" ");
    OUString aRangeString;
    rRange.Format(aRangeString, SCA_VALID, pDoc, pDoc->GetAddressConvention());
    aTitle.append(aRangeString);
    SetText(aTitle.makeStringAndClear());
    mpBtnAdd->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, AddBtnHdl ) );
    mpBtnRemove->SetClickHdl( LINK( mpCondFormList, ScCondFormatList, RemoveBtnHdl ) );
    mpEdRange->SetModifyHdl( LINK( this, ScCondFormatDlg, EdRangeModifyHdl ) );
    mpEdRange->SetGetFocusHdl( LINK( this, ScCondFormatDlg, RangeGetFocusHdl ) );
    mpEdRange->SetLoseFocusHdl( LINK( this, ScCondFormatDlg, RangeLoseFocusHdl ) );

    mpEdRange->SetText(aRangeString);

    SC_MOD()->PushNewAnyRefDlg(this);
}

ScCondFormatDlg::~ScCondFormatDlg()
{
    disposeOnce();
}

void ScCondFormatDlg::dispose()
{
    SC_MOD()->PopAnyRefDlg();
    mpBtnAdd.clear();
    mpBtnRemove.clear();
    mpFtRange.clear();
    mpEdRange.clear();
    mpRbRange.clear();
    mpCondFormList.clear();
    mpLastEdit.clear();

    ScAnyRefModalDlg::dispose();
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
    ScAnyRefModalDlg::RefInputDone(bForced);
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

#define ABS_SREF          SCA_VALID \
    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
    | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

void ScCondFormatDlg::SetReference(const ScRange& rRef, ScDocument*)
{
    formula::RefEdit* pEdit = mpLastEdit;
    if (!mpLastEdit)
        pEdit = mpEdRange;

    if( pEdit->IsEnabled() )
    {
        if(rRef.aStart != rRef.aEnd)
            RefInputStart(pEdit);

        sal_uInt16 n = 0;
        if (mpLastEdit && mpLastEdit != mpEdRange)
            n = ABS_DREF3D;
        else
            n = ABS_DREF;

        OUString aRefStr(rRef.Format(n, mpDoc, ScAddress::Details(mpDoc->GetAddressConvention(), 0, 0)));
        pEdit->SetRefString( aRefStr );
    }
}

ScConditionalFormat* ScCondFormatDlg::GetConditionalFormat() const
{
    OUString aRangeStr = mpEdRange->GetText();
    if(aRangeStr.isEmpty())
        return NULL;

    ScRangeList aRange;
    sal_uInt16 nFlags = aRange.Parse(aRangeStr, mpDoc, SCA_VALID, mpDoc->GetAddressConvention(), maPos.Tab());
    ScConditionalFormat* pFormat = mpCondFormList->GetConditionalFormat();

    if(nFlags & SCA_VALID && !aRange.empty() && pFormat)
        pFormat->SetRange(aRange);
    else
    {
        delete pFormat;
        pFormat = NULL;
    }

    return pFormat;
}

void ScCondFormatDlg::InvalidateRefData()
{
    mpLastEdit = NULL;
}

IMPL_LINK( ScCondFormatDlg, EdRangeModifyHdl, Edit*, pEdit )
{
    OUString aRangeStr = pEdit->GetText();
    ScRangeList aRange;
    sal_uInt16 nFlags = aRange.Parse(aRangeStr, mpDoc, SCA_VALID, mpDoc->GetAddressConvention());
    if(nFlags & SCA_VALID)
        pEdit->SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    else
        pEdit->SetControlBackground(COL_LIGHTRED);
    return 0;
}

IMPL_LINK( ScCondFormatDlg, RangeGetFocusHdl, formula::RefEdit*, pEdit )
{
    mpLastEdit = pEdit;
    return 0;
}

IMPL_STATIC_LINK_NOARG( ScCondFormatDlg, RangeLoseFocusHdl )
{
    //mpLastEdit = NULL;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
