/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "condformatdlg.hxx"
#include "condformatdlgentry.hxx"
#include "condformatdlg.hrc"
#include "conditio.hxx"

#include "document.hxx"

#include <vcl/vclevent.hxx>
#include <svl/style.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <vcl/msgbox.hxx>
#include "tokenarray.hxx"
#include "stlpool.hxx"
#include "tabvwsh.hxx"

#include "colorformat.hxx"

#include "globstr.hrc"

#include <set>

ScCondFrmtEntry::ScCondFrmtEntry(Window* pParent, ScDocument* pDoc, const ScAddress& rPos):
    Control(pParent, ScResId( RID_COND_ENTRY ) ),
    mbActive(false),
    maFtCondNr( this, ScResId( FT_COND_NR ) ),
    maFtCondition( this, ScResId( FT_CONDITION ) ),
    mnIndex(0),
    maStrCondition(ScResId( STR_CONDITION ).toString()),
    maLbType( this, ScResId( LB_TYPE ) ),
    mpDoc(pDoc),
    maPos(rPos)
{
    SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());

    maLbType.SetSelectHdl( LINK( pParent, ScCondFormatList, TypeListHdl ) );
    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );
}

ScCondFrmtEntry::~ScCondFrmtEntry()
{
}

long ScCondFrmtEntry::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        ImplCallEventListenersAndHandler( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, maClickHdl, this );
    }
    return Control::Notify(rNEvt);
}

void ScCondFrmtEntry::SetIndex(sal_Int32 nIndex)
{
    mnIndex = nIndex;
    OUStringBuffer aBuffer(maStrCondition);
    aBuffer.append(OUString::number(nIndex));
    maFtCondNr.SetText(aBuffer.makeStringAndClear());
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

void ScCondFrmtEntry::Select()
{
    maFtCondition.SetText(OUString());
    maFtCondition.Hide();
    maLbType.Show();
    mbActive = true;
    SetHeight();
}

void ScCondFrmtEntry::Deselect()
{
    OUString maCondText = GetExpressionString();
    maFtCondition.SetText(maCondText);
    maFtCondition.Show();
    maLbType.Hide();
    mbActive = false;
    SetHeight();
}

bool ScCondFrmtEntry::IsSelected() const
{
    return mbActive;
}

IMPL_LINK(ScCondFrmtEntry, EdModifyHdl, Edit*, pEdit)
{
    OUString aFormula = pEdit->GetText();
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

//
//condition
//
namespace {

void FillStyleListBox( ScDocument* pDoc, ListBox& rLbStyle )
{
    rLbStyle.SetSeparatorPos(0);
    std::set<OUString> aStyleNames;
    SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        OUString aName = pStyle->GetName();
        aStyleNames.insert(aName);
    }
    for(std::set<OUString>::const_iterator itr = aStyleNames.begin(), itrEnd = aStyleNames.end();
                        itr != itrEnd; ++itr)
    {
        rLbStyle.InsertEntry( *itr );
    }
}

}

ScConditionFrmtEntry::ScConditionFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbCondType( this, ScResId( LB_CELLIS_TYPE ) ),
    maEdVal1( this, NULL, NULL, ScResId( ED_VAL1 ) ),
    maEdVal2( this, NULL, NULL, ScResId( ED_VAL2 ) ),
    maFtStyle( this, ScResId( FT_STYLE ) ),
    maLbStyle( this, ScResId( LB_STYLE ) ),
    maWdPreview( this, ScResId( WD_PREVIEW ) ),
    mbIsInStyleCreate(false)
{

    FreeResource();
    maLbType.SelectEntryPos(1);

    Init();

    StartListening(*pDoc->GetStyleSheetPool(), true);

    if(pFormatEntry)
    {
        OUString aStyleName = pFormatEntry->GetStyle();
        maLbStyle.SelectEntry(aStyleName);
        StyleSelectHdl(NULL);
        ScConditionMode eMode = pFormatEntry->GetOperation();
        maEdVal1.SetText(pFormatEntry->GetExpression(maPos, 0));
        maEdVal2.Hide();
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
                maEdVal2.Show();
                maEdVal2.SetText(pFormatEntry->GetExpression(maPos, 1));
                maLbCondType.SelectEntryPos(6);
                break;
            case SC_COND_NOTBETWEEN:
                maEdVal2.Show();
                maEdVal2.SetText(pFormatEntry->GetExpression(maPos, 1));
                maLbCondType.SelectEntryPos(7);
                break;
            case SC_COND_DUPLICATE:
                maLbCondType.SelectEntryPos(8);
                break;
            case SC_COND_NOTDUPLICATE:
                maLbCondType.SelectEntryPos(9);
                break;
            case SC_COND_DIRECT:
                assert(false);
                //maLbType.SelectEntryPos(2);
                break;
            case SC_COND_TOP10:
                maLbCondType.SelectEntryPos(10);
                break;
            case SC_COND_BOTTOM10:
                maLbCondType.SelectEntryPos(11);
                break;
            case SC_COND_TOP_PERCENT:
                maLbCondType.SelectEntryPos(12);
                break;
            case SC_COND_BOTTOM_PERCENT:
                maLbCondType.SelectEntryPos(13);
                break;
            case SC_COND_ABOVE_AVERAGE:
                maEdVal1.Hide();
                maLbCondType.SelectEntryPos(14);
                break;
            case SC_COND_BELOW_AVERAGE:
                maEdVal1.Hide();
                maLbCondType.SelectEntryPos(15);
                break;
            case SC_COND_ABOVE_EQUAL_AVERAGE:
                maEdVal1.Hide();
                maLbCondType.SelectEntryPos(16);
                break;
            case SC_COND_BELOW_EQUAL_AVERAGE:
                maEdVal1.Hide();
                maLbCondType.SelectEntryPos(17);
                break;
            case SC_COND_ERROR:
                maEdVal1.Hide();
                maLbCondType.SelectEntryPos(18);
                break;
            case SC_COND_NOERROR:
                maEdVal1.Hide();
                maLbCondType.SelectEntryPos(19);
                break;
            case SC_COND_BEGINS_WITH:
                maLbCondType.SelectEntryPos(20);
                break;
            case SC_COND_ENDS_WITH:
                maLbCondType.SelectEntryPos(21);
                break;
            case SC_COND_CONTAINS_TEXT:
                maLbCondType.SelectEntryPos(22);
                break;
            case SC_COND_NOT_CONTAINS_TEXT:
                maLbCondType.SelectEntryPos(23);
                break;
            case SC_COND_NONE:
                break;
        }
    }
    else
    {
        maLbCondType.SelectEntryPos(0);
        maEdVal2.Hide();
        maLbStyle.SelectEntryPos(1);
    }
    maLbType.SelectEntryPos(1);
}

