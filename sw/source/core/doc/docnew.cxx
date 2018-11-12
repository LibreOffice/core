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

#include <config_features.h>

#include <doc.hxx>
#include <dcontact.hxx>
#include <proofreadingiterator.hxx>
#include <com/sun/star/text/XFlatParagraphIteratorProvider.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sal/log.hxx>

#include <svl/macitem.hxx>
#include <svx/svxids.hrc>
#include <svx/svdogrp.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svl/zforlist.hxx>
#include <unotools/lingucfg.hxx>
#include <svx/svdpage.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtfordr.hxx>
#include <fmtpdsc.hxx>
#include <pvprtdat.hxx>
#include <rootfrm.hxx>
#include <layouter.hxx>
#include <pagedesc.hxx>
#include <ndtxt.hxx>
#include <printdata.hxx>
#include <docfld.hxx>
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <docstat.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <rolbck.hxx>
#include <poolfmt.hxx>
#include <dbmgr.hxx>
#include <docsh.hxx>
#include <acorrect.hxx>
#include <visiturl.hxx>
#include <docary.hxx>
#include <lineinfo.hxx>
#include <drawdoc.hxx>
#include <linkenum.hxx>
#include <fldupde.hxx>
#include <extinput.hxx>
#include <viewsh.hxx>
#include <doctxm.hxx>
#include <shellres.hxx>
#include <breakit.hxx>
#include <laycache.hxx>
#include <mvsave.hxx>
#include <istyleaccess.hxx>
#include "swstylemanager.hxx"
#include <IGrammarContact.hxx>
#include <tblafmt.hxx>
#include <tblsel.hxx>
#include <MarkManager.hxx>
#include <UndoManager.hxx>
#include <DocumentDeviceManager.hxx>
#include <DocumentSettingManager.hxx>
#include <DocumentDrawModelManager.hxx>
#include <DocumentChartDataProviderManager.hxx>
#include <DocumentTimerManager.hxx>
#include <DocumentLinksAdministrationManager.hxx>
#include <DocumentListItemsManager.hxx>
#include <DocumentListsManager.hxx>
#include <DocumentOutlineNodesManager.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <DocumentRedlineManager.hxx>
#include <DocumentFieldsManager.hxx>
#include <DocumentStatisticsManager.hxx>
#include <DocumentStateManager.hxx>
#include <DocumentLayoutManager.hxx>
#include <DocumentStylePoolManager.hxx>
#include <DocumentExternalDataManager.hxx>
#include <unochart.hxx>
#include <fldbas.hxx>
#include <wrtsh.hxx>
#include <unocrsr.hxx>
#include <fmthdft.hxx>

#include <cmdid.h>

#include <pausethreadstarting.hxx>
#include <numrule.hxx>
#include <list.hxx>

#include <sfx2/Metadatable.hxx>
#include <fmtmeta.hxx>

#include <svx/xfillit0.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;

/*
 * global functions...
 */
 uno::Reference< linguistic2::XProofreadingIterator > const & SwDoc::GetGCIterator() const
{
    if (!m_xGCIterator.is() && SvtLinguConfig().HasGrammarChecker())
    {
        uno::Reference< uno::XComponentContext >  xContext( comphelper::getProcessComponentContext() );
        try
        {
            m_xGCIterator = sw::proofreadingiterator::get( xContext );
        }
        catch (const uno::Exception &)
        {
            OSL_FAIL( "No GCIterator" );
        }
    }

    return m_xGCIterator;
}

bool SwDoc::StartGrammarChecking( bool bSkipStart )
{
    // check for a visible view
    bool bVisible = false;
    bool bStarted = false;
    const SwDocShell *pDocShell = GetDocShell();
    SfxViewFrame     *pFrame = SfxViewFrame::GetFirst( pDocShell, false );
    while (pFrame && !bVisible)
    {
        if (pFrame->IsVisible())
            bVisible = true;
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell, false );
    }

    //!! only documents with visible views need to be checked
    //!! (E.g. don't check temporary documents created for printing, see printing of notes and selections.
    //!! Those get created on the fly and get hard deleted a bit later as well, and no one should have
    //!! a uno reference to them)
    if (bVisible)
    {
        uno::Reference< linguistic2::XProofreadingIterator > xGCIterator( GetGCIterator() );
        if ( xGCIterator.is() )
        {
            uno::Reference< lang::XComponent >  xDoc( GetDocShell()->GetBaseModel(), uno::UNO_QUERY );
            uno::Reference< text::XFlatParagraphIteratorProvider >  xFPIP( xDoc, uno::UNO_QUERY );

            // start automatic background checking if not active already
            if ( xFPIP.is() && !xGCIterator->isProofreading( xDoc ) )
            {
                bStarted = true;
                if ( !bSkipStart )
                {
                    for (auto pLayout : GetAllLayouts())
                    {   // we're starting it now, don't start grammar checker
                        // again until the user modifies the document
                        pLayout->SetNeedGrammarCheck(false);
                    }
                    xGCIterator->startProofreading( xDoc, xFPIP );
                }
            }
        }
    }

    return bStarted;
}

/*
 * internal functions
 */
static void lcl_DelFormatIndices( SwFormat const * pFormat )
{
    SwFormatContent &rFormatContent = const_cast<SwFormatContent&>(pFormat->GetContent());
    if ( rFormatContent.GetContentIdx() )
        rFormatContent.SetNewContentIdx( nullptr );
    SwFormatAnchor &rFormatAnchor = const_cast<SwFormatAnchor&>(pFormat->GetAnchor());
    if ( rFormatAnchor.GetContentAnchor() )
        rFormatAnchor.SetAnchor( nullptr );
}

/*
 * exported methods
 */
