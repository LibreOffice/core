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

#define ROLBCK_HISTORY_ONLY     // The fight against the CLOOK's
#include <doc.hxx>
#include <dcontact.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/text/XFlatParagraphIteratorProvider.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/logfile.hxx>
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


#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <pausethreadstarting.hxx>
#include <numrule.hxx>
#include <list.hxx>
#include <listfunc.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

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
        uno::Reference< lang::XMultiServiceFactory >  xMgr( comphelper::getProcessServiceFactory() );
        if (xMgr.is())
        {
            try
            {
                rtl::OUString aServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.linguistic2.ProofreadingIterator"));
                m_xGCIterator = uno::Reference< linguistic2::XProofreadingIterator >
                    ( xMgr->createInstance( aServiceName ), uno::UNO_QUERY_THROW );
            }
            catch (uno::Exception &)
            {
                OSL_FAIL( "No GCIterator" );
            }
        }
    }

    return m_xGCIterator;
}

void StartGrammarChecking( SwDoc &rDoc )
{
    // check for a visible view
    bool bVisible = false;
    const SwDocShell *pDocShell = rDoc.GetDocShell();
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( pDocShell, sal_False );
    while (pFrame && !bVisible)
    {
        if (pFrame->IsVisible())
            bVisible = true;
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell, sal_False );
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
    pMarkManager(new ::sw::mark::MarkManager(*this)),
    m_pMetaFieldManager(new ::sw::MetaFieldManager()),
    m_pUndoManager(new ::sw::UndoManager(
            ::std::auto_ptr<SwNodes>(new SwNodes(this)), *this, *this, *this)),
    pDfltFrmFmt( new SwFrmFmt( GetAttrPool(), sFrmFmtStr, 0 ) ),
    pEmptyPageFmt( new SwFrmFmt( GetAttrPool(), sEmptyPageStr, pDfltFrmFmt ) ),
    pColumnContFmt( new SwFrmFmt( GetAttrPool(), sColumnCntStr, pDfltFrmFmt ) ),
    pDfltCharFmt( new SwCharFmt( GetAttrPool(), sCharFmtStr, 0 ) ),
    pDfltTxtFmtColl( new SwTxtFmtColl( GetAttrPool(), sTxtCollStr ) ),
    pDfltGrfFmtColl( new SwGrfFmtColl( GetAttrPool(), sGrfCollStr ) ),
    pFrmFmtTbl( new SwFrmFmts() ),
    pCharFmtTbl( new SwCharFmts() ),
    pSpzFrmFmtTbl( new SwFrmFmts() ),
    pSectionFmtTbl( new SwSectionFmts() ),
    pTblFrmFmtTbl( new SwFrmFmts() ),
    pTxtFmtCollTbl( new SwTxtFmtColls() ),
    pGrfFmtCollTbl( new SwGrfFmtColls() ),
    pTOXTypes( new SwTOXTypes() ),
    pDefTOXBases( new SwDefTOXBase_Impl() ),
    pCurrentView( 0 ),  //swmod 071225
    pDrawModel( 0 ),
    pUpdtFlds( new SwDocUpdtFld() ),
    pFldTypes( new SwFldTypes() ),
    pVirDev( 0 ),
    pPrt( 0 ),
    pPrtData( 0 ),
    pGlossaryDoc( 0 ),
    pOutlineRule( 0 ),
    pFtnInfo( new SwFtnInfo ),
    pEndNoteInfo( new SwEndNoteInfo ),
    pLineNumberInfo( new SwLineNumberInfo ),
    pFtnIdxs( new SwFtnIdxs ),
    pDocStat( new SwDocStat ),
    pDocShell( 0 ),
    pLinkMgr( new sfx2::LinkManager( 0 ) ),
    pACEWord( 0 ),
    pURLStateChgd( 0 ),
    pNumberFormatter( 0 ),
    pNumRuleTbl( new SwNumRuleTbl ),
    maLists(),
    maListStyleLists(),
    pRedlineTbl( new SwRedlineTbl ),
    pAutoFmtRedlnComment( 0 ),
    pUnoCrsrTbl( new SwUnoCrsrTbl() ),
    pPgPViewPrtData( 0 ),
    pExtInputRing( 0 ),
    pLayouter( 0 ),
    pStyleAccess( 0 ),
    pLayoutCache( 0 ),
    pUnoCallBack(new SwModify(0)),
    mpGrammarContact(createGrammarContact()),
    aChartDataProviderImplRef(),
    pChartControllerHelper( 0 ),
    mpListItemsList( new tImplSortedNodeNumList() ), // #i83479#
    m_pXmlIdRegistry(),
    nAutoFmtRedlnCommentNo( 0 ),
    nLinkUpdMode( GLOBALSETTING ),
    eFldUpdMode( AUTOUPD_GLOBALSETTING ),
    eRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE)),
    eChrCmprType( CHARCOMPRESS_NONE ),
    mReferenceCount(0),
    mIdleBlockCount(0),
    nLockExpFld( 0 ),
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

    mbApplyWorkaroundForB6375613(false),

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
    mbLastBrowseMode( false ),
    n32DummyCompatabilityOptions1(0),
    n32DummyCompatabilityOptions2(0),

    // COMPATIBILITY FLAGS END

    mbStartIdleTimer(sal_False),
    mbSetDrawDefaults(false)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::SwDoc" );

    //
    // COMPATIBILITY FLAGS START
    //

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

    //
    // COMPATIBILITY FLAGS END
    //

    /*
     * DefaultFormats and DefaultFormatCollections (FmtColl)
     * are inserted at position 0 at the respective array.
     * The formats in the FmtColls are derived from the
     * DefaultFormats and are also in the list.
     */
    /* Formats */
    pFrmFmtTbl->push_back(pDfltFrmFmt);
    pCharFmtTbl->push_back(pDfltCharFmt);

    /* FmtColls */
    // TXT
    pTxtFmtCollTbl->push_back(pDfltTxtFmtColl);
    // GRF
    pGrfFmtCollTbl->push_back(pDfltGrfFmtColl);

    // Create PageDesc, EmptyPageFmt and ColumnFmt
    if ( aPageDescs.empty() )
        GetPageDescFromPool( RES_POOLPAGE_STANDARD );

    // Set to "Empty Page"
    pEmptyPageFmt->SetFmtAttr( SwFmtFrmSize( ATT_FIX_SIZE ) );
    // Set BodyFmt for columns
    pColumnContFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );

    _InitFieldTypes();

    // Create a default OutlineNumRule (for Filters)
    pOutlineRule = new SwNumRule( rtl::OUString::createFromAscii( SwNumRule::GetOutlineRuleName() ),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(pOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    pOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    new SwTxtNode(
            SwNodeIndex(GetUndoManager().GetUndoNodes().GetEndOfContent()),
            pDfltTxtFmtColl );
    new SwTxtNode( SwNodeIndex( GetNodes().GetEndOfContent() ),
                    GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    // set the own IdleTimer
    aIdleTimer.SetTimeout( 600 );
    aIdleTimer.SetTimeoutHdl( LINK(this, SwDoc, DoIdleJobs) );

    aOLEModifiedTimer.SetTimeout( 1000 );
    aOLEModifiedTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateModifiedOLE ));

    // Create DBMgr
    pNewDBMgr = new SwNewDBMgr;

    // create TOXTypes
    InitTOXTypes();

    // pass empty item set containing the paragraph's list attributes
    // as ignorable items to the stype manager.
    {
        SfxItemSet aIgnorableParagraphItems( GetAttrPool(), RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1);
        pStyleAccess = createStyleManager( &aIgnorableParagraphItems );
    }

    // Initialize the session id of the current document to a random number
    // smaller than 2^21.
    static rtlRandomPool aPool = rtl_random_createPool();
    rtl_random_getBytes( aPool, &nRsid, sizeof ( nRsid ) );
    nRsid &= ( 1<<21 ) - 1;
    nRsid++;
    nRsidRoot = nRsid;

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

/*
 * Speciality: a member of the class SwDoc is located at
 * position 0 in the array of the Format and GDI objects.
 * This MUST not be destroyed using 'delete' in any case!
 */
SwDoc::~SwDoc()
{
    // nothing here should create Undo actions!
    GetIDocumentUndoRedo().DoUndo(false);

    if (pDocShell)
    {
        pDocShell->SetUndoManager(0);
    }


    // #i83479#
    delete mpListItemsList;
    mpListItemsList = 0;

    // clean up chart related structures...
    // Note: the chart data provider gets already diposed in ~SwDocShell
    // since all UNO API related functionality requires an existing SwDocShell
    // this assures that dipose gets called if there is need for it.
    aChartDataProviderImplRef.reset();
    delete pChartControllerHelper;

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

    delete pUnoCallBack, pUnoCallBack = 0;
    delete pURLStateChgd;

    delete pLayouter;
    pLayouter = 0L;

    // Deactivate Undo notification from Draw
    if( pDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    delete pPgPViewPrtData;

    mbDtor = sal_True;

    delete pRedlineTbl;
    delete pUnoCrsrTbl;
    delete pAutoFmtRedlnComment;
    delete pUpdtFlds;
    delete pACEWord;

    // Release the BaseLinks
    {
       ::sfx2::SvLinkSources aTemp(pLinkMgr->GetServers());
       for( ::sfx2::SvLinkSources::const_iterator it = aTemp.begin();
            it != aTemp.end(); ++it )
            (*it)->Closed();

        if( !pLinkMgr->GetLinks().empty() )
            pLinkMgr->Remove( 0, pLinkMgr->GetLinks().size() );
    }

    // The ChapterNumbers/Numbers need to be deleted before the styles
    // or we update all the time!
    m_pNodes->pOutlineNds->clear();
    SwNodes & rUndoNodes( GetUndoManager().GetUndoNodes() );
    rUndoNodes.pOutlineNds->clear();

    pFtnIdxs->clear();

    // indices could be registered in attributes
    m_pUndoManager->DelAllUndoObj();

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    pMarkManager->clearAllMarks();

    if( pExtInputRing )
    {
        Ring* pTmp = pExtInputRing;
        pExtInputRing = 0;
        while( pTmp->GetNext() != pTmp )
            delete pTmp->GetNext();
        delete pTmp;
    }

    // Old - deletion without a Flag is expensive, because we send a Modify
    // aTOXTypes.DeleteAndDestroy( 0, aTOXTypes.Count() );
    {
        for( sal_uInt16 n = pTOXTypes->size(); n; )
        {
            (*pTOXTypes)[ --n ]->SetInDocDTOR();
            delete (*pTOXTypes)[ n ];
        }
        pTOXTypes->clear();
    }
    delete pDefTOXBases;

    // Any of the FrmFormats can still have indices registered.
    // These need to be destroyed now at the latest.
    BOOST_FOREACH( SwFrmFmt* pFmt, *pFrmFmtTbl )
        lcl_DelFmtIndizes( pFmt );
    BOOST_FOREACH( SwFrmFmt* pFmt, *pSpzFrmFmtTbl )
        lcl_DelFmtIndizes( pFmt );
    BOOST_FOREACH( SwSectionFmt* pFmt, *pSectionFmtTbl )
        lcl_DelFmtIndizes( pFmt );

    // The formats/styles that follow depend on the default formats.
    // Destroy these only after destroying the FmtIndices, because the content
    // of headers/footers has to be deleted as well. If in the headers/footers
    // there are still Flys registered at that point, we have a problem.
    BOOST_FOREACH(SwPageDesc *pPageDesc, aPageDescs)
        delete pPageDesc;
    aPageDescs.clear();

    // Delete content selections.
    // Don't wait for the SwNodes dtor to destroy them; so that Formats
    // do not have any dependencies anymore.
    m_pNodes->DelNodes( SwNodeIndex(*m_pNodes), m_pNodes->Count() );
    rUndoNodes.DelNodes( SwNodeIndex( rUndoNodes ), rUndoNodes.Count() );

    // Delete Formats, make it permanent some time in the future

    // Delete for Collections
    // So that we get rid of the dependencies
    pFtnInfo->ReleaseCollection();
    pEndNoteInfo->ReleaseCollection();

    OSL_ENSURE( pDfltTxtFmtColl == (*pTxtFmtCollTbl)[0],
            "Default-Text-Collection must always be at the start" );

    // Optimization: Based on the fact that Standard is always 2nd in the
    // array, we should delete it as the last. With this we avoid
    // reparenting the Formats all the time!
    if( 2 < pTxtFmtCollTbl->size() )
        DeleteAndDestroy(*pTxtFmtCollTbl, 2, pTxtFmtCollTbl->size());
    DeleteAndDestroy(*pTxtFmtCollTbl, 1, pTxtFmtCollTbl->size());
    delete pTxtFmtCollTbl;

    OSL_ENSURE( pDfltGrfFmtColl == (*pGrfFmtCollTbl)[0],
            "DefaultGrfCollection must always be at the start" );

    DeleteAndDestroy(*pGrfFmtCollTbl, 1, pGrfFmtCollTbl->size());
    delete pGrfFmtCollTbl;

    /*
     * DefaultFormats and DefaultFormatCollections (FmtColl)
     * are at position 0 of their respective arrays.
     * In order to not be deleted by the array's dtor, we remove them
     * now.
     */
    pFrmFmtTbl->erase( pFrmFmtTbl->begin() );
    pCharFmtTbl->erase( pCharFmtTbl->begin() );

    DELETEZ( pPrt );
    DELETEZ( pNewDBMgr );

    // All Flys need to be destroyed before the Drawing Model,
    // because Flys can still contain DrawContacts, when no
    // Layout could be constructed due to a read error.
    DeleteAndDestroy( *pSpzFrmFmtTbl, 0, pSpzFrmFmtTbl->size() );

    // Only now destroy the Model, the drawing objects - which are also
    // contained in the Undo - need to remove their attributes from the
    // Model. Also, DrawContacts could exist before this.
    ReleaseDrawModel();
    // Destroy DrawModel before the LinkManager, because it's always set
    // in the DrawModel.
    DELETEZ( pLinkMgr );

    // Clear the Tables before deleting the defaults, or we crash due to
    // dependencies on defaults.
    delete pFrmFmtTbl;
    delete pSpzFrmFmtTbl;

    delete pStyleAccess;

    delete pCharFmtTbl;
    delete pSectionFmtTbl;
    delete pTblFrmFmtTbl;
    delete pDfltTxtFmtColl;
    delete pDfltGrfFmtColl;
    delete pNumRuleTbl;

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

    delete pPrtData;
    delete pNumberFormatter;
    delete pFtnInfo;
    delete pEndNoteInfo;
    delete pLineNumberInfo;
    delete pFtnIdxs;
    delete pFldTypes;
    delete pTOXTypes;
    delete pDocStat;
    delete pEmptyPageFmt;
    delete pColumnContFmt;
    delete pDfltCharFmt;
    delete pDfltFrmFmt;
    delete pLayoutCache;
    delete pVirDev;

    SfxItemPool::Free(mpAttrPool);
}

VirtualDevice& SwDoc::CreateVirtualDevice_() const
{
    VirtualDevice* pNewVir = new VirtualDevice( 1 );

    pNewVir->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );

    // #i60945# External leading compatibility for unix systems.
    if ( get(IDocumentSettingAccess::UNIX_FORCE_ZERO_EXT_LEADING ) )
        pNewVir->Compat_ZeroExtleadBug();

    MapMode aMapMode( pNewVir->GetMapMode() );
    aMapMode.SetMapUnit( MAP_TWIP );
    pNewVir->SetMapMode( aMapMode );

    const_cast<SwDoc*>(this)->setVirtualDevice( pNewVir, true, true );
    return *pVirDev;
}

SfxPrinter& SwDoc::CreatePrinter_() const
{
    OSL_ENSURE( ! pPrt, "Do not call CreatePrinter_(), call getPrinter() instead" );

#if OSL_DEBUG_LEVEL > 1
    OSL_FAIL( "Printer will be created!" );
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
    return *pPrt;
}

void SwDoc::SetDocShell( SwDocShell* pDSh )
{
    if( pDocShell != pDSh )
    {
        if (pDocShell)
        {
            pDocShell->SetUndoManager(0);
        }
        pDocShell = pDSh;
        if (pDocShell)
        {
            pDocShell->SetUndoManager(& GetUndoManager());
        }

        pLinkMgr->SetPersist( pDocShell );
        if( pDrawModel )
        {
            ((SwDrawDocument*)pDrawModel)->SetObjectShell( pDocShell );
            pDrawModel->SetPersist( pDocShell );
            OSL_ENSURE( pDrawModel->GetPersist() == GetPersist(),
                    "draw model's persist is out of sync" );
        }
    }
}

// Convenience method; to avoid excessive includes from docsh.hxx
uno::Reference < embed::XStorage > SwDoc::GetDocStorage()
{
    if( pDocShell )
        return pDocShell->GetStorage();
    if( pLinkMgr->GetPersist() )
        return pLinkMgr->GetPersist()->GetStorage();
    return NULL;
}

SfxObjectShell* SwDoc::GetPersist() const
{
    return pDocShell ? pDocShell : pLinkMgr->GetPersist();
}

void SwDoc::ClearDoc()
{
    GetIDocumentUndoRedo().DelAllUndoObj();
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Deactivate Undo notification from Draw
    if( pDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    // if there are still FlyFrames dangling around, delete them too
    sal_uInt16 n;
    while ( 0 != (n = GetSpzFrmFmts()->size()) )
        DelLayoutFmt((*pSpzFrmFmtTbl)[n-1]);
    OSL_ENSURE( !pDrawModel || !pDrawModel->GetPage(0)->GetObjCount(),
                "not all DrawObjects removed from the page" );

    pRedlineTbl->DeleteAndDestroyAll();

    delete pACEWord;

    // The BookMarks contain indices to the Content. These must be deleted
    // before deleting the Nodes.
    pMarkManager->clearAllMarks();
    InitTOXTypes();

    // create a dummy pagedesc for the layout
    sal_uInt16 nDummyPgDsc = MakePageDesc(rtl::OUString("?DUMMY?"));
    SwPageDesc* pDummyPgDsc = aPageDescs[ nDummyPgDsc ];

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // create the first one over and over again (without attributes/style etc.
    SwTxtNode* pFirstNd = GetNodes().MakeTxtNode( aSttIdx, pDfltTxtFmtColl );

    if( pCurrentView )  //swmod 071029//swmod 071225
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
    pOutlineRule = NULL;
    BOOST_FOREACH( SwNumRule* pNumRule, *pNumRuleTbl )
        delete pNumRule;
    pNumRuleTbl->clear();
    // creation of new outline numbering rule
    pOutlineRule = new SwNumRule( rtl::OUString::createFromAscii( SwNumRule::GetOutlineRuleName() ),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(pOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    pOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    // remove the dummy pagedec from the array and delete all the old ones
    aPageDescs.erase( aPageDescs.begin() + nDummyPgDsc );
    BOOST_FOREACH(SwPageDesc *pPageDesc, aPageDescs)
        delete pPageDesc;
    aPageDescs.clear();

    // Delete for Collections
    // So that we get rid of the dependencies
    pFtnInfo->ReleaseCollection();
    pEndNoteInfo->ReleaseCollection();

    // Optimization: Based on the fact that Standard is always 2nd in the
    // array, we should delete it as the last. With this we avoid
    // reparenting the Formats all the time!
    if( 2 < pTxtFmtCollTbl->size() )
        DeleteAndDestroy(*pTxtFmtCollTbl, 2, pTxtFmtCollTbl->size());
    DeleteAndDestroy(*pTxtFmtCollTbl, 1, pTxtFmtCollTbl->size());
    DeleteAndDestroy(*pGrfFmtCollTbl, 1, pGrfFmtCollTbl->size());
    DeleteAndDestroy(*pCharFmtTbl, 1, pCharFmtTbl->size());

    if( pCurrentView )
    {
        // search the FrameFormat of the root frm. This is not allowed to delete
        pFrmFmtTbl->erase( std::find( pFrmFmtTbl->begin(), pFrmFmtTbl->end(), pCurrentView->GetLayout()->GetFmt() ) );
        DeleteAndDestroy(*pFrmFmtTbl, 1, pFrmFmtTbl->size());
        pFrmFmtTbl->push_back( pCurrentView->GetLayout()->GetFmt() );
    }
    else    //swmod 071029//swmod 071225
        DeleteAndDestroy(*pFrmFmtTbl, 1, pFrmFmtTbl->size());

    xForbiddenCharsTable.clear();

    for(SwFldTypes::const_iterator it = pFldTypes->begin() + INIT_FLDTYPES;
        it != pFldTypes->end(); ++it)
        delete *it;
    pFldTypes->erase( pFldTypes->begin() + INIT_FLDTYPES, pFldTypes->end() );

    delete pNumberFormatter, pNumberFormatter = 0;

    GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    pFirstNd->ChgFmtColl( GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    nDummyPgDsc = aPageDescs.size();
    aPageDescs.push_back( pDummyPgDsc );
    // set the layout back to the new standard pagedesc
    pFirstNd->ResetAllAttr();
    // delete now the dummy pagedesc
    DelPageDesc( nDummyPgDsc );
}

void SwDoc::SetPreViewPrtData( const SwPagePreViewPrtData* pNew )
{
    if( pNew )
    {
        if( pPgPViewPrtData )
            *pPgPViewPrtData = *pNew;
        else
            pPgPViewPrtData = new SwPagePreViewPrtData( *pNew );
    }
    else if( pPgPViewPrtData )
        DELETEZ( pPgPViewPrtData );
    SetModified();
}

SwModify*   SwDoc::GetUnoCallBack() const
{
    return pUnoCallBack;
}


/** SwDoc:
 *  Reading and writing of the layout cache.
 *--------------------------------------------------*/
void SwDoc::ReadLayoutCache( SvStream& rStream )
{
    if( !pLayoutCache )
        pLayoutCache = new SwLayoutCache();
    if( !pLayoutCache->IsLocked() )
    {
        pLayoutCache->GetLockCount() |= 0x8000;
        pLayoutCache->Read( rStream );
        pLayoutCache->GetLockCount() &= 0x7fff;
    }
}

void SwDoc::WriteLayoutCache( SvStream& rStream )
{
    pLayoutCache->Write( rStream, *this );
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
void SwDoc::UpdateLinks( sal_Bool bUI )
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
            sal_Bool bAskUpdate = nLinkMode == MANUAL;
            sal_Bool bUpdate = sal_True;
            switch(nUpdateDocMode)
            {
                case document::UpdateDocMode::NO_UPDATE:   bUpdate = sal_False;break;
                case document::UpdateDocMode::QUIET_UPDATE:bAskUpdate = sal_False; break;
                case document::UpdateDocMode::FULL_UPDATE: bAskUpdate = sal_True; break;
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
                    GetLinkManager().UpdateAllLinks( bAskUpdate , sal_True, sal_False, pDlgParent );
                }
                else
                    GetLinkManager().UpdateAllLinks( bAskUpdate, sal_True, sal_False, pDlgParent );
            }
        }
    }

}

void SwDoc::SetApplyWorkaroundForB6375613( bool p_bApplyWorkaroundForB6375613 )
{
    if ( mbApplyWorkaroundForB6375613 != p_bApplyWorkaroundForB6375613 )
    {
        mbApplyWorkaroundForB6375613 = p_bApplyWorkaroundForB6375613;

        uno::Reference< document::XDocumentInfoSupplier > xDoc(
                                                GetDocShell()->GetBaseModel(),
                                                uno::UNO_QUERY);
        if ( xDoc.is() )
        {
            uno::Reference< beans::XPropertyContainer > xDocInfo(
                                                        xDoc->getDocumentInfo(),
                                                        uno::UNO_QUERY );
            if ( xDocInfo.is() )
            {
                try
                {
                    if ( mbApplyWorkaroundForB6375613 )
                    {
                        xDocInfo->addProperty(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkaroundForB6375613Applied")),
                            beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::REMOVABLE,
                            uno::makeAny( false ) );
                    }
                    else
                    {
                        xDocInfo->removeProperty( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WorkaroundForB6375613Applied")) );
                    }
                }
                catch( uno::Exception& )
                {
                }
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
   pTOXTypes->push_back( pNew );
   pNew = new SwTOXType(TOX_INDEX,                 pShellRes->aTOXIndexName  );
   pTOXTypes->push_back( pNew );
   pNew = new SwTOXType(TOX_USER,                  pShellRes->aTOXUserName  );
   pTOXTypes->push_back( pNew );
   pNew = new SwTOXType(TOX_ILLUSTRATIONS,         pShellRes->aTOXIllustrationsName );
   pTOXTypes->push_back( pNew );
   pNew = new SwTOXType(TOX_OBJECTS,               pShellRes->aTOXObjectsName       );
   pTOXTypes->push_back( pNew );
   pNew = new SwTOXType(TOX_TABLES,                pShellRes->aTOXTablesName        );
   pTOXTypes->push_back( pNew );
   pNew = new SwTOXType(TOX_AUTHORITIES,           pShellRes->aTOXAuthoritiesName   );
   pTOXTypes->push_back( pNew );
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
    n32DummyCompatabilityOptions1 = rSource.n32DummyCompatabilityOptions1;
    n32DummyCompatabilityOptions2 = rSource.n32DummyCompatabilityOptions2;
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

/*-------------------------------------------------------------------------
    copy document content - code from SwFEShell::Paste( SwDoc* , sal_Bool  )
  -----------------------------------------------------------------------*/
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
                sal_Bool bInsWithFmt = sal_True;
                const SwFrmFmt& rCpyFmt = *(*rSource.GetSpzFrmFmts())[i];
                if( bInsWithFmt  )
                {
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