void ScConditionFrmtEntry::Init()
{
    maEdVal1.SetGetFocusHdl( LINK( GetParent()->GetParent(), ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdVal2.SetGetFocusHdl( LINK( GetParent()->GetParent(), ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdVal1.SetLoseFocusHdl( LINK( GetParent()->GetParent(), ScCondFormatDlg, RangeLoseFocusHdl ) );
    maEdVal2.SetLoseFocusHdl( LINK( GetParent()->GetParent(), ScCondFormatDlg, RangeLoseFocusHdl ) );

    maEdVal1.SetStyle( maEdVal1.GetStyle() | WB_FORCECTRLBACKGROUND );
    maEdVal2.SetStyle( maEdVal2.GetStyle() | WB_FORCECTRLBACKGROUND );

    maEdVal1.SetModifyHdl( LINK( this, ScCondFrmtEntry, EdModifyHdl ) );
    maEdVal2.SetModifyHdl( LINK( this, ScCondFrmtEntry, EdModifyHdl ) );

    FillStyleListBox( mpDoc, maLbStyle );
    maLbStyle.SetSelectHdl( LINK( this, ScConditionFrmtEntry, StyleSelectHdl ) );

    maLbCondType.SetSelectHdl( LINK( this, ScConditionFrmtEntry, ConditionTypeSelectHdl ) );
}

ScFormatEntry* ScConditionFrmtEntry::createConditionEntry() const
{
    ScConditionMode eMode;
    OUString aExpr2;
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
        case 10:
            eMode = SC_COND_TOP10;
            break;
        case 11:
            eMode = SC_COND_BOTTOM10;
            break;
        case 12:
            eMode = SC_COND_TOP_PERCENT;
            break;
        case 13:
            eMode = SC_COND_BOTTOM_PERCENT;
            break;
        case 14:
            eMode = SC_COND_ABOVE_AVERAGE;
            break;
        case 15:
            eMode = SC_COND_BELOW_AVERAGE;
            break;
        case 16:
            eMode = SC_COND_ABOVE_EQUAL_AVERAGE;
            break;
        case 17:
            eMode = SC_COND_BELOW_EQUAL_AVERAGE;
            break;
        case 18:
            eMode = SC_COND_ERROR;
            break;
        case 19:
            eMode = SC_COND_NOERROR;
            break;
        case 20:
            eMode = SC_COND_BEGINS_WITH;
            break;
        case 21:
            eMode = SC_COND_ENDS_WITH;
            break;
        case 22:
            eMode = SC_COND_CONTAINS_TEXT;
            break;
        case 23:
            eMode = SC_COND_NOT_CONTAINS_TEXT;
            break;
        default:
            assert(false); // this cannot happen
            return NULL;
    }

    OUString aExpr1 = maEdVal1.GetText();

    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, mpDoc, maPos, maLbStyle.GetSelectEntry());

    return pEntry;
}

OUString ScConditionFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(CONDITION, maLbCondType.GetSelectEntryPos(), maEdVal1.GetText(), maEdVal2.GetText());
}

ScFormatEntry* ScConditionFrmtEntry::GetEntry() const
{
    return createConditionEntry();
}

void ScConditionFrmtEntry::SetActive()
{
    maLbCondType.Show();
    maEdVal1.Show();
    if(maLbCondType.GetSelectEntryPos() == 6 || maLbCondType.GetSelectEntryPos() == 7)
        maEdVal2.Show();
    maFtStyle.Show();
    maLbStyle.Show();
    maWdPreview.Show();

    Select();
}

void ScConditionFrmtEntry::SetInactive()
{
    maLbCondType.Hide();
    maEdVal1.Hide();
    maEdVal2.Hide();
    maFtStyle.Hide();
    maLbStyle.Hide();
    maWdPreview.Hide();

    Deselect();
}

namespace {

void UpdateStyleList(ListBox& rLbStyle, ScDocument* pDoc)
{
    OUString aSelectedStyle = rLbStyle.GetSelectEntry();
    for(sal_Int32 i = rLbStyle.GetEntryCount(); i >= 1; --i)
    {
        rLbStyle.RemoveEntry(i);
    }
    FillStyleListBox(pDoc, rLbStyle);
    rLbStyle.SelectEntry(aSelectedStyle);
}

}

void ScConditionFrmtEntry::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    SfxStyleSheetHint* pHint = PTR_CAST(SfxStyleSheetHint, &rHint);
    if(!pHint)
        return;

    sal_uInt16 nHint = pHint->GetHint();
    if(nHint == SFX_STYLESHEET_MODIFIED)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(maLbStyle, mpDoc);
    }
}

