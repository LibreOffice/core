/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <condformatdlg.hxx>
#include <condformatdlgentry.hxx>
#include <conditio.hxx>
#include <compiler.hxx>
#include <colorscale.hxx>
#include <condformathelper.hxx>

#include <document.hxx>

#include <svl/style.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/colorbox.hxx>
#include <vcl/settings.hxx>
#include <formula/token.hxx>
#include <formula/errorcodes.hxx>
#include <tokenarray.hxx>
#include <stlpool.hxx>
#include <tabvwsh.hxx>
#include <unotools/charclass.hxx>

#include <colorformat.hxx>
#include <scresid.hxx>
#include <globstr.hrc>
#include <strings.hrc>

#include <set>

ScCondFrmtEntry::ScCondFrmtEntry(vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos)
    : VclContainer(pParent, WB_CLIPCHILDREN | WB_BORDER | WB_CHILDDLGCTRL)
    , mbActive(false)
    , maStrCondition(ScResId(SCSTR_CONDITION))
    , mpDoc(pDoc)
    , maPos(rPos)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "modules/scalc/ui/conditionalentry.ui"));

    get(maGrid, "grid");
    get(maFtCondNr, "number");
    get(maFtCondition, "condition");
    get(maLbType, "type");

    Color aBack(GetSettings().GetStyleSettings().GetWindowColor());

    SetControlBackground(aBack);
    SetBackground(GetControlBackground());

    maFtCondNr->SetControlBackground(aBack);
    maFtCondNr->SetBackground(maFtCondNr->GetControlBackground());

    maFtCondition->SetControlBackground(aBack);
    maFtCondition->SetBackground(maFtCondition->GetControlBackground());

    maLbType->SetSelectHdl( LINK( pParent, ScCondFormatList, TypeListHdl ) );
    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );

    Show();
}

ScCondFrmtEntry::~ScCondFrmtEntry()
{
    disposeOnce();
}

Size ScCondFrmtEntry::calculateRequisition() const
{
    return getLayoutRequisition(*maGrid);
}

void ScCondFrmtEntry::setAllocation(const Size &rAllocation)
{
    setLayoutPosSize(*maGrid, Point(0, 0), rAllocation);
}

void ScCondFrmtEntry::dispose()
{
    maFtCondNr.clear();
    maFtCondition.clear();
    maLbType.clear();
    maGrid.clear();
    disposeBuilder();
    VclContainer::dispose();
}

bool ScCondFrmtEntry::EventNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN)
    {
        maClickHdl.Call(*this);
    }
    return VclContainer::EventNotify(rNEvt);
}

void ScCondFrmtEntry::SetIndex(sal_Int32 nIndex)
{
    maFtCondNr->SetText(maStrCondition + OUString::number(nIndex));
}

void ScCondFrmtEntry::SetHeight()
{
    const long nMaxHeight = get_preferred_size().Height();
    if (nMaxHeight > 0)
    {
        Size aSize = GetSizePixel();
        const long nPad = LogicToPixel(Size(0, 2), MapMode(MapUnit::MapAppFont)).getHeight();
        aSize.setHeight( nMaxHeight + nPad );
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
    OUString aCondText = GetExpressionString();
    maFtCondition->SetText(aCondText);
    maFtCondition->Show();
    maLbType->Hide();
    mbActive = false;
    SetHeight();
}

//condition

namespace {

void FillStyleListBox( const ScDocument* pDoc, ListBox& rLbStyle )
{
    rLbStyle.SetSeparatorPos(0);
    std::set<OUString> aStyleNames;
    SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SfxStyleFamily::Para );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        aStyleNames.insert(pStyle->GetName());
    }
    for(const auto& rStyleName : aStyleNames)
    {
        rLbStyle.InsertEntry( rStyleName );
    }
}

}

const ScConditionMode ScConditionFrmtEntry::mpEntryToCond[ScConditionFrmtEntry::NUM_COND_ENTRIES] = {
    ScConditionMode::Equal,
    ScConditionMode::Less,
    ScConditionMode::Greater,
    ScConditionMode::EqLess,
    ScConditionMode::EqGreater,
    ScConditionMode::NotEqual,
    ScConditionMode::Between,
    ScConditionMode::NotBetween,
    ScConditionMode::Duplicate,
    ScConditionMode::NotDuplicate,
    ScConditionMode::Top10,
    ScConditionMode::Bottom10,
    ScConditionMode::TopPercent,
    ScConditionMode::BottomPercent,
    ScConditionMode::AboveAverage,
    ScConditionMode::BelowAverage,
    ScConditionMode::AboveEqualAverage,
    ScConditionMode::BelowEqualAverage,
    ScConditionMode::Error,
    ScConditionMode::NoError,
    ScConditionMode::BeginsWith,
    ScConditionMode::EndsWith,
    ScConditionMode::ContainsText,
    ScConditionMode::NotContainsText
};

