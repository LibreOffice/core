/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif

#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <comphelper/string.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/app.hxx>
#include <o3tl/string_view.hxx>

#include <scresid.hxx>
#include <strings.hrc>

#include <stringutil.hxx>
#include <validat.hxx>
#include <validate.hxx>
#include <compiler.hxx>
#include <formula/opcode.hxx>

// cell range picker
#include <tabvwsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/childwin.hxx>
#include <reffact.hxx>
#include <comphelper/lok.hxx>

/*  Position indexes for "Allow" list box.
    They do not map directly to ScValidationMode and can safely be modified to
    change the order of the list box entries. */
#define SC_VALIDDLG_ALLOW_ANY       0
#define SC_VALIDDLG_ALLOW_WHOLE     1
#define SC_VALIDDLG_ALLOW_DECIMAL   2
#define SC_VALIDDLG_ALLOW_DATE      3
#define SC_VALIDDLG_ALLOW_TIME      4
#define SC_VALIDDLG_ALLOW_RANGE     5
#define SC_VALIDDLG_ALLOW_LIST      6
#define SC_VALIDDLG_ALLOW_TEXTLEN   7
#define SC_VALIDDLG_ALLOW_CUSTOM    8

/*  Position indexes for "Data" list box.
    They do not map directly to ScConditionMode and can safely be modified to
    change the order of the list box entries. */
#define SC_VALIDDLG_DATA_EQUAL        0
#define SC_VALIDDLG_DATA_LESS         1
#define SC_VALIDDLG_DATA_GREATER      2
#define SC_VALIDDLG_DATA_EQLESS       3
#define SC_VALIDDLG_DATA_EQGREATER    4
#define SC_VALIDDLG_DATA_NOTEQUAL     5
#define SC_VALIDDLG_DATA_VALIDRANGE   6
#define SC_VALIDDLG_DATA_INVALIDRANGE 7
#define SC_VALIDDLG_DATA_DIRECT       8

namespace ValidListType = css::sheet::TableValidationVisibility;

const WhichRangesContainer ScTPValidationValue::pValueRanges(svl::Items<
    FID_VALID_LISTTYPE, FID_VALID_LISTTYPE,
    FID_VALID_MODE, FID_VALID_ERRTEXT
>);

static bool isLOKMobilePhone()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return false;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    return pViewShell && pViewShell->isLOKMobilePhone();
}

ScValidationDlg::ScValidationDlg(weld::Window* pParent, const SfxItemSet* pArgSet,
    ScTabViewShell *pTabViewSh)
    : ScValidationDlgBase(pParent,
        "modules/scalc/ui/validationdialog.ui", "ValidationDialog", pArgSet, nullptr)
    , m_pTabVwSh(pTabViewSh)
    , m_sValuePageId("criteria")
    , m_bOwnRefHdlr(false)
    , m_bRefInputting(false)
    , m_xHBox(m_xBuilder->weld_container("refinputbox"))
{
    AddTabPage(m_sValuePageId, ScTPValidationValue::Create, nullptr);
    AddTabPage("inputhelp", ScTPValidationHelp::Create, nullptr);
    AddTabPage("erroralert", ScTPValidationError::Create, nullptr);

    if (isLOKMobilePhone())
    {
        m_xBuilder->weld_button("cancel")->hide();
        m_xBuilder->weld_button("help")->hide();
    }
}

void ScValidationDlg::EndDialog(int nResponse)
{
    // tdf#155708 - do not close, just hide validation window if we click in another sheet
    if (nResponse == nCloseResponseToJustHide && getDialog()->get_visible())
    {
        getDialog()->hide();
        return;
    }
    // tdf#137215 ensure original modality of true is restored before dialog loop ends
    if (m_bOwnRefHdlr)
        RemoveRefDlg(true);
    ScValidationDlgBase::EndDialog(nResponse);
}

ScValidationDlg::~ScValidationDlg()
{
    if (m_bOwnRefHdlr)
        RemoveRefDlg(false);
}

void ScTPValidationValue::SetReferenceHdl( const ScRange&rRange , const ScDocument& rDoc )
{
    if ( rRange.aStart != rRange.aEnd )
        if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
            if( m_pRefEdit )
                pValidationDlg->RefInputStart( m_pRefEdit );

    if ( m_pRefEdit )
    {
        OUString aStr(rRange.Format(rDoc, ScRefFlags::RANGE_ABS_3D, rDoc.GetAddressConvention()));
        m_pRefEdit->SetRefString( aStr );
    }
}

void ScTPValidationValue:: SetActiveHdl()
{
    if ( m_pRefEdit ) m_pRefEdit->GrabFocus();

    if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
        if( m_pRefEdit )
        {
            pValidationDlg->RefInputDone();
        }
}

