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
#include <unotools/compatibility.hxx>
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
#include <rootfrm.hxx>  // So that the RootDtor is being called
#include <layouter.hxx>
#include <pagedesc.hxx> // So that the PageDescs can be destroyed
#include <ndtxt.hxx>
#include <printdata.hxx>
#include <docfld.hxx>
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <docstat.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <rolbck.hxx>           // Undo attributes, SwHistory
#include <poolfmt.hxx>          // for the Pool template
#include <dbmgr.hxx>
#include <docsh.hxx>
#include <acorrect.hxx>         // for the automatic adding of exceptions
#include <visiturl.hxx>         // for the URLChange message
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
#include <unochart.hxx>
#include <fldbas.hxx>

#include <cmdid.h>              // for the default printer in SetJob

#include <pausethreadstarting.hxx>
#include <numrule.hxx>
#include <list.hxx>
#include <listfunc.hxx>

#include <sfx2/Metadatable.hxx>
#include <fmtmeta.hxx> // MetaFieldManager
#include <boost/foreach.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;

const sal_Char sFrmFmtStr[] = "Frameformat";
const sal_Char sEmptyPageStr[] = "Empty Page";
const sal_Char sColumnCntStr[] = "Columncontainer";
const sal_Char sCharFmtStr[] = "Zeichenformat";
const sal_Char sTxtCollStr[] = "Textformatvorlage";
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
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( mpDocShell, sal_False );
    while (pFrame && !bVisible)
    {
        if (pFrame->IsVisible())
            bVisible = true;
        pFrame = SfxViewFrame::GetNext( *pFrame, mpDocShell, sal_False );
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
static void lcl_DelFmtIndizes( SwFmt* pFmt )
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
    : m_pNodes( new SwNodes(this) )
    ,
    mpAttrPool(new SwAttrPool(this)),
    mpMarkManager(new ::sw::mark::MarkManager(*this)),
    m_pMetaFieldManager(new ::sw::MetaFieldManager()),
    m_pUndoManager(new ::sw::UndoManager(
            ::std::auto_ptr<SwNodes>(new SwNodes(this)), *this, *this, *this)),
    mpDfltFrmFmt( new SwFrmFmt( GetAttrPool(), sFrmFmtStr, 0 ) ),
    mpEmptyPageFmt( new SwFrmFmt( GetAttrPool(), sEmptyPageStr, mpDfltFrmFmt ) ),
    mpColumnContFmt( new SwFrmFmt( GetAttrPool(), sColumnCntStr, mpDfltFrmFmt ) ),
    mpDfltCharFmt( new SwCharFmt( GetAttrPool(), sCharFmtStr, 0 ) ),
    mpDfltTxtFmtColl( new SwTxtFmtColl( GetAttrPool(), sTxtCollStr ) ),
    mpDfltGrfFmtColl( new SwGrfFmtColl( GetAttrPool(), sGrfCollStr ) ),
    mpFrmFmtTbl( new SwFrmFmts() ),
    mpCharFmtTbl( new SwCharFmts() ),
    mpSpzFrmFmtTbl( new SwFrmFmts() ),
    mpSectionFmtTbl( new SwSectionFmts() ),
    mpTblFrmFmtTbl( new SwFrmFmts() ),
    mpTxtFmtCollTbl( new SwTxtFmtColls() ),
    mpGrfFmtCollTbl( new SwGrfFmtColls() ),
    mpTOXTypes( new SwTOXTypes() ),
    mpDefTOXBases( new SwDefTOXBase_Impl() ),
    mpCurrentView( 0 ),  //swmod 071225
    mpDrawModel( 0 ),
    mpUpdtFlds( new SwDocUpdtFld( this ) ),
    mpFldTypes( new SwFldTypes() ),
    mpVirDev( 0 ),
    mpPrt( 0 ),
    mpPrtData( 0 ),
    mpGlossaryDoc( 0 ),
    mpOutlineRule( 0 ),
    mpFtnInfo( new SwFtnInfo ),
    mpEndNoteInfo( new SwEndNoteInfo ),
    mpLineNumberInfo( new SwLineNumberInfo ),
    mpFtnIdxs( new SwFtnIdxs ),
    mpDocStat( new SwDocStat ),
    mpDocShell( 0 ),
    mpLinkMgr( new sfx2::LinkManager( 0 ) ),
    mpACEWord( 0 ),
    mpURLStateChgd( 0 ),
    mpNumberFormatter( 0 ),
    mpNumRuleTbl( new SwNumRuleTbl ),
    maLists(),
    maListStyleLists(),
    mpRedlineTbl( new SwRedlineTbl ),
    mpAutoFmtRedlnComment( 0 ),
    mpUnoCrsrTbl( new SwUnoCrsrTbl() ),
    mpPgPViewPrtData( 0 ),
    mpExtInputRing( 0 ),
    mpLayouter( 0 ),
    mpStyleAccess( 0 ),
    mpLayoutCache( 0 ),
    mpUnoCallBack(new SwModify(0)),
    mpGrammarContact(createGrammarContact()),
    maChartDataProviderImplRef(),
    mpChartControllerHelper( 0 ),
    mpListItemsList( new tImplSortedNodeNumList() ), // #i83479#
    m_pXmlIdRegistry(),
    mnAutoFmtRedlnCommentNo( 0 ),
    mnLinkUpdMode( GLOBALSETTING ),
    meFldUpdMode( AUTOUPD_GLOBALSETTING ),
    meRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE)),
    meChrCmprType( CHARCOMPRESS_NONE ),
    mReferenceCount(0),
    mIdleBlockCount(0),
    mnLockExpFld( 0 ),
    mbGlossDoc(false),
    mbModified(false),
    mbDtor(false),
    mbPageNums(false),
    mbLoaded(false),
    mbUpdateExpFld(false),
    mbNewDoc(false),
    mbNewFldLst(true),
    mbCopyIsMove(false),
    mbVisibleLinks(true),
    mbInReading(false),
    mbInXMLImport(false),
    mbUpdateTOX(false),
    mbInLoadAsynchron(false),
    mbHTMLMode(false),
    mbInCallModified(false),
    mbIsGlobalDoc(false),
    mbGlblDocSaveLinks(false),
    mbIsLabelDoc(false),
    mbIsAutoFmtRedline(false),
    mbOLEPrtNotifyPending(false),
    mbAllOLENotify(false),
    mbIsRedlineMove(false),
    mbInsOnlyTxtGlssry(false),
    mbContains_MSVBasic(false),
    mbPurgeOLE(true),
    mbKernAsianPunctuation(false),
    mbReadlineChecked(false),
    mbLinksUpdated( false ), //#i38810#
    mbClipBoard( false ),
    mbColumnSelection( false ),
