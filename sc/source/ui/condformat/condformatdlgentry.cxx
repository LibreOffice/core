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
#include <vcl/settings.hxx>
#include <formula/token.hxx>
#include "tokenarray.hxx"
#include "stlpool.hxx"
#include "tabvwsh.hxx"
#include "simpleformulacalc.hxx"

#include "colorformat.hxx"

#include "globstr.hrc"

#include <set>

ScCondFrmtEntry::ScCondFrmtEntry(vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos):
    Control(pParent, ScResId( RID_COND_ENTRY ) ),
    mbActive(false),
    maFtCondNr( VclPtr<FixedText>::Create( this, ScResId( FT_COND_NR ) ) ),
    maFtCondition( VclPtr<FixedText>::Create( this, ScResId( FT_CONDITION ) ) ),
    mnIndex(0),
    maStrCondition(ScResId( STR_CONDITION ).toString()),
    maLbType( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE ) ) ),
    mpDoc(pDoc),
    maPos(rPos)
{
    Color aBack(GetSettings().GetStyleSettings().GetWindowColor());

    SetControlBackground(aBack);
    SetBackground(GetControlBackground());

    maFtCondNr->SetControlBackground(aBack);
    maFtCondNr->SetBackground(maFtCondNr->GetControlBackground());

    maFtCondition->SetControlBackground(aBack);
    maFtCondition->SetBackground(maFtCondition->GetControlBackground());

    maLbType->SetSelectHdl( LINK( pParent, ScCondFormatList, TypeListHdl ) );
    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );
}

ScCondFrmtEntry::~ScCondFrmtEntry()
{
    disposeOnce();
}

void ScCondFrmtEntry::dispose()
{
    maFtCondNr.disposeAndClear();
    maFtCondition.disposeAndClear();
    maLbType.disposeAndClear();
    Control::dispose();
}

bool ScCondFrmtEntry::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN )
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
    maFtCondNr->SetText(aBuffer.makeStringAndClear());
}

void ScCondFrmtEntry::SetHeight()
{
    long nPad = LogicToPixel(Size(42,2), MapMode(MAP_APPFONT)).getHeight();

    // Calculate maximum height we need from visible widgets
    sal_uInt16 nChildren = GetChildCount();

    long nMaxHeight = 0;
    for(sal_uInt16 i = 0; i < nChildren; i++)
    {
        vcl::Window *pChild  = GetChild(i);
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
    maFtCondition->SetText(OUString());
    maFtCondition->Hide();
    maLbType->Show();
    mbActive = true;
    SetHeight();
}

void ScCondFrmtEntry::Deselect()
{
    OUString maCondText = GetExpressionString();
    maFtCondition->SetText(maCondText);
    maFtCondition->Show();
    maLbType->Hide();
    mbActive = false;
    SetHeight();
}

//condition

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

const ScConditionMode ScConditionFrmtEntry::mpEntryToCond[ScConditionFrmtEntry::NUM_COND_ENTRIES] = {
    SC_COND_EQUAL,
    SC_COND_LESS,
    SC_COND_GREATER,
    SC_COND_EQLESS,
    SC_COND_EQGREATER,
    SC_COND_NOTEQUAL,
    SC_COND_BETWEEN,
    SC_COND_NOTBETWEEN,
    SC_COND_DUPLICATE,
    SC_COND_NOTDUPLICATE,
    SC_COND_TOP10,
    SC_COND_BOTTOM10,
    SC_COND_TOP_PERCENT,
    SC_COND_BOTTOM_PERCENT,
    SC_COND_ABOVE_AVERAGE,
    SC_COND_BELOW_AVERAGE,
    SC_COND_ABOVE_EQUAL_AVERAGE,
    SC_COND_BELOW_EQUAL_AVERAGE,
    SC_COND_ERROR,
    SC_COND_NOERROR,
    SC_COND_BEGINS_WITH,
    SC_COND_ENDS_WITH,
    SC_COND_CONTAINS_TEXT,
    SC_COND_NOT_CONTAINS_TEXT
};

ScConditionFrmtEntry::ScConditionFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent,
        const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbCondType( VclPtr<ListBox>::Create( this, ScResId( LB_CELLIS_TYPE ) ) ),
    maEdVal1( VclPtr<formula::RefEdit>::Create( this, nullptr, nullptr, ScResId( ED_VAL1 ) ) ),
    maEdVal2( VclPtr<formula::RefEdit>::Create( this, nullptr, nullptr, ScResId( ED_VAL2 ) ) ),
    maFtVal( VclPtr<FixedText>::Create( this, ScResId( FT_VAL ) ) ),
    maFtStyle( VclPtr<FixedText>::Create( this, ScResId( FT_STYLE ) ) ),
    maLbStyle( VclPtr<ListBox>::Create( this, ScResId( LB_STYLE ) ) ),
    maWdPreview( VclPtr<SvxFontPrevWindow>::Create( this, ScResId( WD_PREVIEW ) ) ),
    mbIsInStyleCreate(false)
{

    FreeResource();
    maLbType->SelectEntryPos(1);

    Init(pDialogParent);

    StartListening(*pDoc->GetStyleSheetPool(), true);

    if(pFormatEntry)
    {
        OUString aStyleName = pFormatEntry->GetStyle();
        maLbStyle->SelectEntry(aStyleName);
        StyleSelectHdl(NULL);
        ScConditionMode eMode = pFormatEntry->GetOperation();

        maLbCondType->SelectEntryPos(ConditionModeToEntryPos(eMode));

        switch(GetNumberEditFields(eMode))
        {
            case 0:
                maEdVal1->Hide();
                maEdVal2->Hide();
                break;
            case 1:
                maEdVal1->Show();
                maEdVal1->SetText(pFormatEntry->GetExpression(maPos, 0));
                maEdVal2->Hide();
                OnEdChanged(maEdVal1);
                break;
            case 2:
                maEdVal1->Show();
                maEdVal1->SetText(pFormatEntry->GetExpression(maPos, 0));
                OnEdChanged(maEdVal1);
                maEdVal2->Show();
                maEdVal2->SetText(pFormatEntry->GetExpression(maPos, 1));
                OnEdChanged(maEdVal2);
                break;
        }
    }
    else
    {
        maLbCondType->SelectEntryPos(0);
        maEdVal2->Hide();
        maLbStyle->SelectEntryPos(1);
    }
}

ScConditionFrmtEntry::~ScConditionFrmtEntry()
{
    disposeOnce();
}

void ScConditionFrmtEntry::dispose()
{
    maLbCondType.disposeAndClear();
    maEdVal1.disposeAndClear();
    maEdVal2.disposeAndClear();
    maFtVal.disposeAndClear();
    maFtStyle.disposeAndClear();
    maLbStyle.disposeAndClear();
    maWdPreview.disposeAndClear();
    ScCondFrmtEntry::dispose();
}

void ScConditionFrmtEntry::Init(ScCondFormatDlg* pDialogParent)
{
    maEdVal1->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdVal2->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdVal1->SetLoseFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeLoseFocusHdl ) );
    maEdVal2->SetLoseFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeLoseFocusHdl ) );

    maEdVal1->SetStyle( maEdVal1->GetStyle() | WB_FORCECTRLBACKGROUND );
    maEdVal2->SetStyle( maEdVal2->GetStyle() | WB_FORCECTRLBACKGROUND );

    maEdVal1->SetModifyHdl( LINK( this, ScConditionFrmtEntry, OnEdChanged ) );
    maEdVal2->SetModifyHdl( LINK( this, ScConditionFrmtEntry, OnEdChanged ) );

    FillStyleListBox( mpDoc, *maLbStyle.get() );
    maLbStyle->SetSelectHdl( LINK( this, ScConditionFrmtEntry, StyleSelectHdl ) );

    maLbCondType->SetSelectHdl( LINK( this, ScConditionFrmtEntry, ConditionTypeSelectHdl ) );
}

