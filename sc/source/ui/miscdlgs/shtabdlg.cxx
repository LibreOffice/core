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

#include "shtabdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScShowTabDlg::ScShowTabDlg( Window* pParent ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_SHOW_TAB ) ),
    aFtLbTitle      ( this, ScResId( FT_LABEL ) ),
    aLb             ( this, ScResId( LB_ENTRYLIST ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    aLb.Clear();
    aLb.SetDoubleClickHdl( LINK( this, ScShowTabDlg, DblClkHdl ) );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

void ScShowTabDlg::SetDescription(
        const String& rTitle, const String& rFixedText,
        const rtl::OString& rDlgHelpId, const rtl::OString& sLbHelpId )
{
    SetText( rTitle );
    aFtLbTitle.SetText( rFixedText );
    SetHelpId( rDlgHelpId );
    aLb.SetHelpId( sLbHelpId );
}

void ScShowTabDlg::Insert( const String& rString, sal_Bool bSelected )
{
    aLb.InsertEntry( rString );
    if( bSelected )
        aLb.SelectEntryPos( aLb.GetEntryCount() - 1 );
}

//------------------------------------------------------------------------

sal_uInt16 ScShowTabDlg::GetSelectEntryCount() const
{
    return aLb.GetSelectEntryCount();
}

String ScShowTabDlg::GetSelectEntry(sal_uInt16 nPos) const
{
    return aLb.GetSelectEntry(nPos);
}

sal_uInt16 ScShowTabDlg::GetSelectEntryPos(sal_uInt16 nPos) const
{
    return aLb.GetSelectEntryPos(nPos);
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScShowTabDlg, DblClkHdl, void *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( ScShowTabDlg, DblClkHdl, void *, EMPTYARG )

__EXPORT ScShowTabDlg::~ScShowTabDlg()
{
}



