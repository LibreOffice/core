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

#include <sal/config.h>

#include <com/sun/star/task/InteractionHandler.hpp>

#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>

#include <tablink.hxx>

#include <scextopt.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <undoblk.hxx>
#include <undotab.hxx>
#include <global.hxx>
#include <hints.hxx>
#include <dociter.hxx>
#include <formula/opcode.hxx>
#include <formulaiter.hxx>
#include <tokenarray.hxx>

struct TableLink_Impl
{
    ScDocShell*            m_pDocSh;
    Link<sfx2::SvBaseLink&,void> m_aEndEditLink;

    TableLink_Impl() : m_pDocSh( nullptr ) {}
};


ScTableLink::ScTableLink(ScDocShell* pDocSh, const OUString& rFile,
                            const OUString& rFilter, const OUString& rOpt,
                            sal_uLong nRefresh ):
    ::sfx2::SvBaseLink(SfxLinkUpdateMode::ONCALL,SotClipboardFormatId::SIMPLE_FILE),
    ScRefreshTimer( nRefresh ),
    pImpl( new TableLink_Impl ),
    aFileName(rFile),
    aFilterName(rFilter),
    aOptions(rOpt),
    bInCreate( false ),
    bInEdit( false ),
    bAddUndo( true )
{
    pImpl->m_pDocSh = pDocSh;
}

ScTableLink::ScTableLink(SfxObjectShell* pShell, const OUString& rFile,
                            const OUString& rFilter, const OUString& rOpt,
                            sal_uLong nRefresh ):
    ::sfx2::SvBaseLink(SfxLinkUpdateMode::ONCALL,SotClipboardFormatId::SIMPLE_FILE),
    ScRefreshTimer( nRefresh ),
    pImpl( new TableLink_Impl ),
    aFileName(rFile),
    aFilterName(rFilter),
    aOptions(rOpt),
    bInCreate( false ),
    bInEdit( false ),
    bAddUndo( true )
{
    pImpl->m_pDocSh = static_cast< ScDocShell* >( pShell );
    SetRefreshHandler( LINK( this, ScTableLink, RefreshHdl ) );
    SetRefreshControl( &pImpl->m_pDocSh->GetDocument().GetRefreshTimerControlAddress() );
}

ScTableLink::~ScTableLink()
{
    // cancel connection

    StopRefreshTimer();
    ScDocument& rDoc = pImpl->m_pDocSh->GetDocument();
    SCTAB nCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (rDoc.IsLinked(nTab) && aFileName == rDoc.GetLinkDoc(nTab))
            rDoc.SetLink( nTab, ScLinkMode::NONE, "", "", "", "", 0 );
}

void ScTableLink::Edit(weld::Window* pParent, const Link<SvBaseLink&,void>& rEndEditHdl)
{
    pImpl->m_aEndEditLink = rEndEditHdl;

    bInEdit = true;
    SvBaseLink::Edit( pParent, LINK( this, ScTableLink, TableEndEditHdl ) );
}

::sfx2::SvBaseLink::UpdateResult ScTableLink::DataChanged(
    const OUString&, const css::uno::Any& )
{
    sfx2::LinkManager* pLinkManager=pImpl->m_pDocSh->GetDocument().GetLinkManager();
    if (pLinkManager!=nullptr)
    {
        OUString aFile, aFilter;
        sfx2::LinkManager::GetDisplayNames(this, nullptr, &aFile, nullptr, &aFilter);

        //  the file dialog returns the filter name with the application prefix
        //  -> remove prefix
        ScDocumentLoader::RemoveAppPrefix( aFilter );

        if (!bInCreate)
            Refresh( aFile, aFilter, nullptr, GetRefreshDelay() ); // don't load twice
    }
    return SUCCESS;
}

void ScTableLink::Closed()
{
    // delete link: Undo
    ScDocument& rDoc = pImpl->m_pDocSh->GetDocument();
    bool bUndo (rDoc.IsUndoEnabled());

    if (bAddUndo && bUndo)
    {
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoRemoveLink>( pImpl->m_pDocSh, aFileName ) );

        bAddUndo = false;   // only once
    }

    // connection gets cancelled in the dtor

    SvBaseLink::Closed();
}

bool ScTableLink::IsUsed() const
{
    return pImpl->m_pDocSh->GetDocument().HasLink( aFileName, aFilterName, aOptions );
}