ScFormatEntry* ScConditionFrmtEntry::createConditionEntry() const
{
    ScConditionMode eMode = EntryPosToConditionMode(maLbCondType->GetSelectEntryPos());
    OUString aExpr1 = maEdVal1->GetText();
    OUString aExpr2;
    if (GetNumberEditFields(eMode) == 2)
    {
        aExpr2 = maEdVal2->GetText();
        if (aExpr2.isEmpty())
        {
            return NULL;
        }
    }

    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, mpDoc, maPos, maLbStyle->GetSelectEntry());
    return pEntry;
}

IMPL_LINK(ScConditionFrmtEntry, OnEdChanged, Edit*, pEdit)
{
    OUString aFormula = pEdit->GetText();

    if( aFormula.isEmpty() )
    {
        maFtVal->SetText(ScGlobal::GetRscString(STR_ENTER_VALUE));
        return 0;
    }

    ScCompiler aComp( mpDoc, maPos );
    aComp.SetGrammar( mpDoc->GetGrammar() );
    boost::scoped_ptr<ScTokenArray> ta(aComp.CompileString(aFormula));

    // Error, warn the user
    if( ta->GetCodeError() || ( ta->GetLen() == 0 ) )
    {
        pEdit->SetControlBackground(COL_LIGHTRED);
        maFtVal->SetText(ScGlobal::GetRscString(STR_VALID_DEFERROR));
        return 0;
    }

    // Recognized col/row name or string token, warn the user
    formula::FormulaToken* token = ta->First();
    formula::StackVar t = token->GetType();
    OpCode op = token->GetOpCode();
    if( ( op == ocColRowName ) ||
        ( ( op == ocBad ) && ( t == formula::svString ) )
      )
    {
        pEdit->SetControlBackground(COL_YELLOW);
        maFtVal->SetText(ScGlobal::GetRscString(STR_UNQUOTED_STRING));
        return 0;
    }

    pEdit->SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    maFtVal->SetText("");
    return 0;
}

void ScConditionFrmtEntry::Select()
{
    maFtVal->Show();
    ScCondFrmtEntry::Select();
}

void ScConditionFrmtEntry::Deselect()
{
    maFtVal->Hide();
    ScCondFrmtEntry::Deselect();
}


sal_Int32 ScConditionFrmtEntry::ConditionModeToEntryPos( ScConditionMode eMode )
{
    for ( sal_Int32 i = 0; i < NUM_COND_ENTRIES; ++i )
    {
        if (mpEntryToCond[i] == eMode)
        {
            return i;
        }
    }
    assert(false); // should never get here
    return 0;
}

ScConditionMode ScConditionFrmtEntry::EntryPosToConditionMode( sal_Int32 aEntryPos )
{
    assert( 0 <= aEntryPos && aEntryPos < NUM_COND_ENTRIES );
    return mpEntryToCond[aEntryPos];
}

