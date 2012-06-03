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

#include "anyrefdg.hxx"
#include "document.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"
#include "tabvwsh.hxx"

#include "globstr.hrc"

#include <iostream>

ScCondFrmtEntry::ScCondFrmtEntry(Window* pParent, ScDocument* pDoc):
    Control(pParent, ScResId( RID_COND_ENTRY ) ),
    mbActive(false),
    meType(CONDITION),
    maLbType( this, ScResId( LB_TYPE ) ),
    maFtCondNr( this, ScResId( FT_COND_NR ) ),
    maFtCondition( this, ScResId( FT_CONDITION ) ),
    maLbCondType( this, ScResId( LB_CELLIS_TYPE ) ),
    maEdVal1( this, ScResId( ED_VAL1 ) ),
    maEdVal2( this, ScResId( ED_VAL2 ) ),
    maFtStyle( this, ScResId( FT_STYLE ) ),
    maLbStyle( this, ScResId( LB_STYLE ) ),
    maWdPreview( this, ScResId( WD_PREVIEW ) ),
    maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
    maLbColScale2( this, ScResId( LB_COL_SCALE2 ) ),
    maLbColScale3( this, ScResId( LB_COL_SCALE3 ) ),
    maLbEntryTypeMin( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbEntryTypeMiddle( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbEntryTypeMax( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maEdMin( this, ScResId( ED_COL_SCALE ) ),
    maEdMiddle( this, ScResId( ED_COL_SCALE ) ),
    maEdMax( this, ScResId( ED_COL_SCALE ) ),
    mpDoc(pDoc)
{
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    SwitchToType(COLLAPSED);
    SetHeight();
    FreeResource();

    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );

    maLbType.SelectEntryPos(1);
    maLbType.SetSelectHdl( LINK( this, ScCondFrmtEntry, TypeListHdl ) );
    maLbColorFormat.SetSelectHdl( LINK( this, ScCondFrmtEntry, ColFormatTypeHdl ) );
    maLbCondType.SelectEntryPos(0);
    maEdVal2.Hide();

    SfxStyleSheetIterator aStyleIter( mpDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
	rtl::OUString aName = pStyle->GetName();
        maLbStyle.InsertEntry( aName );
    }
    maLbStyle.SetSeparatorPos(0);
    maLbStyle.SelectEntryPos(1);
    maLbStyle.SetSelectHdl( LINK( this, ScCondFrmtEntry, StyleSelectHdl ) );

    //disable entries for color formats
    maLbColorFormat.SelectEntryPos(0);
    maLbEntryTypeMin.SelectEntryPos(0);
    Point aPointLb = maLbEntryTypeMiddle.GetPosPixel();
    Point aPointEd = maEdMiddle.GetPosPixel();
    const long nMovePos = 150;
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    maLbEntryTypeMiddle.SetPosPixel(aPointLb);
    maEdMiddle.SetPosPixel(aPointEd);
    maLbEntryTypeMiddle.SelectEntryPos(2);
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    maLbEntryTypeMax.SelectEntryPos(1);
    maLbEntryTypeMax.SetPosPixel(aPointLb);
    maEdMax.SetPosPixel(aPointEd);
    SetCondType();
}

namespace {

rtl::OUString getTextForType(ScCondFormatEntryType eType)
{
    switch(eType)
    {
	case CONDITION:
	    return rtl::OUString("Cell value");
	case COLORSCALE:
	    return rtl::OUString("Color scale");
	case DATABAR:
	    return rtl::OUString("Data Bar");
	default:
	    break;
    }

    return rtl::OUString("");
}

rtl::OUString getExpression(sal_Int32 nIndex)
{
    switch(nIndex)
    {
	case 0:
	    return rtl::OUString("=");
	case 1:
	    return rtl::OUString("<");
	case 2:
	    return rtl::OUString(">");
	case 5:
	    return rtl::OUString("!=");
	default:
	    return rtl::OUString("not yet supported");
    }
    return rtl::OUString();
}

}

long ScCondFrmtEntry::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
	ImplCallEventListenersAndHandler( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, maClickHdl, this );
    }
    return Control::Notify(rNEvt);
}

void ScCondFrmtEntry::SwitchToType( ScCondFormatEntryType eType )
{
    switch(eType)
    {
	case COLLAPSED:
	    {
		maLbType.Hide();
		rtl::OUStringBuffer maCondText(getTextForType(meType));
		maCondText.append(rtl::OUString(" "));
		maCondText.append(getExpression(maLbCondType.GetSelectEntryPos()));
		maFtCondition.SetText(maCondText.makeStringAndClear());
		maFtCondition.Show();
	    }
	    break;
	default:
	    meType = eType;
	    maLbType.Show();
	    maFtCondition.SetText(rtl::OUString(""));
	    maFtCondition.Hide();
	    break;
    }
}

