/*************************************************************************
 *
 *  $RCSfile: dbnamdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
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

// System - Includes ---------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

// INCLUDE -------------------------------------------------------------------

#include <vcl/msgbox.hxx>

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "dbnamdlg.hrc"
#include "rangenam.hxx"     // IsNameValid

#define _DBNAMDLG_CXX
#include "dbnamdlg.hxx"
#undef _DBNAMDLG_CXX


//============================================================================

#define ABS_SREF          SCA_VALID \
                        | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
                        | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

//----------------------------------------------------------------------------

class DBSaveData;

static DBSaveData* pSaveObj = NULL;

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute()
#define QUERYBOX(m) QueryBox(this,WinBits(WB_YES_NO|WB_DEF_YES),m).Execute()

SEG_EOFGLOBALS()


//============================================================================
//  class DBSaveData

class DBSaveData
{
public:
    DBSaveData( Edit& rEd, CheckBox& rHdr, CheckBox& rSize, CheckBox& rFmt,
                            CheckBox& rStrip, ScRange& rArea )
        : bHeader(FALSE), bSize(FALSE), bFormat(FALSE), bDirty(FALSE),
          rEdAssign(rEd),
          rBtnHeader(rHdr), rBtnSize(rSize), rBtnFormat(rFmt), rBtnStrip(rStrip),
          rCurArea(rArea) {}

    void Clear();
    void Save();
    void Restore();

private:
    Edit&       rEdAssign;
    CheckBox&   rBtnHeader;
    CheckBox&   rBtnSize;
    CheckBox&   rBtnFormat;
    CheckBox&   rBtnStrip;
    ScRange&    rCurArea;
    String      aStr;
    ScRange     aArea;
    BOOL        bHeader:1;
    BOOL        bSize:1;
    BOOL        bFormat:1;
    BOOL        bStrip:1;
    BOOL        bDirty:1;
};



//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_0f)

void DBSaveData::Clear()
{
    aStr.Erase();
    aArea = ScRange();
    bHeader = bSize = bFormat = bStrip = bDirty = FALSE;
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_10)

void DBSaveData::Save()
{
    aArea   = rCurArea;
    aStr    = rEdAssign.GetText();
    bHeader = rBtnHeader.IsChecked();
    bSize   = rBtnSize.IsChecked();
    bFormat = rBtnFormat.IsChecked();
    bStrip  = rBtnStrip.IsChecked();
    bDirty  = TRUE;
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_11)

void DBSaveData::Restore()
{
    if ( bDirty )
    {
        rCurArea = aArea;
        rEdAssign.SetText( aStr );
        rBtnHeader.Check ( bHeader );
        rBtnSize.Check   ( bSize );
        rBtnFormat.Check ( bFormat );
        rBtnStrip.Check  ( bStrip );
        bDirty = FALSE;
    }
}


//============================================================================
//  class ScDbNameDlg

//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_01)

ScDbNameDlg::ScDbNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          ScViewData*   ptrViewData )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_DBNAMES ),
        //
        aGbName         ( this, ScResId( GB_NAME ) ),
        aEdName         ( this, ScResId( ED_NAME ) ),

        aGbAssign       ( this, ScResId( GB_ASSIGN ) ),
        aEdAssign       ( this, ScResId( ED_DBAREA ) ),
        aRbAssign       ( this, ScResId( RB_DBAREA ), &aEdAssign ),
        aGbOptions      ( this, ScResId( GB_OPTIONS ) ),
        aBtnHeader      ( this, ScResId( BTN_HEADER ) ),
        aBtnDoSize      ( this, ScResId( BTN_SIZE ) ),
        aBtnKeepFmt     ( this, ScResId( BTN_FORMAT ) ),
        aBtnStripData   ( this, ScResId( BTN_STRIPDATA ) ),
        aFTSource       ( this, ScResId( FT_SOURCE ) ),
        aFTOperations   ( this, ScResId( FT_OPERATIONS ) ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),

        aStrAdd         ( ScResId( STR_ADD ) ),
        aStrModify      ( ScResId( STR_MODIFY ) ),
        aStrNoName      ( ScGlobal::GetRscString(STR_DB_NONAME) ),
        aStrInvalid     ( ScResId( STR_DB_INVALID ) ),
        //
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),
        aLocalDbCol     ( *(pDoc->GetDBCollection()) ),
        bRefInputMode   ( FALSE )
{
    //  damit die Strings in der Resource bei den FixedTexten bleiben koennen:
    aStrSource      = aFTSource.GetText();
    aStrOperations  = aFTOperations.GetText();

    pSaveObj = new DBSaveData( aEdAssign, aBtnHeader,
                        aBtnDoSize, aBtnKeepFmt, aBtnStripData, theCurArea );
    Init();
    FreeResource();
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_02)

__EXPORT ScDbNameDlg::~ScDbNameDlg()
{
    DELETEZ( pSaveObj );

    ScRange* pEntry = (ScRange*)aRemoveList.First();
    while ( pEntry )
    {
        delete aRemoveList.Remove( pEntry );
        pEntry = (ScRange*)aRemoveList.Next();
    }
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_03)

void ScDbNameDlg::Init()
{
    aBtnHeader.Check( TRUE );       // Default: mit Spaltenkoepfen

    aBtnMore.AddWindow( &aGbOptions );
    aBtnMore.AddWindow( &aBtnHeader );
    aBtnMore.AddWindow( &aBtnDoSize );
    aBtnMore.AddWindow( &aBtnKeepFmt );
    aBtnMore.AddWindow( &aBtnStripData );
    aBtnMore.AddWindow( &aFTSource );
    aBtnMore.AddWindow( &aFTOperations );

    String  theAreaStr;
    USHORT  nStartCol   = 0;
    USHORT  nStartRow   = 0;
    USHORT  nStartTab   = 0;
    USHORT  nEndCol     = 0;
    USHORT  nEndRow     = 0;
    USHORT  nEndTab     = 0;

    aBtnOk.SetClickHdl      ( LINK( this, ScDbNameDlg, OkBtnHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScDbNameDlg, CancelBtnHdl ) );
    aBtnAdd.SetClickHdl     ( LINK( this, ScDbNameDlg, AddBtnHdl ) );
    aBtnRemove.SetClickHdl  ( LINK( this, ScDbNameDlg, RemoveBtnHdl ) );
    aEdName.SetModifyHdl    ( LINK( this, ScDbNameDlg, NameModifyHdl ) );
    aEdAssign.SetModifyHdl  ( LINK( this, ScDbNameDlg, AssModifyHdl ) );
    UpdateNames();

    if ( pViewData && pDoc )
    {
        ScDBCollection* pDBColl = pDoc->GetDBCollection();
        ScDBData*       pDBData = NULL;

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );

        theCurArea = ScRange( ScAddress( nStartCol, nStartRow, nStartTab ),
                              ScAddress( nEndCol,   nEndRow,   nEndTab ) );

        theCurArea.Format( theAreaStr, ABS_DREF3D, pDoc );

        if ( pDBColl )
        {
            // Feststellen, ob definierter DB-Bereich markiert wurde:
            pDBData = pDBColl->GetDBAtCursor( nStartCol, nStartRow, nStartTab, TRUE );
            if ( pDBData )
            {
                String      theDbName;
                ScAddress&  rStart = theCurArea.aStart;
                ScAddress&  rEnd   = theCurArea.aEnd;
                USHORT      nCol1, nCol2, nRow1, nRow2, nTab;

                pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );

                if (   (rStart.Tab() == nTab)
                    && (rStart.Col() == nCol1) && (rStart.Row() == nRow1)
                    && (rEnd.Col()   == nCol2) && (rEnd.Row()   == nRow2 ) )
                {
                    pDBData->GetName( theDbName );
                    if ( theDbName != aStrNoName )
                        aEdName.SetText( theDbName );
                    else
                        aEdName.SetText( EMPTY_STRING );
                    aBtnHeader.Check( pDBData->HasHeader() );
                    aBtnDoSize.Check( pDBData->IsDoSize() );
                    aBtnKeepFmt.Check( pDBData->IsKeepFmt() );
                    aBtnStripData.Check( pDBData->IsStripData() );
                    SetInfoStrings( pDBData );
                }
            }
        }
    }

    aEdAssign.SetText( theAreaStr );
    aEdName.GrabFocus();
    bSaved=TRUE;
    pSaveObj->Save();
    NameModifyHdl( 0 );
}

#pragma SEG_FUNCDEF(dbnamdlg_14)

void ScDbNameDlg::SetInfoStrings( const ScDBData* pDBData )
{
    String aSource = aStrSource;
    if (pDBData)
    {
        aSource += ' ';
        aSource += pDBData->GetSourceString();
    }
    aFTSource.SetText( aSource );

    String aOper = aStrOperations;
    if (pDBData)
    {
        aOper += ' ';
        aOper += pDBData->GetOperations();
    }
    aFTOperations.SetText( aOper );
}

//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
//  neue Selektion im Referenz-Fenster angezeigt wird.
#pragma SEG_FUNCDEF(dbnamdlg_04)

void ScDbNameDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( aEdAssign.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( &aEdAssign );

        theCurArea = rRef;

        String aRefStr;
        theCurArea.Format( aRefStr, ABS_DREF3D, pDoc );
        aEdAssign.SetRefString( aRefStr );
        aBtnHeader.Enable();
        aBtnDoSize.Enable();
        aBtnKeepFmt.Enable();
        aBtnStripData.Enable();
        aFTSource.Enable();
        aFTOperations.Enable();
        aBtnAdd.Enable();
        bSaved=TRUE;
        pSaveObj->Save();
    }
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_05)

BOOL __EXPORT ScDbNameDlg::Close()
{
    return DoClose( ScDbNameDlgWrapper::GetChildWindowId() );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_06)

void ScDbNameDlg::SetActive()
{
    aEdAssign.GrabFocus();

    //  kein NameModifyHdl, weil sonst Bereiche nicht geaendert werden koennen
    //  (nach dem Aufziehen der Referenz wuerde der alte Inhalt wieder angezeigt)
    //  (der ausgewaehlte DB-Name hat sich auch nicht veraendert)

    RefInputDone();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_07)

void ScDbNameDlg::UpdateNames()
{
    USHORT  nNameCount = aLocalDbCol.GetCount();

    aEdName.SetUpdateMode( FALSE );
    //-----------------------------------------------------------
    aEdName.Clear();
    aEdAssign.SetText( EMPTY_STRING );

    if ( nNameCount > 0 )
    {
        ScDBData*   pDbData = NULL;
        String      aString;

        for ( USHORT i=0; i<nNameCount; i++ )
        {
            pDbData = (ScDBData*)(aLocalDbCol.At( i ));
            if ( pDbData )
            {
                pDbData->GetName( aString );
                if ( aString != aStrNoName )
                    aEdName.InsertEntry( aString );
            }
        }
    }
    else
    {
        aBtnAdd.SetText( aStrAdd );
        aBtnAdd.Disable();
        aBtnRemove.Disable();
    }
    //-----------------------------------------------------------
    aEdName.SetUpdateMode( TRUE );
    aEdName.Invalidate();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_0e)

void ScDbNameDlg::UpdateDBData( const String& rStrName )
{
    String      theArea;
    USHORT      nAt;
    ScDBData*   pData;

    aLocalDbCol.SearchName( rStrName, nAt );
    pData = (ScDBData*)(aLocalDbCol.At( nAt ));

    if ( pData )
    {
        USHORT nColStart = 0;
        USHORT nRowStart = 0;
        USHORT nColEnd   = 0;
        USHORT nRowEnd   = 0;
        USHORT nTab      = 0;

        pData->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
        theCurArea = ScRange( ScAddress( nColStart, nRowStart, nTab ),
                              ScAddress( nColEnd,   nRowEnd,   nTab ) );
        theCurArea.Format( theArea, ABS_DREF3D, pDoc );
        aEdAssign.SetText( theArea );
        aBtnAdd.SetText( aStrModify );
        aBtnHeader.Check( pData->HasHeader() );
        aBtnDoSize.Check( pData->IsDoSize() );
        aBtnKeepFmt.Check( pData->IsKeepFmt() );
        aBtnStripData.Check( pData->IsStripData() );
        SetInfoStrings( pData );
    }

    aBtnAdd.SetText( aStrModify );
    aBtnAdd.Enable();
    aBtnRemove.Enable();
    aBtnHeader.Enable();
    aBtnDoSize.Enable();
    aBtnKeepFmt.Enable();
    aBtnStripData.Enable();
    aFTSource.Enable();
    aFTOperations.Enable();
}

//------------------------------------------------------------------------

#pragma SEG_FUNCDEF(dbnamdlg_12)

BOOL ScDbNameDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

//------------------------------------------------------------------------
// Handler:
// ========
#pragma SEG_FUNCDEF(dbnamdlg_08)

IMPL_LINK( ScDbNameDlg, OkBtnHdl, void *, EMPTYARG )
{
    AddBtnHdl( 0 );

    // Der View die Aenderungen und die Remove-Liste uebergeben:
    // beide werden nur als Referenz uebergeben, so dass an dieser
    // Stelle keine Speicherleichen entstehen koennen:
    if ( pViewData )
        pViewData->GetView()->
            NotifyCloseDbNameDlg( aLocalDbCol, aRemoveList );

    Close();
    return 0;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_0d)

IMPL_LINK_INLINE_START( ScDbNameDlg, CancelBtnHdl, void *, EMPTYARG )
{
    Close();
    return 0;
}
IMPL_LINK_INLINE_END( ScDbNameDlg, CancelBtnHdl, void *, EMPTYARG )

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_09)

IMPL_LINK( ScDbNameDlg, AddBtnHdl, void *, EMPTYARG )
{
    String  aNewName = aEdName.GetText();
    String  aNewArea = aEdAssign.GetText();

    aNewName.EraseLeadingChars( ' ' );
    aNewName.EraseTrailingChars( ' ' );

    if ( aNewName.Len() > 0 && aNewArea.Len() > 0 )
    {
        if ( ScRangeData::IsNameValid( aNewName, pDoc ) )
        {
            //  weil jetzt editiert werden kann, muss erst geparst werden
            ScRange aTmpRange;
            String aText = aEdAssign.GetText();
            if ( aTmpRange.ParseAny( aText, pDoc ) & SCA_VALID )
            {
                theCurArea = aTmpRange;
                ScAddress aStart = theCurArea.aStart;
                ScAddress aEnd   = theCurArea.aEnd;

                ScDBData* pOldEntry = NULL;
                USHORT nFoundAt = 0;
                if ( aLocalDbCol.SearchName( aNewName, nFoundAt ) )
                    pOldEntry = aLocalDbCol[nFoundAt];
                if (pOldEntry)
                {
                    //  Bereich veraendern

                    pOldEntry->MoveTo( aStart.Tab(), aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row() );
                    pOldEntry->SetByRow( TRUE );
                    pOldEntry->SetHeader( aBtnHeader.IsChecked() );
                    pOldEntry->SetDoSize( aBtnDoSize.IsChecked() );
                    pOldEntry->SetKeepFmt( aBtnKeepFmt.IsChecked() );
                    pOldEntry->SetStripData( aBtnStripData.IsChecked() );
                }
                else
                {
                    //  neuen Bereich einfuegen

                    ScDBData* pNewEntry = new ScDBData( aNewName, aStart.Tab(),
                                                        aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row(),
                                                        TRUE, aBtnHeader.IsChecked() );
                    pNewEntry->SetDoSize( aBtnDoSize.IsChecked() );
                    pNewEntry->SetKeepFmt( aBtnKeepFmt.IsChecked() );
                    pNewEntry->SetStripData( aBtnStripData.IsChecked() );

                    if ( !aLocalDbCol.Insert( pNewEntry ) )
                        delete pNewEntry;
                }

                UpdateNames();

                aEdName.SetText( EMPTY_STRING );
                aEdName.GrabFocus();
                aBtnAdd.SetText( aStrAdd );
                aBtnAdd.Disable();
                aBtnRemove.Disable();
                aEdAssign.SetText( EMPTY_STRING );
                aBtnHeader.Check( TRUE );       // Default: mit Spaltenkoepfen
                aBtnDoSize.Check( FALSE );
                aBtnKeepFmt.Check( FALSE );
                aBtnStripData.Check( FALSE );
                SetInfoStrings( NULL );     // leer
                theCurArea = ScRange();
                bSaved=TRUE;
                pSaveObj->Save();
                NameModifyHdl( 0 );
            }
            else
            {
                ERRORBOX( aStrInvalid );
                aEdAssign.SetSelection( Selection( 0, SELECTION_MAX ) );
                aEdAssign.GrabFocus();
            }
        }
        else
        {
            ERRORBOX( ScGlobal::GetRscString(STR_INVALIDNAME) );
            aEdName.SetSelection( Selection( 0, SELECTION_MAX ) );
            aEdName.GrabFocus();
        }
    }
    return 0;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_0a)

IMPL_LINK( ScDbNameDlg, RemoveBtnHdl, void *, EMPTYARG )
{
    USHORT       nRemoveAt = 0;
    const String aStrEntry = aEdName.GetText();

    if ( aLocalDbCol.SearchName( aStrEntry, nRemoveAt ) )
    {
        String aStrDelMsg = ScGlobal::GetRscString( STR_QUERY_DELENTRY );
        String aMsg       = aStrDelMsg.GetToken( 0, '#' );

        aMsg += aStrEntry;
        aMsg += aStrDelMsg.GetToken( 1, '#' );

        if ( RET_YES == QUERYBOX(aMsg) )
        {
            ScDBData* pEntry = (ScDBData*)aLocalDbCol.At(nRemoveAt);

            if ( pEntry )
            {
                USHORT nTab, nColStart, nRowStart, nColEnd, nRowEnd;
                pEntry->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
                aRemoveList.Insert(
                    new ScRange( ScAddress( nColStart, nRowStart, nTab ),
                                 ScAddress( nColEnd,   nRowEnd,   nTab ) ) );
            }
            aLocalDbCol.AtFree( nRemoveAt );

            UpdateNames();

            aEdName.SetText( EMPTY_STRING );
            aEdName.GrabFocus();
            aBtnAdd.SetText( aStrAdd );
            aBtnAdd.Disable();
            aBtnRemove.Disable();
            aEdAssign.SetText( EMPTY_STRING );
            theCurArea = ScRange();
            aBtnHeader.Check( TRUE );       // Default: mit Spaltenkoepfen
            aBtnDoSize.Check( FALSE );
            aBtnKeepFmt.Check( FALSE );
            aBtnStripData.Check( FALSE );
            SetInfoStrings( NULL );     // leer
            bSaved=FALSE;
            pSaveObj->Restore();
            NameModifyHdl( 0 );
        }
    }
    return 0;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_0c)

IMPL_LINK( ScDbNameDlg, NameModifyHdl, void *, EMPTYARG )
{
    String  theName     = aEdName.GetText();
    BOOL    bNameFound  = (COMBOBOX_ENTRY_NOTFOUND
                           != aEdName.GetEntryPos( theName ));

    if ( theName.Len() == 0 )
    {
        if ( aBtnAdd.GetText() != aStrAdd )
            aBtnAdd.SetText( aStrAdd );
        aBtnAdd     .Disable();
        aBtnRemove  .Disable();
        aGbAssign   .Disable();
        aBtnHeader  .Disable();
        aBtnDoSize  .Disable();
        aBtnKeepFmt .Disable();
        aBtnStripData.Disable();
        aFTSource   .Disable();
        aFTOperations.Disable();
        aEdAssign   .Disable();
        aRbAssign   .Disable();
        //bSaved=FALSE;
        //pSaveObj->Restore();
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
        bRefInputMode = FALSE;
    }
    else
    {
        if ( bNameFound )
        {
            if ( aBtnAdd.GetText() != aStrModify )
                aBtnAdd.SetText( aStrModify );

            if(!bSaved)
            {
                bSaved=TRUE;
                pSaveObj->Save();
            }
            UpdateDBData( theName );
        }
        else
        {
            if ( aBtnAdd.GetText() != aStrAdd )
                aBtnAdd.SetText( aStrAdd );

            bSaved=FALSE;
            pSaveObj->Restore();

            if ( aEdAssign.GetText().Len() > 0 )
            {
                aBtnAdd.Enable();
                aBtnHeader.Enable();
                aBtnDoSize.Enable();
                aBtnKeepFmt.Enable();
                aBtnStripData.Enable();
                aFTSource.Enable();
                aFTOperations.Enable();
            }
            else
            {
                aBtnAdd.Disable();
                aBtnHeader.Disable();
                aBtnDoSize.Disable();
                aBtnKeepFmt.Disable();
                aBtnStripData.Disable();
                aFTSource.Disable();
                aFTOperations.Disable();
            }
            aBtnRemove.Disable();
        }

        aGbAssign.Enable();
        aEdAssign.Enable();
        aRbAssign.Enable();

        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();
        bRefInputMode = TRUE;
    }
    return 0;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(dbnamdlg_13)

IMPL_LINK( ScDbNameDlg, AssModifyHdl, void *, EMPTYARG )
{
    //  hier parsen fuer Save() etc.

    ScRange aTmpRange;
    String aText = aEdAssign.GetText();
    if ( aTmpRange.ParseAny( aText, pDoc ) & SCA_VALID )
        theCurArea = aTmpRange;

    return 0;
}

/*----------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.54  2000/09/17 14:08:56  willem.vandorp
    OpenOffice header added.

    Revision 1.53  2000/08/31 16:38:20  willem.vandorp
    Header and footer replaced

    Revision 1.52  2000/02/11 12:22:54  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.51  1999/06/09 16:09:04  NN
    Sfx-appwin removed


      Rev 1.50   09 Jun 1999 18:09:04   NN
   Sfx-appwin removed

      Rev 1.49   10 Dec 1998 20:09:18   ANK
   #51738# Umstellung auf ScRefEdit zum Anzeigen von Referenzen

      Rev 1.48   12 Aug 1998 19:47:34   ANK
   #54702# IsRefInputMode ueberarbeitet

      Rev 1.47   06 Aug 1998 21:19:48   ANK
   #54702# Enablen/Disablen der Applikation nur noch in ScAnyRefDlg

      Rev 1.46   01 Jul 1998 15:00:12   ANK
   MUSS-Aenderung wg. Autocomplete

      Rev 1.45   15 Mar 1998 14:23:20   NN
   App-Fenster disablen mit bChild=FALSE

      Rev 1.44   05 Dec 1997 19:56:22   ANK
   Includes geaendert

      Rev 1.43   03 Sep 1997 15:23:42   RG
   change header

      Rev 1.42   12 Jun 1997 13:16:18   NN
   #40646# STR_DB_NONAME statt SCSTR_NONAME fuer DB-Bereiche

      Rev 1.41   04 Mar 1997 18:47:48   NN
   37186# nur gueltige Namen (wie Bereichsnamen) zulassen

      Rev 1.40   23 Feb 1997 22:49:22   NN
   Button 'Importierte Daten nicht speichern'

      Rev 1.39   13 Feb 1997 22:46:16   NN
   neue Optionen einstellbar

      Rev 1.38   17 Jan 1997 15:10:44   NN
   #35110# Bereich kann auch im Edit eingegeben werden

      Rev 1.37   18 Dec 1996 14:22:32   ER
   AddBtnHdl: Index eines geaenderten Bereiches erhalten

      Rev 1.36   13 Nov 1996 20:11:16   NN
   #33143# AutoHide nur bei Bereichsreferenz

      Rev 1.35   29 Oct 1996 14:03:34   NN
   ueberall ScResId statt ResId

      Rev 1.34   22 Oct 1996 15:29:58   RJ
   Buttons fuer Referenzeingabe

      Rev 1.33   08 Oct 1996 15:00:02   RJ
   Einklappen des Fensters bei Referenzeingabe

      Rev 1.32   15 Aug 1996 12:20:26   NN
   #30362# SetReference: ggf. Add-Button enablen

      Rev 1.31   18 Jul 1996 16:51:54   NN
   Parameter bStartOnly bei IsDBAtCursor

      Rev 1.30   10 Jun 1996 16:28:48   NN
   #28513# Restore statt Save im RemoveButtonHdl

      Rev 1.29   07 Jun 1996 14:23:42   NN
   #28451# kein NameModifyHdl in SetActive

      Rev 1.28   05 Jun 1996 19:19:00   NN
   kein Show im ctor (Position wird von aussen gesetzt)

      Rev 1.27   26 Apr 1996 12:02:40   NN
   SfxModelessDialog statt ModelessDialog

      Rev 1.26   25 Apr 1996 18:08:34   NN
   SetReference aufgeteilt in SetReference und AddRefEntry

      Rev 1.25   29 Jan 1996 15:12:22   MO
   neuer Link

      Rev 1.24   27 Nov 1995 16:39:50   MO
   RangeUtil/Area/Tripel gegen Address/Range ersetzt

      Rev 1.23   14 Nov 1995 18:05:36   MO
   Disable-Enable-Logik, DBName ueberpruefen

      Rev 1.22   09 Nov 1995 10:20:14   JN
   weitere Verbesseungen Modeless Dialoge

      Rev 1.21   08 Nov 1995 13:05:48   MO
   301-Aenderungen

      Rev 1.20   08 Nov 1995 10:45:48   JN
   Umstellung auf SfxChildWindow

      Rev 1.19   26 Oct 1995 11:29:44   MO
   Nachfrage beim Loeschen von Eintraegen

      Rev 1.18   14 Sep 1995 12:37:24   MO
   FixedInfos

      Rev 1.17   31 Jul 1995 08:58:00   MO
   CancelBtnHdl

      Rev 1.16   25 Apr 1995 17:01:30   MO
   Knopfflackern beseitigt

      Rev 1.15   20 Apr 1995 10:34:06   MO
   Show() im Ctor

      Rev 1.14   18 Apr 1995 16:55:00   MO
   Flackern bei SetText() an Buttons unterbunden

      Rev 1.13   24 Mar 1995 15:37:50   MO
   Mauseingabe von Referenzen korrigiert

      Rev 1.12   29 Jan 1995 13:17:42   NN
   include dbfunc statt tabview

      Rev 1.11   27 Jan 1995 18:39:50   MO
   __EXECUTE durch __EXPORT ersetzt

      Rev 1.10   26 Jan 1995 18:28:00   TRI
   __EXPORT bei virtuellen Methoden eingebaut

      Rev 1.9   23 Jan 1995 16:44:34   MO
   * unbenannt-DB-Bereich wird ignoriert
   * NotifyClose nicht mehr im Close() sondern im Ok-Handler
     damit ist wieder Abbrechen moeglich

      Rev 1.8   19 Jan 1995 17:12:14   TRI
   __EXPORT vor verschiedene LinkHandler gesetzt

      Rev 1.7   19 Jan 1995 11:36:20   MO
   Uebergabe der Veraenderungen an die View in der Close()-Methode.
   Dabei werden nur Referenzen weitergereicht, so dass im Dialog
   keine Speicherleichen entstehen.

      Rev 1.6   18 Jan 1995 13:56:26   TRI
   Pragmas zur Segementierung eingebaut

      Rev 1.5   12 Jan 1995 14:45:56   MO
   * Verwendung von ScRangeUtil
   * Enable/Disable-Logik ueberarbeitet
   * Erkennung von DB-Bereichen

      Rev 1.4   04 Jan 1995 19:28:00   MO
   Verwendung von DbUiUtil

      Rev 1.3   04 Jan 1995 13:09:46   MO
   Hinzufuegen/Loeschen-Knoepfe: Enable/Diable-Fehler beseitigt

      Rev 1.2   23 Dec 1994 12:26:52   MO
   Bereichseingabe nur noch mit der Maus moeglich

      Rev 1.1   23 Dec 1994 09:27:58   MO
   Korrigierte Version. Zusammen mit namedlg und den
   Collections auf einen lauffaehhigen Stand gebracht.

----------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