void ScTPValidationValue::RefInputStartPreHdl( formula::RefEdit* pEdit, const formula::RefButton* pButton )
{
    ScValidationDlg *pValidationDlg = GetValidationDlg();
    if (!pValidationDlg)
        return;

    weld::Container* pNewParent = pValidationDlg->get_refinput_shrink_parent();
    if (pEdit == m_pRefEdit && pNewParent != m_pRefEditParent)
    {
        m_xRefGrid->move(m_pRefEdit->GetWidget(), pNewParent);
        m_pRefEditParent = pNewParent;
    }

    if (pNewParent != m_pBtnRefParent)
    {
        // if Edit SetParent but button not, the tab order will be
        // incorrect, so move button anyway, and restore
        // parent later in order to restore the tab order. But
        // hide it if it's moved but unwanted.
        m_xRefGrid->move(m_xBtnRef->GetWidget(), pNewParent);
        m_xBtnRef->GetWidget()->set_visible(pButton == m_xBtnRef.get());
        m_pBtnRefParent = pNewParent;
    }

    pNewParent->show();
}

void ScTPValidationValue::RefInputDonePostHdl()
{
    if (ScValidationDlg *pValidationDlg = GetValidationDlg())
    {
        weld::Container* pOldParent = pValidationDlg->get_refinput_shrink_parent();

        if (m_pRefEdit && m_pRefEditParent != m_xRefGrid.get())
        {
            pOldParent->move(m_pRefEdit->GetWidget(), m_xRefGrid.get());
            m_pRefEditParent = m_xRefGrid.get();
        }

        if (m_pBtnRefParent != m_xRefGrid.get())
        {
            pOldParent->move(m_xBtnRef->GetWidget(), m_xRefGrid.get());
            m_xBtnRef->GetWidget()->show();
            m_pBtnRefParent = m_xRefGrid.get();
        }

        pOldParent->hide();
        ScViewData& rViewData = pValidationDlg->GetTabViewShell()->GetViewData();
        SCTAB nCurTab = rViewData.GetTabNo();
        SCTAB nRefTab = rViewData.GetRefTabNo();
        // If RefInput switched to a different sheet from the data sheet,
        // switch back: fdo#53920
        if ( nCurTab != nRefTab )
        {
             rViewData.GetViewShell()->SetTabNo( nRefTab );
        }
    }

    if (m_pRefEdit && !m_pRefEdit->GetWidget()->has_focus())
        m_pRefEdit->GrabFocus();
}