SwDoc::SwDoc()
    : m_pNodes( new SwNodes(this) ),
    mpAttrPool(new SwAttrPool(this)),
    mpMarkManager(new ::sw::mark::MarkManager(*this)),
    m_pMetaFieldManager(new ::sw::MetaFieldManager()),
    m_pDocumentDrawModelManager( new ::sw::DocumentDrawModelManager( *this ) ),
    m_pDocumentRedlineManager( new ::sw::DocumentRedlineManager( *this ) ),
    m_pDocumentStateManager( new ::sw::DocumentStateManager( *this ) ),
    m_pUndoManager(new ::sw::UndoManager(
            std::shared_ptr<SwNodes>(new SwNodes(this)), *m_pDocumentDrawModelManager, *m_pDocumentRedlineManager, *m_pDocumentStateManager)),
    m_pDocumentSettingManager(new ::sw::DocumentSettingManager(*this)),
    m_pDocumentChartDataProviderManager( new sw::DocumentChartDataProviderManager( *this ) ),
    m_pDeviceAccess( new ::sw::DocumentDeviceManager( *this ) ),
    m_pDocumentTimerManager( new ::sw::DocumentTimerManager( *this ) ),
    m_pDocumentLinksAdministrationManager( new ::sw::DocumentLinksAdministrationManager( *this ) ),
    m_pDocumentListItemsManager( new ::sw::DocumentListItemsManager() ),
    m_pDocumentListsManager( new ::sw::DocumentListsManager( *this ) ),
    m_pDocumentOutlineNodesManager( new ::sw::DocumentOutlineNodesManager( *this ) ),
    m_pDocumentContentOperationsManager( new ::sw::DocumentContentOperationsManager( *this ) ),
    m_pDocumentFieldsManager( new ::sw::DocumentFieldsManager( *this ) ),
    m_pDocumentStatisticsManager( new ::sw::DocumentStatisticsManager( *this ) ),
    m_pDocumentLayoutManager( new ::sw::DocumentLayoutManager( *this ) ),
    m_pDocumentStylePoolManager( new ::sw::DocumentStylePoolManager( *this ) ),
    m_pDocumentExternalDataManager( new ::sw::DocumentExternalDataManager ),
    mpDfltFrameFormat( new SwFrameFormat( GetAttrPool(), "Frameformat", nullptr ) ),
    mpEmptyPageFormat( new SwFrameFormat( GetAttrPool(), "Empty Page", mpDfltFrameFormat.get() ) ),
    mpColumnContFormat( new SwFrameFormat( GetAttrPool(), "Columncontainer", mpDfltFrameFormat.get() ) ),
    mpDfltCharFormat( new SwCharFormat( GetAttrPool(), "Character style", nullptr ) ),
    mpDfltTextFormatColl( new SwTextFormatColl( GetAttrPool(), "Paragraph style" ) ),
    mpDfltGrfFormatColl( new SwGrfFormatColl( GetAttrPool(), "Graphikformatvorlage" ) ),
    mpFrameFormatTable( new SwFrameFormats() ),
    mpCharFormatTable( new SwCharFormats ),
    mpSpzFrameFormatTable( new SwFrameFormats() ),
    mpSectionFormatTable( new SwSectionFormats ),
    mpTableFrameFormatTable( new SwFrameFormats() ),
    mpTextFormatCollTable( new SwTextFormatColls() ),
    mpGrfFormatCollTable( new SwGrfFormatColls() ),
    mpTOXTypes( new SwTOXTypes ),
    mpDefTOXBases( new SwDefTOXBase_Impl() ),
    mpOutlineRule( nullptr ),
    mpFootnoteInfo( new SwFootnoteInfo ),
    mpEndNoteInfo( new SwEndNoteInfo ),
    mpLineNumberInfo( new SwLineNumberInfo ),
    mpFootnoteIdxs( new SwFootnoteIdxs ),
    mpDocShell( nullptr ),
    mpNumberFormatter( nullptr ),
    mpNumRuleTable( new SwNumRuleTable ),
    mpExtInputRing( nullptr ),
    mpGrammarContact(createGrammarContact()),
    mpCellStyles(new SwCellStyleTable),
    m_pXmlIdRegistry(),
    mReferenceCount(0),
    mbDtor(false),
    mbCopyIsMove(false),
    mbInReading(false),
    mbInMailMerge(false),
    mbInXMLImport(false),
    mbUpdateTOX(false),
    mbInLoadAsynchron(false),
    mbIsAutoFormatRedline(false),
    mbOLEPrtNotifyPending(false),
    mbAllOLENotify(false),
    mbInsOnlyTextGlssry(false),
    mbContains_MSVBasic(false),
    mbClipBoard( false ),
    mbColumnSelection( false ),
    mbIsPrepareSelAll(false),
    meDictionaryMissing( MissingDictionary::Undefined ),
    mbContainsAtPageObjWithContentAnchor(false), //#i119292#, fdo#37024

    meDocType(DOCTYPE_NATIVE)
{
    // The DrawingLayer ItemPool which is used as 2nd pool for Writer documents' pool
    // has a default for the XFillStyleItem of XFILL_SOLID and the color for it is the default
    // fill color (blue7 or similar). This is a problem, in Writer we want the default fill
    // style to be drawing::FillStyle_NONE. This cannot simply be done by changing it in the 2nd pool at the
    // pool defaults when the DrawingLayer ItemPool is used for Writer, that would lead to
    // countless problems like DrawObjects initial fill and others.
    // It is also hard to find all places where the initial ItemSets for Writer (including
    // style hierarchies) are created and to always set (but only at the root) the FillStyle
    // to NONE fixed; that will add that attribute to the file format. It will be hard to reset
    // attribute sets (which is done at import and using UI). Also not a good solution.
    // Luckily Writer uses pDfltTextFormatColl as default parent for all paragraphs and similar, thus
    // it is possible to set this attribute here. It will be not reset when importing.
    mpDfltTextFormatColl->SetFormatAttr(XFillStyleItem(drawing::FillStyle_NONE));
    mpDfltFrameFormat->SetFormatAttr(XFillStyleItem(drawing::FillStyle_NONE));
    // prevent paragraph default margins being applied to everything
    mpDfltFrameFormat->SetFormatAttr(SvxULSpaceItem(RES_UL_SPACE));
    mpDfltFrameFormat->SetFormatAttr(SvxLRSpaceItem(RES_LR_SPACE));

    /*
     * DefaultFormats and DefaultFormatCollections (FormatColl)
     * are inserted at position 0 at the respective array.
     * The formats in the FormatColls are derived from the
     * DefaultFormats and are also in the list.
     */
    /* Formats */
    mpFrameFormatTable->push_back(mpDfltFrameFormat.get());
    mpCharFormatTable->push_back(mpDfltCharFormat.get());

    /* FormatColls */
    // TXT
    mpTextFormatCollTable->push_back(mpDfltTextFormatColl.get());
    // GRF
    mpGrfFormatCollTable->push_back(mpDfltGrfFormatColl.get());

    // Create PageDesc, EmptyPageFormat and ColumnFormat
    if (m_PageDescs.empty())
        getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD );

    // Set to "Empty Page"
    mpEmptyPageFormat->SetFormatAttr( SwFormatFrameSize( ATT_FIX_SIZE ) );
    // Set BodyFormat for columns
    mpColumnContFormat->SetFormatAttr( SwFormatFillOrder( ATT_LEFT_TO_RIGHT ) );

    GetDocumentFieldsManager().InitFieldTypes();

    // Create a default OutlineNumRule (for Filters)
    mpOutlineRule = new SwNumRule( SwNumRule::GetOutlineRuleName(),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    mpOutlineRule->SetCountPhantoms( !GetDocumentSettingManager().get(DocumentSettingId::OLD_NUMBERING) );

    new SwTextNode(
            SwNodeIndex(GetUndoManager().GetUndoNodes().GetEndOfContent()),
            mpDfltTextFormatColl.get() );
    new SwTextNode( SwNodeIndex( GetNodes().GetEndOfContent() ),
                    getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ));

    maOLEModifiedIdle.SetPriority( TaskPriority::LOWEST );
    maOLEModifiedIdle.SetInvokeHandler( LINK( this, SwDoc, DoUpdateModifiedOLE ));
    maOLEModifiedIdle.SetDebugName( "sw::SwDoc maOLEModifiedIdle" );