ScConditionFrmtEntry::ScConditionFrmtEntry(vcl::Window* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent,
        const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
    , mbIsInStyleCreate(false)
{
    get(maLbCondType, "typeis");
    get(maEdVal1, "val1");
    get(maEdVal2, "val2");
    get(maFtVal, "valueft");
    get(maFtStyle, "styleft");
    get(maLbStyle, "style");
    get(maWdPreview, "preview");
    maWdPreview->set_height_request(maLbStyle->get_preferred_size().Height());

    maLbType->SelectEntryPos(1);

    Init(pDialogParent);

    StartListening(*pDoc->GetStyleSheetPool(), DuplicateHandling::Prevent);

    if(pFormatEntry)
    {
        maLbStyle->SelectEntry(pFormatEntry->GetStyle());
        StyleSelectHdl(*maLbStyle);
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
                OnEdChanged(*maEdVal1);
                break;
            case 2:
                maEdVal1->Show();
                maEdVal1->SetText(pFormatEntry->GetExpression(maPos, 0));
                OnEdChanged(*maEdVal1);
                maEdVal2->Show();
                maEdVal2->SetText(pFormatEntry->GetExpression(maPos, 1));
                OnEdChanged(*maEdVal2);
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
    maLbCondType.clear();
    maEdVal1.clear();
    maEdVal2.clear();
    maFtVal.clear();
    maFtStyle.clear();
    maLbStyle.clear();
    maWdPreview.clear();
    ScCondFrmtEntry::dispose();
}

void ScConditionFrmtEntry::Init(ScCondFormatDlg* pDialogParent)
{
    maEdVal1->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );
    maEdVal2->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );

    maEdVal1->SetForceControlBackground(true);
    maEdVal2->SetForceControlBackground(true);

    maEdVal1->SetModifyHdl( LINK( this, ScConditionFrmtEntry, OnEdChanged ) );
    maEdVal2->SetModifyHdl( LINK( this, ScConditionFrmtEntry, OnEdChanged ) );

    FillStyleListBox( mpDoc, *maLbStyle );
    maLbStyle->SetSelectHdl( LINK( this, ScConditionFrmtEntry, StyleSelectHdl ) );

    maLbCondType->SetSelectHdl( LINK( this, ScConditionFrmtEntry, ConditionTypeSelectHdl ) );
}

ScFormatEntry* ScConditionFrmtEntry::createConditionEntry() const
{
    ScConditionMode eMode = EntryPosToConditionMode(maLbCondType->GetSelectedEntryPos());
    OUString aExpr1 = maEdVal1->GetText();
    OUString aExpr2;
    if (GetNumberEditFields(eMode) == 2)
    {
        aExpr2 = maEdVal2->GetText();
        if (aExpr2.isEmpty())
        {
            return nullptr;
        }
    }

    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, mpDoc, maPos, maLbStyle->GetSelectedEntry());
    return pEntry;
}

IMPL_LINK(ScConditionFrmtEntry, OnEdChanged, Edit&, rEdit, void)
{
    OUString aFormula = rEdit.GetText();

    if( aFormula.isEmpty() )
    {
        maFtVal->SetText(ScResId(STR_ENTER_VALUE));
        return;
    }

    ScCompiler aComp( mpDoc, maPos, mpDoc->GetGrammar() );
    std::unique_ptr<ScTokenArray> ta(aComp.CompileString(aFormula));

    // Error, warn the user
    if( ta->GetCodeError() != FormulaError::NONE || ( ta->GetLen() == 0 ) )
    {
        rEdit.SetControlBackground(COL_LIGHTRED);
        maFtVal->SetText(ScResId(STR_VALID_DEFERROR));
        return;
    }

    // Recognized col/row name or string token, warn the user
    formula::FormulaToken* token = ta->FirstToken();
    formula::StackVar t = token->GetType();
    OpCode op = token->GetOpCode();
    if( ( op == ocColRowName ) ||
        ( ( op == ocBad ) && ( t == formula::svString ) )
      )
    {
        rEdit.SetControlBackground(COL_YELLOW);
        maFtVal->SetText(ScResId(STR_UNQUOTED_STRING));
        return;
    }

    rEdit.SetControlBackground(GetSettings().GetStyleSettings().GetWindowColor());
    maFtVal->SetText("");
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
        case ScConditionMode::Equal:
        case ScConditionMode::Less:
        case ScConditionMode::Greater:
        case ScConditionMode::EqLess:
        case ScConditionMode::EqGreater:
        case ScConditionMode::NotEqual:
        case ScConditionMode::Top10:
        case ScConditionMode::Bottom10:
        case ScConditionMode::TopPercent:
        case ScConditionMode::BottomPercent:
        case ScConditionMode::BeginsWith:
        case ScConditionMode::EndsWith:
        case ScConditionMode::ContainsText:
        case ScConditionMode::NotContainsText:
        case ScConditionMode::Error:
        case ScConditionMode::NoError:
            return 1;
        case ScConditionMode::AboveAverage:
        case ScConditionMode::BelowAverage:
        case ScConditionMode::AboveEqualAverage:
        case ScConditionMode::BelowEqualAverage:
        case ScConditionMode::Duplicate:
        case ScConditionMode::NotDuplicate:
            return 0;
        case ScConditionMode::Between:
        case ScConditionMode::NotBetween:
            return 2;
        default:
            assert(false); // should never get here
            return 0;
    }
}

