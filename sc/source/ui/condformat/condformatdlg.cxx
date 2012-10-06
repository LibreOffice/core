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

#include "globstr.hrc"

#include <cassert>
#include <iostream>

namespace {

void SetColorScaleEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit, ColorListBox& rLbCol )
{
    switch(rEntry.GetType())
    {
        case COLORSCALE_MIN:
            rLbType.SelectEntryPos(0);
            break;
        case COLORSCALE_MAX:
            rLbType.SelectEntryPos(1);
            break;
        case COLORSCALE_PERCENTILE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(2);
            break;
        case COLORSCALE_PERCENT:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(4);
            break;
        case COLORSCALE_FORMULA:
            rEdit.SetText(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            rLbType.SelectEntryPos(5);
            break;
        case COLORSCALE_VALUE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(3);
            break;
        case COLORSCALE_AUTOMIN:
            rLbType.SelectEntryPos(6);
            break;
        case COLORSCALE_AUTOMAX:
            rLbType.SelectEntryPos(7);
            break;
    }
    rLbCol.SelectEntry(rEntry.GetColor());
}

void SetDataBarEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit )
{
    switch(rEntry.GetType())
    {
        case COLORSCALE_MIN:
            rLbType.SelectEntryPos(0);
            break;
        case COLORSCALE_MAX:
            rLbType.SelectEntryPos(1);
            break;
        case COLORSCALE_PERCENTILE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(2);
            break;
        case COLORSCALE_PERCENT:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(4);
            break;
        case COLORSCALE_FORMULA:
            rEdit.SetText(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            rLbType.SelectEntryPos(5);
            break;
        case COLORSCALE_VALUE:
            rEdit.SetText(rtl::OUString::valueOf(rEntry.GetValue()));
            rLbType.SelectEntryPos(3);
            break;
        case COLORSCALE_AUTOMIN:
            rLbType.SelectEntryPos(6);
            break;
        case COLORSCALE_AUTOMAX:
            rLbType.SelectEntryPos(7);
            break;
    }
}

}

