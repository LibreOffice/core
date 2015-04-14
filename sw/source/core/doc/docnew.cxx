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
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/linguistic2/ProofreadingIterator.hpp>
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

#include <cmdid.h>

#include <pausethreadstarting.hxx>
#include <numrule.hxx>
#include <list.hxx>

#include <sfx2/Metadatable.hxx>
#include <fmtmeta.hxx>
#include <boost/foreach.hpp>

//UUUU
#include <svx/xfillit0.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;

const sal_Char sFrmFmtStr[] = "Frameformat";
const sal_Char sEmptyPageStr[] = "Empty Page";
const sal_Char sColumnCntStr[] = "Columncontainer";
const sal_Char sCharFmtStr[] = "Character style";
const sal_Char sTxtCollStr[] = "Paragraph style";
const sal_Char sGrfCollStr[] = "Graphikformatvorlage";

/*
 * global functions...
 */
 uno::Reference< linguistic2::XProofreadingIterator > SwDoc::GetGCIterator() const
{
    if (!m_xGCIterator.is() && SvtLinguConfig().HasGrammarChecker())
    {
        uno::Reference< uno::XComponentContext >  xContext( comphelper::getProcessComponentContext() );
        try
        {
            m_xGCIterator = linguistic2::ProofreadingIterator::create( xContext );
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
    const SwDocShell *mpDocShell = rDoc.GetDocShell();
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( mpDocShell, false );
    while (pFrame && !bVisible)
    {
        if (pFrame->IsVisible())
            bVisible = true;
        pFrame = SfxViewFrame::GetNext( *pFrame, mpDocShell, false );
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
static void lcl_DelFmtIndices( SwFmt* pFmt )
{
    SwFmtCntnt &rFmtCntnt = (SwFmtCntnt&)pFmt->GetCntnt();
    if ( rFmtCntnt.GetCntntIdx() )
        rFmtCntnt.SetNewCntntIdx( 0 );
    SwFmtAnchor &rFmtAnchor = (SwFmtAnchor&)pFmt->GetAnchor();
    if ( rFmtAnchor.GetCntntAnchor() )
        rFmtAnchor.SetAnchor( 0 );
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
            boost::shared_ptr<SwNodes>(new SwNodes(this)), *m_pDocumentDrawModelManager, *m_pDocumentRedlineManager, *m_pDocumentStateManager)),
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
    mpDfltFrmFmt( new SwFrmFmt( GetAttrPool(), sFrmFmtStr, 0 ) ),
    mpEmptyPageFmt( new SwFrmFmt( GetAttrPool(), sEmptyPageStr, mpDfltFrmFmt ) ),
    mpColumnContFmt( new SwFrmFmt( GetAttrPool(), sColumnCntStr, mpDfltFrmFmt ) ),
    mpDfltCharFmt( new SwCharFmt( GetAttrPool(), sCharFmtStr, 0 ) ),
    mpDfltTxtFmtColl( new SwTxtFmtColl( GetAttrPool(), sTxtCollStr ) ),
    mpDfltGrfFmtColl( new SwGrfFmtColl( GetAttrPool(), sGrfCollStr ) ),
    mpFrmFmtTbl( new SwFrmFmts() ),
    mpFrmFmtAnchorMap( new SwFrmFmtAnchorMap( this ) ),
    mpCharFmtTbl( new SwCharFmts() ),
    mpSpzFrmFmtTbl( new SwFrmFmts() ),
    mpSectionFmtTbl( new SwSectionFmts() ),
    mpTblFrmFmtTbl( new SwFrmFmts() ),
    mpTxtFmtCollTbl( new SwTxtFmtColls() ),
    mpGrfFmtCollTbl( new SwGrfFmtColls() ),
    mpTOXTypes( new SwTOXTypes() ),
    mpDefTOXBases( new SwDefTOXBase_Impl() ),
    mpGlossaryDoc( 0 ),
    mpOutlineRule( 0 ),
    mpFtnInfo( new SwFtnInfo ),
    mpEndNoteInfo( new SwEndNoteInfo ),
    mpLineNumberInfo( new SwLineNumberInfo ),
    mpFtnIdxs( new SwFtnIdxs ),
    mpDocShell( 0 ),
    mpACEWord( 0 ),
    mpURLStateChgd( 0 ),
    mpNumberFormatter( 0 ),
    mpNumRuleTbl( new SwNumRuleTbl ),
    mpUnoCrsrTbl( new SwUnoCrsrTbl() ),
    mpPgPViewPrtData( 0 ),
    mpExtInputRing( 0 ),
    mpStyleAccess( 0 ),
    mpLayoutCache( 0 ),
    mpGrammarContact(createGrammarContact()),
    m_pXmlIdRegistry(),
    mReferenceCount(0),
    mbGlossDoc(false),
    mbDtor(false),
    mbCopyIsMove(false),
    mbInReading(false),
    mbInMailMerge(false),
    mbInXMLImport(false),
    mbUpdateTOX(false),
    mbInLoadAsynchron(false),
    mbIsAutoFmtRedline(false),
    mbOLEPrtNotifyPending(false),
    mbAllOLENotify(false),
    mbInsOnlyTxtGlssry(false),
    mbContains_MSVBasic(false),
    mbClipBoard( false ),
    mbColumnSelection( false ),
    mbIsPrepareSelAll(false),
#ifdef DBG_UTIL
    mbXMLExport(false),
#endif
    mbContainsAtPageObjWithContentAnchor(false), //#i119292#, fdo#37024

    mbReadOnly(false),
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
    // attribbute sets (which is done at import and using UI). Also not a good solution.
    // Luckily Writer uses pDfltTxtFmtColl as default parent for all paragraphs and similar, thus
    // it is possible to set this attribute here. It will be not reset when importing.
    mpDfltTxtFmtColl->SetFmtAttr(XFillStyleItem(drawing::FillStyle_NONE));
    mpDfltFrmFmt->SetFmtAttr(XFillStyleItem(drawing::FillStyle_NONE));

    /*
     * DefaultFormats and DefaultFormatCollections (FmtColl)
     * are inserted at position 0 at the respective array.
     * The formats in the FmtColls are derived from the
     * DefaultFormats and are also in the list.
     */
    /* Formats */
    mpFrmFmtTbl->push_back(mpDfltFrmFmt);
    mpCharFmtTbl->push_back(mpDfltCharFmt);

    /* FmtColls */
    // TXT
    mpTxtFmtCollTbl->push_back(mpDfltTxtFmtColl);
    // GRF
    mpGrfFmtCollTbl->push_back(mpDfltGrfFmtColl);

    // Create PageDesc, EmptyPageFmt and ColumnFmt
    if ( maPageDescs.empty() )
        getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD );

    // Set to "Empty Page"
    mpEmptyPageFmt->SetFmtAttr( SwFmtFrmSize( ATT_FIX_SIZE ) );
    // Set BodyFmt for columns
    mpColumnContFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );

    GetDocumentFieldsManager()._InitFieldTypes();

    // Create a default OutlineNumRule (for Filters)
    mpOutlineRule = new SwNumRule( SwNumRule::GetOutlineRuleName(),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    mpOutlineRule->SetCountPhantoms( !GetDocumentSettingManager().get(IDocumentSettingAccess::OLD_NUMBERING) );

    new SwTxtNode(
            SwNodeIndex(GetUndoManager().GetUndoNodes().GetEndOfContent()),
            mpDfltTxtFmtColl );
    new SwTxtNode( SwNodeIndex( GetNodes().GetEndOfContent() ),
                    getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    maOLEModifiedTimer.SetTimeout( 1000 );
    maOLEModifiedTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateModifiedOLE ));

#if HAVE_FEATURE_DBCONNECTIVITY
    // Create DBManager
    mpDBManager = new SwDBManager;
#endif

    // create TOXTypes
    InitTOXTypes();

    // pass empty item set containing the paragraph's list attributes
    // as ignorable items to the stype manager.
    {
        SfxItemSet aIgnorableParagraphItems( GetAttrPool(), RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1);
        mpStyleAccess = createStyleManager( &aIgnorableParagraphItems );
    }

    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != NULL);

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

    getIDocumentState().ResetModified();
}