namespace {

void StyleSelect( ListBox& rLbStyle, ScDocument* pDoc, SvxFontPrevWindow& rWdPreview )
{
    if(rLbStyle.GetSelectEntryPos() == 0)
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
        SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
        bool bFound = false;
        for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle && !bFound; pStyle = aStyleIter.Next() )
        {
            OUString aName = pStyle->GetName();
            if ( rLbStyle.GetEntryPos(aName) == LISTBOX_ENTRY_NOTFOUND )    // all lists contain the same entries
            {
                for( sal_uInt16 i = 1, n = rLbStyle.GetEntryCount(); i <= n && !bFound; ++i)
                {
                    OUString aStyleName = ScGlobal::pCharClass->uppercase(OUString(rLbStyle.GetEntry(i)));
                    if( i == n )
                    {
                        rLbStyle.InsertEntry(aName);
                        rLbStyle.SelectEntry(aName);
                        bFound = true;
                    }
                    else if( aStyleName > ScGlobal::pCharClass->uppercase(aName) )
                    {
                        rLbStyle.InsertEntry(aName, i);
                        rLbStyle.SelectEntry(aName);
                        bFound = true;
                    }
                }
            }
        }
    }

    OUString aStyleName = rLbStyle.GetSelectEntry();
    SfxStyleSheetBase* pStyleSheet = pDoc->GetStyleSheetPool()->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
    if(pStyleSheet)
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        rWdPreview.Init( rSet );
    }
}

}

IMPL_LINK_NOARG(ScConditionFrmtEntry, StyleSelectHdl)
{
    mbIsInStyleCreate = true;
    StyleSelect( maLbStyle, mpDoc, maWdPreview );
    mbIsInStyleCreate = false;
    return 0;
}

// formula

ScFormulaFrmtEntry::ScFormulaFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maFtStyle( this, ScResId( FT_STYLE ) ),
    maLbStyle( this, ScResId( LB_STYLE ) ),
    maWdPreview( this, ScResId( WD_PREVIEW ) ),
    maEdFormula( this, NULL, NULL, ScResId( ED_FORMULA ) )
{
    Init();

    FreeResource();
    maLbType.SelectEntryPos(2);

    if(pFormat)
    {
        maEdFormula.SetText(pFormat->GetExpression(rPos, 0, 0, pDoc->GetGrammar()));
        maLbStyle.SelectEntry(pFormat->GetStyle());
    }
    else
    {
        maLbStyle.SelectEntryPos(1);
    }

    StyleSelectHdl(NULL);
}

void ScFormulaFrmtEntry::Init()
{
    maEdFormula.SetGetFocusHdl( LINK( GetParent()->GetParent(), ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdFormula.SetLoseFocusHdl( LINK( GetParent()->GetParent(), ScCondFormatDlg, RangeLoseFocusHdl ) );

    FillStyleListBox( mpDoc, maLbStyle );
    maLbStyle.SetSelectHdl( LINK( this, ScFormulaFrmtEntry, StyleSelectHdl ) );
}

IMPL_LINK_NOARG(ScFormulaFrmtEntry, StyleSelectHdl)
{
    StyleSelect( maLbStyle, mpDoc, maWdPreview );

    return 0;
}

ScFormatEntry* ScFormulaFrmtEntry::createFormulaEntry() const
{
    ScConditionMode eMode = SC_COND_DIRECT;
    OUString aFormula = maEdFormula.GetText();
    if(aFormula.isEmpty())
        return NULL;

    OUString aExpr2;
    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aFormula, aExpr2, mpDoc, maPos, maLbStyle.GetSelectEntry());
    return pEntry;
}

ScFormatEntry* ScFormulaFrmtEntry::GetEntry() const
{
    return createFormulaEntry();
}

OUString ScFormulaFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(FORMULA, 0, maEdFormula.GetText());
}

void ScFormulaFrmtEntry::SetActive()
{
    maWdPreview.Show();
    maFtStyle.Show();
    maLbStyle.Show();
    maEdFormula.Show();

    Select();
}

void ScFormulaFrmtEntry::SetInactive()
{
    maWdPreview.Hide();
    maFtStyle.Hide();
    maLbStyle.Hide();
    maEdFormula.Hide();

    Deselect();
}

//color scale