sal_Int32 ScConditionFrmtEntry::GetNumberEditFields( ScConditionMode eMode )
{
    switch(eMode)
    {
        case SC_COND_EQUAL:
        case SC_COND_LESS:
        case SC_COND_GREATER:
        case SC_COND_EQLESS:
        case SC_COND_EQGREATER:
        case SC_COND_NOTEQUAL:
        case SC_COND_TOP10:
        case SC_COND_BOTTOM10:
        case SC_COND_TOP_PERCENT:
        case SC_COND_BOTTOM_PERCENT:
        case SC_COND_BEGINS_WITH:
        case SC_COND_ENDS_WITH:
        case SC_COND_CONTAINS_TEXT:
        case SC_COND_NOT_CONTAINS_TEXT:
        case SC_COND_ERROR:
        case SC_COND_NOERROR:
            return 1;
        case SC_COND_ABOVE_AVERAGE:
        case SC_COND_BELOW_AVERAGE:
        case SC_COND_ABOVE_EQUAL_AVERAGE:
        case SC_COND_BELOW_EQUAL_AVERAGE:
        case SC_COND_DUPLICATE:
        case SC_COND_NOTDUPLICATE:
            return 0;
        case SC_COND_BETWEEN:
        case SC_COND_NOTBETWEEN:
            return 2;
        default:
            assert(false); // should never get here
            return 0;
    }
}

OUString ScConditionFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(CONDITION, maLbCondType->GetSelectEntryPos(), maEdVal1->GetText(), maEdVal2->GetText());
}

ScFormatEntry* ScConditionFrmtEntry::GetEntry() const
{
    return createConditionEntry();
}

void ScConditionFrmtEntry::SetActive()
{
    ScConditionMode eMode = EntryPosToConditionMode(maLbCondType->GetSelectEntryPos());
    maLbCondType->Show();
    switch(GetNumberEditFields(eMode))
    {
        case 1:
            maEdVal1->Show();
            break;
        case 2:
            maEdVal1->Show();
            maEdVal2->Show();
            break;
    }
    maFtStyle->Show();
    maLbStyle->Show();
    maWdPreview->Show();

    Select();
}

void ScConditionFrmtEntry::SetInactive()
{
    maLbCondType->Hide();
    maEdVal1->Hide();
    maEdVal2->Hide();
    maFtStyle->Hide();
    maLbStyle->Hide();
    maWdPreview->Hide();

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
    const SfxStyleSheetHint* pHint = dynamic_cast<const SfxStyleSheetHint*>(&rHint);
    if(!pHint)
        return;

    sal_uInt16 nHint = pHint->GetHint();
    if(nHint == SfxStyleSheetHintId::MODIFIED)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(*maLbStyle.get(), mpDoc);
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
        bool bLocked = pDisp->IsLocked();
        if (bLocked)
            pDisp->Lock(false);

        // Execute the "new style" slot, complete with undo and all necessary updates.
        // The return value (SfxUInt16Item) is ignored, look for new styles instead.
        pDisp->Execute( SID_STYLE_NEW, SfxCallMode::SYNCHRON | SfxCallMode::RECORD | SfxCallMode::MODAL,
                &aFamilyItem,
                &aRefItem,
                0L );

        if (bLocked)
            pDisp->Lock(true);

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
    StyleSelect( *maLbStyle.get(), mpDoc, *maWdPreview.get() );
    mbIsInStyleCreate = false;
    return 0;
}

// formula

ScFormulaFrmtEntry::ScFormulaFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent, const ScAddress& rPos, const ScCondFormatEntry* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maFtStyle( VclPtr<FixedText>::Create( this, ScResId( FT_STYLE ) ) ),
    maLbStyle( VclPtr<ListBox>::Create( this, ScResId( LB_STYLE ) ) ),
    maWdPreview( VclPtr<SvxFontPrevWindow>::Create( this, ScResId( WD_PREVIEW ) ) ),
    maEdFormula( VclPtr<formula::RefEdit>::Create(this, nullptr, nullptr, ScResId( ED_FORMULA ) ) )
{
    Init(pDialogParent);

    FreeResource();
    maLbType->SelectEntryPos(2);

    if(pFormat)
    {
        maEdFormula->SetText(pFormat->GetExpression(rPos, 0, 0, pDoc->GetGrammar()));
        maLbStyle->SelectEntry(pFormat->GetStyle());
    }
    else
    {
        maLbStyle->SelectEntryPos(1);
    }

    StyleSelectHdl(NULL);
}

ScFormulaFrmtEntry::~ScFormulaFrmtEntry()
{
    disposeOnce();
}

void ScFormulaFrmtEntry::dispose()
{
    maFtStyle.disposeAndClear();
    maLbStyle.disposeAndClear();
    maWdPreview.disposeAndClear();
    maEdFormula.disposeAndClear();
    ScCondFrmtEntry::dispose();
}

void ScFormulaFrmtEntry::Init(ScCondFormatDlg* pDialogParent)
{
    maEdFormula->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdFormula->SetLoseFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeLoseFocusHdl ) );

    FillStyleListBox( mpDoc, *maLbStyle.get() );
    maLbStyle->SetSelectHdl( LINK( this, ScFormulaFrmtEntry, StyleSelectHdl ) );
}

IMPL_LINK_NOARG(ScFormulaFrmtEntry, StyleSelectHdl)
{
    StyleSelect( *maLbStyle.get(), mpDoc, *maWdPreview.get() );

    return 0;
}

ScFormatEntry* ScFormulaFrmtEntry::createFormulaEntry() const
{
    ScConditionMode eMode = SC_COND_DIRECT;
    OUString aFormula = maEdFormula->GetText();
    if(aFormula.isEmpty())
        return NULL;

    OUString aExpr2;
    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aFormula, aExpr2, mpDoc, maPos, maLbStyle->GetSelectEntry());
    return pEntry;
}

