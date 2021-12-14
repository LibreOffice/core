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
#include <sfx2/frame.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/numformat.hxx>
#include <svx/colorbox.hxx>
#include <vcl/svapp.hxx>
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

// set the widget width to something to override their auto-width calc and
// force them to take a 1/3 of the available space
#define CommonWidgetWidth 10

ScCondFrmtEntry::ScCondFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos)
    : mpParent(pParent)
    , mxBuilder(Application::CreateBuilder(pParent->GetContainer(), (SfxViewShell::Current() && SfxViewShell::Current()->isLOKMobilePhone())?OUString("modules/scalc/ui/conditionalentrymobile.ui"):OUString("modules/scalc/ui/conditionalentry.ui")))
    , mxBorder(mxBuilder->weld_widget("border"))
    , mxGrid(mxBuilder->weld_container("grid"))
    , mxFtCondNr(mxBuilder->weld_label("number"))
    , mxFtCondition(mxBuilder->weld_label("condition"))
    , mbActive(false)
    , maStrCondition(ScResId(SCSTR_CONDITION))
    , mxLbType(mxBuilder->weld_combo_box("type"))
    , mpDoc(pDoc)
    , maPos(rPos)
{
    mxLbType->set_size_request(CommonWidgetWidth, -1);
    mxLbType->connect_changed(LINK(pParent, ScCondFormatList, TypeListHdl));
    mxGrid->connect_mouse_press(LINK(this, ScCondFrmtEntry, EntrySelectHdl));
    maClickHdl = LINK( pParent, ScCondFormatList, EntrySelectHdl );

    Show();
}

ScCondFrmtEntry::~ScCondFrmtEntry()
{
    mpParent->GetContainer()->move(mxBorder.get(), nullptr);
}

IMPL_LINK_NOARG(ScCondFrmtEntry, EntrySelectHdl, const MouseEvent&, bool)
{
    maClickHdl.Call(*this);
    return false;
}

void ScCondFrmtEntry::SetIndex(sal_Int32 nIndex)
{
    OUString sLabel = maStrCondition + OUString::number(nIndex);
    mxFtCondNr->set_label(sLabel);

    // tdf#124412: uitest
    mxFtCondition->set_buildable_name(sLabel.toUtf8());
}

void ScCondFrmtEntry::Select()
{
    mxFtCondition->set_label(OUString());
    mxFtCondition->hide();
    mxLbType->show();
    mbActive = true;
}

void ScCondFrmtEntry::Deselect()
{
    OUString aCondText = GetExpressionString();
    mxFtCondition->set_label(aCondText);
    mxFtCondition->show();
    mxLbType->hide();
    mbActive = false;
}

//condition

namespace {

void FillStyleListBox( const ScDocument* pDoc, weld::ComboBox& rLbStyle )
{
    std::set<OUString> aStyleNames;
    SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SfxStyleFamily::Para );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        aStyleNames.insert(pStyle->GetName());
    }
    for(const auto& rStyleName : aStyleNames)
    {
        rLbStyle.append_text(rStyleName);
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

ScConditionFrmtEntry::ScConditionFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent,
        const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
    , mxLbCondType(mxBuilder->weld_combo_box("typeis"))
    , mxEdVal1(new formula::RefEdit(mxBuilder->weld_entry("val1")))
    , mxEdVal2(new formula::RefEdit(mxBuilder->weld_entry("val2")))
    , mxFtVal(mxBuilder->weld_label("valueft"))
    , mxFtStyle(mxBuilder->weld_label("styleft"))
    , mxLbStyle(mxBuilder->weld_combo_box("style"))
    , mxWdPreviewWin(mxBuilder->weld_widget("previewwin"))
    , mxWdPreview(new weld::CustomWeld(*mxBuilder, "preview", maWdPreview))
    , mbIsInStyleCreate(false)
{
    mxLbCondType->set_size_request(CommonWidgetWidth, -1);
    mxLbType->set_size_request(CommonWidgetWidth, -1);
    mxWdPreview->set_size_request(-1, mxLbStyle->get_preferred_size().Height());

    mxLbType->set_active(1);

    Init(pDialogParent);

    StartListening(*pDoc->GetStyleSheetPool(), DuplicateHandling::Prevent);

    if(pFormatEntry)
    {
        mxLbStyle->set_active_text(pFormatEntry->GetStyle());
        StyleSelectHdl(*mxLbStyle);
        ScConditionMode eMode = pFormatEntry->GetOperation();

        mxLbCondType->set_active(ConditionModeToEntryPos(eMode));

        switch(GetNumberEditFields(eMode))
        {
            case 0:
                mxEdVal1->GetWidget()->hide();
                mxEdVal2->GetWidget()->hide();
                break;
            case 1:
                mxEdVal1->GetWidget()->show();
                mxEdVal1->SetText(pFormatEntry->GetExpression(maPos, 0));
                mxEdVal2->GetWidget()->hide();
                OnEdChanged(*mxEdVal1);
                break;
            case 2:
                mxEdVal1->GetWidget()->show();
                mxEdVal1->SetText(pFormatEntry->GetExpression(maPos, 0));
                OnEdChanged(*mxEdVal1);
                mxEdVal2->GetWidget()->show();
                mxEdVal2->SetText(pFormatEntry->GetExpression(maPos, 1));
                OnEdChanged(*mxEdVal2);
                break;
        }
    }
    else
    {
        mxLbCondType->set_active(0);
        mxEdVal2->GetWidget()->hide();
        mxLbStyle->set_active(1);
    }
}

ScConditionFrmtEntry::~ScConditionFrmtEntry()
{
}

void ScConditionFrmtEntry::Init(ScCondFormatDlg* pDialogParent)
{
    mxEdVal1->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );
    mxEdVal2->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );

    mxEdVal1->SetModifyHdl( LINK( this, ScConditionFrmtEntry, OnEdChanged ) );
    mxEdVal2->SetModifyHdl( LINK( this, ScConditionFrmtEntry, OnEdChanged ) );

    FillStyleListBox( mpDoc, *mxLbStyle );
    mxLbStyle->connect_changed( LINK( this, ScConditionFrmtEntry, StyleSelectHdl ) );

    mxLbCondType->connect_changed( LINK( this, ScConditionFrmtEntry, ConditionTypeSelectHdl ) );
}

