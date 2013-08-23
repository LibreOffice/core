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
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>

#include "arealink.hxx"

#include "tablink.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "undoblk.hxx"
#include "globstr.hrc"
#include "markdata.hxx"
#include "hints.hxx"
#include "filter.hxx"

#include "attrib.hxx"           // raus, wenn ResetAttrib am Dokument
#include "patattr.hxx"          // raus, wenn ResetAttrib am Dokument
#include "docpool.hxx"          // raus, wenn ResetAttrib am Dokument

#include "sc.hrc"
#include "scabstdlg.hxx"
#include "clipparam.hxx"

struct AreaLink_Impl
{
    ScDocShell* m_pDocSh;
    AbstractScLinkedAreaDlg* m_pDialog;

    AreaLink_Impl() : m_pDocSh( NULL ), m_pDialog( NULL ) {}
};

TYPEINIT1(ScAreaLink,::sfx2::SvBaseLink);

//------------------------------------------------------------------------

ScAreaLink::ScAreaLink( SfxObjectShell* pShell, const String& rFile,
                        const String& rFilter, const String& rOpt,
                        const String& rArea, const ScRange& rDest,
                        sal_uLong nRefresh ) :
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ONCALL,FORMAT_FILE),
    ScRefreshTimer  ( nRefresh ),
    pImpl           ( new AreaLink_Impl() ),
    aFileName       (rFile),
    aFilterName     (rFilter),
    aOptions        (rOpt),
    aSourceArea     (rArea),
    aDestArea       (rDest),
    bAddUndo        (true),
    bInCreate       (false),
    bDoInsert       (true)
{
    OSL_ENSURE(pShell->ISA(ScDocShell), "ScAreaLink mit falscher ObjectShell");
    pImpl->m_pDocSh = static_cast< ScDocShell* >( pShell );
    SetRefreshHandler( LINK( this, ScAreaLink, RefreshHdl ) );
    SetRefreshControl( pImpl->m_pDocSh->GetDocument()->GetRefreshTimerControlAddress() );
}

ScAreaLink::~ScAreaLink()
{
    StopRefreshTimer();
    delete pImpl;
}

void ScAreaLink::Edit(Window* pParent, const Link& /* rEndEditHdl */ )
{
    //  use own dialog instead of SvBaseLink::Edit...
    //  DefModalDialogParent setzen, weil evtl. aus der DocShell beim ConvertFrom
    //  ein Optionen-Dialog kommt...

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    AbstractScLinkedAreaDlg* pDlg = pFact->CreateScLinkedAreaDlg(pParent);
    OSL_ENSURE(pDlg, "Dialog create fail!");
    pDlg->InitFromOldLink( aFileName, aFilterName, aOptions, aSourceArea, GetRefreshDelay() );
    pImpl->m_pDialog = pDlg;
    pDlg->StartExecuteModal( LINK( this, ScAreaLink, AreaEndEditHdl ) );
}

::sfx2::SvBaseLink::UpdateResult ScAreaLink::DataChanged(
    const OUString&, const ::com::sun::star::uno::Any& )
{
    //  bei bInCreate nichts tun, damit Update gerufen werden kann, um den Status im
    //  LinkManager zu setzen, ohne die Daten im Dokument zu aendern

    if (bInCreate)
        return SUCCESS;

    sfx2::LinkManager* pLinkManager=pImpl->m_pDocSh->GetDocument()->GetLinkManager();
    if (pLinkManager!=NULL)
    {
        OUString aFile, aArea, aFilter;
        pLinkManager->GetDisplayNames(this, NULL, &aFile, &aArea, &aFilter);

        //  the file dialog returns the filter name with the application prefix
        //  -> remove prefix
        ScDocumentLoader::RemoveAppPrefix( aFilter );

        // dialog doesn't set area, so keep old one
        if (aArea.isEmpty())
        {
            aArea = aSourceArea;

            // adjust in dialog:
            String aNewLinkName;
            String aTmp = aFilter;
            sfx2::MakeLnkName(aNewLinkName, NULL, aFile, aArea, &aTmp);
            aFilter = aTmp;
            SetName( aNewLinkName );
        }

        sfx2::SvBaseLinkRef const xThis(this); // keep yourself alive
        Refresh( aFile, aFilter, aArea, GetRefreshDelay() );
    }

    return SUCCESS;
}

void ScAreaLink::Closed()
{
    // Verknuepfung loeschen: Undo

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
    sal_Bool bUndo (pDoc->IsUndoEnabled());
    if (bAddUndo && bUndo)
    {
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction( new ScUndoRemoveAreaLink( pImpl->m_pDocSh,
                                                        aFileName, aFilterName, aOptions,
                                                        aSourceArea, aDestArea, GetRefreshDelay() ) );

        bAddUndo = false;   // nur einmal
    }

    SCTAB nDestTab = aDestArea.aStart.Tab();
    if (pDoc->IsStreamValid(nDestTab))
        pDoc->SetStreamValid(nDestTab, false);

    SvBaseLink::Closed();
}

