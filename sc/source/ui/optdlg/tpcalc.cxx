/*************************************************************************
 *
 *  $RCSfile: tpcalc.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 11:37:52 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <math.h>

#include "scitems.hxx"
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "globstr.hrc"
#include "uiitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "docoptio.hxx"
#include "scresid.hxx"
#include "sc.hrc"       // -> Slot-IDs
#include "optdlg.hrc"

#define _TPCALC_CXX
#include "tpcalc.hxx"
#undef _TPCALC_CXX

// STATIC DATA -----------------------------------------------------------

static USHORT pCalcOptRanges[] =
{
    SID_SCDOCOPTIONS,
    SID_SCDOCOPTIONS,
    0
};

//========================================================================

ScTpCalcOptions::ScTpCalcOptions( Window*           pParent,
                                  const SfxItemSet& rCoreAttrs )

    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_CALC ),
                          rCoreAttrs ),

        aBtnCase        ( this, ScResId( BTN_CASE ) ),
        aBtnCalc        ( this, ScResId( BTN_CALC ) ),
        aBtnMatch       ( this, ScResId( BTN_MATCH ) ),
        aBtnRegex       ( this, ScResId( BTN_REGEX ) ),
        aBtnLookUp      ( this, ScResId( BTN_LOOKUP ) ),
        aBtnIterate     ( this, ScResId( BTN_ITERATE ) ),
        aFtSteps        ( this, ScResId( FT_STEPS ) ),
        aEdSteps        ( this, ScResId( ED_STEPS ) ),
        aFtEps          ( this, ScResId( FT_EPS ) ),
        aEdEps          ( this, ScResId( ED_EPS ) ),
        aGbZRefs        ( this, ScResId( GB_ZREFS ) ),
        aBtnDateStd     ( this, ScResId( BTN_DATESTD ) ),
        aBtnDateSc10    ( this, ScResId( BTN_DATESC10 ) ),
        aBtnDate1904    ( this, ScResId( BTN_DATE1904 ) ),

        aGbDate         ( this, ScResId( GB_DATE ) ),
        aFtPrec         ( this, ScResId( FT_PREC ) ),
        aEdPrec         ( this, ScResId( ED_PREC ) ),
        aSeparatorFL    ( this, ScResId( FL_SEPARATOR ) ),
        aHSeparatorFL   ( this, ScResId( FL_H_SEPARATOR ) ),
        nWhichCalc      ( GetWhich( SID_SCDOCOPTIONS ) ),
        pOldOptions     ( new ScDocOptions(
                            ((const ScTpCalcItem&)rCoreAttrs.Get(
                                GetWhich( SID_SCDOCOPTIONS ))).
                                    GetDocOptions() ) ),
        pLocalOptions   ( new ScDocOptions )
{
    aSeparatorFL.SetStyle( aSeparatorFL.GetStyle() | WB_VERT );
    Init();
    FreeResource();
    SetExchangeSupport();
}

//-----------------------------------------------------------------------

__EXPORT ScTpCalcOptions::~ScTpCalcOptions()
{
    delete pOldOptions;
    delete pLocalOptions;
}

//-----------------------------------------------------------------------

void ScTpCalcOptions::Init()
{
    aBtnIterate .SetClickHdl( LINK( this, ScTpCalcOptions, CheckClickHdl ) );
    aBtnDateStd .SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    aBtnDateSc10.SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    aBtnDate1904.SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
}

//-----------------------------------------------------------------------

USHORT* __EXPORT ScTpCalcOptions::GetRanges()
{
    return pCalcOptRanges;
}

//-----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTpCalcOptions::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new ScTpCalcOptions( pParent, rAttrSet ) );
}

//-----------------------------------------------------------------------

void __EXPORT ScTpCalcOptions::Reset( const SfxItemSet& rCoreAttrs )
{
    USHORT  d,m,y;

    *pLocalOptions  = *pOldOptions;

    aBtnCase   .Check( !pLocalOptions->IsIgnoreCase() );
    aBtnCalc   .Check( pLocalOptions->IsCalcAsShown() );
    aBtnMatch  .Check( pLocalOptions->IsMatchWholeCell() );
    aBtnRegex  .Check( pLocalOptions->IsFormulaRegexEnabled() );
    aBtnLookUp .Check( pLocalOptions->IsLookUpColRowNames() );
    aBtnIterate.Check( pLocalOptions->IsIter() );
    aEdSteps   .SetValue( pLocalOptions->GetIterCount() );
    aEdPrec    .SetValue( pLocalOptions->GetStdPrecision() );
    aEdEps     .SetValue( pLocalOptions->GetIterEps(), 6 );

    pLocalOptions->GetDate( d, m, y );

    switch ( y )
    {
        case 1899:
            aBtnDateStd.Check();
            break;
        case 1900:
            aBtnDateSc10.Check();
            break;
        case 1904:
            aBtnDate1904.Check();
            break;
    }

    CheckClickHdl( &aBtnIterate );
}


//-----------------------------------------------------------------------

BOOL __EXPORT ScTpCalcOptions::FillItemSet( SfxItemSet& rCoreAttrs )
{
    // alle weiteren Optionen werden in den Handlern aktualisiert
    pLocalOptions->SetIterCount( (USHORT)aEdSteps.GetValue() );
    pLocalOptions->SetStdPrecision( (USHORT)aEdPrec.GetValue() );
    pLocalOptions->SetIgnoreCase( !aBtnCase.IsChecked() );
    pLocalOptions->SetCalcAsShown( aBtnCalc.IsChecked() );
    pLocalOptions->SetMatchWholeCell( aBtnMatch.IsChecked() );
    pLocalOptions->SetFormulaRegexEnabled( aBtnRegex.IsChecked() );
    pLocalOptions->SetLookUpColRowNames( aBtnLookUp.IsChecked() );

    if ( *pLocalOptions != *pOldOptions )
    {
        rCoreAttrs.Put( ScTpCalcItem( nWhichCalc, *pLocalOptions ) );
        return TRUE;
    }
    else
        return FALSE;
}

//------------------------------------------------------------------------

int __EXPORT ScTpCalcOptions::DeactivatePage( SfxItemSet* pSet )
{
    int nReturn = KEEP_PAGE;

    double fEps;
    if( aEdEps.GetValue( fEps ) && (fEps > 0.0) )
    {
        pLocalOptions->SetIterEps( fEps );
        nReturn = LEAVE_PAGE;
    }

    if ( nReturn == KEEP_PAGE )
    {
        ErrorBox( this,
                  WinBits( WB_OK | WB_DEF_OK ),
                  ScGlobal::GetRscString( STR_INVALID_EPS )
                ).Execute();

        aEdEps.GrabFocus();
    }
    else if ( pSet )
        FillItemSet( *pSet );

    return nReturn;
}

//-----------------------------------------------------------------------
// Handler:

IMPL_LINK( ScTpCalcOptions, RadioClickHdl, RadioButton*, pBtn )
{
    if ( pBtn == &aBtnDateStd )
    {
        pLocalOptions->SetDate( 30, 12, 1899 );
    }
    else if ( pBtn == &aBtnDateSc10 )
    {
        pLocalOptions->SetDate( 1, 1, 1900 );
    }
    else if ( pBtn == &aBtnDate1904 )
    {
        pLocalOptions->SetDate( 1, 1, 1904 );
    }

    return 0;
}

//-----------------------------------------------------------------------

IMPL_LINK(  ScTpCalcOptions, CheckClickHdl, CheckBox*, pBtn )
{
    if ( pBtn->IsChecked() )
    {
        pLocalOptions->SetIter( TRUE );
        aFtSteps.Enable();  aEdSteps.Enable();
        aFtEps  .Enable();  aEdEps  .Enable();
    }
    else
    {
        pLocalOptions->SetIter( FALSE );
        aFtSteps.Disable(); aEdSteps.Disable();
        aFtEps  .Disable(); aEdEps  .Disable();
    }

    return 0;
}




