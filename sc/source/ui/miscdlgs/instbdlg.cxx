/*************************************************************************
 *
 *  $RCSfile: instbdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
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

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "scresid.hxx"
#include "instbdlg.hrc"
#include "globstr.hrc"


#define SC_INSTBDLG_CXX
#include "instbdlg.hxx"



//==================================================================

ScInsertTableDlg::ScInsertTableDlg( Window* pParent, ScViewData& rData, USHORT nTabCount)

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_INSERT_TABLE ) ),
        //
        aBtnBefore      ( this, ScResId( RB_BEFORE ) ),
        aBtnBehind      ( this, ScResId( RB_BEHIND ) ),
        aGbPos          ( this, ScResId( GB_POSITION ) ),
        aFtCount        ( this, ScResId( FT_COUNT ) ),
        aNfCount        ( this, ScResId( NF_COUNT ) ),
        aFtName         ( this, ScResId( FT_NAME ) ),
        aEdName         ( this, ScResId( ED_TABNAME ) ),
        aLbTables       ( this, ScResId( LB_TABLES ) ),
        aFtPath         ( this, ScResId( FT_PATH ) ),
        aBtnBrowse      ( this, ScResId( BTN_BROWSE ) ),
        aBtnLink        ( this, ScResId( BTN_LINK ) ),
        aGbTable        ( this, ScResId( GB_TABLE ) ),
        aBtnNew         ( this, ScResId( BTN_NEW ) ),
        aBtnFromFile    ( this, ScResId( BTN_FROMFILE ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        rViewData       ( rData ),
        rDoc            ( *rData.GetDocument() ),
        pDocShTables    ( NULL ),
        nSelTabIndex    ( 0 ),
        nTableCount     (nTabCount)
{
    Init_Impl();
    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScInsertTableDlg::~ScInsertTableDlg()
{
    if (pDocShTables)
        pDocShTables->DoClose();
}

//------------------------------------------------------------------------

void ScInsertTableDlg::Init_Impl()
{
    aBtnBrowse      .SetClickHdl( LINK( this, ScInsertTableDlg, BrowseHdl_Impl ) );
    aBtnNew         .SetClickHdl( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    aBtnFromFile    .SetClickHdl( LINK( this, ScInsertTableDlg, ChoiceHdl_Impl ) );
    aLbTables       .SetSelectHdl( LINK( this, ScInsertTableDlg, SelectHdl_Impl ) );
    aNfCount        .SetModifyHdl( LINK( this, ScInsertTableDlg, CountHdl_Impl));
    aBtnOk          .SetClickHdl( LINK( this, ScInsertTableDlg, DoEnterHdl ));
    aBtnBefore.Check();
    aBtnNew.Check();
    SetNewTable_Impl();

    ScMarkData& rMark    = rViewData.GetMarkData();
    USHORT  nTabSelCount = rMark.GetSelectCount();

    aNfCount.SetText( String::CreateFromInt32(nTableCount) );

    if(nTableCount==1)
    {
        String aName;
        rDoc.CreateValidTabName( aName );
        aEdName.SetText( aName );
    }
    else
    {
        String aName=aGbTable.GetText();
        aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM("..."));
        aEdName.SetText( aName );
        aFtName.Disable();
        aEdName.Disable();
    }
}

//------------------------------------------------------------------------

short __EXPORT ScInsertTableDlg::Execute()
{
    // Parent fuer InsertDocumentDialog und Doc-Manager setzen:

    Window* pOldDefParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );

    short nRet = ModalDialog::Execute();

    Application::SetDefDialogParent( pOldDefParent );

    return nRet;
}

//------------------------------------------------------------------------

void ScInsertTableDlg::SetNewTable_Impl()
{
    if (aBtnNew.IsChecked() )
    {
        aEdName     .Enable();
        aFtName     .Enable();
        aFtCount    .Enable();
        aLbTables   .Disable();
        aFtPath     .Disable();
        aBtnBrowse  .Disable();
        aBtnLink    .Disable();

        if(nTableCount==1)  aNfCount.Enable();
    }
}

//------------------------------------------------------------------------

void ScInsertTableDlg::SetFromTo_Impl()
{
    if (aBtnFromFile.IsChecked() )
    {
        aEdName     .Disable();
        aFtName     .Disable();
        aFtCount    .Disable();
        aNfCount    .Disable();
        aLbTables   .Enable();
        aFtPath     .Enable();
        aBtnBrowse  .Enable();
        aBtnLink    .Enable();
    }
}

//------------------------------------------------------------------------

void ScInsertTableDlg::FillTables_Impl( ScDocument* pSrcDoc )
{
    aLbTables.SetUpdateMode( FALSE );
    aLbTables.Clear();

    if ( pSrcDoc )
    {
        USHORT nCount = pSrcDoc->GetTableCount();
        String aName;

        for ( USHORT i=0; i<nCount; i++ )
        {
            pSrcDoc->GetName( i, aName );
            aLbTables.InsertEntry( aName );
        }
    }

    aLbTables.SetUpdateMode( TRUE );

    if(aLbTables.GetEntryCount()==1)
        aLbTables.SelectEntryPos(0);
}

//------------------------------------------------------------------------

const String* ScInsertTableDlg::GetFirstTable( USHORT* pN )
{
    const String* pStr = NULL;

    if ( aBtnNew.IsChecked() )
    {
        aStrCurSelTable = aEdName.GetText();
        pStr = &aStrCurSelTable;
    }
    else if ( nSelTabIndex < aLbTables.GetSelectEntryCount() )
    {
        aStrCurSelTable = aLbTables.GetSelectEntry( 0 );
        pStr = &aStrCurSelTable;
        if ( pN )
            *pN = aLbTables.GetSelectEntryPos( 0 );
        nSelTabIndex = 1;
    }

    return pStr;
}

//------------------------------------------------------------------------

const String* ScInsertTableDlg::GetNextTable( USHORT* pN )
{
    const String* pStr = NULL;

    if ( !aBtnNew.IsChecked() && nSelTabIndex < aLbTables.GetSelectEntryCount() )
    {
        aStrCurSelTable = aLbTables.GetSelectEntry( nSelTabIndex );
        pStr = &aStrCurSelTable;
        if ( pN )
            *pN = aLbTables.GetSelectEntryPos( nSelTabIndex );
        nSelTabIndex++;
    }

    return pStr;
}


//------------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------------

IMPL_LINK( ScInsertTableDlg, CountHdl_Impl, NumericField*, EMPTYARG )
{
    nTableCount = aNfCount.GetText().ToInt32();
    if ( nTableCount==1)
    {
        String aName;
        rDoc.CreateValidTabName( aName );
        aEdName.SetText( aName );
        aFtName.Enable();
        aEdName.Enable();
    }
    else
    {
        String aName=aGbTable.GetText();
        aName.AppendAscii(RTL_CONSTASCII_STRINGPARAM("..."));
        aEdName.SetText( aName );
        aFtName.Disable();
        aEdName.Disable();
    }

    DoEnable_Impl();
    return 0;
}

//------------------------------------------------------------------------
IMPL_LINK( ScInsertTableDlg, ChoiceHdl_Impl, RadioButton*, EMPTYARG )
{
    if ( aBtnNew.IsChecked() )
        SetNewTable_Impl();
    else
        SetFromTo_Impl();

    DoEnable_Impl();
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScInsertTableDlg, BrowseHdl_Impl, PushButton*, EMPTYARG )
{
        //  Dialog-Parent ist schon in Execute gesetzt worden

    SfxApplication* pApp = SFX_APP();
    SfxMedium* pMed = pApp->InsertDocumentDialog( 0, ScDocShell::Factory() );

    if ( pMed )
    {
        //  ERRCTX_SFX_OPENDOC -> "Fehler beim Laden des Dokumentes"
        SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

        if (pDocShTables)
            pDocShTables->DoClose();        // delete passiert beim Zuweisen auf die Ref

        pDocShTables = new ScDocShell;
        aDocShTablesRef = pDocShTables;
        pDocShTables->DoLoad( pMed );

        ULONG nErr = pDocShTables->GetErrorCode();
        if (nErr)
            ErrorHandler::HandleError( nErr );              // auch Warnings

        if ( !pDocShTables->GetError() )                    // nur Errors
        {
            FillTables_Impl( pDocShTables->GetDocument() );
            aFtPath.SetText( pDocShTables->GetTitle( SFX_TITLE_FULLNAME ) );
        }
        else
        {
            pDocShTables->DoClose();
            aDocShTablesRef.Clear();
            pDocShTables = NULL;

            FillTables_Impl( NULL );
            aFtPath.SetText( EMPTY_STRING );
        }
    }

    DoEnable_Impl();
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScInsertTableDlg, SelectHdl_Impl, MultiListBox*, EMPTYARG )
{
    DoEnable_Impl();
    return 0;
}

//------------------------------------------------------------------------

void ScInsertTableDlg::DoEnable_Impl()
{
    if ( aBtnNew.IsChecked() || ( pDocShTables && aLbTables.GetSelectEntryCount() ) )
        aBtnOk.Enable();
    else
        aBtnOk.Disable();
}

IMPL_LINK( ScInsertTableDlg, DoEnterHdl, PushButton*, EMPTYARG )
{
    if(nTableCount > 1 || rDoc.ValidTabName(aEdName.GetText()))
    {
        EndDialog(RET_OK);
    }
    else
    {
        String aErrMsg ( ScGlobal::GetRscString( STR_INVALIDTABNAME ) );
        USHORT nRet = ErrorBox( this,WinBits( WB_OK | WB_DEF_OK ),aErrMsg).Execute();
    }
    return 0;
}