ScCondFrmtEntry::ScCondFrmtEntry(Window* pParent, ScDocument* pDoc, const ScAddress& rPos):
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
    maLbColMin( this, ScResId( LB_COL) ),
    maLbColMiddle( this, ScResId( LB_COL) ),
    maLbColMax( this, ScResId( LB_COL) ),
    maLbDataBarMinType( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbDataBarMaxType( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maEdDataBarMin( this, ScResId( ED_COL_SCALE ) ),
    maEdDataBarMax( this, ScResId( ED_COL_SCALE ) ),
    maBtOptions( this, ScResId( BTN_OPTIONS ) ),
    mpDoc(pDoc),
    maPos(rPos),
    mnIndex(0),
    maStrCondition(ScResId( STR_CONDITION ).toString())
{
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    FreeResource();

    maLbType.SelectEntryPos(1);
    maLbCondType.SelectEntryPos(0);
    maEdVal2.Hide();

    maLbStyle.SetSeparatorPos(0);

    //disable entries for color formats
    maLbColorFormat.SelectEntryPos(0);
    maLbEntryTypeMin.SelectEntryPos(0);
    maLbEntryTypeMiddle.SelectEntryPos(2);
    maLbEntryTypeMax.SelectEntryPos(1);
    maLbDataBarMinType.SelectEntryPos(0);
    maLbDataBarMaxType.SelectEntryPos(1);
    maEdMiddle.SetText(rtl::OUString::valueOf(static_cast<sal_Int32>(50)));
    maEdDataBarMin.Disable();
    maEdDataBarMax.Disable();

    Init();
    maLbStyle.SelectEntryPos(1);
    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );
    SwitchToType(COLLAPSED);
    SetHeight();
    SetCondType();

    EntryTypeHdl(&maLbEntryTypeMin);
    EntryTypeHdl(&maLbEntryTypeMiddle);
    EntryTypeHdl(&maLbEntryTypeMax);
}

ScCondFrmtEntry::ScCondFrmtEntry(Window* pParent, ScDocument* pDoc, const ScFormatEntry* pFormatEntry, const ScAddress& rPos):
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
    maLbColMin( this, ScResId( LB_COL) ),
    maLbColMiddle( this, ScResId( LB_COL) ),
    maLbColMax( this, ScResId( LB_COL) ),
    maLbDataBarMinType( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maLbDataBarMaxType( this, ScResId( LB_TYPE_COL_SCALE ) ),
    maEdDataBarMin( this, ScResId( ED_COL_SCALE ) ),
    maEdDataBarMax( this, ScResId( ED_COL_SCALE ) ),
    maBtOptions( this, ScResId( BTN_OPTIONS ) ),
    mpDoc(pDoc),
    maPos(rPos)
{
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    FreeResource();
    Init();

    if(pFormatEntry && pFormatEntry->GetType() == condformat::CONDITION)
    {
        const ScCondFormatEntry* pEntry = static_cast<const ScCondFormatEntry*>(pFormatEntry);
        rtl::OUString aStyleName = pEntry->GetStyle();
        maLbStyle.SelectEntry(aStyleName);
        StyleSelectHdl(NULL);
        ScConditionMode eMode = pEntry->GetOperation();
        maLbType.SelectEntryPos(1);
        maEdVal1.SetText(pEntry->GetExpression(maPos, 0));
        SetCondType();
        switch(eMode)
        {
            case SC_COND_EQUAL:
                maLbCondType.SelectEntryPos(0);
                break;
            case SC_COND_LESS:
                maLbCondType.SelectEntryPos(1);
                break;
            case SC_COND_GREATER:
                maLbCondType.SelectEntryPos(2);
                break;
            case SC_COND_EQLESS:
                maLbCondType.SelectEntryPos(3);
                break;
            case SC_COND_EQGREATER:
                maLbCondType.SelectEntryPos(4);
                break;
            case SC_COND_NOTEQUAL:
                maLbCondType.SelectEntryPos(5);
                break;
            case SC_COND_BETWEEN:
                maEdVal2.SetText(pEntry->GetExpression(maPos, 1));
                maLbCondType.SelectEntryPos(6);
                break;
            case SC_COND_NOTBETWEEN:
                maEdVal2.SetText(pEntry->GetExpression(maPos, 1));
                maLbCondType.SelectEntryPos(7);
                break;
            case SC_COND_DUPLICATE:
                maLbCondType.SelectEntryPos(8);
                break;
            case SC_COND_NOTDUPLICATE:
                maLbCondType.SelectEntryPos(9);
                break;
            case SC_COND_DIRECT:
                maLbType.SelectEntryPos(2);
                SwitchToType(FORMULA);
                break;
            case SC_COND_NONE:
                break;
        }
    }
    else if( pFormatEntry && pFormatEntry->GetType() == condformat::COLORSCALE )
    {
        const ScColorScaleFormat* pEntry = static_cast<const ScColorScaleFormat*>(pFormatEntry);
        maLbType.SelectEntryPos(0);
        if(pEntry->size() == 2)
            maLbColorFormat.SelectEntryPos(0);
        else
            maLbColorFormat.SelectEntryPos(1);
        SetColorScaleType();
        ScColorScaleFormat::const_iterator itr = pEntry->begin();
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMin, maEdMin, maLbColMin);
        if(pEntry->size() == 3)
        {
            ++itr;
            SetColorScaleEntryTypes(*itr, maLbEntryTypeMiddle, maEdMiddle, maLbColMiddle);
        }
        ++itr;
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMax, maEdMax, maLbColMax);
    }
    else if( pFormatEntry && pFormatEntry->GetType() == condformat::DATABAR )
    {
        const ScDataBarFormat* pEntry = static_cast<const ScDataBarFormat*>(pFormatEntry);
        mpDataBarData.reset(new ScDataBarFormatData(*pEntry->GetDataBarData()));
        maLbType.SelectEntryPos(0);
        maLbColorFormat.SelectEntryPos(2);
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, maLbDataBarMinType, maEdDataBarMin);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, maLbDataBarMaxType, maEdDataBarMax);
        DataBarTypeSelectHdl(NULL);
        SetDataBarType();
    }

    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );
    SwitchToType(COLLAPSED);
    SetHeight();

    EntryTypeHdl(&maLbEntryTypeMin);
    EntryTypeHdl(&maLbEntryTypeMiddle);
    EntryTypeHdl(&maLbEntryTypeMax);
}

