/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namepast.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:12:54 $
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

#undef SC_DLLIMPLEMENTATION

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "namepast.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"
#include "rangenam.hxx"


//==================================================================

ScNamePasteDlg::ScNamePasteDlg( Window * pParent, const ScRangeName* pList, BOOL bInsList )
    : ModalDialog( pParent, ScResId( RID_SCDLG_NAMES_PASTE ) ),
    aLabelText      ( this, ScResId( FT_LABEL ) ),
    aNameList       ( this, ScResId( LB_ENTRYLIST ) ),
    aOKButton       ( this, ScResId( BTN_OK ) ),
    aCancelButton   ( this, ScResId( BTN_CANCEL ) ),
    aHelpButton     ( this, ScResId( BTN_HELP ) ),
    aInsListButton  ( this, ScResId( BTN_ADD ) )
{
    if( ! bInsList )
        aInsListButton.Disable();

    aInsListButton.SetClickHdl( LINK( this,ScNamePasteDlg,ButtonHdl) );
    aOKButton.SetClickHdl( LINK( this,ScNamePasteDlg,ButtonHdl) );
    aNameList.SetSelectHdl( LINK( this,ScNamePasteDlg,ListSelHdl) );
    aNameList.SetDoubleClickHdl( LINK( this,ScNamePasteDlg,ListDblClickHdl) );

    USHORT  nCnt = pList->GetCount();
    String  aText;

    for( USHORT i=0 ; i<nCnt ; i++ )
    {
        ScRangeData* pData = (*pList)[ i ];

        if( pData )
        {
            if (   !pData->HasType( RT_DATABASE )
                && !pData->HasType( RT_SHARED ) )
            {
                pData->GetName( aText );
                aNameList.InsertEntry( aText );
            }
        }
    }

    ListSelHdl( &aNameList );

    FreeResource();
}

//------------------------------------------------------------------

IMPL_LINK( ScNamePasteDlg, ButtonHdl, Button *, pButton )
{
    if( pButton == &aInsListButton )
    {
        EndDialog( BTN_PASTE_LIST );
    }
    else if( pButton == &aOKButton )
    {
        EndDialog( BTN_PASTE_NAME );
    }
    return 0;
}

//------------------------------------------------------------------

IMPL_LINK( ScNamePasteDlg, ListSelHdl, ListBox *, pListBox )
{
    if( pListBox == &aNameList )
    {
        if( aNameList.GetSelectEntryCount() )
            aOKButton.Enable();
        else
            aOKButton.Disable();
    }
    return 0;
}

//------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScNamePasteDlg, ListDblClickHdl, ListBox *, pListBox )
{
    if( pListBox == &aNameList )
    {
        ButtonHdl( &aOKButton );
    }
    return 0;
}
IMPL_LINK_INLINE_END( ScNamePasteDlg, ListDblClickHdl, ListBox *, pListBox )

//------------------------------------------------------------------

String ScNamePasteDlg::GetSelectedName() const
{
    return aNameList.GetSelectEntry();
}