namespace {

OUString convertNumberToString(double nVal, ScDocument* pDoc)
{
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    OUString aText;
    pNumberFormatter->GetInputLineString(nVal, 0, aText);
    return aText;
}

void SetColorScaleEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit, ColorListBox& rLbCol, ScDocument* pDoc )
{
    // entry Automatic is not available for color scales
    sal_Int32 nIndex = static_cast<sal_Int32>(rEntry.GetType());
    assert(nIndex > 0);
    rLbType.SelectEntryPos(nIndex - 1);
    switch(rEntry.GetType())
    {
        case COLORSCALE_MIN:
        case COLORSCALE_MAX:
            break;
        case COLORSCALE_PERCENTILE:
        case COLORSCALE_VALUE:
        case COLORSCALE_PERCENT:
            {
                double nVal = rEntry.GetValue();
                rEdit.SetText(convertNumberToString(nVal, pDoc));
            }
            break;
        case COLORSCALE_FORMULA:
            rEdit.SetText(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            break;
        case COLORSCALE_AUTO:
            abort();
            break;
    }
    rLbCol.SelectEntry(rEntry.GetColor());
}

void SetColorScaleEntry( ScColorScaleEntry* pEntry, const ListBox& rType, const Edit& rValue, ScDocument* pDoc, const ScAddress& rPos, bool bDataBar )
{
    sal_uInt32 nIndex = 0;
    double nVal = 0;
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    pNumberFormatter->IsNumberFormat(rValue.GetText(), nIndex, nVal);

    // color scale does not have the automatic entry
    sal_Int32 nPos = rType.GetSelectEntryPos();
    if(!bDataBar)
        ++nPos;

    pEntry->SetType(static_cast<ScColorScaleEntryType>(nPos));
    switch(nPos)
    {
        case COLORSCALE_AUTO:
        case COLORSCALE_MIN:
        case COLORSCALE_MAX:
            break;
        case COLORSCALE_PERCENTILE:
        case COLORSCALE_VALUE:
        case COLORSCALE_PERCENT:
            pEntry->SetValue(nVal);
            break;
        case COLORSCALE_FORMULA:
            pEntry->SetFormula(rValue.GetText(), pDoc, rPos);
            break;
        default:
            break;
    }
}

ScColorScaleEntry* createColorScaleEntry( const ListBox& rType, const ColorListBox& rColor, const Edit& rValue, ScDocument* pDoc, const ScAddress& rPos )
{
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    SetColorScaleEntry( pEntry, rType, rValue, pDoc, rPos, false );
    Color aColor = rColor.GetSelectEntryColor();
    pEntry->SetColor(aColor);
    return pEntry;
}

}

ScColorScale2FrmtEntry::ScColorScale2FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
    maLbEntryTypeMin( this, ScResId( LB_TYPE_COL_SCALE_MIN ) ),
    maLbEntryTypeMax( this, ScResId( LB_TYPE_COL_SCALE_MAX ) ),
    maEdMin( this, ScResId( ED_COL_SCALE_MIN ) ),
    maEdMax( this, ScResId( ED_COL_SCALE_MAX ) ),
    maLbColMin( this, ScResId( LB_COL_MIN ) ),
    maLbColMax( this, ScResId( LB_COL_MAX ) )
{
    // remove the automatic entry from color scales
    maLbEntryTypeMin.RemoveEntry(0);
    maLbEntryTypeMax.RemoveEntry(0);

    maLbType.SelectEntryPos(0);
    maLbColorFormat.SelectEntryPos(0);
    Init();
    if(pFormat)
    {
        ScColorScaleFormat::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMin, maEdMin, maLbColMin, pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMax, maEdMax, maLbColMax, pDoc);
    }
    else
    {
        maLbEntryTypeMin.SelectEntryPos(0);
        maLbEntryTypeMax.SelectEntryPos(1);
    }
    FreeResource();

    maLbColorFormat.SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    EntryTypeHdl(&maLbEntryTypeMin);
    EntryTypeHdl(&maLbEntryTypeMax);
}

void ScColorScale2FrmtEntry::Init()
{
    maLbEntryTypeMin.SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax.SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );

    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }
    if ( pColorTable.is() )
    {
        // filling the line color box
        maLbColMin.SetUpdateMode( false );
        maLbColMax.SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            maLbColMin.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMax.InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                maLbColMin.SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                maLbColMax.SelectEntryPos(i);
        }
        maLbColMin.SetUpdateMode( sal_True );
        maLbColMax.SetUpdateMode( sal_True );
    }
}

ScFormatEntry* ScColorScale2FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMin, maLbColMin, maEdMin, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMax, maLbColMax, maEdMax, mpDoc, maPos));
    return pColorScale;
}

OUString ScColorScale2FrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression( COLORSCALE, 0 );
}

ScFormatEntry* ScColorScale2FrmtEntry::GetEntry() const
{
    return createColorscaleEntry();
}

void ScColorScale2FrmtEntry::SetActive()
{
    maLbColorFormat.Show();

    maLbEntryTypeMin.Show();
    maLbEntryTypeMax.Show();

    maEdMin.Show();
    maEdMax.Show();

    maLbColMin.Show();
    maLbColMax.Show();

    Select();
}

