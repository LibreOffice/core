/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mtrindlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:07:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        aEdValue        ( this, ScResId( ED_VALUE ) ),
        aBtnDefVal      ( this, ScResId( BTN_DEFVAL ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aFtEditTitle    ( this, ScResId( FT_LABEL ) )
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
    nDefaultValue = aEdValue.GetValue();
    aEdValue.SetValue           ( aEdValue.Normalize( nCurrent ), FUNIT_TWIP );
    nCurrentValue = aEdValue.GetValue();
    aBtnDefVal.Check( nCurrentValue == nDefaultValue );

    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScMetricInputDlg::~ScMetricInputDlg()
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

    return aEdValue.Denormalize( aEdValue.GetValue( eUnit ) );
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
    // #95990# add mnemonic char width to fixed text width
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
        nCurrentValue = aEdValue.GetValue();
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