void ScAreaLink::SetDestArea(const ScRange& rNew)
{
    aDestArea = rNew;           // fuer Undo
}

void ScAreaLink::SetSource(const String& rDoc, const String& rFlt, const String& rOpt,
                                const String& rArea)
{
    aFileName   = rDoc;
    aFilterName = rFlt;
    aOptions    = rOpt;
    aSourceArea = rArea;

    //  also update link name for dialog
    String aNewLinkName;
    sfx2::MakeLnkName( aNewLinkName, NULL, aFileName, aSourceArea, &aFilterName );
    SetName( aNewLinkName );
}

bool ScAreaLink::IsEqual( const String& rFile, const String& rFilter, const String& rOpt,
                            const String& rSource, const ScRange& rDest ) const
{
    return aFileName == rFile && aFilterName == rFilter && aOptions == rOpt &&
            aSourceArea == rSource && aDestArea.aStart == rDest.aStart;
}

// find a range with name >rAreaName< in >pSrcDoc<, return it in >rRange<
bool ScAreaLink::FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const OUString& rAreaName )
{
    bool bFound = false;
    OUString aUpperName = ScGlobal::pCharClass->uppercase(rAreaName);
    ScRangeName* pNames = pSrcDoc->GetRangeName();
    if (pNames)         // benannte Bereiche
    {
        const ScRangeData* p = pNames->findByUpperName(aUpperName);
        if (p && p->IsValidReference(rRange))
            bFound = true;
    }
    if (!bFound)        // Datenbankbereiche
    {
        ScDBCollection* pDBColl = pSrcDoc->GetDBCollection();
        if (pDBColl)
        {
            const ScDBData* pDB = pDBColl->getNamedDBs().findByUpperName(aUpperName);
            if (pDB)
            {
                SCTAB nTab;
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                pDB->GetArea(nTab,nCol1,nRow1,nCol2,nRow2);
                rRange = ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                bFound = true;
            }
        }
    }
    if (!bFound)        // direct reference (range or cell)
    {
        ScAddress::Details aDetails(pSrcDoc->GetAddressConvention(), 0, 0);
        if ( rRange.ParseAny( rAreaName, pSrcDoc, aDetails ) & SCA_VALID )
            bFound = true;
    }
    return bFound;
}

//  ausfuehren:

sal_Bool ScAreaLink::Refresh( const String& rNewFile, const String& rNewFilter,
                            const String& rNewArea, sal_uLong nNewRefresh )
{
    //  Dokument laden - wie TabLink

    if (!rNewFile.Len() || !rNewFilter.Len())
        return false;

    String aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pImpl->m_pDocSh ) );
    sal_Bool bNewUrlName = (aNewUrl != aFileName);

    const SfxFilter* pFilter = pImpl->m_pDocSh->GetFactory().GetFilterContainer()->GetFilter4FilterName(rNewFilter);
    if (!pFilter)
        return false;

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();

    sal_Bool bUndo (pDoc->IsUndoEnabled());
    pDoc->SetInLinkUpdate( sal_True );

    //  wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
    if ( rNewFilter != aFilterName )
        aOptions.Erase();

    SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, pFilter);

    // aRef->DoClose() will be closed explicitly, but it is still more safe to use SfxObjectShellLock here
    ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
    SfxObjectShellLock aRef = pSrcShell;
    pSrcShell->DoLoad(pMed);

    ScDocument* pSrcDoc = pSrcShell->GetDocument();

    // Optionen koennten gesetzt worden sein
    String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
    if (!aNewOpt.Len())
        aNewOpt = aOptions;

    // correct source range name list for web query import
    String aTempArea;

    if( rNewFilter == ScDocShell::GetWebQueryFilterName() )
        aTempArea = ScFormatFilter::Get().GetHTMLRangeNameList( pSrcDoc, rNewArea );
    else
        aTempArea = rNewArea;

    // find total size of source area
    SCCOL nWidth = 0;
    SCROW nHeight = 0;
    xub_StrLen nTokenCnt = comphelper::string::getTokenCount(aTempArea, ';');
    sal_Int32 nStringIx = 0;
    xub_StrLen nToken;

    for( nToken = 0; nToken < nTokenCnt; nToken++ )
    {
        String aToken( aTempArea.GetToken( 0, ';', nStringIx ) );
        ScRange aTokenRange;
        if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
        {
            // columns: find maximum
            nWidth = std::max( nWidth, (SCCOL)(aTokenRange.aEnd.Col() - aTokenRange.aStart.Col() + 1) );
            // rows: add row range + 1 empty row
            nHeight += aTokenRange.aEnd.Row() - aTokenRange.aStart.Row() + 2;
        }
    }
    // remove the last empty row
    if( nHeight > 0 )
        nHeight--;

    //  alte Daten loeschen / neue kopieren

    ScAddress aDestPos = aDestArea.aStart;
    SCTAB nDestTab = aDestPos.Tab();
    ScRange aOldRange = aDestArea;
    ScRange aNewRange = aDestArea;          // alter Bereich, wenn Datei nicht gefunden o.ae.
    if (nWidth > 0 && nHeight > 0)
    {
        aNewRange.aEnd.SetCol( aNewRange.aStart.Col() + nWidth - 1 );
        aNewRange.aEnd.SetRow( aNewRange.aStart.Row() + nHeight - 1 );
    }

    //! check CanFitBlock only if bDoInsert is set?
    sal_Bool bCanDo = ValidColRow( aNewRange.aEnd.Col(), aNewRange.aEnd.Row() ) &&
                  pDoc->CanFitBlock( aOldRange, aNewRange );
    if (bCanDo)
    {
        ScDocShellModificator aModificator( *pImpl->m_pDocSh );

        SCCOL nOldEndX = aOldRange.aEnd.Col();
        SCROW nOldEndY = aOldRange.aEnd.Row();
        SCCOL nNewEndX = aNewRange.aEnd.Col();
        SCROW nNewEndY = aNewRange.aEnd.Row();
        ScRange aMaxRange( aDestPos,
                    ScAddress(std::max(nOldEndX,nNewEndX), std::max(nOldEndY,nNewEndY), nDestTab) );

        //  Undo initialisieren

        ScDocument* pUndoDoc = NULL;
        if ( bAddUndo && bUndo )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            if ( bDoInsert )
            {
                if ( nNewEndX != nOldEndX || nNewEndY != nOldEndY )             // Bereich veraendert?
                {
                    pUndoDoc->InitUndo( pDoc, 0, pDoc->GetTableCount()-1 );
                    pDoc->CopyToDocument( 0,0,0,MAXCOL,MAXROW,MAXTAB,
                                            IDF_FORMULA, false, pUndoDoc );     // alle Formeln
                }
                else
                    pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );             // nur Zieltabelle
                pDoc->CopyToDocument( aOldRange, IDF_ALL & ~IDF_NOTE, false, pUndoDoc );
            }
            else        // ohne Einfuegen
            {
                pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );             // nur Zieltabelle
                pDoc->CopyToDocument( aMaxRange, IDF_ALL & ~IDF_NOTE, false, pUndoDoc );
            }
        }

        //  Zellen einfuegen / loeschen
        //  DeleteAreaTab loescht auch MERGE_FLAG Attribute

        if (bDoInsert)
            pDoc->FitBlock( aOldRange, aNewRange );         // incl. loeschen
        else
            pDoc->DeleteAreaTab( aMaxRange, IDF_ALL & ~IDF_NOTE );

        //  Daten kopieren

        if (nWidth > 0 && nHeight > 0)
        {
            ScDocument aClipDoc( SCDOCMODE_CLIP );
            ScRange aNewTokenRange( aNewRange.aStart );
            nStringIx = 0;
            for( nToken = 0; nToken < nTokenCnt; nToken++ )
            {
                String aToken( aTempArea.GetToken( 0, ';', nStringIx ) );
                ScRange aTokenRange;
                if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
                {
                    SCTAB nSrcTab = aTokenRange.aStart.Tab();
                    ScMarkData aSourceMark;
                    aSourceMark.SelectOneTable( nSrcTab );      // selektieren fuer CopyToClip
                    aSourceMark.SetMarkArea( aTokenRange );

                    ScClipParam aClipParam(aTokenRange, false);
                    pSrcDoc->CopyToClip(aClipParam, &aClipDoc, &aSourceMark);

                    if ( aClipDoc.HasAttrib( 0,0,nSrcTab, MAXCOL,MAXROW,nSrcTab,
                                            HASATTR_MERGED | HASATTR_OVERLAPPED ) )
                    {
                        //! ResetAttrib am Dokument !!!

                        ScPatternAttr aPattern( pSrcDoc->GetPool() );
                        aPattern.GetItemSet().Put( ScMergeAttr() );             // Defaults
                        aPattern.GetItemSet().Put( ScMergeFlagAttr() );
                        aClipDoc.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nSrcTab, aPattern );
                    }

                    aNewTokenRange.aEnd.SetCol( aNewTokenRange.aStart.Col() + (aTokenRange.aEnd.Col() - aTokenRange.aStart.Col()) );
                    aNewTokenRange.aEnd.SetRow( aNewTokenRange.aStart.Row() + (aTokenRange.aEnd.Row() - aTokenRange.aStart.Row()) );
                    ScMarkData aDestMark;
                    aDestMark.SelectOneTable( nDestTab );
                    aDestMark.SetMarkArea( aNewTokenRange );
                    pDoc->CopyFromClip( aNewTokenRange, aDestMark, IDF_ALL, NULL, &aClipDoc, false );
                    aNewTokenRange.aStart.SetRow( aNewTokenRange.aEnd.Row() + 2 );
                }
            }
        }
        else
        {
            String aErr = ScGlobal::GetRscString(STR_LINKERROR);
            pDoc->SetString( aDestPos.Col(), aDestPos.Row(), aDestPos.Tab(), aErr );
        }

        //  Undo eintragen

        if ( bAddUndo && bUndo)
        {
            ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndo( pDoc, nDestTab, nDestTab );
            pDoc->CopyToDocument( aNewRange, IDF_ALL & ~IDF_NOTE, false, pRedoDoc );

            pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoUpdateAreaLink( pImpl->m_pDocSh,
                                            aFileName, aFilterName, aOptions,
                                            aSourceArea, aOldRange, GetRefreshDelay(),
                                            aNewUrl, rNewFilter, aNewOpt,
                                            rNewArea, aNewRange, nNewRefresh,
                                            pUndoDoc, pRedoDoc, bDoInsert ) );
        }

        //  neue Einstellungen merken

        if ( bNewUrlName )
            aFileName = aNewUrl;
        if ( rNewFilter != aFilterName )
            aFilterName = rNewFilter;
        if ( rNewArea != aSourceArea )
            aSourceArea = rNewArea;
        if ( aNewOpt != aOptions )
            aOptions = aNewOpt;

        if ( aNewRange != aDestArea )
            aDestArea = aNewRange;

        if ( nNewRefresh != GetRefreshDelay() )
            SetRefreshDelay( nNewRefresh );

        SCCOL nPaintEndX = std::max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
        SCROW nPaintEndY = std::max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );

        if ( aOldRange.aEnd.Col() != aNewRange.aEnd.Col() )
            nPaintEndX = MAXCOL;
        if ( aOldRange.aEnd.Row() != aNewRange.aEnd.Row() )
            nPaintEndY = MAXROW;

        if ( !pImpl->m_pDocSh->AdjustRowHeight( aDestPos.Row(), nPaintEndY, nDestTab ) )
            pImpl->m_pDocSh->PostPaint(
                ScRange(aDestPos.Col(), aDestPos.Row(), nDestTab, nPaintEndX, nPaintEndY, nDestTab),
                PAINT_GRID);
        aModificator.SetDocumentModified();
    }
    else
    {
        //  CanFitBlock sal_False -> Probleme mit zusammengefassten Zellen
        //                       oder Tabellengrenze erreicht!
        //! Zellschutz ???

        //! Link-Dialog muss Default-Parent setzen
        //  "kann keine Zeilen einfuegen"
        InfoBox aBox( Application::GetDefDialogParent(),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_2 ) );
        aBox.Execute();
    }

    //  aufraeumen

    aRef->DoClose();

    pDoc->SetInLinkUpdate( false );

    if (bCanDo)
    {
        //  notify Uno objects (for XRefreshListener)
        //! also notify Uno objects if file name was changed!
        ScLinkRefreshedHint aHint;
        aHint.SetAreaLink( aDestPos );
        pDoc->BroadcastUno( aHint );
    }

    return bCanDo;
}


IMPL_LINK_NOARG(ScAreaLink, RefreshHdl)
{
    long nRes = Refresh( aFileName, aFilterName, aSourceArea,
        GetRefreshDelay() ) != 0;
    return nRes;
}

IMPL_LINK_NOARG(ScAreaLink, AreaEndEditHdl)
{
    //  #i76514# can't use link argument to access the dialog,
    //  because it's the ScLinkedAreaDlg, not AbstractScLinkedAreaDlg

    if ( pImpl->m_pDialog && pImpl->m_pDialog->GetResult() == RET_OK )
    {
        aOptions = pImpl->m_pDialog->GetOptions();
        Refresh( pImpl->m_pDialog->GetURL(), pImpl->m_pDialog->GetFilter(),
                 pImpl->m_pDialog->GetSource(), pImpl->m_pDialog->GetRefresh() );

        //  copy source data from members (set in Refresh) into link name for dialog
        String aNewLinkName;
        sfx2::MakeLnkName( aNewLinkName, NULL, aFileName, aSourceArea, &aFilterName );
        SetName( aNewLinkName );
    }
    pImpl->m_pDialog = NULL;    // dialog is deleted with parent

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
