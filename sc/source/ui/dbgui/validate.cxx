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
#include <vcl/svapp.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/app.hxx>

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

const sal_uInt16 ScTPValidationValue::pValueRanges[] =
{
    FID_VALID_MODE, FID_VALID_ERRTEXT,
    FID_VALID_LISTTYPE, FID_VALID_LISTTYPE,
    0
};

ScValidationDlg::ScValidationDlg(vcl::Window* pParent, const SfxItemSet* pArgSet,
    ScTabViewShell *pTabViewSh)
    : ScValidationDlgBase(pParent ? pParent : SfxGetpApp()->GetTopWindow(),
        "ValidationDialog", "modules/scalc/ui/validationdialog.ui", pArgSet, nullptr)
    , m_pTabVwSh(pTabViewSh)
    , m_nValuePageId(0)
    , m_bOwnRefHdlr(false)
    , m_bRefInputting(false)
{
    m_nValuePageId = AddTabPage("criteria", ScTPValidationValue::Create);
    AddTabPage("inputhelp", ScTPValidationHelp::Create);
    AddTabPage("erroralert", ScTPValidationError::Create);
    get(m_pHBox, "refinputbox");
}

ScValidationDlg::~ScValidationDlg()
{
    disposeOnce();
}

void ScTPValidationValue::SetReferenceHdl( const ScRange&rRange , const ScDocument* pDoc )
{
    if ( rRange.aStart != rRange.aEnd )
        if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
            if( m_pRefEdit )
                pValidationDlg->RefInputStart( m_pRefEdit );

    if ( m_pRefEdit )
    {
        OUString aStr(rRange.Format(ScRefFlags::RANGE_ABS_3D, pDoc, pDoc->GetAddressConvention()));
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
    if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        vcl::Window *pNewParent = pValidationDlg->get_refinput_shrink_parent();
        if( pEdit == m_pRefEdit && m_pRefEdit->GetParent() != pNewParent )
        {
            m_pRefEdit->SetParent(pNewParent);
        }

        if( pButton == m_pBtnRef && m_pBtnRef->GetParent() != pNewParent )
        {
            m_pBtnRef->SetParent(pNewParent);
        }

        pNewParent->Show();
    }
}

void ScTPValidationValue::RefInputDonePostHdl()
{
    if( m_pRefEdit && m_pRefEdit->GetParent() != m_pRefGrid )
    {
        m_pRefEdit->SetParent( m_pRefGrid );
        m_pBtnRef->SetParent( m_pRefEdit ); //if Edit SetParent but button not, the tab order will be incorrect, need button to setparent to another window and restore parent later in order to restore the tab order
    }

    if( m_pBtnRef->GetParent() != m_pRefGrid )
        m_pBtnRef->SetParent( m_pRefGrid );

    if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        pValidationDlg->get_refinput_shrink_parent()->Hide();
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

    if( m_pRefEdit && !m_pRefEdit->HasFocus() )
        m_pRefEdit->GrabFocus();
}

ScTPValidationValue::ScRefButtonEx::~ScRefButtonEx()
{
    disposeOnce();
}