void ScColorScale2FrmtEntry::SetInactive()
{
    maLbColorFormat.Hide();

    maLbEntryTypeMin.Hide();
    maLbEntryTypeMax.Hide();

    maEdMin.Hide();
    maEdMax.Hide();

    maLbColMin.Hide();
    maLbColMax.Hide();

    Deselect();
}

IMPL_LINK( ScColorScale2FrmtEntry, EntryTypeHdl, ListBox*, pBox )
{
    bool bEnableEdit = true;
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    if(nPos < 2)
    {
        bEnableEdit = false;
    }

    Edit* pEd = NULL;
    if(pBox == &maLbEntryTypeMin)
        pEd = &maEdMin;
    else if(pBox == &maLbEntryTypeMax)
        pEd = &maEdMax;

    if(bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();

    return 0;
}

ScColorScale3FrmtEntry::ScColorScale3FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
    maLbEntryTypeMin( this, ScResId( LB_TYPE_COL_SCALE_MIN ) ),
    maLbEntryTypeMiddle( this, ScResId( LB_TYPE_COL_SCALE_MIDDLE ) ),
    maLbEntryTypeMax( this, ScResId( LB_TYPE_COL_SCALE_MAX ) ),
    maEdMin( this, ScResId( ED_COL_SCALE_MIN ) ),
    maEdMiddle( this, ScResId( ED_COL_SCALE_MIDDLE ) ),
    maEdMax( this, ScResId( ED_COL_SCALE_MAX ) ),
    maLbColMin( this, ScResId( LB_COL_MIN ) ),
    maLbColMiddle( this, ScResId( LB_COL_MIDDLE ) ),
    maLbColMax( this, ScResId( LB_COL_MAX ) )
{
    // remove the automatic entry from color scales
    maLbEntryTypeMin.RemoveEntry(0);
    maLbEntryTypeMiddle.RemoveEntry(0);
    maLbEntryTypeMax.RemoveEntry(0);
    maLbColorFormat.SelectEntryPos(1);

    Init();
    maLbType.SelectEntryPos(0);
    if(pFormat)
    {
        ScColorScaleFormat::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMin, maEdMin, maLbColMin, pDoc);
        assert(pFormat->size() == 3);
        ++itr;
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMiddle, maEdMiddle, maLbColMiddle, pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr, maLbEntryTypeMax, maEdMax, maLbColMax, pDoc);
    }
    else
    {
        maLbColorFormat.SelectEntryPos(1);
        maLbEntryTypeMin.SelectEntryPos(0);
        maLbEntryTypeMiddle.SelectEntryPos(2);
        maLbEntryTypeMax.SelectEntryPos(1);
        maEdMiddle.SetText(OUString::number(50));
    }
    FreeResource();

    maLbColorFormat.SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );
    EntryTypeHdl(&maLbEntryTypeMin);
    EntryTypeHdl(&maLbEntryTypeMiddle);
    EntryTypeHdl(&maLbEntryTypeMax);
}

void ScColorScale3FrmtEntry::Init()
{
    maLbEntryTypeMin.SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax.SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMiddle.SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );

    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        const SfxPoolItem* pItem = pDocSh->GetItem( SID_COLOR_TABLE );
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
}

ScFormatEntry* ScColorScale3FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMin, maLbColMin, maEdMin, mpDoc, maPos));
    if(maLbColorFormat.GetSelectEntryPos() == 1)
        pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMiddle, maLbColMiddle, maEdMiddle, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(maLbEntryTypeMax, maLbColMax, maEdMax, mpDoc, maPos));
    return pColorScale;
}

OUString ScColorScale3FrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression( COLORSCALE, 0 );
}

ScFormatEntry* ScColorScale3FrmtEntry::GetEntry() const
{
    return createColorscaleEntry();
}

void ScColorScale3FrmtEntry::SetActive()
{
    maLbColorFormat.Show();
    maLbEntryTypeMin.Show();
    maLbEntryTypeMiddle.Show();
    maLbEntryTypeMax.Show();

    maEdMin.Show();
    maEdMiddle.Show();
    maEdMax.Show();

    maLbColMin.Show();
    maLbColMiddle.Show();
    maLbColMax.Show();

    Select();
}

void ScColorScale3FrmtEntry::SetInactive()
{
    maLbColorFormat.Hide();

    maLbEntryTypeMin.Hide();
    maLbEntryTypeMiddle.Hide();
    maLbEntryTypeMax.Hide();

    maEdMin.Hide();
    maEdMiddle.Hide();
    maEdMax.Hide();

    maLbColMin.Hide();
    maLbColMiddle.Hide();
    maLbColMax.Hide();

    Deselect();
}

