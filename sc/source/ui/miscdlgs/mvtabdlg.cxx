/*************************************************************************
 *
 *  $RCSfile: mvtabdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
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
        aLbDoc      ( this, ScResId( LB_DEST ) ),
        aLbTable    ( this, ScResId( LB_INSERT ) ),
        aFtDoc      ( this, ScResId( FT_DEST ) ),
        aFtTable    ( this, ScResId( FT_INSERT ) ),
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

USHORT ScMoveTableDlg::GetSelectedTable    () const { return nTable;     }

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
    nTable      = (nTabSel != nTabLast) ? nTabSel : SC_TAB_APPEND;
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
        USHORT      nLast  = 0;
        String      aName;

        aLbTable.Clear();
        aLbTable.SetUpdateMode( FALSE );
        if ( pDoc )
        {
            nLast = pDoc->GetTableCount()-1;
            for ( USHORT i=0; i<=nLast; i++ )
            {
                pDoc->GetName( i, aName );
                aLbTable.InsertEntry( aName, i );
            }
        }
        aLbTable.InsertEntry( ScGlobal::GetRscString(STR_MOVE_TO_END) );
        aLbTable.SetUpdateMode( TRUE );
        aLbTable.SelectEntryPos( 0 );
    }

    return 0;
}



