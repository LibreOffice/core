/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <unochart.hxx>
#include <fldbas.hxx>

#include <cmdid.h>

#include <pausethreadstarting.hxx>
#include <numrule.hxx>
#include <list.hxx>
#include <listfunc.hxx>

#include <sfx2/Metadatable.hxx>
#include <fmtmeta.hxx>
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
    
    bool bVisible = false;
    const SwDocShell *mpDocShell = rDoc.GetDocShell();
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( mpDocShell, sal_False );
    while (pFrame && !bVisible)
    {
        if (pFrame->IsVisible())
            bVisible = true;
        pFrame = SfxViewFrame::GetNext( *pFrame, mpDocShell, sal_False );
    }

    
    
    
    
    if (bVisible)
    {
        uno::Reference< linguistic2::XProofreadingIterator > xGCIterator( rDoc.GetGCIterator() );
        if ( xGCIterator.is() )
        {
            uno::Reference< lang::XComponent >  xDoc( rDoc.GetDocShell()->GetBaseModel(), uno::UNO_QUERY );
            uno::Reference< text::XFlatParagraphIteratorProvider >  xFPIP( xDoc, uno::UNO_QUERY );

            
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
    : m_pNodes( new SwNodes(this) )
    ,
    mpAttrPool(new SwAttrPool(this)),
    mpMarkManager(new ::sw::mark::MarkManager(*this)),
    m_pMetaFieldManager(new ::sw::MetaFieldManager()),
    m_pUndoManager(new ::sw::UndoManager(
            boost::shared_ptr<SwNodes>(new SwNodes(this)), *this, *this, *this)),
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
    mpCurrentView( 0 ),
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
    mpExtraRedlineTbl ( new SwExtraRedlineTbl ),
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
    mpListItemsList( new tImplSortedNodeNumList() ), 
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
    mbLinksUpdated( false ), 
    mbClipBoard( false ),
    mbColumnSelection( false ),
    mbIsPrepareSelAll(false),
#ifdef DBG_UTIL
    mbXMLExport(false),
#endif
    mbContainsAtPageObjWithContentAnchor(false), 

    

    mbAddFlyOffsets(false),
    mbUseHiResolutionVirtualDevice(true),
    mbMathBaselineAlignment(false), 
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
    mbProtectForm(false), 
    mbInvertBorderSpacing (false),
    mbCollapseEmptyCellPara(true),
    mbTabAtLeftIndentForParagraphsInList(false), 
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

    

    mbStartIdleTimer(false),
    mbReadOnly(false)
{
    

    
    
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
    
    mpTxtFmtCollTbl->push_back(mpDfltTxtFmtColl);
    
    mpGrfFmtCollTbl->push_back(mpDfltGrfFmtColl);

    
    if ( maPageDescs.empty() )
        GetPageDescFromPool( RES_POOLPAGE_STANDARD );

    
    mpEmptyPageFmt->SetFmtAttr( SwFmtFrmSize( ATT_FIX_SIZE ) );
    
    mpColumnContFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );

    _InitFieldTypes();

    
    mpOutlineRule = new SwNumRule( SwNumRule::GetOutlineRuleName(),
                                  
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    
    mpOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    new SwTxtNode(
            SwNodeIndex(GetUndoManager().GetUndoNodes().GetEndOfContent()),
            mpDfltTxtFmtColl );
    new SwTxtNode( SwNodeIndex( GetNodes().GetEndOfContent() ),
                    GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    
    maIdleTimer.SetTimeout( 600 );
    maIdleTimer.SetTimeoutHdl( LINK(this, SwDoc, DoIdleJobs) );

    maOLEModifiedTimer.SetTimeout( 1000 );
    maOLEModifiedTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateModifiedOLE ));

    maStatsUpdateTimer.SetTimeout( 100 );
    maStatsUpdateTimer.SetTimeoutHdl( LINK( this, SwDoc, DoIdleStatsUpdate ) );

    
    mpNewDBMgr = new SwNewDBMgr;

    
    InitTOXTypes();

    
    
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
        
        
        static rtlRandomPool aPool = rtl_random_createPool();
        rtl_random_getBytes( aPool, &mnRsid, sizeof ( mnRsid ) );
        mnRsid &= ( 1<<21 ) - 1;
        mnRsid++;
    }
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
    
    GetIDocumentUndoRedo().DoUndo(false);

    if (mpDocShell)
    {
        mpDocShell->SetUndoManager(0);
    }

    
    delete mpListItemsList;
    mpListItemsList = 0;

    
    
    
    
    maChartDataProviderImplRef.reset();
    delete mpChartControllerHelper;

    delete mpGrammarContact;
    mpGrammarContact = 0;

    
    
    
    
    SwFmtDrop aDrop;
    SetDefault(aDrop);
    
    SwFmtCharFmt aCharFmt(NULL);
    SetDefault(aCharFmt);

    StopIdling();   
    maStatsUpdateTimer.Stop();

    delete mpUnoCallBack, mpUnoCallBack = 0;
    delete mpURLStateChgd;

    delete mpLayouter;
    mpLayouter = 0L;

    
    if( mpDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    delete mpPgPViewPrtData;

    mbDtor = true;

    delete mpRedlineTbl;
    delete mpExtraRedlineTbl;
    delete mpUnoCrsrTbl;
    delete mpAutoFmtRedlnComment;
    delete mpUpdtFlds;
    delete mpACEWord;

    
    {
       ::sfx2::SvLinkSources aTemp(mpLinkMgr->GetServers());
       for( ::sfx2::SvLinkSources::const_iterator it = aTemp.begin();
            it != aTemp.end(); ++it )
            (*it)->Closed();

        if( !mpLinkMgr->GetLinks().empty() )
            mpLinkMgr->Remove( 0, mpLinkMgr->GetLinks().size() );
    }

    
    
    m_pNodes->pOutlineNds->clear();
    SwNodes & rUndoNodes( GetUndoManager().GetUndoNodes() );
    rUndoNodes.pOutlineNds->clear();

    mpFtnIdxs->clear();

    
    m_pUndoManager->DelAllUndoObj();

    
    
    mpMarkManager->clearAllMarks();

    if( mpExtInputRing )
    {
        Ring* pTmp = mpExtInputRing;
        mpExtInputRing = 0;
        while( pTmp->GetNext() != pTmp )
            delete pTmp->GetNext();
        delete pTmp;
    }

    
    
    {
        for( sal_uInt16 n = mpTOXTypes->size(); n; )
        {
            (*mpTOXTypes)[ --n ]->SetInDocDTOR();
            delete (*mpTOXTypes)[ n ];
        }
        mpTOXTypes->clear();
    }
    delete mpDefTOXBases;

    
    
    BOOST_FOREACH( SwFrmFmt* pFmt, *mpFrmFmtTbl )
        lcl_DelFmtIndices( pFmt );
    BOOST_FOREACH( SwFrmFmt* pFmt, *mpSpzFrmFmtTbl )
        lcl_DelFmtIndices( pFmt );
    BOOST_FOREACH( SwSectionFmt* pFmt, *mpSectionFmtTbl )
        lcl_DelFmtIndices( pFmt );

    
    
    
    
    BOOST_FOREACH(SwPageDesc *pPageDesc, maPageDescs)
        delete pPageDesc;
    maPageDescs.clear();

    
    
    
    m_pNodes->DelNodes( SwNodeIndex(*m_pNodes), m_pNodes->Count() );
    rUndoNodes.DelNodes( SwNodeIndex( rUndoNodes ), rUndoNodes.Count() );

    

    
    
    mpFtnInfo->ReleaseCollection();
    mpEndNoteInfo->ReleaseCollection();

    OSL_ENSURE( mpDfltTxtFmtColl == (*mpTxtFmtCollTbl)[0],
            "Default-Text-Collection must always be at the start" );

    
    
    
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

    
    
    
    DeleteAndDestroy( *mpSpzFrmFmtTbl, 0, mpSpzFrmFmtTbl->size() );

    
    
    
    ReleaseDrawModel();
    
    
    DELETEZ( mpLinkMgr );

    
    
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
        for ( boost::unordered_map< OUString, SwList*, OUStringHash >::iterator
                                                    aListIter = maLists.begin();
              aListIter != maLists.end();
              ++aListIter )
        {
            delete (*aListIter).second;
        }
        maLists.clear();
    }
    maListStyleLists.clear();

    disposeXForms(); 

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

    
    if( mpDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    
    sal_uInt16 n;
    while ( 0 != (n = GetSpzFrmFmts()->size()) )
        DelLayoutFmt((*mpSpzFrmFmtTbl)[n-1]);
    OSL_ENSURE( !mpDrawModel || !mpDrawModel->GetPage(0)->GetObjCount(),
                "not all DrawObjects removed from the page" );

    mpRedlineTbl->DeleteAndDestroyAll();
    mpExtraRedlineTbl->DeleteAndDestroyAll();

    delete mpACEWord;

    
    
    mpMarkManager->clearAllMarks();
    InitTOXTypes();

    
    sal_uInt16 nDummyPgDsc = MakePageDesc(OUString("?DUMMY?"));
    SwPageDesc* pDummyPgDsc = maPageDescs[ nDummyPgDsc ];

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    
    SwTxtNode* pFirstNd = GetNodes().MakeTxtNode( aSttIdx, mpDfltTxtFmtColl );

    if( mpCurrentView )
    {
        
        pFirstNd->SetAttr( SwFmtPageDesc( pDummyPgDsc ));

        SwPosition aPos( *pFirstNd, SwIndex( pFirstNd ));
        SwPaM const tmpPaM(aSttIdx, SwNodeIndex(GetNodes().GetEndOfContent()));
        ::PaMCorrAbs(tmpPaM, aPos);
    }

    GetNodes().Delete( aSttIdx,
            GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );

    
    
    
    mpOutlineRule = NULL;
    BOOST_FOREACH( SwNumRule* pNumRule, *mpNumRuleTbl )
        delete pNumRule;
    mpNumRuleTbl->clear();
    
    {
        for ( boost::unordered_map< OUString, SwList*, OUStringHash >::iterator
                                                    aListIter = maLists.begin();
              aListIter != maLists.end();
              ++aListIter )
        {
            delete (*aListIter).second;
        }
        maLists.clear();
    }
    maListStyleLists.clear();

    
    mpOutlineRule = new SwNumRule( SwNumRule::GetOutlineRuleName(),
                                  
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(mpOutlineRule);
    
    mpOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    
    maPageDescs.erase( maPageDescs.begin() + nDummyPgDsc );
    BOOST_FOREACH(SwPageDesc *pPageDesc, maPageDescs)
        delete pPageDesc;
    maPageDescs.clear();

    
    
    mpFtnInfo->ReleaseCollection();
    mpEndNoteInfo->ReleaseCollection();

    
    
    
    if( 2 < mpTxtFmtCollTbl->size() )
        DeleteAndDestroy(*mpTxtFmtCollTbl, 2, mpTxtFmtCollTbl->size());
    DeleteAndDestroy(*mpTxtFmtCollTbl, 1, mpTxtFmtCollTbl->size());
    DeleteAndDestroy(*mpGrfFmtCollTbl, 1, mpGrfFmtCollTbl->size());
    DeleteAndDestroy(*mpCharFmtTbl, 1, mpCharFmtTbl->size());

    if( mpCurrentView )
    {
        
        mpFrmFmtTbl->erase( std::find( mpFrmFmtTbl->begin(), mpFrmFmtTbl->end(), mpCurrentView->GetLayout()->GetFmt() ) );
        DeleteAndDestroy(*mpFrmFmtTbl, 1, mpFrmFmtTbl->size());
        mpFrmFmtTbl->push_back( mpCurrentView->GetLayout()->GetFmt() );
    }
    else
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
    
    pFirstNd->ResetAllAttr();
    
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
            SwViewShell* pVSh = 0;
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
                if( GetCurrentViewShell() && !GetEditShell( &pVSh ) && !pVSh )
                {
                    SwViewShell aVSh( *this, 0, 0 );

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
}

void SwDoc::ReplaceDefaults(const SwDoc& rSource)
{
    
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

    
    
    
    SfxObjectShell* pRetShell = new SwDocShell( pRet, SFX_CREATE_MODE_STANDARD );
    if( bCallInitNew )
    {
        
        pRetShell->DoInitNew();
    }

    pRet->acquire();

    pRet->ReplaceDefaults(*this);

    pRet->ReplaceCompatabilityOptions(*this);

    pRet->ReplaceStyles(*this);

    
    pRet->Paste( *this );

    
    pRet->SetTmpDocShell( (SfxObjectShell*)NULL );

    pRet->release();

    return pRetShell;
}


void SwDoc::Paste( const SwDoc& rSource )
{
    
    
    SwNodeIndex aSourceIdx( rSource.GetNodes().GetEndOfExtras(), 2 );
    SwPaM aCpyPam( aSourceIdx ); 
    SwNodeIndex aTargetIdx( GetNodes().GetEndOfExtras(), 2 );
    SwPaM aInsertPam( aTargetIdx ); 

    aCpyPam.SetMark();
    aCpyPam.Move( fnMoveForward, fnGoDoc );

    this->GetIDocumentUndoRedo().StartUndo( UNDO_INSGLOSSARY, NULL );
    this->LockExpFlds();

    {
        SwPosition& rInsPos = *aInsertPam.GetPoint();
        
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

        
        if(/*bIncludingPageFrames && */bStartWithTable)
        {
            
            SwPaM aPara(aInsertPosition);
            this->DelFullPara(aPara);
        }
        
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
