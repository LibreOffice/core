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
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XFlatParagraphIteratorProvider.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/random.h>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>

#include <svl/macitem.hxx>
#include <svx/svxids.hrc>
#include <svx/svdogrp.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svl/zforlist.hxx>
#include <unotools/lingucfg.hxx>
#include <svx/svdpage.hxx>
#include <paratr.hxx>
#include <fchrfmt.hxx>
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
#include <swstylemanager.hxx>
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

//UUUU
#include <svx/xfillit0.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;

const sal_Char sFrameFormatStr[] = "Frameformat";
const sal_Char sEmptyPageStr[] = "Empty Page";
const sal_Char sColumnCntStr[] = "Columncontainer";
const sal_Char sCharFormatStr[] = "Character style";
const sal_Char sTextCollStr[] = "Paragraph style";
const sal_Char sGrfCollStr[] = "Graphikformatvorlage";

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

void StartGrammarChecking( SwDoc &rDoc )
{
    // check for a visible view
    bool bVisible = false;
    const SwDocShell *pDocShell = rDoc.GetDocShell();
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
        uno::Reference< linguistic2::XProofreadingIterator > xGCIterator( rDoc.GetGCIterator() );
        if ( xGCIterator.is() )
        {
            uno::Reference< lang::XComponent >  xDoc( rDoc.GetDocShell()->GetBaseModel(), uno::UNO_QUERY );
            uno::Reference< text::XFlatParagraphIteratorProvider >  xFPIP( xDoc, uno::UNO_QUERY );

            // start automatic background checking if not active already
            if ( xFPIP.is() && !xGCIterator->isProofreading( xDoc ) )
                xGCIterator->startProofreading( xDoc, xFPIP );
        }
    }
}

/*
 * internal functions
 */
static void lcl_DelFormatIndices( SwFormat* pFormat )
{
    SwFormatContent &rFormatContent = (SwFormatContent&)pFormat->GetContent();
    if ( rFormatContent.GetContentIdx() )
        rFormatContent.SetNewContentIdx( nullptr );
    SwFormatAnchor &rFormatAnchor = (SwFormatAnchor&)pFormat->GetAnchor();
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
    m_pDocumentExternalDataManager( new ::sw::DocumentExternalDataManager() ),
    mpDfltFrameFormat( new SwFrameFormat( GetAttrPool(), sFrameFormatStr, nullptr ) ),
    mpEmptyPageFormat( new SwFrameFormat( GetAttrPool(), sEmptyPageStr, mpDfltFrameFormat ) ),
    mpColumnContFormat( new SwFrameFormat( GetAttrPool(), sColumnCntStr, mpDfltFrameFormat ) ),
    mpDfltCharFormat( new SwCharFormat( GetAttrPool(), sCharFormatStr, nullptr ) ),
    mpDfltTextFormatColl( new SwTextFormatColl( GetAttrPool(), sTextCollStr ) ),
    mpDfltGrfFormatColl( new SwGrfFormatColl( GetAttrPool(), sGrfCollStr ) ),
    mpFrameFormatTable( new SwFrameFormats() ),
    mpCharFormatTable( new SwCharFormats() ),
    mpSpzFrameFormatTable( new SwFrameFormats() ),
    mpSectionFormatTable( new SwSectionFormats() ),
    mpTableFrameFormatTable( new SwFrameFormats() ),
    mpTextFormatCollTable( new SwTextFormatColls() ),
    mpGrfFormatCollTable( new SwGrfFormatColls() ),
    mpTOXTypes( new SwTOXTypes() ),
    mpDefTOXBases( new SwDefTOXBase_Impl() ),
    mpOutlineRule( nullptr ),
    mpFootnoteInfo( new SwFootnoteInfo ),
    mpEndNoteInfo( new SwEndNoteInfo ),
    mpLineNumberInfo( new SwLineNumberInfo ),
    mpFootnoteIdxs( new SwFootnoteIdxs ),
    mpDocShell( nullptr ),
    mpACEWord( nullptr ),
    mpURLStateChgd( nullptr ),
    mpNumberFormatter( nullptr ),
    mpNumRuleTable( new SwNumRuleTable ),
    mpPgPViewPrtData( nullptr ),
    mpExtInputRing( nullptr ),
    mpStyleAccess( nullptr ),
    mpLayoutCache( nullptr ),
    mpGrammarContact(createGrammarContact()),
    mpTableStyles(new SwTableAutoFormatTable),
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
#ifdef DBG_UTIL
    mbXMLExport(false),
#endif
    mbContainsAtPageObjWithContentAnchor(false), //#i119292#, fdo#37024

    meDocType(DOCTYPE_NATIVE)
{
    //UUUU The DrawingLayer ItemPool which is used as 2nd pool for Writer documents' pool
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

    /*
     * DefaultFormats and DefaultFormatCollections (FormatColl)
     * are inserted at position 0 at the respective array.
     * The formats in the FormatColls are derived from the
     * DefaultFormats and are also in the list.
     */
    /* Formats */
    mpFrameFormatTable->push_back(mpDfltFrameFormat);
    mpCharFormatTable->push_back(mpDfltCharFormat);

    /* FormatColls */
    // TXT
    mpTextFormatCollTable->push_back(mpDfltTextFormatColl);
    // GRF
    mpGrfFormatCollTable->push_back(mpDfltGrfFormatColl);

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
            mpDfltTextFormatColl );
    new SwTextNode( SwNodeIndex( GetNodes().GetEndOfContent() ),
                    getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ));

    maOLEModifiedIdle.SetPriority( SchedulerPriority::LOWEST );
    maOLEModifiedIdle.SetIdleHdl( LINK( this, SwDoc, DoUpdateModifiedOLE ));