bool ScTableLink::Refresh(const OUString& rNewFile, const OUString& rNewFilter,
                            const OUString* pNewOptions, sal_uLong nNewRefresh )
{
    //  load document

    if (rNewFile.isEmpty() || rNewFilter.isEmpty())
        return false;

    OUString aNewUrl = ScGlobal::GetAbsDocName(rNewFile, pImpl->m_pDocSh);
    bool bNewUrlName = aFileName != aNewUrl;

    std::shared_ptr<const SfxFilter> pFilter = pImpl->m_pDocSh->GetFactory().GetFilterContainer()->GetFilter4FilterName(rNewFilter);
    if (!pFilter)
        return false;

    ScDocument& rDoc = pImpl->m_pDocSh->GetDocument();
    rDoc.SetInLinkUpdate( true );

    bool bUndo(rDoc.IsUndoEnabled());

    //  if new filter has been selected, forget options
    if (aFilterName != rNewFilter)
        aOptions.clear();
    if ( pNewOptions )                  // options hard-specified?
        aOptions = *pNewOptions;

    //  always create ItemSet, so that DocShell can set the options
    std::unique_ptr<SfxItemSet> pSet(new SfxAllItemSet( SfxGetpApp()->GetPool() ));
    if (!aOptions.isEmpty())
        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );

    SfxMedium* pMed = new SfxMedium(aNewUrl, StreamMode::STD_READ, pFilter, std::move(pSet));

    if ( bInEdit )                              // only if using the edit dialog,
        pMed->UseInteractionHandler(true);    // enable the filter options dialog

    // aRef->DoClose() will be called explicitly, but it is still more safe to use SfxObjectShellLock here
    ScDocShell* pSrcShell = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS);
    SfxObjectShellLock aRef = pSrcShell;
    pSrcShell->DoLoad(pMed);

    // options might have been set
    OUString aNewOpt = ScDocumentLoader::GetOptions(*pMed);
    if (aNewOpt.isEmpty())
        aNewOpt = aOptions;

    //  Undo...

    ScDocumentUniquePtr pUndoDoc;
    bool bFirst = true;
    if (bAddUndo && bUndo)
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));

    //  copy tables

    ScDocShellModificator aModificator( *pImpl->m_pDocSh );

    bool bNotFound = false;
    ScDocument& rSrcDoc = pSrcShell->GetDocument();

    //  from text filters that don't set the table name,
    //  use the one table regardless of link table name
    bool bAutoTab = (rSrcDoc.GetTableCount() == 1) &&
                    ScDocShell::HasAutomaticTableName( rNewFilter );

    SCTAB nCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
    {
        ScLinkMode nMode = rDoc.GetLinkMode(nTab);
        if (nMode != ScLinkMode::NONE && aFileName == rDoc.GetLinkDoc(nTab))
        {
            OUString aTabName = rDoc.GetLinkTab(nTab);

            //  Undo

            if (bAddUndo && bUndo)
            {
                if (bFirst)
                    pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
                else
                    pUndoDoc->AddUndoTab( nTab, nTab, true, true );
                bFirst = false;
                ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
                rDoc.CopyToDocument(aRange, InsertDeleteFlags::ALL, false, *pUndoDoc);
                pUndoDoc->TransferDrawPage( &rDoc, nTab, nTab );
                pUndoDoc->SetLink( nTab, nMode, aFileName, aFilterName,
                                   aOptions, aTabName, GetRefreshDelay() );
                pUndoDoc->SetTabBgColor( nTab, rDoc.GetTabBgColor(nTab) );
            }

            //  adjust table name of an ExtDocRef

            if ( bNewUrlName && nMode == ScLinkMode::VALUE )
            {
                OUString aName;
                rDoc.GetName( nTab, aName );
                if ( ScGlobal::GetpTransliteration()->isEqual(
                        ScGlobal::GetDocTabName( aFileName, aTabName ), aName ) )
                {
                    rDoc.RenameTab( nTab,
                        ScGlobal::GetDocTabName( aNewUrl, aTabName ),
                        true/*bExternalDocument*/ );
                }
            }

            //  copy

            SCTAB nSrcTab = 0;
            bool bFound = false;
            /*  #i71497# check if external document is loaded successfully,
                otherwise we may find the empty default sheet "Sheet1" in
                rSrcDoc, even if the document does not exist. */
            if( pMed->GetError() == ERRCODE_NONE )
            {
                // no sheet name -> use first sheet
                if ( !aTabName.isEmpty() && !bAutoTab )
                    bFound = rSrcDoc.GetTable( aTabName, nSrcTab );
                else
                    bFound = true;
            }

            if (bFound)
                rDoc.TransferTab( &rSrcDoc, nSrcTab, nTab, false,       // don't insert anew
                                        (nMode == ScLinkMode::VALUE) );     // only values?
            else
            {
                rDoc.DeleteAreaTab( 0,0,MAXCOL,MAXROW, nTab, InsertDeleteFlags::ALL );

                bool bShowError = true;
                if ( nMode == ScLinkMode::VALUE )
                {
                    //  Value link (used with external references in formulas):
                    //  Look for formulas that reference the sheet, and put errors in the referenced cells.

                    ScRangeList aErrorCells;        // cells on the linked sheets that need error values

                    ScCellIterator aIter(&rDoc, ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB));          // all sheets
                    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
                    {
                        if (aIter.getType() != CELLTYPE_FORMULA)
                            continue;

                        ScFormulaCell* pCell = aIter.getFormulaCell();
                        ScDetectiveRefIter aRefIter(pCell);
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
                            const ScRange & rRange = aErrorCells[ nPos ];
                            SCCOL nStartCol = rRange.aStart.Col();
                            SCROW nStartRow = rRange.aStart.Row();
                            SCCOL nEndCol = rRange.aEnd.Col();
                            SCROW nEndRow = rRange.aEnd.Row();
                            for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
                                for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
                                {
                                    ScAddress aDestPos( nCol, nRow, nTab );
                                    rDoc.SetFormula(aDestPos, aTokenArr);
                                }
                        }

                        bShowError = false;
                    }
                    // if no references were found, insert error message (don't leave the sheet empty)
                }

                if ( bShowError )
                {
                    //  Normal link or no references: put error message on sheet.

                    rDoc.SetString( 0,0,nTab, ScResId(STR_LINKERROR) );
                    rDoc.SetString( 0,1,nTab, ScResId(STR_LINKERRORFILE) );
                    rDoc.SetString( 1,1,nTab, aNewUrl );
                    rDoc.SetString( 0,2,nTab, ScResId(STR_LINKERRORTAB) );
                    rDoc.SetString( 1,2,nTab, aTabName );
                }

                bNotFound = true;
            }

            if ( bNewUrlName || aFilterName != rNewFilter ||
                    aOptions != aNewOpt || pNewOptions ||
                    nNewRefresh != GetRefreshDelay() )
                rDoc.SetLink( nTab, nMode, aNewUrl, rNewFilter, aNewOpt,
                    aTabName, nNewRefresh );
        }
    }

    //  memorize new settings

    if ( bNewUrlName )
        aFileName = aNewUrl;
    if (aFilterName != rNewFilter)
        aFilterName = rNewFilter;
    if (aOptions != aNewOpt)
        aOptions = aNewOpt;

    //  clean up

    aRef->DoClose();

    //  Undo

    if (bAddUndo && bUndo)
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoRefreshLink>( pImpl->m_pDocSh, std::move(pUndoDoc) ) );

    //  Paint (may be several tables)

    pImpl->m_pDocSh->PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
                                PaintPartFlags::Grid | PaintPartFlags::Top | PaintPartFlags::Left | PaintPartFlags::Extras );
    aModificator.SetDocumentModified();

    if (bNotFound)
    {
        //! output error ?
    }

    rDoc.SetInLinkUpdate( false );

    //  notify Uno objects (for XRefreshListener)
    //! also notify Uno objects if file name was changed!
    ScLinkRefreshedHint aHint;
    aHint.SetSheetLink( aFileName );
    rDoc.BroadcastUno( aHint );

    return true;
}

