/*************************************************************************
 *
 *  $RCSfile: pvfundlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dr $ $Date: 2002-03-01 11:35:58 $
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


#ifndef PCH
#include <vcl/msgbox.hxx>
#endif

#include "scresid.hxx"
#include "pivot.hxx"
#include "pvfundlg.hrc"
#include "pvfundlg.hxx"

//========================================================================

ScDPFunctionDlg::ScDPFunctionDlg(   Window*         pParent,
                                    BOOL            bSubTotalFunc,
                                    const String&   rName,
                                    USHORT          nFunctions,
                                    BOOL            bIsShowAll )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_PIVOTSUBT ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),
        aBtnNone    ( this, ScResId( BTN_NONE ) ),
        aBtnAuto    ( this, ScResId( BTN_AUTO ) ),
        aBtnUser    ( this, ScResId( BTN_USER ) ),
        aFlFunc     ( this, ScResId( FL_FUNC ) ),
        aCbShowAll  ( this, ScResId( CB_SHOWALL ) ),
        aFtNameLabel( this, ScResId( FT_NAMELABEL ) ),
        aFtName     ( this, ScResId( FT_NAME ) ),
        aLbFunc     ( this, ScResId( LB_FUNC ) ),
        nFuncMask   ( nFunctions )
{
    aFtName.SetText( rName );
    SetUI( bSubTotalFunc );
    InitFuncLb( nFunctions );
    if ( bSubTotalFunc )                    // hidden in SetUI otherwise
        aCbShowAll.Check( bIsShowAll );
    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScDPFunctionDlg::~ScDPFunctionDlg()
{
    USHORT  nEntryCount = aLbFunc.GetEntryCount();

    for ( USHORT i=0; i<nEntryCount; i++ )
    {
        delete (USHORT*)aLbFunc.GetEntryData( i );
        aLbFunc.SetEntryData( i, NULL );
    }
}

//------------------------------------------------------------------------

BOOL ScDPFunctionDlg::GetShowAll() const
{
    return aCbShowAll.IsChecked();
}

//------------------------------------------------------------------------

#define FUNC_SET(f) ( (nFunctions & (f)) == (f) )


void ScDPFunctionDlg::InitFuncLb( USHORT nFunctions )
{
    aLbFunc.SetEntryData(  0, new USHORT(PIVOT_FUNC_SUM) );
    aLbFunc.SetEntryData(  1, new USHORT(PIVOT_FUNC_COUNT) );
    aLbFunc.SetEntryData(  2, new USHORT(PIVOT_FUNC_AVERAGE) );
    aLbFunc.SetEntryData(  3, new USHORT(PIVOT_FUNC_MAX) );
    aLbFunc.SetEntryData(  4, new USHORT(PIVOT_FUNC_MIN) );
    aLbFunc.SetEntryData(  5, new USHORT(PIVOT_FUNC_PRODUCT) );
    aLbFunc.SetEntryData(  6, new USHORT(PIVOT_FUNC_COUNT_NUM) );
    aLbFunc.SetEntryData(  7, new USHORT(PIVOT_FUNC_STD_DEV) );
    aLbFunc.SetEntryData(  8, new USHORT(PIVOT_FUNC_STD_DEVP) );
    aLbFunc.SetEntryData(  9, new USHORT(PIVOT_FUNC_STD_VAR) );
    aLbFunc.SetEntryData( 10, new USHORT(PIVOT_FUNC_STD_VARP) );

    if (    nFunctions == PIVOT_FUNC_NONE
         || nFunctions == PIVOT_FUNC_AUTO )
    {
        aLbFunc.SetNoSelection();
    }
    else
    {
        aLbFunc.SelectEntryPos(  0, FUNC_SET( PIVOT_FUNC_SUM        ) );
        aLbFunc.SelectEntryPos(  1, FUNC_SET( PIVOT_FUNC_COUNT      ) );
        aLbFunc.SelectEntryPos(  2, FUNC_SET( PIVOT_FUNC_AVERAGE    ) );
        aLbFunc.SelectEntryPos(  3, FUNC_SET( PIVOT_FUNC_MAX        ) );
        aLbFunc.SelectEntryPos(  4, FUNC_SET( PIVOT_FUNC_MIN        ) );
        aLbFunc.SelectEntryPos(  5, FUNC_SET( PIVOT_FUNC_PRODUCT    ) );
        aLbFunc.SelectEntryPos(  6, FUNC_SET( PIVOT_FUNC_COUNT_NUM  ) );
        aLbFunc.SelectEntryPos(  7, FUNC_SET( PIVOT_FUNC_STD_DEV    ) );
        aLbFunc.SelectEntryPos(  8, FUNC_SET( PIVOT_FUNC_STD_DEVP   ) );
        aLbFunc.SelectEntryPos(  9, FUNC_SET( PIVOT_FUNC_STD_VAR    ) );
        aLbFunc.SelectEntryPos( 10, FUNC_SET( PIVOT_FUNC_STD_VARP   ) );
    }
    aLbFunc.SetDoubleClickHdl( LINK( this, ScDPFunctionDlg, DblClickHdl ) );
}

#undef FUNC_SET

//------------------------------------------------------------------------

void ScDPFunctionDlg::SetUI( BOOL bSubTotals )
{
    aBtnOk.SetClickHdl  ( LINK( this, ScDPFunctionDlg, ClickHdl ) );

    if ( bSubTotals )
    {
        aFlFunc.SetText( ScResId( STR_SUBTOTALS ) );

        aBtnNone.SetClickHdl    ( LINK( this, ScDPFunctionDlg, RadioClickHdl ) );
        aBtnAuto.SetClickHdl    ( LINK( this, ScDPFunctionDlg, RadioClickHdl ) );
        aBtnUser.SetClickHdl    ( LINK( this, ScDPFunctionDlg, RadioClickHdl ) );

        if ( nFuncMask == PIVOT_FUNC_NONE )
        {
            aBtnNone.Check( nFuncMask == PIVOT_FUNC_NONE );
            aLbFunc.Disable();
        }
        else if ( nFuncMask == PIVOT_FUNC_AUTO )
        {
            aBtnAuto.Check();
            aLbFunc.Disable();
        }
        else
        {
            aBtnUser.Check();
            aLbFunc.Enable();
        }
    }
    else
    {
        aFlFunc.SetText( ScResId( STR_DATAFUNC ) );

        Size    aSize( aFlFunc.GetSizePixel() );
        Point   aPos1( aFtNameLabel.GetPosPixel() );
        Point   aPos2( aFtName.GetPosPixel() );
        long    nYDiff =  aBtnUser.GetPosPixel().Y()
                        - aBtnNone.GetPosPixel().Y()
                        + aBtnNone.GetSizePixel().Height()
                        + LogicToPixel( Point(5,0), MAP_APPFONT ).X();

        aPos1.Y()       -= nYDiff;
        aPos2.Y()       -= nYDiff;

        aLbFunc     .SetPosPixel( aBtnNone.GetPosPixel() );
        aFtNameLabel.SetPosPixel( aPos1 );
        aFtName     .SetPosPixel( aPos2 );

        aLbFunc.Enable();
        aBtnNone.Hide();
        aBtnAuto.Hide();
        aBtnUser.Hide();
        aBtnUser.Check();

        aCbShowAll.Disable();
        aCbShowAll.Hide();

        aSize = GetSizePixel();
        aSize.Height() -= nYDiff;
        SetSizePixel( aSize );
    }
}

//------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScDPFunctionDlg, RadioClickHdl, RadioButton *, pBtn )
{
    if ( pBtn == &aBtnNone || pBtn == &aBtnAuto )
    {
        aLbFunc.Disable();
    }
    else if ( pBtn == &aBtnUser )
    {
        aLbFunc.Enable();
    }
    return 0;
}


//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScDPFunctionDlg, DblClickHdl, MultiListBox *, EMPTYARG )
{
    ClickHdl( &aBtnOk );
    return 0;
}
IMPL_LINK_INLINE_END( ScDPFunctionDlg, DblClickHdl, MultiListBox *, EMPTYARG )

//------------------------------------------------------------------------

IMPL_LINK( ScDPFunctionDlg, ClickHdl, PushButton *, pBtn )
{
    if ( pBtn == &aBtnOk )
    {
        if ( aBtnNone.IsChecked() )
            nFuncMask = PIVOT_FUNC_NONE;
        else if ( aBtnAuto.IsChecked() )
            nFuncMask = PIVOT_FUNC_AUTO;
        else if ( aBtnUser.IsChecked() )
        {
            USHORT  nPos        = 0;
            USHORT  nSelCount   = aLbFunc.GetSelectEntryCount();

            nFuncMask = PIVOT_FUNC_NONE;

            for ( USHORT i=0; i<nSelCount; i++ )
            {
                nPos = aLbFunc.GetSelectEntryPos( i );
                nFuncMask = ( nFuncMask | *((USHORT*)aLbFunc.GetEntryData( nPos )) );
            }
        }

        EndDialog( RET_OK );
    }
    return 0;
}