OUString ScConditionFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(CONDITION, maLbCondType->GetSelectedEntryPos(), maEdVal1->GetText(), maEdVal2->GetText());
}

ScFormatEntry* ScConditionFrmtEntry::GetEntry() const
{
    return createConditionEntry();
}

void ScConditionFrmtEntry::SetActive()
{
    ScConditionMode eMode = EntryPosToConditionMode(maLbCondType->GetSelectedEntryPos());
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

void UpdateStyleList(ListBox& rLbStyle, const ScDocument* pDoc)
{
    OUString aSelectedStyle = rLbStyle.GetSelectedEntry();
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
    if(rHint.GetId() == SfxHintId::StyleSheetModified)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(*maLbStyle, mpDoc);
    }
}

namespace {

void StyleSelect( ListBox& rLbStyle, const ScDocument* pDoc, SvxFontPrevWindow& rWdPreview )
{
    if(rLbStyle.GetSelectedEntryPos() == 0)
    {
        // call new style dialog
        SfxUInt16Item aFamilyItem( SID_STYLE_FAMILY, sal_uInt16(SfxStyleFamily::Para) );
        SfxStringItem aRefItem( SID_STYLE_REFERENCE, ScResId(STR_STYLENAME_STANDARD) );

        // unlock the dispatcher so SID_STYLE_NEW can be executed
        // (SetDispatcherLock would affect all Calc documents)
        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        SfxDispatcher* pDisp = pViewShell->GetDispatcher();
        bool bLocked = pDisp->IsLocked();
        if (bLocked)
            pDisp->Lock(false);

        // Execute the "new style" slot, complete with undo and all necessary updates.
        // The return value (SfxUInt16Item) is ignored, look for new styles instead.
        pDisp->ExecuteList(SID_STYLE_NEW,
            SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
            { &aFamilyItem, &aRefItem });

        if (bLocked)
            pDisp->Lock(true);

        // Find the new style and add it into the style list boxes
        SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SfxStyleFamily::Para );
        bool bFound = false;
        for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle && !bFound; pStyle = aStyleIter.Next() )
        {
            const OUString& aName = pStyle->GetName();
            if ( rLbStyle.GetEntryPos(aName) == LISTBOX_ENTRY_NOTFOUND )    // all lists contain the same entries
            {
                for( sal_Int32 i = 1, n = rLbStyle.GetEntryCount(); i <= n && !bFound; ++i)
                {
                    OUString aStyleName = ScGlobal::pCharClass->uppercase(rLbStyle.GetEntry(i));
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

    OUString aStyleName = rLbStyle.GetSelectedEntry();
    SfxStyleSheetBase* pStyleSheet = pDoc->GetStyleSheetPool()->Find( aStyleName, SfxStyleFamily::Para );
    if(pStyleSheet)
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        rWdPreview.Init( rSet );
    }
}

}

IMPL_LINK_NOARG(ScConditionFrmtEntry, StyleSelectHdl, ListBox&, void)
{
    mbIsInStyleCreate = true;
    StyleSelect( *maLbStyle, mpDoc, *maWdPreview );
    mbIsInStyleCreate = false;
}

// formula

ScFormulaFrmtEntry::ScFormulaFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent, const ScAddress& rPos, const ScCondFormatEntry* pFormat )
    : ScCondFrmtEntry(pParent, pDoc, rPos)
{
    get(maFtStyle, "styleft");
    get(maLbStyle, "style");
    get(maWdPreview, "preview");
    maWdPreview->set_height_request(maLbStyle->get_preferred_size().Height());
    get(maEdFormula, "formula");

    Init(pDialogParent);

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

    StyleSelectHdl(*maLbStyle);
}

ScFormulaFrmtEntry::~ScFormulaFrmtEntry()
{
    disposeOnce();
}

void ScFormulaFrmtEntry::dispose()
{
    maFtStyle.clear();
    maLbStyle.clear();
    maWdPreview.clear();
    maEdFormula.clear();
    ScCondFrmtEntry::dispose();
}

void ScFormulaFrmtEntry::Init(ScCondFormatDlg* pDialogParent)
{
    maEdFormula->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );

    FillStyleListBox( mpDoc, *maLbStyle );
    maLbStyle->SetSelectHdl( LINK( this, ScFormulaFrmtEntry, StyleSelectHdl ) );
}

IMPL_LINK_NOARG(ScFormulaFrmtEntry, StyleSelectHdl, ListBox&, void)
{
    StyleSelect( *maLbStyle, mpDoc, *maWdPreview );
}