ScCondFrmtEntry::~ScCondFrmtEntry()
{
}

void ScCondFrmtEntry::Init()
{
    maLbType.SetSelectHdl( LINK( this, ScCondFrmtEntry, TypeListHdl ) );
    maLbColorFormat.SetSelectHdl( LINK( this, ScCondFrmtEntry, ColFormatTypeHdl ) );
    maLbEntryTypeMin.SetSelectHdl( LINK( this, ScCondFrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax.SetSelectHdl( LINK( this, ScCondFrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMiddle.SetSelectHdl( LINK( this, ScCondFrmtEntry, EntryTypeHdl ) );
    maEdVal1.SetStyle( maEdVal1.GetStyle() | WB_FORCECTRLBACKGROUND );
    maEdVal2.SetStyle( maEdVal2.GetStyle() | WB_FORCECTRLBACKGROUND );

    maEdVal1.SetModifyHdl( LINK( this, ScCondFrmtEntry, EdModifyHdl ) );
    maEdVal2.SetModifyHdl( LINK( this, ScCondFrmtEntry, EdModifyHdl ) );

    SfxStyleSheetIterator aStyleIter( mpDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        rtl::OUString aName = pStyle->GetName();
        maLbStyle.InsertEntry( aName );
    }
    maLbStyle.SetSelectHdl( LINK( this, ScCondFrmtEntry, StyleSelectHdl ) );

    Point aPointLb = maLbEntryTypeMiddle.GetPosPixel();
    Point aPointEd = maEdMiddle.GetPosPixel();
    Point aPointCol = maLbColMiddle.GetPosPixel();
    Point aPointEdDataBar = maEdDataBarMin.GetPosPixel();
    Point aPointLbDataBar = maLbDataBarMaxType.GetPosPixel();
    const long nMovePos = maLbEntryTypeMiddle.GetSizePixel().Width() * 1.2;
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    aPointCol.X() += nMovePos;
    aPointEdDataBar.X() += 2*nMovePos;
    aPointLbDataBar.X() += 2*nMovePos;
    maLbEntryTypeMiddle.SetPosPixel(aPointLb);
    maEdMiddle.SetPosPixel(aPointEd);
    maLbColMiddle.SetPosPixel(aPointCol);
    maEdDataBarMax.SetPosPixel(aPointEdDataBar);
    maLbDataBarMaxType.SetPosPixel(aPointLbDataBar);
    aPointLb.X() += nMovePos;
    aPointEd.X() += nMovePos;
    aPointCol.X() += nMovePos;
    maLbEntryTypeMax.SetPosPixel(aPointLb);
    maEdMax.SetPosPixel(aPointEd);
    maLbColMax.SetPosPixel(aPointCol);

    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    const SfxPoolItem*  pItem       = NULL;
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }
    if ( pColorTable.is() )
    {
        // filling the line color box
        maLbColMin.SetUpdateMode( false );
        maLbColMiddle.SetUpdateMode( false );
        maLbColMax.SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            maLbColMin.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMiddle.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMax.InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                maLbColMin.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_GREEN))
                maLbColMiddle.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                maLbColMax.SelectEntryPos(i);
        }
        maLbColMin.SetUpdateMode( sal_True );
        maLbColMiddle.SetUpdateMode( sal_True );
        maLbColMax.SetUpdateMode( sal_True );
    }

    maBtOptions.SetClickHdl( LINK( this, ScCondFrmtEntry, OptionBtnHdl ) );
    maLbDataBarMinType.SetSelectHdl( LINK( this, ScCondFrmtEntry, DataBarTypeSelectHdl ) );
    maLbDataBarMaxType.SetSelectHdl( LINK( this, ScCondFrmtEntry, DataBarTypeSelectHdl ) );
    maLbCondType.SetSelectHdl( LINK( this, ScCondFrmtEntry, ConditionTypeSelectHdl ) );

    mpDataBarData.reset(new ScDataBarFormatData());
    mpDataBarData->mpUpperLimit.reset(new ScColorScaleEntry());
    mpDataBarData->mpLowerLimit.reset(new ScColorScaleEntry());
    mpDataBarData->mpLowerLimit->SetType(COLORSCALE_MIN);
    mpDataBarData->mpUpperLimit->SetType(COLORSCALE_MAX);
    mpDataBarData->maPositiveColor = COL_LIGHTBLUE;
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
                rtl::OUString maCondText = ScCondFormatHelper::GetExpression(meType, maLbCondType.GetSelectEntryPos());
                maFtCondition.SetText(maCondText);
                maFtCondition.Show();
                maLbType.Hide();
                HideCondElements();
                HideColorScaleElements();
                HideDataBarElements();
            }
            break;
        default:
            meType = eType;
            maLbType.Show();
            maFtCondition.SetText(rtl::OUString(""));
            maFtCondition.Hide();
            maLbType.Show();
            break;
    }
}

