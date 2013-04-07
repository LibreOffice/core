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


#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "mtrindlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScMetricInputDlg::ScMetricInputDlg( Window*         pParent,
                                    sal_uInt16          nResId,
                                    long            nCurrent,
                                    long            nDefault,
                                    FieldUnit       eFUnit,
                                    sal_uInt16          nDecimals,
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
    sal_uInt16  nDecs   = aEdValue.GetDecimalDigits();
    double  nFactor = 0.0;

    // static long ImpPower10( sal_uInt16 nDecs )
    {
        nFactor = 1.0;

        for ( sal_uInt16 i=0; i < nDecs; i++ )
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
    aFtSize.Width() += aFtEditTitle.GetTextWidth(OUString("(W)"));
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

IMPL_LINK_NOARG(ScMetricInputDlg, SetDefValHdl)
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

IMPL_LINK_NOARG_INLINE_START(ScMetricInputDlg, ModifyHdl)
{
    aBtnDefVal.Check( nDefaultValue == aEdValue.GetValue() );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScMetricInputDlg, ModifyHdl)



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