void ScTPValidationValue::ScRefButtonEx::dispose()
{
    m_pPage.clear();
    ::formula::RefButton::dispose();
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
void lclGetFormulaFromStringList( OUString& rFmlaStr, const OUString& rStringList, sal_Unicode cFmlaSep )
{
    rFmlaStr.clear();
    if (!rStringList.isEmpty())
    {
        sal_Int32 nIdx {0};
        do
        {
            OUString aToken {rStringList.getToken( 0, '\n', nIdx )};
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
    const OUString aQuotes( "\"\"" );

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

ScTPValidationValue::ScTPValidationValue( vcl::Window* pParent, const SfxItemSet& rArgSet )
    : SfxTabPage( pParent, "ValidationCriteriaPage",
        "modules/scalc/ui/validationcriteriapage.ui", &rArgSet)
    , maStrMin(ScResId(SCSTR_VALID_MINIMUM))
    , maStrMax(ScResId(SCSTR_VALID_MAXIMUM))
    , maStrValue(ScResId(SCSTR_VALID_VALUE))
    , maStrFormula(ScResId(SCSTR_VALID_FORMULA))
    , maStrRange(ScResId(SCSTR_VALID_RANGE))
    , maStrList(ScResId(SCSTR_VALID_LIST))
    , m_pRefEdit(nullptr)
{
    get(m_pLbAllow, "allow");
    get(m_pCbAllow, "allowempty");
    get(m_pCbShow, "showlist");
    get(m_pCbSort, "sortascend");
    get(m_pFtValue, "valueft");
    get(m_pLbValue, "data");
    get(m_pFtMin, "minft");
    get(m_pMinGrid, "mingrid");
    get(m_pEdMin, "min");
    m_pEdMin->SetReferences(nullptr, m_pFtMin);
    get(m_pEdList, "minlist");
    Size aSize(LogicToPixel(Size(174, 105), MapMode(MapUnit::MapAppFont)));
    m_pEdList->set_width_request(aSize.Width());
    m_pEdList->set_height_request(aSize.Height());
    get(m_pFtMax, "maxft");
    get(m_pEdMax, "max");
    m_pEdMax->SetReferences(nullptr, m_pFtMax);
    get(m_pFtHint, "hintft");
    get(m_pBtnRef, "validref");
    m_pBtnRef->SetParentPage(this);
    get(m_pRefGrid, "refgrid");

    //lock in the max size initial config
    aSize = get_preferred_size();
    set_width_request(aSize.Width());
    set_height_request(aSize.Height());

    Init();

    // list separator in formulas
    OUString aListSep = ::ScCompiler::GetNativeSymbol( ocSep );
    OSL_ENSURE( aListSep.getLength() == 1, "ScTPValidationValue::ScTPValidationValue - list separator error" );
    mcFmlaSep = aListSep.getLength() ? aListSep[0] : ';';
    m_pBtnRef->Hide(); // cell range picker
}

ScTPValidationValue::~ScTPValidationValue()
{
    disposeOnce();
}

void ScTPValidationValue::dispose()
{
    m_pLbAllow.clear();
    m_pCbAllow.clear();
    m_pCbShow.clear();
    m_pCbSort.clear();
    m_pFtValue.clear();
    m_pLbValue.clear();
    m_pFtMin.clear();
    m_pMinGrid.clear();
    m_pEdMin.clear();
    m_pEdList.clear();
    m_pFtMax.clear();
    m_pEdMax.clear();
    m_pFtHint.clear();
    m_pRefEdit.clear();
    m_pBtnRef.clear();
    m_pRefGrid.clear();
    SfxTabPage::dispose();
}


void ScTPValidationValue::Init()
{
    m_pLbAllow->SetSelectHdl( LINK( this, ScTPValidationValue, SelectHdl ) );
    m_pLbValue->SetSelectHdl( LINK( this, ScTPValidationValue, SelectHdl ) );
    m_pCbShow->SetClickHdl( LINK( this, ScTPValidationValue, CheckHdl ) );

    // cell range picker
    m_pEdMin->SetGetFocusHdl( LINK( this, ScTPValidationValue, EditSetFocusHdl ) );
    m_pEdMin->SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillFocusHdl ) );
    m_pEdMax->SetGetFocusHdl( LINK( this, ScTPValidationValue, EditSetFocusHdl ) );
    m_pBtnRef->SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillFocusHdl ) );
    m_pEdMax->SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillFocusHdl ) );

    m_pLbAllow->SelectEntryPos( SC_VALIDDLG_ALLOW_ANY );
    m_pLbValue->SelectEntryPos( SC_VALIDDLG_DATA_EQUAL );

    SelectHdl( *m_pLbAllow.get() );
    CheckHdl( nullptr );
}

VclPtr<SfxTabPage> ScTPValidationValue::Create( TabPageParent pParent, const SfxItemSet* rArgSet )
{
    return VclPtr<ScTPValidationValue>::Create( pParent.pParent, *rArgSet );
}