namespace {

/** Converts the passed ScValidationMode to the position in the list box. */
sal_uInt16 lclGetPosFromValMode( ScValidationMode eValMode )
{
    sal_uInt16 nLbPos = SC_VALIDDLG_ALLOW_ANY;
    switch( eValMode )
    {
        case SC_VALID_ANY:      nLbPos = SC_VALIDDLG_ALLOW_ANY;     break;
        case SC_VALID_WHOLE:    nLbPos = SC_VALIDDLG_ALLOW_WHOLE;   break;
        case SC_VALID_DECIMAL:  nLbPos = SC_VALIDDLG_ALLOW_DECIMAL; break;
        case SC_VALID_DATE:     nLbPos = SC_VALIDDLG_ALLOW_DATE;    break;
        case SC_VALID_TIME:     nLbPos = SC_VALIDDLG_ALLOW_TIME;    break;
        case SC_VALID_TEXTLEN:  nLbPos = SC_VALIDDLG_ALLOW_TEXTLEN; break;
        case SC_VALID_LIST:     nLbPos = SC_VALIDDLG_ALLOW_RANGE;   break;
        case SC_VALID_CUSTOM:   nLbPos = SC_VALIDDLG_ALLOW_CUSTOM;  break;
        default:    OSL_FAIL( "lclGetPosFromValMode - unknown validity mode" );
    }
    return nLbPos;
}

/** Converts the passed list box position to an ScValidationMode. */
ScValidationMode lclGetValModeFromPos( sal_uInt16 nLbPos )
{
    ScValidationMode eValMode = SC_VALID_ANY;
    switch( nLbPos )
    {
        case SC_VALIDDLG_ALLOW_ANY:     eValMode = SC_VALID_ANY;        break;
        case SC_VALIDDLG_ALLOW_WHOLE:   eValMode = SC_VALID_WHOLE;      break;
        case SC_VALIDDLG_ALLOW_DECIMAL: eValMode = SC_VALID_DECIMAL;    break;
        case SC_VALIDDLG_ALLOW_DATE:    eValMode = SC_VALID_DATE;       break;
        case SC_VALIDDLG_ALLOW_TIME:    eValMode = SC_VALID_TIME;       break;
        case SC_VALIDDLG_ALLOW_RANGE:   eValMode = SC_VALID_LIST;       break;
        case SC_VALIDDLG_ALLOW_LIST:    eValMode = SC_VALID_LIST;       break;
        case SC_VALIDDLG_ALLOW_TEXTLEN: eValMode = SC_VALID_TEXTLEN;    break;
        case SC_VALIDDLG_ALLOW_CUSTOM:  eValMode = SC_VALID_CUSTOM;     break;
        default:    OSL_FAIL( "lclGetValModeFromPos - invalid list box position" );
    }
    return eValMode;
}

/** Converts the passed ScConditionMode to the position in the list box. */
sal_uInt16 lclGetPosFromCondMode( ScConditionMode eCondMode )
{
    sal_uInt16 nLbPos = SC_VALIDDLG_DATA_EQUAL;
    switch( eCondMode )
    {
        case ScConditionMode::NONE:          // may occur in old XML files after Excel import
        case ScConditionMode::Equal:         nLbPos = SC_VALIDDLG_DATA_EQUAL;        break;
        case ScConditionMode::Less:          nLbPos = SC_VALIDDLG_DATA_LESS;         break;
        case ScConditionMode::Greater:       nLbPos = SC_VALIDDLG_DATA_GREATER;      break;
        case ScConditionMode::EqLess:        nLbPos = SC_VALIDDLG_DATA_EQLESS;       break;
        case ScConditionMode::EqGreater:     nLbPos = SC_VALIDDLG_DATA_EQGREATER;    break;
        case ScConditionMode::NotEqual:      nLbPos = SC_VALIDDLG_DATA_NOTEQUAL;     break;
        case ScConditionMode::Between:       nLbPos = SC_VALIDDLG_DATA_VALIDRANGE;      break;
        case ScConditionMode::NotBetween:    nLbPos = SC_VALIDDLG_DATA_INVALIDRANGE;   break;
        case ScConditionMode::Direct:        nLbPos = SC_VALIDDLG_DATA_DIRECT;         break;
        default:    OSL_FAIL( "lclGetPosFromCondMode - unknown condition mode" );
    }
    return nLbPos;
}

/** Converts the passed list box position to an ScConditionMode. */
ScConditionMode lclGetCondModeFromPos( sal_uInt16 nLbPos )
{
    ScConditionMode eCondMode = ScConditionMode::Equal;
    switch( nLbPos )
    {
        case SC_VALIDDLG_DATA_EQUAL:        eCondMode = ScConditionMode::Equal;      break;
        case SC_VALIDDLG_DATA_LESS:         eCondMode = ScConditionMode::Less;       break;
        case SC_VALIDDLG_DATA_GREATER:      eCondMode = ScConditionMode::Greater;    break;
        case SC_VALIDDLG_DATA_EQLESS:       eCondMode = ScConditionMode::EqLess;     break;
        case SC_VALIDDLG_DATA_EQGREATER:    eCondMode = ScConditionMode::EqGreater;  break;
        case SC_VALIDDLG_DATA_NOTEQUAL:     eCondMode = ScConditionMode::NotEqual;   break;
        case SC_VALIDDLG_DATA_VALIDRANGE:      eCondMode = ScConditionMode::Between;    break;
        case SC_VALIDDLG_DATA_INVALIDRANGE:   eCondMode = ScConditionMode::NotBetween; break;
        case SC_VALIDDLG_DATA_DIRECT:         eCondMode = ScConditionMode::Direct;   break;
        default:    OSL_FAIL( "lclGetCondModeFromPos - invalid list box position" );
    }
    return eCondMode;
}

/** Converts line feed separated string to a formula with strings separated by semicolons.
    @descr  Keeps all empty strings.
    Example: abc\ndef\n\nghi -> "abc";"def";"";"ghi".
    @param rFmlaStr  (out-param) The converted formula string. */
void lclGetFormulaFromStringList( OUString& rFmlaStr, std::u16string_view rStringList, sal_Unicode cFmlaSep )
{
    rFmlaStr.clear();
    if (!rStringList.empty())
    {
        sal_Int32 nIdx {0};
        do
        {
            OUString aToken {o3tl::getToken(rStringList, 0, '\n', nIdx )};
            ScGlobal::AddQuotes( aToken, '"' );
            rFmlaStr = ScGlobal::addToken(rFmlaStr, aToken, cFmlaSep);
        }
        while (nIdx>0);
    }
    if( rFmlaStr.isEmpty() )
        rFmlaStr = "\"\"";
}

/** Converts formula with strings separated by semicolons to line feed separated string.
    @descr  Keeps all empty strings. Ignores all empty tokens (multiple semicolons).
    Example: "abc";;;"def";"";"ghi" -> abc\ndef\n\nghi.
    @param rStringList  (out-param) The converted line feed separated string list.
    @return  true = Conversion successful. */
bool lclGetStringListFromFormula( OUString& rStringList, const OUString& rFmlaStr, sal_Unicode cFmlaSep )
{
    static constexpr OUStringLiteral aQuotes( u"\"\"" );

    rStringList.clear();
    bool bIsStringList = !rFmlaStr.isEmpty();
    bool bTokenAdded = false;

    for ( sal_Int32 nStringIx = 0; bIsStringList && nStringIx>=0; )
    {
        OUString aToken( ScStringUtil::GetQuotedToken(rFmlaStr, 0, aQuotes, cFmlaSep, nStringIx ) );
        aToken = comphelper::string::strip(aToken, ' ');
        if( !aToken.isEmpty() )      // ignore empty tokens, i.e. "a";;"b"
        {
            bIsStringList = ScGlobal::IsQuoted( aToken, '"' );
            if( bIsStringList )
            {
                ScGlobal::EraseQuotes( aToken, '"' );
                rStringList = ScGlobal::addToken(rStringList, aToken, '\n', 1, bTokenAdded);
                bTokenAdded = true;
            }
        }
    }

    return bIsStringList;
}

} // namespace

