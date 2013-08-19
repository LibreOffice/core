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

#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <sfx2/app.hxx>

#include "scresid.hxx"
#include "sc.hrc"       // -> Slot IDs

#include "stringutil.hxx"
#include "validat.hxx"
#include "validate.hrc"
#include "validate.hxx"
#include "compiler.hxx"
#include "formula/opcode.hxx"

// cell range picker
#include "tabvwsh.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/childwin.hxx>
#include "reffact.hxx"

// ============================================================================

static sal_uInt16 pValueRanges[] =
{
    FID_VALID_MODE, FID_VALID_ERRTEXT,
    FID_VALID_LISTTYPE, FID_VALID_LISTTYPE,
    0
};


// ============================================================================

ScValidationDlg::ScValidationDlg( Window*           pParent,
                                 const SfxItemSet* pArgSet,
                                 ScTabViewShell *pTabViewSh,
                                 SfxBindings *pB /*= NULL*/
                      ) :
        ScValidationDlgBase( pParent ? pParent : SFX_APP()->GetTopWindow(), TAB_DLG_VALIDATION, pArgSet, pB ),
            m_bOwnRefHdlr( false ),
            m_pTabVwSh( pTabViewSh ),
            m_bRefInputting( false )
{
    AddTabPage( TP_VALIDATION_VALUES,    ScTPValidationValue::Create, 0 );
    AddTabPage( TP_VALIDATION_INPUTHELP, ScTPValidationHelp::Create,  0 );
    AddTabPage( TP_VALIDATION_ERROR,     ScTPValidationError::Create, 0 );
    FreeResource();
    //temp hack until converted to .ui
    mpHBox = new VclHBox(get_content_area());
}

void ScTPValidationValue::SetReferenceHdl( const ScRange&rRange , ScDocument* pDoc )
{
    if ( rRange.aStart != rRange.aEnd )
        if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
            if( m_pRefEdit )
                pValidationDlg->RefInputStart( m_pRefEdit );

    if ( m_pRefEdit )
    {
        OUString  aStr;
        rRange.Format( aStr, SCR_ABS_3D, pDoc );
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

void            ScTPValidationValue::RefInputStartPreHdl( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        Window *pNewParent = pValidationDlg->get_refinput_shrink_parent();
        if( pEdit == m_pRefEdit && m_pRefEdit->GetParent() != pNewParent )
        {
            maRefEditPos = m_pRefEdit->GetPosPixel();
            maRefEditSize = m_pRefEdit->GetSizePixel();
            m_pRefEdit->SetParent(pNewParent);
        }

        if( pButton == &m_btnRef && m_btnRef.GetParent() != pNewParent )
        {
            maBtnRefPos = m_btnRef.GetPosPixel();
            maBtnRefSize = m_btnRef.GetSizePixel();
            m_btnRef.SetParent(pNewParent);
        }

        pNewParent->Show();
    }
}

void            ScTPValidationValue::RefInputDonePostHdl()
{
    if( m_pRefEdit && m_pRefEdit->GetParent()!= this )
    {
        m_pRefEdit->SetParent( this );
        m_pRefEdit->SetPosSizePixel( maRefEditPos, maRefEditSize );

        m_btnRef.SetParent( m_pRefEdit ); //if Edit SetParent but button not, the tab order will be incorrect, need button to setparent to anthor window and restore parent later in order to restore the tab order
    }

    if( m_btnRef.GetParent()!=this )
    {
        m_btnRef.SetParent( this );
        m_btnRef.SetPosSizePixel( maBtnRefPos, maBtnRefSize );
    }

    if ( ScValidationDlg *pValidationDlg = GetValidationDlg() )
        pValidationDlg->get_refinput_shrink_parent()->Hide();

    if( m_pRefEdit && !m_pRefEdit->HasFocus() )
        m_pRefEdit->GrabFocus();

}


sal_Bool ScValidationDlg::Close()
{
    if( m_bOwnRefHdlr )
        if( SfxTabPage* pPage = GetTabPage( TP_VALIDATION_VALUES ) )
            static_cast<ScTPValidationValue*>(pPage)->RemoveRefDlg();

    return ScValidationDlgBase::Close();
}

ScValidationDlg::~ScValidationDlg()
{
    if( m_bOwnRefHdlr )
        RemoveRefDlg( false );
    delete mpHBox;
}


// ============================================================================

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
        case SC_VALID_CUSTOM:   nLbPos = SC_VALIDDLG_ALLOW_ANY;     break;  // not supported
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
        case SC_COND_NONE:          // may occur in old XML files after Excel import
        case SC_COND_EQUAL:         nLbPos = SC_VALIDDLG_DATA_EQUAL;        break;
        case SC_COND_LESS:          nLbPos = SC_VALIDDLG_DATA_LESS;         break;
        case SC_COND_GREATER:       nLbPos = SC_VALIDDLG_DATA_GREATER;      break;
        case SC_COND_EQLESS:        nLbPos = SC_VALIDDLG_DATA_EQLESS;       break;
        case SC_COND_EQGREATER:     nLbPos = SC_VALIDDLG_DATA_EQGREATER;    break;
        case SC_COND_NOTEQUAL:      nLbPos = SC_VALIDDLG_DATA_NOTEQUAL;     break;
        case SC_COND_BETWEEN:       nLbPos = SC_VALIDDLG_DATA_VALIDRANGE;      break;
        case SC_COND_NOTBETWEEN:    nLbPos = SC_VALIDDLG_DATA_INVALIDRANGE;   break;
        default:    OSL_FAIL( "lclGetPosFromCondMode - unknown condition mode" );
    }
    return nLbPos;
}