void ScCondFrmtEntry::HideCondElements()
{
    maEdVal1.Hide();
    maEdVal2.Hide();
    maLbStyle.Hide();
    maFtStyle.Hide();
    maLbCondType.Hide();
    maWdPreview.Hide();
}

void ScCondFrmtEntry::SetCondType()
{
    maEdVal1.Show();
    maEdVal2.Show();
    maLbStyle.Show();
    maLbCondType.Show();
    maFtStyle.Show();
    maWdPreview.Show();
    HideColorScaleElements();
    HideDataBarElements();
    SwitchToType(CONDITION);
}

void ScCondFrmtEntry::HideColorScaleElements()
{
    maLbColorFormat.Hide();
    maLbColScale2.Hide();
    maLbColScale3.Hide();
    maLbEntryTypeMin.Hide();
    maLbEntryTypeMiddle.Hide();
    maLbEntryTypeMax.Hide();
    maEdMin.Hide();
    maEdMiddle.Hide();
    maEdMax.Hide();
}

void ScCondFrmtEntry::SetHeight()
{
    if(mbActive)
    {
	Size aSize = GetSizePixel();
	switch (meType)
	{
	    case CONDITION:
		std::cout << "CONDITION: set height 120" << std::endl;
		aSize.Height() = 120;
		break;
	    case COLORSCALE:
		std::cout << "set height 200" << std::endl;
		aSize.Height() = 200;
		break;
	    case DATABAR:
		std::cout << "DATABAR: set height 120" << std::endl;
		aSize.Height() = 120;
		break;
	    default:
		break;
	}
	SetSizePixel(aSize);
    }
    else
    {
	std::cout << "set height 40" << std::endl;
	Size aSize = GetSizePixel();
	aSize.Height() = 40;
	SetSizePixel(aSize);
    }
}

void ScCondFrmtEntry::SetColorScaleType()
{
    HideCondElements();
    HideDataBarElements();
    maLbColorFormat.Show();
    if(maLbColorFormat.GetSelectEntryPos() == 0)
    {
	maEdMiddle.Hide();
	maLbEntryTypeMiddle.Hide();
	maLbColScale2.Show();
	maLbColScale3.Hide();
    }
    else
    {
	maEdMiddle.Show();
	maLbEntryTypeMiddle.Show();
	maLbColScale2.Hide();
	maLbColScale3.Show();
    }
    maLbEntryTypeMin.Show();
    maLbEntryTypeMax.Show();
    maEdMin.Show();
    maEdMax.Show();
    SwitchToType(COLORSCALE);
}

void ScCondFrmtEntry::HideDataBarElements()
{
    maLbColorFormat.Hide();
}

void ScCondFrmtEntry::SetDataBarType()
{
    SwitchToType(DATABAR);
    HideCondElements();
    HideColorScaleElements();
    maLbColorFormat.Show();
}

void ScCondFrmtEntry::Select()
{
    SetControlForeground(Color(COL_RED));
    SwitchToType(meType);
    mbActive = true;
    SetHeight();
}

void ScCondFrmtEntry::Deselect()
{
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    SwitchToType(COLLAPSED);
    mbActive = false;
    SetHeight();
}

bool ScCondFrmtEntry::IsSelected() const
{
    return mbActive;
}

IMPL_LINK_NOARG(ScCondFrmtEntry, TypeListHdl)
{
    sal_Int32 nPos = maLbType.GetSelectEntryPos();
    switch(nPos)
    {
	case 1:
	    SetCondType();
	    break;
	case 0:
	    if(maLbColorFormat.GetSelectEntryPos() < 2)
		SetColorScaleType();
	    else
		SetDataBarType();
	    break;
	case 2:
	    SetCondType();
	    break;
	default:
	    break;
    }
    SetHeight();
    return 0;
}

IMPL_LINK_NOARG(ScCondFrmtEntry, ColFormatTypeHdl)
{
    if(maLbColorFormat.GetSelectEntryPos() < 2)
    {
	SetColorScaleType();
    }
    else
    {
	SetDataBarType();
    }

    SetHeight();

    return 0;
}