IMPL_LINK_NOARG(ScTableLink, RefreshHdl, Timer *, void)
{
    Refresh( aFileName, aFilterName, nullptr, GetRefreshDelay() );
}

IMPL_LINK( ScTableLink, TableEndEditHdl, ::sfx2::SvBaseLink&, rLink, void )
{
    pImpl->m_aEndEditLink.Call( rLink );
    bInEdit = false;
}

// === ScDocumentLoader ==================================================

OUString ScDocumentLoader::GetOptions( const SfxMedium& rMedium )
{
    SfxItemSet* pSet = rMedium.GetItemSet();
    const SfxPoolItem* pItem;
    if ( pSet && SfxItemState::SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS, true, &pItem ) )
        return static_cast<const SfxStringItem*>(pItem)->GetValue();

    return EMPTY_OUSTRING;
}

bool ScDocumentLoader::GetFilterName( const OUString& rFileName,
                                      OUString& rFilter, OUString& rOptions,
                                      bool bWithContent, bool bWithInteraction )
{
    SfxObjectShell* pDocSh = SfxObjectShell::GetFirst( checkSfxObjectShell<ScDocShell> );
    while ( pDocSh )
    {
        if ( pDocSh->HasName() )
        {
            SfxMedium* pMed = pDocSh->GetMedium();
            if ( pMed->GetName() == rFileName )
            {
                rFilter = pMed->GetFilter()->GetFilterName();
                rOptions = GetOptions(*pMed);
                return true;
            }
        }
        pDocSh = SfxObjectShell::GetNext( *pDocSh, checkSfxObjectShell<ScDocShell> );
    }

    INetURLObject aUrl( rFileName );
    INetProtocol eProt = aUrl.GetProtocol();
    if ( eProt == INetProtocol::NotValid )         // invalid URL?
        return false;                           // abort without creating a medium

    //  Filter detection

    std::shared_ptr<const SfxFilter> pSfxFilter;
    auto pMedium = std::make_unique<SfxMedium>( rFileName, StreamMode::STD_READ );
    if (pMedium->GetError() == ERRCODE_NONE && !utl::ConfigManager::IsFuzzing())
    {
        if ( bWithInteraction )
            pMedium->UseInteractionHandler(true);   // #i73992# no longer called from GuessFilter

        SfxFilterMatcher aMatcher("scalc");
        if( bWithContent )
            aMatcher.GuessFilter( *pMedium, pSfxFilter );
        else
            aMatcher.GuessFilterIgnoringContent( *pMedium, pSfxFilter );
    }

    bool bOK = false;
    if ( pMedium->GetError() == ERRCODE_NONE )
    {
        if ( pSfxFilter )
            rFilter = pSfxFilter->GetFilterName();
        else
            rFilter = ScDocShell::GetOwnFilterName();       //  otherwise Calc file
        bOK = !rFilter.isEmpty();
    }

    return bOK;
}

