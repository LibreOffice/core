/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