IMPL_LINK_NOARG(ScCondFrmtEntry, StyleSelectHdl)
{
    if(maLbStyle.GetSelectEntryPos() == 0)
    {
	// call new style dialog
	SfxUInt16Item aFamilyItem( SID_STYLE_FAMILY, SFX_STYLE_FAMILY_PARA );
	SfxStringItem aRefItem( SID_STYLE_REFERENCE, ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );

	// unlock the dispatcher so SID_STYLE_NEW can be executed
	// (SetDispatcherLock would affect all Calc documents)
	ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
	SfxDispatcher* pDisp = pViewShell->GetDispatcher();
	sal_Bool bLocked = pDisp->IsLocked();
	if (bLocked)
	    pDisp->Lock(false);

	// Execute the "new style" slot, complete with undo and all necessary updates.
	// The return value (SfxUInt16Item) is ignored, look for new styles instead.
	pDisp->Execute( SID_STYLE_NEW, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD | SFX_CALLMODE_MODAL,
		&aFamilyItem,
		&aRefItem,
		0L );

	if (bLocked)
	    pDisp->Lock(sal_True);

	// Find the new style and add it into the style list boxes
	rtl::OUString aNewStyle;
	SfxStyleSheetIterator aStyleIter( mpDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
	for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
	{
	    rtl::OUString aName = pStyle->GetName();
	    if ( maLbStyle.GetEntryPos(aName) == LISTBOX_ENTRY_NOTFOUND )    // all lists contain the same entries
	    {
		maLbStyle.InsertEntry(aName);
		maLbStyle.SelectEntry(aName);
	    }
	}
    }

    rtl::OUString aStyleName = maLbStyle.GetSelectEntry();
    SfxStyleSheetBase* pStyleSheet = mpDoc->GetStyleSheetPool()->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
    if(pStyleSheet)
    {
	const SfxItemSet& rSet = pStyleSheet->GetItemSet();
	maWdPreview.Init( rSet );
    }

    return 0;
}

ScCondFormatList::ScCondFormatList(Window* pParent, const ResId& rResId, ScDocument* pDoc):
    Control(pParent, rResId),
    mbHasScrollBar(false),
    mpScrollBar(new ScrollBar(this, WB_VERT )),
    mnTopIndex(0),
    mpDoc(pDoc)
{
    mpScrollBar->SetScrollHdl( LINK( this, ScCondFormatList, ScrollHdl ) );
    mpScrollBar->EnableDrag();

    RecalcAll();
    FreeResource();
}

void ScCondFormatList::RecalcAll()
{
    sal_Int32 nTotalHeight = 0;
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
	nTotalHeight += itr->GetSizePixel().Height();
    }

    Size aCtrlSize = GetOutputSize();
    long nSrcBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if(nTotalHeight > GetSizePixel().Height())
    {
	mbHasScrollBar = true;
	mpScrollBar->SetPosSizePixel(Point(aCtrlSize.Width() -nSrcBarSize, 0),
					Size(nSrcBarSize, aCtrlSize.Height()) );
	std::cout << "Need ScrollBar" << std::endl;
	mpScrollBar->SetRangeMax(nTotalHeight);
	mpScrollBar->SetVisibleSize(aCtrlSize.Height());
	mpScrollBar->Show();
    }
    else
    {
	std::cout << "Don't need ScrollBar" << std::endl;

	mbHasScrollBar = false;
	mpScrollBar->Hide();
    }

    Point aPoint(0,0);
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

ScCondFormatDlg::ScCondFormatDlg(Window* pParent, ScDocument* pDoc, ScConditionalFormat* pFormat, const ScRangeList& rRange):
    ModalDialog(pParent, ScResId( RID_SCDLG_CONDFORMAT )),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnRemove( this, ScResId( BTN_REMOVE ) ),
    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maCondFormList( this, ScResId( CTRL_LIST ), pDoc ),
    mpDoc(pDoc),
    mpFormat(pFormat)
{

    rtl::OUStringBuffer aTitle( GetText() );
    aTitle.append(rtl::OUString(" "));
    rtl::OUString aRangeString;
    rRange.Format(aRangeString, 0, pDoc);
    aTitle.append(aRangeString);
    SetText(aTitle.makeStringAndClear());
    maBtnAdd.SetClickHdl( LINK( &maCondFormList, ScCondFormatList, AddBtnHdl ) );
    maBtnRemove.SetClickHdl( LINK( &maCondFormList, ScCondFormatList, RemoveBtnHdl ) );
    FreeResource();
}

IMPL_LINK_NOARG( ScCondFormatList, AddBtnHdl )
{
    maEntries.push_back( new ScCondFrmtEntry(this, mpDoc) );
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
    RecalcAll();
    return 0;
}

IMPL_LINK( ScCondFormatList, EntrySelectHdl, ScCondFrmtEntry*, pEntry )
{
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
	itr->Deselect();
    }
    pEntry->Select();
    RecalcAll();
    return 0;
}

IMPL_LINK_NOARG( ScCondFormatList, ScrollHdl )
{
    DoScroll(mpScrollBar->GetDelta());
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