ScFormatEntry* ScConditionFrmtEntry::createConditionEntry() const
{
    ScConditionMode eMode = EntryPosToConditionMode(mxLbCondType->get_active());
    OUString aExpr1 = mxEdVal1->GetText();
    OUString aExpr2;
    if (GetNumberEditFields(eMode) == 2)
    {
        aExpr2 = mxEdVal2->GetText();
        if (aExpr2.isEmpty())
        {
            return nullptr;
        }
    }

    ScFormatEntry* pEntry = new ScCondFormatEntry(eMode, aExpr1, aExpr2, *mpDoc, maPos, mxLbStyle->get_active_text());
    return pEntry;
}

IMPL_LINK(ScConditionFrmtEntry, OnEdChanged, formula::RefEdit&, rRefEdit, void)
{
    weld::Entry& rEdit = *rRefEdit.GetWidget();
    OUString aFormula = rEdit.get_text();

    if( aFormula.isEmpty() )
    {
        mxFtVal->set_label(ScResId(STR_ENTER_VALUE));
        return;
    }

    ScCompiler aComp( *mpDoc, maPos, mpDoc->GetGrammar() );
    std::unique_ptr<ScTokenArray> ta(aComp.CompileString(aFormula));

    // Error, warn the user
    if( ta->GetCodeError() != FormulaError::NONE || ( ta->GetLen() == 0 ) )
    {
        rEdit.set_message_type(weld::EntryMessageType::Error);
        mxFtVal->set_label(ScResId(STR_VALID_DEFERROR));
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
        rEdit.set_message_type(weld::EntryMessageType::Warning);
        mxFtVal->set_label(ScResId(STR_UNQUOTED_STRING));
        return;
    }

    rEdit.set_message_type(weld::EntryMessageType::Normal);
    mxFtVal->set_label("");
}

void ScConditionFrmtEntry::Select()
{
    mxFtVal->show();
    ScCondFrmtEntry::Select();
}

void ScConditionFrmtEntry::Deselect()
{
    mxFtVal->hide();
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
    return ScCondFormatHelper::GetExpression(CONDITION, mxLbCondType->get_active(), mxEdVal1->GetText(), mxEdVal2->GetText());
}

ScFormatEntry* ScConditionFrmtEntry::GetEntry() const
{
    return createConditionEntry();
}

void ScConditionFrmtEntry::SetActive()
{
    ScConditionMode eMode = EntryPosToConditionMode(mxLbCondType->get_active());
    mxLbCondType->show();
    switch(GetNumberEditFields(eMode))
    {
        case 1:
            mxEdVal1->GetWidget()->show();
            break;
        case 2:
            mxEdVal1->GetWidget()->show();
            mxEdVal2->GetWidget()->show();
            break;
    }
    mxFtStyle->show();
    mxLbStyle->show();
    mxWdPreviewWin->show();

    Select();
}

void ScConditionFrmtEntry::SetInactive()
{
    mxLbCondType->hide();
    mxEdVal1->GetWidget()->hide();
    mxEdVal2->GetWidget()->hide();
    mxFtStyle->hide();
    mxLbStyle->hide();
    mxWdPreviewWin->hide();

    Deselect();
}

namespace {

void UpdateStyleList(weld::ComboBox& rLbStyle, const ScDocument* pDoc)
{
    OUString aSelectedStyle = rLbStyle.get_active_text();
    for (sal_Int32 i = rLbStyle.get_count(); i > 1; --i)
        rLbStyle.remove(i - 1);
    FillStyleListBox(pDoc, rLbStyle);
    rLbStyle.set_active_text(aSelectedStyle);
}

}

void ScConditionFrmtEntry::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::StyleSheetModified)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(*mxLbStyle, mpDoc);
    }
}

namespace {

void StyleSelect(weld::Window* pDialogParent, weld::ComboBox& rLbStyle, const ScDocument* pDoc, SvxFontPrevWindow& rWdPreview)
{
    if (rLbStyle.get_active() == 0)
    {
        // call new style dialog
        SfxUInt16Item aFamilyItem( SID_STYLE_FAMILY, sal_uInt16(SfxStyleFamily::Para) );
        SfxStringItem aRefItem( SID_STYLE_REFERENCE, ScResId(STR_STYLENAME_STANDARD) );
        css::uno::Any aAny(pDialogParent->GetXWindow());
        SfxUnoAnyItem aDialogParent( SID_DIALOG_PARENT, aAny );

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
            { &aFamilyItem, &aRefItem }, { &aDialogParent });

        if (bLocked)
            pDisp->Lock(true);

        // Find the new style and add it into the style list boxes
        SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SfxStyleFamily::Para );
        bool bFound = false;
        for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle && !bFound; pStyle = aStyleIter.Next() )
        {
            const OUString& aName = pStyle->GetName();
            if (rLbStyle.find_text(aName) == -1)    // all lists contain the same entries
            {
                for( sal_Int32 i = 1, n = rLbStyle.get_count(); i <= n && !bFound; ++i)
                {
                    OUString aStyleName = ScGlobal::getCharClass().uppercase(rLbStyle.get_text(i));
                    if( i == n )
                    {
                        rLbStyle.append_text(aName);
                        rLbStyle.set_active_text(aName);
                        bFound = true;
                    }
                    else if( aStyleName > ScGlobal::getCharClass().uppercase(aName) )
                    {
                        rLbStyle.insert_text(i, aName);
                        rLbStyle.set_active_text(aName);
                        bFound = true;
                    }
                }
            }
        }
    }

    OUString aStyleName = rLbStyle.get_active_text();
    SfxStyleSheetBase* pStyleSheet = pDoc->GetStyleSheetPool()->Find( aStyleName, SfxStyleFamily::Para );
    if(pStyleSheet)
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        rWdPreview.SetFromItemSet(rSet, false);
    }
}

}

