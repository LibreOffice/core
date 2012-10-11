/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

#include "anyrefdg.hxx"
#include "document.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"
#include "tabvwsh.hxx"
#include "conditio.hxx"
#include "colorscale.hxx"
#include "colorformat.hxx"
#include "reffact.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "condformatdlgentry.hxx"

#include "globstr.hrc"

ScCondFormatList::ScCondFormatList(Window* pParent, const ResId& rResId, ScDocument* pDoc, const ScConditionalFormat* pFormat,
                                const ScRangeList& rRanges, const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType):
    Control(pParent, rResId),
    mbHasScrollBar(false),
    mpScrollBar(new ScrollBar(this, WB_VERT )),
    mpDoc(pDoc),
    maPos(rPos),
    maRanges(rRanges)
{
    mpScrollBar->SetScrollHdl( LINK( this, ScCondFormatList, ScrollHdl ) );
    mpScrollBar->EnableDrag();

    if(pFormat)
    {
        size_t nCount = pFormat->size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const ScFormatEntry* pEntry = pFormat->GetEntry(nIndex);
            switch(pEntry->GetType())
            {
                case condformat::CONDITION:
                    maEntries.push_back(new ScConditionFrmtEntry( this, mpDoc, maPos, static_cast<const ScCondFormatEntry*>( pEntry ) ) );
                    break;
                case condformat::COLORSCALE:
                    maEntries.push_back(new ScColorScale3FrmtEntry( this, mpDoc, maPos, static_cast<const ScColorScaleFormat*>( pEntry ) ) );
                    break;
                case condformat::DATABAR:
                    maEntries.push_back(new ScDataBarFrmtEntry( this, mpDoc, maPos, static_cast<const ScDataBarFormat*>( pEntry ) ) );
                    break;
            }
        }
    }
    else
    {
        switch(eType)
        {
            case condformat::dialog::CONDITION:
                maEntries.push_back(new ScConditionFrmtEntry( this, mpDoc, maPos ));
                break;
            case condformat::dialog::COLORSCALE:
                maEntries.push_back(new ScColorScale3FrmtEntry( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATABAR:
                maEntries.push_back(new ScDataBarFrmtEntry( this, mpDoc, maPos ));
                break;
            default:
                break;
        }
    }
    if (!maEntries.empty())
        maEntries.begin()->SetActive();

    RecalcAll();
    FreeResource();
}

ScConditionalFormat* ScCondFormatList::GetConditionalFormat() const
{
    if(maEntries.empty())
        return NULL;

    ScConditionalFormat* pFormat = new ScConditionalFormat(0, mpDoc);
    for(EntryContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        ScFormatEntry* pEntry = itr->GetEntry();
        if(pEntry)
            pFormat->AddEntry(pEntry);
    }

    pFormat->AddRange(maRanges);

    return pFormat;
}

void ScCondFormatList::RecalcAll()
{
    sal_Int32 nTotalHeight = 0;
    sal_Int32 nIndex = 1;
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        nTotalHeight += itr->GetSizePixel().Height();
        itr->SetIndex( nIndex );
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
        itr->SetPosPixel(aPoint);
        Size aSize = itr->GetSizePixel();
        if(mbHasScrollBar)
            aSize.Width() = aCtrlSize.Width() - nSrcBarSize;
        else
            aSize.Width() = aCtrlSize.Width();
        itr->SetSizePixel(aSize);

        aPoint.Y() += itr->GetSizePixel().Height();
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
        if(itr->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return 0;

    sal_Int32 nPos = pBox->GetSelectEntryPos();
    switch(nPos)
    {
        case 0:
            if(itr->GetType() == condformat::entry::COLORSCALE2)
                return 0;

            maEntries.replace( itr, new ScColorScale2FrmtEntry( this, mpDoc, maPos ) );
            break;
        case 1:
            if(itr->GetType() == condformat::entry::COLORSCALE3)
                return 0;

            maEntries.replace( itr, new ScColorScale3FrmtEntry( this, mpDoc, maPos ) );
            break;
        case 2:
            if(itr->GetType() == condformat::entry::DATABAR)
                return 0;

            maEntries.replace( itr, new ScDataBarFrmtEntry( this, mpDoc, maPos ) );
            break;
        default:
            break;
    }
    static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
    itr->SetActive();
    RecalcAll();
    return 0;
}

IMPL_LINK(ScCondFormatList, TypeListHdl, ListBox*, pBox)
{
    EntryContainer::iterator itr = maEntries.begin();
    for(; itr != maEntries.end(); ++itr)
    {
        if(itr->IsSelected())
            break;
    }
    if(itr == maEntries.end())
        return 0;;

    sal_Int32 nPos = pBox->GetSelectEntryPos();
    switch(nPos)
    {
        case 0:
            switch(itr->GetType())
            {
                case condformat::entry::FORMULA:
                case condformat::entry::CONDITION:
                    break;
                case condformat::entry::COLORSCALE2:
                case condformat::entry::COLORSCALE3:
                case condformat::entry::DATABAR:
                    return 0;
            }
            maEntries.replace( itr, new ScColorScale3FrmtEntry(this, mpDoc, maPos));
            static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
            itr->SetActive();
            break;
        case 1:
            if(itr->GetType() == condformat::entry::CONDITION)
                return 0;

            maEntries.replace( itr, new ScConditionFrmtEntry(this, mpDoc, maPos));
            static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
            itr->SetActive();
            break;
        case 2:
            if(itr->GetType() == condformat::entry::FORMULA)
                return 0;

            maEntries.replace( itr, new ScFormulaFrmtEntry(this, mpDoc, maPos));
            static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
            itr->SetActive();
            break;
    }
    RecalcAll();
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatList, AddBtnHdl )
{
    ScCondFrmtEntry* pNewEntry = new ScConditionFrmtEntry(this, mpDoc, maPos);
    maEntries.push_back( pNewEntry );
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        itr->SetInactive();
    }
    static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
    pNewEntry->SetActive();
    RecalcAll();
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatList, RemoveBtnHdl )
{
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        if(itr->IsSelected())
        {
            maEntries.erase(itr);
            break;
        }
    }
    static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
    RecalcAll();
    return 0;
}

