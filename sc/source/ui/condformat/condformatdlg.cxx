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

#include "anyrefdg.hxx"
#include "document.hxx"
#include "conditio.hxx"

#include <iostream>

ScCondFrmtEntry::ScCondFrmtEntry(Window* pParent):
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
    maLbStyle( this, ScResId( LB_STYLE ) )
{
    Size aSize = GetSizePixel();
    aSize.Height() = 40;
    SetSizePixel(aSize);
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    SwitchToType(COLLAPSED);
    FreeResource();

    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );

    maLbType.SelectEntryPos(1);
    maLbCondType.SelectEntryPos(0);
    maEdVal2.Hide();
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
	std::cout << "ButtonDown " << std::endl;
	ImplCallEventListenersAndHandler( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, maClickHdl, this );
    }
    Control::Notify(rNEvt);
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
		maEdVal2.Hide();
		maEdVal1.Hide();
	    }
	    break;
	default:
	    maLbType.Show();
	    maFtCondition.SetText(rtl::OUString(""));
	    maFtCondition.Hide();
	    maEdVal1.Show();
	    maEdVal2.Show();
	    break;
    }
}

void ScCondFrmtEntry::Select()
{
    Size aSize = GetSizePixel();
    aSize.Height() = 130;
    SetSizePixel(aSize);
    SetControlBackground(Color(COL_RED));
    SwitchToType(meType);
}

void ScCondFrmtEntry::Deselect()
{
    Size aSize = GetSizePixel();
    aSize.Height() = 40;
    SetSizePixel(aSize);
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    SwitchToType(COLLAPSED);
}

ScCondFormatList::ScCondFormatList(Window* pParent, const ResId& rResId):
    Control(pParent, rResId),
    mbHasScrollBar(false),
    mpScrollBar(new ScrollBar(this, WB_VERT )),
    mnTopIndex(0)
{
    mpScrollBar->SetScrollHdl( LINK( this, ScCondFormatList, ScrollHdl ) );
    mpScrollBar->EnableDrag();
    maEntries.push_back( new ScCondFrmtEntry(this) );
    maEntries.push_back( new ScCondFrmtEntry(this) );
    maEntries.push_back( new ScCondFrmtEntry(this) );

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

ScCondFormatDlg::ScCondFormatDlg(Window* pParent, ScDocument* pDoc, ScConditionalFormat* pFormat, const ScRangeList& rRange):
    ModalDialog(pParent, ScResId( RID_SCDLG_CONDFORMAT )),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnRemove( this, ScResId( BTN_REMOVE ) ),
    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maCondFormList( this, ScResId( CTRL_LIST ) ),
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
}

IMPL_LINK_NOARG( ScCondFormatList, AddBtnHdl )
{
    maEntries.push_back( new ScCondFrmtEntry(this) );
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

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