IMPL_LINK_NOARG(ScConditionFrmtEntry, StyleSelectHdl, weld::ComboBox&, void)
{
    mbIsInStyleCreate = true;
    StyleSelect(mpParent->GetFrameWeld(), *mxLbStyle, mpDoc, maWdPreview);
    mbIsInStyleCreate = false;
}

// formula

ScFormulaFrmtEntry::ScFormulaFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent, const ScAddress& rPos, const ScCondFormatEntry* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
    , mxFtStyle(mxBuilder->weld_label("styleft"))
    , mxLbStyle(mxBuilder->weld_combo_box("style"))
    , mxWdPreviewWin(mxBuilder->weld_widget("previewwin"))
    , mxWdPreview(new weld::CustomWeld(*mxBuilder, "preview", maWdPreview))
    , mxEdFormula(new formula::RefEdit(mxBuilder->weld_entry("formula")))
{
    mxLbType->set_size_request(CommonWidgetWidth, -1);
    mxWdPreview->set_size_request(-1, mxLbStyle->get_preferred_size().Height());

    Init(pDialogParent);

    mxLbType->set_active(2);

    if(pFormat)
    {
        mxEdFormula->SetText(pFormat->GetExpression(rPos, 0, 0, pDoc->GetGrammar()));
        mxLbStyle->set_active_text(pFormat->GetStyle());
    }
    else
    {
        mxLbStyle->set_active(1);
    }

    StyleSelectHdl(*mxLbStyle);
}

ScFormulaFrmtEntry::~ScFormulaFrmtEntry()
{
}

void ScFormulaFrmtEntry::Init(ScCondFormatDlg* pDialogParent)
{
    mxEdFormula->SetGetFocusHdl( LINK( pDialogParent, ScCondFormatDlg, RangeGetFocusHdl ) );

    FillStyleListBox( mpDoc, *mxLbStyle );
    mxLbStyle->connect_changed( LINK( this, ScFormulaFrmtEntry, StyleSelectHdl ) );
}

IMPL_LINK_NOARG(ScFormulaFrmtEntry, StyleSelectHdl, weld::ComboBox&, void)
{
    StyleSelect(mpParent->GetFrameWeld(), *mxLbStyle, mpDoc, maWdPreview);
}

ScFormatEntry* ScFormulaFrmtEntry::createFormulaEntry() const
{
    OUString aFormula = mxEdFormula->GetText();
    if(aFormula.isEmpty())
        return nullptr;

    ScFormatEntry* pEntry = new ScCondFormatEntry(ScConditionMode::Direct, aFormula, OUString(), *mpDoc, maPos, mxLbStyle->get_active_text());
    return pEntry;
}

ScFormatEntry* ScFormulaFrmtEntry::GetEntry() const
{
    return createFormulaEntry();
}

OUString ScFormulaFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(FORMULA, 0, mxEdFormula->GetText());
}

void ScFormulaFrmtEntry::SetActive()
{
    mxWdPreviewWin->show();
    mxFtStyle->show();
    mxLbStyle->show();
    mxEdFormula->GetWidget()->show();

    Select();
}

void ScFormulaFrmtEntry::SetInactive()
{
    mxWdPreviewWin->hide();
    mxFtStyle->hide();
    mxLbStyle->hide();
    mxEdFormula->GetWidget()->hide();

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

ScColorScaleEntryType getSelectedType(const weld::ComboBox& rListBox)
{
    return getTypeForId(rListBox.get_active_id());
}

sal_Int32 getEntryPos(const weld::ComboBox& rListBox, ScColorScaleEntryType eType)
{
    const sal_Int32 nSize = rListBox.get_count();
    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        if (getTypeForId(rListBox.get_id(i)) == eType)
            return i;
    }
    return -1;
}

void selectType(weld::ComboBox& rListBox, ScColorScaleEntryType eType)
{
    const sal_Int32 nPos = getEntryPos(rListBox, eType);
    if (nPos >= 0)
        rListBox.set_active(nPos);
}

void removeType(weld::ComboBox& rListBox, ScColorScaleEntryType eType)
{
    const sal_Int32 nPos = getEntryPos(rListBox, eType);
    if (nPos >= 0)
        rListBox.remove(nPos);
}

void SetColorScaleEntryTypes( const ScColorScaleEntry& rEntry, weld::ComboBox& rLbType, weld::Entry& rEdit, ColorListBox& rLbCol, const ScDocument* pDoc )
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
                rEdit.set_text(convertNumberToString(nVal, pDoc));
            }
            break;
        case COLORSCALE_FORMULA:
            rEdit.set_text(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            break;
        case COLORSCALE_AUTO:
            abort();
            break;
    }
    rLbCol.SelectEntry(rEntry.GetColor());
}

void SetColorScaleEntry(ScColorScaleEntry* pEntry, const weld::ComboBox& rType, const weld::Entry& rValue,
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
                (void)pNumberFormatter->IsNumberFormat(rValue.get_text(), nIndex, nVal);
                pEntry->SetValue(nVal);
            }
            break;
        case COLORSCALE_FORMULA:
            pEntry->SetFormula(rValue.get_text(), *pDoc, rPos);
            break;
        default:
            break;
    }
}