#if HAVE_FEATURE_DBCONNECTIVITY
    // Create DBManager
    m_pOwnDBManager.reset(new SwDBManager(this));
    m_pDBManager = m_pOwnDBManager.get();
#else
    m_pDBManager = nullptr;
#endif

    // create TOXTypes
    InitTOXTypes();

    // pass empty item set containing the paragraph's list attributes
    // as ignorable items to the stype manager.
    {
        SfxItemSet aIgnorableParagraphItems( GetAttrPool(), svl::Items<RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1>{});
        mpStyleAccess = createStyleManager( &aIgnorableParagraphItems );
    }

    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);

    if (bHack)
    {
        mnRsid = 0;
    }
    else
    {
        // Initialize the session id of the current document to a random number
        // smaller than 2^21.
        mnRsid = comphelper::rng::uniform_uint_distribution(1, (1 << 21) - 1);
    }
    mnRsidRoot = mnRsid;

    getIDocumentState().ResetModified();
}

/**
 * Speciality: a member of the class SwDoc is located at
 * position 0 in the array of the Format and GDI objects.
 * This MUST not be destroyed using 'delete' in any case!
 */
SwDoc::~SwDoc()
{
    // nothing here should create Undo actions!
    GetIDocumentUndoRedo().DoUndo(false);

    if (mpDocShell)
    {
        mpDocShell->SetUndoManager(nullptr);
    }

    mpGrammarContact.reset();

    getIDocumentTimerAccess().StopIdling();   // stop idle timer

    mpURLStateChgd.reset();

    // Deactivate Undo notification from Draw
    if( GetDocumentDrawModelManager().GetDrawModel() )
    {
        GetDocumentDrawModelManager().DrawNotifyUndoHdl();
        ClrContourCache();
    }

    m_pPgPViewPrtData.reset();

    mbDtor = true;

    //Clear the redline table before the nodes array is destroyed
    getIDocumentRedlineAccess().GetRedlineTable().DeleteAndDestroyAll();
    getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteAndDestroyAll();

    const sw::UnoCursorHint aHint;
    cleanupUnoCursorTable();
    for(const auto& pWeakCursor : mvUnoCursorTable)
    {
        auto pCursor(pWeakCursor.lock());
        if(pCursor)
            pCursor->m_aNotifier.Broadcast(aHint);
    }
    mpACEWord.reset();

    // Release the BaseLinks
    {
        ::sfx2::SvLinkSources aTemp(getIDocumentLinksAdministration().GetLinkManager().GetServers());
        for( const auto& rpLinkSrc : aTemp )
            rpLinkSrc->Closed();

        if( !getIDocumentLinksAdministration().GetLinkManager().GetLinks().empty() )
            getIDocumentLinksAdministration().GetLinkManager().Remove( 0, getIDocumentLinksAdministration().GetLinkManager().GetLinks().size() );
    }

    // The ChapterNumbers/Numbers need to be deleted before the styles
    // or we update all the time!
    m_pNodes->m_pOutlineNodes->clear();
    SwNodes & rUndoNodes( GetUndoManager().GetUndoNodes() );
    rUndoNodes.m_pOutlineNodes->clear();

    mpFootnoteIdxs->clear();

    // indices could be registered in attributes
    m_pUndoManager->DelAllUndoObj();

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    mpMarkManager->clearAllMarks();

    if( mpExtInputRing )
    {
        SwPaM* pTmp = mpExtInputRing;
        mpExtInputRing = nullptr;
        while( pTmp->GetNext() != pTmp )
            delete pTmp->GetNext();
        delete pTmp;
    }

    // Old - deletion without a Flag is expensive, because we send a Modify
    // aTOXTypes.DeleteAndDestroy( 0, aTOXTypes.Count() );
    {
        for( auto n = mpTOXTypes->size(); n; )
        {
            (*mpTOXTypes)[ --n ]->SetInDocDTOR();
            delete (*mpTOXTypes)[ n ];
        }
        mpTOXTypes->clear();
    }
    mpDefTOXBases.reset();

    // Any of the FrameFormats can still have indices registered.
    // These need to be destroyed now at the latest.
    for( SwFrameFormat* pFormat : *mpFrameFormatTable )
        lcl_DelFormatIndices( pFormat );
    for( SwFrameFormat* pFormat : *mpSpzFrameFormatTable )
        lcl_DelFormatIndices( pFormat );
    for( SwSectionFormat* pFormat : *mpSectionFormatTable )
        lcl_DelFormatIndices( pFormat );

    // The formats/styles that follow depend on the default formats.
    // Destroy these only after destroying the FormatIndices, because the content
    // of headers/footers has to be deleted as well. If in the headers/footers
    // there are still Flys registered at that point, we have a problem.
    for( SwPageDesc *pPageDesc : m_PageDescs )
        delete pPageDesc;
    m_PageDescs.clear();

    // Delete content selections.
    // Don't wait for the SwNodes dtor to destroy them; so that Formats
    // do not have any dependencies anymore.
    m_pNodes->DelNodes( SwNodeIndex(*m_pNodes), m_pNodes->Count() );
    rUndoNodes.DelNodes( SwNodeIndex( rUndoNodes ), rUndoNodes.Count() );

    // Delete Formats, make it permanent some time in the future

    // Delete for Collections
    // So that we get rid of the dependencies
    mpFootnoteInfo->EndListeningAll();
    mpEndNoteInfo->EndListeningAll();

    assert(mpDfltTextFormatColl.get() == (*mpTextFormatCollTable)[0]
            && "Default-Text-Collection must always be at the start");

    // Optimization: Based on the fact that Standard is always 2nd in the
    // array, we should delete it as the last. With this we avoid
    // reparenting the Formats all the time!
    if( 2 < mpTextFormatCollTable->size() )
        mpTextFormatCollTable->DeleteAndDestroy(2, mpTextFormatCollTable->size());
    mpTextFormatCollTable->DeleteAndDestroy(1, mpTextFormatCollTable->size());
    mpTextFormatCollTable.reset();

    assert(mpDfltGrfFormatColl.get() == (*mpGrfFormatCollTable)[0]
            && "DefaultGrfCollection must always be at the start");

    mpGrfFormatCollTable->DeleteAndDestroy(1, mpGrfFormatCollTable->size());
    mpGrfFormatCollTable.reset();

    // Without explicitly freeing the DocumentDeviceManager
    // and relying on the implicit freeing there would be a crash
    // due to it happening after SwAttrPool is freed.
    m_pDeviceAccess.reset();

    /*
     * DefaultFormats and DefaultFormatCollections (FormatColl)
     * are at position 0 of their respective arrays.
     * In order to not be deleted by the array's dtor, we remove them
     * now.
     */
    mpFrameFormatTable->erase( mpFrameFormatTable->begin() );
    mpCharFormatTable->erase( mpCharFormatTable->begin() );

#if HAVE_FEATURE_DBCONNECTIVITY
    // On load, SwDBManager::setEmbeddedName() may register a data source.
    // If we have an embedded one, then sDataSource points to the registered name, so revoke it here.
    if (!m_pOwnDBManager->getEmbeddedName().isEmpty() && !maDBData.sDataSource.isEmpty())
    {
        // Remove the revoke listener here first, so that we don't remove the data source from the document.
        m_pOwnDBManager->releaseRevokeListener();
        SwDBManager::RevokeDataSource(maDBData.sDataSource);
        SwDBManager::RevokeDataSource(m_pOwnDBManager->getEmbeddedName());
    }
    else if (!m_pOwnDBManager->getEmbeddedName().isEmpty())
    {
        // Remove the revoke listener here first, so that we don't remove the data source from the document.
        m_pOwnDBManager->releaseRevokeListener();
        // Remove connections which was committed but not used.
        m_pOwnDBManager->RevokeNotUsedConnections();
    }

    m_pOwnDBManager.reset();
#endif

    // All Flys need to be destroyed before the Drawing Model,
    // because Flys can still contain DrawContacts, when no
    // Layout could be constructed due to a read error.
    mpSpzFrameFormatTable->DeleteAndDestroyAll();

    // Only now destroy the Model, the drawing objects - which are also
    // contained in the Undo - need to remove their attributes from the
    // Model. Also, DrawContacts could exist before this.
    GetDocumentDrawModelManager().ReleaseDrawModel();
    // Destroy DrawModel before the LinkManager, because it's always set
    // in the DrawModel.
    //The LinkManager gets destroyed automatically with m_pLinksAdministrationManager

    // Clear the Tables before deleting the defaults, or we crash due to
    // dependencies on defaults.
    mpFrameFormatTable.reset();
    mpSpzFrameFormatTable.reset();

    mpStyleAccess.reset();

    mpCharFormatTable.reset();
    mpSectionFormatTable.reset();
    mpTableFrameFormatTable.reset();
    mpDfltTextFormatColl.reset();
    mpDfltGrfFormatColl.reset();
    mpNumRuleTable.reset();

    disposeXForms(); // #i113606#, dispose the XForms objects

    delete mpNumberFormatter.load(); mpNumberFormatter= nullptr;
    mpFootnoteInfo.reset();
    mpEndNoteInfo.reset();
    mpLineNumberInfo.reset();
    mpFootnoteIdxs.reset();
    mpTOXTypes.reset();
    mpEmptyPageFormat.reset();
    mpColumnContFormat.reset();
    mpDfltCharFormat.reset();
    mpDfltFrameFormat.reset();
    mpLayoutCache.reset();

    SfxItemPool::Free(mpAttrPool);
}