void ScTPValidationValue::Reset( const SfxItemSet* rArgSet )
{
    const SfxPoolItem* pItem;

    sal_uInt16 nLbPos = SC_VALIDDLG_ALLOW_ANY;
    if( rArgSet->GetItemState( FID_VALID_MODE, true, &pItem ) == SfxItemState::SET )
        nLbPos = lclGetPosFromValMode( static_cast< ScValidationMode >(
            static_cast< const SfxAllEnumItem* >( pItem )->GetValue() ) );
    m_pLbAllow->SelectEntryPos( nLbPos );

    nLbPos = SC_VALIDDLG_DATA_EQUAL;
    if( rArgSet->GetItemState( FID_VALID_CONDMODE, true, &pItem ) == SfxItemState::SET )
        nLbPos = lclGetPosFromCondMode( static_cast< ScConditionMode >(
            static_cast< const SfxAllEnumItem* >( pItem )->GetValue() ) );
    m_pLbValue->SelectEntryPos( nLbPos );

    // *** check boxes ***
    bool bCheck = true;
    if( rArgSet->GetItemState( FID_VALID_BLANK, true, &pItem ) == SfxItemState::SET )
        bCheck = static_cast< const SfxBoolItem* >( pItem )->GetValue();
    m_pCbAllow->Check( bCheck );

    sal_Int32 nListType = ValidListType::UNSORTED;
    if( rArgSet->GetItemState( FID_VALID_LISTTYPE, true, &pItem ) == SfxItemState::SET )
        nListType = static_cast< const SfxInt16Item* >( pItem )->GetValue();
    m_pCbShow->Check( nListType != ValidListType::INVISIBLE );
    m_pCbSort->Check( nListType == ValidListType::SORTEDASCENDING );

    // *** formulas ***
    OUString aFmlaStr;
    if ( rArgSet->GetItemState( FID_VALID_VALUE1, true, &pItem ) == SfxItemState::SET )
        aFmlaStr = static_cast< const SfxStringItem* >( pItem )->GetValue();
    SetFirstFormula( aFmlaStr );

    aFmlaStr.clear();
    if ( rArgSet->GetItemState( FID_VALID_VALUE2, true, &pItem ) == SfxItemState::SET )
        aFmlaStr = static_cast< const SfxStringItem* >( pItem )->GetValue();
    SetSecondFormula( aFmlaStr );

    SelectHdl( *m_pLbAllow.get() );
    CheckHdl( nullptr );
}

bool ScTPValidationValue::FillItemSet( SfxItemSet* rArgSet )
{
    sal_Int16 nListType = m_pCbShow->IsChecked() ?
        (m_pCbSort->IsChecked() ? ValidListType::SORTEDASCENDING : ValidListType::UNSORTED) :
        ValidListType::INVISIBLE;

    const sal_Int32 nLbPos = m_pLbAllow->GetSelectedEntryPos();
    bool bCustom = (nLbPos == SC_VALIDDLG_ALLOW_CUSTOM);
    ScConditionMode eCondMode = bCustom ?
            ScConditionMode::Direct : lclGetCondModeFromPos( m_pLbValue->GetSelectedEntryPos() );

    rArgSet->Put( SfxAllEnumItem( FID_VALID_MODE, sal::static_int_cast<sal_uInt16>(
                    lclGetValModeFromPos( nLbPos ) ) ) );
    rArgSet->Put( SfxAllEnumItem( FID_VALID_CONDMODE, sal::static_int_cast<sal_uInt16>( eCondMode ) ) );
    rArgSet->Put( SfxStringItem( FID_VALID_VALUE1, GetFirstFormula() ) );
    rArgSet->Put( SfxStringItem( FID_VALID_VALUE2, GetSecondFormula() ) );
    rArgSet->Put( SfxBoolItem( FID_VALID_BLANK, m_pCbAllow->IsChecked() ) );
    rArgSet->Put( SfxInt16Item( FID_VALID_LISTTYPE, nListType ) );
    return true;
}