ScColorScaleEntry* createColorScaleEntry( const weld::ComboBox& rType, const ColorListBox& rColor, const weld::Entry& rValue, ScDocument* pDoc, const ScAddress& rPos )
{
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    SetColorScaleEntry(pEntry, rType, rValue, pDoc, rPos);
    Color aColor = rColor.GetSelectEntryColor();
    pEntry->SetColor(aColor);
    return pEntry;
}

}

ScColorScale2FrmtEntry::ScColorScale2FrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
    , mxLbColorFormat(mxBuilder->weld_combo_box("colorformat"))
    , mxLbEntryTypeMin(mxBuilder->weld_combo_box("colscalemin"))
    , mxLbEntryTypeMax(mxBuilder->weld_combo_box("colscalemax"))
    , mxEdMin(mxBuilder->weld_entry("edcolscalemin"))
    , mxEdMax(mxBuilder->weld_entry("edcolscalemax"))
    , mxLbColMin(new ColorListBox(mxBuilder->weld_menu_button("lbcolmin"), [this]{ return mpParent->GetFrameWeld(); }))
    , mxLbColMax(new ColorListBox(mxBuilder->weld_menu_button("lbcolmax"), [this]{ return mpParent->GetFrameWeld(); }))
    , mxFtMin(mxBuilder->weld_label("Label_minimum"))
    , mxFtMax(mxBuilder->weld_label("Label_maximum"))
{
    mxLbColorFormat->set_size_request(CommonWidgetWidth, -1);
    mxLbEntryTypeMin->set_size_request(CommonWidgetWidth, -1);
    mxLbEntryTypeMax->set_size_request(CommonWidgetWidth, -1);
    mxLbColMin->get_widget().set_size_request(CommonWidgetWidth, -1);
    mxLbColMax->get_widget().set_size_request(CommonWidgetWidth, -1);

    mxFtMin->show();
    mxFtMax->show();

    // remove the automatic entry from color scales
    removeType(*mxLbEntryTypeMin, COLORSCALE_AUTO);
    removeType(*mxLbEntryTypeMax, COLORSCALE_AUTO);
    // "min" selector doesn't need "max" entry, and vice versa
    removeType(*mxLbEntryTypeMin, COLORSCALE_MAX);
    removeType(*mxLbEntryTypeMax, COLORSCALE_MIN);

    mxLbType->set_active(0);
    mxLbColorFormat->set_active(0);
    Init();
    if(pFormat)
    {
        ScColorScaleEntries::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr[0], *mxLbEntryTypeMin, *mxEdMin, *mxLbColMin, pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr[0], *mxLbEntryTypeMax, *mxEdMax, *mxLbColMax, pDoc);
    }
    else
    {
        selectType(*mxLbEntryTypeMin, COLORSCALE_MIN);
        selectType(*mxLbEntryTypeMax, COLORSCALE_MAX);
    }

    mxLbColorFormat->connect_changed( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );

    EntryTypeHdl(*mxLbEntryTypeMin);
    EntryTypeHdl(*mxLbEntryTypeMax);
}

ScColorScale2FrmtEntry::~ScColorScale2FrmtEntry()
{
}

void ScColorScale2FrmtEntry::Init()
{
    mxLbEntryTypeMin->connect_changed( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );
    mxLbEntryTypeMax->connect_changed( LINK( this, ScColorScale2FrmtEntry, EntryTypeHdl ) );
    mxLbColMin->SelectEntry(Color(0xffff6d)); // Light Yellow 2
    mxLbColMax->SelectEntry(Color(0x77bc65)); // Light Green 2
}

ScFormatEntry* ScColorScale2FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(*mxLbEntryTypeMin, *mxLbColMin, *mxEdMin, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(*mxLbEntryTypeMax, *mxLbColMax, *mxEdMax, mpDoc, maPos));
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
    mxLbColorFormat->show();

    mxLbEntryTypeMin->show();
    mxLbEntryTypeMax->show();

    mxEdMin->show();
    mxEdMax->show();

    mxLbColMin->show();
    mxLbColMax->show();

    Select();
}

void ScColorScale2FrmtEntry::SetInactive()
{
    mxLbColorFormat->hide();

    mxLbEntryTypeMin->hide();
    mxLbEntryTypeMax->hide();

    mxEdMin->hide();
    mxEdMax->hide();

    mxLbColMin->hide();
    mxLbColMax->hide();

    Deselect();
}

IMPL_LINK( ScColorScale2FrmtEntry, EntryTypeHdl, weld::ComboBox&, rBox, void )
{
    weld::Entry* pEd = nullptr;
    if (&rBox == mxLbEntryTypeMin.get())
        pEd = mxEdMin.get();
    else if (&rBox == mxLbEntryTypeMax.get())
        pEd = mxEdMax.get();

    if (!pEd)
        return;

    bool bEnableEdit = true;
    if (getSelectedType(rBox) <= COLORSCALE_MAX)
    {
        bEnableEdit = false;
    }

    if (bEnableEdit)
        pEd->set_sensitive(true);
    else
        pEd->set_sensitive(false);
}