ScTPValidationValue::ScTPValidationValue(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/validationcriteriapage.ui",
                 "ValidationCriteriaPage", &rArgSet)
    , maStrMin(ScResId(SCSTR_VALID_MINIMUM))
    , maStrMax(ScResId(SCSTR_VALID_MAXIMUM))
    , maStrValue(ScResId(SCSTR_VALID_VALUE))
    , maStrFormula(ScResId(SCSTR_VALID_FORMULA))
    , maStrRange(ScResId(SCSTR_VALID_RANGE))
    , maStrList(ScResId(SCSTR_VALID_LIST))
    , m_pRefEdit(nullptr)
    , m_xLbAllow(m_xBuilder->weld_combo_box("allow"))
    , m_xCbAllow(m_xBuilder->weld_check_button("allowempty"))
    , m_xCbCaseSens(m_xBuilder->weld_check_button("casesens"))
    , m_xCbShow(m_xBuilder->weld_check_button("showlist"))
    , m_xCbSort(m_xBuilder->weld_check_button("sortascend"))
    , m_xFtValue(m_xBuilder->weld_label("valueft"))
    , m_xLbValue(m_xBuilder->weld_combo_box("data"))
    , m_xFtMin(m_xBuilder->weld_label("minft"))
    , m_xMinGrid(m_xBuilder->weld_widget("mingrid"))
    , m_xEdMin(new formula::RefEdit(m_xBuilder->weld_entry("min")))
    , m_xEdList(m_xBuilder->weld_text_view("minlist"))
    , m_xFtMax(m_xBuilder->weld_label("maxft"))
    , m_xEdMax(new formula::RefEdit(m_xBuilder->weld_entry("max")))
    , m_xFtHint(m_xBuilder->weld_label("hintft"))
    , m_xBtnRef(new formula::RefButton(m_xBuilder->weld_button("validref")))
    , m_xRefGrid(m_xBuilder->weld_container("refgrid"))
    , m_pRefEditParent(m_xRefGrid.get())
    , m_pBtnRefParent(m_xRefGrid.get())
{
    m_xEdMin->SetReferences(nullptr, m_xFtMin.get());

    Size aSize(m_xEdList->get_approximate_digit_width() * 40,
               m_xEdList->get_height_rows(10));
    m_xEdList->set_size_request(aSize.Width(), aSize.Height());
    m_xEdMax->SetReferences(nullptr, m_xFtMax.get());

    m_xBtnRef->SetClickHdl(LINK(this, ScTPValidationValue, ClickHdl));

    //lock in the max size initial config
    aSize = m_xContainer->get_preferred_size();
    m_xContainer->set_size_request(aSize.Width(), aSize.Height());

    Init();

    // list separator in formulas
    OUString aListSep = ::ScCompiler::GetNativeSymbol( ocSep );
    OSL_ENSURE( aListSep.getLength() == 1, "ScTPValidationValue::ScTPValidationValue - list separator error" );
    mcFmlaSep = aListSep.getLength() ? aListSep[0] : ';';
    m_xBtnRef->GetWidget()->hide(); // cell range picker
}

ScTPValidationValue::~ScTPValidationValue()
{
    m_xEdMin.reset();
    m_xEdMax.reset();
    m_xBtnRef.reset();
}

void ScTPValidationValue::Init()
{
    m_xLbAllow->connect_changed( LINK( this, ScTPValidationValue, SelectHdl ) );
    m_xLbValue->connect_changed( LINK( this, ScTPValidationValue, SelectHdl ) );
    m_xCbShow->connect_toggled( LINK( this, ScTPValidationValue, CheckHdl ) );

    // cell range picker
    m_xEdMin->SetGetFocusHdl( LINK( this, ScTPValidationValue, EditSetFocusHdl ) );
    m_xEdMin->SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillEditFocusHdl ) );
    m_xEdMax->SetGetFocusHdl( LINK( this, ScTPValidationValue, EditSetFocusHdl ) );
    m_xEdMax->SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillEditFocusHdl ) );
    m_xBtnRef->SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillButtonFocusHdl ) );

    m_xLbAllow->set_active( SC_VALIDDLG_ALLOW_ANY );
    m_xLbValue->set_active( SC_VALIDDLG_DATA_EQUAL );

    SelectHdl( *m_xLbAllow );
    CheckHdl( *m_xCbShow );
}

std::unique_ptr<SfxTabPage> ScTPValidationValue::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rArgSet)
{
    return std::make_unique<ScTPValidationValue>(pPage, pController, *rArgSet);
}