void SwDoc::SetDocShell( SwDocShell* pDSh )
{
    if( mpDocShell != pDSh )
    {
        if (mpDocShell)
        {
            mpDocShell->SetUndoManager(nullptr);
        }
        mpDocShell = pDSh;
        if (mpDocShell)
        {
            mpDocShell->SetUndoManager(& GetUndoManager());
            GetUndoManager().SetDocShell(mpDocShell);
        }

        getIDocumentLinksAdministration().GetLinkManager().SetPersist( mpDocShell );

        // set DocShell pointer also on DrawModel
        InitDrawModelAndDocShell(mpDocShell, GetDocumentDrawModelManager().GetDrawModel());
        assert(!GetDocumentDrawModelManager().GetDrawModel() ||
            GetDocumentDrawModelManager().GetDrawModel()->GetPersist() == GetPersist());
    }
}

// Convenience method; to avoid excessive includes from docsh.hxx
uno::Reference < embed::XStorage > SwDoc::GetDocStorage()
{
    if( mpDocShell )
        return mpDocShell->GetStorage();
    if( getIDocumentLinksAdministration().GetLinkManager().GetPersist() )
        return getIDocumentLinksAdministration().GetLinkManager().GetPersist()->GetStorage();
    return nullptr;
}

SfxObjectShell* SwDoc::GetPersist() const
{
    return mpDocShell ? mpDocShell : getIDocumentLinksAdministration().GetLinkManager().GetPersist();
}