#if HAVE_FEATURE_DBCONNECTIVITY
    // Create DBManager
    mpDBManager = new SwDBManager(this);
#endif

    // create TOXTypes
    InitTOXTypes();

    // pass empty item set containing the paragraph's list attributes
    // as ignorable items to the stype manager.
    {
        SfxItemSet aIgnorableParagraphItems( GetAttrPool(), RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1);
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
        static rtlRandomPool aPool = rtl_random_createPool();
        rtl_random_getBytes( aPool, &mnRsid, sizeof ( mnRsid ) );
        mnRsid &= ( 1<<21 ) - 1;
        mnRsid++;
    }
    mnRsidRoot = mnRsid;

    mpTableStyles->Load();

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

    delete mpGrammarContact;
    mpGrammarContact = nullptr;

    //!! needs to be done to destroy a possible SwFormatDrop format that may
    //!! be connected to a char format which may not otherwise be removed
    //!! and thus would leave a unremoved SwFormat object. (TL)
    //!! (this is case is not possible via UI but via API...)
    SwFormatDrop aDrop;
    SetDefault(aDrop);
    //!! same for SwFormatCharFormat
    SwFormatCharFormat aCharFormat(nullptr);
    SetDefault(aCharFormat);

    getIDocumentTimerAccess().StopIdling();   // stop idle timer

    delete mpURLStateChgd;

    // Deactivate Undo notification from Draw
    if( GetDocumentDrawModelManager().GetDrawModel() )
    {
        GetDocumentDrawModelManager().DrawNotifyUndoHdl();
        ClrContourCache();
    }

    delete mpPgPViewPrtData;

    mbDtor = true;

    //Clear the redline table before the nodes array is destroyed
    getIDocumentRedlineAccess().GetRedlineTable().DeleteAndDestroyAll();
    getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteAndDestroyAll();

    const sw::DocDisposingHint aHint;
    cleanupUnoCursorTable();
    for(const auto& pWeakCursor : mvUnoCursorTable)
    {
        auto pCursor(pWeakCursor.lock());
        if(pCursor)
            pCursor->CallSwClientNotify(aHint);
    }
    delete mpACEWord;

    // Release the BaseLinks
    {
       ::sfx2::SvLinkSources aTemp(getIDocumentLinksAdministration().GetLinkManager().GetServers());
       for( ::sfx2::SvLinkSources::const_iterator it = aTemp.begin();
            it != aTemp.end(); ++it )
            (*it)->Closed();

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
    delete mpDefTOXBases;

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
    m_PageDescs.clear();

    // Delete content selections.
    // Don't wait for the SwNodes dtor to destroy them; so that Formats
    // do not have any dependencies anymore.
    m_pNodes->DelNodes( SwNodeIndex(*m_pNodes), m_pNodes->Count() );
    rUndoNodes.DelNodes( SwNodeIndex( rUndoNodes ), rUndoNodes.Count() );

    // Delete Formats, make it permanent some time in the future

    // Delete for Collections
    // So that we get rid of the dependencies
    mpFootnoteInfo->ReleaseCollection();
    mpEndNoteInfo->ReleaseCollection();

    OSL_ENSURE( mpDfltTextFormatColl == (*mpTextFormatCollTable)[0],
            "Default-Text-Collection must always be at the start" );

    // Optimization: Based on the fact that Standard is always 2nd in the
    // array, we should delete it as the last. With this we avoid
    // reparenting the Formats all the time!
    if( 2 < mpTextFormatCollTable->size() )
        mpTextFormatCollTable->DeleteAndDestroy(2, mpTextFormatCollTable->size());
    mpTextFormatCollTable->DeleteAndDestroy(1, mpTextFormatCollTable->size());
    delete mpTextFormatCollTable;

    OSL_ENSURE( mpDfltGrfFormatColl == (*mpGrfFormatCollTable)[0],
            "DefaultGrfCollection must always be at the start" );

    mpGrfFormatCollTable->DeleteAndDestroy(1, mpGrfFormatCollTable->size());
    delete mpGrfFormatCollTable;

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
    // If we have an embedded one, then sDataSoure points to the registered name, so revoke it here.
    if (!mpDBManager->getEmbeddedName().isEmpty() && !maDBData.sDataSource.isEmpty())
    {
        // Remove the revoke listener here first, so that we don't remove the data source from the document.
        mpDBManager->releaseRevokeListener();
        SwDBManager::RevokeDataSource(maDBData.sDataSource);
    }

    DELETEZ( mpDBManager );
#endif

    // All Flys need to be destroyed before the Drawing Model,
    // because Flys can still contain DrawContacts, when no
    // Layout could be constructed due to a read error.
    mpSpzFrameFormatTable->DeleteAndDestroy( 0, mpSpzFrameFormatTable->size() );

    // Only now destroy the Model, the drawing objects - which are also
    // contained in the Undo - need to remove their attributes from the
    // Model. Also, DrawContacts could exist before this.
    GetDocumentDrawModelManager().ReleaseDrawModel();
    // Destroy DrawModel before the LinkManager, because it's always set
    // in the DrawModel.
    //The LinkManager gets destroyed automatically with m_pLinksAdministrationManager

    // Clear the Tables before deleting the defaults, or we crash due to
    // dependencies on defaults.
    delete mpFrameFormatTable;
    delete mpSpzFrameFormatTable;

    delete mpStyleAccess;

    delete mpCharFormatTable;
    delete mpSectionFormatTable;
    delete mpTableFrameFormatTable;
    delete mpDfltTextFormatColl;
    delete mpDfltGrfFormatColl;
    delete mpNumRuleTable;

    disposeXForms(); // #i113606#, dispose the XForms objects

    delete mpNumberFormatter;
    delete mpFootnoteInfo;
    delete mpEndNoteInfo;
    delete mpLineNumberInfo;
    delete mpFootnoteIdxs;
    delete mpTOXTypes;
    delete mpEmptyPageFormat;
    delete mpColumnContFormat;
    delete mpDfltCharFormat;
    delete mpDfltFrameFormat;
    delete mpLayoutCache;

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
        }

        getIDocumentLinksAdministration().GetLinkManager().SetPersist( mpDocShell );

        // set DocShell pointer also on DrawModel
        InitDrawModelAndDocShell(mpDocShell, GetDocumentDrawModelManager().GetDrawModel());
        OSL_ENSURE(!GetDocumentDrawModelManager().GetDrawModel() ||
            GetDocumentDrawModelManager().GetDrawModel()->GetPersist() == GetPersist(),
            "draw model's persist is out of sync");
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
    OSL_ENSURE( !GetDocumentDrawModelManager().GetDrawModel() || !GetDocumentDrawModelManager().GetDrawModel()->GetPage(0)->GetObjCount(),
                "not all DrawObjects removed from the page" );

    getIDocumentRedlineAccess().GetRedlineTable().DeleteAndDestroyAll();
    getIDocumentRedlineAccess().GetExtraRedlineTable().DeleteAndDestroyAll();

    delete mpACEWord;

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    mpMarkManager->clearAllMarks();
    InitTOXTypes();

    // create a dummy pagedesc for the layout
    SwPageDesc* pDummyPgDsc = MakePageDesc("?DUMMY?");

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // create the first one over and over again (without attributes/style etc.
    SwTextNode* pFirstNd = GetNodes().MakeTextNode( aSttIdx, mpDfltTextFormatColl );

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
        delete pNumRule;
    mpNumRuleTable->clear();

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
        pDummyPgDsc = m_PageDescs[nDummyPgDsc].release();
    m_PageDescs.clear();

    // Delete for Collections
    // So that we get rid of the dependencies
    mpFootnoteInfo->ReleaseCollection();
    mpEndNoteInfo->ReleaseCollection();

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
        mpFrameFormatTable->erase( std::find( mpFrameFormatTable->begin(), mpFrameFormatTable->end(), getIDocumentLayoutAccess().GetCurrentViewShell()->GetLayout()->GetFormat() ) );
        mpFrameFormatTable->DeleteAndDestroy(1, mpFrameFormatTable->size());
        mpFrameFormatTable->push_back( getIDocumentLayoutAccess().GetCurrentViewShell()->GetLayout()->GetFormat() );
    }
    else
        mpFrameFormatTable->DeleteAndDestroy(1, mpFrameFormatTable->size());

    mxForbiddenCharsTable.clear();

    GetDocumentFieldsManager().ClearFieldTypes();

    delete mpNumberFormatter;
    mpNumberFormatter = nullptr;

    getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    pFirstNd->ChgFormatColl( getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ));
    nDummyPgDsc = m_PageDescs.size();
    m_PageDescs.push_back(std::unique_ptr<SwPageDesc>(pDummyPgDsc));
    // set the layout back to the new standard pagedesc
    pFirstNd->ResetAllAttr();
    // delete now the dummy pagedesc
    DelPageDesc( nDummyPgDsc );
}

