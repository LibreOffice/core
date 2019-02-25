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

#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/weld.hxx>
#include <unotools/charclass.hxx>

#include <arealink.hxx>

#include <tablink.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <undoblk.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <markdata.hxx>
#include <hints.hxx>
#include <filter.hxx>

#include <attrib.hxx>
#include <patattr.hxx>
#include <docpool.hxx>

#include <scabstdlg.hxx>
#include <clipparam.hxx>


ScAreaLink::ScAreaLink( SfxObjectShell* pShell, const OUString& rFile,
                        const OUString& rFilter, const OUString& rOpt,
                        const OUString& rArea, const ScRange& rDest,
                        sal_uLong nRefresh ) :
    ::sfx2::SvBaseLink(SfxLinkUpdateMode::ONCALL,SotClipboardFormatId::SIMPLE_FILE),
    ScRefreshTimer  ( nRefresh ),
    m_pDocSh(static_cast<ScDocShell*>(pShell)),
    aFileName       (rFile),
    aFilterName     (rFilter),
    aOptions        (rOpt),
    aSourceArea     (rArea),
    aDestArea       (rDest),
    bAddUndo        (true),
    bInCreate       (false),
    bDoInsert       (true)
{
    OSL_ENSURE(dynamic_cast< const ScDocShell *>( pShell ) !=  nullptr, "ScAreaLink with wrong ObjectShell");
    SetRefreshHandler( LINK( this, ScAreaLink, RefreshHdl ) );
    SetRefreshControl( &m_pDocSh->GetDocument().GetRefreshTimerControlAddress() );
}

ScAreaLink::~ScAreaLink()
{
    StopRefreshTimer();
}

void ScAreaLink::Edit(weld::Window* pParent, const Link<SvBaseLink&,void>& /* rEndEditHdl */ )
{
    //  use own dialog instead of SvBaseLink::Edit...
    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    ScopedVclPtr<AbstractScLinkedAreaDlg> pDlg(pFact->CreateScLinkedAreaDlg(pParent));
    pDlg->InitFromOldLink( aFileName, aFilterName, aOptions, aSourceArea, GetRefreshDelay() );
    if ( pDlg->Execute() == RET_OK )
    {
        aOptions = pDlg->GetOptions();
        Refresh( pDlg->GetURL(), pDlg->GetFilter(),
                 pDlg->GetSource(), pDlg->GetRefresh() );

        //  copy source data from members (set in Refresh) into link name for dialog
        OUString aNewLinkName;
        sfx2::MakeLnkName( aNewLinkName, nullptr, aFileName, aSourceArea, &aFilterName );
        SetName( aNewLinkName );
    }
}

::sfx2::SvBaseLink::UpdateResult ScAreaLink::DataChanged(
    const OUString&, const css::uno::Any& )
{
    //  Do not do anything at bInCreate so that update can be called to set
    //  the status in the LinkManager without changing the data in the document

    if (bInCreate)
        return SUCCESS;

    sfx2::LinkManager* pLinkManager=m_pDocSh->GetDocument().GetLinkManager();
    if (pLinkManager!=nullptr)
    {
        OUString aFile, aArea, aFilter;
        sfx2::LinkManager::GetDisplayNames(this, nullptr, &aFile, &aArea, &aFilter);

        //  the file dialog returns the filter name with the application prefix
        //  -> remove prefix
        ScDocumentLoader::RemoveAppPrefix( aFilter );

        // dialog doesn't set area, so keep old one
        if (aArea.isEmpty())
        {
            aArea = aSourceArea;

            // adjust in dialog:
            OUString aNewLinkName;
            OUString aTmp = aFilter;
            sfx2::MakeLnkName(aNewLinkName, nullptr, aFile, aArea, &aTmp);
            aFilter = aTmp;
            SetName( aNewLinkName );
        }

        tools::SvRef<sfx2::SvBaseLink> const xThis(this); // keep yourself alive
        Refresh( aFile, aFilter, aArea, GetRefreshDelay() );
    }

    return SUCCESS;
}

