/*************************************************************************
 *
 *  $RCSfile: validate.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:44:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <vcl/svapp.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <tools/urlobj.hxx>
#include <offmgr/app.hxx>

#include "scresid.hxx"
#include "sc.hrc"       // -> Slot IDs

#include "validat.hxx"
#include "validate.hrc"
#include "validate.hxx"
#include "compiler.hxx"


// ============================================================================

static USHORT pValueRanges[] =
{
    FID_VALID_MODE, FID_VALID_ERRTEXT,
    FID_VALID_LISTTYPE, FID_VALID_LISTTYPE,
    0
};

static USHORT pHelpRanges[] =
{
    FID_VALIDATION,
    FID_VALIDATION,
    0
};

static USHORT pErrorRanges[] =
{
    FID_VALIDATION,
    FID_VALIDATION,
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
        aToken.SearchAndReplaceAllAscii( "\"", String( RTL_CONSTASCII_USTRINGPARAM( "\"\"" ) ) );
        ScGlobal::AddQuotes( aToken );
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
            bIsStringList = ScGlobal::IsQuoted( aToken );
            if( bIsStringList )
            {
                ScGlobal::EraseQuotes( aToken );
                aToken.SearchAndReplaceAllAscii( "\"\"", String( '"' ) );
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
    const String& rListSep = ScCompiler::pSymbolTableNative[ ocSep ];
    DBG_ASSERT( rListSep.Len() == 1, "ScTPValidationValue::ScTPValidationValue - list separator error" );
    mcFmlaSep = rListSep.Len() ? rListSep.GetChar( 0 ) : ';';
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

    rArgSet.Put( SfxAllEnumItem( FID_VALID_MODE, lclGetValModeFromPos( maLbAllow.GetSelectEntryPos() ) ) );
    rArgSet.Put( SfxAllEnumItem( FID_VALID_CONDMODE, lclGetCondModeFromPos( maLbValue.GetSelectEntryPos() ) ) );
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

IMPL_LINK( ScTPValidationValue, SelectHdl, ListBox*, pListBox )
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

IMPL_LINK( ScTPValidationValue, CheckHdl, CheckBox*, pCheckBox )
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
        aFtTitle        ( this, ScResId( FT_TITLE ) ),
        aEdtTitle       ( this, ScResId( EDT_TITLE ) ),
        aFtInputHelp    ( this, ScResId( FT_INPUTHELP ) ),
        aEdInputHelp    ( this, ScResId( EDT_INPUTHELP ) ),
        aFlContent      ( this, ScResId( FL_CONTENT ) ),

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
        aFtAction       ( this, ScResId( FT_ACTION ) ),
        aLbAction       ( this, ScResId( LB_ACTION ) ),
        aBtnSearch      ( this, ScResId( BTN_SEARCH ) ),
        aFtTitle        ( this, ScResId( FT_TITLE ) ),
        aEdtTitle       ( this, ScResId( EDT_TITLE ) ),
        aFtError        ( this, ScResId( FT_ERROR ) ),
        aEdError        ( this, ScResId( EDT_ERROR ) ),
        aFlContent      ( this, ScResId( FL_CONTENT ) ),

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

IMPL_LINK( ScTPValidationError, SelectActionHdl, ListBox*, pListBox )
{
    ScValidErrorStyle eStyle = (ScValidErrorStyle) aLbAction.GetSelectEntryPos();
    BOOL bMacro = ( eStyle == SC_VALERR_MACRO );

    aBtnSearch.Enable( bMacro );
    aFtError.Enable( !bMacro );
    aEdError.Enable( !bMacro );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTPValidationError, ClickSearchHdl, PushButton*, pBtn )
{
    Window* pOld = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );

    // choose macro dialog
    String aScriptURL = OfficeApplication::ChooseMacro(FALSE, TRUE);

    Application::SetDefDialogParent( pOld );

    // aScriptURL has the following format:
    // vnd.sun.star.script:language=[language],macro=[macro],location=[location]
    // [language] = StarBasic
    // [macro] = libname.modulename.macroname
    // [location] = application|document
    // e.g. 'vnd.sun.star.script:language=StarBasic,macro=Standard.Module1.Main,location=document'
    //
    // but for the UI we need this format:
    // 'macroname'

    if ( aScriptURL.Len() != 0 )
    {
        // parse script URL
        BOOL bFound;
        String aValue;
        INetURLObject aINetScriptURL( aScriptURL );

        // get language
        String aLanguage;
        bFound = aINetScriptURL.getParameter( String( RTL_CONSTASCII_USTRINGPARAM("language") ), &aValue );
        if ( bFound )
            aLanguage = aValue;

        // get macro
        String aMacro;
        String aLibName;
        String aModuleName;
        String aMacroName;
        bFound = aINetScriptURL.getParameter( String( RTL_CONSTASCII_USTRINGPARAM("macro") ), &aValue );
        if ( bFound )
        {
            aMacro = aValue;
            aLibName    = aMacro.GetToken(0, sal_Unicode('.'));
            aModuleName = aMacro.GetToken(1, sal_Unicode('.'));
            aMacroName  = aMacro.GetToken(2, sal_Unicode('.'));
        }

        // get location
        String aLocation;
        bFound = aINetScriptURL.getParameter( String( RTL_CONSTASCII_USTRINGPARAM("location") ), &aValue );
        if ( bFound )
            aLocation = aValue;

        aEdtTitle.SetText( aMacroName );
    }

    return( 0L );
}