void ScCondFrmtEntry::SetIndex(sal_Int32 nIndex)
{
    mnIndex = nIndex;
    rtl::OUStringBuffer aBuffer(maStrCondition);
    aBuffer.append(rtl::OUString::valueOf(nIndex));
    maFtCondNr.SetText(aBuffer.makeStringAndClear());
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
    maEdVal1.SetSizePixel(maEdVal2.GetSizePixel());
    Point aPoint(maLbCondType.GetPosPixel().X() + maLbCondType.GetSizePixel().Width() + LogicToPixel(Size(5,1), MapMode(MAP_APPFONT)).getWidth(),
            maEdVal1.GetPosPixel().Y());
    maEdVal1.SetPosPixel(aPoint);
    maEdVal1.Show();
    maEdVal2.Show();
    maLbStyle.Show();
    maLbCondType.Show();
    maFtStyle.Show();
    maWdPreview.Show();
    HideColorScaleElements();
    HideDataBarElements();
    SwitchToType(CONDITION);
    ConditionTypeSelectHdl(NULL);
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
    maLbColMin.Hide();
    maLbColMiddle.Hide();
    maLbColMax.Hide();
}

void ScCondFrmtEntry::SetHeight()
{
    long nPad = LogicToPixel(Size(42,2), MapMode(MAP_APPFONT)).getHeight();

    // Calculate maximum height we need from visible widgets
    sal_uInt16 nChildren = GetChildCount();

    long nMaxHeight = 0;
    for(sal_uInt16 i = 0; i < nChildren; i++)
    {
        Window *pChild  = GetChild(i);
        if(!pChild || !pChild->IsVisible())
            continue;
        Point aPos = pChild->GetPosPixel();
        Size aSize = pChild->GetSizePixel();
        nMaxHeight = std::max(aPos.Y() + aSize.Height(), nMaxHeight);
    }
    Size aSize = GetSizePixel();
    if(nMaxHeight > 0)
    {
        aSize.Height() = nMaxHeight + nPad;
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
        // TODO: enale for 3.7 again with good presets
        //maLbColScale2.Show();
        maLbColScale3.Hide();
        maLbColMiddle.Hide();
    }
    else
    {
        maEdMiddle.Show();
        maLbEntryTypeMiddle.Show();
        maLbColScale2.Hide();
        // TODO: enale for 3.7 again with good presets
        // maLbColScale3.Show();
        maLbColMiddle.Show();
    }
    maLbEntryTypeMin.Show();
    maLbEntryTypeMax.Show();
    maEdMin.Show();
    maEdMax.Show();
    maLbColMin.Show();
    maLbColMax.Show();
    SwitchToType(COLORSCALE);
}

void ScCondFrmtEntry::HideDataBarElements()
{
    maLbColorFormat.Hide();
    maLbDataBarMinType.Hide();
    maLbDataBarMaxType.Hide();
    maEdDataBarMin.Hide();
    maEdDataBarMax.Hide();
    maBtOptions.Hide();
}