ScFormatEntry* ScFormulaFrmtEntry::GetEntry() const
{
    return createFormulaEntry();
}

OUString ScFormulaFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(FORMULA, 0, maEdFormula->GetText());
}

void ScFormulaFrmtEntry::SetActive()
{
    maWdPreview->Show();
    maFtStyle->Show();
    maLbStyle->Show();
    maEdFormula->Show();

    Select();
}

void ScFormulaFrmtEntry::SetInactive()
{
    maWdPreview->Hide();
    maFtStyle->Hide();
    maLbStyle->Hide();
    maEdFormula->Hide();

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
            {
                sal_uInt32 nIndex = 0;
                double nVal = 0;
                SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
                (void)pNumberFormatter->IsNumberFormat(rValue.GetText(), nIndex, nVal);
                pEntry->SetValue(nVal);
            }
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

ScColorScale2FrmtEntry::ScColorScale2FrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( VclPtr<ListBox>::Create( this, ScResId( LB_COLOR_FORMAT ) ) ),
    maLbEntryTypeMin( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE_COL_SCALE_MIN ) ) ),
    maLbEntryTypeMax( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE_COL_SCALE_MAX ) ) ),
    maEdMin( VclPtr<Edit>::Create( this, ScResId( ED_COL_SCALE_MIN ) ) ),
    maEdMax( VclPtr<Edit>::Create( this, ScResId( ED_COL_SCALE_MAX ) ) ),
    maLbColMin( VclPtr<ColorListBox>::Create( this, ScResId( LB_COL_MIN ) ) ),
    maLbColMax( VclPtr<ColorListBox>::Create( this, ScResId( LB_COL_MAX ) ) )
{
    // remove the automatic entry from color scales
    maLbEntryTypeMin->RemoveEntry(0);
    maLbEntryTypeMax->RemoveEntry(0);

    maLbType->SelectEntryPos(0);
    maLbColorFormat->SelectEntryPos(0);
    Init();
    if(pFormat)
    {
        ScColorScaleFormat::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr, *maLbEntryTypeMin.get(), *maEdMin.get(), *maLbColMin.get(), pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr, *maLbEntryTypeMax.get(), *maEdMax.get(), *maLbColMax.get(), pDoc);
    }
    else
    {
        maLbEntryTypeMin->SelectEntryPos(0);
        maLbEntryTypeMax->SelectEntryPos(1);
    }
    FreeResource();

    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    EntryTypeHdl(maLbEntryTypeMin.get());
    EntryTypeHdl(maLbEntryTypeMax.get());
}

ScColorScale2FrmtEntry::~ScColorScale2FrmtEntry()
{
    disposeOnce();
}

void ScColorScale2FrmtEntry::dispose()
{
    maLbColorFormat.disposeAndClear();
    maLbEntryTypeMin.disposeAndClear();
    maLbEntryTypeMax.disposeAndClear();
    maEdMin.disposeAndClear();
    maEdMax.disposeAndClear();
    maLbColMin.disposeAndClear();
    maLbColMax.disposeAndClear();
    ScCondFrmtEntry::dispose();
}

void ScColorScale2FrmtEntry::Init()
{
    maLbEntryTypeMin->SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax->SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );

    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = static_cast<const SvxColorListItem*>(pItem) ->GetColorList();
    }
    if ( pColorTable.is() )
    {
        // filling the line color box
        maLbColMin->SetUpdateMode( false );
        maLbColMax->SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            maLbColMin->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMax->InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                maLbColMin->SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                maLbColMax->SelectEntryPos(i);
        }
        maLbColMin->SetUpdateMode( true );
        maLbColMax->SetUpdateMode( true );
    }
}

ScFormatEntry* ScColorScale2FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMin.get(), *maLbColMin.get(), *maEdMin.get(), mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMax.get(), *maLbColMax.get(), *maEdMax.get(), mpDoc, maPos));
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
    maLbColorFormat->Show();

    maLbEntryTypeMin->Show();
    maLbEntryTypeMax->Show();

    maEdMin->Show();
    maEdMax->Show();

    maLbColMin->Show();
    maLbColMax->Show();

    Select();
}

void ScColorScale2FrmtEntry::SetInactive()
{
    maLbColorFormat->Hide();

    maLbEntryTypeMin->Hide();
    maLbEntryTypeMax->Hide();

    maEdMin->Hide();
    maEdMax->Hide();

    maLbColMin->Hide();
    maLbColMax->Hide();

    Deselect();
}