/** Converts the passed list box position to an ScConditionMode. */
ScConditionMode lclGetCondModeFromPos( sal_uInt16 nLbPos )
{
    ScConditionMode eCondMode = SC_COND_EQUAL;
    switch( nLbPos )
    {
        case SC_VALIDDLG_DATA_EQUAL:        eCondMode = SC_COND_EQUAL;      break;
        case SC_VALIDDLG_DATA_LESS:         eCondMode = SC_COND_LESS;       break;
        case SC_VALIDDLG_DATA_GREATER:      eCondMode = SC_COND_GREATER;    break;
        case SC_VALIDDLG_DATA_EQLESS:       eCondMode = SC_COND_EQLESS;     break;
        case SC_VALIDDLG_DATA_EQGREATER:    eCondMode = SC_COND_EQGREATER;  break;
        case SC_VALIDDLG_DATA_NOTEQUAL:     eCondMode = SC_COND_NOTEQUAL;   break;
        case SC_VALIDDLG_DATA_VALIDRANGE:      eCondMode = SC_COND_BETWEEN;    break;
        case SC_VALIDDLG_DATA_INVALIDRANGE:   eCondMode = SC_COND_NOTBETWEEN; break;
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
    rFmlaStr = "";
    sal_Int32 nTokenCnt = comphelper::string::getTokenCount(rStringList, '\n');
    for( sal_Int32 nToken = 0, nStringIx = 0; nToken < (sal_Int32) nTokenCnt; ++nToken )
    {
        OUString aToken( rStringList.getToken( 0, '\n', nStringIx ) );
        ScGlobal::AddQuotes( aToken, '"' );
        rFmlaStr = ScGlobal::addToken(rFmlaStr, aToken, cFmlaSep);
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
    OUString aQuotes( "\"\"" );
    sal_Int32 nTokenCnt = ScStringUtil::GetQuotedTokenCount(rFmlaStr, aQuotes, cFmlaSep );

    rStringList="";
    bool bIsStringList = (nTokenCnt > 0);
    bool bTokenAdded = false;

    for( sal_Int32 nToken = 0, nStringIx = 0; bIsStringList && (nToken < nTokenCnt); ++nToken )
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

// ----------------------------------------------------------------------------

ScTPValidationValue::ScTPValidationValue( Window* pParent, const SfxItemSet& rArgSet ) :
    SfxTabPage( pParent, ScResId( TP_VALIDATION_VALUES ), rArgSet ),
    maFtAllow ( this, ScResId( FT_ALLOW ) ),
    maLbAllow ( this, ScResId( LB_ALLOW ) ),
    maCbAllow ( this, ScResId( TSB_ALLOW_BLANKS ) ),
    maCbShow  ( this, ScResId( CB_SHOWLIST ) ),
    maCbSort  ( this, ScResId( CB_SORTLIST ) ),
    maFtValue ( this, ScResId( FT_VALUE ) ),
    maLbValue ( this, ScResId( LB_VALUE ) ),
    maFtMin   ( this, ScResId( FT_MIN ) ),
    maEdMin   ( this, NULL, &maFtMin, ScResId( EDT_MIN ) ),
    maEdList  ( this, ScResId( EDT_LIST ) ),
    maFtMax   ( this, ScResId( FT_MAX ) ),
    maEdMax   ( this, NULL, &maFtMax, ScResId( EDT_MAX ) ),
    maFtHint  ( this, ScResId( FT_SOURCEHINT ) ),
    maStrMin  ( ScResId( SCSTR_VALID_MINIMUM ) ),
    maStrMax  ( ScResId( SCSTR_VALID_MAXIMUM ) ),
    maStrValue( ScResId( SCSTR_VALID_VALUE ) ),
    maStrRange( ScResId( SCSTR_VALID_RANGE ) ),
    maStrList ( ScResId( SCSTR_VALID_LIST ) ),
    m_btnRef( this, ScResId( RB_VALIDITY_REF ) )
{
    Init();
    FreeResource();

    // list separator in formulas
    OUString aListSep = ::ScCompiler::GetNativeSymbol( ocSep );
    OSL_ENSURE( aListSep.getLength() == 1, "ScTPValidationValue::ScTPValidationValue - list separator error" );
    mcFmlaSep = aListSep.getLength() ? aListSep[0] : ';';
    m_btnRef.Hide(); // cell range picker
}

ScTPValidationValue::~ScTPValidationValue()
{
}

void ScTPValidationValue::Init()
{
    maLbAllow.SetSelectHdl( LINK( this, ScTPValidationValue, SelectHdl ) );
    maLbValue.SetSelectHdl( LINK( this, ScTPValidationValue, SelectHdl ) );
    maCbShow.SetClickHdl( LINK( this, ScTPValidationValue, CheckHdl ) );

    // cell range picker
    maEdMin.SetGetFocusHdl( LINK( this, ScTPValidationValue, EditSetFocusHdl ) );
    maEdMin.SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillFocusHdl ) );
    maEdMax.SetGetFocusHdl( LINK( this, ScTPValidationValue, EditSetFocusHdl ) );
    m_btnRef.SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillFocusHdl ) );
    maEdMax.SetLoseFocusHdl( LINK( this, ScTPValidationValue, KillFocusHdl ) );

    maLbAllow.SelectEntryPos( SC_VALIDDLG_ALLOW_ANY );
    maLbValue.SelectEntryPos( SC_VALIDDLG_DATA_EQUAL );

    SelectHdl( NULL );
    CheckHdl( NULL );
}