void ScTPValidationValue::Reset( const SfxItemSet* rArgSet )
{
    sal_uInt16 nLbPos = SC_VALIDDLG_ALLOW_ANY;
    if( const SfxUInt16Item* pItem = rArgSet->GetItemIfSet( FID_VALID_MODE ) )
        nLbPos = lclGetPosFromValMode( static_cast< ScValidationMode >( pItem->GetValue() ) );
    m_xLbAllow->set_active( nLbPos );

    nLbPos = SC_VALIDDLG_DATA_EQUAL;
    if( const SfxUInt16Item* pItem = rArgSet->GetItemIfSet( FID_VALID_CONDMODE ) )
        nLbPos = lclGetPosFromCondMode( static_cast< ScConditionMode >( pItem->GetValue() ) );
    m_xLbValue->set_active( nLbPos );

    // *** check boxes ***
    bool bCheck = true;
    if( const SfxBoolItem* pItem = rArgSet->GetItemIfSet( FID_VALID_BLANK ) )
        bCheck = pItem->GetValue();
    m_xCbAllow->set_active( bCheck );

    bool bCaseSensetive = false;
    if (const SfxBoolItem* pItem = rArgSet->GetItemIfSet( FID_VALID_CASESENS ) )
        bCaseSensetive = pItem->GetValue();
    m_xCbCaseSens->set_active( bCaseSensetive );

    sal_Int32 nListType = ValidListType::UNSORTED;
    if( const SfxInt16Item* pItem = rArgSet->GetItemIfSet( FID_VALID_LISTTYPE ) )
        nListType = pItem->GetValue();
    m_xCbShow->set_active( nListType != ValidListType::INVISIBLE );
    m_xCbSort->set_active( nListType == ValidListType::SORTEDASCENDING );

    // *** formulas ***
    OUString aFmlaStr;
    if ( const SfxStringItem* pItem = rArgSet->GetItemIfSet( FID_VALID_VALUE1 ) )
        aFmlaStr = pItem->GetValue();
    SetFirstFormula( aFmlaStr );

    aFmlaStr.clear();
    if ( const SfxStringItem* pItem = rArgSet->GetItemIfSet( FID_VALID_VALUE2 ) )
        aFmlaStr = pItem->GetValue();
    SetSecondFormula( aFmlaStr );

    SelectHdl( *m_xLbAllow );
    CheckHdl( *m_xCbShow );
}

bool ScTPValidationValue::FillItemSet( SfxItemSet* rArgSet )
{
    sal_Int16 nListType = m_xCbShow->get_active() ?
        (m_xCbSort->get_active() ? ValidListType::SORTEDASCENDING : ValidListType::UNSORTED) :
        ValidListType::INVISIBLE;

    const sal_Int32 nLbPos = m_xLbAllow->get_active();
    bool bCustom = (nLbPos == SC_VALIDDLG_ALLOW_CUSTOM);
    ScConditionMode eCondMode = bCustom ?
            ScConditionMode::Direct : lclGetCondModeFromPos( m_xLbValue->get_active() );

    rArgSet->Put( SfxUInt16Item( FID_VALID_MODE, sal::static_int_cast<sal_uInt16>(
                    lclGetValModeFromPos( nLbPos ) ) ) );
    rArgSet->Put( SfxUInt16Item( FID_VALID_CONDMODE, sal::static_int_cast<sal_uInt16>( eCondMode ) ) );
    rArgSet->Put( SfxStringItem( FID_VALID_VALUE1, GetFirstFormula() ) );
    rArgSet->Put( SfxStringItem( FID_VALID_VALUE2, GetSecondFormula() ) );
    rArgSet->Put( SfxBoolItem( FID_VALID_BLANK, m_xCbAllow->get_active() ) );
    rArgSet->Put( SfxBoolItem( FID_VALID_CASESENS, m_xCbCaseSens->get_active() ) );
    rArgSet->Put( SfxInt16Item( FID_VALID_LISTTYPE, nListType ) );
    return true;
}

OUString ScTPValidationValue::GetFirstFormula() const
{
    OUString aFmlaStr;
    if( m_xLbAllow->get_active() == SC_VALIDDLG_ALLOW_LIST )
        lclGetFormulaFromStringList( aFmlaStr, m_xEdList->get_text(), mcFmlaSep );
    else
        aFmlaStr = m_xEdMin->GetText();
    return aFmlaStr;
}

OUString ScTPValidationValue::GetSecondFormula() const
{
    return m_xEdMax->GetText();
}

void ScTPValidationValue::SetFirstFormula( const OUString& rFmlaStr )
{
    // try if formula is a string list, validation mode must already be set
    OUString aStringList;
    if( (m_xLbAllow->get_active() == SC_VALIDDLG_ALLOW_RANGE) &&
        lclGetStringListFromFormula( aStringList, rFmlaStr, mcFmlaSep ) )
    {
        m_xEdList->set_text( aStringList );
        m_xEdMin->SetText( OUString() );
        // change validation mode to string list
        m_xLbAllow->set_active( SC_VALIDDLG_ALLOW_LIST );
    }
    else
    {
        m_xEdMin->SetText( rFmlaStr );
        m_xEdList->set_text( OUString() );
    }
}

void ScTPValidationValue::SetSecondFormula( const OUString& rFmlaStr )
{
    m_xEdMax->SetText( rFmlaStr );
}

ScValidationDlg * ScTPValidationValue::GetValidationDlg()
{
    return dynamic_cast<ScValidationDlg*>(GetDialogController());
}