static void DeleteAndDestroy(SwFrmFmts& rFmts, int aStartIdx, int aEndIdx)
{
    if (aEndIdx < aStartIdx)
        return;
    for( SwFrmFmts::const_iterator it = rFmts.begin() + aStartIdx;
         it != rFmts.begin() + aEndIdx; ++it )
             delete *it;
    rFmts.erase( rFmts.begin() + aStartIdx, rFmts.begin() + aEndIdx);
}

static void DeleteAndDestroy(SwTxtFmtColls& rFmts, int aStartIdx, int aEndIdx)
{
    if (aEndIdx < aStartIdx)
        return;
    for( SwTxtFmtColls::const_iterator it = rFmts.begin() + aStartIdx;
         it != rFmts.begin() + aEndIdx; ++it )
             delete *it;
    rFmts.erase( rFmts.begin() + aStartIdx, rFmts.begin() + aEndIdx);
}

static void DeleteAndDestroy(SwCharFmts& rFmts, int aStartIdx, int aEndIdx)
{
    if (aEndIdx < aStartIdx)
        return;
    for( SwCharFmts::const_iterator it = rFmts.begin() + aStartIdx;
         it != rFmts.begin() + aEndIdx; ++it )
             delete *it;
    rFmts.erase( rFmts.begin() + aStartIdx, rFmts.begin() + aEndIdx);
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
        mpDocShell->SetUndoManager(0);
    }

    delete mpGrammarContact;
    mpGrammarContact = 0;

    //!! needs to be done to destroy a possible SwFmtDrop format that may
    //!! be connected to a char format which may not otherwise be removed
    //!! and thus would leave a unremoved SwFmt object. (TL)
    //!! (this is case is not possible via UI but via API...)
    SwFmtDrop aDrop;
    SetDefault(aDrop);
    //!! same for SwFmtCharFmt
    SwFmtCharFmt aCharFmt(NULL);
    SetDefault(aCharFmt);

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
    getIDocumentRedlineAccess().GetRedlineTbl().DeleteAndDestroyAll();
    getIDocumentRedlineAccess().GetExtraRedlineTbl().DeleteAndDestroyAll();

    delete mpUnoCrsrTbl;
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
    m_pNodes->pOutlineNds->clear();
    SwNodes & rUndoNodes( GetUndoManager().GetUndoNodes() );
    rUndoNodes.pOutlineNds->clear();

    mpFtnIdxs->clear();

    // indices could be registered in attributes
    m_pUndoManager->DelAllUndoObj();

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    mpMarkManager->clearAllMarks();

    if( mpExtInputRing )
    {
        Ring* pTmp = mpExtInputRing;
        mpExtInputRing = 0;
        while( pTmp->GetNext() != pTmp )
            delete pTmp->GetNext();
        delete pTmp;
    }

    // Old - deletion without a Flag is expensive, because we send a Modify
    // aTOXTypes.DeleteAndDestroy( 0, aTOXTypes.Count() );
    {
        for( sal_uInt16 n = mpTOXTypes->size(); n; )
        {
            (*mpTOXTypes)[ --n ]->SetInDocDTOR();
            delete (*mpTOXTypes)[ n ];
        }
        mpTOXTypes->clear();
    }
    delete mpDefTOXBases;

    // Any of the FrmFormats can still have indices registered.
    // These need to be destroyed now at the latest.
    BOOST_FOREACH( SwFrmFmt* pFmt, *mpFrmFmtTbl )
        lcl_DelFmtIndices( pFmt );
    BOOST_FOREACH( SwFrmFmt* pFmt, *mpSpzFrmFmtTbl )
        lcl_DelFmtIndices( pFmt );
    BOOST_FOREACH( SwSectionFmt* pFmt, *mpSectionFmtTbl )
        lcl_DelFmtIndices( pFmt );

    // The formats/styles that follow depend on the default formats.
    // Destroy these only after destroying the FmtIndices, because the content
    // of headers/footers has to be deleted as well. If in the headers/footers
    // there are still Flys registered at that point, we have a problem.
    BOOST_FOREACH(SwPageDesc *pPageDesc, maPageDescs)
        delete pPageDesc;
    maPageDescs.clear();

    // Delete content selections.
    // Don't wait for the SwNodes dtor to destroy them; so that Formats
    // do not have any dependencies anymore.
    m_pNodes->DelNodes( SwNodeIndex(*m_pNodes), m_pNodes->Count() );
    rUndoNodes.DelNodes( SwNodeIndex( rUndoNodes ), rUndoNodes.Count() );

    // Delete Formats, make it permanent some time in the future

    // Delete for Collections
    // So that we get rid of the dependencies
    mpFtnInfo->ReleaseCollection();
    mpEndNoteInfo->ReleaseCollection();

    OSL_ENSURE( mpDfltTxtFmtColl == (*mpTxtFmtCollTbl)[0],
            "Default-Text-Collection must always be at the start" );

    // Optimization: Based on the fact that Standard is always 2nd in the
    // array, we should delete it as the last. With this we avoid
    // reparenting the Formats all the time!
    if( 2 < mpTxtFmtCollTbl->size() )
        DeleteAndDestroy(*mpTxtFmtCollTbl, 2, mpTxtFmtCollTbl->size());
    DeleteAndDestroy(*mpTxtFmtCollTbl, 1, mpTxtFmtCollTbl->size());
    delete mpTxtFmtCollTbl;

    OSL_ENSURE( mpDfltGrfFmtColl == (*mpGrfFmtCollTbl)[0],
            "DefaultGrfCollection must always be at the start" );

    mpGrfFmtCollTbl->DeleteAndDestroy(1, mpGrfFmtCollTbl->size());
    delete mpGrfFmtCollTbl;

    // Without explicitly freeing the DocumentDeviceManager
    // and relying on the implicit freeing there would be a crash
    // due to it happening after SwAttrPool is freed.
    m_pDeviceAccess.reset();

    /*
     * DefaultFormats and DefaultFormatCollections (FmtColl)
     * are at position 0 of their respective arrays.
     * In order to not be deleted by the array's dtor, we remove them
     * now.
     */
    mpFrmFmtTbl->erase( mpFrmFmtTbl->begin() );
    mpCharFmtTbl->erase( mpCharFmtTbl->begin() );