OUString ScTPValidationValue::GetFirstFormula() const
{
    OUString aFmlaStr;
    if( m_pLbAllow->GetSelectedEntryPos() == SC_VALIDDLG_ALLOW_LIST )
        lclGetFormulaFromStringList( aFmlaStr, m_pEdList->GetText(), mcFmlaSep );
    else
        aFmlaStr = m_pEdMin->GetText();
    return aFmlaStr;
}

OUString ScTPValidationValue::GetSecondFormula() const
{
    return m_pEdMax->GetText();
}

void ScTPValidationValue::SetFirstFormula( const OUString& rFmlaStr )
{
    // try if formula is a string list, validation mode must already be set
    OUString aStringList;
    if( (m_pLbAllow->GetSelectedEntryPos() == SC_VALIDDLG_ALLOW_RANGE) &&
        lclGetStringListFromFormula( aStringList, rFmlaStr, mcFmlaSep ) )
    {
        m_pEdList->SetText( aStringList );
        m_pEdMin->SetText( EMPTY_OUSTRING );
        // change validation mode to string list
        m_pLbAllow->SelectEntryPos( SC_VALIDDLG_ALLOW_LIST );
    }
    else
    {
        m_pEdMin->SetText( rFmlaStr );
        m_pEdList->SetText( EMPTY_OUSTRING );
    }
}

void ScTPValidationValue::SetSecondFormula( const OUString& rFmlaStr )
{
    m_pEdMax->SetText( rFmlaStr );
}

ScValidationDlg * ScTPValidationValue::GetValidationDlg()
{
    if( vcl::Window *pParent = GetParent() )
        do{
            if ( auto pValidationDlg = dynamic_cast<ScValidationDlg*>( pParent ) )
                return pValidationDlg;
        }while ( nullptr != ( pParent = pParent->GetParent() ) );
    return nullptr;
}

void ScTPValidationValue::SetupRefDlg()
{
    if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        if( pValidationDlg->SetupRefDlg() )
        {
            pValidationDlg->SetHandler( this );
            pValidationDlg->SetSetRefHdl( static_cast<ScRefHandlerHelper::PFUNCSETREFHDLTYPE>( &ScTPValidationValue::SetReferenceHdl ) );
            pValidationDlg->SetSetActHdl( static_cast<ScRefHandlerHelper::PCOMMONHDLTYPE>( &ScTPValidationValue::SetActiveHdl ) );
            pValidationDlg->SetRefInputStartPreHdl( static_cast<ScRefHandlerHelper::PINPUTSTARTDLTYPE>( &ScTPValidationValue::RefInputStartPreHdl ) );
            pValidationDlg->SetRefInputDonePostHdl( static_cast<ScRefHandlerHelper::PCOMMONHDLTYPE>( &ScTPValidationValue::RefInputDonePostHdl ) );

            vcl::Window *pLabel = nullptr;

            if ( m_pEdMax->IsVisible() )
            {
                m_pRefEdit = m_pEdMax;
                pLabel = m_pFtMax;
            }
            else if ( m_pEdMin->IsVisible() )
            {
                m_pRefEdit = m_pEdMin;
                pLabel = m_pFtMin;
            }

            if( m_pRefEdit && !m_pRefEdit->HasFocus() )
                m_pRefEdit->GrabFocus();

            if( m_pRefEdit )
                m_pRefEdit->SetReferences( pValidationDlg, pLabel );

            m_pBtnRef->SetReferences( pValidationDlg, m_pRefEdit );
        }
    }
}

void ScTPValidationValue::RemoveRefDlg()
{
    if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        if( pValidationDlg->RemoveRefDlg(true) )
        {
            pValidationDlg->SetHandler( nullptr );
            pValidationDlg->SetSetRefHdl( nullptr );
            pValidationDlg->SetSetActHdl( nullptr );
            pValidationDlg->SetRefInputStartPreHdl( nullptr );
            pValidationDlg->SetRefInputDonePostHdl( nullptr );

            if( m_pRefEdit )
                m_pRefEdit->SetReferences( nullptr, nullptr );
            m_pRefEdit = nullptr;

            m_pBtnRef->SetReferences( nullptr, nullptr );
        }
    }
}

