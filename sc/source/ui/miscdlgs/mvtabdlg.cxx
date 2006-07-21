/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mvtabdlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:08:08 $
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
        bCopyTable  ( FALSE )
{
    Init();
    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScMoveTableDlg::~ScMoveTableDlg()
{
}

//------------------------------------------------------------------------

USHORT ScMoveTableDlg::GetSelectedDocument () const { return nDocument;  }

SCTAB ScMoveTableDlg::GetSelectedTable    () const { return nTable;     }

BOOL   ScMoveTableDlg::GetCopyTable        () const { return bCopyTable; }

void ScMoveTableDlg::SetCopyTable(BOOL bFlag)
{
    aBtnCopy.Check(bFlag);
}
void ScMoveTableDlg::EnableCopyTable(BOOL bFlag)
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
    aBtnCopy.Check( FALSE );
    InitDocListBox();
    SelHdl( &aLbDoc );
}

//------------------------------------------------------------------------

void ScMoveTableDlg::InitDocListBox()
{
    SfxObjectShell* pSh     = SfxObjectShell::GetFirst();
    ScDocShell*     pScSh   = NULL;
    ScDocument*     pDoc    = NULL;
    USHORT          nSelPos = 0;
    USHORT          i       = 0;

    aLbDoc.Clear();
    aLbDoc.SetUpdateMode( FALSE );

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

    aLbDoc.SetUpdateMode( TRUE );
    aLbDoc.InsertEntry( String( ScResId( STR_NEWDOC ) ) );
    aLbDoc.SelectEntryPos( nSelPos );
}


//------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScMoveTableDlg, OkHdl, void *, EMPTYARG )
{
    USHORT  nDocSel     = aLbDoc.GetSelectEntryPos();
    USHORT  nDocLast    = aLbDoc.GetEntryCount()-1;
    USHORT  nTabSel     = aLbTable.GetSelectEntryPos();
    USHORT  nTabLast    = aLbTable.GetEntryCount()-1;

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
        aLbTable.SetUpdateMode( FALSE );
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
        aLbTable.SetUpdateMode( TRUE );
        aLbTable.SelectEntryPos( 0 );
    }

    return 0;
}