IMPL_LINK( ScCondFormatList, EntrySelectHdl, ScCondFrmtEntry*, pEntry )
{
    if(pEntry->IsSelected())
        return 0;

    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        itr->SetInactive();
    }
    static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
    pEntry->SetActive();
    RecalcAll();
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatList, ScrollHdl )
{
    DoScroll(mpScrollBar->GetDelta());
    return 0;
}

//---------------------------------------------------
//ScCondFormatDlg
//---------------------------------------------------

ScCondFormatDlg::ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW, Window* pParent, ScDocument* pDoc, const ScConditionalFormat* pFormat, const ScRangeList& rRange,
                                    const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType):
    ScAnyRefDlg(pB, pCW, pParent, RID_SCDLG_CONDFORMAT ),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnRemove( this, ScResId( BTN_REMOVE ) ),
    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maFtRange( this, ScResId( FT_RANGE ) ),
    maEdRange( this, this, ScResId( ED_RANGE ) ),
    maRbRange( this, ScResId( RB_RANGE ), &maEdRange, this ),
    maCondFormList( this, ScResId( CTRL_LIST ), pDoc, pFormat, rRange, rPos, eType ),
    maPos(rPos),
    mpDoc(pDoc),
    mpFormat(pFormat),
    mpLastEdit(NULL),
    meType(eType)
{
    rtl::OUStringBuffer aTitle( GetText() );
    aTitle.append(rtl::OUString(" "));
    rtl::OUString aRangeString;
    rRange.Format(aRangeString, SCA_VALID, pDoc, pDoc->GetAddressConvention());
    aTitle.append(aRangeString);
    SetText(aTitle.makeStringAndClear());
    maBtnAdd.SetClickHdl( LINK( &maCondFormList, ScCondFormatList, AddBtnHdl ) );
    maBtnRemove.SetClickHdl( LINK( &maCondFormList, ScCondFormatList, RemoveBtnHdl ) );
    maEdRange.SetModifyHdl( LINK( this, ScCondFormatDlg, EdRangeModifyHdl ) );
    maBtnOk.SetClickHdl( LINK( this, ScCondFormatDlg, OkBtnHdl ) );
    maBtnCancel.SetClickHdl( LINK( this, ScCondFormatDlg, CancelBtnHdl ) );
    maEdRange.SetGetFocusHdl( LINK( this, ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdRange.SetLoseFocusHdl( LINK( this, ScCondFormatDlg, RangeLoseFocusHdl ) );
    FreeResource();

    maEdRange.SetText(aRangeString);
}

ScCondFormatDlg::~ScCondFormatDlg()
{
}

void ScCondFormatDlg::SetActive()
{
    if(mpLastEdit)
        mpLastEdit->GrabFocus();
    else
        maEdRange.GrabFocus();

    RefInputDone();
}

void ScCondFormatDlg::RefInputDone( sal_Bool bForced )
{
    ScAnyRefDlg::RefInputDone(bForced);
}

sal_Bool ScCondFormatDlg::IsTableLocked() const
{
    if(mpLastEdit && mpLastEdit != &maEdRange)
        return sal_False;

    return sal_True;
}

sal_Bool ScCondFormatDlg::IsRefInputMode() const
{
    return maEdRange.IsEnabled();
}

#define ABS_SREF          SCA_VALID \
    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
    | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE

void ScCondFormatDlg::SetReference(const ScRange& rRef, ScDocument*)
{
    formula::RefEdit* pEdit = mpLastEdit;
    if(!mpLastEdit)
        pEdit = &maEdRange;

    if( pEdit->IsEnabled() )
    {
        if(rRef.aStart != rRef.aEnd)
            RefInputStart(pEdit);

        rtl::OUString aRefStr;
        rRef.Format( aRefStr, ABS_DREF, mpDoc, ScAddress::Details(mpDoc->GetAddressConvention(), 0, 0) );
        pEdit->SetRefString( aRefStr );
    }
}

ScConditionalFormat* ScCondFormatDlg::GetConditionalFormat() const
{
    rtl::OUString aRangeStr = maEdRange.GetText();
    ScRangeList aRange;
    sal_uInt16 nFlags = aRange.Parse(aRangeStr, mpDoc, SCA_VALID, mpDoc->GetAddressConvention());
    ScConditionalFormat* pFormat = maCondFormList.GetConditionalFormat();

    if(nFlags & SCA_VALID && !aRange.empty() && pFormat)
        pFormat->AddRange(aRange);

    return pFormat;
}

void ScCondFormatDlg::InvalidateRefData()
{
    mpLastEdit = NULL;
}

IMPL_LINK( ScCondFormatDlg, EdRangeModifyHdl, Edit*, pEdit )
{
    rtl::OUString aRangeStr = pEdit->GetText();
    ScRangeList aRange;
    sal_uInt16 nFlags = aRange.Parse(aRangeStr, mpDoc, SCA_VALID, mpDoc->GetAddressConvention());
    if(nFlags & SCA_VALID)
        pEdit->SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    else
        pEdit->SetControlBackground(COL_LIGHTRED);
    return 0;
}

sal_Bool ScCondFormatDlg::Close()
{
    sal_uInt16 nId = 0;
    switch(meType)
    {
        case condformat::dialog::NONE:
        case condformat::dialog::CONDITION:
            nId = ScCondFormatConditionDlgWrapper::GetChildWindowId();
            break;
        case condformat::dialog::COLORSCALE:
            nId = ScCondFormatColorScaleDlgWrapper::GetChildWindowId();
            break;
        case condformat::dialog::DATABAR:
            nId = ScCondFormatDataBarDlgWrapper::GetChildWindowId();
            break;
    }

    return DoClose(nId);
}

IMPL_LINK_NOARG( ScCondFormatDlg, OkBtnHdl )
{
    ScConditionalFormat* pFormat = GetConditionalFormat();
    SfxObjectShell* pObjectShell = mpDoc->GetDocumentShell();
    sal_Int32 nKey = 0;
    if(mpFormat)
        nKey = mpFormat->GetKey();

    static_cast<ScDocShell*>(pObjectShell)->GetDocFunc().ReplaceConditionalFormat(nKey, pFormat, maPos.Tab(), pFormat->GetRange());

    Close();
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatDlg, CancelBtnHdl )
{
    Close();

    return 0;
}

IMPL_LINK( ScCondFormatDlg, RangeGetFocusHdl, formula::RefEdit*, pEdit )
{
    mpLastEdit = pEdit;
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatDlg, RangeLoseFocusHdl )
{
    //mpLastEdit = NULL;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