ScColorScale3FrmtEntry::ScColorScale3FrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
    , mxLbColorFormat(mxBuilder->weld_combo_box("colorformat"))
    , mxLbEntryTypeMin(mxBuilder->weld_combo_box("colscalemin"))
    , mxLbEntryTypeMiddle(mxBuilder->weld_combo_box("colscalemiddle"))
    , mxLbEntryTypeMax(mxBuilder->weld_combo_box("colscalemax"))
    , mxEdMin(mxBuilder->weld_entry("edcolscalemin"))
    , mxEdMiddle(mxBuilder->weld_entry("edcolscalemiddle"))
    , mxEdMax(mxBuilder->weld_entry("edcolscalemax"))
    , mxLbColMin(new ColorListBox(mxBuilder->weld_menu_button("lbcolmin"), [this]{ return mpParent->GetFrameWeld(); }))
    , mxLbColMiddle(new ColorListBox(mxBuilder->weld_menu_button("lbcolmiddle"), [this]{ return mpParent->GetFrameWeld(); }))
    , mxLbColMax(new ColorListBox(mxBuilder->weld_menu_button("lbcolmax"), [this]{ return mpParent->GetFrameWeld(); }))
    , mxFtMin(mxBuilder->weld_label("Label_minimum"))
    , mxFtMax(mxBuilder->weld_label("Label_maximum"))
{
    mxLbColorFormat->set_size_request(CommonWidgetWidth, -1);
    mxLbEntryTypeMin->set_size_request(CommonWidgetWidth, -1);
    mxLbEntryTypeMiddle->set_size_request(CommonWidgetWidth, -1);
    mxLbEntryTypeMax->set_size_request(CommonWidgetWidth, -1);
    mxLbColMin->get_widget().set_size_request(CommonWidgetWidth, -1);
    mxLbColMiddle->get_widget().set_size_request(CommonWidgetWidth, -1);
    mxLbColMax->get_widget().set_size_request(CommonWidgetWidth, -1);
    mxFtMin->show();
    mxFtMax->show();

    // remove the automatic entry from color scales
    removeType(*mxLbEntryTypeMin, COLORSCALE_AUTO);
    removeType(*mxLbEntryTypeMiddle, COLORSCALE_AUTO);
    removeType(*mxLbEntryTypeMax, COLORSCALE_AUTO);
    // "min" selector doesn't need "max" entry, and vice versa
    removeType(*mxLbEntryTypeMin, COLORSCALE_MAX);
    removeType(*mxLbEntryTypeMax, COLORSCALE_MIN);
    mxLbColorFormat->set_active(1);

    Init();
    mxLbType->set_active(0);
    if(pFormat)
    {
        ScColorScaleEntries::const_iterator itr = pFormat->begin();
        SetColorScaleEntryTypes(*itr[0], *mxLbEntryTypeMin, *mxEdMin, *mxLbColMin, pDoc);
        assert(pFormat->size() == 3);
        ++itr;
        SetColorScaleEntryTypes(*itr[0], *mxLbEntryTypeMiddle, *mxEdMiddle, *mxLbColMiddle, pDoc);
        ++itr;
        SetColorScaleEntryTypes(*itr[0], *mxLbEntryTypeMax, *mxEdMax, *mxLbColMax, pDoc);
    }
    else
    {
        mxLbColorFormat->set_active(1);
        selectType(*mxLbEntryTypeMin, COLORSCALE_MIN);
        selectType(*mxLbEntryTypeMiddle, COLORSCALE_PERCENTILE);
        selectType(*mxLbEntryTypeMax, COLORSCALE_MAX);
        mxEdMiddle->set_text(OUString::number(50));
    }

    mxLbColorFormat->connect_changed( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );
    EntryTypeHdl(*mxLbEntryTypeMin);
    EntryTypeHdl(*mxLbEntryTypeMiddle);
    EntryTypeHdl(*mxLbEntryTypeMax);
}

ScColorScale3FrmtEntry::~ScColorScale3FrmtEntry()
{
}

void ScColorScale3FrmtEntry::Init()
{
    mxLbEntryTypeMin->connect_changed( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    mxLbEntryTypeMax->connect_changed( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    mxLbEntryTypeMiddle->connect_changed( LINK( this, ScColorScale3FrmtEntry, EntryTypeHdl ) );
    mxLbColMin->SelectEntry(COL_LIGHTRED);
    mxLbColMiddle->SelectEntry(COL_YELLOW);
    mxLbColMax->SelectEntry(Color(0x00a933));
}

ScFormatEntry* ScColorScale3FrmtEntry::createColorscaleEntry() const
{
    ScColorScaleFormat* pColorScale = new ScColorScaleFormat(mpDoc);
    pColorScale->AddEntry(createColorScaleEntry(*mxLbEntryTypeMin, *mxLbColMin, *mxEdMin, mpDoc, maPos));
    if (mxLbColorFormat->get_active() == 1)
        pColorScale->AddEntry(createColorScaleEntry(*mxLbEntryTypeMiddle, *mxLbColMiddle, *mxEdMiddle, mpDoc, maPos));
    pColorScale->AddEntry(createColorScaleEntry(*mxLbEntryTypeMax, *mxLbColMax, *mxEdMax, mpDoc, maPos));
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
    mxLbColorFormat->show();
    mxLbEntryTypeMin->show();
    mxLbEntryTypeMiddle->show();
    mxLbEntryTypeMax->show();

    mxEdMin->show();
    mxEdMiddle->show();
    mxEdMax->show();

    mxLbColMin->show();
    mxLbColMiddle->show();
    mxLbColMax->show();

    Select();
}

void ScColorScale3FrmtEntry::SetInactive()
{
    mxLbColorFormat->hide();

    mxLbEntryTypeMin->hide();
    mxLbEntryTypeMiddle->hide();
    mxLbEntryTypeMax->hide();

    mxEdMin->hide();
    mxEdMiddle->hide();
    mxEdMax->hide();

    mxLbColMin->hide();
    mxLbColMiddle->hide();
    mxLbColMax->hide();

    Deselect();
}

IMPL_LINK( ScColorScale3FrmtEntry, EntryTypeHdl, weld::ComboBox&, rBox, void )
{
    weld::Entry* pEd = nullptr;
    if(&rBox == mxLbEntryTypeMin.get())
        pEd = mxEdMin.get();
    else if(&rBox == mxLbEntryTypeMiddle.get())
        pEd = mxEdMiddle.get();
    else if(&rBox == mxLbEntryTypeMax.get())
        pEd = mxEdMax.get();

    if (!pEd)
        return;

    bool bEnableEdit = true;
    if (getSelectedType(rBox) <= COLORSCALE_MAX)
    {
        bEnableEdit = false;
    }

    if(bEnableEdit)
        pEd->set_sensitive(true);
    else
        pEd->set_sensitive(false);
}

IMPL_LINK_NOARG(ScConditionFrmtEntry, ConditionTypeSelectHdl, weld::ComboBox&, void)
{
    sal_Int32 nSelectPos = mxLbCondType->get_active();
    ScConditionMode eMode = EntryPosToConditionMode(nSelectPos);
    switch(GetNumberEditFields(eMode))
    {
        case 0:
            mxEdVal1->GetWidget()->hide();
            mxEdVal2->GetWidget()->hide();
            mxFtVal->hide();
            break;
        case 1:
            mxEdVal1->GetWidget()->show();
            mxEdVal2->GetWidget()->hide();
            mxFtVal->show();
            break;
        case 2:
            mxEdVal1->GetWidget()->show();
            mxEdVal2->GetWidget()->show();
            mxFtVal->show();
            break;
    }
}

//databar

namespace {

void SetDataBarEntryTypes( const ScColorScaleEntry& rEntry, weld::ComboBox& rLbType, weld::Entry& rEdit, const ScDocument* pDoc )
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
                rEdit.set_text(aText);
            }
            break;
        case COLORSCALE_FORMULA:
            rEdit.set_text(rEntry.GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
            break;
    }
}

}

