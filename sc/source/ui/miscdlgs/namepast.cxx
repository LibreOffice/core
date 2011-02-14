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

#include "namepast.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"
#include "rangenam.hxx"


//==================================================================

ScNamePasteDlg::ScNamePasteDlg( Window * pParent, const ScRangeName* pList, sal_Bool bInsList )
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

    sal_uInt16  nCnt = pList->GetCount();
    String  aText;

    for( sal_uInt16 i=0 ; i<nCnt ; i++ )
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