void SwDoc::ClearDoc()
{
    GetIDocumentUndoRedo().DelAllUndoObj();
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Deactivate Undo notification from Draw
    if( GetDocumentDrawModelManager().GetDrawModel() )
    {
        GetDocumentDrawModelManager().DrawNotifyUndoHdl();
        ClrContourCache();
    }

    // if there are still FlyFrames dangling around, delete them too
    while ( !mpSpzFrameFormatTable->empty() )
        getIDocumentLayoutAccess().DelLayoutFormat((*mpSpzFrameFormatTable)[mpSpzFrameFormatTable->size()-1]);
    assert(!GetDocumentDrawModelManager().GetDrawModel()
        || !GetDocumentDrawModelManager().GetDrawModel()->GetPage(0)->GetObjCount());

    getIDocumentRedlineAccess().GetRedlineTable().DeleteAndDestroyAll();
    getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteAndDestroyAll();

    mpACEWord.reset();

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    mpMarkManager->clearAllMarks();
    InitTOXTypes();

    // create a dummy pagedesc for the layout
    SwPageDesc* pDummyPgDsc = MakePageDesc("?DUMMY?");

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // create the first one over and over again (without attributes/style etc.
    SwTextNode* pFirstNd = GetNodes().MakeTextNode( aSttIdx, mpDfltTextFormatColl.get() );

    if( getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        // set the layout to the dummy pagedesc
        pFirstNd->SetAttr( SwFormatPageDesc( pDummyPgDsc ));

        SwPosition aPos( *pFirstNd, SwIndex( pFirstNd ));
        SwPaM const tmpPaM(aSttIdx, SwNodeIndex(GetNodes().GetEndOfContent()));
        ::PaMCorrAbs(tmpPaM, aPos);
    }

    GetNodes().Delete( aSttIdx,
            GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );

    // #i62440#
    // destruction of numbering rules and creation of new outline rule
    // *after* the document nodes are deleted.
    mpOutlineRule = nullptr;
    for( SwNumRule* pNumRule : *mpNumRuleTable )
    {
        getIDocumentListsAccess().deleteListForListStyle(pNumRule->GetName());
        delete pNumRule;
    }
    mpNumRuleTable->clear();
    maNumRuleMap.clear();

    // creation of new outline numbering rule
    mpOutlineRule = new SwNumRule( SwNumRule::GetOutlineRuleName(),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    mpOutlineRule->SetCountPhantoms( !GetDocumentSettingManager().get(DocumentSettingId::OLD_NUMBERING) );

    // remove the dummy pagedesc from the array and delete all the old ones
    size_t nDummyPgDsc = 0;
    if (FindPageDesc(pDummyPgDsc->GetName(), &nDummyPgDsc))
        m_PageDescs.erase( nDummyPgDsc );
    for( SwPageDesc *pPageDesc : m_PageDescs )
        delete pPageDesc;
    m_PageDescs.clear();

    // Delete for Collections
    // So that we get rid of the dependencies
    mpFootnoteInfo->EndListeningAll();
    mpEndNoteInfo->EndListeningAll();

    // Optimization: Based on the fact that Standard is always 2nd in the
    // array, we should delete it as the last. With this we avoid
    // reparenting the Formats all the time!
    if( 2 < mpTextFormatCollTable->size() )
        mpTextFormatCollTable->DeleteAndDestroy(2, mpTextFormatCollTable->size());
    mpTextFormatCollTable->DeleteAndDestroy(1, mpTextFormatCollTable->size());
    mpGrfFormatCollTable->DeleteAndDestroy(1, mpGrfFormatCollTable->size());
    mpCharFormatTable->DeleteAndDestroy(1, mpCharFormatTable->size());

    if( getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        // search the FrameFormat of the root frm. This is not allowed to delete
        mpFrameFormatTable->erase( getIDocumentLayoutAccess().GetCurrentViewShell()->GetLayout()->GetFormat() );
        mpFrameFormatTable->DeleteAndDestroyAll( true );
        mpFrameFormatTable->push_back( getIDocumentLayoutAccess().GetCurrentViewShell()->GetLayout()->GetFormat() );
    }
    else
        mpFrameFormatTable->DeleteAndDestroyAll( true );

    GetDocumentFieldsManager().ClearFieldTypes();

    delete mpNumberFormatter.load(); mpNumberFormatter= nullptr;

    getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    pFirstNd->ChgFormatColl( getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ));
    nDummyPgDsc = m_PageDescs.size();
    m_PageDescs.push_back( pDummyPgDsc );
    // set the layout back to the new standard pagedesc
    pFirstNd->ResetAllAttr();
    // delete now the dummy pagedesc
    DelPageDesc( nDummyPgDsc );
}

void SwDoc::SetPreviewPrtData( const SwPagePreviewPrtData* pNew )
{
    if( pNew )
    {
        if (m_pPgPViewPrtData)
        {
            *m_pPgPViewPrtData = *pNew;
        }
        else
        {
            m_pPgPViewPrtData.reset(new SwPagePreviewPrtData(*pNew));
        }
    }
    else if (m_pPgPViewPrtData)
    {
        m_pPgPViewPrtData.reset();
    }
    getIDocumentState().SetModified();
}

void SwDoc::SetOLEObjModified()
{
    if( getIDocumentLayoutAccess().GetCurrentViewShell() ) maOLEModifiedIdle.Start();
}

/** SwDoc: Reading and writing of the layout cache. */
void SwDoc::ReadLayoutCache( SvStream& rStream )
{
    if( !mpLayoutCache )
        mpLayoutCache.reset( new SwLayoutCache() );
    if( !mpLayoutCache->IsLocked() )
    {
        mpLayoutCache->GetLockCount() |= 0x8000;
        mpLayoutCache->Read( rStream );
        mpLayoutCache->GetLockCount() &= 0x7fff;
    }
}

void SwDoc::WriteLayoutCache( SvStream& rStream )
{
    SwLayoutCache::Write( rStream, *this );
}

IGrammarContact* getGrammarContact( const SwTextNode& rTextNode )
{
    const SwDoc* pDoc = rTextNode.GetDoc();
    if( !pDoc || pDoc->IsInDtor() )
        return nullptr;
    return pDoc->getGrammarContact();
}

::sfx2::IXmlIdRegistry&
SwDoc::GetXmlIdRegistry()
{
    // UGLY: this relies on SetClipBoard being called before GetXmlIdRegistry!
    if (!m_pXmlIdRegistry)
    {
        m_pXmlIdRegistry.reset( ::sfx2::createXmlIdRegistry( IsClipBoard() ) );
    }
    return *m_pXmlIdRegistry;
}

void SwDoc::InitTOXTypes()
{
    ShellResource* pShellRes = SwViewShell::GetShellRes();
    SwTOXType * pNew = new SwTOXType(TOX_CONTENT,   pShellRes->aTOXContentName        );
    mpTOXTypes->push_back( pNew );
    pNew = new SwTOXType(TOX_INDEX,                 pShellRes->aTOXIndexName  );
    mpTOXTypes->push_back( pNew );
    pNew = new SwTOXType(TOX_USER,                  pShellRes->aTOXUserName  );
    mpTOXTypes->push_back( pNew );
    pNew = new SwTOXType(TOX_ILLUSTRATIONS,         pShellRes->aTOXIllustrationsName );
    mpTOXTypes->push_back( pNew );
    pNew = new SwTOXType(TOX_OBJECTS,               pShellRes->aTOXObjectsName       );
    mpTOXTypes->push_back( pNew );
    pNew = new SwTOXType(TOX_TABLES,                pShellRes->aTOXTablesName        );
    mpTOXTypes->push_back( pNew );
    pNew = new SwTOXType(TOX_AUTHORITIES,           pShellRes->aTOXAuthoritiesName   );
    mpTOXTypes->push_back( pNew );
    pNew = new SwTOXType(TOX_CITATION,           pShellRes->aTOXCitationName   );
    mpTOXTypes->push_back( pNew );
}