ScDataBarFrmtEntry::ScDataBarFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
    , mxLbColorFormat(mxBuilder->weld_combo_box("colorformat"))
    , mxLbDataBarMinType(mxBuilder->weld_combo_box("colscalemin"))
    , mxLbDataBarMaxType(mxBuilder->weld_combo_box("colscalemax"))
    , mxEdDataBarMin(mxBuilder->weld_entry("edcolscalemin"))
    , mxEdDataBarMax(mxBuilder->weld_entry("edcolscalemax"))
    , mxBtOptions(mxBuilder->weld_button("options"))
    , mxFtMin(mxBuilder->weld_label("Label_minimum"))
    , mxFtMax(mxBuilder->weld_label("Label_maximum"))
{
    mxLbColorFormat->set_size_request(CommonWidgetWidth, -1);
    mxLbDataBarMinType->set_size_request(CommonWidgetWidth, -1);
    mxLbDataBarMaxType->set_size_request(CommonWidgetWidth, -1);

    // "min" selector doesn't need "max" entry, and vice versa
    removeType(*mxLbDataBarMinType, COLORSCALE_MAX);
    removeType(*mxLbDataBarMaxType, COLORSCALE_MIN);

    mxFtMin->show();
    mxFtMax->show();

    mxLbColorFormat->set_active(2);
    mxLbType->set_active(0);
    if(pFormat)
    {
        mpDataBarData.reset(new ScDataBarFormatData(*pFormat->GetDataBarData()));
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, *mxLbDataBarMinType, *mxEdDataBarMin, pDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, *mxLbDataBarMaxType, *mxEdDataBarMax, pDoc);
        DataBarTypeSelectHdl(*mxLbDataBarMinType);
    }
    else
    {
        selectType(*mxLbDataBarMinType, COLORSCALE_AUTO);
        selectType(*mxLbDataBarMaxType, COLORSCALE_AUTO);
        DataBarTypeSelectHdl(*mxLbDataBarMinType);
    }
    Init();

    mxLbColorFormat->connect_changed( LINK( pParent, ScCondFormatList, ColFormatTypeHdl ) );
}

ScDataBarFrmtEntry::~ScDataBarFrmtEntry()
{
}

ScFormatEntry* ScDataBarFrmtEntry::GetEntry() const
{
    return createDatabarEntry();
}

void ScDataBarFrmtEntry::Init()
{
    mxLbDataBarMinType->connect_changed( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );
    mxLbDataBarMaxType->connect_changed( LINK( this, ScDataBarFrmtEntry, DataBarTypeSelectHdl ) );

    mxBtOptions->connect_clicked( LINK( this, ScDataBarFrmtEntry, OptionBtnHdl ) );

    if(!mpDataBarData)
    {
        mpDataBarData.reset(new ScDataBarFormatData());
        mpDataBarData->mpUpperLimit.reset(new ScColorScaleEntry());
        mpDataBarData->mpLowerLimit.reset(new ScColorScaleEntry());
        mpDataBarData->mpLowerLimit->SetType(COLORSCALE_AUTO);
        mpDataBarData->mpUpperLimit->SetType(COLORSCALE_AUTO);
        mpDataBarData->maPositiveColor = 0x2a6099;
    }
}

ScFormatEntry* ScDataBarFrmtEntry::createDatabarEntry() const
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), *mxLbDataBarMinType,
                       *mxEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), *mxLbDataBarMaxType,
                       *mxEdDataBarMax, mpDoc, maPos);
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
    mxLbColorFormat->show();

    mxLbDataBarMinType->show();
    mxLbDataBarMaxType->show();
    mxEdDataBarMin->show();
    mxEdDataBarMax->show();
    mxBtOptions->show();

    Select();
}

void ScDataBarFrmtEntry::SetInactive()
{
    mxLbColorFormat->hide();

    mxLbDataBarMinType->hide();
    mxLbDataBarMaxType->hide();
    mxEdDataBarMin->hide();
    mxEdDataBarMax->hide();
    mxBtOptions->hide();

    Deselect();
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, DataBarTypeSelectHdl, weld::ComboBox&, void )
{
    if (getSelectedType(*mxLbDataBarMinType) <= COLORSCALE_MAX)
        mxEdDataBarMin->set_sensitive(false);
    else
        mxEdDataBarMin->set_sensitive(true);

    if (getSelectedType(*mxLbDataBarMaxType) <= COLORSCALE_MAX)
        mxEdDataBarMax->set_sensitive(false);
    else
        mxEdDataBarMax->set_sensitive(true);
}

