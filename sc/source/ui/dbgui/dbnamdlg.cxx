/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "rangenam.hxx"     // IsNameValid
#include "globalnames.hxx"
#include "dbnamdlg.hxx"


//============================================================================

#define ABS_SREF          SCA_VALID \
                        | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
                        | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

//----------------------------------------------------------------------------

class DBSaveData;

static DBSaveData* pSaveObj = NULL;

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute()

//============================================================================
//  class DBSaveData

class DBSaveData
{
public:
    DBSaveData( Edit& rEd, CheckBox& rHdr, CheckBox& rSize, CheckBox& rFmt,
                            CheckBox& rStrip, ScRange& rArea )
        : rEdAssign(rEd),
          rBtnHeader(rHdr), rBtnSize(rSize), rBtnFormat(rFmt), rBtnStrip(rStrip),
          rCurArea(rArea),
          bHeader(false), bSize(false), bFormat(false), bDirty(false) {}
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
    sal_Bool        bHeader:1;
    sal_Bool        bSize:1;
    sal_Bool        bFormat:1;
    sal_Bool        bStrip:1;
    sal_Bool        bDirty:1;
};



//----------------------------------------------------------------------------

void DBSaveData::Save()
{
    aArea   = rCurArea;
    aStr    = rEdAssign.GetText();
    bHeader = rBtnHeader.IsChecked();
    bSize   = rBtnSize.IsChecked();
    bFormat = rBtnFormat.IsChecked();
    bStrip  = rBtnStrip.IsChecked();
    bDirty  = sal_True;
}


//----------------------------------------------------------------------------

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
        bDirty = false;
    }
}


//============================================================================
//  class ScDbNameDlg

//----------------------------------------------------------------------------

ScDbNameDlg::ScDbNameDlg(SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
    ScViewData* ptrViewData)
    : ScAnyRefDlg(pB, pCW, pParent,
        "DefineDatabaseRangeDialog",
            "modules/scalc/ui/definedatabaserangedialog.ui")
    , pViewData(ptrViewData)
    , pDoc(ptrViewData->GetDocument())
    , bRefInputMode(false)
    , aAddrDetails(pDoc->GetAddressConvention(), 0, 0)
    , aLocalDbCol(*(pDoc->GetDBCollection()))
{
    get(m_pEdName, "entry");
    m_pEdName->set_height_request(m_pEdName->GetOptimalSize().Height() + m_pEdName->GetTextHeight() * 8);
    get(m_pEdAssign, "assign");
    get(m_pAssignFrame, "RangeFrame");
    m_pEdAssign->SetReferences(this, m_pAssignFrame->get_label_widget());
    get(m_pRbAssign, "assignrb");
    m_pRbAssign->SetReferences(this, m_pEdAssign);
    get(m_pOptions, "Options");
    get(m_pBtnHeader, "ContainsColumnLabels");
    get(m_pBtnDoSize, "InsertOrDeleteCells");
    get(m_pBtnKeepFmt, "KeepFormatting");
    get(m_pBtnStripData, "DontSaveImportedData");
    get(m_pFTSource, "Source");
    get(m_pFTOperations, "Operations");
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    get(m_pBtnAdd, "add");
    aStrAdd = m_pBtnAdd->GetText();
    aStrModify = get<Window>("modify")->GetText();
    get(m_pBtnRemove, "delete");
    aStrInvalid = get<Window>("invalid")->GetText();

    m_pFTSource->SetStyle(m_pFTSource->GetStyle() | WB_NOLABEL);
    m_pFTOperations->SetStyle(m_pFTOperations->GetStyle() | WB_NOLABEL);

    //  damit die Strings in der Resource bei den FixedTexten bleiben koennen:
    aStrSource      = m_pFTSource->GetText();
    aStrOperations  = m_pFTOperations->GetText();

    pSaveObj = new DBSaveData( *m_pEdAssign, *m_pBtnHeader,
                        *m_pBtnDoSize, *m_pBtnKeepFmt, *m_pBtnStripData, theCurArea );
    Init();
}