#ifdef DBG_UTIL
    mbXMLExport(false),
#endif
    mbContainsAtPageObjWithContentAnchor(false), //#i119292#, fdo#37024

    // COMPATIBILITY FLAGS START

    mbAddFlyOffsets(false),
    mbUseHiResolutionVirtualDevice(true),
    mbMathBaselineAlignment(false), // default for *old* documents is 'off'
    mbStylesNoDefault(false),
    mbFloattableNomargins(false),
    mEmbedFonts(false),
    mEmbedSystemFonts(false),
    mbOldNumbering(false),
    mbIgnoreFirstLineIndentInNumbering(false),
    mbDoNotResetParaAttrsForNumFont(false),
    mbTableRowKeep(false),
    mbIgnoreTabsAndBlanksForLineCalculation(false),
    mbDoNotCaptureDrawObjsOnPage(false),
    mbOutlineLevelYieldsOutlineRule(false),
    mbClipAsCharacterAnchoredWriterFlyFrames(false),
    mbUnixForceZeroExtLeading(false),
    mbOldPrinterMetrics(false),
    mbTabRelativeToIndent(true),
    mbProtectForm(false), // i#78591#
    mbInvertBorderSpacing (false),
    mbCollapseEmptyCellPara(true),
    mbTabAtLeftIndentForParagraphsInList(false), //#i89181#
    mbSmallCapsPercentage66(false),
    mbTabOverflow(true),
    mbUnbreakableNumberings(false),
    mbClippedPictures(false),
    mbBackgroundParaOverDrawings(false),
    mbTabOverMargin(false),
    mbSurroundTextWrapSmall(false),
    mbLastBrowseMode( false ),
    mn32DummyCompatabilityOptions1(0),
    mn32DummyCompatabilityOptions2(0),

    // COMPATIBILITY FLAGS END

    mbStartIdleTimer(false)
{
    // COMPATIBILITY FLAGS START

    // Note: Any non-hidden compatibility flag should obtain its default
    // by asking SvtCompatibilityOptions, see below.
    //
    const SvtCompatibilityOptions aOptions;
    mbParaSpaceMax                      = aOptions.IsAddSpacing();
    mbParaSpaceMaxAtPages               = aOptions.IsAddSpacingAtPages();
    mbTabCompat                         = !aOptions.IsUseOurTabStops();
    mbUseVirtualDevice                  = !aOptions.IsUsePrtDevice();
    mbAddExternalLeading                = !aOptions.IsNoExtLeading();
    mbOldLineSpacing                    = aOptions.IsUseLineSpacing();
    mbAddParaSpacingToTableCells        = aOptions.IsAddTableSpacing();
    mbUseFormerObjectPos                = aOptions.IsUseObjectPositioning();
    mbUseFormerTextWrapping             = aOptions.IsUseOurTextWrapping();
    mbConsiderWrapOnObjPos              = aOptions.IsConsiderWrappingStyle();

    mbDoNotJustifyLinesWithManualBreak      = !aOptions.IsExpandWordSpace();

    // COMPATIBILITY FLAGS END

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
        GetPageDescFromPool( RES_POOLPAGE_STANDARD );

    // Set to "Empty Page"
    mpEmptyPageFmt->SetFmtAttr( SwFmtFrmSize( ATT_FIX_SIZE ) );
    // Set BodyFmt for columns
    mpColumnContFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );

    _InitFieldTypes();

    // Create a default OutlineNumRule (for Filters)
    mpOutlineRule = new SwNumRule( OUString::createFromAscii( SwNumRule::GetOutlineRuleName() ),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    mpOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    new SwTxtNode(
            SwNodeIndex(GetUndoManager().GetUndoNodes().GetEndOfContent()),
            mpDfltTxtFmtColl );
    new SwTxtNode( SwNodeIndex( GetNodes().GetEndOfContent() ),
                    GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    // set the own IdleTimer
    maIdleTimer.SetTimeout( 600 );
    maIdleTimer.SetTimeoutHdl( LINK(this, SwDoc, DoIdleJobs) );

    maOLEModifiedTimer.SetTimeout( 1000 );
    maOLEModifiedTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateModifiedOLE ));

    maStatsUpdateTimer.SetTimeout( 100 );
    maStatsUpdateTimer.SetTimeoutHdl( LINK( this, SwDoc, DoIdleStatsUpdate ) );

    // Create DBMgr
    mpNewDBMgr = new SwNewDBMgr;

    // create TOXTypes
    InitTOXTypes();

    // pass empty item set containing the paragraph's list attributes
    // as ignorable items to the stype manager.
    {
        SfxItemSet aIgnorableParagraphItems( GetAttrPool(), RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1);
        mpStyleAccess = createStyleManager( &aIgnorableParagraphItems );
    }

    // Initialize the session id of the current document to a random number
    // smaller than 2^21.
    static rtlRandomPool aPool = rtl_random_createPool();
    rtl_random_getBytes( aPool, &mnRsid, sizeof ( mnRsid ) );
    mnRsid &= ( 1<<21 ) - 1;
    mnRsid++;
    mnRsidRoot = mnRsid;

    ResetModified();
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