SfxTabPage* ScTPValidationValue::Create( Window* pParent, const SfxItemSet& rArgSet )
{
    return( new ScTPValidationValue( pParent, rArgSet ) );
}

sal_uInt16* ScTPValidationValue::GetRanges()
{
    return pValueRanges;
}

void ScTPValidationValue::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    sal_uInt16 nLbPos = SC_VALIDDLG_ALLOW_ANY;
    if( rArgSet.GetItemState( FID_VALID_MODE, sal_True, &pItem ) == SFX_ITEM_SET )
        nLbPos = lclGetPosFromValMode( static_cast< ScValidationMode >(
            static_cast< const SfxAllEnumItem* >( pItem )->GetValue() ) );
    maLbAllow.SelectEntryPos( nLbPos );

    nLbPos = SC_VALIDDLG_DATA_EQUAL;
    if( rArgSet.GetItemState( FID_VALID_CONDMODE, sal_True, &pItem ) == SFX_ITEM_SET )
        nLbPos = lclGetPosFromCondMode( static_cast< ScConditionMode >(
            static_cast< const SfxAllEnumItem* >( pItem )->GetValue() ) );
    maLbValue.SelectEntryPos( nLbPos );

    // *** check boxes ***
    sal_Bool bCheck = sal_True;
    if( rArgSet.GetItemState( FID_VALID_BLANK, sal_True, &pItem ) == SFX_ITEM_SET )
        bCheck = static_cast< const SfxBoolItem* >( pItem )->GetValue();
    maCbAllow.Check( bCheck );

    sal_Int32 nListType = ValidListType::UNSORTED;
    if( rArgSet.GetItemState( FID_VALID_LISTTYPE, sal_True, &pItem ) == SFX_ITEM_SET )
        nListType = static_cast< const SfxInt16Item* >( pItem )->GetValue();
    maCbShow.Check( nListType != ValidListType::INVISIBLE );
    maCbSort.Check( nListType == ValidListType::SORTEDASCENDING );

    // *** formulas ***
    OUString aFmlaStr;
    if ( rArgSet.GetItemState( FID_VALID_VALUE1, sal_True, &pItem ) == SFX_ITEM_SET )
        aFmlaStr = static_cast< const SfxStringItem* >( pItem )->GetValue();
    SetFirstFormula( aFmlaStr );

    aFmlaStr= "";
    if ( rArgSet.GetItemState( FID_VALID_VALUE2, sal_True, &pItem ) == SFX_ITEM_SET )
        aFmlaStr = static_cast< const SfxStringItem* >( pItem )->GetValue();
    SetSecondFormula( aFmlaStr );

    SelectHdl( NULL );
    CheckHdl( NULL );
}