void ScTPValidationValue::SetupRefDlg()
{
    ScValidationDlg *pValidationDlg = GetValidationDlg();
    if( !pValidationDlg )
        return;

    if( !pValidationDlg->SetupRefDlg() )
        return;

    pValidationDlg->SetHandler( this );
    pValidationDlg->SetSetRefHdl( static_cast<ScRefHandlerHelper::PFUNCSETREFHDLTYPE>( &ScTPValidationValue::SetReferenceHdl ) );
    pValidationDlg->SetSetActHdl( static_cast<ScRefHandlerHelper::PCOMMONHDLTYPE>( &ScTPValidationValue::SetActiveHdl ) );
    pValidationDlg->SetRefInputStartPreHdl( static_cast<ScRefHandlerHelper::PINPUTSTARTDLTYPE>( &ScTPValidationValue::RefInputStartPreHdl ) );
    pValidationDlg->SetRefInputDonePostHdl( static_cast<ScRefHandlerHelper::PCOMMONHDLTYPE>( &ScTPValidationValue::RefInputDonePostHdl ) );

    weld::Label* pLabel = nullptr;

    if (m_xEdMax->GetWidget()->get_visible())
    {
        m_pRefEdit = m_xEdMax.get();
        pLabel = m_xFtMax.get();
    }
    else if (m_xEdMin->GetWidget()->get_visible())
    {
        m_pRefEdit = m_xEdMin.get();
        pLabel = m_xFtMin.get();
    }

    if (m_pRefEdit && !m_pRefEdit->GetWidget()->has_focus())
        m_pRefEdit->GrabFocus();

    if( m_pRefEdit )
        m_pRefEdit->SetReferences( pValidationDlg, pLabel );

    m_xBtnRef->SetReferences( pValidationDlg, m_pRefEdit );
}

void ScTPValidationValue::RemoveRefDlg(bool bRestoreModal)
{
    ScValidationDlg *pValidationDlg = GetValidationDlg();
    if( !pValidationDlg )
        return;

    if( !pValidationDlg->RemoveRefDlg(bRestoreModal) )
        return;

    pValidationDlg->SetHandler( nullptr );
    pValidationDlg->SetSetRefHdl( nullptr );
    pValidationDlg->SetSetActHdl( nullptr );
    pValidationDlg->SetRefInputStartPreHdl( nullptr );
    pValidationDlg->SetRefInputDonePostHdl( nullptr );

    if( m_pRefEdit )
        m_pRefEdit->SetReferences( nullptr, nullptr );
    m_pRefEdit = nullptr;

    m_xBtnRef->SetReferences( nullptr, nullptr );
}

IMPL_LINK_NOARG(ScTPValidationValue, EditSetFocusHdl, formula::RefEdit&, void)
{
    const sal_Int32 nPos = m_xLbAllow->get_active();

    if ( nPos == SC_VALIDDLG_ALLOW_RANGE )
    {
        SetupRefDlg();
    }
}

IMPL_LINK( ScTPValidationValue, KillEditFocusHdl, formula::RefEdit&, rWnd, void )
{
    if (&rWnd != m_pRefEdit)
        return;
    if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        if (pValidationDlg->IsChildFocus() && !pValidationDlg->IsRefInputting())
        {
            if( ( !m_pRefEdit || !m_pRefEdit->GetWidget()->has_focus()) && !m_xBtnRef->GetWidget()->has_focus() )
            {
                RemoveRefDlg(true);
            }
        }
    }
}

IMPL_LINK( ScTPValidationValue, KillButtonFocusHdl, formula::RefButton&, rWnd, void )
{
    if( &rWnd != m_xBtnRef.get())
        return;
    if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
        if (pValidationDlg->IsChildFocus() && !pValidationDlg->IsRefInputting())
            if( ( !m_pRefEdit || !m_pRefEdit->GetWidget()->has_focus()) && !m_xBtnRef->GetWidget()->has_focus() )
            {
                RemoveRefDlg(true);
            }
}

IMPL_LINK_NOARG(ScTPValidationValue, SelectHdl, weld::ComboBox&, void)
{
    const sal_Int32 nLbPos = m_xLbAllow->get_active();
    bool bEnable = (nLbPos != SC_VALIDDLG_ALLOW_ANY);
    bool bRange = (nLbPos == SC_VALIDDLG_ALLOW_RANGE);
    bool bList = (nLbPos == SC_VALIDDLG_ALLOW_LIST);
    bool bCustom = (nLbPos == SC_VALIDDLG_ALLOW_CUSTOM);

    m_xCbAllow->set_sensitive( bEnable );   // Empty cell
    m_xCbCaseSens->set_sensitive( bEnable &&
        (bRange || bList || bCustom) ); // Case Sensitive
    m_xFtValue->set_sensitive( bEnable );
    m_xLbValue->set_sensitive( bEnable );
    m_xFtMin->set_sensitive( bEnable );
    m_xEdMin->GetWidget()->set_sensitive( bEnable );
    m_xEdList->set_sensitive( bEnable );
    m_xFtMax->set_sensitive( bEnable );
    m_xEdMax->GetWidget()->set_sensitive( bEnable );

    bool bShowMax = false;

    if( bRange )
        m_xFtMin->set_label( maStrRange );
    else if( bList )
        m_xFtMin->set_label( maStrList );
    else if( bCustom )
        m_xFtMin->set_label( maStrFormula );
    else
    {
        switch( m_xLbValue->get_active() )
        {
            case SC_VALIDDLG_DATA_EQUAL:
            case SC_VALIDDLG_DATA_NOTEQUAL:     m_xFtMin->set_label( maStrValue );  break;

            case SC_VALIDDLG_DATA_LESS:
            case SC_VALIDDLG_DATA_EQLESS:       m_xFtMin->set_label( maStrMax );    break;

            case SC_VALIDDLG_DATA_VALIDRANGE:
            case SC_VALIDDLG_DATA_INVALIDRANGE:   bShowMax = true;
                [[fallthrough]];
            case SC_VALIDDLG_DATA_GREATER:
            case SC_VALIDDLG_DATA_EQGREATER:    m_xFtMin->set_label( maStrMin );    break;

            default:
                OSL_FAIL( "ScTPValidationValue::SelectHdl - unknown condition mode" );
        }
    }

    m_xCbCaseSens->set_visible( bRange || bList || bCustom ); // Case Sensitive
    m_xCbShow->set_visible( bRange || bList );
    m_xCbSort->set_visible( bRange || bList );
    m_xFtValue->set_visible( !bRange && !bList && !bCustom);
    m_xLbValue->set_visible( !bRange && !bList && !bCustom );
    m_xEdMin->GetWidget()->set_visible( !bList );
    m_xEdList->set_visible( bList );
    m_xMinGrid->set_vexpand( bList );
    m_xFtMax->set_visible( bShowMax );
    m_xEdMax->GetWidget()->set_visible( bShowMax );
    m_xFtHint->set_visible( bRange );
    m_xBtnRef->GetWidget()->set_visible( bRange );  // cell range picker
}

