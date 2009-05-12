/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: validate.cxx,v $
 * $Revision: 1.18 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif

#include <vcl/svapp.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <sfx2/app.hxx>

#include "scresid.hxx"
#include "sc.hrc"       // -> Slot IDs

#include "validat.hxx"
#include "validate.hrc"
#include "validate.hxx"
#include "compiler.hxx"
#include "formula/opcode.hxx" //CHINA001

// ============================================================================

static USHORT pValueRanges[] =
{
    FID_VALID_MODE, FID_VALID_ERRTEXT,
    FID_VALID_LISTTYPE, FID_VALID_LISTTYPE,
    0
};


// ============================================================================

ScValidationDlg::ScValidationDlg( Window* pParent, const SfxItemSet* pArgSet ) :
    SfxTabDialog( pParent, ScResId( TAB_DLG_VALIDATION ), pArgSet )
{
    AddTabPage( TP_VALIDATION_VALUES,    ScTPValidationValue::Create, 0 );
    AddTabPage( TP_VALIDATION_INPUTHELP, ScTPValidationHelp::Create,  0 );
    AddTabPage( TP_VALIDATION_ERROR,     ScTPValidationError::Create, 0 );
    FreeResource();
}

ScValidationDlg::~ScValidationDlg()
{
}


// ============================================================================

namespace {

/** Converts the passed ScValidationMode to the position in the list box. */
USHORT lclGetPosFromValMode( ScValidationMode eValMode )
{
    USHORT nLbPos = SC_VALIDDLG_ALLOW_ANY;
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
        default:    DBG_ERRORFILE( "lclGetPosFromValMode - unknown validity mode" );
    }
    return nLbPos;
}

/** Converts the passed list box position to an ScValidationMode. */
ScValidationMode lclGetValModeFromPos( USHORT nLbPos )
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
        default:    DBG_ERRORFILE( "lclGetValModeFromPos - invalid list box position" );
    }
    return eValMode;
}

/** Converts the passed ScConditionMode to the position in the list box. */
USHORT lclGetPosFromCondMode( ScConditionMode eCondMode )
{
    USHORT nLbPos = SC_VALIDDLG_DATA_EQUAL;
    switch( eCondMode )
    {
        case SC_COND_NONE:          // #111771# may occur in old XML files after Excel import
        case SC_COND_EQUAL:         nLbPos = SC_VALIDDLG_DATA_EQUAL;        break;
        case SC_COND_LESS:          nLbPos = SC_VALIDDLG_DATA_LESS;         break;
        case SC_COND_GREATER:       nLbPos = SC_VALIDDLG_DATA_GREATER;      break;
        case SC_COND_EQLESS:        nLbPos = SC_VALIDDLG_DATA_EQLESS;       break;
        case SC_COND_EQGREATER:     nLbPos = SC_VALIDDLG_DATA_EQGREATER;    break;
        case SC_COND_NOTEQUAL:      nLbPos = SC_VALIDDLG_DATA_NOTEQUAL;     break;
        case SC_COND_BETWEEN:       nLbPos = SC_VALIDDLG_DATA_BETWEEN;      break;
        case SC_COND_NOTBETWEEN:    nLbPos = SC_VALIDDLG_DATA_NOTBETWEEN;   break;
        default:    DBG_ERRORFILE( "lclGetPosFromCondMode - unknown condition mode" );
    }
    return nLbPos;
}

/** Converts the passed list box position to an ScConditionMode. */
ScConditionMode lclGetCondModeFromPos( USHORT nLbPos )
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
        case SC_VALIDDLG_DATA_BETWEEN:      eCondMode = SC_COND_BETWEEN;    break;
        case SC_VALIDDLG_DATA_NOTBETWEEN:   eCondMode = SC_COND_NOTBETWEEN; break;
        default:    DBG_ERRORFILE( "lclGetCondModeFromPos - invalid list box position" );
    }
    return eCondMode;
}

/** Converts line feed separated string to a formula with strings separated by semicolons.
    @descr  Keeps all empty strings.
    Example: abc\ndef\n\nghi -> "abc";"def";"";"ghi".
    @param rFmlaStr  (out-param) The converted formula string. */