IMPL_LINK( ScColorScale2FrmtEntry, EntryTypeHdl, ListBox*, pBox )
{
    Edit* pEd = NULL;
    if (pBox == maLbEntryTypeMin.get())
        pEd = maEdMin;
    else if (pBox == maLbEntryTypeMax.get())
        pEd = maEdMax.get();

    if (!pEd)
        return 0;

    bool bEnableEdit = true;
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    if(nPos < 2)
    {
        bEnableEdit = false;
    }

    if (bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();

    return 0;
}

ScColorScale3FrmtEntry::ScColorScale3FrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( VclPtr<ListBox>::Create( this, ScResId( LB_COLOR_FORMAT ) ) ),
    maLbEntryTypeMin( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE_COL_SCALE_MIN ) ) ),
    maLbEntryTypeMiddle( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE_COL_SCALE_MIDDLE ) ) ),
    maLbEntryTypeMax( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE_COL_SCALE_MAX ) ) ),
    maEdMin( VclPtr<Edit>::Create( this, ScResId( ED_COL_SCALE_MIN ) ) ),
    maEdMiddle( VclPtr<Edit>::Create( this, ScResId( ED_COL_SCALE_MIDDLE ) ) ),
    maEdMax( VclPtr<Edit>::Create( this, ScResId( ED_COL_SCALE_MAX ) ) ),
    maLbColMin( VclPtr<ColorListBox>::Create( this, ScResId( LB_COL_MIN ) ) ),
    maLbColMiddle( VclPtr<ColorListBox>::Create( this, ScResId( LB_COL_MIDDLE ) ) ),
    maLbColMax( VclPtr<ColorListBox>::Create( this, ScResId( LB_COL_MAX ) ) )
{
    // remove the automatic entry from color scales
    maLbEntryTypeMin->RemoveEntry(0);
    maLbEntryTypeMiddle->RemoveEntry(0);
    maLbEntryTypeMax->RemoveEntry(0);
    maLbColorFormat->SelectEntryPos(1);

    Init();
    maLbType->SelectEntryPos(0);
    if(pFormat)
    {
        ScColorScaleFormat::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr, *maLbEntryTypeMin.get(), *maEdMin.get(), *maLbColMin.get(), pDoc);
        assert(pFormat->size() == 3);
        ++itr;
        SetColorScaleEntryTypes(*itr, *maLbEntryTypeMiddle.get(), *maEdMiddle.get(), *maLbColMiddle.get(), pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr, *maLbEntryTypeMax.get(), *maEdMax.get(), *maLbColMax.get(), pDoc);
    }
    else
    {
        maLbColorFormat->SelectEntryPos(1);
        maLbEntryTypeMin->SelectEntryPos(0);
        maLbEntryTypeMiddle->SelectEntryPos(2);
        maLbEntryTypeMax->SelectEntryPos(1);
        maEdMiddle->SetText(OUString::number(50));
    }
    FreeResource();

    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );
    EntryTypeHdl(maLbEntryTypeMin.get());
    EntryTypeHdl(maLbEntryTypeMiddle.get());
    EntryTypeHdl(maLbEntryTypeMax.get());
}

ScColorScale3FrmtEntry::~ScColorScale3FrmtEntry()
{
    disposeOnce();
}

void ScColorScale3FrmtEntry::dispose()
{
    maLbColorFormat.disposeAndClear();
    maLbEntryTypeMin.disposeAndClear();
    maLbEntryTypeMiddle.disposeAndClear();
    maLbEntryTypeMax.disposeAndClear();
    maEdMin.disposeAndClear();
    maEdMiddle.disposeAndClear();
    maEdMax.disposeAndClear();
    maLbColMin.disposeAndClear();
    maLbColMiddle.disposeAndClear();
    maLbColMax.disposeAndClear();
    ScCondFrmtEntry::dispose();
}

void ScColorScale3FrmtEntry::Init()
{
    maLbEntryTypeMin->SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax->SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMiddle->SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );

    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    XColorListRef       pColorTable;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh )
    {
        const SfxPoolItem* pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorTable = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }
    if ( pColorTable.is() )
    {
        // filling the line color box
        maLbColMin->SetUpdateMode( false );
        maLbColMiddle->SetUpdateMode( false );
        maLbColMax->SetUpdateMode( false );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            maLbColMin->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMiddle->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            maLbColMax->InsertEntry( pEntry->GetColor(), pEntry->GetName() );

            if(pEntry->GetColor() == Color(COL_LIGHTRED))
                maLbColMin->SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_GREEN))
                maLbColMiddle->SelectEntryPos(i);
            if(pEntry->GetColor() == Color(COL_LIGHTBLUE))
                maLbColMax->SelectEntryPos(i);
        }
        maLbColMin->SetUpdateMode( true );
        maLbColMiddle->SetUpdateMode( true );
        maLbColMax->SetUpdateMode( true );
    }
}

ScFormatEntry* ScColorScale3FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMin.get(), *maLbColMin.get(), *maEdMin.get(), mpDoc, maPos));
    if(maLbColorFormat->GetSelectEntryPos() == 1)
        pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMiddle.get(), *maLbColMiddle.get(), *maEdMiddle.get(), mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMax.get(), *maLbColMax.get(), *maEdMax.get(), mpDoc, maPos));
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
    maLbColorFormat->Show();
    maLbEntryTypeMin->Show();
    maLbEntryTypeMiddle->Show();
    maLbEntryTypeMax->Show();

    maEdMin->Show();
    maEdMiddle->Show();
    maEdMax->Show();

    maLbColMin->Show();
    maLbColMiddle->Show();
    maLbColMax->Show();

    Select();
}

void ScColorScale3FrmtEntry::SetInactive()
{
    maLbColorFormat->Hide();

    maLbEntryTypeMin->Hide();
    maLbEntryTypeMiddle->Hide();
    maLbEntryTypeMax->Hide();

    maEdMin->Hide();
    maEdMiddle->Hide();
    maEdMax->Hide();

    maLbColMin->Hide();
    maLbColMiddle->Hide();
    maLbColMax->Hide();

    Deselect();
}

