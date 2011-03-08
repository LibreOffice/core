/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifdef _MSC_VER
#pragma optimize("",off)
#endif

//------------------------------------------------------------------



// INCLUDE ---------------------------------------------------------

#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "tablink.hxx"

#include "scextopt.hxx"
#include "table.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "globstr.hrc"
#include "undoblk.hxx"
#include "undotab.hxx"
#include "global.hxx"
#include "hints.hxx"
#include "cell.hxx"
#include "dociter.hxx"
#include "formula/opcode.hxx"

struct TableLink_Impl
{
    ScDocShell* m_pDocSh;
    Window*     m_pOldParent;
    Link        m_aEndEditLink;

    TableLink_Impl() : m_pDocSh( NULL ), m_pOldParent( NULL ) {}
};

TYPEINIT1(ScTableLink, ::sfx2::SvBaseLink);

//------------------------------------------------------------------------

ScTableLink::ScTableLink(ScDocShell* pDocSh, const String& rFile,
                            const String& rFilter, const String& rOpt,
                            ULONG nRefresh ):
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ONCALL,FORMAT_FILE),
    ScRefreshTimer( nRefresh ),
    pImpl( new TableLink_Impl ),
    aFileName(rFile),
    aFilterName(rFilter),
    aOptions(rOpt),
    bInCreate( FALSE ),
    bInEdit( FALSE ),
    bAddUndo( TRUE ),
    bDoPaint( TRUE )
{
    pImpl->m_pDocSh = pDocSh;
}

ScTableLink::ScTableLink(SfxObjectShell* pShell, const String& rFile,
                            const String& rFilter, const String& rOpt,
                            ULONG nRefresh ):
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ONCALL,FORMAT_FILE),
    ScRefreshTimer( nRefresh ),
    pImpl( new TableLink_Impl ),
    aFileName(rFile),
    aFilterName(rFilter),
    aOptions(rOpt),
    bInCreate( FALSE ),
    bInEdit( FALSE ),
    bAddUndo( TRUE ),
    bDoPaint( TRUE )
{
    pImpl->m_pDocSh = static_cast< ScDocShell* >( pShell );
    SetRefreshHandler( LINK( this, ScTableLink, RefreshHdl ) );
    SetRefreshControl( pImpl->m_pDocSh->GetDocument()->GetRefreshTimerControlAddress() );
}

ScTableLink::~ScTableLink()
{
    // Verbindung aufheben

    StopRefreshTimer();
    String aEmpty;
    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (pDoc->IsLinked(nTab) && pDoc->GetLinkDoc(nTab)==aFileName)
            pDoc->SetLink( nTab, SC_LINK_NONE, aEmpty, aEmpty, aEmpty, aEmpty, 0 );
    delete pImpl;
}

void ScTableLink::Edit( Window* pParent, const Link& rEndEditHdl )
{
    //  DefModalDialogParent setzen, weil evtl. aus der DocShell beim ConvertFrom
    //  ein Optionen-Dialog kommt...

    pImpl->m_aEndEditLink = rEndEditHdl;
    pImpl->m_pOldParent = Application::GetDefDialogParent();
    if (pParent)
        Application::SetDefDialogParent(pParent);

    bInEdit = TRUE;
    SvBaseLink::Edit( pParent, LINK( this, ScTableLink, TableEndEditHdl ) );
}

void ScTableLink::DataChanged( const String&,
                                        const ::com::sun::star::uno::Any& )
{
    sfx2::LinkManager* pLinkManager=pImpl->m_pDocSh->GetDocument()->GetLinkManager();
    if (pLinkManager!=NULL)
    {
        String aFile;
        String aFilter;
        pLinkManager->GetDisplayNames( this,0,&aFile,NULL,&aFilter);

        //  the file dialog returns the filter name with the application prefix
        //  -> remove prefix
        ScDocumentLoader::RemoveAppPrefix( aFilter );

        if (!bInCreate)
            Refresh( aFile, aFilter, NULL, GetRefreshDelay() ); // don't load twice
    }
}

void ScTableLink::Closed()
{
    // Verknuepfung loeschen: Undo
    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
    BOOL bUndo (pDoc->IsUndoEnabled());

    if (bAddUndo && bUndo)
    {
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoRemoveLink( pImpl->m_pDocSh, aFileName ) );

        bAddUndo = FALSE;   // nur einmal
    }

    // Verbindung wird im dtor aufgehoben

    SvBaseLink::Closed();
}

BOOL ScTableLink::IsUsed() const
{
    return pImpl->m_pDocSh->GetDocument()->HasLink( aFileName, aFilterName, aOptions );
}