sal_Bool ScTPValidationValue::FillItemSet( SfxItemSet& rArgSet )
{
    sal_Int16 nListType = maCbShow.IsChecked() ?
        (maCbSort.IsChecked() ? ValidListType::SORTEDASCENDING : ValidListType::UNSORTED) :
        ValidListType::INVISIBLE;

    rArgSet.Put( SfxAllEnumItem( FID_VALID_MODE, sal::static_int_cast<sal_uInt16>(
                    lclGetValModeFromPos( maLbAllow.GetSelectEntryPos() ) ) ) );
    rArgSet.Put( SfxAllEnumItem( FID_VALID_CONDMODE, sal::static_int_cast<sal_uInt16>(
                    lclGetCondModeFromPos( maLbValue.GetSelectEntryPos() ) ) ) );
    rArgSet.Put( SfxStringItem( FID_VALID_VALUE1, GetFirstFormula() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_VALUE2, GetSecondFormula() ) );
    rArgSet.Put( SfxBoolItem( FID_VALID_BLANK, maCbAllow.IsChecked() ) );
    rArgSet.Put( SfxInt16Item( FID_VALID_LISTTYPE, nListType ) );
    return sal_True;
}

OUString ScTPValidationValue::GetFirstFormula() const
{
    OUString aFmlaStr;
    if( maLbAllow.GetSelectEntryPos() == SC_VALIDDLG_ALLOW_LIST )
        lclGetFormulaFromStringList( aFmlaStr, maEdList.GetText(), mcFmlaSep );
    else
        aFmlaStr = maEdMin.GetText();
    return aFmlaStr;
}

OUString ScTPValidationValue::GetSecondFormula() const
{
    return maEdMax.GetText();
}

void ScTPValidationValue::SetFirstFormula( const OUString& rFmlaStr )
{
    // try if formula is a string list, validation mode must already be set
    OUString aStringList;
    if( (maLbAllow.GetSelectEntryPos() == SC_VALIDDLG_ALLOW_RANGE) &&
        lclGetStringListFromFormula( aStringList, rFmlaStr, mcFmlaSep ) )
    {
        maEdList.SetText( aStringList );
        maEdMin.SetText( EMPTY_STRING );
        // change validation mode to string list
        maLbAllow.SelectEntryPos( SC_VALIDDLG_ALLOW_LIST );
    }
    else
    {
        maEdMin.SetText( rFmlaStr );
        maEdList.SetText( EMPTY_STRING );
    }
}