IMPL_LINK( ScColorScale3FrmtEntry, EntryTypeHdl, ListBox*, pBox )
{
    Edit* pEd = NULL;
    if(pBox == maLbEntryTypeMin.get())
        pEd = maEdMin.get();
    else if(pBox == maLbEntryTypeMiddle.get())
        pEd = maEdMiddle.get();
    else if(pBox == maLbEntryTypeMax.get())
        pEd = maEdMax.get();

    if (!pEd)
        return 0;

    bool bEnableEdit = true;
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    if(nPos < 2)
    {
        bEnableEdit = false;
    }

    if(bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();

    return 0;
}

IMPL_LINK_NOARG( ScConditionFrmtEntry, ConditionTypeSelectHdl )
{
    sal_Int32 nSelectPos = maLbCondType->GetSelectEntryPos();
    ScConditionMode eMode = EntryPosToConditionMode(nSelectPos);
    switch(GetNumberEditFields(eMode))
    {
        case 0:
            maEdVal1->Hide();
            maEdVal2->Hide();
            maFtVal->Hide();
            break;
        case 1:
            maEdVal1->Show();
            maEdVal2->Hide();
            maFtVal->Show();
            break;
        case 2:
            maEdVal1->Show();
            maEdVal2->Show();
            maFtVal->Show();
            break;
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

ScDataBarFrmtEntry::ScDataBarFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, rPos ),
    maLbColorFormat( VclPtr<ListBox>::Create( this, ScResId( LB_COLOR_FORMAT ) ) ),
    maLbDataBarMinType( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE_COL_SCALE_MIN ) ) ),
    maLbDataBarMaxType( VclPtr<ListBox>::Create( this, ScResId( LB_TYPE_COL_SCALE_MAX ) ) ),
    maEdDataBarMin( VclPtr<Edit>::Create( this, ScResId( ED_COL_SCALE_MIN ) ) ),
    maEdDataBarMax( VclPtr<Edit>::Create( this, ScResId( ED_COL_SCALE_MAX ) ) ),
    maBtOptions( VclPtr<PushButton>::Create( this, ScResId( BTN_OPTIONS ) ) )
{
    maLbColorFormat->SelectEntryPos(2);
    maLbType->SelectEntryPos(0);
    if(pFormat)
    {
        mpDataBarData.reset(new ScDataBarFormatData(*pFormat->GetDataBarData()));
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, *maLbDataBarMinType.get(), *maEdDataBarMin.get(), pDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, *maLbDataBarMaxType.get(), *maEdDataBarMax.get(), pDoc);
        DataBarTypeSelectHdl(NULL);
    }
    else
    {
        maLbDataBarMinType->SelectEntryPos(0);
        maLbDataBarMaxType->SelectEntryPos(0);
        DataBarTypeSelectHdl(NULL);
    }
    Init();

    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    FreeResource();
}

ScDataBarFrmtEntry::~ScDataBarFrmtEntry()
{
    disposeOnce();
}

void ScDataBarFrmtEntry::dispose()
{
    maLbColorFormat.disposeAndClear();
    maLbDataBarMinType.disposeAndClear();
    maLbDataBarMaxType.disposeAndClear();
    maEdDataBarMin.disposeAndClear();
    maEdDataBarMax.disposeAndClear();
    maBtOptions.disposeAndClear();
    ScCondFrmtEntry::dispose();
}

ScFormatEntry* ScDataBarFrmtEntry::GetEntry() const
{
    return createDatabarEntry();
}

void ScDataBarFrmtEntry::Init()
{
    maLbDataBarMinType->SetSelectHdl( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );
    maLbDataBarMaxType->SetSelectHdl( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );

    maBtOptions->SetClickHdl( LINK( this, ScDataBarFrmtEntry, OptionBtnHdl ) );

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
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), *maLbDataBarMinType.get(), *maEdDataBarMin.get(), mpDoc, maPos, true);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), *maLbDataBarMaxType.get(), *maEdDataBarMax.get(), mpDoc, maPos, true);
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
    maLbColorFormat->Show();

    maLbDataBarMinType->Show();
    maLbDataBarMaxType->Show();
    maEdDataBarMin->Show();
    maEdDataBarMax->Show();
    maBtOptions->Show();

    Select();
}

void ScDataBarFrmtEntry::SetInactive()
{
    maLbColorFormat->Hide();

    maLbDataBarMinType->Hide();
    maLbDataBarMaxType->Hide();
    maEdDataBarMin->Hide();
    maEdDataBarMax->Hide();
    maBtOptions->Hide();

    Deselect();
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, DataBarTypeSelectHdl )
{
    sal_Int32 nSelectPos = maLbDataBarMinType->GetSelectEntryPos();
    if(nSelectPos <= COLORSCALE_MAX)
        maEdDataBarMin->Disable();
    else
        maEdDataBarMin->Enable();

    nSelectPos = maLbDataBarMaxType->GetSelectEntryPos();
    if(nSelectPos <= COLORSCALE_MAX)
        maEdDataBarMax->Disable();
    else
        maEdDataBarMax->Enable();

    return 0;
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, OptionBtnHdl )
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), *maLbDataBarMinType.get(), *maEdDataBarMin.get(), mpDoc, maPos, true);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), *maLbDataBarMaxType.get(), *maEdDataBarMax.get(), mpDoc, maPos, true);
    ScopedVclPtrInstance<ScDataBarSettingsDlg> pDlg(this, *mpDataBarData, mpDoc, maPos);
    if( pDlg->Execute() == RET_OK)
    {
        mpDataBarData.reset(pDlg->GetData());
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, *maLbDataBarMinType, *maEdDataBarMin.get(), mpDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, *maLbDataBarMaxType.get(), *maEdDataBarMax.get(), mpDoc);
        DataBarTypeSelectHdl(NULL);
    }
    return 0;
}