//----------------------------------------------------------------------------

ScDbNameDlg::~ScDbNameDlg()
{
    DELETEZ( pSaveObj );
}


//----------------------------------------------------------------------------

void ScDbNameDlg::Init()
{
    m_pBtnHeader->Check( sal_True );       // Default: mit Spaltenkoepfen

    m_pBtnOk->SetClickHdl      ( LINK( this, ScDbNameDlg, OkBtnHdl ) );
    m_pBtnCancel->SetClickHdl  ( LINK( this, ScDbNameDlg, CancelBtnHdl ) );
    m_pBtnAdd->SetClickHdl     ( LINK( this, ScDbNameDlg, AddBtnHdl ) );
    m_pBtnRemove->SetClickHdl  ( LINK( this, ScDbNameDlg, RemoveBtnHdl ) );
    m_pEdName->SetModifyHdl    ( LINK( this, ScDbNameDlg, NameModifyHdl ) );
    m_pEdAssign->SetModifyHdl  ( LINK( this, ScDbNameDlg, AssModifyHdl ) );
    UpdateNames();

    String  theAreaStr;

    if ( pViewData && pDoc )
    {
        SCCOL   nStartCol   = 0;
        SCROW   nStartRow   = 0;
        SCTAB   nStartTab   = 0;
        SCCOL   nEndCol     = 0;
        SCROW   nEndRow     = 0;
        SCTAB   nEndTab     = 0;

        ScDBCollection* pDBColl = pDoc->GetDBCollection();
        ScDBData*       pDBData = NULL;

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );

        theCurArea = ScRange( ScAddress( nStartCol, nStartRow, nStartTab ),
                              ScAddress( nEndCol,   nEndRow,   nEndTab ) );

        theAreaStr = theCurArea.Format(ABS_DREF3D, pDoc, aAddrDetails);

        if ( pDBColl )
        {
            // Feststellen, ob definierter DB-Bereich markiert wurde:
            pDBData = pDBColl->GetDBAtCursor( nStartCol, nStartRow, nStartTab, sal_True );
            if ( pDBData )
            {
                ScAddress&  rStart = theCurArea.aStart;
                ScAddress&  rEnd   = theCurArea.aEnd;
                SCCOL nCol1;
                SCCOL  nCol2;
                SCROW  nRow1;
                SCROW  nRow2;
                SCTAB  nTab;

                pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );

                if (   (rStart.Tab() == nTab)
                    && (rStart.Col() == nCol1) && (rStart.Row() == nRow1)
                    && (rEnd.Col()   == nCol2) && (rEnd.Row()   == nRow2 ) )
                {
                    OUString aDBName = pDBData->GetName();
                    if ( aDBName != STR_DB_LOCAL_NONAME )
                        m_pEdName->SetText(aDBName);

                    m_pBtnHeader->Check( pDBData->HasHeader() );
                    m_pBtnDoSize->Check( pDBData->IsDoSize() );
                    m_pBtnKeepFmt->Check( pDBData->IsKeepFmt() );
                    m_pBtnStripData->Check( pDBData->IsStripData() );
                    SetInfoStrings( pDBData );
                }
            }
        }
    }

    m_pEdAssign->SetText( theAreaStr );
    m_pEdName->GrabFocus();
    bSaved=sal_True;
    pSaveObj->Save();
    NameModifyHdl( 0 );
}


void ScDbNameDlg::SetInfoStrings( const ScDBData* pDBData )
{
    OUStringBuffer aBuf;
    aBuf.append(aStrSource);
    if (pDBData)
    {
        aBuf.append(sal_Unicode(' '));
        aBuf.append(pDBData->GetSourceString());
    }
    m_pFTSource->SetText(aBuf.makeStringAndClear());

    aBuf.append(aStrOperations);
    if (pDBData)
    {
        aBuf.append(sal_Unicode(' '));
        aBuf.append(pDBData->GetOperations());
    }
    m_pFTOperations->SetText(aBuf.makeStringAndClear());
}