void SwDoc::SetPreviewPrtData( const SwPagePreviewPrtData* pNew )
{
    if( pNew )
    {
        if( mpPgPViewPrtData )
            *mpPgPViewPrtData = *pNew;
        else
            mpPgPViewPrtData = new SwPagePreviewPrtData( *pNew );
    }
    else if( mpPgPViewPrtData )
        DELETEZ( mpPgPViewPrtData );
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
        mpLayoutCache = new SwLayoutCache();
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
    if (!m_pXmlIdRegistry.get())
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
    SwDoc* pRet = new SwDoc;

    // we have to use pointer here, since the callee has to decide whether
    // SfxObjectShellLock or SfxObjectShellRef should be used sometimes the
    // object will be returned with refcount set to 0 ( if no DoInitNew is done )
    SfxObjectShell* pRetShell = new SwDocShell( pRet, SfxObjectCreateMode::STANDARD );
    if( bCallInitNew )
    {
        // it could happen that DoInitNew creates model,
        // that increases the refcount of the object
        pRetShell->DoInitNew();
    }

    (void)pRet->acquire();

    pRet->ReplaceDefaults(*this);

    pRet->ReplaceCompatibilityOptions(*this);

    pRet->ReplaceStyles(*this);

    if( !bEmpty )
    {
#ifdef DBG_UTIL
        SAL_INFO( "sw.createcopy", "CC-Nd-Src: " << CNTNT_DOC( this ) );
        SAL_INFO( "sw.createcopy", "CC-Nd: " << CNTNT_DOC( pRet ) );
#endif
        pRet->AppendDoc(*this, 0, bCallInitNew, 0, 0);
#ifdef DBG_UTIL
        SAL_INFO( "sw.createcopy", "CC-Nd: " << CNTNT_DOC( pRet ) );
#endif
    }

    // remove the temporary shell if it is there as it was done before
    pRet->SetTmpDocShell( nullptr );

    (void)pRet->release();

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
    const SwPageDesc* pFollowPageDesc = rSourcePageDesc.GetFollow();
    OUString sFollowPageDesc = pFollowPageDesc->GetName();
    if( sFollowPageDesc != rSourcePageDesc.GetName() )
    {
        SwDoc* pTargetDoc = rTargetShell.GetDoc();
        OUString sNewFollowPageDesc = lcl_FindUniqueName(&rTargetShell, sFollowPageDesc, nDocNo );
        SwPageDesc* pTargetFollowPageDesc = pTargetDoc->MakePageDesc(sNewFollowPageDesc);

        pTargetDoc->CopyPageDesc(*pFollowPageDesc, *pTargetFollowPageDesc, false);
        SwPageDesc aDesc(rTargetPageDesc);
        aDesc.SetFollow(pTargetFollowPageDesc);
        pTargetDoc->ChgPageDesc(rTargetPageDesc.GetName(), aDesc);
    }
}