IMPL_LINK_NOARG( ScDataBarFrmtEntry, OptionBtnHdl, weld::Button&, void )
{
    SetColorScaleEntry(mpDataBarData->mpLowerLimit.get(), *mxLbDataBarMinType,
                       *mxEdDataBarMin, mpDoc, maPos);
    SetColorScaleEntry(mpDataBarData->mpUpperLimit.get(), *mxLbDataBarMaxType,
                       *mxEdDataBarMax, mpDoc, maPos);
    ScDataBarSettingsDlg aDlg(mpParent->GetFrameWeld(), *mpDataBarData, mpDoc, maPos);
    if (aDlg.run() == RET_OK)
    {
        mpDataBarData.reset(aDlg.GetData());
        SetDataBarEntryTypes(*mpDataBarData->mpLowerLimit, *mxLbDataBarMinType, *mxEdDataBarMin, mpDoc);
        SetDataBarEntryTypes(*mpDataBarData->mpUpperLimit, *mxLbDataBarMaxType, *mxEdDataBarMax, mpDoc);
        DataBarTypeSelectHdl(*mxLbDataBarMinType);
    }
}

ScDateFrmtEntry::ScDateFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScCondDateFormatEntry* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, ScAddress())
    , mxLbDateEntry(mxBuilder->weld_combo_box("datetype"))
    , mxFtStyle(mxBuilder->weld_label("styleft"))
    , mxLbStyle(mxBuilder->weld_combo_box("style"))
    , mxWdPreviewWin(mxBuilder->weld_widget("previewwin"))
    , mxWdPreview(new weld::CustomWeld(*mxBuilder, "preview", maWdPreview))
    , mbIsInStyleCreate(false)
{
    mxLbDateEntry->set_size_request(CommonWidgetWidth, -1);
    mxLbStyle->set_size_request(CommonWidgetWidth, -1);

    mxWdPreview->set_size_request(mxLbStyle->get_preferred_size().Height(), -1);

    Init();

    StartListening(*pDoc->GetStyleSheetPool(), DuplicateHandling::Prevent);

    if(pFormat)
    {
        sal_Int32 nPos = static_cast<sal_Int32>(pFormat->GetDateType());
        mxLbDateEntry->set_active(nPos);

        mxLbStyle->set_active_text(pFormat->GetStyleName());
    }

    StyleSelectHdl(*mxLbStyle);
}

ScDateFrmtEntry::~ScDateFrmtEntry()
{
}

void ScDateFrmtEntry::Init()
{
    mxLbDateEntry->set_active(0);
    mxLbType->set_active(3);

    FillStyleListBox( mpDoc, *mxLbStyle );
    mxLbStyle->connect_changed( LINK( this, ScDateFrmtEntry, StyleSelectHdl ) );
    mxLbStyle->set_active(1);
}

void ScDateFrmtEntry::SetActive()
{
    mxLbDateEntry->show();
    mxFtStyle->show();
    mxWdPreviewWin->show();
    mxLbStyle->show();

    Select();
}

void ScDateFrmtEntry::SetInactive()
{
    mxLbDateEntry->hide();
    mxFtStyle->hide();
    mxWdPreviewWin->hide();
    mxLbStyle->hide();

    Deselect();
}

void ScDateFrmtEntry::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if(rHint.GetId() == SfxHintId::StyleSheetModified)
    {
        if(!mbIsInStyleCreate)
            UpdateStyleList(*mxLbStyle, mpDoc);
    }
}

ScFormatEntry* ScDateFrmtEntry::GetEntry() const
{
    ScCondDateFormatEntry* pNewEntry = new ScCondDateFormatEntry(mpDoc);
    condformat::ScCondFormatDateType eType = static_cast<condformat::ScCondFormatDateType>(mxLbDateEntry->get_active());
    pNewEntry->SetDateType(eType);
    pNewEntry->SetStyleName(mxLbStyle->get_active_text());
    return pNewEntry;
}

OUString ScDateFrmtEntry::GetExpressionString()
{
    // tdf#124412 - set actual condition for an inactive date entry
    return ScCondFormatHelper::GetExpression(DATE, mxLbDateEntry->get_active());
}

IMPL_LINK_NOARG( ScDateFrmtEntry, StyleSelectHdl, weld::ComboBox&, void )
{
    mbIsInStyleCreate = true;
    StyleSelect(mpParent->GetFrameWeld(), *mxLbStyle, mpDoc, maWdPreview);
    mbIsInStyleCreate = false;
}

class ScIconSetFrmtDataEntry
{
protected:
    std::unique_ptr<weld::Builder> mxBuilder;
private:
    std::unique_ptr<weld::Container> mxGrid;
    std::unique_ptr<weld::Image> mxImgIcon;
    std::unique_ptr<weld::Label> mxFtEntry;
    std::unique_ptr<weld::Entry> mxEdEntry;
    std::unique_ptr<weld::ComboBox> mxLbEntryType;
    weld::Container* mpContainer;

public:
    ScIconSetFrmtDataEntry(weld::Container* pParent, ScIconSetType eType, const ScDocument* pDoc,
            sal_Int32 i, const ScColorScaleEntry* pEntry = nullptr);
    ~ScIconSetFrmtDataEntry();
    void Show() { mxGrid->show(); }
    void Hide() { mxGrid->hide(); }
    void set_grid_top_attach(int nTop)
    {
        mxGrid->set_grid_left_attach(0);
        mxGrid->set_grid_top_attach(nTop);
    }

    ScColorScaleEntry* CreateEntry(ScDocument& rDoc, const ScAddress& rPos) const;

    void SetFirstEntry();
};

