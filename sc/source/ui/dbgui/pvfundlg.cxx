/*************************************************************************
 *
 *  $RCSfile: pvfundlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
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

#ifndef PCH
#include <vcl/msgbox.hxx>
#include <segmentc.hxx>
#endif

#include "scresid.hxx"
#include "pivot.hxx"
#include "pvfundlg.hrc"
#include "pvfundlg.hxx"

SEG_EOFGLOBALS()

//========================================================================
#pragma SEG_FUNCDEF(pvfundlg_01)

ScPivotFunctionDlg::ScPivotFunctionDlg( Window*         pParent,
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
        aGbFunc     ( this, ScResId( GB_FUNC ) ),
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
#pragma SEG_FUNCDEF(pvfundlg_02)

__EXPORT ScPivotFunctionDlg::~ScPivotFunctionDlg()
{
    USHORT  nEntryCount = aLbFunc.GetEntryCount();

    for ( USHORT i=0; i<nEntryCount; i++ )
    {
        delete (USHORT*)aLbFunc.GetEntryData( i );
        aLbFunc.SetEntryData( i, NULL );
    }
}

//------------------------------------------------------------------------

BOOL ScPivotFunctionDlg::GetShowAll() const
{
    return aCbShowAll.IsChecked();
}

//------------------------------------------------------------------------

#define FUNC_SET(f) ( (nFunctions & (f)) == (f) )

#pragma SEG_FUNCDEF(pvfundlg_03)

void ScPivotFunctionDlg::InitFuncLb( USHORT nFunctions )
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
    aLbFunc.SetDoubleClickHdl( LINK( this, ScPivotFunctionDlg, DblClickHdl ) );
}

#undef FUNC_SET

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pvfundlg_05)

void ScPivotFunctionDlg::SetUI( BOOL bSubTotals )
{
    aBtnOk.SetClickHdl  ( LINK( this, ScPivotFunctionDlg, ClickHdl ) );

    if ( bSubTotals )
    {
        aGbFunc.SetText( ScResId( STR_SUBTOTALS ) );

        aBtnNone.SetClickHdl    ( LINK( this, ScPivotFunctionDlg, RadioClickHdl ) );
        aBtnAuto.SetClickHdl    ( LINK( this, ScPivotFunctionDlg, RadioClickHdl ) );
        aBtnUser.SetClickHdl    ( LINK( this, ScPivotFunctionDlg, RadioClickHdl ) );

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
        aGbFunc.SetText( ScResId( STR_DATAFUNC ) );

        Size    aSize( aGbFunc.GetSizePixel() );
        Point   aPos1( aFtNameLabel.GetPosPixel() );
        Point   aPos2( aFtName.GetPosPixel() );
        long    nYDiff =  aBtnUser.GetPosPixel().Y()
                        - aBtnNone.GetPosPixel().Y()
                        + aBtnNone.GetSizePixel().Height()
                        + LogicToPixel( Point(5,0), MAP_APPFONT ).X();

        aSize.Height()  -= nYDiff;
        aPos1.Y()       -= nYDiff;
        aPos2.Y()       -= nYDiff;

        aGbFunc     .SetSizePixel( aSize );
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
#pragma SEG_FUNCDEF(pvfundlg_04)

IMPL_LINK( ScPivotFunctionDlg, RadioClickHdl, RadioButton *, pBtn )
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
#pragma SEG_FUNCDEF(pvfundlg_07)

IMPL_LINK_INLINE_START( ScPivotFunctionDlg, DblClickHdl, MultiListBox *, EMPTYARG )
{
    ClickHdl( &aBtnOk );
    return 0;
}
IMPL_LINK_INLINE_END( ScPivotFunctionDlg, DblClickHdl, MultiListBox *, EMPTYARG )

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pvfundlg_06)

IMPL_LINK( ScPivotFunctionDlg, ClickHdl, PushButton *, pBtn )
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.15  2000/09/17 14:08:57  willem.vandorp
    OpenOffice header added.

    Revision 1.14  2000/08/31 16:38:20  willem.vandorp
    Header and footer replaced

    Revision 1.13  2000/02/11 12:23:45  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.12  1999/11/02 08:47:39  nn
    show all

    Revision 1.11  1997/12/05 18:55:36  ANK
    Includes geaendert


      Rev 1.10   05 Dec 1997 19:55:36   ANK
   Includes geaendert

      Rev 1.9   29 Oct 1996 14:03:54   NN
   ueberall ScResId statt ResId

      Rev 1.8   29 Jan 1996 15:12:26   MO
   neuer Link

      Rev 1.7   08 Nov 1995 13:05:48   MO
   301-Aenderungen

      Rev 1.6   20 Sep 1995 14:18:52   MO
   DoubleClick-Handler (BugId: 19541)

      Rev 1.5   14 Sep 1995 13:05:08   MO
   FixedInfos

      Rev 1.4   24 Jul 1995 14:09:08   MO
   EXPORT

      Rev 1.3   05 May 1995 18:16:42   MO
   Funktionen fuer Data

      Rev 1.2   02 May 1995 18:45:42   MO
   Teilergebnis/Datenfunktionen

      Rev 1.1   26 Apr 1995 17:44:10   MO
   Teilergebnisse/Datenfunktion

      Rev 1.0   26 Apr 1995 16:43:26   MO
   Dialog fuer Pivotfeldfunktionen

------------------------------------------------------------------------ */

#pragma SEG_EOFMODULE