void ScAreaLink::Closed()
{
    // delete link: Undo

    ScDocument& rDoc = m_pDocSh->GetDocument();
    bool bUndo (rDoc.IsUndoEnabled());
    if (bAddUndo && bUndo)
    {
        m_pDocSh->GetUndoManager()->AddUndoAction( std::make_unique<ScUndoRemoveAreaLink>( m_pDocSh,
                                                        aFileName, aFilterName, aOptions,
                                                        aSourceArea, aDestArea, GetRefreshDelay() ) );

        bAddUndo = false;   // only once
    }

    SCTAB nDestTab = aDestArea.aStart.Tab();
    rDoc.SetStreamValid(nDestTab, false);

    SvBaseLink::Closed();
}

void ScAreaLink::SetDestArea(const ScRange& rNew)
{
    aDestArea = rNew;           // for Undo
}

void ScAreaLink::SetSource(const OUString& rDoc, const OUString& rFlt, const OUString& rOpt,
                                const OUString& rArea)
{
    aFileName   = rDoc;
    aFilterName = rFlt;
    aOptions    = rOpt;
    aSourceArea = rArea;

    //  also update link name for dialog
    OUString aNewLinkName;
    sfx2::MakeLnkName( aNewLinkName, nullptr, aFileName, aSourceArea, &aFilterName );
    SetName( aNewLinkName );
}

bool ScAreaLink::IsEqual( const OUString& rFile, const OUString& rFilter, const OUString& rOpt,
                            const OUString& rSource, const ScRange& rDest ) const
{
    return aFileName == rFile && aFilterName == rFilter && aOptions == rOpt &&
            aSourceArea == rSource && aDestArea.aStart == rDest.aStart;
}

// find a range with name >rAreaName< in >pSrcDoc<, return it in >rRange<
bool ScAreaLink::FindExtRange( ScRange& rRange, const ScDocument* pSrcDoc, const OUString& rAreaName )
{
    bool bFound = false;
    OUString aUpperName = ScGlobal::pCharClass->uppercase(rAreaName);
    ScRangeName* pNames = pSrcDoc->GetRangeName();
    if (pNames)         // named ranges
    {
        const ScRangeData* p = pNames->findByUpperName(aUpperName);
        if (p && p->IsValidReference(rRange))
            bFound = true;
    }
    if (!bFound)        // database ranges
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
        if ( rRange.ParseAny( rAreaName, pSrcDoc, aDetails ) & ScRefFlags::VALID )
            bFound = true;
    }
    return bFound;
}

//  execute:

bool ScAreaLink::Refresh( const OUString& rNewFile, const OUString& rNewFilter,
                            const OUString& rNewArea, sal_uLong nNewRefresh )
{
    //  load document - like TabLink

    if (rNewFile.isEmpty() || rNewFilter.isEmpty())
        return false;

    OUString aNewUrl( ScGlobal::GetAbsDocName( rNewFile, m_pDocSh ) );
    bool bNewUrlName = (aNewUrl != aFileName);

    std::shared_ptr<const SfxFilter> pFilter = m_pDocSh->GetFactory().GetFilterContainer()->GetFilter4FilterName(rNewFilter);
    if (!pFilter)
        return false;

    ScDocument& rDoc = m_pDocSh->GetDocument();

    bool bUndo (rDoc.IsUndoEnabled());
    rDoc.SetInLinkUpdate( true );

    //  if new filter was selected, forget options
    if ( rNewFilter != aFilterName )
        aOptions.clear();

    SfxMedium* pMed = ScDocumentLoader::CreateMedium( aNewUrl, pFilter, aOptions);

    // aRef->DoClose() will be closed explicitly, but it is still more safe to use SfxObjectShellLock here
    ScDocShell* pSrcShell = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS);
    SfxObjectShellLock aRef = pSrcShell;
    pSrcShell->DoLoad(pMed);

    ScDocument& rSrcDoc = pSrcShell->GetDocument();

    // options could have been set
    OUString aNewOpt = ScDocumentLoader::GetOptions(*pMed);
    if (aNewOpt.isEmpty())
        aNewOpt = aOptions;

    // correct source range name list for web query import
    OUString aTempArea;

    if( rNewFilter == ScDocShell::GetWebQueryFilterName() )
        aTempArea = ScFormatFilter::Get().GetHTMLRangeNameList( &rSrcDoc, rNewArea );
    else
        aTempArea = rNewArea;

    // find total size of source area
    SCCOL nWidth = 0;
    SCROW nHeight = 0;
    ScRangeList aSourceRanges;

    if (rNewFilter == SC_TEXT_CSV_FILTER_NAME && aTempArea == "CSV_all")
    {
        // The dummy All range. All data, including top/left empty
        // rows/columns.
        aTempArea.clear();
        SCCOL nEndCol = 0;
        SCROW nEndRow = 0;
        if (rSrcDoc.GetCellArea( 0, nEndCol, nEndRow))
        {
            aSourceRanges.push_back( ScRange( 0,0,0, nEndCol, nEndRow, 0));
            nWidth = nEndCol + 1;
            nHeight = nEndRow + 2;
        }
    }

    if (!aTempArea.isEmpty())
    {
        sal_Int32 nIdx {0};
        do
        {
            ScRange aTokenRange;
            if( FindExtRange( aTokenRange, &rSrcDoc, aTempArea.getToken( 0, ';', nIdx ) ) )
            {
                aSourceRanges.push_back( aTokenRange);
                // columns: find maximum
                nWidth = std::max( nWidth, static_cast<SCCOL>(aTokenRange.aEnd.Col() - aTokenRange.aStart.Col() + 1) );
                // rows: add row range + 1 empty row
                nHeight += aTokenRange.aEnd.Row() - aTokenRange.aStart.Row() + 2;
            }
        }
        while (nIdx>0);
    }
    // remove the last empty row
    if( nHeight > 0 )
        nHeight--;

    //  delete old data / copy new

    ScAddress aDestPos = aDestArea.aStart;
    SCTAB nDestTab = aDestPos.Tab();
    ScRange aOldRange = aDestArea;
    ScRange aNewRange = aDestArea;          // old range, if file not found or similar
    if (nWidth > 0 && nHeight > 0)
    {
        aNewRange.aEnd.SetCol( aNewRange.aStart.Col() + nWidth - 1 );
        aNewRange.aEnd.SetRow( aNewRange.aStart.Row() + nHeight - 1 );
    }

    //! check CanFitBlock only if bDoInsert is set?
    bool bCanDo = ValidColRow( aNewRange.aEnd.Col(), aNewRange.aEnd.Row() ) &&
                  rDoc.CanFitBlock( aOldRange, aNewRange );
    if (bCanDo)
    {
        ScDocShellModificator aModificator( *m_pDocSh );

        SCCOL nOldEndX = aOldRange.aEnd.Col();
        SCROW nOldEndY = aOldRange.aEnd.Row();
        SCCOL nNewEndX = aNewRange.aEnd.Col();
        SCROW nNewEndY = aNewRange.aEnd.Row();
        ScRange aMaxRange( aDestPos,
                    ScAddress(std::max(nOldEndX,nNewEndX), std::max(nOldEndY,nNewEndY), nDestTab) );

        //  initialise Undo

        ScDocumentUniquePtr pUndoDoc;
        if ( bAddUndo && bUndo )
        {
            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            if ( bDoInsert )
            {
                if ( nNewEndX != nOldEndX || nNewEndY != nOldEndY )             // range changed?
                {
                    pUndoDoc->InitUndo( &rDoc, 0, rDoc.GetTableCount()-1 );
                    rDoc.CopyToDocument(0, 0, 0, MAXCOL, MAXROW, MAXTAB,
                                        InsertDeleteFlags::FORMULA, false, *pUndoDoc);     // all formulas
                }
                else
                    pUndoDoc->InitUndo( &rDoc, nDestTab, nDestTab );             // only destination table
                rDoc.CopyToDocument(aOldRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, *pUndoDoc);
            }
            else        // without insertion
            {
                pUndoDoc->InitUndo( &rDoc, nDestTab, nDestTab );             // only destination table
                rDoc.CopyToDocument(aMaxRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, *pUndoDoc);
            }
        }

        //  insert / delete cells
        //  DeleteAreaTab also deletes MERGE_FLAG attributes

        if (bDoInsert)
            rDoc.FitBlock( aOldRange, aNewRange );         // incl. deletion
        else
            rDoc.DeleteAreaTab( aMaxRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE );

        //  copy data

        if (nWidth > 0 && nHeight > 0)
        {
            ScDocument aClipDoc( SCDOCMODE_CLIP );
            ScRange aNewTokenRange( aNewRange.aStart );
            for (size_t nRange = 0; nRange < aSourceRanges.size(); ++nRange)
            {
                ScRange const & rTokenRange( aSourceRanges[nRange]);
                SCTAB nSrcTab = rTokenRange.aStart.Tab();
                ScMarkData aSourceMark;
                aSourceMark.SelectOneTable( nSrcTab );      // selecting for CopyToClip
                aSourceMark.SetMarkArea( rTokenRange );

                ScClipParam aClipParam(rTokenRange, false);
                rSrcDoc.CopyToClip(aClipParam, &aClipDoc, &aSourceMark, false, false);

                if ( aClipDoc.HasAttrib( 0,0,nSrcTab, MAXCOL,MAXROW,nSrcTab,
                            HasAttrFlags::Merged | HasAttrFlags::Overlapped ) )
                {
                    //! ResetAttrib at document !!!

                    ScPatternAttr aPattern( rSrcDoc.GetPool() );
                    aPattern.GetItemSet().Put( ScMergeAttr() );             // Defaults
                    aPattern.GetItemSet().Put( ScMergeFlagAttr() );
                    aClipDoc.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nSrcTab, aPattern );
                }

                aNewTokenRange.aEnd.SetCol( aNewTokenRange.aStart.Col() + (rTokenRange.aEnd.Col() - rTokenRange.aStart.Col()) );
                aNewTokenRange.aEnd.SetRow( aNewTokenRange.aStart.Row() + (rTokenRange.aEnd.Row() - rTokenRange.aStart.Row()) );
                ScMarkData aDestMark;
                aDestMark.SelectOneTable( nDestTab );
                aDestMark.SetMarkArea( aNewTokenRange );
                rDoc.CopyFromClip( aNewTokenRange, aDestMark, InsertDeleteFlags::ALL, nullptr, &aClipDoc, false );
                aNewTokenRange.aStart.SetRow( aNewTokenRange.aEnd.Row() + 2 );
            }
        }
        else
        {
            OUString aErr = ScResId(STR_LINKERROR);
            rDoc.SetString( aDestPos.Col(), aDestPos.Row(), aDestPos.Tab(), aErr );
        }

        //  enter Undo

        if ( bAddUndo && bUndo)
        {
            ScDocumentUniquePtr pRedoDoc(new ScDocument( SCDOCMODE_UNDO ));
            pRedoDoc->InitUndo( &rDoc, nDestTab, nDestTab );
            rDoc.CopyToDocument(aNewRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false, *pRedoDoc);

            m_pDocSh->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoUpdateAreaLink>( m_pDocSh,
                                            aFileName, aFilterName, aOptions,
                                            aSourceArea, aOldRange, GetRefreshDelay(),
                                            aNewUrl, rNewFilter, aNewOpt,
                                            rNewArea, aNewRange, nNewRefresh,
                                            std::move(pUndoDoc), std::move(pRedoDoc), bDoInsert ) );
        }

        //  remember new settings

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

        if ( !m_pDocSh->AdjustRowHeight( aDestPos.Row(), nPaintEndY, nDestTab ) )
            m_pDocSh->PostPaint(
                ScRange(aDestPos.Col(), aDestPos.Row(), nDestTab, nPaintEndX, nPaintEndY, nDestTab),
                PaintPartFlags::Grid);
        aModificator.SetDocumentModified();
    }
    else
    {
        //  CanFitBlock sal_False -> Problems with summarized cells or table boundary reached!
        //! cell protection ???

        //! Link dialog must set default parent
        //  "cannot insert rows"
        vcl::Window* pWin = Application::GetDefDialogParent();
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(STR_MSSG_DOSUBTOTALS_2)));
        xInfoBox->run();
    }

    //  clean up

    aRef->DoClose();

    rDoc.SetInLinkUpdate( false );

    if (bCanDo)
    {
        //  notify Uno objects (for XRefreshListener)
        //! also notify Uno objects if file name was changed!
        ScLinkRefreshedHint aHint;
        aHint.SetAreaLink( aDestPos );
        rDoc.BroadcastUno( aHint );
    }

    return bCanDo;
}

IMPL_LINK_NOARG(ScAreaLink, RefreshHdl, Timer *, void)
{
    Refresh( aFileName, aFilterName, aSourceArea, GetRefreshDelay() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