void lclGetFormulaFromStringList( String& rFmlaStr, const String& rStringList, sal_Unicode cFmlaSep )
{
    rFmlaStr.Erase();
    xub_StrLen nTokenCnt = rStringList.GetTokenCount( '\n' );
    for( xub_StrLen nToken = 0, nStringIx = 0; nToken < nTokenCnt; ++nToken )
    {
        String aToken( rStringList.GetToken( 0, '\n', nStringIx ) );
        ScGlobal::AddQuotes( aToken, '"' );
        ScGlobal::AddToken( rFmlaStr, aToken, cFmlaSep );
    }
    if( !rFmlaStr.Len() )
        rFmlaStr.AssignAscii( "\"\"" );
}


/** Converts formula with strings separated by semicolons to line feed separated string.
    @descr  Keeps all empty strings. Ignores all empty tokens (multiple semicolons).
    Example: "abc";;;"def";"";"ghi" -> abc\ndef\n\nghi.
    @param rStringList  (out-param) The converted line feed separated string list.
    @return  true = Conversion successful. */
bool lclGetStringListFromFormula( String& rStringList, const String& rFmlaStr, sal_Unicode cFmlaSep )
{
    String aQuotes( RTL_CONSTASCII_USTRINGPARAM( "\"\"" ) );
    xub_StrLen nTokenCnt = rFmlaStr.GetQuotedTokenCount( aQuotes, cFmlaSep );

    rStringList.Erase();
    bool bIsStringList = (nTokenCnt > 0);
    bool bTokenAdded = false;

    for( xub_StrLen nToken = 0, nStringIx = 0; bIsStringList && (nToken < nTokenCnt); ++nToken )
    {
        String aToken( rFmlaStr.GetQuotedToken( 0, aQuotes, cFmlaSep, nStringIx ) );
        aToken.EraseLeadingAndTrailingChars();
        if( aToken.Len() )      // ignore empty tokens, i.e. "a";;"b"
        {
            bIsStringList = ScGlobal::IsQuoted( aToken, '"' );
            if( bIsStringList )
            {
                ScGlobal::EraseQuotes( aToken, '"' );
                ScGlobal::AddToken( rStringList, aToken, '\n', 1, bTokenAdded );
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
    maEdMin   ( this, ScResId( EDT_MIN ) ),
    maEdList  ( this, ScResId( EDT_LIST ) ),
    maFtMax   ( this, ScResId( FT_MAX ) ),
    maEdMax   ( this, ScResId( EDT_MAX ) ),
    maFtHint  ( this, ScResId( FT_SOURCEHINT ) ),
    maStrMin  ( ScResId( SCSTR_VALID_MINIMUM ) ),
    maStrMax  ( ScResId( SCSTR_VALID_MAXIMUM ) ),
    maStrValue( ScResId( SCSTR_VALID_VALUE ) ),
    maStrRange( ScResId( SCSTR_VALID_RANGE ) ),
    maStrList ( ScResId( SCSTR_VALID_LIST ) )
{
    Init();
    FreeResource();

    // list separator in formulas
    //CHINA001 const String& rListSep = ScCompiler::pSymbolTableNative[ ocSep ];
    String aListSep = ::GetScCompilerNativeSymbol( ocSep ); //CHINA001
    DBG_ASSERT( aListSep.Len() == 1, "ScTPValidationValue::ScTPValidationValue - list separator error" );
    mcFmlaSep = aListSep.Len() ? aListSep.GetChar( 0 ) : ';';
}

ScTPValidationValue::~ScTPValidationValue()
{
}

void ScTPValidationValue::Init()
{
    maLbAllow.SetSelectHdl( LINK( this, ScTPValidationValue, SelectHdl ) );
    maLbValue.SetSelectHdl( LINK( this, ScTPValidationValue, SelectHdl ) );
    maCbShow.SetClickHdl( LINK( this, ScTPValidationValue, CheckHdl ) );

    maLbAllow.SelectEntryPos( SC_VALIDDLG_ALLOW_ANY );
    maLbValue.SelectEntryPos( SC_VALIDDLG_DATA_EQUAL );

    SelectHdl( NULL );
    CheckHdl( NULL );
}

SfxTabPage* ScTPValidationValue::Create( Window* pParent, const SfxItemSet& rArgSet )
{
    return( new ScTPValidationValue( pParent, rArgSet ) );
}

USHORT* ScTPValidationValue::GetRanges()
{
    return pValueRanges;
}

void ScTPValidationValue::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    USHORT nLbPos = SC_VALIDDLG_ALLOW_ANY;
    if( rArgSet.GetItemState( FID_VALID_MODE, TRUE, &pItem ) == SFX_ITEM_SET )
        nLbPos = lclGetPosFromValMode( static_cast< ScValidationMode >(
            static_cast< const SfxAllEnumItem* >( pItem )->GetValue() ) );
    maLbAllow.SelectEntryPos( nLbPos );

    nLbPos = SC_VALIDDLG_DATA_EQUAL;
    if( rArgSet.GetItemState( FID_VALID_CONDMODE, TRUE, &pItem ) == SFX_ITEM_SET )
        nLbPos = lclGetPosFromCondMode( static_cast< ScConditionMode >(
            static_cast< const SfxAllEnumItem* >( pItem )->GetValue() ) );
    maLbValue.SelectEntryPos( nLbPos );

    // *** check boxes ***
    BOOL bCheck = TRUE;
    if( rArgSet.GetItemState( FID_VALID_BLANK, TRUE, &pItem ) == SFX_ITEM_SET )
        bCheck = static_cast< const SfxBoolItem* >( pItem )->GetValue();
    maCbAllow.Check( bCheck );

    sal_Int32 nListType = ValidListType::UNSORTED;
    if( rArgSet.GetItemState( FID_VALID_LISTTYPE, TRUE, &pItem ) == SFX_ITEM_SET )
        nListType = static_cast< const SfxInt16Item* >( pItem )->GetValue();
    maCbShow.Check( nListType != ValidListType::INVISIBLE );
    maCbSort.Check( nListType == ValidListType::SORTEDASCENDING );

    // *** formulas ***
    String aFmlaStr;
    if ( rArgSet.GetItemState( FID_VALID_VALUE1, TRUE, &pItem ) == SFX_ITEM_SET )
        aFmlaStr = static_cast< const SfxStringItem* >( pItem )->GetValue();
    SetFirstFormula( aFmlaStr );

    aFmlaStr.Erase();
    if ( rArgSet.GetItemState( FID_VALID_VALUE2, TRUE, &pItem ) == SFX_ITEM_SET )
        aFmlaStr = static_cast< const SfxStringItem* >( pItem )->GetValue();
    SetSecondFormula( aFmlaStr );

    SelectHdl( NULL );
    CheckHdl( NULL );
}

BOOL ScTPValidationValue::FillItemSet( SfxItemSet& rArgSet )
{
    sal_Int16 nListType = maCbShow.IsChecked() ?
        (maCbSort.IsChecked() ? ValidListType::SORTEDASCENDING : ValidListType::UNSORTED) :
        ValidListType::INVISIBLE;

    rArgSet.Put( SfxAllEnumItem( FID_VALID_MODE, sal::static_int_cast<USHORT>(
                    lclGetValModeFromPos( maLbAllow.GetSelectEntryPos() ) ) ) );
    rArgSet.Put( SfxAllEnumItem( FID_VALID_CONDMODE, sal::static_int_cast<USHORT>(
                    lclGetCondModeFromPos( maLbValue.GetSelectEntryPos() ) ) ) );
    rArgSet.Put( SfxStringItem( FID_VALID_VALUE1, GetFirstFormula() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_VALUE2, GetSecondFormula() ) );
    rArgSet.Put( SfxBoolItem( FID_VALID_BLANK, maCbAllow.IsChecked() ) );
    rArgSet.Put( SfxInt16Item( FID_VALID_LISTTYPE, nListType ) );
    return TRUE;
}

String ScTPValidationValue::GetFirstFormula() const
{
    String aFmlaStr;
    if( maLbAllow.GetSelectEntryPos() == SC_VALIDDLG_ALLOW_LIST )
        lclGetFormulaFromStringList( aFmlaStr, maEdList.GetText(), mcFmlaSep );
    else
        aFmlaStr = maEdMin.GetText();
    return aFmlaStr;
}

String ScTPValidationValue::GetSecondFormula() const
{
    return maEdMax.GetText();
}

void ScTPValidationValue::SetFirstFormula( const String& rFmlaStr )
{
    // try if formula is a string list, validation mode must already be set
    String aStringList;
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

void ScTPValidationValue::SetSecondFormula( const String& rFmlaStr )
{
    maEdMax.SetText( rFmlaStr );
}


// ----------------------------------------------------------------------------

IMPL_LINK( ScTPValidationValue, SelectHdl, ListBox*, EMPTYARG )
{
    USHORT nLbPos = maLbAllow.GetSelectEntryPos();
    bool bEnable = (nLbPos != SC_VALIDDLG_ALLOW_ANY);
    bool bRange = (nLbPos == SC_VALIDDLG_ALLOW_RANGE);
    bool bList = (nLbPos == SC_VALIDDLG_ALLOW_LIST);

    maCbAllow.Enable( bEnable );   // Leerzellen
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

            case SC_VALIDDLG_DATA_BETWEEN:
            case SC_VALIDDLG_DATA_NOTBETWEEN:   bShowMax = true;    // fall through
            case SC_VALIDDLG_DATA_GREATER:
            case SC_VALIDDLG_DATA_EQGREATER:    maFtMin.SetText( maStrMin );    break;

            default:
                DBG_ERRORFILE( "ScTPValidationValue::SelectHdl - unknown condition mode" );
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

    return 0;
}

IMPL_LINK( ScTPValidationValue, CheckHdl, CheckBox*, EMPTYARG )
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
                          ScResId( TP_VALIDATION_INPUTHELP ),
                          rArgSet ),
        aTsbHelp        ( this, ScResId( TSB_HELP ) ),
        aFlContent      ( this, ScResId( FL_CONTENT ) ),
        aFtTitle        ( this, ScResId( FT_TITLE ) ),
        aEdtTitle       ( this, ScResId( EDT_TITLE ) ),
        aFtInputHelp    ( this, ScResId( FT_INPUTHELP ) ),
        aEdInputHelp    ( this, ScResId( EDT_INPUTHELP ) ),

        mrArgSet            ( rArgSet )
{
    Init();
    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScTPValidationHelp::~ScTPValidationHelp()
{
}

// -----------------------------------------------------------------------

void ScTPValidationHelp::Init()
{
    //aLb.SetSelectHdl( LINK( this, ScTPValidationHelp, SelectHdl ) );

    aTsbHelp.EnableTriState( FALSE );
}

//------------------------------------------------------------------------

USHORT* __EXPORT ScTPValidationHelp::GetRanges()
{
    return pValueRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTPValidationHelp::Create( Window*    pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTPValidationHelp( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void __EXPORT ScTPValidationHelp::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet.GetItemState( FID_VALID_SHOWHELP, TRUE, &pItem ) == SFX_ITEM_SET )
        aTsbHelp.SetState( ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK );
    else
        aTsbHelp.SetState( STATE_NOCHECK );

    if ( rArgSet.GetItemState( FID_VALID_HELPTITLE, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdtTitle.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        aEdtTitle.SetText( EMPTY_STRING );

    if ( rArgSet.GetItemState( FID_VALID_HELPTEXT, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdInputHelp.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        aEdInputHelp.SetText( EMPTY_STRING );
}

// -----------------------------------------------------------------------

BOOL __EXPORT ScTPValidationHelp::FillItemSet( SfxItemSet& rArgSet )
{
    rArgSet.Put( SfxBoolItem( FID_VALID_SHOWHELP, aTsbHelp.GetState() == STATE_CHECK ) );
    rArgSet.Put( SfxStringItem( FID_VALID_HELPTITLE, aEdtTitle.GetText() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_HELPTEXT, aEdInputHelp.GetText() ) );

    return TRUE;
}

//========================================================================
//========================================================================
// Error Alert Page

ScTPValidationError::ScTPValidationError( Window*           pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( TP_VALIDATION_ERROR ),
                          rArgSet ),
        aTsbShow        ( this, ScResId( TSB_SHOW ) ),
        aFlContent      ( this, ScResId( FL_CONTENT ) ),
        aFtAction       ( this, ScResId( FT_ACTION ) ),
        aLbAction       ( this, ScResId( LB_ACTION ) ),
        aBtnSearch      ( this, ScResId( BTN_SEARCH ) ),
        aFtTitle        ( this, ScResId( FT_TITLE ) ),
        aEdtTitle       ( this, ScResId( EDT_TITLE ) ),
        aFtError        ( this, ScResId( FT_ERROR ) ),
        aEdError        ( this, ScResId( EDT_ERROR ) ),

        mrArgSet            ( rArgSet )
{
    Init();
    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScTPValidationError::~ScTPValidationError()
{
}

// -----------------------------------------------------------------------

void ScTPValidationError::Init()
{
    aLbAction.SetSelectHdl( LINK( this, ScTPValidationError, SelectActionHdl ) );
    aBtnSearch.SetClickHdl( LINK( this, ScTPValidationError, ClickSearchHdl ) );

    aLbAction.SelectEntryPos( 0 );
    aTsbShow.EnableTriState( FALSE );

    SelectActionHdl( NULL );
}

//------------------------------------------------------------------------

USHORT* __EXPORT ScTPValidationError::GetRanges()
{
    return pValueRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTPValidationError::Create( Window*   pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTPValidationError( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void __EXPORT ScTPValidationError::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet.GetItemState( FID_VALID_SHOWERR, TRUE, &pItem ) == SFX_ITEM_SET )
        aTsbShow.SetState( ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK );
    else
        aTsbShow.SetState( STATE_CHECK );   // #111720# check by default

    if ( rArgSet.GetItemState( FID_VALID_ERRSTYLE, TRUE, &pItem ) == SFX_ITEM_SET )
        aLbAction.SelectEntryPos( ((const SfxAllEnumItem*)pItem)->GetValue() );
    else
        aLbAction.SelectEntryPos( 0 );

    if ( rArgSet.GetItemState( FID_VALID_ERRTITLE, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdtTitle.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        aEdtTitle.SetText( EMPTY_STRING );

    if ( rArgSet.GetItemState( FID_VALID_ERRTEXT, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdError.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    else
        aEdError.SetText( EMPTY_STRING );

    SelectActionHdl( NULL );
}

// -----------------------------------------------------------------------

BOOL __EXPORT ScTPValidationError::FillItemSet( SfxItemSet& rArgSet )
{
    rArgSet.Put( SfxBoolItem( FID_VALID_SHOWERR, aTsbShow.GetState() == STATE_CHECK ) );
    rArgSet.Put( SfxAllEnumItem( FID_VALID_ERRSTYLE, aLbAction.GetSelectEntryPos() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_ERRTITLE, aEdtTitle.GetText() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_ERRTEXT, aEdError.GetText() ) );

    return TRUE;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTPValidationError, SelectActionHdl, ListBox*, EMPTYARG )
{
    ScValidErrorStyle eStyle = (ScValidErrorStyle) aLbAction.GetSelectEntryPos();
    BOOL bMacro = ( eStyle == SC_VALERR_MACRO );

    aBtnSearch.Enable( bMacro );
    aFtError.Enable( !bMacro );
    aEdError.Enable( !bMacro );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTPValidationError, ClickSearchHdl, PushButton*, EMPTYARG )
{
    Window* pOld = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );

    // Use static SfxApplication method to bring up selector dialog for
    // choosing a script
    ::rtl::OUString aScriptURL = SfxApplication::ChooseScript();

    Application::SetDefDialogParent( pOld );

    if ( aScriptURL != NULL && aScriptURL.getLength() != 0 )
    {
        aEdtTitle.SetText( aScriptURL );
    }

    return( 0L );
}