//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
//  neue Selektion im Referenz-Fenster angezeigt wird.

void ScDbNameDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdAssign->IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdAssign);

        theCurArea = rRef;

        OUString aRefStr(theCurArea.Format(ABS_DREF3D, pDocP, aAddrDetails));
        m_pEdAssign->SetRefString( aRefStr );
        m_pOptions->Enable();
        m_pBtnAdd->Enable();
        bSaved=sal_True;
        pSaveObj->Save();
    }
}


//----------------------------------------------------------------------------

sal_Bool ScDbNameDlg::Close()
{
    return DoClose( ScDbNameDlgWrapper::GetChildWindowId() );
}

//------------------------------------------------------------------------

void ScDbNameDlg::SetActive()
{
    m_pEdAssign->GrabFocus();

    //  kein NameModifyHdl, weil sonst Bereiche nicht geaendert werden koennen
    //  (nach dem Aufziehen der Referenz wuerde der alte Inhalt wieder angezeigt)
    //  (der ausgewaehlte DB-Name hat sich auch nicht veraendert)

    RefInputDone();
}

//------------------------------------------------------------------------

void ScDbNameDlg::UpdateNames()
{
    typedef ScDBCollection::NamedDBs DBsType;

    const DBsType& rDBs = aLocalDbCol.getNamedDBs();

    m_pEdName->SetUpdateMode( false );
    //-----------------------------------------------------------
    m_pEdName->Clear();
    m_pEdAssign->SetText( EMPTY_STRING );

    if (!rDBs.empty())
    {
        DBsType::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
        for (; itr != itrEnd; ++itr)
            m_pEdName->InsertEntry(itr->GetName());
    }
    else
    {
        m_pBtnAdd->SetText( aStrAdd );
        m_pBtnAdd->Disable();
        m_pBtnRemove->Disable();
    }
    //-----------------------------------------------------------
    m_pEdName->SetUpdateMode( sal_True );
    m_pEdName->Invalidate();
}

//------------------------------------------------------------------------

void ScDbNameDlg::UpdateDBData( const String& rStrName )
{

    const ScDBData* pData = aLocalDbCol.getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rStrName));

    if ( pData )
    {
        SCCOL nColStart = 0;
        SCROW nRowStart = 0;
        SCCOL nColEnd    = 0;
        SCROW nRowEnd    = 0;
        SCTAB nTab       = 0;

        pData->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
        theCurArea = ScRange( ScAddress( nColStart, nRowStart, nTab ),
                              ScAddress( nColEnd,   nRowEnd,   nTab ) );
        OUString theArea(theCurArea.Format(ABS_DREF3D, pDoc, aAddrDetails));
        m_pEdAssign->SetText( theArea );
        m_pBtnAdd->SetText( aStrModify );
        m_pBtnHeader->Check( pData->HasHeader() );
        m_pBtnDoSize->Check( pData->IsDoSize() );
        m_pBtnKeepFmt->Check( pData->IsKeepFmt() );
        m_pBtnStripData->Check( pData->IsStripData() );
        SetInfoStrings( pData );
    }

    m_pBtnAdd->SetText( aStrModify );
    m_pBtnAdd->Enable();
    m_pBtnRemove->Enable();
    m_pOptions->Enable();
}

//------------------------------------------------------------------------


sal_Bool ScDbNameDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

//------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK_NOARG(ScDbNameDlg, OkBtnHdl)
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

IMPL_LINK_NOARG_INLINE_START(ScDbNameDlg, CancelBtnHdl)
{
    Close();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScDbNameDlg, CancelBtnHdl)