IMPL_LINK( ScColorScale3FrmtEntry, EntryTypeHdl, ListBox*, pBox )
{
    bool bEnableEdit = true;
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    if(nPos < 2)
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

IMPL_LINK_NOARG( ScConditionFrmtEntry, ConditionTypeSelectHdl )
{
    sal_Int32 nSelectPos = maLbCondType.GetSelectEntryPos();
    if(nSelectPos == 6 || nSelectPos == 7)
    {
        maEdVal1.Show();
        maEdVal2.Show();
    }
    else if(nSelectPos == 8 || nSelectPos == 9)
    {
        maEdVal2.Hide();
        maEdVal1.Hide();
    }
    else if(nSelectPos <= 5 || (nSelectPos >= 10 && nSelectPos <= 13)
            || nSelectPos >= 18)
    {
        maEdVal1.Show();
        maEdVal2.Hide();
    }
    else
    {
        maEdVal1.Hide();
        maEdVal2.Hide();
    }

    return 0;
}

//databar

namespace {

void SetDataBarEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit, ScDocument* pDoc )
{
    rLbType.SelectEntryPos(rEntry.GetType());
    switch(rEntry.GetType())
    {
        case COLORSCALE_AUTO:
        case COLORSCALE_MIN:
        case COLORSCALE_MAX:
            break;
        case COLORSCALE_VALUE:
        case COLORSCALE_PERCENT:
        case COLORSCALE_PERCENTILE:
            {
                double nVal = rEntry.GetValue();
                SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
                OUString aText;
                pNumberFormatter->GetInputLineString(nVal, 0, aText);
                rEdit.SetText(aText);
            }
            break;
        case COLORSCALE_FORMULA:
            rEdit.SetText(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            break;
    }
}

}

ScDataBarFrmtEntry::ScDataBarFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
    maLbDataBarMinType( this, ScResId( LB_TYPE_COL_SCALE_MIN ) ),
    maLbDataBarMaxType( this, ScResId( LB_TYPE_COL_SCALE_MAX ) ),
    maEdDataBarMin( this, ScResId( ED_COL_SCALE_MIN ) ),
    maEdDataBarMax( this, ScResId( ED_COL_SCALE_MAX ) ),
    maBtOptions( this, ScResId( BTN_OPTIONS ) )
{
    maLbColorFormat.SelectEntryPos(2);
    maLbType.SelectEntryPos(0);
    if(pFormat)
    {
        mpDataBarData.reset(new ScDataBarFormatData(*pFormat->GetDataBarData()));
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, maLbDataBarMinType, maEdDataBarMin, pDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, maLbDataBarMaxType, maEdDataBarMax, pDoc);
        DataBarTypeSelectHdl(NULL);
    }
    else
    {
        maLbDataBarMinType.SelectEntryPos(0);
        maLbDataBarMaxType.SelectEntryPos(0);
        DataBarTypeSelectHdl(NULL);
    }
    Init();

    maLbColorFormat.SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    FreeResource();
}

ScFormatEntry* ScDataBarFrmtEntry::GetEntry() const
{
    return createDatabarEntry();
}

void ScDataBarFrmtEntry::Init()
{
    maLbDataBarMinType.SetSelectHdl( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );
    maLbDataBarMaxType.SetSelectHdl( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );

    maBtOptions.SetClickHdl( LINK( this, ScDataBarFrmtEntry, OptionBtnHdl ) );

    if(!mpDataBarData)
    {
        mpDataBarData.reset(new ScDataBarFormatData());
        mpDataBarData->mpUpperLimit.reset(new ScColorScaleEntry());
        mpDataBarData->mpLowerLimit.reset(new ScColorScaleEntry());
        mpDataBarData->mpLowerLimit->SetType(COLORSCALE_AUTO);
        mpDataBarData->mpUpperLimit->SetType(COLORSCALE_AUTO);
        mpDataBarData->maPositiveColor = COL_LIGHTBLUE;
    }
}

ScFormatEntry* ScDataBarFrmtEntry::createDatabarEntry() const
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), maLbDataBarMinType, maEdDataBarMin, mpDoc, maPos, true);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), maLbDataBarMaxType, maEdDataBarMax, mpDoc, maPos, true);
    ScDataBarFormat* pDataBar = new ScDataBarFormat(mpDoc);
    pDataBar->SetDataBarData(new ScDataBarFormatData(*mpDataBarData.get()));
    return pDataBar;
}

OUString ScDataBarFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression( DATABAR, 0 );
}

void ScDataBarFrmtEntry::SetActive()
{
    maLbColorFormat.Show();

    maLbDataBarMinType.Show();
    maLbDataBarMaxType.Show();
    maEdDataBarMin.Show();
    maEdDataBarMax.Show();
    maBtOptions.Show();

    Select();
}

void ScDataBarFrmtEntry::SetInactive()
{
    maLbColorFormat.Hide();

    maLbDataBarMinType.Hide();
    maLbDataBarMaxType.Hide();
    maEdDataBarMin.Hide();
    maEdDataBarMax.Hide();
    maBtOptions.Hide();

    Deselect();
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, DataBarTypeSelectHdl )
{
    sal_Int32 nSelectPos = maLbDataBarMinType.GetSelectEntryPos();
    if(nSelectPos <= COLORSCALE_MAX)
        maEdDataBarMin.Disable();
    else
        maEdDataBarMin.Enable();

    nSelectPos = maLbDataBarMaxType.GetSelectEntryPos();
    if(nSelectPos <= COLORSCALE_MAX)
        maEdDataBarMax.Disable();
    else
        maEdDataBarMax.Enable();

    return 0;
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, OptionBtnHdl )
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), maLbDataBarMinType, maEdDataBarMin, mpDoc, maPos, true);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), maLbDataBarMaxType, maEdDataBarMax, mpDoc, maPos, true);
    ScDataBarSettingsDlg* pDlg = new ScDataBarSettingsDlg(this, *mpDataBarData, mpDoc, maPos);
    if( pDlg->Execute() == RET_OK)
    {
        mpDataBarData.reset(pDlg->GetData());
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, maLbDataBarMinType, maEdDataBarMin, mpDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, maLbDataBarMaxType, maEdDataBarMax, mpDoc);
        DataBarTypeSelectHdl(NULL);
    }
    return 0;
}