ScFormatEntry* ScFormulaFrmtEntry::createFormulaEntry() const
{
    OUString aFormula = maEdFormula->GetText();
    if(aFormula.isEmpty())
        return nullptr;

    ScFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, aFormula, OUString(), mpDoc, maPos, maLbStyle->GetSelectedEntry());
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

OUString convertNumberToString(double nVal, const ScDocument* pDoc)
{
    SvNumberFormatter* pNumberFormatter = pDoc->GetFormatTable();
    OUString aText;
    pNumberFormatter->GetInputLineString(nVal, 0, aText);
    return aText;
}

const struct
{
    ScColorScaleEntryType eType;
    const char* sId;
} TypeIdMap[] = {
    { COLORSCALE_AUTO,       "auto" },
    { COLORSCALE_MIN,        "min" },
    { COLORSCALE_MAX,        "max" },
    { COLORSCALE_PERCENTILE, "percentil" },
    { COLORSCALE_VALUE,      "value" },
    { COLORSCALE_PERCENT,    "percent" },
    { COLORSCALE_FORMULA,    "formula" },
};

ScColorScaleEntryType getTypeForId(const OUString& sId)
{
    for (auto& r : TypeIdMap)
    {
        if (sId.equalsAscii(r.sId))
            return r.eType;
    }
    assert(false); // The id is not in TypeIdMap - something not in sync?
    return COLORSCALE_AUTO; // invalid id - use default
}

// Item ids are imported from .ui into OUString* and are referenced by entry data.
// See commit 83cefb5ceb4428d61a5b9fae80d1e673131e9bfe

ScColorScaleEntryType getSelectedType(const ListBox& rListBox)
{
    const OUString* sId = static_cast<OUString*>(rListBox.GetSelectedEntryData());
    assert(sId
           && "The color scale type entries must have item ids in "
              "sc/uiconfig/scalc/ui/conditionalentry.ui");
    return getTypeForId(*sId);
}

sal_Int32 getEntryPos(const ListBox& rListBox, ScColorScaleEntryType eType)
{
    const sal_Int32 nSize = rListBox.GetEntryCount();
    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        const OUString* sId = static_cast<OUString*>(rListBox.GetEntryData(i));
        assert(sId
               && "The color scale type entries must have item ids in "
                  "sc/uiconfig/scalc/ui/conditionalentry.ui");
        if (getTypeForId(*sId) == eType)
            return i;
    }
    return -1;
}

void selectType(ListBox& rListBox, ScColorScaleEntryType eType)
{
    const sal_Int32 nPos = getEntryPos(rListBox, eType);
    if (nPos >= 0)
        rListBox.SelectEntryPos(nPos);
}

void removeType(ListBox& rListBox, ScColorScaleEntryType eType)
{
    const sal_Int32 nPos = getEntryPos(rListBox, eType);
    if (nPos >= 0)
        rListBox.RemoveEntry(nPos);
}

void SetColorScaleEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit, SvxColorListBox& rLbCol, const ScDocument* pDoc )
{
    // entry Automatic is not available for color scales
    assert(rEntry.GetType() > COLORSCALE_AUTO);
    selectType(rLbType, rEntry.GetType());
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

void SetColorScaleEntry(ScColorScaleEntry* pEntry, const ListBox& rType, const Edit& rValue,
                        ScDocument* pDoc, const ScAddress& rPos)
{
    ScColorScaleEntryType eType = getSelectedType(rType);

    pEntry->SetType(eType);
    switch (eType)
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

ScColorScaleEntry* createColorScaleEntry( const ListBox& rType, const SvxColorListBox& rColor, const Edit& rValue, ScDocument* pDoc, const ScAddress& rPos )
{
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    SetColorScaleEntry(pEntry, rType, rValue, pDoc, rPos);
    Color aColor = rColor.GetSelectEntryColor();
    pEntry->SetColor(aColor);
    return pEntry;
}

}

ScColorScale2FrmtEntry::ScColorScale2FrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat )
    : ScCondFrmtEntry(pParent, pDoc, rPos)
{
    get(maLbColorFormat, "colorformat");
    get(maLbEntryTypeMin, "colscalemin");
    get(maLbEntryTypeMax, "colscalemax");
    get(maEdMin, "edcolscalemin");
    get(maEdMax, "edcolscalemax");
    get(maLbColMin, "lbcolmin");
    get(maLbColMax, "lbcolmax");
    get(maFtMin, "Label_minimum");
    get(maFtMax, "Label_maximum");

    maFtMin->Show();
    maFtMax->Show();

    // remove the automatic entry from color scales
    removeType(*maLbEntryTypeMin, COLORSCALE_AUTO);
    removeType(*maLbEntryTypeMax, COLORSCALE_AUTO);
    // "min" selector doesn't need "max" entry, and vice versa
    removeType(*maLbEntryTypeMin, COLORSCALE_MAX);
    removeType(*maLbEntryTypeMax, COLORSCALE_MIN);

    maLbType->SelectEntryPos(0);
    maLbColorFormat->SelectEntryPos(0);
    Init();
    if(pFormat)
    {
        ScColorScaleEntries::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr[0], *maLbEntryTypeMin, *maEdMin, *maLbColMin, pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr[0], *maLbEntryTypeMax, *maEdMax, *maLbColMax, pDoc);
    }
    else
    {
        selectType(*maLbEntryTypeMin, COLORSCALE_MIN);
        selectType(*maLbEntryTypeMax, COLORSCALE_MAX);
    }

    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    EntryTypeHdl(*maLbEntryTypeMin);
    EntryTypeHdl(*maLbEntryTypeMax);
}

