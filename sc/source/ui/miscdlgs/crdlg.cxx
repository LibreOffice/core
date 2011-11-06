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

#include "crdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScColOrRowDlg::ScColOrRowDlg( Window*       pParent,
                              const String& rStrTitle,
                              const String& rStrLabel,
                              sal_Bool          bColDefault )

    :   ModalDialog     ( pParent, ScResId( RID_SCDLG_COLORROW ) ),
        //
        aFlFrame        ( this, ScResId( FL_FRAME ) ),
        aBtnRows        ( this, ScResId( BTN_GROUP_ROWS ) ),
        aBtnCols        ( this, ScResId( BTN_GROUP_COLS ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    SetText( rStrTitle );
    aFlFrame.SetText( rStrLabel );

    if ( bColDefault )
        aBtnCols.Check();
    else
        aBtnRows.Check();

    aBtnOk.SetClickHdl( LINK( this, ScColOrRowDlg, OkHdl ) );

    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScColOrRowDlg::~ScColOrRowDlg()
{
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScColOrRowDlg, OkHdl, OKButton *, EMPTYARG )
{
    EndDialog( aBtnCols.IsChecked() ? SCRET_COLS : SCRET_ROWS );
    return 0;
}
IMPL_LINK_INLINE_END( ScColOrRowDlg, OkHdl, OKButton *, EMPTYARG )