ScDateFrmtEntry::ScDateFrmtEntry( Window* pParent, ScDocument* pDoc, const ScCondDateFormatEntry* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, ScAddress() ),
    maLbDateEntry( this, ScResId( LB_DATE_TYPE ) ),
    maFtStyle( this, ScResId( FT_STYLE ) ),
    maLbStyle( this, ScResId( LB_STYLE ) ),
    maWdPreview( this, ScResId( WD_PREVIEW ) ),
    mbIsInStyleCreate(false)
{
    Init();
    FreeResource();

    StartListening(*pDoc->GetStyleSheetPool(), sal_True);

    if(pFormat)
    {
        sal_Int32 nPos = static_cast<sal_Int32>(pFormat->GetDateType());
        maLbDateEntry.SelectEntryPos(nPos);

        OUString aStyleName = pFormat->GetStyleName();
        maLbStyle.SelectEntry(aStyleName);
    }

    StyleSelectHdl(NULL);
}

void ScDateFrmtEntry::Init()
{
    maLbDateEntry.SelectEntryPos(0);
    maLbType.SelectEntryPos(3);

    FillStyleListBox( mpDoc, maLbStyle );
    maLbStyle.SetSelectHdl( LINK( this, ScDateFrmtEntry, StyleSelectHdl ) );
    maLbStyle.SelectEntryPos(1);
}

void ScDateFrmtEntry::SetActive()
{
    maLbDateEntry.Show();
    maFtStyle.Show();
    maWdPreview.Show();
    maLbStyle.Show();

    Select();
}

void ScDateFrmtEntry::SetInactive()
{
    maLbDateEntry.Hide();
    maFtStyle.Hide();
    maWdPreview.Hide();
    maLbStyle.Hide();

    Deselect();
}

void ScDateFrmtEntry::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    SfxStyleSheetHint* pHint = PTR_CAST(SfxStyleSheetHint, &rHint);
    if(!pHint)
        return;

    sal_uInt16 nHint = pHint->GetHint();
    if(nHint == SFX_STYLESHEET_MODIFIED)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(maLbStyle, mpDoc);
    }
}

ScFormatEntry* ScDateFrmtEntry::GetEntry() const
{
    ScCondDateFormatEntry* pNewEntry = new ScCondDateFormatEntry(mpDoc);
    condformat::ScCondFormatDateType eType = static_cast<condformat::ScCondFormatDateType>(maLbDateEntry.GetSelectEntryPos());
    pNewEntry->SetDateType(eType);
    pNewEntry->SetStyleName(maLbStyle.GetSelectEntry());
    return pNewEntry;
}

OUString ScDateFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(DATE, 0);
}

IMPL_LINK_NOARG( ScDateFrmtEntry, StyleSelectHdl )
{
    mbIsInStyleCreate = true;
    StyleSelect( maLbStyle, mpDoc, maWdPreview );
    mbIsInStyleCreate = false;

    return 0;
}

class ScIconSetFrmtDataEntry : public Control
{
    private:
        FixedImage maImgIcon;
        FixedText maFtEntry;
        Edit maEdEntry;
        ListBox maLbEntryType;

    public:
        ScIconSetFrmtDataEntry( Window* pParent, ScIconSetType eType, ScDocument* pDoc,
                sal_Int32 i, const ScColorScaleEntry* pEntry = NULL );

        ScColorScaleEntry* CreateEntry(ScDocument* pDoc, const ScAddress& rPos) const;

        void SetFirstEntry();
};

