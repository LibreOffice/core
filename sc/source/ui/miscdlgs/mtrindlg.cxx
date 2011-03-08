/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "mtrindlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScMetricInputDlg::ScMetricInputDlg( Window*         pParent,
                                    USHORT          nResId,
                                    long            nCurrent,
                                    long            nDefault,
                                    FieldUnit       eFUnit,
                                    USHORT          nDecimals,
                                    long            nMaximum,
                                    long            nMinimum,
                                    long            nFirst,
                                    long            nLast )

    :   ModalDialog     ( pParent, ScResId( nResId ) ),
        //
        aFtEditTitle    ( this, ScResId( FT_LABEL ) ),
        aEdValue        ( this, ScResId( ED_VALUE ) ),
        aBtnDefVal      ( this, ScResId( BTN_DEFVAL ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    //SetText( rTitle );
    //
    //aFtEditTitle.SetText( rEditTitle );
    CalcPositions();
    aBtnDefVal.SetClickHdl ( LINK( this, ScMetricInputDlg, SetDefValHdl ) );
    aEdValue.  SetModifyHdl( LINK( this, ScMetricInputDlg, ModifyHdl    ) );

    aEdValue.SetUnit            ( eFUnit );
    aEdValue.SetDecimalDigits   ( nDecimals );
    aEdValue.SetMax             ( aEdValue.Normalize( nMaximum ), FUNIT_TWIP );
    aEdValue.SetMin             ( aEdValue.Normalize( nMinimum ), FUNIT_TWIP );
    aEdValue.SetLast            ( aEdValue.Normalize( nLast ),    FUNIT_TWIP );
    aEdValue.SetFirst           ( aEdValue.Normalize( nFirst ),   FUNIT_TWIP );
    aEdValue.SetSpinSize        ( aEdValue.Normalize( 1 ) / 10 );
    aEdValue.SetValue           ( aEdValue.Normalize( nDefault ), FUNIT_TWIP );
    nDefaultValue = sal::static_int_cast<long>( aEdValue.GetValue() );
    aEdValue.SetValue           ( aEdValue.Normalize( nCurrent ), FUNIT_TWIP );
    nCurrentValue = sal::static_int_cast<long>( aEdValue.GetValue() );
    aBtnDefVal.Check( nCurrentValue == nDefaultValue );

    FreeResource();
}

//------------------------------------------------------------------------

ScMetricInputDlg::~ScMetricInputDlg()
{
}

//------------------------------------------------------------------------

long ScMetricInputDlg::GetInputValue( FieldUnit eUnit ) const
{
/*
    mit Nachkommastellen:

    double  nVal    = aEdValue.GetValue( eUnit );
    USHORT  nDecs   = aEdValue.GetDecimalDigits();
    double  nFactor = 0.0;

    // static long ImpPower10( USHORT nDecs )
    {
        nFactor = 1.0;

        for ( USHORT i=0; i < nDecs; i++ )
            nFactor *= 10.0;
    }

    return nVal / nFactor;
*/
    // erstmal Nachkommastellen abschneiden - nich so doll...

    return sal::static_int_cast<long>( aEdValue.Denormalize( aEdValue.GetValue( eUnit ) ) );
}

//------------------------------------------------------------------------

void ScMetricInputDlg::CalcPositions()
{
    MapMode oldMode     = GetMapMode();
    SetMapMode( MAP_APPFONT );

    Size    aDlgSize    = GetOutputSizePixel();
    Size    aFtSize     = aFtEditTitle.GetSizePixel();
    Point   aNewPos;

    aFtSize.Width() = aFtEditTitle.GetTextWidth(aFtEditTitle.GetText());
    // add mnemonic char width to fixed text width
    aFtSize.Width() += aFtEditTitle.GetTextWidth(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("(W)")));
    aFtEditTitle.SetSizePixel( aFtSize );

    aNewPos.Y()  = aEdValue.GetPosPixel().Y();
    aNewPos.X()  = aFtEditTitle.GetPosPixel().X();
    aNewPos.X() += aFtEditTitle.GetSizePixel().Width();
    aNewPos.X() += LogicToPixel( Point(3,0) ).X();
    aEdValue.SetPosPixel( aNewPos );

    aNewPos.Y()  = aBtnDefVal.GetPosPixel().Y();
    aBtnDefVal.SetPosPixel( aNewPos );

    aNewPos.Y()  = aBtnOk.GetPosPixel().Y();
    aNewPos.X() += aEdValue.GetSizePixel().Width();
    aNewPos.X() += LogicToPixel( Point(6,0) ).X();
    aBtnOk.SetPosPixel( aNewPos );
    aNewPos.Y()  = aBtnCancel.GetPosPixel().Y();
    aBtnCancel.SetPosPixel( aNewPos );
    aNewPos.Y()  = aBtnHelp.GetPosPixel().Y();
    aBtnHelp.SetPosPixel( aNewPos );

    aNewPos.X() += aBtnOk.GetSizePixel().Width();
    aNewPos.X() += LogicToPixel( Point(6,0) ).X();
    aDlgSize.Width() = aNewPos.X();
    SetOutputSizePixel( aDlgSize );

    SetMapMode( oldMode );
}

//------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScMetricInputDlg, SetDefValHdl, CheckBox *, EMPTYARG )
{
    if ( aBtnDefVal.IsChecked() )
    {
        nCurrentValue = sal::static_int_cast<long>( aEdValue.GetValue() );
        aEdValue.SetValue( nDefaultValue );
    }
    else
        aEdValue.SetValue( nCurrentValue );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScMetricInputDlg, ModifyHdl, MetricField *, EMPTYARG )
{
    aBtnDefVal.Check( nDefaultValue == aEdValue.GetValue() );
    return 0;
}
IMPL_LINK_INLINE_END( ScMetricInputDlg, ModifyHdl, MetricField *, EMPTYARG )



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