void SwDoc::ReplaceDefaults(const SwDoc& rSource)
{
    // copy property defaults
    const sal_uInt16 aRangeOfDefaults[] =
    {
        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
        RES_PARATR_BEGIN, RES_PARATR_END-1,
        RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
        XATTR_START, XATTR_END-1,
        0
    };

    SfxItemSet aNewDefaults(GetAttrPool(), aRangeOfDefaults);

    for (auto nRange = 0; aRangeOfDefaults[nRange] != 0; nRange += 2)
    {
        for (sal_uInt16 nWhich = aRangeOfDefaults[nRange];
             nWhich <= aRangeOfDefaults[nRange + 1]; ++nWhich)
        {
            const SfxPoolItem& rSourceAttr =
                rSource.mpAttrPool->GetDefaultItem(nWhich);
            if (rSourceAttr != mpAttrPool->GetDefaultItem(nWhich))
                aNewDefaults.Put(rSourceAttr);
        }
    }

    if (aNewDefaults.Count())
        SetDefault(aNewDefaults);
}

void SwDoc::ReplaceCompatibilityOptions(const SwDoc& rSource)
{
    m_pDocumentSettingManager->ReplaceCompatibilityOptions(rSource.GetDocumentSettingManager());
}

#ifdef DBG_UTIL
#define CNTNT_DOC( doc ) \
    ((doc)->GetNodes().GetEndOfContent().GetIndex() - (doc)->GetNodes().GetEndOfExtras().GetIndex() - 2)
#define CNTNT_IDX( idx ) \
    ((idx).GetNode().GetIndex() - GetNodes().GetEndOfExtras().GetIndex() - 1)
#endif

SfxObjectShell* SwDoc::CreateCopy( bool bCallInitNew, bool bEmpty ) const
{
    SAL_INFO( "sw.pageframe", "(SwDoc::CreateCopy in" );
    rtl::Reference<SwDoc> xRet( new SwDoc );

    // we have to use pointer here, since the callee has to decide whether
    // SfxObjectShellLock or SfxObjectShellRef should be used sometimes the
    // object will be returned with refcount set to 0 ( if no DoInitNew is done )
    SfxObjectShell* pRetShell = new SwDocShell( xRet.get(), SfxObjectCreateMode::STANDARD );
    if( bCallInitNew )
    {
        // it could happen that DoInitNew creates model,
        // that increases the refcount of the object
        pRetShell->DoInitNew();
    }

    xRet->ReplaceDefaults(*this);

    xRet->ReplaceCompatibilityOptions(*this);

    xRet->ReplaceStyles(*this);

    if( !bEmpty )
    {
#ifdef DBG_UTIL
        SAL_INFO( "sw.createcopy", "CC-Nd-Src: " << CNTNT_DOC( this ) );
        SAL_INFO( "sw.createcopy", "CC-Nd: " << CNTNT_DOC( xRet ) );
#endif
        xRet->AppendDoc(*this, 0, bCallInitNew, 0, 0);
#ifdef DBG_UTIL
        SAL_INFO( "sw.createcopy", "CC-Nd: " << CNTNT_DOC( xRet ) );
#endif
    }

    // remove the temporary shell if it is there as it was done before
    xRet->SetTmpDocShell( nullptr );

    SAL_INFO( "sw.pageframe", "SwDoc::CreateCopy out)" );
    return pRetShell;
}

// save bulk letters as single documents
static OUString lcl_FindUniqueName(SwWrtShell* pTargetShell, const OUString& rStartingPageDesc, sal_uLong nDocNo )
{
    do
    {
        OUString sTest = rStartingPageDesc;
        sTest += OUString::number( nDocNo );
        if( !pTargetShell->FindPageDescByName( sTest ) )
            return sTest;
        ++nDocNo;
    }
    while( true );
}

static void lcl_CopyFollowPageDesc(
                            SwWrtShell& rTargetShell,
                            const SwPageDesc& rSourcePageDesc,
                            const SwPageDesc& rTargetPageDesc,
                            const sal_uLong nDocNo )
{
    //now copy the follow page desc, too
    // note: these may at any point form a cycle, so a loop is needed and it
    // must be detected that the last iteration closes the cycle and doesn't
    // copy the first page desc of the cycle again.
    std::map<OUString, OUString> followMap{ { rSourcePageDesc.GetName(), rTargetPageDesc.GetName() } };
    SwPageDesc const* pCurSourcePageDesc(&rSourcePageDesc);
    SwPageDesc const* pCurTargetPageDesc(&rTargetPageDesc);
    do
    {
        const SwPageDesc* pFollowPageDesc = pCurSourcePageDesc->GetFollow();
        OUString sFollowPageDesc = pFollowPageDesc->GetName();
        if (sFollowPageDesc == pCurSourcePageDesc->GetName())
        {
            break;
        }
        SwDoc* pTargetDoc = rTargetShell.GetDoc();
        SwPageDesc* pTargetFollowPageDesc(nullptr);
        auto const itMapped(followMap.find(sFollowPageDesc));
        if (itMapped == followMap.end())
        {
            OUString sNewFollowPageDesc = lcl_FindUniqueName(&rTargetShell, sFollowPageDesc, nDocNo);
            pTargetFollowPageDesc = pTargetDoc->MakePageDesc(sNewFollowPageDesc);
            pTargetDoc->CopyPageDesc(*pFollowPageDesc, *pTargetFollowPageDesc, false);
        }
        else
        {
            pTargetFollowPageDesc = pTargetDoc->FindPageDesc(itMapped->second);
        }
        SwPageDesc aDesc(*pCurTargetPageDesc);
        aDesc.SetFollow(pTargetFollowPageDesc);
        pTargetDoc->ChgPageDesc(pCurTargetPageDesc->GetName(), aDesc);
        if (itMapped != followMap.end())
        {
            break; // was already copied
        }
        pCurSourcePageDesc = pCurSourcePageDesc->GetFollow();
        pCurTargetPageDesc = pTargetFollowPageDesc;
        followMap[pCurSourcePageDesc->GetName()] = pCurTargetPageDesc->GetName();
    }
    while (true);
}