void ScTPValidationValue::SetSecondFormula( const OUString& rFmlaStr )
{
    maEdMax.SetText( rFmlaStr );
}

ScValidationDlg * ScTPValidationValue::GetValidationDlg()
{
    if( Window *pParent = GetParent() )
        do{
            if ( dynamic_cast<ScValidationDlg*>( pParent ) )
                return static_cast< ScValidationDlg * >( pParent );
        }while ( NULL != ( pParent = pParent->GetParent() ) );
    return NULL;
}

void ScTPValidationValue::SetupRefDlg()
{
    if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        if( pValidationDlg->SetupRefDlg() )
        {
            pValidationDlg->SetHandler( this );
            pValidationDlg->SetSetRefHdl( (ScRefHandlerHelper::PFUNCSETREFHDLTYPE)( &ScTPValidationValue::SetReferenceHdl ) );
            pValidationDlg->SetSetActHdl( (ScRefHandlerHelper::PCOMMONHDLTYPE)( &ScTPValidationValue::SetActiveHdl ) );
            pValidationDlg->SetRefInputStartPreHdl( (ScRefHandlerHelper::PINPUTSTARTDLTYPE)( &ScTPValidationValue::RefInputStartPreHdl ) );
            pValidationDlg->SetRefInputDonePostHdl( (ScRefHandlerHelper::PCOMMONHDLTYPE)( &ScTPValidationValue::RefInputDonePostHdl ) );

            Window *pLabel = NULL;

            if ( maEdMax.IsVisible() )
            {
                m_pRefEdit = &maEdMax;
                pLabel = &maFtMax;
            }
            else if ( maEdMin.IsVisible() )
            {
                m_pRefEdit = &maEdMin;
                pLabel = &maFtMin;
            }

            if( m_pRefEdit && !m_pRefEdit->HasFocus() )
                m_pRefEdit->GrabFocus();

            if( m_pRefEdit )
                m_pRefEdit->SetReferences( pValidationDlg, pLabel );

            m_btnRef.SetReferences( pValidationDlg, m_pRefEdit );
        }
    }
}

void ScTPValidationValue::RemoveRefDlg()
{
    if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
    {
        if( pValidationDlg->RemoveRefDlg() )
        {
            pValidationDlg->SetHandler( NULL );
            pValidationDlg->SetSetRefHdl( NULL );
            pValidationDlg->SetSetActHdl( NULL );
            pValidationDlg->SetRefInputStartPreHdl( NULL );
            pValidationDlg->SetRefInputDonePostHdl( NULL );

            if( m_pRefEdit )
                m_pRefEdit->SetReferences( NULL, NULL );
            m_pRefEdit = NULL;

            m_btnRef.SetReferences( NULL, NULL );

#if ! defined( WNT ) && !defined( _MSC_VER )
            TidyListBoxes();
#endif
        }
    }
}

void ScTPValidationValue::TidyListBoxes()
{
    if ( Window *pWnd = GetChild( 0 ) )
    {
        bool bFindLst = false;
        std::list<Window*> alstOrder;

        do{
            if( pWnd->GetParent() == this )
            {
                if ( !bFindLst )
                {
                    try{
                        if( dynamic_cast<ListBox*>(pWnd)||dynamic_cast<ListBox*>(pWnd->GetWindow(WINDOW_CLIENT) ) )
                            bFindLst = true;
                    }
                    catch( ... )
                    {
                        if ( *(void**)pWnd == *(void**)&maLbValue )
                            bFindLst = true;
                        else if ( Window *pClient = pWnd->GetWindow( WINDOW_CLIENT ) )
                            if ( *(void**)pClient == *(void**)&maLbValue )
                                bFindLst = true;
                    }
                }

                if ( bFindLst )
                    alstOrder.push_back( pWnd->GetWindow( WINDOW_CLIENT ) );
            }
        }while( NULL != ( pWnd = pWnd->GetWindow( WINDOW_NEXT  ) ) );

        pWnd = GetChild(0);

        while( std::find( alstOrder.begin(), alstOrder.end(), pWnd ) != alstOrder.end() && NULL != ( pWnd = pWnd->GetWindow( WINDOW_NEXT) ) ) ;

        if ( pWnd )
        {
            for ( std::list<Window*>::iterator i = alstOrder.begin(); i!=alstOrder.end(); ++i )
            {
                Window *pParent = (*i)->GetParent();
                (*i)->SetParent( pWnd );
                (*i)->SetParent( pParent );
            }
        }
    }
}

