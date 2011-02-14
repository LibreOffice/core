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
        pScSh = PTR_CAST( ScDocShell, pSh );

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