ScIconSetFrmtDataEntry::ScIconSetFrmtDataEntry( Window* pParent, ScIconSetType eType, ScDocument* pDoc, sal_Int32 i, const ScColorScaleEntry* pEntry ):
    Control( pParent, ScResId( RID_ICON_SET_ENTRY ) ),
    maImgIcon( this, ScResId( IMG_ICON ) ),
    maFtEntry( this, ScResId( FT_ICON_SET_ENTRY_TEXT ) ),
    maEdEntry( this, ScResId( ED_ICON_SET_ENTRY_VALUE ) ),
    maLbEntryType( this, ScResId( LB_ICON_SET_ENTRY_TYPE ) )
{
    maImgIcon.SetImage(ScIconSetFormat::getBitmap( eType, i ));
    if(pEntry)
    {
        switch(pEntry->GetType())
        {
            case COLORSCALE_VALUE:
                maLbEntryType.SelectEntryPos(0);
                maEdEntry.SetText(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_PERCENTILE:
                maLbEntryType.SelectEntryPos(2);
                maEdEntry.SetText(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_PERCENT:
                maLbEntryType.SelectEntryPos(1);
                maEdEntry.SetText(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_FORMULA:
                maLbEntryType.SelectEntryPos(3);
                maEdEntry.SetText(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
                break;
            default:
                assert(false);
        }
    }
    else
    {
        maLbEntryType.SelectEntryPos(1);
    }
    FreeResource();
}

ScColorScaleEntry* ScIconSetFrmtDataEntry::CreateEntry(ScDocument* pDoc, const ScAddress& rPos) const
{
    sal_Int32 nPos = maLbEntryType.GetSelectEntryPos();
    OUString aText = maEdEntry.GetText();
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    sal_uInt32 nIndex = 0;
    double nVal = 0;
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    pNumberFormatter->IsNumberFormat(aText, nIndex, nVal);
    pEntry->SetValue(nVal);

    switch(nPos)
    {
        case 0:
            pEntry->SetType(COLORSCALE_VALUE);
            break;
        case 1:
            pEntry->SetType(COLORSCALE_PERCENT);
            break;
        case 2:
            pEntry->SetType(COLORSCALE_PERCENTILE);
            break;
        case 3:
            pEntry->SetType(COLORSCALE_FORMULA);
            pEntry->SetFormula(aText, pDoc, rPos, pDoc->GetGrammar());
            break;
        default:
            assert(false);
    }

    return pEntry;
}

void ScIconSetFrmtDataEntry::SetFirstEntry()
{
    maEdEntry.Hide();
    maLbEntryType.Hide();
    maFtEntry.Hide();
    maEdEntry.SetText(OUString("0"));
    maLbEntryType.SelectEntryPos(1);
}

ScIconSetFrmtEntry::ScIconSetFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScIconSetFormat* pFormat ):
        ScCondFrmtEntry( pParent, pDoc, rPos ),
        maLbColorFormat( this, ScResId( LB_COLOR_FORMAT ) ),
        maLbIconSetType( this, ScResId( LB_ICONSET_TYPE ) )
{
    Init();
    FreeResource();
    maLbColorFormat.SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    if(pFormat)
    {
        const ScIconSetFormatData* pIconSetFormatData = pFormat->GetIconSetData();
        ScIconSetType eType = pIconSetFormatData->eIconSetType;
        sal_Int32 nType = static_cast<sal_Int32>(eType);
        maLbIconSetType.SelectEntryPos(nType);

        for(size_t i = 0, n = pIconSetFormatData->maEntries.size();
                i < n; ++i)
        {
            maEntries.push_back( new ScIconSetFrmtDataEntry( this, eType, pDoc, i, &pIconSetFormatData->maEntries[i] ) );
            Point aPos = maEntries[0].GetPosPixel();
            aPos.Y() += maEntries[0].GetSizePixel().Height() * i * 1.2;
            maEntries[i].SetPosPixel( aPos );
        }
        maEntries.begin()->SetFirstEntry();
    }
    else
        IconSetTypeHdl(NULL);
}

void ScIconSetFrmtEntry::Init()
{
    maLbColorFormat.SelectEntryPos(3);
    maLbType.SelectEntryPos(0);
    maLbIconSetType.SelectEntryPos(0);

    maLbIconSetType.SetSelectHdl( LINK( this, ScIconSetFrmtEntry, IconSetTypeHdl ) );
}

IMPL_LINK_NOARG( ScIconSetFrmtEntry, IconSetTypeHdl )
{
    ScIconSetMap* pMap = ScIconSetFormat::getIconSetMap();

    sal_Int32 nPos = maLbIconSetType.GetSelectEntryPos();
    sal_uInt32 nElements = pMap[nPos].nElements;
    maEntries.clear();

    for(size_t i = 0; i < nElements; ++i)
    {
        maEntries.push_back( new ScIconSetFrmtDataEntry( this, static_cast<ScIconSetType>(nPos), mpDoc, i ) );
        Point aPos = maEntries[0].GetPosPixel();
        aPos.Y() += maEntries[0].GetSizePixel().Height() * i * 1.2;
        maEntries[i].SetPosPixel( aPos );
        maEntries[i].Show();
    }
    maEntries.begin()->SetFirstEntry();

    SetHeight();
    static_cast<ScCondFormatList*>(GetParent())->RecalcAll();

    return 0;
}

OUString ScIconSetFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(ICONSET, 0);
}

void ScIconSetFrmtEntry::SetActive()
{
    maLbColorFormat.Show();
    maLbIconSetType.Show();
    for(ScIconSetFrmtDataEntriesType::iterator itr = maEntries.begin(),
            itrEnd = maEntries.end(); itr != itrEnd; ++itr)
    {
        itr->Show();
    }

    Select();
}

void ScIconSetFrmtEntry::SetInactive()
{
    maLbColorFormat.Hide();
    maLbIconSetType.Hide();
    for(ScIconSetFrmtDataEntriesType::iterator itr = maEntries.begin(),
            itrEnd = maEntries.end(); itr != itrEnd; ++itr)
    {
        itr->Hide();
    }

    Deselect();
}

ScFormatEntry* ScIconSetFrmtEntry::GetEntry() const
{
    ScIconSetFormat* pFormat = new ScIconSetFormat(mpDoc);

    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->eIconSetType = static_cast<ScIconSetType>(maLbIconSetType.GetSelectEntryPos());
    for(ScIconSetFrmtDataEntriesType::const_iterator itr = maEntries.begin(),
            itrEnd = maEntries.end(); itr != itrEnd; ++itr)
    {
        pData->maEntries.push_back(itr->CreateEntry(mpDoc, maPos));
    }
    pFormat->SetIconSetData(pData);

    return pFormat;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