BOOL ScTableLink::Refresh(const String& rNewFile, const String& rNewFilter,
                            const String* pNewOptions, ULONG nNewRefresh )
{
    //  Dokument laden

    if (!rNewFile.Len() || !rNewFilter.Len())
        return FALSE;

    String aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pImpl->m_pDocSh ) );
    BOOL bNewUrlName = (aNewUrl != aFileName);

    const SfxFilter* pFilter = pImpl->m_pDocSh->GetFactory().GetFilterContainer()->GetFilter4FilterName(rNewFilter);
    if (!pFilter)
        return FALSE;

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
    pDoc->SetInLinkUpdate( TRUE );

    BOOL bUndo(pDoc->IsUndoEnabled());

    //  wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
    if ( rNewFilter != aFilterName )
        aOptions.Erase();
    if ( pNewOptions )                  // Optionen hart angegeben?
        aOptions = *pNewOptions;

    //  ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
    SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    if ( aOptions.Len() )
        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );

    SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, FALSE, pFilter, pSet);

    if ( bInEdit )                              // only if using the edit dialog,
        pMed->UseInteractionHandler( TRUE );    // enable the filter options dialog

    ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
    SfxObjectShellRef aRef = pSrcShell;
    pSrcShell->DoLoad(pMed);

    // Optionen koennten gesetzt worden sein
    String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
    if (!aNewOpt.Len())
        aNewOpt = aOptions;

    //  Undo...

    ScDocument* pUndoDoc = NULL;
    BOOL bFirst = TRUE;
    if (bAddUndo && bUndo)
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );

    //  Tabellen kopieren

    ScDocShellModificator aModificator( *pImpl->m_pDocSh );

    BOOL bNotFound = FALSE;
    ScDocument* pSrcDoc = pSrcShell->GetDocument();

    //  from text filters that don't set the table name,
    //  use the one table regardless of link table name
    BOOL bAutoTab = (pSrcDoc->GetTableCount() == 1) &&
                    ScDocShell::HasAutomaticTableName( rNewFilter );

    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
    {
        BYTE nMode = pDoc->GetLinkMode(nTab);
        if (nMode && pDoc->GetLinkDoc(nTab)==aFileName)
        {
            String aTabName = pDoc->GetLinkTab(nTab);

            //  Undo

            if (bAddUndo && bUndo)
            {
                if (bFirst)
                    pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
                else
                    pUndoDoc->AddUndoTab( nTab, nTab, TRUE, TRUE );
                bFirst = FALSE;
                ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
                pDoc->CopyToDocument(aRange, IDF_ALL, FALSE, pUndoDoc);
                pUndoDoc->TransferDrawPage( pDoc, nTab, nTab );
                pUndoDoc->SetLink( nTab, nMode, aFileName, aFilterName,
                    aOptions, aTabName, GetRefreshDelay() );
            }

            //  Tabellenname einer ExtDocRef anpassen

            if ( bNewUrlName && nMode == SC_LINK_VALUE )
            {
                String aName;
                pDoc->GetName( nTab, aName );
                if ( ScGlobal::GetpTransliteration()->isEqual(
                        ScGlobal::GetDocTabName( aFileName, aTabName ), aName ) )
                {
                    pDoc->RenameTab( nTab,
                        ScGlobal::GetDocTabName( aNewUrl, aTabName ),
                        FALSE, TRUE );  // kein RefUpdate, kein ValidTabName
                }
            }

            //  kopieren

            SCTAB nSrcTab = 0;
            bool bFound = false;
            /*  #i71497# check if external document is loaded successfully,
                otherwise we may find the empty default sheet "Sheet1" in
                pSrcDoc, even if the document does not exist. */
            if( pMed->GetError() == 0 )
            {
                // no sheet name -> use first sheet
                if ( aTabName.Len() && !bAutoTab )
                    bFound = pSrcDoc->GetTable( aTabName, nSrcTab );
                else
                    bFound = true;
            }

            if (bFound)
                pDoc->TransferTab( pSrcDoc, nSrcTab, nTab, FALSE,       // nicht neu einfuegen
                                        (nMode == SC_LINK_VALUE) );     // nur Werte?
            else
            {
                pDoc->DeleteAreaTab( 0,0,MAXCOL,MAXROW, nTab, IDF_ALL );

                bool bShowError = true;
                if ( nMode == SC_LINK_VALUE )
                {
                    //  Value link (used with external references in formulas):
                    //  Look for formulas that reference the sheet, and put errors in the referenced cells.

                    ScRangeList aErrorCells;        // cells on the linked sheets that need error values

                    ScCellIterator aCellIter( pDoc, 0,0,0, MAXCOL,MAXROW,MAXTAB );          // all sheets
                    ScBaseCell* pCell = aCellIter.GetFirst();
                    while (pCell)
                    {
                        if (pCell->GetCellType() == CELLTYPE_FORMULA)
                        {
                            ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);

                            ScDetectiveRefIter aRefIter( pFCell );
                            ScRange aRefRange;
                            while ( aRefIter.GetNextRef( aRefRange ) )
                            {
                                if ( aRefRange.aStart.Tab() <= nTab && aRefRange.aEnd.Tab() >= nTab )
                                {
                                    // use first cell of range references (don't fill potentially large ranges)

                                    aErrorCells.Join( ScRange( aRefRange.aStart ) );
                                }
                            }
                        }
                        pCell = aCellIter.GetNext();
                    }

                    size_t nRanges = aErrorCells.size();
                    if ( nRanges )                          // found any?
                    {
                        ScTokenArray aTokenArr;
                        aTokenArr.AddOpCode( ocNotAvail );
                        aTokenArr.AddOpCode( ocOpen );
                        aTokenArr.AddOpCode( ocClose );
                        aTokenArr.AddOpCode( ocStop );

                        for (size_t nPos=0; nPos < nRanges; nPos++)
                        {
                            const ScRange* pRange = aErrorCells[ nPos ];
                            SCCOL nStartCol = pRange->aStart.Col();
                            SCROW nStartRow = pRange->aStart.Row();
                            SCCOL nEndCol = pRange->aEnd.Col();
                            SCROW nEndRow = pRange->aEnd.Row();
                            for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
                                for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
                                {
                                    ScAddress aDestPos( nCol, nRow, nTab );
                                    ScFormulaCell* pNewCell = new ScFormulaCell( pDoc, aDestPos, &aTokenArr );
                                    pDoc->PutCell( aDestPos, pNewCell );
                                }
                        }

                        bShowError = false;
                    }
                    // if no references were found, insert error message (don't leave the sheet empty)
                }

                if ( bShowError )
                {
                    //  Normal link or no references: put error message on sheet.

                    pDoc->SetString( 0,0,nTab, ScGlobal::GetRscString(STR_LINKERROR) );
                    pDoc->SetString( 0,1,nTab, ScGlobal::GetRscString(STR_LINKERRORFILE) );
                    pDoc->SetString( 1,1,nTab, aNewUrl );
                    pDoc->SetString( 0,2,nTab, ScGlobal::GetRscString(STR_LINKERRORTAB) );
                    pDoc->SetString( 1,2,nTab, aTabName );
                }

                bNotFound = TRUE;
            }

            if ( bNewUrlName || rNewFilter != aFilterName ||
                    aNewOpt != aOptions || pNewOptions ||
                    nNewRefresh != GetRefreshDelay() )
                pDoc->SetLink( nTab, nMode, aNewUrl, rNewFilter, aNewOpt,
                    aTabName, nNewRefresh );
        }
    }

    //  neue Einstellungen merken

    if ( bNewUrlName )
        aFileName = aNewUrl;
    if ( rNewFilter != aFilterName )
        aFilterName = rNewFilter;
    if ( aNewOpt != aOptions )
        aOptions = aNewOpt;

    //  aufraeumen

    aRef->DoClose();

    //  Undo

    if (bAddUndo && bUndo)
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoRefreshLink( pImpl->m_pDocSh, pUndoDoc ) );

    //  Paint (koennen mehrere Tabellen sein)

    if (bDoPaint)
    {
        pImpl->m_pDocSh->PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
                                PAINT_GRID | PAINT_TOP | PAINT_LEFT );
        aModificator.SetDocumentModified();
    }

    if (bNotFound)
    {
        //! Fehler ausgeben ?
    }

    pDoc->SetInLinkUpdate( FALSE );

    //  notify Uno objects (for XRefreshListener)
    //! also notify Uno objects if file name was changed!
    ScLinkRefreshedHint aHint;
    aHint.SetSheetLink( aFileName );
    pDoc->BroadcastUno( aHint );

    return TRUE;
}