#if HAVE_FEATURE_DBCONNECTIVITY
    DELETEZ( mpDBManager );
#endif

    // All Flys need to be destroyed before the Drawing Model,
    // because Flys can still contain DrawContacts, when no
    // Layout could be constructed due to a read error.
    DeleteAndDestroy( *mpSpzFrmFmtTbl, 0, mpSpzFrmFmtTbl->size() );

    // Only now destroy the Model, the drawing objects - which are also
    // contained in the Undo - need to remove their attributes from the
    // Model. Also, DrawContacts could exist before this.
    GetDocumentDrawModelManager().ReleaseDrawModel();
    // Destroy DrawModel before the LinkManager, because it's always set
    // in the DrawModel.
    //The LinkManager gets destroyed automatically with m_pLinksAdministrationManager

    // Clear the Tables before deleting the defaults, or we crash due to
    // dependencies on defaults.
    delete mpFrmFmtTbl;
    delete mpSpzFrmFmtTbl;

    delete mpStyleAccess;

    delete mpCharFmtTbl;
    delete mpSectionFmtTbl;
    delete mpTblFrmFmtTbl;
    delete mpDfltTxtFmtColl;
    delete mpDfltGrfFmtColl;
    delete mpNumRuleTbl;

    disposeXForms(); // #i113606#, dispose the XForms objects

    delete mpNumberFormatter;
    delete mpFtnInfo;
    delete mpEndNoteInfo;
    delete mpLineNumberInfo;
    delete mpFtnIdxs;
    delete mpTOXTypes;
    delete mpEmptyPageFmt;
    delete mpColumnContFmt;
    delete mpDfltCharFmt;
    delete mpDfltFrmFmt;
    delete mpLayoutCache;
    delete mpFrmFmtAnchorMap;

    SfxItemPool::Free(mpAttrPool);
}

