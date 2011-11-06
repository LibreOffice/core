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

#include <vcl/msgbox.hxx>

#include "lbseldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScSelEntryDlg::ScSelEntryDlg(      Window*  pParent,
                                   sal_uInt16   nResId,
                             const String&  aTitle,
                             const String&  aLbTitle,
                                   List&    aEntryList ) :
    ModalDialog     ( pParent, ScResId( nResId ) ),
    //
    aFlLbTitle      ( this, ScResId( FL_ENTRYLIST ) ),
    aLb             ( this, ScResId( LB_ENTRYLIST ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    SetText( aTitle );
    aFlLbTitle.SetText( aLbTitle );
    aLb.Clear();
    aLb.SetDoubleClickHdl( LINK( this, ScSelEntryDlg, DblClkHdl ) );

    void*   pListEntry = aEntryList.First();
    while ( pListEntry )
    {
        aLb.InsertEntry( *((String*)pListEntry ) );
        pListEntry = aEntryList.Next();
    }

    if ( aLb.GetEntryCount() > 0 )
        aLb.SelectEntryPos( 0 );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

String ScSelEntryDlg::GetSelectEntry() const
{
    return aLb.GetSelectEntry();
}

//------------------------------------------------------------------------

//UNUSED2008-05  sal_uInt16 ScSelEntryDlg::GetSelectEntryPos() const
//UNUSED2008-05  {
//UNUSED2008-05      return aLb.GetSelectEntryPos();
//UNUSED2008-05  }

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScSelEntryDlg, DblClkHdl, void *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( ScSelEntryDlg, DblClkHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

__EXPORT ScSelEntryDlg::~ScSelEntryDlg()
{
}



