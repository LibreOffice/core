/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

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

#include <math.h>

// STATIC DATA -----------------------------------------------------------

static sal_uInt16 pCalcOptRanges[] =
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

        aGbZRefs        ( this, ScResId( GB_ZREFS ) ),
        aBtnIterate     ( this, ScResId( BTN_ITERATE ) ),
        aFtSteps        ( this, ScResId( FT_STEPS ) ),
        aEdSteps        ( this, ScResId( ED_STEPS ) ),
        aFtEps          ( this, ScResId( FT_EPS ) ),
        aEdEps          ( this, ScResId( ED_EPS ) ),
        aSeparatorFL    ( this, ScResId( FL_SEPARATOR ) ),
        aGbDate         ( this, ScResId( GB_DATE ) ),
        aBtnDateStd     ( this, ScResId( BTN_DATESTD ) ),
        aBtnDateSc10    ( this, ScResId( BTN_DATESC10 ) ),
        aBtnDate1904    ( this, ScResId( BTN_DATE1904 ) ),
        aHSeparatorFL   ( this, ScResId( FL_H_SEPARATOR ) ),
        aBtnCase        ( this, ScResId( BTN_CASE ) ),
        aBtnCalc        ( this, ScResId( BTN_CALC ) ),
        aBtnMatch       ( this, ScResId( BTN_MATCH ) ),
        aBtnRegex       ( this, ScResId( BTN_REGEX ) ),
        aBtnLookUp      ( this, ScResId( BTN_LOOKUP ) ),
        aBtnGeneralPrec ( this, ScResId( BTN_GENERAL_PREC ) ),
        aFtPrec         ( this, ScResId( FT_PREC ) ),
        aEdPrec         ( this, ScResId( ED_PREC ) ),
        pOldOptions     ( new ScDocOptions(
                            ((const ScTpCalcItem&)rCoreAttrs.Get(
                                GetWhich( SID_SCDOCOPTIONS ))).
                                    GetDocOptions() ) ),
        pLocalOptions   ( new ScDocOptions ),
        nWhichCalc      ( GetWhich( SID_SCDOCOPTIONS ) )
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
    aBtnGeneralPrec.SetClickHdl( LINK(this, ScTpCalcOptions, CheckClickHdl) );
    aBtnDateStd .SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    aBtnDateSc10.SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
    aBtnDate1904.SetClickHdl( LINK( this, ScTpCalcOptions, RadioClickHdl ) );
}

//-----------------------------------------------------------------------

sal_uInt16* __EXPORT ScTpCalcOptions::GetRanges()
{
    return pCalcOptRanges;
}

//-----------------------------------------------------------------------

SfxTabPage* __EXPORT ScTpCalcOptions::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new ScTpCalcOptions( pParent, rAttrSet ) );
}

//-----------------------------------------------------------------------

void __EXPORT ScTpCalcOptions::Reset( const SfxItemSet& /* rCoreAttrs */ )
{
    sal_uInt16  d,m,y;

    *pLocalOptions  = *pOldOptions;

    aBtnCase   .Check( !pLocalOptions->IsIgnoreCase() );
    aBtnCalc   .Check( pLocalOptions->IsCalcAsShown() );
    aBtnMatch  .Check( pLocalOptions->IsMatchWholeCell() );
    aBtnRegex  .Check( pLocalOptions->IsFormulaRegexEnabled() );
    aBtnLookUp .Check( pLocalOptions->IsLookUpColRowNames() );
    aBtnIterate.Check( pLocalOptions->IsIter() );
    aEdSteps   .SetValue( pLocalOptions->GetIterCount() );
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

    sal_uInt16 nPrec = pLocalOptions->GetStdPrecision();
    if (nPrec == SvNumberFormatter::UNLIMITED_PRECISION)
    {
        aFtPrec.Disable();
        aEdPrec.Disable();
        aBtnGeneralPrec.Check(false);
    }
    else
    {
        aBtnGeneralPrec.Check();
        aFtPrec.Enable();
        aEdPrec.Enable();
        aEdPrec.SetValue(nPrec);
    }

    CheckClickHdl( &aBtnIterate );
}


//-----------------------------------------------------------------------

sal_Bool __EXPORT ScTpCalcOptions::FillItemSet( SfxItemSet& rCoreAttrs )
{
    // alle weiteren Optionen werden in den Handlern aktualisiert
    pLocalOptions->SetIterCount( (sal_uInt16)aEdSteps.GetValue() );
    pLocalOptions->SetIgnoreCase( !aBtnCase.IsChecked() );
    pLocalOptions->SetCalcAsShown( aBtnCalc.IsChecked() );
    pLocalOptions->SetMatchWholeCell( aBtnMatch.IsChecked() );
    pLocalOptions->SetFormulaRegexEnabled( aBtnRegex.IsChecked() );
    pLocalOptions->SetLookUpColRowNames( aBtnLookUp.IsChecked() );

    if (aBtnGeneralPrec.IsChecked())
        pLocalOptions->SetStdPrecision(
            static_cast<sal_uInt16>(aEdPrec.GetValue()) );
    else
        pLocalOptions->SetStdPrecision( SvNumberFormatter::UNLIMITED_PRECISION );

    if ( *pLocalOptions != *pOldOptions )
    {
        rCoreAttrs.Put( ScTpCalcItem( nWhichCalc, *pLocalOptions ) );
        return sal_True;
    }
    else
        return sal_False;
}

//------------------------------------------------------------------------

int __EXPORT ScTpCalcOptions::DeactivatePage( SfxItemSet* pSetP )
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
    else if ( pSetP )
        FillItemSet( *pSetP );

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

IMPL_LINK( ScTpCalcOptions, CheckClickHdl, CheckBox*, pBtn )
{
    if (pBtn == &aBtnGeneralPrec)
    {
        if (pBtn->IsChecked())
        {
            aEdPrec.Enable();
            aFtPrec.Enable();
        }
        else
        {
            aEdPrec.Disable();
            aFtPrec.Disable();
        }
    }
    else if (pBtn == &aBtnIterate)
    {
        if ( pBtn->IsChecked() )
        {
            pLocalOptions->SetIter( sal_True );
            aFtSteps.Enable();  aEdSteps.Enable();
            aFtEps  .Enable();  aEdEps  .Enable();
        }
        else
        {
            pLocalOptions->SetIter( sal_False );
            aFtSteps.Disable(); aEdSteps.Disable();
            aFtEps  .Disable(); aEdEps  .Disable();
        }
    }

    return 0;
}