void SwDoc::SetDocShell( SwDocShell* pDSh )
{
    if( mpDocShell != pDSh )
    {
        if (mpDocShell)
        {
            mpDocShell->SetUndoManager(0);
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
    return NULL;
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
    sal_uInt16 n;
    while ( 0 != (n = GetSpzFrmFmts()->size()) )
        getIDocumentLayoutAccess().DelLayoutFmt((*mpSpzFrmFmtTbl)[n-1]);
    OSL_ENSURE( !GetDocumentDrawModelManager().GetDrawModel() || !GetDocumentDrawModelManager().GetDrawModel()->GetPage(0)->GetObjCount(),
                "not all DrawObjects removed from the page" );

    getIDocumentRedlineAccess().GetRedlineTbl().DeleteAndDestroyAll();
    getIDocumentRedlineAccess().GetExtraRedlineTbl().DeleteAndDestroyAll();

    delete mpACEWord;

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    mpMarkManager->clearAllMarks();
    InitTOXTypes();

    // create a dummy pagedesc for the layout
    SwPageDesc* pDummyPgDsc = MakePageDesc("?DUMMY?");

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // create the first one over and over again (without attributes/style etc.
    SwTxtNode* pFirstNd = GetNodes().MakeTxtNode( aSttIdx, mpDfltTxtFmtColl );

    if( getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        // set the layout to the dummy pagedesc
        pFirstNd->SetAttr( SwFmtPageDesc( pDummyPgDsc ));

        SwPosition aPos( *pFirstNd, SwIndex( pFirstNd ));
        SwPaM const tmpPaM(aSttIdx, SwNodeIndex(GetNodes().GetEndOfContent()));
        ::PaMCorrAbs(tmpPaM, aPos);
    }

    GetNodes().Delete( aSttIdx,
            GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );

    // #i62440#
    // destruction of numbering rules and creation of new outline rule
    // *after* the document nodes are deleted.
    mpOutlineRule = NULL;
    BOOST_FOREACH( SwNumRule* pNumRule, *mpNumRuleTbl )
        delete pNumRule;
    mpNumRuleTbl->clear();

    // creation of new outline numbering rule
    mpOutlineRule = new SwNumRule( SwNumRule::GetOutlineRuleName(),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    mpOutlineRule->SetCountPhantoms( !GetDocumentSettingManager().get(IDocumentSettingAccess::OLD_NUMBERING) );

    // remove the dummy pagedesc from the array and delete all the old ones
    sal_uInt16 nDummyPgDsc = 0;
    if (FindPageDesc(pDummyPgDsc->GetName(), &nDummyPgDsc))
        maPageDescs.erase(maPageDescs.begin() + nDummyPgDsc);

    BOOST_FOREACH(SwPageDesc *pPageDesc, maPageDescs)
        delete pPageDesc;
    maPageDescs.clear();

    // Delete for Collections
    // So that we get rid of the dependencies
    mpFtnInfo->ReleaseCollection();
    mpEndNoteInfo->ReleaseCollection();

    // Optimization: Based on the fact that Standard is always 2nd in the
    // array, we should delete it as the last. With this we avoid
    // reparenting the Formats all the time!
    if( 2 < mpTxtFmtCollTbl->size() )
        DeleteAndDestroy(*mpTxtFmtCollTbl, 2, mpTxtFmtCollTbl->size());
    DeleteAndDestroy(*mpTxtFmtCollTbl, 1, mpTxtFmtCollTbl->size());
    mpGrfFmtCollTbl->DeleteAndDestroy(1, mpGrfFmtCollTbl->size());
    DeleteAndDestroy(*mpCharFmtTbl, 1, mpCharFmtTbl->size());

    if( getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        // search the FrameFormat of the root frm. This is not allowed to delete
        mpFrmFmtTbl->erase( std::find( mpFrmFmtTbl->begin(), mpFrmFmtTbl->end(), getIDocumentLayoutAccess().GetCurrentViewShell()->GetLayout()->GetFmt() ) );
        DeleteAndDestroy(*mpFrmFmtTbl, 1, mpFrmFmtTbl->size());
        mpFrmFmtTbl->push_back( getIDocumentLayoutAccess().GetCurrentViewShell()->GetLayout()->GetFmt() );
    }
    else
        DeleteAndDestroy(*mpFrmFmtTbl, 1, mpFrmFmtTbl->size());

    mxForbiddenCharsTable.clear();

    GetDocumentFieldsManager().ClearFieldTypes();

    delete mpNumberFormatter, mpNumberFormatter = 0;

    getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    pFirstNd->ChgFmtColl( getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    nDummyPgDsc = maPageDescs.size();
    maPageDescs.push_back( pDummyPgDsc );
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
    if( getIDocumentLayoutAccess().GetCurrentViewShell() ) maOLEModifiedTimer.Start();
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
    mpLayoutCache->Write( rStream, *this );
}

IGrammarContact* getGrammarContact( const SwTxtNode& rTxtNode )
{
    const SwDoc* pDoc = rTxtNode.GetDoc();
    if( !pDoc || pDoc->IsInDtor() )
        return 0;
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

    sal_uInt16 nRange = 0;
    while (aRangeOfDefaults[nRange] != 0)
    {
        for (sal_uInt16 nWhich = aRangeOfDefaults[nRange];
             nWhich <= aRangeOfDefaults[nRange + 1]; ++nWhich)
        {
            const SfxPoolItem& rSourceAttr =
                rSource.mpAttrPool->GetDefaultItem(nWhich);
            if (rSourceAttr != mpAttrPool->GetDefaultItem(nWhich))
                aNewDefaults.Put(rSourceAttr);
        }
        nRange += 2;
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

SfxObjectShell* SwDoc::CreateCopy(bool bCallInitNew ) const
{
    SwDoc* pRet = new SwDoc;

    // we have to use pointer here, since the callee has to decide whether
    // SfxObjectShellLock or SfxObjectShellRef should be used sometimes the
    // object will be returned with refcount set to 0 ( if no DoInitNew is done )
    SfxObjectShell* pRetShell = new SwDocShell( pRet, SFX_CREATE_MODE_STANDARD );
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

#ifdef DBG_UTIL
    SAL_INFO( "sw.createcopy", "CC-Nd-Src: " << CNTNT_DOC( this ) );
    SAL_INFO( "sw.createcopy", "CC-Nd: " << CNTNT_DOC( pRet ) );
#endif
    pRet->AppendDoc(*this, 0, NULL, bCallInitNew);
#ifdef DBG_UTIL
    SAL_INFO( "sw.createcopy", "CC-Nd: " << CNTNT_DOC( pRet ) );
#endif

    // remove the temporary shell if it is there as it was done before
    pRet->SetTmpDocShell( (SfxObjectShell*)NULL );

    (void)pRet->release();

    return pRetShell;
}

// appends all pages of source SwDoc - based on SwFEShell::Paste( SwDoc* )
SwNodeIndex SwDoc::AppendDoc(const SwDoc& rSource, sal_uInt16 const nStartPageNumber,
            SwPageDesc *const pTargetPageDesc, bool const bDeletePrevious, int pageOffset)
{
    // GetEndOfExtras + 1 = StartOfContent == no content node!
    // @see IDocumentContentOperations::CopyRange
    SwNodeIndex aSourceIdx( rSource.GetNodes().GetEndOfExtras(), 1 );
    SwNodeIndex aSourceEndIdx( rSource.GetNodes().GetEndOfContent(), -1 );
    SwPaM aCpyPam( aSourceIdx );

    if ( aSourceEndIdx.GetNode().IsTxtNode() ) {
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
    if ( pTargetShell ) {
#ifdef DBG_UTIL
        SAL_INFO( "sw.docappend", "Has target write shell" );
#endif
        pTargetShell->StartAllAction();

        // Otherwise we have to handle SwDummySectionNodes as first node
        if ( pTargetPageDesc ) {
            OUString name = pTargetPageDesc->GetName();
            pTargetShell->InsertPageBreak( &name, nStartPageNumber );

            // There is now a new empty text node on the new page. If it has
            // any marks, those are from the previous page: move them back
            // there, otherwise later we can't delete that empty text node.
            SwNodeIndex aNodeIndex(GetNodes().GetEndOfContent(), -1);
            if (SwTxtNode* pTxtNode = aNodeIndex.GetNode().GetTxtNode())
            {
                // Position of the last paragraph on the previous page.
                --aNodeIndex;
                SwPaM aPaM(aNodeIndex);
                // Collect the marks starting or ending at this text node.
                std::set<sw::mark::IMark*> aSeenMarks;
                IDocumentMarkAccess* pMarkAccess = getIDocumentMarkAccess();
                for (const SwIndex* pIndex = pTxtNode->GetFirstIndex(); pIndex; pIndex = pIndex->GetNext())
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
    SAL_INFO( "sw.docappend", "Pam-Nd: " << aCpyPam.GetNode( true ).GetIndex() - aCpyPam.GetNode( false ).GetIndex() + 1
                              << " (0x" << std::hex << (int) aCpyPam.GetNode( false ).GetNodeType() << std::dec
                              << " " << aCpyPam.GetNode( false ).GetIndex()
                              << " - 0x" << std::hex << (int) aCpyPam.GetNode( true ).GetNodeType() << std::dec
                              << " " << aCpyPam.GetNode( true ).GetIndex() << ")" );
#endif

    this->GetIDocumentUndoRedo().StartUndo( UNDO_INSGLOSSARY, NULL );
    this->getIDocumentFieldsAccess().LockExpFlds();

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
            rSource.getIDocumentContentOperations().CopyRange( aCpyPam, rInsPos, true );
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
                SwTxtNode *const pTxtNode = rDestNodes[nIdx]->GetTxtNode();
                if ( pTxtNode )
                    UpdateParRsid( pTxtNode );
            }
        }

        {
            sal_uInt16 iDelNodes = 0;
            SwNodeIndex aDelIdx( aFixupIdx );

            // we just need to set the new page description and reset numbering
            // this keeps all other settings as in the pasted document
            if ( nStartPageNumber || pTargetPageDesc ) {
                SfxPoolItem *pNewItem;
                SwTxtNode *aTxtNd = 0;
                SwFmt *pFmt = 0;

                // find the first node allowed to contain a RES_PAGEDESC
                while (true) {
                    aFixupIdx++;

                    SwNode &node = aFixupIdx.GetNode();
                    if ( node.IsTxtNode() ) {
                        // every document contains at least one text node!
                        aTxtNd = node.GetTxtNode();
                        pNewItem = aTxtNd->GetAttr( RES_PAGEDESC ).Clone();
                        break;
                    }
                    else if ( node.IsTableNode() ) {
                        pFmt = node.GetTableNode()->GetTable().GetFrmFmt();
                        pNewItem = pFmt->GetFmtAttr( RES_PAGEDESC ).Clone();
                        break;
                    }
                }

#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "Idx Del " << CNTNT_IDX( aDelIdx ) );
    SAL_INFO( "sw.docappend", "Idx Fix " << CNTNT_IDX( aFixupIdx ) );
#endif
                // just update the original instead of overwriting
                SwFmtPageDesc *aDesc = static_cast< SwFmtPageDesc* >( pNewItem );
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
                if ( aTxtNd )
                    aTxtNd->SetAttr( *aDesc );
                else
                    pFmt->SetFmtAttr( *aDesc );
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
        const SwFrmFmts *pSpzFrmFmts = rSource.GetSpzFrmFmts();
        for ( sal_uInt16 i = 0; i < pSpzFrmFmts->size(); ++i )
        {
            const SwFrmFmt& rCpyFmt = *(*pSpzFrmFmts)[i];
            SwFmtAnchor aAnchor( rCpyFmt.GetAnchor() );
            if (FLY_AT_PAGE != aAnchor.GetAnchorId())
                continue;
#ifdef DBG_UTIL
    SAL_INFO( "sw.docappend", "PaAn: " << aAnchor.GetPageNum()
                              << " => " << aAnchor.GetPageNum() + pageOffset );
#endif
            if ( pageOffset != 0 )
                aAnchor.SetPageNum( aAnchor.GetPageNum() + pageOffset );
            this->getIDocumentLayoutAccess().CopyLayoutFmt( rCpyFmt, aAnchor, true, true );
        }
    }

    this->GetIDocumentUndoRedo().EndUndo( UNDO_INSGLOSSARY, NULL );

    getIDocumentFieldsAccess().UnlockExpFlds();
    getIDocumentFieldsAccess().UpdateFlds(NULL, false);

    if ( pTargetShell )
        pTargetShell->EndAllAction();

    return aStartAppendIndex;
}

sal_uInt16 SwTxtFmtColls::GetPos(const SwTxtFmtColl* p) const
{
    const_iterator it = std::find(begin(), end(), p);
    return it == end() ? USHRT_MAX : it - begin();
}

void SwGrfFmtColls::DeleteAndDestroy(int nStartIdx, int nEndIdx)
{
    if (nEndIdx < nStartIdx)
        return;
    for( std::vector<SwGrfFmtColl*>::const_iterator it = mvColls.begin() + nStartIdx;
         it != mvColls.begin() + nEndIdx; ++it )
             delete *it;
    mvColls.erase( mvColls.begin() + nStartIdx, mvColls.begin() + nEndIdx);
}

sal_uInt16 SwGrfFmtColls::GetPos(const SwGrfFmtColl* p) const
{
    std::vector<SwGrfFmtColl*>::const_iterator it = std::find(mvColls.begin(), mvColls.end(), p);
    return it == mvColls.end() ? USHRT_MAX : it - mvColls.begin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