//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScDbNameDlg, AddBtnHdl)
{
    String  aNewName = comphelper::string::strip(m_pEdName->GetText(), ' ');
    String  aNewArea = m_pEdAssign->GetText();

    if ( aNewName.Len() > 0 && aNewArea.Len() > 0 )
    {
        if ( ScRangeData::IsNameValid( aNewName, pDoc ) && !aNewName.EqualsAscii(STR_DB_LOCAL_NONAME) )
        {
            //  weil jetzt editiert werden kann, muss erst geparst werden
            ScRange aTmpRange;
            String aText = m_pEdAssign->GetText();
            if ( aTmpRange.ParseAny( aText, pDoc, aAddrDetails ) & SCA_VALID )
            {
                theCurArea = aTmpRange;
                ScAddress aStart = theCurArea.aStart;
                ScAddress aEnd   = theCurArea.aEnd;

                ScDBData* pOldEntry = aLocalDbCol.getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(aNewName));
                if (pOldEntry)
                {
                    //  Bereich veraendern

                    pOldEntry->MoveTo( aStart.Tab(), aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row() );
                    pOldEntry->SetByRow( sal_True );
                    pOldEntry->SetHeader( m_pBtnHeader->IsChecked() );
                    pOldEntry->SetDoSize( m_pBtnDoSize->IsChecked() );
                    pOldEntry->SetKeepFmt( m_pBtnKeepFmt->IsChecked() );
                    pOldEntry->SetStripData( m_pBtnStripData->IsChecked() );
                }
                else
                {
                    //  neuen Bereich einfuegen

                    ScDBData* pNewEntry = new ScDBData( aNewName, aStart.Tab(),
                                                        aStart.Col(), aStart.Row(),
                                                        aEnd.Col(), aEnd.Row(),
                                                        sal_True, m_pBtnHeader->IsChecked() );
                    pNewEntry->SetDoSize( m_pBtnDoSize->IsChecked() );
                    pNewEntry->SetKeepFmt( m_pBtnKeepFmt->IsChecked() );
                    pNewEntry->SetStripData( m_pBtnStripData->IsChecked() );

                    aLocalDbCol.getNamedDBs().insert(pNewEntry);
                }

                UpdateNames();

                m_pEdName->SetText( EMPTY_STRING );
                m_pEdName->GrabFocus();
                m_pBtnAdd->SetText( aStrAdd );
                m_pBtnAdd->Disable();
                m_pBtnRemove->Disable();
                m_pEdAssign->SetText( EMPTY_STRING );
                m_pBtnHeader->Check( sal_True );       // Default: mit Spaltenkoepfen
                m_pBtnDoSize->Check( false );
                m_pBtnKeepFmt->Check( false );
                m_pBtnStripData->Check( false );
                SetInfoStrings( NULL );     // leer
                theCurArea = ScRange();
                bSaved=sal_True;
                pSaveObj->Save();
                NameModifyHdl( 0 );
            }
            else
            {
                ERRORBOX( aStrInvalid );
                m_pEdAssign->SetSelection( Selection( 0, SELECTION_MAX ) );
                m_pEdAssign->GrabFocus();
            }
        }
        else
        {
            ERRORBOX( ScGlobal::GetRscString(STR_INVALIDNAME) );
            m_pEdName->SetSelection( Selection( 0, SELECTION_MAX ) );
            m_pEdName->GrabFocus();
        }
    }
    return 0;
}

namespace {

class FindByName : public ::std::unary_function<ScDBData, bool>
{
    const OUString& mrName;
public:
    FindByName(const OUString& rName) : mrName(rName) {}
    bool operator() (const ScDBData& r) const
    {
        return r.GetName().equals(mrName);
    }
};

}