void ScDocumentLoader::RemoveAppPrefix( OUString& rFilterName )
{
    OUString aAppPrefix( STRING_SCAPP ": ");
    if (rFilterName.startsWith( aAppPrefix))
        rFilterName = rFilterName.copy( aAppPrefix.getLength());
}

SfxMedium* ScDocumentLoader::CreateMedium( const OUString& rFileName, std::shared_ptr<const SfxFilter> const & pFilter,
        const OUString& rOptions, weld::Window* pInteractionParent )
{
    // Always create SfxItemSet so ScDocShell can set options.
    std::unique_ptr<SfxItemSet> pSet(new SfxAllItemSet( SfxGetpApp()->GetPool() ));
    if ( !rOptions.isEmpty() )
        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, rOptions ) );

    if (pInteractionParent)
    {
        css::uno::Reference<css::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        css::uno::Reference<css::task::XInteractionHandler> xIHdl(css::task::InteractionHandler::createWithParent(xContext,
                    pInteractionParent->GetXWindow()), css::uno::UNO_QUERY_THROW);
        pSet->Put(SfxUnoAnyItem(SID_INTERACTIONHANDLER, makeAny(xIHdl)));
    }

    SfxMedium *pRet = new SfxMedium( rFileName, StreamMode::STD_READ, pFilter, std::move(pSet) );
    if (pInteractionParent)
        pRet->UseInteractionHandler(true); // to enable the filter options dialog
    return pRet;
}

ScDocumentLoader::ScDocumentLoader(const OUString& rFileName,
                                   OUString& rFilterName, OUString& rOptions,
                                   sal_uInt32 nRekCnt, weld::Window* pInteractionParent)
    : pDocShell(nullptr)
    , pMedium(nullptr)
{
    if ( rFilterName.isEmpty() )
        GetFilterName(rFileName, rFilterName, rOptions, true, pInteractionParent != nullptr);

    std::shared_ptr<const SfxFilter> pFilter = ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName( rFilterName );

    pMedium = CreateMedium(rFileName, pFilter, rOptions, pInteractionParent);
    if ( pMedium->GetError() != ERRCODE_NONE )
        return ;

    pDocShell = new ScDocShell( SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS );
    aRef = pDocShell;

    ScDocument& rDoc = pDocShell->GetDocument();
    ScExtDocOptions*    pExtDocOpt = rDoc.GetExtDocOptions();
    if( !pExtDocOpt )
    {
        rDoc.SetExtDocOptions( std::make_unique<ScExtDocOptions>() );
        pExtDocOpt = rDoc.GetExtDocOptions();
    }
    pExtDocOpt->GetDocSettings().mnLinkCnt = nRekCnt;

    pDocShell->DoLoad( pMedium );

    OUString aNew = GetOptions(*pMedium);         // options are set per dialog on load
    if (!aNew.isEmpty() && aNew != rOptions)
        rOptions = aNew;
}

ScDocumentLoader::~ScDocumentLoader()
{
    if ( aRef.is() )
        aRef->DoClose();
    else
        delete pMedium;
}

void ScDocumentLoader::ReleaseDocRef()
{
    if ( aRef.is() )
    {
        //  release reference without calling DoClose - caller must
        //  have another reference to the doc and call DoClose later

        pDocShell = nullptr;
        pMedium = nullptr;
        aRef.clear();
    }
}

ScDocument* ScDocumentLoader::GetDocument()
{
    return pDocShell ? &pDocShell->GetDocument() : nullptr;
}

bool ScDocumentLoader::IsError() const
{
    if ( pDocShell && pMedium )
        return pMedium->GetError() != ERRCODE_NONE;
    else
        return true;
}

OUString ScDocumentLoader::GetTitle() const
{
    if ( pDocShell )
        return pDocShell->GetTitle();
    else
        return EMPTY_OUSTRING;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