IMPL_LINK( ScTableLink, RefreshHdl, ScTableLink*, EMPTYARG )
{
    long nRes = Refresh( aFileName, aFilterName, NULL, GetRefreshDelay() ) != 0;
    return nRes;
}

IMPL_LINK( ScTableLink, TableEndEditHdl, ::sfx2::SvBaseLink*, pLink )
{
    if ( pImpl->m_aEndEditLink.IsSet() )
        pImpl->m_aEndEditLink.Call( pLink );
    bInEdit = FALSE;
    Application::SetDefDialogParent( pImpl->m_pOldParent );
    return 0;
}

// === ScDocumentLoader ==================================================

String ScDocumentLoader::GetOptions( SfxMedium& rMedium )
{
    SfxItemSet* pSet = rMedium.GetItemSet();
    const SfxPoolItem* pItem;
    if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
        return ((const SfxStringItem*)pItem)->GetValue();

    return EMPTY_STRING;
}

BOOL ScDocumentLoader::GetFilterName( const String& rFileName,
                                    String& rFilter, String& rOptions,
                                    BOOL bWithContent, BOOL bWithInteraction )
{
    TypeId aScType = TYPE(ScDocShell);
    SfxObjectShell* pDocSh = SfxObjectShell::GetFirst( &aScType );
    while ( pDocSh )
    {
        if ( pDocSh->HasName() )
        {
            SfxMedium* pMed = pDocSh->GetMedium();
            if ( rFileName == pMed->GetName() )
            {
                rFilter = pMed->GetFilter()->GetFilterName();
                rOptions = GetOptions(*pMed);
                return TRUE;
            }
        }
        pDocSh = SfxObjectShell::GetNext( *pDocSh, &aScType );
    }

    INetURLObject aUrl( rFileName );
    INetProtocol eProt = aUrl.GetProtocol();
    if ( eProt == INET_PROT_NOT_VALID )         // invalid URL?
        return FALSE;                           // abort without creating a medium

    //  Filter-Detection

    const SfxFilter* pSfxFilter = NULL;
    SfxMedium* pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE );
    if ( pMedium->GetError() == ERRCODE_NONE )
    {
        if ( bWithInteraction )
            pMedium->UseInteractionHandler(TRUE);   // #i73992# no longer called from GuessFilter

        SfxFilterMatcher aMatcher( String::CreateFromAscii("scalc") );
        if( bWithContent )
            aMatcher.GuessFilter( *pMedium, &pSfxFilter );
        else
            aMatcher.GuessFilterIgnoringContent( *pMedium, &pSfxFilter );
    }

    BOOL bOK = FALSE;
    if ( pMedium->GetError() == ERRCODE_NONE )
    {
        if ( pSfxFilter )
            rFilter = pSfxFilter->GetFilterName();
        else
            rFilter = ScDocShell::GetOwnFilterName();       //  sonst Calc-Datei
        bOK = (rFilter.Len()>0);
    }

    delete pMedium;
    return bOK;
}