void ScCondFrmtEntry::SetDataBarType()
{
    SwitchToType(DATABAR);
    HideCondElements();
    HideColorScaleElements();
    maLbColorFormat.Show();
    maLbDataBarMinType.Show();
    maLbDataBarMaxType.Show();
    maEdDataBarMin.Show();
    maEdDataBarMax.Show();
    maBtOptions.Show();
}

void ScCondFrmtEntry::SetFormulaType()
{
    SwitchToType(FORMULA);
    HideColorScaleElements();
    HideDataBarElements();
    maEdVal1.SetPosPixel(maLbCondType.GetPosPixel());
    Size aSize(maEdVal2.GetPosPixel().X() + maEdVal2.GetSizePixel().Width() - maLbCondType.GetPosPixel().X(), maEdVal1.GetSizePixel().Height());
    maEdVal1.SetPosPixel(maLbCondType.GetPosPixel());
    maEdVal1.SetSizePixel(aSize);
    maEdVal1.Show();
    maEdVal2.Hide();
    maLbCondType.Hide();
    maLbStyle.Show();
    maFtCondition.Show();
    maWdPreview.Show();
    maFtStyle.Show();
}

void ScCondFrmtEntry::Select()
{
    SetControlForeground(Color(COL_RED));
    switch(meType)
    {
        case CONDITION:
            SetCondType();
            break;
        case COLORSCALE:
            SetColorScaleType();
            break;
        case DATABAR:
            SetDataBarType();
            break;
        case FORMULA:
            SetFormulaType();
            break;
        default:
            //should not happen
            break;
    }
    SwitchToType(meType);
    mbActive = true;
    SetHeight();
}

void ScCondFrmtEntry::SetType( ScCondFormatEntryType eType )
{
    meType = eType;
    if(eType == DATABAR)
        maLbColorFormat.SelectEntryPos(2);
    Select();
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

ScFormatEntry* ScCondFrmtEntry::createConditionEntry() const
{
    ScConditionMode eMode;
    rtl::OUString aExpr2;
    switch(maLbCondType.GetSelectEntryPos())
    {
        case 0:
            eMode = SC_COND_EQUAL;
            break;
        case 1:
            eMode = SC_COND_LESS;
            break;
        case 2:
            eMode = SC_COND_GREATER;
            break;
        case 3:
            eMode = SC_COND_EQLESS;
            break;
        case 4:
            eMode = SC_COND_EQGREATER;
            break;
        case 5:
            eMode = SC_COND_NOTEQUAL;
            break;
        case 6:
            aExpr2 = maEdVal2.GetText();
            eMode = SC_COND_BETWEEN;
            if(aExpr2.isEmpty())
                return NULL;
            break;
        case 7:
            eMode = SC_COND_NOTBETWEEN;
            aExpr2 = maEdVal2.GetText();
            if(aExpr2.isEmpty())
                return NULL;
            break;
        case 8:
            eMode = SC_COND_DUPLICATE;
            break;
        case 9:
            eMode = SC_COND_NOTDUPLICATE;
            break;
        default:
            assert(false); // this cannot happen
            return NULL;
    }

    rtl::OUString aExpr1 = maEdVal1.GetText();

    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, mpDoc, maPos, maLbStyle.GetSelectEntry());

    return pEntry;
}