ScDateFrmtEntry::ScDateFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScCondDateFormatEntry* pFormat ):
    ScCondFrmtEntry( pParent, pDoc, ScAddress() ),
    maLbDateEntry( VclPtr<ListBox>::Create( this, ScResId( LB_DATE_TYPE ) ) ),
    maFtStyle( VclPtr<FixedText>::Create( this, ScResId( FT_STYLE ) ) ),
    maLbStyle( VclPtr<ListBox>::Create( this, ScResId( LB_STYLE ) ) ),
    maWdPreview( VclPtr<SvxFontPrevWindow>::Create( this, ScResId( WD_PREVIEW ) ) ),
    mbIsInStyleCreate(false)
{
    Init();
    FreeResource();

    StartListening(*pDoc->GetStyleSheetPool(), true);

    if(pFormat)
    {
        sal_Int32 nPos = static_cast<sal_Int32>(pFormat->GetDateType());
        maLbDateEntry->SelectEntryPos(nPos);

        OUString aStyleName = pFormat->GetStyleName();
        maLbStyle->SelectEntry(aStyleName);
    }

    StyleSelectHdl(NULL);
}

ScDateFrmtEntry::~ScDateFrmtEntry()
{
    disposeOnce();
}

void ScDateFrmtEntry::dispose()
{
    maLbDateEntry.disposeAndClear();
    maFtStyle.disposeAndClear();
    maLbStyle.disposeAndClear();
    maWdPreview.disposeAndClear();
    ScCondFrmtEntry::dispose();
}

void ScDateFrmtEntry::Init()
{
    maLbDateEntry->SelectEntryPos(0);
    maLbType->SelectEntryPos(3);

    FillStyleListBox( mpDoc, *maLbStyle.get() );
    maLbStyle->SetSelectHdl( LINK( this, ScDateFrmtEntry, StyleSelectHdl ) );
    maLbStyle->SelectEntryPos(1);
}

void ScDateFrmtEntry::SetActive()
{
    maLbDateEntry->Show();
    maFtStyle->Show();
    maWdPreview->Show();
    maLbStyle->Show();

    Select();
}

void ScDateFrmtEntry::SetInactive()
{
    maLbDateEntry->Hide();
    maFtStyle->Hide();
    maWdPreview->Hide();
    maLbStyle->Hide();

    Deselect();
}

void ScDateFrmtEntry::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxStyleSheetHint* pHint = dynamic_cast<const SfxStyleSheetHint*>(&rHint);
    if(!pHint)
        return;

    sal_uInt16 nHint = pHint->GetHint();
    if(nHint == SfxStyleSheetHintId::MODIFIED)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(*maLbStyle.get(), mpDoc);
    }
}

ScFormatEntry* ScDateFrmtEntry::GetEntry() const
{
    ScCondDateFormatEntry* pNewEntry = new ScCondDateFormatEntry(mpDoc);
    condformat::ScCondFormatDateType eType = static_cast<condformat::ScCondFormatDateType>(maLbDateEntry->GetSelectEntryPos());
    pNewEntry->SetDateType(eType);
    pNewEntry->SetStyleName(maLbStyle->GetSelectEntry());
    return pNewEntry;
}

OUString ScDateFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(DATE, 0);
}

IMPL_LINK_NOARG( ScDateFrmtEntry, StyleSelectHdl )
{
    mbIsInStyleCreate = true;
    StyleSelect( *maLbStyle.get(), mpDoc, *maWdPreview.get() );
    mbIsInStyleCreate = false;

    return 0;
}

class ScIconSetFrmtDataEntry : public Control
{
    private:
        VclPtr<FixedImage> maImgIcon;
        VclPtr<FixedText> maFtEntry;
        VclPtr<Edit> maEdEntry;
        VclPtr<ListBox> maLbEntryType;

    public:
        ScIconSetFrmtDataEntry( vcl::Window* pParent, ScIconSetType eType, ScDocument* pDoc,
                sal_Int32 i, const ScColorScaleEntry* pEntry = NULL );
        virtual ~ScIconSetFrmtDataEntry();
        virtual void dispose() SAL_OVERRIDE;

        ScColorScaleEntry* CreateEntry(ScDocument* pDoc, const ScAddress& rPos) const;

        void SetFirstEntry();
};