IMPL_LINK_NOARG(ScTPValidationValue, EditSetFocusHdl)
{
    sal_uInt16  nPos=maLbAllow.GetSelectEntryPos();

    if ( nPos == SC_VALIDDLG_ALLOW_RANGE )
    {
        SetupRefDlg();
    }

    return 0;
}

IMPL_LINK( ScTPValidationValue, KillFocusHdl, Window *, pWnd )
{
    if( pWnd == m_pRefEdit || pWnd == &m_btnRef )
        if( ScValidationDlg *pValidationDlg = GetValidationDlg() )
            if ( (pValidationDlg->IsActive() || pValidationDlg->IsChildFocus() ) && !pValidationDlg->IsRefInputting() )
                if( ( !m_pRefEdit || !m_pRefEdit->HasFocus()) && !m_btnRef.HasFocus() )
                {
                    RemoveRefDlg();
                }

    return 0;
}

// ----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScTPValidationValue, SelectHdl)
{
    sal_uInt16 nLbPos = maLbAllow.GetSelectEntryPos();
    bool bEnable = (nLbPos != SC_VALIDDLG_ALLOW_ANY);
    bool bRange = (nLbPos == SC_VALIDDLG_ALLOW_RANGE);
    bool bList = (nLbPos == SC_VALIDDLG_ALLOW_LIST);

    maCbAllow.Enable( bEnable );   // Empty cell
    maFtValue.Enable( bEnable );
    maLbValue.Enable( bEnable );
    maFtMin.Enable( bEnable );
    maEdMin.Enable( bEnable );
    maEdList.Enable( bEnable );
    maFtMax.Enable( bEnable );
    maEdMax.Enable( bEnable );

    bool bShowMax = false;
    if( bRange )
        maFtMin.SetText( maStrRange );
    else if( bList )
        maFtMin.SetText( maStrList );
    else
    {
        switch( maLbValue.GetSelectEntryPos() )
        {
            case SC_VALIDDLG_DATA_EQUAL:
            case SC_VALIDDLG_DATA_NOTEQUAL:     maFtMin.SetText( maStrValue );  break;

            case SC_VALIDDLG_DATA_LESS:
            case SC_VALIDDLG_DATA_EQLESS:       maFtMin.SetText( maStrMax );    break;

            case SC_VALIDDLG_DATA_VALIDRANGE:
            case SC_VALIDDLG_DATA_INVALIDRANGE:   bShowMax = true;    // fall through
            case SC_VALIDDLG_DATA_GREATER:
            case SC_VALIDDLG_DATA_EQGREATER:    maFtMin.SetText( maStrMin );    break;

            default:
                OSL_FAIL( "ScTPValidationValue::SelectHdl - unknown condition mode" );
        }
    }

    maCbShow.Show( bRange || bList );
    maCbSort.Show( bRange || bList );
    maFtValue.Show( !bRange && !bList );
    maLbValue.Show( !bRange && !bList );
    maEdMin.Show( !bList );
    maEdList.Show( bList );
    maFtMax.Show( bShowMax );
    maEdMax.Show( bShowMax );
    maFtHint.Show( bRange );
    m_btnRef.Show( bRange );  // cell range picker
    return 0;
}

IMPL_LINK_NOARG(ScTPValidationValue, CheckHdl)
{
    maCbSort.Enable( maCbShow.IsChecked() );
    return 0;
}