ScIconSetFrmtDataEntry::ScIconSetFrmtDataEntry(weld::Container* pParent, ScIconSetType eType, const ScDocument* pDoc, sal_Int32 i, const ScColorScaleEntry* pEntry)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/conditionaliconset.ui"))
    , mxGrid(mxBuilder->weld_container("ConditionalIconSet"))
    , mxImgIcon(mxBuilder->weld_image("icon"))
    , mxFtEntry(mxBuilder->weld_label("label"))
    , mxEdEntry(mxBuilder->weld_entry("entry"))
    , mxLbEntryType(mxBuilder->weld_combo_box("listbox"))
    , mpContainer(pParent)
{
    mxImgIcon->set_from_icon_name(ScIconSetFormat::getIconName(eType, i));
    if(pEntry)
    {
        switch(pEntry->GetType())
        {
            case COLORSCALE_VALUE:
                mxLbEntryType->set_active(0);
                mxEdEntry->set_text(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_PERCENTILE:
                mxLbEntryType->set_active(2);
                mxEdEntry->set_text(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_PERCENT:
                mxLbEntryType->set_active(1);
                mxEdEntry->set_text(convertNumberToString(pEntry->GetValue(), pDoc));
                break;
            case COLORSCALE_FORMULA:
                mxLbEntryType->set_active(3);
                mxEdEntry->set_text(pEntry->GetFormula(formula::FormulaGrammar::GRAM_DEFAULT));
                break;
            default:
                assert(false);
        }
    }
    else
    {
        mxLbEntryType->set_active(1);
    }
}

ScIconSetFrmtDataEntry::~ScIconSetFrmtDataEntry()
{
    mpContainer->move(mxGrid.get(), nullptr);
}

ScColorScaleEntry* ScIconSetFrmtDataEntry::CreateEntry(ScDocument& rDoc, const ScAddress& rPos) const
{
    sal_Int32 nPos = mxLbEntryType->get_active();
    OUString aText = mxEdEntry->get_text();
    ScColorScaleEntry* pEntry = new ScColorScaleEntry();

    sal_uInt32 nIndex = 0;
    double nVal = 0;
    SvNumberFormatter* pNumberFormatter = rDoc.GetFormatTable();
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
            pEntry->SetFormula(aText, rDoc, rPos, rDoc.GetGrammar());
            break;
        default:
            assert(false);
    }

    return pEntry;
}

void ScIconSetFrmtDataEntry::SetFirstEntry()
{
    mxEdEntry->hide();
    mxLbEntryType->hide();
    mxFtEntry->hide();
    mxEdEntry->set_text("0");
    mxLbEntryType->set_active(1);
}

ScIconSetFrmtEntry::ScIconSetFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScIconSetFormat* pFormat)
    : ScCondFrmtEntry(pParent, pDoc, rPos)
    , mxLbColorFormat(mxBuilder->weld_combo_box("colorformat"))
    , mxLbIconSetType(mxBuilder->weld_combo_box("iconsettype"))
    , mxIconParent(mxBuilder->weld_container("iconparent"))
{
    mxLbColorFormat->set_size_request(CommonWidgetWidth, -1);
    mxLbIconSetType->set_size_request(CommonWidgetWidth, -1);

    Init();
    mxLbColorFormat->connect_changed(LINK(pParent, ScCondFormatList, ColFormatTypeHdl));

    if(pFormat)
    {
        const ScIconSetFormatData* pIconSetFormatData = pFormat->GetIconSetData();
        ScIconSetType eType = pIconSetFormatData->eIconSetType;
        sal_Int32 nType = static_cast<sal_Int32>(eType);
        mxLbIconSetType->set_active(nType);

        for (size_t i = 0, n = pIconSetFormatData->m_Entries.size();
                i < n; ++i)
        {
            maEntries.emplace_back(new ScIconSetFrmtDataEntry(
                mxIconParent.get(), eType, pDoc, i, pIconSetFormatData->m_Entries[i].get()));
            maEntries[i]->set_grid_top_attach(i);
        }
        maEntries[0]->SetFirstEntry();
    }
    else
        IconSetTypeHdl(*mxLbIconSetType);
}

ScIconSetFrmtEntry::~ScIconSetFrmtEntry()
{
}

void ScIconSetFrmtEntry::Init()
{
    mxLbColorFormat->set_active(3);
    mxLbType->set_active(0);
    mxLbIconSetType->set_active(0);

    mxLbIconSetType->connect_changed(LINK(this, ScIconSetFrmtEntry, IconSetTypeHdl));
}

IMPL_LINK_NOARG( ScIconSetFrmtEntry, IconSetTypeHdl, weld::ComboBox&, void )
{
    const ScIconSetMap* pMap = ScIconSetFormat::g_IconSetMap;

    sal_Int32 nPos = mxLbIconSetType->get_active();
    sal_uInt32 nElements = pMap[nPos].nElements;

    maEntries.clear();

    for(size_t i = 0; i < nElements; ++i)
    {
        maEntries.emplace_back(new ScIconSetFrmtDataEntry(mxIconParent.get(), static_cast<ScIconSetType>(nPos), mpDoc, i));
        maEntries[i]->set_grid_top_attach(i);
        maEntries[i]->Show();
    }
    maEntries[0]->SetFirstEntry();
}

OUString ScIconSetFrmtEntry::GetExpressionString()
{
    return ScCondFormatHelper::GetExpression(ICONSET, 0);
}

void ScIconSetFrmtEntry::SetActive()
{
    mxLbColorFormat->show();
    mxLbIconSetType->show();
    for(auto& rxEntry : maEntries)
    {
        rxEntry->Show();
    }

    Select();
}

void ScIconSetFrmtEntry::SetInactive()
{
    mxLbColorFormat->hide();
    mxLbIconSetType->hide();
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
    pData->eIconSetType = static_cast<ScIconSetType>(mxLbIconSetType->get_active());
    for(const auto& rxEntry : maEntries)
    {
        pData->m_Entries.emplace_back(rxEntry->CreateEntry(*mpDoc, maPos));
    }
    pFormat->SetIconSetData(pData);

    return pFormat;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