// appends all pages of source SwDoc - based on SwFEShell::Paste( SwDoc* )
SwNodeIndex SwDoc::AppendDoc(const SwDoc& rSource, sal_uInt16 const nStartPageNumber,
                             bool const bDeletePrevious, int pageOffset, const sal_uLong nDocNo)
{
    SAL_INFO( "sw.pageframe", "(SwDoc::AppendDoc in " << bDeletePrevious );

    // GetEndOfExtras + 1 = StartOfContent == no content node!
    // This ensures it won't be merged in the SwTextNode at the position.
    SwNodeIndex aSourceIdx( rSource.GetNodes().GetEndOfExtras(), 1 );
    // CopyRange works on the range a [mark, point[ and considers an
    // index < point outside the selection.
    // @see IDocumentContentOperations::CopyRange
    SwNodeIndex aSourceEndIdx( rSource.GetNodes().GetEndOfContent(), 0 );
    SwPaM aCpyPam( aSourceIdx, aSourceEndIdx );

#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << static_cast<int>(aSourceIdx.GetNode().GetNodeType())
                              << std::dec << " " << aSourceIdx.GetNode().GetIndex() );
    aSourceIdx++;
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << static_cast<int>(aSourceIdx.GetNode().GetNodeType())
                                            << std::dec << " " << aSourceIdx.GetNode().GetIndex() );
    if ( aSourceIdx.GetNode().GetNodeType() != SwNodeType::End ) {
        aSourceIdx++;
        SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << static_cast<int>(aSourceIdx.GetNode().GetNodeType()) << std::dec );
        aSourceIdx--;
    }
    aSourceIdx--;
    SAL_INFO( "sw.docappend", ".." );
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << static_cast<int>(aSourceEndIdx.GetNode().GetNodeType())
                              << std::dec << " " << aSourceEndIdx.GetNode().GetIndex() );
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << static_cast<int>(aSourceEndIdx.GetNode().GetNodeType())
                              << std::dec << " " << aSourceEndIdx.GetNode().GetIndex() );
    SAL_INFO( "sw.docappend", "Src-Nd: " << CNTNT_DOC( &rSource ) );
    SAL_INFO( "sw.docappend", "Nd: " << CNTNT_DOC( this ) );
#endif

    SwWrtShell* pTargetShell = GetDocShell()->GetWrtShell();
    SwPageDesc* pTargetPageDesc = nullptr;

    if ( pTargetShell ) {
#ifdef DBG_UTIL
        SAL_INFO( "sw.docappend", "Has target write shell" );
#endif
        pTargetShell->StartAllAction();

        if( nDocNo > 0 )
        {
            // #i72517# put the styles to the target document
            // if the source uses headers or footers the target document
            // needs inidividual page styles
            const SwWrtShell *pSourceShell = rSource.GetDocShell()->GetWrtShell();
            const SwPageDesc *pSourcePageDesc = &pSourceShell->GetPageDesc(
                                                    pSourceShell->GetCurPageDesc());
            const OUString sStartingPageDesc = pSourcePageDesc->GetName();
            const SwFrameFormat& rMaster = pSourcePageDesc->GetMaster();
            const bool bPageStylesWithHeaderFooter = rMaster.GetHeader().IsActive() ||
                                                     rMaster.GetFooter().IsActive();
            if( bPageStylesWithHeaderFooter )
            {
                // create a new pagestyle
                // copy the pagedesc from the current document to the new
                // document and change the name of the to-be-applied style
                OUString sNewPageDescName = lcl_FindUniqueName(pTargetShell, sStartingPageDesc, nDocNo );
                pTargetPageDesc = MakePageDesc( sNewPageDescName );
                if( pTargetPageDesc )
                {
                    CopyPageDesc( *pSourcePageDesc, *pTargetPageDesc, false );
                    lcl_CopyFollowPageDesc( *pTargetShell, *pSourcePageDesc, *pTargetPageDesc, nDocNo );
                }
            }
            else
                pTargetPageDesc = pTargetShell->FindPageDescByName( sStartingPageDesc );
        }

        // Otherwise we have to handle SwPlaceholderNodes as first node
        if ( pTargetPageDesc )
        {
            SwNodeIndex aBreakIdx( GetNodes().GetEndOfContent(), -1 );
            SwPosition aBreakPos( aBreakIdx );
            // InsertPageBreak just works on SwTextNode nodes, so make
            // sure the last node is one!
            bool bIsTextNode = aBreakIdx.GetNode().IsTextNode();
            if ( !bIsTextNode )
                getIDocumentContentOperations().AppendTextNode( aBreakPos );
            OUString name = pTargetPageDesc->GetName();
            pTargetShell->InsertPageBreak( &name, nStartPageNumber );
            if ( !bIsTextNode )
            {
                pTargetShell->SttEndDoc( false );
                --aBreakIdx;
                GetNodes().Delete( aBreakIdx );
            }

            // There is now a new empty text node on the new page. If it has
            // any marks, those are from the previous page: move them back
            // there, otherwise later we can't delete that empty text node.
            SwNodeIndex aNodeIndex(GetNodes().GetEndOfContent(), -1);
            if (SwTextNode* pTextNode = aNodeIndex.GetNode().GetTextNode())
            {
                // Position of the last paragraph on the previous page.
                --aNodeIndex;
                SwPaM aPaM(aNodeIndex);
                // Collect the marks starting or ending at this text node.
                std::set<sw::mark::IMark*> aSeenMarks;
                IDocumentMarkAccess* pMarkAccess = getIDocumentMarkAccess();
                for (const SwIndex* pIndex = pTextNode->GetFirstIndex(); pIndex; pIndex = pIndex->GetNext())
                {
                    sw::mark::IMark* pMark = const_cast<sw::mark::IMark*>(pIndex->GetMark());
                    if (!pMark)
                        continue;
                    if (aSeenMarks.find(pMark) != aSeenMarks.end())
                        continue;
                    aSeenMarks.insert(pMark);
                }
                // And move them back.
                for (sw::mark::IMark* pMark : aSeenMarks)
                    pMarkAccess->repositionMark(pMark, aPaM);
            }

            // Flush the page break, if we want to keep it
            if ( !bDeletePrevious )
            {
                SAL_INFO( "sw.pageframe", "(Flush pagebreak AKA EndAllAction" );
                pTargetShell->EndAllAction();
                SAL_INFO( "sw.pageframe",  "Flush changes AKA EndAllAction)" );
                pTargetShell->StartAllAction();
            }
        }
    }
#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "Nd: " << CNTNT_DOC( this ) );
#endif

    // -1, otherwise aFixupIdx would move to new EOC
    SwNodeIndex aFixupIdx( GetNodes().GetEndOfContent(), -1 );

    // append at the end of document / content
    SwNodeIndex aTargetIdx( GetNodes().GetEndOfContent() );
    SwPaM aInsertPam( aTargetIdx );

#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "Pam-Nd: " << aCpyPam.GetNode().GetIndex() - aCpyPam.GetNode( false ).GetIndex() + 1
                              << " (0x" << std::hex << static_cast<int>(aCpyPam.GetNode( false ).GetNodeType()) << std::dec
                              << " " << aCpyPam.GetNode( false ).GetIndex()
                              << " - 0x" << std::hex << static_cast<int>(aCpyPam.GetNode().GetNodeType()) << std::dec
                              << " " << aCpyPam.GetNode().GetIndex() << ")" );