ScColorScale2FrmtEntry::~ScColorScale2FrmtEntry()
{
    disposeOnce();
}

void ScColorScale2FrmtEntry::dispose()
{
    maLbColorFormat.clear();
    maLbEntryTypeMin.clear();
    maLbEntryTypeMax.clear();
    maEdMin.clear();
    maEdMax.clear();
    maLbColMin.clear();
    maLbColMax.clear();
    maFtMin.clear();
    maFtMax.clear();
    ScCondFrmtEntry::dispose();
}

void ScColorScale2FrmtEntry::Init()
{
    maLbEntryTypeMin->SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax->SetSelectHdl( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );
    maLbColMin->SelectEntry(Color(0xFFF685)); // Light Yellow 2
    maLbColMax->SelectEntry(Color(0x65C295)); // Light Green 2
}

ScFormatEntry* ScColorScale2FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMin, *maLbColMin, *maEdMin, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMax, *maLbColMax, *maEdMax, mpDoc, maPos));
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

IMPL_LINK( ScColorScale2FrmtEntry, EntryTypeHdl, ListBox&, rBox, void )
{
    Edit* pEd = nullptr;
    if (&rBox == maLbEntryTypeMin.get())
        pEd = maEdMin;
    else if (&rBox == maLbEntryTypeMax.get())
        pEd = maEdMax.get();

    if (!pEd)
        return;

    bool bEnableEdit = true;
    if (getSelectedType(rBox) <= COLORSCALE_MAX)
    {
        bEnableEdit = false;
    }

    if (bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();
}

ScColorScale3FrmtEntry::ScColorScale3FrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat )
    : ScCondFrmtEntry(pParent, pDoc, rPos)
{
    get(maLbColorFormat, "colorformat");
    get(maLbEntryTypeMin, "colscalemin");
    get(maLbEntryTypeMiddle, "colscalemiddle");
    get(maLbEntryTypeMax, "colscalemax");
    get(maEdMin, "edcolscalemin");
    get(maEdMiddle, "edcolscalemiddle");
    get(maEdMax, "edcolscalemax");
    get(maLbColMin, "lbcolmin");
    get(maLbColMiddle, "lbcolmiddle");
    get(maLbColMax, "lbcolmax");
    get(maFtMin, "Label_minimum");
    get(maFtMax, "Label_maximum");

    maFtMin->Show();
    maFtMax->Show();

    // remove the automatic entry from color scales
    removeType(*maLbEntryTypeMin, COLORSCALE_AUTO);
    removeType(*maLbEntryTypeMiddle, COLORSCALE_AUTO);
    removeType(*maLbEntryTypeMax, COLORSCALE_AUTO);
    // "min" selector doesn't need "max" entry, and vice versa
    removeType(*maLbEntryTypeMin, COLORSCALE_MAX);
    removeType(*maLbEntryTypeMax, COLORSCALE_MIN);
    maLbColorFormat->SelectEntryPos(1);

    Init();
    maLbType->SelectEntryPos(0);
    if(pFormat)
    {
        ScColorScaleEntries::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr[0], *maLbEntryTypeMin, *maEdMin, *maLbColMin, pDoc);
        assert(pFormat->size() == 3);
        ++itr;
        SetColorScaleEntryTypes(*itr[0], *maLbEntryTypeMiddle, *maEdMiddle, *maLbColMiddle, pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr[0], *maLbEntryTypeMax, *maEdMax, *maLbColMax, pDoc);
    }
    else
    {
        maLbColorFormat->SelectEntryPos(1);
        selectType(*maLbEntryTypeMin, COLORSCALE_MIN);
        selectType(*maLbEntryTypeMiddle, COLORSCALE_PERCENTILE);
        selectType(*maLbEntryTypeMax, COLORSCALE_MAX);
        maEdMiddle->SetText(OUString::number(50));
    }

    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );
    EntryTypeHdl(*maLbEntryTypeMin);
    EntryTypeHdl(*maLbEntryTypeMiddle);
    EntryTypeHdl(*maLbEntryTypeMax);
}

ScColorScale3FrmtEntry::~ScColorScale3FrmtEntry()
{
    disposeOnce();
}