namespace {

void SetColorScaleEntry( ScColorScaleEntry* pEntry, const ListBox& rType, const Edit& rValue, ScDocument* pDoc, const ScAddress& rPos )
{
    sal_uInt32 nIndex = 0;
    double nVal = 0;
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    pNumberFormatter->IsNumberFormat(rValue.GetText(), nIndex, nVal);
    switch(rType.GetSelectEntryPos())
    {
        case 0:
            pEntry->SetType(COLORSCALE_MIN);
            break;
        case 1:
            pEntry->SetType(COLORSCALE_MAX);
            break;
        case 2:
            pEntry->SetType(COLORSCALE_PERCENTILE);
            pEntry->SetValue(nVal);
            break;
        case 3:
            pEntry->SetType(COLORSCALE_VALUE);
            pEntry->SetValue(nVal);
            break;
        case 4:
            pEntry->SetType(COLORSCALE_PERCENT);
            pEntry->SetValue(nVal);
            break;
        case 5:
            pEntry->SetType(COLORSCALE_FORMULA);
            pEntry->SetFormula(rValue.GetText(), pDoc, rPos);
            break;
        case 6:
            pEntry->SetType(COLORSCALE_AUTOMIN);
            break;
        case 7:
            pEntry->SetType(COLORSCALE_AUTOMAX);
            break;
        default:
            break;
    }
}

ScColorScaleEntry* createColorScaleEntry( const ListBox& rType, const ColorListBox& rColor, const Edit& rValue, ScDocument* pDoc, const ScAddress& rPos )
{
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    SetColorScaleEntry( pEntry, rType, rValue, pDoc, rPos );
    Color aColor = rColor.GetSelectEntryColor();
    pEntry->SetColor(aColor);
    return pEntry;
}

}

ScFormatEntry* ScCondFrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMin, maLbColMin, maEdMin, mpDoc, maPos));
    if(maLbColorFormat.GetSelectEntryPos() == 1)
        pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMiddle, maLbColMiddle, maEdMiddle, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMax, maLbColMax, maEdMax, mpDoc, maPos));
    return pColorScale;
}

ScFormatEntry* ScCondFrmtEntry::createDatabarEntry() const
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), maLbDataBarMinType, maEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), maLbDataBarMaxType, maEdDataBarMax, mpDoc, maPos);
    ScDataBarFormat* pDataBar = new ScDataBarFormat(mpDoc);
    pDataBar->SetDataBarData(new ScDataBarFormatData(*mpDataBarData.get()));
    return pDataBar;
}

ScFormatEntry* ScCondFrmtEntry::createFormulaEntry() const
{
    ScConditionMode eMode = SC_COND_DIRECT;
    rtl::OUString aFormula = maEdVal1.GetText();
    if(aFormula.isEmpty())
        return NULL;

    rtl::OUString aExpr2;
    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aFormula, aExpr2, mpDoc, maPos, maLbStyle.GetSelectEntry());
    return pEntry;
}

ScFormatEntry* ScCondFrmtEntry::GetEntry() const
{
    switch(meType)
    {
        case CONDITION:
            return createConditionEntry();
            break;
        case COLORSCALE:
            return createColorscaleEntry();
            break;
        case DATABAR:
            return createDatabarEntry();
            break;
        case FORMULA:
            return createFormulaEntry();
            break;
        default:
            break;
    }
    return NULL;
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
            SetFormulaType();
            break;
        default:
            break;
    }
    SetHeight();
    return 0;
}

IMPL_LINK(ScCondFrmtEntry, EdModifyHdl, Edit*, pEdit)
{
    rtl::OUString aFormula = pEdit->GetText();
    ScCompiler aComp( mpDoc, maPos );
    aComp.SetGrammar( mpDoc->GetGrammar() );
    boost::scoped_ptr<ScTokenArray> mpCode(aComp.CompileString(aFormula));
    if(mpCode->GetCodeError())
    {
        pEdit->SetControlBackground(COL_LIGHTRED);
    }
    else
    {
        pEdit->SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    }
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

IMPL_LINK( ScCondFrmtEntry, EntryTypeHdl, ListBox*, pBox )
{
    bool bEnableEdit = true;
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    if(nPos == 0 || nPos == 1)
    {
        bEnableEdit = false;
    }

    Edit* pEd = NULL;
    if(pBox == &maLbEntryTypeMin)
        pEd = &maEdMin;
    else if(pBox == &maLbEntryTypeMiddle)
        pEd = &maEdMiddle;
    else if(pBox == &maLbEntryTypeMax)
        pEd = &maEdMax;

    if(bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();

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

IMPL_LINK_NOARG( ScCondFrmtEntry, DataBarTypeSelectHdl )
{
    sal_Int32 nSelectPos = maLbDataBarMinType.GetSelectEntryPos();
    if(nSelectPos == 0 || nSelectPos == 1)
        maEdDataBarMin.Disable();
    else
        maEdDataBarMin.Enable();

    nSelectPos = maLbDataBarMaxType.GetSelectEntryPos();
    if(nSelectPos == 0 || nSelectPos == 1)
        maEdDataBarMax.Disable();
    else
        maEdDataBarMax.Enable();

    return 0;
}

IMPL_LINK_NOARG( ScCondFrmtEntry, OptionBtnHdl )
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), maLbDataBarMinType, maEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), maLbDataBarMaxType, maEdDataBarMax, mpDoc, maPos);
    ScDataBarSettingsDlg* pDlg = new ScDataBarSettingsDlg(this, *mpDataBarData, mpDoc);
    if( pDlg->Execute() == RET_OK)
    {
        mpDataBarData.reset(pDlg->GetData());
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, maLbDataBarMinType, maEdDataBarMin);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, maLbDataBarMaxType, maEdDataBarMax);
        DataBarTypeSelectHdl(NULL);
    }
    return 0;
}