//========================================================================
//========================================================================
// Input Help Page

ScTPValidationHelp::ScTPValidationHelp( Window*         pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          "ValidationHelpTabPage" , "modules/scalc/ui/validationhelptabpage.ui" ,
                          rArgSet )
{
    get(pTsbHelp,"tsbhelp");
    get(pEdtTitle,"title");
    get(pEdInputHelp,"inputhelp");
    pEdInputHelp->set_height_request(pEdInputHelp->GetTextHeight() * 12);
    pEdInputHelp->set_width_request(pEdInputHelp->approximate_char_width() * 50);
    Init();
}

// -----------------------------------------------------------------------

ScTPValidationHelp::~ScTPValidationHelp()
{
}

// -----------------------------------------------------------------------

void ScTPValidationHelp::Init()
{
    pTsbHelp->EnableTriState( false );
}

//------------------------------------------------------------------------

sal_uInt16* ScTPValidationHelp::GetRanges()
{
    return pValueRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* ScTPValidationHelp::Create( Window* pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTPValidationHelp( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void ScTPValidationHelp::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet.GetItemState( FID_VALID_SHOWHELP, sal_True, &pItem ) == SFX_ITEM_SET )
        pTsbHelp->SetState( ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK );
    else
        pTsbHelp->SetState( STATE_NOCHECK );

    if ( rArgSet.GetItemState( FID_VALID_HELPTITLE, sal_True, &pItem ) == SFX_ITEM_SET )
        pEdtTitle->SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        pEdtTitle->SetText( EMPTY_STRING );

    if ( rArgSet.GetItemState( FID_VALID_HELPTEXT, sal_True, &pItem ) == SFX_ITEM_SET )
        pEdInputHelp->SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        pEdInputHelp->SetText( EMPTY_STRING );
}

// -----------------------------------------------------------------------

sal_Bool ScTPValidationHelp::FillItemSet( SfxItemSet& rArgSet )
{
    rArgSet.Put( SfxBoolItem( FID_VALID_SHOWHELP, pTsbHelp->GetState() == STATE_CHECK ) );
    rArgSet.Put( SfxStringItem( FID_VALID_HELPTITLE, pEdtTitle->GetText() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_HELPTEXT, pEdInputHelp->GetText() ) );

    return sal_True;
}

//========================================================================
//========================================================================
// Error Alert Page

ScTPValidationError::ScTPValidationError( Window*           pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          "ErrorAlertTabPage" , "modules/scalc/ui/erroralerttabpage.ui" ,
                          rArgSet )
{
    get(m_pTsbShow,"tsbshow");
    get(m_pLbAction,"actionCB");
    get(m_pBtnSearch,"browseBtn");
    get(m_pEdtTitle,"title");
    get(m_pFtError,"errormsg_label");
    get(m_pEdError,"errorMsg");
    m_pEdError->set_height_request(m_pEdError->GetTextHeight() * 12);
    m_pEdError->set_width_request(m_pEdError->approximate_char_width() * 50);
    Init();
}

// -----------------------------------------------------------------------

ScTPValidationError::~ScTPValidationError()
{
}

// -----------------------------------------------------------------------

void ScTPValidationError::Init()
{
    m_pLbAction->SetSelectHdl( LINK( this, ScTPValidationError, SelectActionHdl ) );
    m_pBtnSearch->SetClickHdl( LINK( this, ScTPValidationError, ClickSearchHdl ) );

    m_pLbAction->SelectEntryPos( 0 );
    m_pTsbShow->EnableTriState( false );

    SelectActionHdl( NULL );
}

//------------------------------------------------------------------------

sal_uInt16* ScTPValidationError::GetRanges()
{
    return pValueRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* ScTPValidationError::Create( Window*    pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTPValidationError( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void ScTPValidationError::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet.GetItemState( FID_VALID_SHOWERR, sal_True, &pItem ) == SFX_ITEM_SET )
        m_pTsbShow->SetState( ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK );
    else
        m_pTsbShow->SetState( STATE_CHECK );   // check by default

    if ( rArgSet.GetItemState( FID_VALID_ERRSTYLE, sal_True, &pItem ) == SFX_ITEM_SET )
        m_pLbAction->SelectEntryPos( ((const SfxAllEnumItem*)pItem)->GetValue() );
    else
        m_pLbAction->SelectEntryPos( 0 );

    if ( rArgSet.GetItemState( FID_VALID_ERRTITLE, sal_True, &pItem ) == SFX_ITEM_SET )
        m_pEdtTitle->SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        m_pEdtTitle->SetText( EMPTY_STRING );

    if ( rArgSet.GetItemState( FID_VALID_ERRTEXT, sal_True, &pItem ) == SFX_ITEM_SET )
        m_pEdError->SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        m_pEdError->SetText( EMPTY_STRING );

    SelectActionHdl( NULL );
}

// -----------------------------------------------------------------------

sal_Bool ScTPValidationError::FillItemSet( SfxItemSet& rArgSet )
{
    rArgSet.Put( SfxBoolItem( FID_VALID_SHOWERR, m_pTsbShow->GetState() == STATE_CHECK ) );
    rArgSet.Put( SfxAllEnumItem( FID_VALID_ERRSTYLE, m_pLbAction->GetSelectEntryPos() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_ERRTITLE, m_pEdtTitle->GetText() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_ERRTEXT, m_pEdError->GetText() ) );

    return sal_True;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ScTPValidationError, SelectActionHdl)
{
    ScValidErrorStyle eStyle = (ScValidErrorStyle) m_pLbAction->GetSelectEntryPos();
    sal_Bool bMacro = ( eStyle == SC_VALERR_MACRO );

    m_pBtnSearch->Enable( bMacro );
    m_pFtError->Enable( !bMacro );
    m_pEdError->Enable( !bMacro );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ScTPValidationError, ClickSearchHdl)
{
    Window* pOld = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );

    // Use static SfxApplication method to bring up selector dialog for
    // choosing a script
    OUString aScriptURL = SfxApplication::ChooseScript();

    Application::SetDefDialogParent( pOld );

    if ( aScriptURL != NULL && !aScriptURL.isEmpty() )
    {
        m_pEdtTitle->SetText( aScriptURL );
    }

    return( 0L );
}

bool ScValidationDlg::EnterRefStatus()
{
    ScTabViewShell *pTabViewShell = GetTabViewShell();

    if( !pTabViewShell ) return false;

    sal_uInt16 nId  = SLOTID;
    SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
    SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

    if ( pWnd && pWnd->GetWindow()!= this ) pWnd = NULL;

    SC_MOD()->SetRefDialog( nId, pWnd ? false : sal_True );

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

bool ScValidationDlg::RemoveRefDlg( sal_Bool bRestoreModal /* = sal_True */ )
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
            SetModal( sal_True );
    }

    if ( SfxChildWindow* pWnd = pTabVwSh->GetViewFrame()->GetChildWindow( SID_VALIDITY_REFERENCE ) )
    {
        static_cast<ScValidityRefChildWin*>(pWnd)->LockVisible( bVisLock );
        static_cast<ScValidityRefChildWin*>(pWnd)->LockFreeWindow( bFreeWindowLock );
    }

    return true;
}

void ScTPValidationValue::ScRefButtonEx::Click()
{
    if( ScTPValidationValue *pParent = dynamic_cast< ScTPValidationValue*>( GetParent() ) )
        pParent->OnClick( this );

    formula::RefButton::Click();
}

void ScTPValidationValue::OnClick( Button *pBtn )
{
    if( pBtn == &m_btnRef )
        SetupRefDlg();
}

sal_Bool ScValidationDlg::IsChildFocus()
{
    if ( const Window *pWin = Application::GetFocusWindow() )
        while( NULL != ( pWin = pWin->GetParent() ) )
            if( pWin == this )
                return sal_True;

    return false;
}


bool    ScValidationDlg::IsAlive()
{
    return SC_MOD()->IsAliveRefDlg( SLOTID, this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