IMPL_LINK_NOARG(ScDbNameDlg, RemoveBtnHdl)
{
    OUString aStrEntry = m_pEdName->GetText();
    ScDBCollection::NamedDBs& rDBs = aLocalDbCol.getNamedDBs();
    ScDBCollection::NamedDBs::iterator itr =
        ::std::find_if(rDBs.begin(), rDBs.end(), FindByName(aStrEntry));

    if (itr != rDBs.end())
    {
        String aStrDelMsg = ScGlobal::GetRscString( STR_QUERY_DELENTRY );

        OUStringBuffer aBuf;
        aBuf.append(aStrDelMsg.GetToken(0, '#'));
        aBuf.append(aStrEntry);
        aBuf.append(aStrDelMsg.GetToken(1, '#'));
        QueryBox aBox(this, WinBits(WB_YES_NO|WB_DEF_YES), aBuf.makeStringAndClear());

        if (RET_YES == aBox.Execute())
        {
            SCTAB nTab;
            SCCOL nColStart, nColEnd;
            SCROW nRowStart, nRowEnd;
            itr->GetArea( nTab, nColStart, nRowStart, nColEnd, nRowEnd );
            aRemoveList.push_back(
                ScRange( ScAddress( nColStart, nRowStart, nTab ),
                         ScAddress( nColEnd,   nRowEnd,   nTab ) ) );

            rDBs.erase(itr);

            UpdateNames();

            m_pEdName->SetText( EMPTY_STRING );
            m_pEdName->GrabFocus();
            m_pBtnAdd->SetText( aStrAdd );
            m_pBtnAdd->Disable();
            m_pBtnRemove->Disable();
            m_pEdAssign->SetText( EMPTY_STRING );
            theCurArea = ScRange();
            m_pBtnHeader->Check( sal_True );       // Default: mit Spaltenkoepfen
            m_pBtnDoSize->Check( false );
            m_pBtnKeepFmt->Check( false );
            m_pBtnStripData->Check( false );
            SetInfoStrings( NULL );     // leer
            bSaved=false;
            pSaveObj->Restore();
            NameModifyHdl( 0 );
        }
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScDbNameDlg, NameModifyHdl)
{
    OUString  theName     = m_pEdName->GetText();
    sal_Bool    bNameFound  = (COMBOBOX_ENTRY_NOTFOUND
                           != m_pEdName->GetEntryPos( theName ));

    if ( theName.isEmpty() )
    {
        if (m_pBtnAdd->GetText() != aStrAdd)
            m_pBtnAdd->SetText( aStrAdd );
        m_pBtnAdd->Disable();
        m_pBtnRemove->Disable();
        m_pAssignFrame->Disable();
        m_pOptions->Disable();
        //bSaved=sal_False;
        //pSaveObj->Restore();
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Disable(sal_False);        //! allgemeine Methode im ScAnyRefDlg
        bRefInputMode = false;
    }
    else
    {
        if ( bNameFound )
        {
            if (m_pBtnAdd->GetText() != aStrModify)
                m_pBtnAdd->SetText( aStrModify );

            if(!bSaved)
            {
                bSaved=sal_True;
                pSaveObj->Save();
            }
            UpdateDBData( theName );
        }
        else
        {
            if (m_pBtnAdd->GetText() != aStrAdd)
                m_pBtnAdd->SetText( aStrAdd );

            bSaved=false;
            pSaveObj->Restore();

            if ( !m_pEdAssign->GetText().isEmpty() )
            {
                m_pBtnAdd->Enable();
                m_pOptions->Enable();
            }
            else
            {
                m_pBtnAdd->Disable();
                m_pOptions->Disable();
            }
            m_pBtnRemove->Disable();
        }

        m_pAssignFrame->Enable();

        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();
        bRefInputMode = true;
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(ScDbNameDlg, AssModifyHdl)
{
    //  hier parsen fuer Save() etc.

    ScRange aTmpRange;
    String aText = m_pEdAssign->GetText();
    if ( aTmpRange.ParseAny( aText, pDoc, aAddrDetails ) & SCA_VALID )
        theCurArea = aTmpRange;

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