IMPL_LINK_NOARG(ScTPValidationValue, CheckHdl, weld::Toggleable&, void)
{
    m_xCbSort->set_sensitive( m_xCbShow->get_active() );
}

// Input Help Page

ScTPValidationHelp::ScTPValidationHelp(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet)
    : SfxTabPage(pPage, pController, "modules/scalc/ui/validationhelptabpage.ui", "ValidationHelpTabPage", &rArgSet)
    , m_xTsbHelp(m_xBuilder->weld_check_button("tsbhelp"))
    , m_xEdtTitle(m_xBuilder->weld_entry("title"))
    , m_xEdInputHelp(m_xBuilder->weld_text_view("inputhelp_text"))
{
    m_xEdInputHelp->set_size_request(m_xEdInputHelp->get_approximate_digit_width() * 40, m_xEdInputHelp->get_height_rows(13));
}

ScTPValidationHelp::~ScTPValidationHelp()
{
}

std::unique_ptr<SfxTabPage> ScTPValidationHelp::Create(weld::Container* pPage, weld::DialogController* pController,
                                              const SfxItemSet* rArgSet)
{
    return std::make_unique<ScTPValidationHelp>(pPage, pController, *rArgSet);
}

void ScTPValidationHelp::Reset( const SfxItemSet* rArgSet )
{
    if ( const SfxBoolItem* pItem = rArgSet->GetItemIfSet( FID_VALID_SHOWHELP ) )
        m_xTsbHelp->set_state( pItem->GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    else
        m_xTsbHelp->set_state( TRISTATE_FALSE );

    if ( const SfxStringItem* pItem = rArgSet->GetItemIfSet( FID_VALID_HELPTITLE ) )
        m_xEdtTitle->set_text( pItem->GetValue() );
    else
        m_xEdtTitle->set_text( OUString() );

    if ( const SfxStringItem* pItem = rArgSet->GetItemIfSet( FID_VALID_HELPTEXT ) )
        m_xEdInputHelp->set_text( pItem->GetValue() );
    else
        m_xEdInputHelp->set_text( OUString() );
}

bool ScTPValidationHelp::FillItemSet( SfxItemSet* rArgSet )
{
    rArgSet->Put( SfxBoolItem( FID_VALID_SHOWHELP, m_xTsbHelp->get_state() == TRISTATE_TRUE ) );
    rArgSet->Put( SfxStringItem( FID_VALID_HELPTITLE, m_xEdtTitle->get_text() ) );
    rArgSet->Put( SfxStringItem( FID_VALID_HELPTEXT, m_xEdInputHelp->get_text() ) );

    return true;
}

// Error Alert Page

ScTPValidationError::ScTPValidationError(weld::Container* pPage, weld::DialogController* pController,
                                         const SfxItemSet& rArgSet)

    :   SfxTabPage      ( pPage, pController,
                          "modules/scalc/ui/erroralerttabpage.ui", "ErrorAlertTabPage",
                          &rArgSet )
    , m_xTsbShow(m_xBuilder->weld_check_button("tsbshow"))
    , m_xLbAction(m_xBuilder->weld_combo_box("actionCB"))
    , m_xBtnSearch(m_xBuilder->weld_button("browseBtn"))
    , m_xEdtTitle(m_xBuilder->weld_entry("erroralert_title"))
    , m_xFtError(m_xBuilder->weld_label("errormsg_label"))
    , m_xEdError(m_xBuilder->weld_text_view("errorMsg"))
{
    m_xEdError->set_size_request(m_xEdError->get_approximate_digit_width() * 40, m_xEdError->get_height_rows(12));
    Init();
}

ScTPValidationError::~ScTPValidationError()
{
}

void ScTPValidationError::Init()
{
    m_xLbAction->connect_changed(LINK(this, ScTPValidationError, SelectActionHdl));
    m_xBtnSearch->connect_clicked(LINK( this, ScTPValidationError, ClickSearchHdl));

    m_xLbAction->set_active(0);

    SelectActionHdl(*m_xLbAction);
}

std::unique_ptr<SfxTabPage> ScTPValidationError::Create(weld::Container* pPage, weld::DialogController* pController,
                                               const SfxItemSet* rArgSet)
{
    return std::make_unique<ScTPValidationError>(pPage, pController, *rArgSet);
}

void ScTPValidationError::Reset( const SfxItemSet* rArgSet )
{
    if ( const SfxBoolItem* pItem = rArgSet->GetItemIfSet( FID_VALID_SHOWERR ) )
        m_xTsbShow->set_state( pItem->GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    else
        m_xTsbShow->set_state( TRISTATE_TRUE );   // check by default

    if ( const SfxUInt16Item* pItem = rArgSet->GetItemIfSet( FID_VALID_ERRSTYLE ) )
        m_xLbAction->set_active( pItem->GetValue() );
    else
        m_xLbAction->set_active( 0 );

    if ( const SfxStringItem* pItem = rArgSet->GetItemIfSet( FID_VALID_ERRTITLE ) )
        m_xEdtTitle->set_text( pItem->GetValue() );
    else
        m_xEdtTitle->set_text( OUString() );

    if ( const SfxStringItem* pItem = rArgSet->GetItemIfSet( FID_VALID_ERRTEXT ) )
        m_xEdError->set_text( pItem->GetValue() );
    else
        m_xEdError->set_text( OUString() );

    SelectActionHdl(*m_xLbAction);
}

bool ScTPValidationError::FillItemSet( SfxItemSet* rArgSet )
{
    rArgSet->Put( SfxBoolItem( FID_VALID_SHOWERR, m_xTsbShow->get_state() == TRISTATE_TRUE ) );
    rArgSet->Put( SfxUInt16Item( FID_VALID_ERRSTYLE, m_xLbAction->get_active() ) );
    rArgSet->Put( SfxStringItem( FID_VALID_ERRTITLE, m_xEdtTitle->get_text() ) );
    rArgSet->Put( SfxStringItem( FID_VALID_ERRTEXT, m_xEdError->get_text() ) );

    return true;
}

IMPL_LINK_NOARG(ScTPValidationError, SelectActionHdl, weld::ComboBox&, void)
{
    ScValidErrorStyle eStyle = static_cast<ScValidErrorStyle>(m_xLbAction->get_active());
    bool bMacro = ( eStyle == SC_VALERR_MACRO );

    m_xBtnSearch->set_sensitive( bMacro );
    m_xFtError->set_sensitive( !bMacro );
    m_xEdError->set_sensitive( !bMacro );
}

IMPL_LINK_NOARG(ScTPValidationError, ClickSearchHdl, weld::Button&, void)
{
    // Use static SfxApplication method to bring up selector dialog for
    // choosing a script
    OUString aScriptURL = SfxApplication::ChooseScript(GetFrameWeld());

    if ( !aScriptURL.isEmpty() )
    {
        m_xEdtTitle->set_text( aScriptURL );
    }
}

bool ScValidationDlg::EnterRefStatus()
{
    ScTabViewShell *pTabViewShell = GetTabViewShell();

    if( !pTabViewShell ) return false;

    sal_uInt16 nId  = SLOTID;
    SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
    SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

    if (pWnd && pWnd->GetController().get() != this) pWnd = nullptr;

    SC_MOD()->SetRefDialog( nId, pWnd == nullptr );

    return true;
}

bool ScValidationDlg::LeaveRefStatus()
{
    ScTabViewShell *pTabViewShell = GetTabViewShell();

    if( !pTabViewShell ) return false;

    sal_uInt16 nId  = SLOTID;
    SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
    if (rViewFrm.GetChildWindow(nId))
    {
        DoClose( nId );
    }
    return true;
}

bool ScValidationDlg::SetupRefDlg()
{
    if ( m_bOwnRefHdlr ) return false;
    if( EnterRefMode() )
    {
        SetModal( false );
        m_bOwnRefHdlr = true;
        return EnterRefStatus();
    }

    return false;
}

bool ScValidationDlg::RemoveRefDlg( bool bRestoreModal /* = true */ )
{
    bool bVisLock = false;
    bool bFreeWindowLock = false;

    ScTabViewShell *pTabVwSh = GetTabViewShell();

    if( !pTabVwSh ) return false;

    if ( SfxChildWindow* pWnd = pTabVwSh->GetViewFrame().GetChildWindow( SID_VALIDITY_REFERENCE ) )
    {
        bVisLock = static_cast<ScValidityRefChildWin*>(pWnd)->LockVisible( true );
        bFreeWindowLock = static_cast<ScValidityRefChildWin*>(pWnd)->LockFreeWindow( true );
    }

    if ( !m_bOwnRefHdlr ) return false;
    if( LeaveRefStatus() && LeaveRefMode() )
    {
        m_bOwnRefHdlr = false;

        if( bRestoreModal )
        {
            SetModal( true );
        }
    }

    if ( SfxChildWindow* pWnd = pTabVwSh->GetViewFrame().GetChildWindow( SID_VALIDITY_REFERENCE ) )
    {
        static_cast<ScValidityRefChildWin*>(pWnd)->LockVisible( bVisLock );
        static_cast<ScValidityRefChildWin*>(pWnd)->LockFreeWindow( bFreeWindowLock );
    }

    return true;
}

IMPL_LINK_NOARG(ScTPValidationValue, ClickHdl, formula::RefButton&, void)
{
    SetupRefDlg();
}

bool ScValidationDlg::IsChildFocus() const
{
    return m_xDialog->has_toplevel_focus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