IMPL_LINK_NOARG(ScTPValidationValue, EditSetFocusHdl, Control&, void)
{
    const sal_Int32 nPos = m_pLbAllow->GetSelectedEntryPos();

    if ( nPos == SC_VALIDDLG_ALLOW_RANGE )
    {
        SetupRefDlg();
    }
}

IMPL_LINK( ScTPValidationValue, KillFocusHdl, Control&, rControl, void )
{
    vcl::Window* pWnd = static_cast<vcl::Window*>(&rControl);
    if( pWnd == m_pRefEdit || pWnd == m_pBtnRef )
        if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
            if ( (pValidationDlg->IsActive() || pValidationDlg->IsChildFocus() ) && !pValidationDlg->IsRefInputting() )
                if( ( !m_pRefEdit || !m_pRefEdit->HasFocus()) && !m_pBtnRef->HasFocus() )
                {
                    RemoveRefDlg();
                }
}

IMPL_LINK_NOARG(ScTPValidationValue, SelectHdl, ListBox&, void)
{
    const sal_Int32 nLbPos = m_pLbAllow->GetSelectedEntryPos();
    bool bEnable = (nLbPos != SC_VALIDDLG_ALLOW_ANY);
    bool bRange = (nLbPos == SC_VALIDDLG_ALLOW_RANGE);
    bool bList = (nLbPos == SC_VALIDDLG_ALLOW_LIST);
    bool bCustom = (nLbPos == SC_VALIDDLG_ALLOW_CUSTOM);

    m_pCbAllow->Enable( bEnable );   // Empty cell
    m_pFtValue->Enable( bEnable );
    m_pLbValue->Enable( bEnable );
    m_pFtMin->Enable( bEnable );
    m_pEdMin->Enable( bEnable );
    m_pEdList->Enable( bEnable );
    m_pFtMax->Enable( bEnable );
    m_pEdMax->Enable( bEnable );

    bool bShowMax = false;

    if( bRange )
        m_pFtMin->SetText( maStrRange );
    else if( bList )
        m_pFtMin->SetText( maStrList );
    else if( bCustom )
        m_pFtMin->SetText( maStrFormula );
    else
    {
        switch( m_pLbValue->GetSelectedEntryPos() )
        {
            case SC_VALIDDLG_DATA_EQUAL:
            case SC_VALIDDLG_DATA_NOTEQUAL:     m_pFtMin->SetText( maStrValue );  break;

            case SC_VALIDDLG_DATA_LESS:
            case SC_VALIDDLG_DATA_EQLESS:       m_pFtMin->SetText( maStrMax );    break;

            case SC_VALIDDLG_DATA_VALIDRANGE:
            case SC_VALIDDLG_DATA_INVALIDRANGE:   bShowMax = true;
                [[fallthrough]];
            case SC_VALIDDLG_DATA_GREATER:
            case SC_VALIDDLG_DATA_EQGREATER:    m_pFtMin->SetText( maStrMin );    break;

            default:
                OSL_FAIL( "ScTPValidationValue::SelectHdl - unknown condition mode" );
        }
    }

    m_pCbShow->Show( bRange || bList );
    m_pCbSort->Show( bRange || bList );
    m_pFtValue->Show( !bRange && !bList && !bCustom);
    m_pLbValue->Show( !bRange && !bList && !bCustom );
    m_pEdMin->Show( !bList );
    m_pEdList->Show( bList );
    m_pMinGrid->set_vexpand( bList );
    WinBits nBits = m_pFtMin->GetStyle();
    nBits &= ~(WB_TOP | WB_VCENTER | WB_BOTTOM);
    if (bList)
        nBits |= WB_TOP;
    else
        nBits |= WB_VCENTER;
    m_pFtMin->SetStyle( nBits );
    m_pFtMax->Show( bShowMax );
    m_pEdMax->Show( bShowMax );
    m_pFtHint->Show( bRange );
    m_pBtnRef->Show( bRange );  // cell range picker
}