void ScColorScale3FrmtEntry::dispose()
{
    maLbColorFormat.clear();
    maLbEntryTypeMin.clear();
    maLbEntryTypeMiddle.clear();
    maLbEntryTypeMax.clear();
    maEdMin.clear();
    maEdMiddle.clear();
    maEdMax.clear();
    maLbColMin.clear();
    maLbColMiddle.clear();
    maLbColMax.clear();
    maFtMin.clear();
    maFtMax.clear();
    ScCondFrmtEntry::dispose();
}

void ScColorScale3FrmtEntry::Init()
{
    maLbEntryTypeMin->SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMax->SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbEntryTypeMiddle->SetSelectHdl( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    maLbColMin->SelectEntry(COL_LIGHTRED);
    maLbColMiddle->SelectEntry(COL_YELLOW);
    maLbColMax->SelectEntry(Color(0x00CC00));
}

ScFormatEntry* ScColorScale3FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMin, *maLbColMin, *maEdMin, mpDoc, maPos));
    if(maLbColorFormat->GetSelectedEntryPos() == 1)
        pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMiddle, *maLbColMiddle, *maEdMiddle, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(*maLbEntryTypeMax, *maLbColMax, *maEdMax, mpDoc, maPos));
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

IMPL_LINK( ScColorScale3FrmtEntry, EntryTypeHdl, ListBox&, rBox, void )
{
    Edit* pEd = nullptr;
    if(&rBox == maLbEntryTypeMin.get())
        pEd = maEdMin.get();
    else if(&rBox == maLbEntryTypeMiddle.get())
        pEd = maEdMiddle.get();
    else if(&rBox == maLbEntryTypeMax.get())
        pEd = maEdMax.get();

    if (!pEd)
        return;

    bool bEnableEdit = true;
    if (getSelectedType(rBox) <= COLORSCALE_MAX)
    {
        bEnableEdit = false;
    }

    if(bEnableEdit)
        pEd->Enable();
    else
        pEd->Disable();
}

IMPL_LINK_NOARG( ScConditionFrmtEntry, ConditionTypeSelectHdl, ListBox&, void )
{
    sal_Int32 nSelectPos = maLbCondType->GetSelectedEntryPos();
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
}

//databar

namespace {

void SetDataBarEntryTypes( const ScColorScaleEntry& rEntry, ListBox& rLbType, Edit& rEdit, const ScDocument* pDoc )
{
    selectType(rLbType, rEntry.GetType());
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

ScDataBarFrmtEntry::ScDataBarFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat )
    : ScCondFrmtEntry(pParent, pDoc, rPos)
{
    get(maLbColorFormat, "colorformat");
    get(maLbDataBarMinType, "colscalemin");
    get(maLbDataBarMaxType, "colscalemax");
    get(maEdDataBarMin, "edcolscalemin");
    get(maEdDataBarMax, "edcolscalemax");
    get(maBtOptions, "options");
    get(maFtMin, "Label_minimum");
    get(maFtMax, "Label_maximum");

    // "min" selector doesn't need "max" entry, and vice versa
    removeType(*maLbDataBarMinType, COLORSCALE_MAX);
    removeType(*maLbDataBarMaxType, COLORSCALE_MIN);

    maFtMin->Show();
    maFtMax->Show();

    maLbColorFormat->SelectEntryPos(2);
    maLbType->SelectEntryPos(0);
    if(pFormat)
    {
        mpDataBarData.reset(new ScDataBarFormatData(*pFormat->GetDataBarData()));
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, *maLbDataBarMinType, *maEdDataBarMin, pDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, *maLbDataBarMaxType, *maEdDataBarMax, pDoc);
        DataBarTypeSelectHdl(*maLbDataBarMinType);
    }
    else
    {
        selectType(*maLbDataBarMinType, COLORSCALE_AUTO);
        selectType(*maLbDataBarMaxType, COLORSCALE_AUTO);
        DataBarTypeSelectHdl(*maLbDataBarMinType);
    }
    Init();

    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );
}

ScDataBarFrmtEntry::~ScDataBarFrmtEntry()
{
    disposeOnce();
}

void ScDataBarFrmtEntry::dispose()
{
    maLbColorFormat.clear();
    maLbDataBarMinType.clear();
    maLbDataBarMaxType.clear();
    maEdDataBarMin.clear();
    maEdDataBarMax.clear();
    maBtOptions.clear();
    maFtMin.clear();
    maFtMax.clear();
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
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), *maLbDataBarMinType,
                       *maEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), *maLbDataBarMaxType,
                       *maEdDataBarMax, mpDoc, maPos);
    ScDataBarFormat* pDataBar = new ScDataBarFormat(mpDoc);
    pDataBar->SetDataBarData(new ScDataBarFormatData(*mpDataBarData));
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