// appends all pages of source SwDoc - based on SwFEShell::Paste( SwDoc* )
SwNodeIndex SwDoc::AppendDoc(const SwDoc& rSource, sal_uInt16 const nStartPageNumber,
                             bool const bDeletePrevious, int pageOffset, const sal_uLong nDocNo)
{
    // GetEndOfExtras + 1 = StartOfContent == no content node!
    // this ensures, that we have at least two nodes in the SwPaM.
    // @see IDocumentContentOperations::CopyRange
    SwNodeIndex aSourceIdx( rSource.GetNodes().GetEndOfExtras(), 1 );
    SwNodeIndex aSourceEndIdx( rSource.GetNodes().GetEndOfContent(), -1 );
    SwPaM aCpyPam( aSourceIdx );

    if ( aSourceEndIdx.GetNode().IsTextNode() ) {
        aCpyPam.SetMark();
        // moves to the last content node before EOC; for single paragraph
        // documents this would result in [n, n], which is considered empty
        aCpyPam.Move( fnMoveForward, fnGoDoc );
    }
    else
        aCpyPam = SwPaM( aSourceIdx, aSourceEndIdx );

#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << (int) aSourceIdx.GetNode().GetNodeType()
                              << std::dec << " " << aSourceIdx.GetNode().GetIndex() );
    aSourceIdx++;
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << (int) aSourceIdx.GetNode().GetNodeType()
                                            << std::dec << " " << aSourceIdx.GetNode().GetIndex() );
    if ( aSourceIdx.GetNode().GetNodeType() != ND_ENDNODE ) {
        aSourceIdx++;
        SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << (int) aSourceIdx.GetNode().GetNodeType() << std::dec );
        aSourceIdx--;
    }
    aSourceIdx--;
    SAL_INFO( "sw.docappend", ".." );
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << (int) aSourceEndIdx.GetNode().GetNodeType()
                              << std::dec << " " << aSourceEndIdx.GetNode().GetIndex() );
    aSourceEndIdx++;
    SAL_INFO( "sw.docappend", "NodeType 0x" << std::hex << (int) aSourceEndIdx.GetNode().GetNodeType()
                              << std::dec << " " << aSourceEndIdx.GetNode().GetIndex() );
    aSourceEndIdx--;
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
                pTargetPageDesc = this->MakePageDesc( sNewPageDescName );
                if( pTargetPageDesc )
                {
                    this->CopyPageDesc( *pSourcePageDesc, *pTargetPageDesc, false );
                    lcl_CopyFollowPageDesc( *pTargetShell, *pSourcePageDesc, *pTargetPageDesc, nDocNo );
                }
            }
            else
                pTargetPageDesc = pTargetShell->FindPageDescByName( sStartingPageDesc );
        }

        // Otherwise we have to handle SwPlaceholderNodes as first node
        if ( pTargetPageDesc ) {
            OUString name = pTargetPageDesc->GetName();
            pTargetShell->InsertPageBreak( &name, nStartPageNumber );

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
                              << " (0x" << std::hex << (int) aCpyPam.GetNode( false ).GetNodeType() << std::dec
                              << " " << aCpyPam.GetNode( false ).GetIndex()
                              << " - 0x" << std::hex << (int) aCpyPam.GetNode().GetNodeType() << std::dec
                              << " " << aCpyPam.GetNode().GetIndex() << ")" );