#endif

    GetIDocumentUndoRedo().StartUndo( SwUndoId::INSGLOSSARY, nullptr );
    getIDocumentFieldsAccess().LockExpFields();

    // Position where the appended doc starts. Will be filled in later.
    // Initially uses GetEndOfContent() because SwNodeIndex has no default ctor.
    SwNodeIndex aStartAppendIndex( GetNodes().GetEndOfContent() );

    {
        // **
        // ** refer to SwFEShell::Paste, if you change the following code **
        // **

        SwPosition& rInsPos = *aInsertPam.GetPoint();

        {
            SwNodeIndex aIndexBefore(rInsPos.nNode);

            aIndexBefore--;
#ifdef DBG_UTIL
            SAL_INFO( "sw.docappend", "CopyRange In: " << CNTNT_DOC( this ) );
#endif
            rSource.getIDocumentContentOperations().CopyRange( aCpyPam, rInsPos, /*bCopyAll=*/true, /*bCheckPos=*/true );
            // Note: aCpyPam is invalid now
#ifdef DBG_UTIL
            SAL_INFO( "sw.docappend", "CopyRange Out: " << CNTNT_DOC( this ) );
#endif

            ++aIndexBefore;
            SwPaM aPaM(SwPosition(aIndexBefore),
                       SwPosition(rInsPos.nNode));

            aPaM.GetDoc()->MakeUniqueNumRules(aPaM);

            // Update the rsid of each pasted text node
            SwNodes &rDestNodes = GetNodes();
            sal_uLong const nEndIdx = aPaM.End()->nNode.GetIndex();

            for (sal_uLong nIdx = aPaM.Start()->nNode.GetIndex();
                    nIdx <= nEndIdx; ++nIdx)
            {
                SwTextNode *const pTextNode = rDestNodes[nIdx]->GetTextNode();
                if ( pTextNode )
                    UpdateParRsid( pTextNode );
            }
        }

        {
            sal_uLong iDelNodes = 0;
            SwNodeIndex aDelIdx( aFixupIdx );

            // we just need to set the new page description and reset numbering
            // this keeps all other settings as in the pasted document
            if ( nStartPageNumber || pTargetPageDesc ) {
                std::unique_ptr<SfxPoolItem> pNewItem;
                SwTextNode *aTextNd = nullptr;
                SwFormat *pFormat = nullptr;

                // find the first node allowed to contain a RES_PAGEDESC
                while (true) {
                    aFixupIdx++;

                    SwNode &node = aFixupIdx.GetNode();
                    if ( node.IsTextNode() ) {
                        // every document contains at least one text node!
                        aTextNd = node.GetTextNode();
                        pNewItem.reset(aTextNd->GetAttr( RES_PAGEDESC ).Clone());
                        break;
                    }
                    else if ( node.IsTableNode() ) {
                        pFormat = node.GetTableNode()->GetTable().GetFrameFormat();
                        pNewItem.reset(pFormat->GetFormatAttr( RES_PAGEDESC ).Clone());
                        break;
                    }
                }

#ifdef DBG_UTIL
                SAL_INFO( "sw.docappend", "Idx Del " << CNTNT_IDX( aDelIdx ) );
                SAL_INFO( "sw.docappend", "Idx Fix " << CNTNT_IDX( aFixupIdx ) );
#endif
                // just update the original instead of overwriting
                SwFormatPageDesc *aDesc = static_cast< SwFormatPageDesc* >( pNewItem.get() );
#ifdef DBG_UTIL
                if ( aDesc->GetPageDesc() )
                    SAL_INFO( "sw.docappend", "PD Update " << aDesc->GetPageDesc()->GetName() );
                else
                    SAL_INFO( "sw.docappend", "PD New" );
#endif
                if ( nStartPageNumber )
                    aDesc->SetNumOffset( nStartPageNumber );
                if ( pTargetPageDesc )
                    aDesc->RegisterToPageDesc( *pTargetPageDesc );
                if ( aTextNd )
                    aTextNd->SetAttr( *aDesc );
                else
                    pFormat->SetFormatAttr( *aDesc );

#ifdef DBG_UTIL
                SAL_INFO( "sw.docappend", "Idx " << CNTNT_IDX( aDelIdx ) );
#endif
                iDelNodes++;
            }

            if ( bDeletePrevious )
                iDelNodes++;

            if ( iDelNodes ) {
                // delete leading empty page(s), e.g. from InsertPageBreak or
                // new SwDoc. this has to be done before copying the page bound
                // frames, otherwise the drawing layer gets confused.
                if ( pTargetShell )
                    pTargetShell->SttEndDoc( false );
                aDelIdx -= iDelNodes - 1;
#ifdef DBG_UTIL
                SAL_INFO( "sw.docappend", "iDelNodes: " << iDelNodes
                              << "  Idx: " << aDelIdx.GetNode().GetIndex()
                              << "  EOE: " << GetNodes().GetEndOfExtras().GetIndex() );
#endif
                GetNodes().Delete( aDelIdx, iDelNodes );
                aStartAppendIndex = aFixupIdx;
            }
            else
            {
                aStartAppendIndex = aFixupIdx;
                ++aStartAppendIndex;
            }
        }

        // finally copy page bound frames
        for ( auto pCpyFormat : *rSource.GetSpzFrameFormats() )
        {
            const SwFrameFormat& rCpyFormat = *pCpyFormat;
            SwFormatAnchor aAnchor( rCpyFormat.GetAnchor() );
            if (RndStdIds::FLY_AT_PAGE != aAnchor.GetAnchorId())
                continue;
            SAL_INFO( "sw.docappend", "PaAn: " << aAnchor.GetPageNum()
                                      << " => " << aAnchor.GetPageNum() + pageOffset );
            if ( pageOffset != 0 )
                aAnchor.SetPageNum( aAnchor.GetPageNum() + pageOffset );
            getIDocumentLayoutAccess().CopyLayoutFormat( rCpyFormat, aAnchor, true, true );
        }
    }

    GetIDocumentUndoRedo().EndUndo( SwUndoId::INSGLOSSARY, nullptr );

    getIDocumentFieldsAccess().UnlockExpFields();
    getIDocumentFieldsAccess().UpdateFields(false);

    if ( pTargetShell )
        pTargetShell->EndAllAction();

    SAL_INFO( "sw.pageframe", "SwDoc::AppendDoc out)" );
    return aStartAppendIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