ScIconSetFrmtDataEntry::ScIconSetFrmtDataEntry( vcl::Window* pParent, ScIconSetType eType, ScDocument* pDoc, sal_Int32 i, const ScColorScaleEntry* pEntry ):
    Control( pParent, ScResId( RID_ICON_SET_ENTRY ) ),
    maImgIcon( VclPtr<FixedImage>::Create( this, ScResId( IMG_ICON ) ) ),
    maFtEntry( VclPtr<FixedText>::Create( this, ScResId( FT_ICON_SET_ENTRY_TEXT ) ) ),
    maEdEntry( VclPtr<Edit>::Create( this, ScResId( ED_ICON_SET_ENTRY_VALUE ) ) ),
    maLbEntryType( VclPtr<ListBox>::Create( this, ScResId( LB_ICON_SET_ENTRY_TYPE ) ) )
{
    maImgIcon->SetImage(Image(ScIconSetFormat::getBitmap(eType, i)));
    if(pEntry)
    {
        switch(pEntry->GetType())
        {
            case COLORSCALE_VALUE:
                maLbEntryType->SelectEntryPos(0);
                maEdEntry->SetText(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_PERCENTILE:
                maLbEntryType->SelectEntryPos(2);
                maEdEntry->SetText(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_PERCENT:
                maLbEntryType->SelectEntryPos(1);
                maEdEntry->SetText(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_FORMULA:
                maLbEntryType->SelectEntryPos(3);
                maEdEntry->SetText(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
                break;
            default:
                assert(false);
        }
    }
    else
    {
        maLbEntryType->SelectEntryPos(1);
    }
    FreeResource();
}

ScIconSetFrmtDataEntry::~ScIconSetFrmtDataEntry()
{
    disposeOnce();
}

void ScIconSetFrmtDataEntry::dispose()
{
    maImgIcon.disposeAndClear();
    maFtEntry.disposeAndClear();
    maEdEntry.disposeAndClear();
    maLbEntryType.disposeAndClear();
    Control::dispose();
}

ScColorScaleEntry* ScIconSetFrmtDataEntry::CreateEntry(ScDocument* pDoc, const ScAddress& rPos) const
{
    sal_Int32 nPos = maLbEntryType->GetSelectEntryPos();
    OUString aText = maEdEntry->GetText();
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    sal_uInt32 nIndex = 0;
    double nVal = 0;
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    (void)pNumberFormatter->IsNumberFormat(aText, nIndex, nVal);
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
    maEdEntry->Hide();
    maLbEntryType->Hide();
    maFtEntry->Hide();
    maEdEntry->SetText(OUString("0"));
    maLbEntryType->SelectEntryPos(1);
}

ScIconSetFrmtEntry::ScIconSetFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScIconSetFormat* pFormat ):
        ScCondFrmtEntry( pParent, pDoc, rPos ),
        maLbColorFormat( VclPtr<ListBox>::Create( this, ScResId( LB_COLOR_FORMAT ) ) ),
        maLbIconSetType( VclPtr<ListBox>::Create( this, ScResId( LB_ICONSET_TYPE ) ) )
{
    Init();
    FreeResource();
    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    if(pFormat)
    {
        const ScIconSetFormatData* pIconSetFormatData = pFormat->GetIconSetData();
        ScIconSetType eType = pIconSetFormatData->eIconSetType;
        sal_Int32 nType = static_cast<sal_Int32>(eType);
        maLbIconSetType->SelectEntryPos(nType);

        for(size_t i = 0, n = pIconSetFormatData->maEntries.size();
                i < n; ++i)
        {
            maEntries.push_back( VclPtr<ScIconSetFrmtDataEntry>::Create( this, eType, pDoc, i, &pIconSetFormatData->maEntries[i] ) );
            Point aPos = maEntries[0]->GetPosPixel();
            aPos.Y() += maEntries[0]->GetSizePixel().Height() * i * 1.2;
            maEntries[i]->SetPosPixel( aPos );
        }
        maEntries[0]->SetFirstEntry();
    }
    else
        IconSetTypeHdl(NULL);
}

ScIconSetFrmtEntry::~ScIconSetFrmtEntry()
{
    disposeOnce();
}

void ScIconSetFrmtEntry::dispose()
{
    for (auto it = maEntries.begin(); it != maEntries.end(); ++it)
        it->disposeAndClear();
    maEntries.clear();
    maLbColorFormat.disposeAndClear();
    maLbIconSetType.disposeAndClear();
    ScCondFrmtEntry::dispose();
}

void ScIconSetFrmtEntry::Init()
{
    maLbColorFormat->SelectEntryPos(3);
    maLbType->SelectEntryPos(0);
    maLbIconSetType->SelectEntryPos(0);

    maLbIconSetType->SetSelectHdl( LINK( this, ScIconSetFrmtEntry, IconSetTypeHdl ) );
}

IMPL_LINK_NOARG( ScIconSetFrmtEntry, IconSetTypeHdl )
{
    ScIconSetMap* pMap = ScIconSetFormat::getIconSetMap();

    sal_Int32 nPos = maLbIconSetType->GetSelectEntryPos();
    sal_uInt32 nElements = pMap[nPos].nElements;

    for (auto it = maEntries.begin(); it != maEntries.end(); ++it)
        it->disposeAndClear();
    maEntries.clear();

    for(size_t i = 0; i < nElements; ++i)
    {
        maEntries.push_back( VclPtr<ScIconSetFrmtDataEntry>::Create( this, static_cast<ScIconSetType>(nPos), mpDoc, i ) );
        Point aPos = maEntries[0]->GetPosPixel();
        aPos.Y() += maEntries[0]->GetSizePixel().Height() * i * 1.2;
        maEntries[i]->SetPosPixel( aPos );
        maEntries[i]->Show();
    }
    maEntries[0]->SetFirstEntry();

    SetHeight();

    return 0;
}

OUString ScIconSetFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(ICONSET, 0);
}

void ScIconSetFrmtEntry::SetActive()
{
    maLbColorFormat->Show();
    maLbIconSetType->Show();
    for(ScIconSetFrmtDataEntriesType::iterator itr = maEntries.begin(),
            itrEnd = maEntries.end(); itr != itrEnd; ++itr)
    {
        (*itr)->Show();
    }

    Select();
}

void ScIconSetFrmtEntry::SetInactive()
{
    maLbColorFormat->Hide();
    maLbIconSetType->Hide();
    for(ScIconSetFrmtDataEntriesType::iterator itr = maEntries.begin(),
            itrEnd = maEntries.end(); itr != itrEnd; ++itr)
    {
        (*itr)->Hide();
    }

    Deselect();
}

ScFormatEntry* ScIconSetFrmtEntry::GetEntry() const
{
    ScIconSetFormat* pFormat = new ScIconSetFormat(mpDoc);

    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->eIconSetType = static_cast<ScIconSetType>(maLbIconSetType->GetSelectEntryPos());
    for(ScIconSetFrmtDataEntriesType::const_iterator itr = maEntries.begin(),
            itrEnd = maEntries.end(); itr != itrEnd; ++itr)
    {
        pData->maEntries.push_back((*itr)->CreateEntry(mpDoc, maPos));
    }
    pFormat->SetIconSetData(pData);

    return pFormat;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