IMPL_LINK_NOARG(ScTPValidationValue, CheckHdl, Button*, void)
{
    m_pCbSort->Enable( m_pCbShow->IsChecked() );
}

// Input Help Page

ScTPValidationHelp::ScTPValidationHelp(TabPageParent pParent, const SfxItemSet& rArgSet)
    : SfxTabPage(pParent, "modules/scalc/ui/validationhelptabpage.ui", "ValidationHelpTabPage", &rArgSet)
    , m_xTsbHelp(m_xBuilder->weld_check_button("tsbhelp"))
    , m_xEdtTitle(m_xBuilder->weld_entry("title"))
    , m_xEdInputHelp(m_xBuilder->weld_text_view("inputhelp"))
{
    m_xEdInputHelp->set_size_request(m_xEdInputHelp->get_approximate_digit_width() * 40, m_xEdInputHelp->get_height_rows(13));
}

ScTPValidationHelp::~ScTPValidationHelp()
{
    disposeOnce();
}

VclPtr<SfxTabPage> ScTPValidationHelp::Create(TabPageParent pParent,
                                              const SfxItemSet* rArgSet)
{
    return VclPtr<ScTPValidationHelp>::Create(pParent, *rArgSet);
}

void ScTPValidationHelp::Reset( const SfxItemSet* rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet->GetItemState( FID_VALID_SHOWHELP, true, &pItem ) == SfxItemState::SET )
        m_xTsbHelp->set_state( static_cast<const SfxBoolItem*>(pItem)->GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    else
        m_xTsbHelp->set_state( TRISTATE_FALSE );

    if ( rArgSet->GetItemState( FID_VALID_HELPTITLE, true, &pItem ) == SfxItemState::SET )
        m_xEdtTitle->set_text( static_cast<const SfxStringItem*>(pItem)->GetValue() );
    else
        m_xEdtTitle->set_text( EMPTY_OUSTRING );

    if ( rArgSet->GetItemState( FID_VALID_HELPTEXT, true, &pItem ) == SfxItemState::SET )
        m_xEdInputHelp->set_text( static_cast<const SfxStringItem*>(pItem)->GetValue() );
    else
        m_xEdInputHelp->set_text( EMPTY_OUSTRING );
}

bool ScTPValidationHelp::FillItemSet( SfxItemSet* rArgSet )
{
    rArgSet->Put( SfxBoolItem( FID_VALID_SHOWHELP, m_xTsbHelp->get_state() == TRISTATE_TRUE ) );
    rArgSet->Put( SfxStringItem( FID_VALID_HELPTITLE, m_xEdtTitle->get_text() ) );
    rArgSet->Put( SfxStringItem( FID_VALID_HELPTEXT, m_xEdInputHelp->get_text() ) );

    return true;
}

// Error Alert Page

ScTPValidationError::ScTPValidationError(TabPageParent pParent,
                                         const SfxItemSet& rArgSet)

    :   SfxTabPage      ( pParent,
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
    disposeOnce();
}

void ScTPValidationError::Init()
{
    m_xLbAction->connect_changed(LINK(this, ScTPValidationError, SelectActionHdl));
    m_xBtnSearch->connect_clicked(LINK( this, ScTPValidationError, ClickSearchHdl));

    m_xLbAction->set_active(0);

    SelectActionHdl(*m_xLbAction);
}

VclPtr<SfxTabPage> ScTPValidationError::Create(TabPageParent pParent,
                                               const SfxItemSet* rArgSet)
{
    return VclPtr<ScTPValidationError>::Create(pParent, *rArgSet);
}