IMPL_LINK_NOARG( ScDataBarFrmtEntry, DataBarTypeSelectHdl, ListBox&, void )
{
    if (getSelectedType(*maLbDataBarMinType) <= COLORSCALE_MAX)
        maEdDataBarMin->Disable();
    else
        maEdDataBarMin->Enable();

    if (getSelectedType(*maLbDataBarMaxType) <= COLORSCALE_MAX)
        maEdDataBarMax->Disable();
    else
        maEdDataBarMax->Enable();
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, OptionBtnHdl, Button*, void )
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), *maLbDataBarMinType,
                       *maEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), *maLbDataBarMaxType,
                       *maEdDataBarMax, mpDoc, maPos);
    ScDataBarSettingsDlg aDlg(GetFrameWeld(), *mpDataBarData, mpDoc, maPos);
    if (aDlg.run() == RET_OK)
    {
        mpDataBarData.reset(aDlg.GetData());
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, *maLbDataBarMinType, *maEdDataBarMin, mpDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, *maLbDataBarMaxType, *maEdDataBarMax, mpDoc);
        DataBarTypeSelectHdl(*maLbDataBarMinType);
    }
}

ScDateFrmtEntry::ScDateFrmtEntry(vcl::Window* pParent, ScDocument* pDoc, const ScCondDateFormatEntry* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, ScAddress())
    , mbIsInStyleCreate(false)
{
    get(maLbDateEntry, "datetype");
    get(maFtStyle, "styleft");
    get(maLbStyle, "style");
    get(maWdPreview, "preview");
    maWdPreview->set_height_request(maLbStyle->get_preferred_size().Height());

    Init();

    StartListening(*pDoc->GetStyleSheetPool(), DuplicateHandling::Prevent);

    if(pFormat)
    {
        sal_Int32 nPos = static_cast<sal_Int32>(pFormat->GetDateType());
        maLbDateEntry->SelectEntryPos(nPos);

        maLbStyle->SelectEntry(pFormat->GetStyleName());
    }

    StyleSelectHdl(*maLbStyle);
}

ScDateFrmtEntry::~ScDateFrmtEntry()
{
    disposeOnce();
}

void ScDateFrmtEntry::dispose()
{
    maLbDateEntry.clear();
    maFtStyle.clear();
    maLbStyle.clear();
    maWdPreview.clear();
    ScCondFrmtEntry::dispose();
}

void ScDateFrmtEntry::Init()
{
    maLbDateEntry->SelectEntryPos(0);
    maLbType->SelectEntryPos(3);

    FillStyleListBox( mpDoc, *maLbStyle );
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
    if(rHint.GetId() == SfxHintId::StyleSheetModified)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(*maLbStyle, mpDoc);
    }
}

ScFormatEntry* ScDateFrmtEntry::GetEntry() const
{
    ScCondDateFormatEntry* pNewEntry = new ScCondDateFormatEntry(mpDoc);
    condformat::ScCondFormatDateType eType = static_cast<condformat::ScCondFormatDateType>(maLbDateEntry->GetSelectedEntryPos());
    pNewEntry->SetDateType(eType);
    pNewEntry->SetStyleName(maLbStyle->GetSelectedEntry());
    return pNewEntry;
}

OUString ScDateFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(DATE, 0);
}

IMPL_LINK_NOARG( ScDateFrmtEntry, StyleSelectHdl, ListBox&, void )
{
    mbIsInStyleCreate = true;
    StyleSelect( *maLbStyle, mpDoc, *maWdPreview );
    mbIsInStyleCreate = false;
}

class ScIconSetFrmtDataEntry : public VclContainer
                             , public VclBuilderContainer
{
private:
    VclPtr<VclGrid> maGrid;
    VclPtr<FixedImage> maImgIcon;
    VclPtr<FixedText> maFtEntry;
    VclPtr<Edit> maEdEntry;
    VclPtr<ListBox> maLbEntryType;

public:
    ScIconSetFrmtDataEntry( vcl::Window* pParent, ScIconSetType eType, ScDocument* pDoc,
            sal_Int32 i, const ScColorScaleEntry* pEntry = nullptr );
    virtual ~ScIconSetFrmtDataEntry() override;
    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;
    virtual void dispose() override;

    ScColorScaleEntry* CreateEntry(ScDocument* pDoc, const ScAddress& rPos) const;

    void SetFirstEntry();
};

ScIconSetFrmtDataEntry::ScIconSetFrmtDataEntry(vcl::Window* pParent, ScIconSetType eType, ScDocument* pDoc, sal_Int32 i, const ScColorScaleEntry* pEntry)
    : VclContainer(pParent)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "modules/scalc/ui/conditionaliconset.ui"));
    get(maGrid, "ConditionalIconSet");
    get(maImgIcon, "icon");
    get(maFtEntry, "label");
    get(maEdEntry, "entry");
    get(maLbEntryType, "listbox");
    maImgIcon->SetImage(Image(ScIconSetFormat::getBitmap(pDoc->GetIconSetBitmapMap(), eType, i)));
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
}

Size ScIconSetFrmtDataEntry::calculateRequisition() const
{
    if (!maGrid) //during dispose
        return Size();
    return getLayoutRequisition(*maGrid);
}