void ScDocumentLoader::RemoveAppPrefix( String& rFilterName )
{
    String aAppPrefix = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( STRING_SCAPP ));
    aAppPrefix.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
    xub_StrLen nPreLen = aAppPrefix.Len();
    if ( rFilterName.Copy(0,nPreLen) == aAppPrefix )
        rFilterName.Erase(0,nPreLen);
}

ScDocumentLoader::ScDocumentLoader( const String& rFileName,
                                    String& rFilterName, String& rOptions,
                                    UINT32 nRekCnt, BOOL bWithInteraction ) :
        pDocShell(0),
        pMedium(0)
{
    if ( !rFilterName.Len() )
        GetFilterName( rFileName, rFilterName, rOptions, TRUE, bWithInteraction );

    const SfxFilter* pFilter = ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName( rFilterName );

    //  ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
    SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    if ( rOptions.Len() )
        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, rOptions ) );

    pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE, pFilter, pSet );
    if ( pMedium->GetError() != ERRCODE_NONE )
        return ;

    if ( bWithInteraction )
        pMedium->UseInteractionHandler( TRUE ); // to enable the filter options dialog

    pDocShell = new ScDocShell( SFX_CREATE_MODE_INTERNAL );
    aRef = pDocShell;

    ScDocument* pDoc = pDocShell->GetDocument();
    if( pDoc )
    {
        ScExtDocOptions*    pExtDocOpt = pDoc->GetExtDocOptions();
        if( !pExtDocOpt )
        {
            pExtDocOpt = new ScExtDocOptions;
            pDoc->SetExtDocOptions( pExtDocOpt );
        }
        pExtDocOpt->GetDocSettings().mnLinkCnt = nRekCnt;
    }

    pDocShell->DoLoad( pMedium );

    String aNew = GetOptions(*pMedium);         // Optionen werden beim Laden per Dialog gesetzt
    if (aNew.Len() && aNew != rOptions)
        rOptions = aNew;
}

ScDocumentLoader::~ScDocumentLoader()
{
    if ( aRef.Is() )
        aRef->DoClose();
    else if ( pMedium )
        delete pMedium;
}

void ScDocumentLoader::ReleaseDocRef()
{
    if ( aRef.Is() )
    {
        //  release reference without calling DoClose - caller must
        //  have another reference to the doc and call DoClose later

        pDocShell = NULL;
        pMedium = NULL;
        aRef.Clear();
    }
}

ScDocument* ScDocumentLoader::GetDocument()
{
    return pDocShell ? pDocShell->GetDocument() : 0;
}

BOOL ScDocumentLoader::IsError() const
{
    if ( pDocShell && pMedium )
        return pMedium->GetError() != ERRCODE_NONE;
    else
        return TRUE;
}

String ScDocumentLoader::GetTitle() const
{
    if ( pDocShell )
        return pDocShell->GetTitle();
    else
        return EMPTY_STRING;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