void ScTPValidationError::Reset( const SfxItemSet* rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet->GetItemState( FID_VALID_SHOWERR, true, &pItem ) == SfxItemState::SET )
        m_xTsbShow->set_state( static_cast<const SfxBoolItem*>(pItem)->GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    else
        m_xTsbShow->set_state( TRISTATE_TRUE );   // check by default

    if ( rArgSet->GetItemState( FID_VALID_ERRSTYLE, true, &pItem ) == SfxItemState::SET )
        m_xLbAction->set_active( static_cast<const SfxAllEnumItem*>(pItem)->GetValue() );
    else
        m_xLbAction->set_active( 0 );

    if ( rArgSet->GetItemState( FID_VALID_ERRTITLE, true, &pItem ) == SfxItemState::SET )
        m_xEdtTitle->set_text( static_cast<const SfxStringItem*>(pItem)->GetValue() );
    else
        m_xEdtTitle->set_text( EMPTY_OUSTRING );

    if ( rArgSet->GetItemState( FID_VALID_ERRTEXT, true, &pItem ) == SfxItemState::SET )
        m_xEdError->set_text( static_cast<const SfxStringItem*>(pItem)->GetValue() );
    else
        m_xEdError->set_text( EMPTY_OUSTRING );

    SelectActionHdl(*m_xLbAction);
}

bool ScTPValidationError::FillItemSet( SfxItemSet* rArgSet )
{
    rArgSet->Put( SfxBoolItem( FID_VALID_SHOWERR, m_xTsbShow->get_state() == TRISTATE_TRUE ) );
    rArgSet->Put( SfxAllEnumItem( FID_VALID_ERRSTYLE, m_xLbAction->get_active() ) );
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
    OUString aScriptURL = SfxApplication::ChooseScript(GetDialogFrameWeld());

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
    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

    if ( pWnd && pWnd->GetWindow()!= this ) pWnd = nullptr;

    SC_MOD()->SetRefDialog( nId, pWnd == nullptr );

    return true;
}

bool ScValidationDlg::LeaveRefStatus()
{
    ScTabViewShell *pTabViewShell = GetTabViewShell();

    if( !pTabViewShell ) return false;

    sal_uInt16 nId  = SLOTID;
    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
    if ( pViewFrm->GetChildWindow( nId ) )
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
        return  m_bOwnRefHdlr = true && EnterRefStatus();
    }

    return false;
}

bool ScValidationDlg::RemoveRefDlg( bool bRestoreModal /* = true */ )
{
    bool bVisLock = false;
    bool bFreeWindowLock = false;

    ScTabViewShell *pTabVwSh = GetTabViewShell();

    if( !pTabVwSh ) return false;

    if ( SfxChildWindow* pWnd = pTabVwSh->GetViewFrame()->GetChildWindow( SID_VALIDITY_REFERENCE ) )
    {
        bVisLock = static_cast<ScValidityRefChildWin*>(pWnd)->LockVisible( true );
        bFreeWindowLock = static_cast<ScValidityRefChildWin*>(pWnd)->LockFreeWindow( true );
    }

    if ( !m_bOwnRefHdlr ) return false;
    if( LeaveRefStatus() && LeaveRefMode() )
    {
        m_bOwnRefHdlr = false;

        if( bRestoreModal )
            SetModal( true );
    }

    if ( SfxChildWindow* pWnd = pTabVwSh->GetViewFrame()->GetChildWindow( SID_VALIDITY_REFERENCE ) )
    {
        static_cast<ScValidityRefChildWin*>(pWnd)->LockVisible( bVisLock );
        static_cast<ScValidityRefChildWin*>(pWnd)->LockFreeWindow( bFreeWindowLock );
    }

    return true;
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeScRefButtonEx(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<ScTPValidationValue::ScRefButtonEx>::Create(pParent, 0);
}

void ScTPValidationValue::ScRefButtonEx::Click()
{
    if( ScTPValidationValue *pParent = GetParentPage() )
        pParent->OnClick( this );

    formula::RefButton::Click();
}

void ScTPValidationValue::OnClick( const Button *pBtn )
{
    if( pBtn == m_pBtnRef )
        SetupRefDlg();
}

bool ScValidationDlg::IsChildFocus()
{
    if ( const vcl::Window *pWin = Application::GetFocusWindow() )
        while( nullptr != ( pWin = pWin->GetParent() ) )
            if( pWin == this )
                return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
