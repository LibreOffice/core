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

#include "mvtabdlg.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "miscdlgs.hrc"
#include "global.hxx"
#include "scresid.hxx"
#include "globstr.hrc"

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef ScResId
#define ScResId(x) #x
#undef ModalDialog
#define ModalDialog( parent, id ) Dialog( parent, "move-copy-sheet.xml", id )
#endif /* ENABLE_LAYOUT */

//==================================================================

ScMoveTableDlg::ScMoveTableDlg( Window* pParent )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_MOVETAB ) ),
        //
        aFtDoc      ( this, ScResId( FT_DEST ) ),
        aLbDoc      ( this, ScResId( LB_DEST ) ),
        aFtTable    ( this, ScResId( FT_INSERT ) ),
        aLbTable    ( this, ScResId( LB_INSERT ) ),
        aBtnCopy    ( this, ScResId( BTN_COPY ) ),
        aBtnOk      ( this, ScResId( BTN_OK ) ),
        aBtnCancel  ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp    ( this, ScResId( BTN_HELP ) ),
        //
        nDocument   ( 0 ),
        nTable      ( 0 ),
        bCopyTable  ( sal_False )
{
#if ENABLE_LAYOUT
#undef ScResId
    SetHelpId (FID_TAB_MOVE);
#endif /* ENABLE_LAYOUT */
    Init();
    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScMoveTableDlg::~ScMoveTableDlg()
{
}

//------------------------------------------------------------------------

sal_uInt16 ScMoveTableDlg::GetSelectedDocument () const { return nDocument;  }

SCTAB ScMoveTableDlg::GetSelectedTable    () const { return nTable;     }

sal_Bool   ScMoveTableDlg::GetCopyTable        () const { return bCopyTable; }

void ScMoveTableDlg::SetCopyTable(sal_Bool bFlag)
{
    aBtnCopy.Check(bFlag);
}
void ScMoveTableDlg::EnableCopyTable(sal_Bool bFlag)
{
    if(bFlag)
        aBtnCopy.Enable();
    else
        aBtnCopy.Disable();
}


//------------------------------------------------------------------------

void __EXPORT ScMoveTableDlg::Init()
{
    aBtnOk.SetClickHdl   ( LINK( this, ScMoveTableDlg, OkHdl ) );
    aLbDoc.SetSelectHdl  ( LINK( this, ScMoveTableDlg, SelHdl ) );
    aBtnCopy.Check( sal_False );
    InitDocListBox();
    SelHdl( &aLbDoc );
}

//------------------------------------------------------------------------

void ScMoveTableDlg::InitDocListBox()
{
    SfxObjectShell* pSh     = SfxObjectShell::GetFirst();
    ScDocShell*     pScSh   = NULL;
    sal_uInt16          nSelPos = 0;
    sal_uInt16          i       = 0;

    aLbDoc.Clear();
    aLbDoc.SetUpdateMode( sal_False );

    while ( pSh )
    {
        pScSh = dynamic_cast< ScDocShell* >( pSh );

        if ( pScSh )
        {
            if ( pScSh == SfxObjectShell::Current() )
                nSelPos = i;

            aLbDoc.InsertEntry( pScSh->GetTitle(), i );
            aLbDoc.SetEntryData( i, (void*)pScSh->GetDocument() );

            i++;
        }
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    aLbDoc.SetUpdateMode( sal_True );
    aLbDoc.InsertEntry( String( ScResId( STR_NEWDOC ) ) );
    aLbDoc.SelectEntryPos( nSelPos );
}


//------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScMoveTableDlg, OkHdl, void *, EMPTYARG )
{
    sal_uInt16  nDocSel     = aLbDoc.GetSelectEntryPos();
    sal_uInt16  nDocLast    = aLbDoc.GetEntryCount()-1;
    sal_uInt16  nTabSel     = aLbTable.GetSelectEntryPos();
    sal_uInt16  nTabLast    = aLbTable.GetEntryCount()-1;

    nDocument   = (nDocSel != nDocLast) ? nDocSel : SC_DOC_NEW;
    nTable      = (nTabSel != nTabLast) ? static_cast<SCTAB>(nTabSel) : SC_TAB_APPEND;
    bCopyTable  = aBtnCopy.IsChecked();
    EndDialog( RET_OK );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScMoveTableDlg, SelHdl, ListBox *, pLb )
{
    if ( pLb == &aLbDoc )
    {
        ScDocument* pDoc   = (ScDocument*)
                             aLbDoc.GetEntryData( aLbDoc.GetSelectEntryPos() );
        SCTAB      nLast  = 0;
        String      aName;

        aLbTable.Clear();
        aLbTable.SetUpdateMode( sal_False );
        if ( pDoc )
        {
            nLast = pDoc->GetTableCount()-1;
            for ( SCTAB i=0; i<=nLast; i++ )
            {
                pDoc->GetName( i, aName );
                aLbTable.InsertEntry( aName, static_cast<sal_uInt16>(i) );
            }
        }
        aLbTable.InsertEntry( ScGlobal::GetRscString(STR_MOVE_TO_END) );
        aLbTable.SetUpdateMode( sal_True );
        aLbTable.SelectEntryPos( 0 );
    }

    return 0;
}