static void DeleteAndDestroy(SwGrfFmtColls& rFmts, int aStartIdx, int aEndIdx)
{
    if (aEndIdx < aStartIdx)
        return;
    for( SwGrfFmtColls::const_iterator it = rFmts.begin() + aStartIdx;
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

    // #i83479#
    delete mpListItemsList;
    mpListItemsList = 0;

    // clean up chart related structures...
    // Note: the chart data provider gets already diposed in ~SwDocShell
    // since all UNO API related functionality requires an existing SwDocShell
    // this assures that dipose gets called if there is need for it.
    maChartDataProviderImplRef.reset();
    delete mpChartControllerHelper;

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

    StopIdling();   // stop idle timer
    maStatsUpdateTimer.Stop();

    delete mpUnoCallBack, mpUnoCallBack = 0;
    delete mpURLStateChgd;

    delete mpLayouter;
    mpLayouter = 0L;

    // Deactivate Undo notification from Draw
    if( mpDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    delete mpPgPViewPrtData;

    mbDtor = true;

    delete mpRedlineTbl;
    delete mpUnoCrsrTbl;
    delete mpAutoFmtRedlnComment;
    delete mpUpdtFlds;
    delete mpACEWord;

    // Release the BaseLinks
    {
       ::sfx2::SvLinkSources aTemp(mpLinkMgr->GetServers());
       for( ::sfx2::SvLinkSources::const_iterator it = aTemp.begin();
            it != aTemp.end(); ++it )
            (*it)->Closed();

        if( !mpLinkMgr->GetLinks().empty() )
            mpLinkMgr->Remove( 0, mpLinkMgr->GetLinks().size() );
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
        lcl_DelFmtIndizes( pFmt );
    BOOST_FOREACH( SwFrmFmt* pFmt, *mpSpzFrmFmtTbl )
        lcl_DelFmtIndizes( pFmt );
    BOOST_FOREACH( SwSectionFmt* pFmt, *mpSectionFmtTbl )
        lcl_DelFmtIndizes( pFmt );

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

    DeleteAndDestroy(*mpGrfFmtCollTbl, 1, mpGrfFmtCollTbl->size());
    delete mpGrfFmtCollTbl;

    /*
     * DefaultFormats and DefaultFormatCollections (FmtColl)
     * are at position 0 of their respective arrays.
     * In order to not be deleted by the array's dtor, we remove them
     * now.
     */
    mpFrmFmtTbl->erase( mpFrmFmtTbl->begin() );
    mpCharFmtTbl->erase( mpCharFmtTbl->begin() );

    DELETEZ( mpPrt );
    DELETEZ( mpNewDBMgr );

    // All Flys need to be destroyed before the Drawing Model,
    // because Flys can still contain DrawContacts, when no
    // Layout could be constructed due to a read error.
    DeleteAndDestroy( *mpSpzFrmFmtTbl, 0, mpSpzFrmFmtTbl->size() );

    // Only now destroy the Model, the drawing objects - which are also
    // contained in the Undo - need to remove their attributes from the
    // Model. Also, DrawContacts could exist before this.
    ReleaseDrawModel();
    // Destroy DrawModel before the LinkManager, because it's always set
    // in the DrawModel.
    DELETEZ( mpLinkMgr );

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

    {
        for ( boost::unordered_map< String, SwList*, StringHash >::iterator
                                                    aListIter = maLists.begin();
              aListIter != maLists.end();
              ++aListIter )
        {
            delete (*aListIter).second;
        }
        maLists.clear();
    }
    maListStyleLists.clear();

    disposeXForms(); // #i113606#, dispose the XForms objects

    delete mpPrtData;
    delete mpNumberFormatter;
    delete mpFtnInfo;
    delete mpEndNoteInfo;
    delete mpLineNumberInfo;
    delete mpFtnIdxs;
    delete mpFldTypes;
    delete mpTOXTypes;
    delete mpDocStat;
    delete mpEmptyPageFmt;
    delete mpColumnContFmt;
    delete mpDfltCharFmt;
    delete mpDfltFrmFmt;
    delete mpLayoutCache;
    delete mpVirDev;

    SfxItemPool::Free(mpAttrPool);
}

VirtualDevice& SwDoc::CreateVirtualDevice_() const
{
#ifdef IOS
    VirtualDevice* pNewVir = new VirtualDevice( 8 );
#else
    VirtualDevice* pNewVir = new VirtualDevice( 1 );
#endif

    pNewVir->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );

    // #i60945# External leading compatibility for unix systems.
    if ( get(IDocumentSettingAccess::UNIX_FORCE_ZERO_EXT_LEADING ) )
        pNewVir->Compat_ZeroExtleadBug();

    MapMode aMapMode( pNewVir->GetMapMode() );
    aMapMode.SetMapUnit( MAP_TWIP );
    pNewVir->SetMapMode( aMapMode );

    const_cast<SwDoc*>(this)->setVirtualDevice( pNewVir, true, true );
    return *mpVirDev;
}

SfxPrinter& SwDoc::CreatePrinter_() const
{
    OSL_ENSURE( ! mpPrt, "Do not call CreatePrinter_(), call getPrinter() instead" );

#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "Printer will be created!" );
#endif

    // We create a default SfxPrinter.
    // The ItemSet is deleted by Sfx!
    SfxItemSet *pSet = new SfxItemSet( ((SwDoc*)this)->GetAttrPool(),
                    FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                    SID_HTML_MODE,  SID_HTML_MODE,
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );

    SfxPrinter* pNewPrt = new SfxPrinter( pSet );
    const_cast<SwDoc*>(this)->setPrinter( pNewPrt, true, true );
    return *mpPrt;
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

        mpLinkMgr->SetPersist( mpDocShell );
        if( mpDrawModel )
        {
            ((SwDrawDocument*)mpDrawModel)->SetObjectShell( mpDocShell );
            mpDrawModel->SetPersist( mpDocShell );
            OSL_ENSURE( mpDrawModel->GetPersist() == GetPersist(),
                    "draw model's persist is out of sync" );
        }
    }
}

// Convenience method; to avoid excessive includes from docsh.hxx
uno::Reference < embed::XStorage > SwDoc::GetDocStorage()
{
    if( mpDocShell )
        return mpDocShell->GetStorage();
    if( mpLinkMgr->GetPersist() )
        return mpLinkMgr->GetPersist()->GetStorage();
    return NULL;
}

SfxObjectShell* SwDoc::GetPersist() const
{
    return mpDocShell ? mpDocShell : mpLinkMgr->GetPersist();
}

void SwDoc::ClearDoc()
{
    GetIDocumentUndoRedo().DelAllUndoObj();
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Deactivate Undo notification from Draw
    if( mpDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    // if there are still FlyFrames dangling around, delete them too
    sal_uInt16 n;
    while ( 0 != (n = GetSpzFrmFmts()->size()) )
        DelLayoutFmt((*mpSpzFrmFmtTbl)[n-1]);
    OSL_ENSURE( !mpDrawModel || !mpDrawModel->GetPage(0)->GetObjCount(),
                "not all DrawObjects removed from the page" );

    mpRedlineTbl->DeleteAndDestroyAll();

    delete mpACEWord;

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    mpMarkManager->clearAllMarks();
    InitTOXTypes();

    // create a dummy pagedesc for the layout
    sal_uInt16 nDummyPgDsc = MakePageDesc(OUString("?DUMMY?"));
    SwPageDesc* pDummyPgDsc = maPageDescs[ nDummyPgDsc ];

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // create the first one over and over again (without attributes/style etc.
    SwTxtNode* pFirstNd = GetNodes().MakeTxtNode( aSttIdx, mpDfltTxtFmtColl );

    if( mpCurrentView )  //swmod 071029//swmod 071225
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
    // #i114725#,#i115828#
    {
        for ( boost::unordered_map< String, SwList*, StringHash >::iterator
                                                    aListIter = maLists.begin();
              aListIter != maLists.end();
              ++aListIter )
        {
            delete (*aListIter).second;
        }
        maLists.clear();
    }
    maListStyleLists.clear();

    // creation of new outline numbering rule
    mpOutlineRule = new SwNumRule( OUString::createFromAscii( SwNumRule::GetOutlineRuleName() ),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    mpOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    // remove the dummy pagedec from the array and delete all the old ones
    maPageDescs.erase( maPageDescs.begin() + nDummyPgDsc );
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
    DeleteAndDestroy(*mpGrfFmtCollTbl, 1, mpGrfFmtCollTbl->size());
    DeleteAndDestroy(*mpCharFmtTbl, 1, mpCharFmtTbl->size());

    if( mpCurrentView )
    {
        // search the FrameFormat of the root frm. This is not allowed to delete
        mpFrmFmtTbl->erase( std::find( mpFrmFmtTbl->begin(), mpFrmFmtTbl->end(), mpCurrentView->GetLayout()->GetFmt() ) );
        DeleteAndDestroy(*mpFrmFmtTbl, 1, mpFrmFmtTbl->size());
        mpFrmFmtTbl->push_back( mpCurrentView->GetLayout()->GetFmt() );
    }
    else    //swmod 071029//swmod 071225
        DeleteAndDestroy(*mpFrmFmtTbl, 1, mpFrmFmtTbl->size());

    mxForbiddenCharsTable.clear();

    for(SwFldTypes::const_iterator it = mpFldTypes->begin() + INIT_FLDTYPES;
        it != mpFldTypes->end(); ++it)
        delete *it;
    mpFldTypes->erase( mpFldTypes->begin() + INIT_FLDTYPES, mpFldTypes->end() );

    delete mpNumberFormatter, mpNumberFormatter = 0;

    GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    pFirstNd->ChgFmtColl( GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    nDummyPgDsc = maPageDescs.size();
    maPageDescs.push_back( pDummyPgDsc );
    // set the layout back to the new standard pagedesc
    pFirstNd->ResetAllAttr();
    // delete now the dummy pagedesc
    DelPageDesc( nDummyPgDsc );
}

void SwDoc::SetPreViewPrtData( const SwPagePreViewPrtData* pNew )
{
    if( pNew )
    {
        if( mpPgPViewPrtData )
            *mpPgPViewPrtData = *pNew;
        else
            mpPgPViewPrtData = new SwPagePreViewPrtData( *pNew );
    }
    else if( mpPgPViewPrtData )
        DELETEZ( mpPgPViewPrtData );
    SetModified();
}

SwModify*   SwDoc::GetUnoCallBack() const
{
    return mpUnoCallBack;
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

// #i42634# Moved common code of SwReader::Read() and SwDocShell::UpdateLinks()
// to new SwDoc::UpdateLinks():
void SwDoc::UpdateLinks( bool bUI )
{
    SfxObjectCreateMode eMode;
    sal_uInt16 nLinkMode = getLinkUpdateMode( true );
    if ( GetDocShell()) {
        sal_uInt16 nUpdateDocMode = GetDocShell()->GetUpdateDocMode();
        if( (nLinkMode != NEVER ||  document::UpdateDocMode::FULL_UPDATE == nUpdateDocMode) &&
            !GetLinkManager().GetLinks().empty() &&
            SFX_CREATE_MODE_INTERNAL !=
                        ( eMode = GetDocShell()->GetCreateMode()) &&
            SFX_CREATE_MODE_ORGANIZER != eMode &&
            SFX_CREATE_MODE_PREVIEW != eMode &&
            !GetDocShell()->IsPreview() )
        {
            ViewShell* pVSh = 0;
            bool bAskUpdate = nLinkMode == MANUAL;
            bool bUpdate = true;
            switch(nUpdateDocMode)
            {
                case document::UpdateDocMode::NO_UPDATE:   bUpdate = false;break;
                case document::UpdateDocMode::QUIET_UPDATE:bAskUpdate = false; break;
                case document::UpdateDocMode::FULL_UPDATE: bAskUpdate = true; break;
            }
            if( bUpdate && (bUI || !bAskUpdate) )
            {
                SfxMedium* pMedium = GetDocShell()->GetMedium();
                SfxFrame* pFrm = pMedium ? pMedium->GetLoadTargetFrame() : 0;
                Window* pDlgParent = pFrm ? &pFrm->GetWindow() : 0;
                if( GetCurrentViewShell() && !GetEditShell( &pVSh ) && !pVSh )  //swmod 071108//swmod 071225
                {
                    ViewShell aVSh( *this, 0, 0 );

                    SET_CURR_SHELL( &aVSh );
                    GetLinkManager().UpdateAllLinks( bAskUpdate , true, false, pDlgParent );
                }
                else
                    GetLinkManager().UpdateAllLinks( bAskUpdate, true, false, pDlgParent );
            }
        }
    }

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

::sw::MetaFieldManager &
SwDoc::GetMetaFieldManager()
{
    return *m_pMetaFieldManager;
}

::sw::UndoManager &
SwDoc::GetUndoManager()
{
    return *m_pUndoManager;
}

::sw::UndoManager const&
SwDoc::GetUndoManager() const
{
    return *m_pUndoManager;
}

IDocumentUndoRedo &
SwDoc::GetIDocumentUndoRedo()
{
    return *m_pUndoManager;
}

IDocumentUndoRedo const&
SwDoc::GetIDocumentUndoRedo() const
{
    return *m_pUndoManager;
}

void SwDoc::InitTOXTypes()
{
   ShellResource* pShellRes = ViewShell::GetShellRes();
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
        0
    };

    SfxItemSet aNewDefaults(GetAttrPool(), aRangeOfDefaults);

    sal_uInt16 nRange = 0;
    while (aRangeOfDefaults[nRange] != 0)
    {
        for (sal_uInt16 nWhich = aRangeOfDefaults[nRange];
             nWhich < aRangeOfDefaults[nRange + 1]; ++nWhich)
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

void SwDoc::ReplaceCompatabilityOptions(const SwDoc& rSource)
{
    mn32DummyCompatabilityOptions1 = rSource.mn32DummyCompatabilityOptions1;
    mn32DummyCompatabilityOptions2 = rSource.mn32DummyCompatabilityOptions2;
    mbParaSpaceMax = rSource.mbParaSpaceMax;
    mbParaSpaceMaxAtPages = rSource.mbParaSpaceMaxAtPages;
    mbTabCompat = rSource.mbTabCompat;
    mbUseVirtualDevice = rSource.mbUseVirtualDevice;
    mbAddExternalLeading = rSource.mbAddExternalLeading;
    mbOldLineSpacing = rSource.mbOldLineSpacing;
    mbAddParaSpacingToTableCells = rSource.mbAddParaSpacingToTableCells;
    mbUseFormerObjectPos = rSource.mbUseFormerObjectPos;
    mbUseFormerTextWrapping = rSource.mbUseFormerTextWrapping;
    mbConsiderWrapOnObjPos = rSource.mbConsiderWrapOnObjPos;
    mbAddFlyOffsets = rSource.mbAddFlyOffsets;
    mbOldNumbering = rSource.mbOldNumbering;
    mbUseHiResolutionVirtualDevice = rSource.mbUseHiResolutionVirtualDevice;
    mbIgnoreFirstLineIndentInNumbering = rSource.mbIgnoreFirstLineIndentInNumbering;
    mbDoNotJustifyLinesWithManualBreak = rSource.mbDoNotJustifyLinesWithManualBreak;
    mbDoNotResetParaAttrsForNumFont = rSource.mbDoNotResetParaAttrsForNumFont;
    mbOutlineLevelYieldsOutlineRule = rSource.mbOutlineLevelYieldsOutlineRule;
    mbTableRowKeep = rSource.mbTableRowKeep;
    mbIgnoreTabsAndBlanksForLineCalculation = rSource.mbIgnoreTabsAndBlanksForLineCalculation;
    mbDoNotCaptureDrawObjsOnPage = rSource.mbDoNotCaptureDrawObjsOnPage;
    mbClipAsCharacterAnchoredWriterFlyFrames = rSource.mbClipAsCharacterAnchoredWriterFlyFrames;
    mbUnixForceZeroExtLeading = rSource.mbUnixForceZeroExtLeading;
    mbOldPrinterMetrics = rSource.mbOldPrinterMetrics;
    mbTabRelativeToIndent = rSource.mbTabRelativeToIndent;
    mbTabAtLeftIndentForParagraphsInList = rSource.mbTabAtLeftIndentForParagraphsInList;
}

SfxObjectShell* SwDoc::CreateCopy(bool bCallInitNew ) const
{
    SwDoc* pRet = new SwDoc;

    // we have to use pointer here, since the callee has to decide whether
    // SfxObjectShellLock or SfxObjectShellRef should be used sometimes the
    // object will be returned with refcount set to 0 ( if no DoInitNew is done )
    SfxObjectShell* pRetShell = new SwDocShell( pRet, SFX_CREATE_MODE_STANDARD );
    if( bCallInitNew )
    {
        // it could happen that DoInitNew creates model, that increases the refcount of the object
        pRetShell->DoInitNew();
    }

    pRet->acquire();

    pRet->ReplaceDefaults(*this);

    pRet->ReplaceCompatabilityOptions(*this);

    pRet->ReplaceStyles(*this);

    // copy content
    pRet->Paste( *this );

    // remove the temporary shell if it is there as it was done before
    pRet->SetTmpDocShell( (SfxObjectShell*)NULL );

    pRet->release();

    return pRetShell;
}

// copy document content - code from SwFEShell::Paste( SwDoc* )
void SwDoc::Paste( const SwDoc& rSource )
{
    // this has to be empty const sal_uInt16 nStartPageNumber = GetPhyPageNum();
    // until the end of the NodesArray
    SwNodeIndex aSourceIdx( rSource.GetNodes().GetEndOfExtras(), 2 );
    SwPaM aCpyPam( aSourceIdx ); //DocStart
    SwNodeIndex aTargetIdx( GetNodes().GetEndOfExtras(), 2 );
    SwPaM aInsertPam( aTargetIdx ); //replaces PCURCRSR from SwFEShell::Paste()


    aCpyPam.SetMark();
    aCpyPam.Move( fnMoveForward, fnGoDoc );

    this->GetIDocumentUndoRedo().StartUndo( UNDO_INSGLOSSARY, NULL );
    this->LockExpFlds();

    {
        SwPosition& rInsPos = *aInsertPam.GetPoint();
        //find out if the clipboard document starts with a table
        bool bStartWithTable = 0 != aCpyPam.Start()->nNode.GetNode().FindTableNode();
        SwPosition aInsertPosition( rInsPos );

        {
            SwNodeIndex aIndexBefore(rInsPos.nNode);

            aIndexBefore--;

            rSource.CopyRange( aCpyPam, rInsPos, true );

            {
                ++aIndexBefore;
                SwPaM aPaM(SwPosition(aIndexBefore),
                           SwPosition(rInsPos.nNode));

                MakeUniqueNumRules(aPaM);
            }
        }

        //TODO: Is this necessary here? SaveTblBoxCntnt( &rInsPos );
        if(/*bIncludingPageFrames && */bStartWithTable)
        {
            //remove the paragraph in front of the table
            SwPaM aPara(aInsertPosition);
            this->DelFullPara(aPara);
        }
        //additionally copy page bound frames
        if( /*bIncludingPageFrames && */rSource.GetSpzFrmFmts()->size() )
        {
            for ( sal_uInt16 i = 0; i < rSource.GetSpzFrmFmts()->size(); ++i )
            {
                const SwFrmFmt& rCpyFmt = *(*rSource.GetSpzFrmFmts())[i];
                    SwFmtAnchor aAnchor( rCpyFmt.GetAnchor() );
                    if (FLY_AT_PAGE == aAnchor.GetAnchorId())
                    {
                        aAnchor.SetPageNum( aAnchor.GetPageNum() /*+ nStartPageNumber - */);
                    }
                    else
                        continue;
                    this->CopyLayoutFmt( rCpyFmt, aAnchor, true, true );
            }
        }
    }

    this->GetIDocumentUndoRedo().EndUndo( UNDO_INSGLOSSARY, NULL );

    UnlockExpFlds();
    UpdateFlds(NULL, false);
}

sal_uInt16 SwTxtFmtColls::GetPos(const SwTxtFmtColl* p) const
{
    const_iterator it = std::find(begin(), end(), p);
    return it == end() ? USHRT_MAX : it - begin();
}

sal_uInt16 SwGrfFmtColls::GetPos(const SwGrfFmtColl* p) const
{
    const_iterator it = std::find(begin(), end(), p);
    return it == end() ? USHRT_MAX : it - begin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