IMPL_LINK_NOARG( ScCondFrmtEntry, ConditionTypeSelectHdl )
{
    if(maLbCondType.GetSelectEntryPos() == 6 || maLbCondType.GetSelectEntryPos() == 7)
    {
        maEdVal2.Show();
    }
    else
    {
        maEdVal2.Hide();
    }

    return 0;
}

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
            maEntries.push_back(new ScCondFrmtEntry( this, mpDoc, pFormat->GetEntry(nIndex), maPos ));
        }
    }
    else
    {
        switch(eType)
        {
            case condformat::dialog::CONDITION:
                maEntries.push_back(new ScCondFrmtEntry( this, mpDoc, maPos ));
                break;
            case condformat::dialog::COLORSCALE:
                maEntries.push_back(new ScCondFrmtEntry( this, mpDoc, maPos ));
                maEntries[0].SetType(COLORSCALE);
                break;
            case condformat::dialog::DATABAR:
                maEntries.push_back(new ScCondFrmtEntry( this, mpDoc, maPos ));
                maEntries[0].SetType(DATABAR);
                break;
            default:
                break;
        }
    }
    if (!maEntries.empty())
        maEntries.begin()->Select();

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

ScCondFormatDlg::ScCondFormatDlg(Window* pParent, ScDocument* pDoc, const ScConditionalFormat* pFormat, const ScRangeList& rRange,
                                    const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType):
    ModalDialog(pParent, ScResId( RID_SCDLG_CONDFORMAT )),
    maBtnAdd( this, ScResId( BTN_ADD ) ),
    maBtnRemove( this, ScResId( BTN_REMOVE ) ),
    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maFtRange( this, ScResId( FT_RANGE ) ),
    maEdRange( this, ScResId( ED_RANGE ) ),
    maCondFormList( this, ScResId( CTRL_LIST ), pDoc, pFormat, rRange, rPos, eType ),
    maPos(rPos),
    mpDoc(pDoc)
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
    FreeResource();

    maEdRange.SetText(aRangeString);
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

IMPL_LINK_NOARG( ScCondFormatList, AddBtnHdl )
{
    ScCondFrmtEntry* pNewEntry = new ScCondFrmtEntry(this, mpDoc, maPos);
    maEntries.push_back( pNewEntry );
    for(EntryContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        itr->Deselect();
    }
    pNewEntry->Select();
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

IMPL_LINK_NOARG( ScCondFormatDlg, OkBtnHdl )
{
    rtl::OUString aRangeStr = maEdRange.GetText();
    ScRangeList aRange;
    aRange.Parse(aRangeStr, mpDoc, SCA_VALID, mpDoc->GetAddressConvention());
    boost::scoped_ptr<ScConditionalFormat> pFormat(maCondFormList.GetConditionalFormat());
    if(pFormat && pFormat->GetRange().empty() && aRange.empty())
        return 0;
    else
    {
        EndDialog(RET_OK);
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
