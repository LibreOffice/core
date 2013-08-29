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
                    {
                        const ScCondFormatEntry* pConditionEntry = static_cast<const ScCondFormatEntry*>( pEntry );
                        if(pConditionEntry->GetOperation() != SC_COND_DIRECT)
                            maEntries.push_back(new ScConditionFrmtEntry( this, mpDoc, maPos, pConditionEntry ) );
                        else
                            maEntries.push_back(new ScFormulaFrmtEntry( this, mpDoc, maPos, pConditionEntry ) );

                    }
                    break;
                case condformat::COLORSCALE:
                    {
                        const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>( pEntry );
                        if( pColorScale->size() == 2 )
                            maEntries.push_back(new ScColorScale2FrmtEntry( this, mpDoc, maPos, pColorScale ) );
                        else
                            maEntries.push_back(new ScColorScale3FrmtEntry( this, mpDoc, maPos, pColorScale ) );
                    }
                    break;
                case condformat::DATABAR:
                    maEntries.push_back(new ScDataBarFrmtEntry( this, mpDoc, maPos, static_cast<const ScDataBarFormat*>( pEntry ) ) );
                    break;
                case condformat::ICONSET:
                    maEntries.push_back(new ScIconSetFrmtEntry( this, mpDoc, maPos, static_cast<const ScIconSetFormat*>( pEntry ) ) );
                    break;
                case condformat::DATE:
                    maEntries.push_back(new ScDateFrmtEntry( this, mpDoc, static_cast<const ScCondDateFormatEntry*>( pEntry ) ) );
                    break;
            }
        }
        if(nCount)
            EntrySelectHdl(&maEntries[0]);
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
            case condformat::dialog::ICONSET:
                maEntries.push_back(new ScIconSetFrmtEntry( this, mpDoc, maPos ));
                break;
            case condformat::dialog::DATE:
                maEntries.push_back(new ScDateFrmtEntry( this, mpDoc ));
                break;
            case condformat::dialog::NONE:
                break;
        }
    }
    RecalcAll();
    if (!maEntries.empty())
        maEntries.begin()->SetActive();

    RecalcAll();
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
        case 3:
            if(itr->GetType() == condformat::entry::ICONSET)
                return 0;

            maEntries.replace( itr, new ScIconSetFrmtEntry( this, mpDoc, maPos ) );
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
                case condformat::entry::DATE:
                    break;
                case condformat::entry::COLORSCALE2:
                case condformat::entry::COLORSCALE3:
                case condformat::entry::DATABAR:
                case condformat::entry::ICONSET:
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
        case 3:
            if(itr->GetType() == condformat::entry::DATE)
                return 0;

            maEntries.replace( itr, new ScDateFrmtEntry( this, mpDoc ));
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

    //A child has focus, but we will hide that, so regrab to whatever new thing gets
    //shown instead of leaving it stuck in the inaccessible hidden element
    bool bReGrabFocus = HasChildPathFocus();
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        itr->SetInactive();
    }
    static_cast<ScCondFormatDlg*>(GetParent())->InvalidateRefData();
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

//---------------------------------------------------
//ScCondFormatDlg
//---------------------------------------------------

ScCondFormatDlg::ScCondFormatDlg(Window* pParent, ScDocument* pDoc, const ScConditionalFormat* pFormat, const ScRangeList& rRange,
                                    const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType):
    ScAnyRefModalDlg(pParent, ScResId(RID_SCDLG_CONDFORMAT) ),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnRemove( this, ScResId( BTN_REMOVE ) ),
    maFtRange( this, ScResId( FT_RANGE ) ),
    maEdRange( this, this, &maFtRange, ScResId( ED_RANGE ) ),
    maRbRange( this, ScResId( RB_RANGE ), &maEdRange, this ),
    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maCondFormList( this, ScResId( CTRL_LIST ), pDoc, pFormat, rRange, rPos, eType ),
    maPos(rPos),
    mpDoc(pDoc),
    mpLastEdit(NULL)
{
    OUStringBuffer aTitle( GetText() );
    aTitle.append(OUString(" "));
    OUString aRangeString;
    rRange.Format(aRangeString, SCA_VALID, pDoc, pDoc->GetAddressConvention());
    aTitle.append(aRangeString);
    SetText(aTitle.makeStringAndClear());
    maBtnAdd.SetClickHdl( LINK( &maCondFormList, ScCondFormatList, AddBtnHdl ) );
    maBtnRemove.SetClickHdl( LINK( &maCondFormList, ScCondFormatList, RemoveBtnHdl ) );
    maEdRange.SetModifyHdl( LINK( this, ScCondFormatDlg, EdRangeModifyHdl ) );
    maEdRange.SetGetFocusHdl( LINK( this, ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdRange.SetLoseFocusHdl( LINK( this, ScCondFormatDlg, RangeLoseFocusHdl ) );
    FreeResource();

    maEdRange.SetText(aRangeString);

    SC_MOD()->PushNewAnyRefDlg(this);
}

ScCondFormatDlg::~ScCondFormatDlg()
{
    SC_MOD()->PopAnyRefDlg();
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
    ScAnyRefModalDlg::RefInputDone(bForced);
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
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

void ScCondFormatDlg::SetReference(const ScRange& rRef, ScDocument*)
{
    formula::RefEdit* pEdit = mpLastEdit;
    if(!mpLastEdit)
        pEdit = &maEdRange;

    if( pEdit->IsEnabled() )
    {
        if(rRef.aStart != rRef.aEnd)
            RefInputStart(pEdit);

        sal_uInt16 n = 0;
        if(mpLastEdit && mpLastEdit != &maEdRange)
            n = ABS_DREF3D;
        else
            n = ABS_DREF;

        OUString aRefStr(rRef.Format(n, mpDoc, ScAddress::Details(mpDoc->GetAddressConvention(), 0, 0)));
        pEdit->SetRefString( aRefStr );
    }
}

ScConditionalFormat* ScCondFormatDlg::GetConditionalFormat() const
{
    OUString aRangeStr = maEdRange.GetText();
    if(aRangeStr.isEmpty())
        return NULL;

    ScRangeList aRange;
    sal_uInt16 nFlags = aRange.Parse(aRangeStr, mpDoc, SCA_VALID, mpDoc->GetAddressConvention(), maPos.Tab());
    ScConditionalFormat* pFormat = maCondFormList.GetConditionalFormat();

    if(nFlags & SCA_VALID && !aRange.empty() && pFormat)
        pFormat->AddRange(aRange);
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

IMPL_LINK_NOARG( ScCondFormatDlg, RangeLoseFocusHdl )
{
    //mpLastEdit = NULL;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