void ScIconSetFrmtDataEntry::setAllocation(const Size &rAllocation)
{
    if (!maGrid) //during dispose
        return;
    setLayoutPosSize(*maGrid, Point(0, 0), rAllocation);
}

ScIconSetFrmtDataEntry::~ScIconSetFrmtDataEntry()
{
    disposeOnce();
}

void ScIconSetFrmtDataEntry::dispose()
{
    maImgIcon.clear();
    maFtEntry.clear();
    maEdEntry.clear();
    maLbEntryType.clear();
    maGrid.clear();
    disposeBuilder();
    VclContainer::dispose();
}

ScColorScaleEntry* ScIconSetFrmtDataEntry::CreateEntry(ScDocument* pDoc, const ScAddress& rPos) const
{
    sal_Int32 nPos = maLbEntryType->GetSelectedEntryPos();
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
    maEdEntry->SetText("0");
    maLbEntryType->SelectEntryPos(1);
}

ScIconSetFrmtEntry::ScIconSetFrmtEntry(vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScIconSetFormat* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
{
    get(maLbColorFormat, "colorformat");
    get(maLbIconSetType, "iconsettype");
    get(maIconParent, "iconparent");

    Init();
    maLbColorFormat->SetSelectHdl( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    if(pFormat)
    {
        const ScIconSetFormatData* pIconSetFormatData = pFormat->GetIconSetData();
        ScIconSetType eType = pIconSetFormatData->eIconSetType;
        sal_Int32 nType = static_cast<sal_Int32>(eType);
        maLbIconSetType->SelectEntryPos(nType);

        for (size_t i = 0, n = pIconSetFormatData->m_Entries.size();
                i < n; ++i)
        {
            maEntries.push_back(VclPtr<ScIconSetFrmtDataEntry>::Create(
                maIconParent, eType, pDoc, i, pIconSetFormatData->m_Entries[i].get()));
            maEntries[i]->set_grid_top_attach(i);
        }
        maEntries[0]->SetFirstEntry();
    }
    else
        IconSetTypeHdl(*maLbIconSetType);
}

ScIconSetFrmtEntry::~ScIconSetFrmtEntry()
{
    disposeOnce();
}

void ScIconSetFrmtEntry::dispose()
{
    for (auto& rxEntry : maEntries)
        rxEntry.disposeAndClear();
    maEntries.clear();
    maIconParent.clear();
    maLbColorFormat.clear();
    maLbIconSetType.clear();
    ScCondFrmtEntry::dispose();
}

void ScIconSetFrmtEntry::Init()
{
    maLbColorFormat->SelectEntryPos(3);
    maLbType->SelectEntryPos(0);
    maLbIconSetType->SelectEntryPos(0);

    maLbIconSetType->SetSelectHdl( LINK( this, ScIconSetFrmtEntry, IconSetTypeHdl ) );
}

IMPL_LINK_NOARG( ScIconSetFrmtEntry, IconSetTypeHdl, ListBox&, void )
{
    const ScIconSetMap* pMap = ScIconSetFormat::g_IconSetMap;

    sal_Int32 nPos = maLbIconSetType->GetSelectedEntryPos();
    sal_uInt32 nElements = pMap[nPos].nElements;

    for (auto& rxEntry : maEntries)
        rxEntry.disposeAndClear();
    maEntries.clear();

    for(size_t i = 0; i < nElements; ++i)
    {
        maEntries.push_back(VclPtr<ScIconSetFrmtDataEntry>::Create(maIconParent, static_cast<ScIconSetType>(nPos), mpDoc, i));
        maEntries[i]->set_grid_top_attach(i);
        maEntries[i]->Show();
    }
    maEntries[0]->SetFirstEntry();

    SetHeight();
}

OUString ScIconSetFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(ICONSET, 0);
}

void ScIconSetFrmtEntry::SetActive()
{
    maLbColorFormat->Show();
    maLbIconSetType->Show();
    for(auto& rxEntry : maEntries)
    {
        rxEntry->Show();
    }

    Select();
}

void ScIconSetFrmtEntry::SetInactive()
{
    maLbColorFormat->Hide();
    maLbIconSetType->Hide();
    for(auto& rxEntry : maEntries)
    {
        rxEntry->Hide();
    }

    Deselect();
}

ScFormatEntry* ScIconSetFrmtEntry::GetEntry() const
{
    ScIconSetFormat* pFormat = new ScIconSetFormat(mpDoc);

    ScIconSetFormatData* pData = new ScIconSetFormatData;
    pData->eIconSetType = static_cast<ScIconSetType>(maLbIconSetType->GetSelectedEntryPos());
    for(const auto& rxEntry : maEntries)
    {
        pData->m_Entries.push_back(std::unique_ptr<ScColorScaleEntry>(rxEntry->CreateEntry(mpDoc, maPos)));
    }
    pFormat->SetIconSetData(pData);

    return pFormat;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