#endif

    this->GetIDocumentUndoRedo().StartUndo( UNDO_INSGLOSSARY, nullptr );
    this->getIDocumentFieldsAccess().LockExpFields();

    // Position where the appended doc starts. Will be filled in later (uses GetEndOfContent() because SwNodeIndex has no default ctor).
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
                SfxPoolItem *pNewItem;
                SwTextNode *aTextNd = nullptr;
                SwFormat *pFormat = nullptr;

                // find the first node allowed to contain a RES_PAGEDESC
                while (true) {
                    aFixupIdx++;

                    SwNode &node = aFixupIdx.GetNode();
                    if ( node.IsTextNode() ) {
                        // every document contains at least one text node!
                        aTextNd = node.GetTextNode();
                        pNewItem = aTextNd->GetAttr( RES_PAGEDESC ).Clone();
                        break;
                    }
                    else if ( node.IsTableNode() ) {
                        pFormat = node.GetTableNode()->GetTable().GetFrameFormat();
                        pNewItem = pFormat->GetFormatAttr( RES_PAGEDESC ).Clone();
                        break;
                    }
                }

#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "Idx Del " << CNTNT_IDX( aDelIdx ) );
    SAL_INFO( "sw.docappend", "Idx Fix " << CNTNT_IDX( aFixupIdx ) );
#endif
                // just update the original instead of overwriting
                SwFormatPageDesc *aDesc = static_cast< SwFormatPageDesc* >( pNewItem );
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
                delete pNewItem;

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
                aDelIdx -= (iDelNodes - 1);
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
            if (FLY_AT_PAGE != aAnchor.GetAnchorId())
                continue;
#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "PaAn: " << aAnchor.GetPageNum()
                              << " => " << aAnchor.GetPageNum() + pageOffset );
#endif
            if ( pageOffset != 0 )
                aAnchor.SetPageNum( aAnchor.GetPageNum() + pageOffset );
            this->getIDocumentLayoutAccess().CopyLayoutFormat( rCpyFormat, aAnchor, true, true );
        }
    }

    this->GetIDocumentUndoRedo().EndUndo( UNDO_INSGLOSSARY, nullptr );

    getIDocumentFieldsAccess().UnlockExpFields();
    getIDocumentFieldsAccess().UpdateFields(false);

    if ( pTargetShell )
        pTargetShell->EndAllAction();

    return aStartAppendIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
