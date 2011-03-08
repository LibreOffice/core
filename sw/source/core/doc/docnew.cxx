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
#include "precompiled_sw.hxx"
#define ROLBCK_HISTORY_ONLY     // Der Kampf gegen die CLOOK's
#include <doc.hxx>
#include <dcontact.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/text/XFlatParagraphIteratorProvider.hpp>

#include <unotools/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/logfile.hxx>
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
#include <rootfrm.hxx>  //Damit der RootDtor gerufen wird.
#include <layouter.hxx>
#include <pagedesc.hxx> //Damit die PageDescs zerstoert werden koennen.
#include <ndtxt.hxx>
#include <printdata.hxx>
#include <docfld.hxx>
#include <ftninfo.hxx>
#include <ftnidx.hxx>
#include <docstat.hxx>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <rolbck.hxx>           // Undo-Attr, SwHistory
#include <poolfmt.hxx>          // fuer die Pool-Vorlage
#include <dbmgr.hxx>
#include <docsh.hxx>
#include <acorrect.hxx>         // fuer die autom. Aufnahme von Ausnahmen
#include <visiturl.hxx>         // fuer die URL-Change Benachrichtigung
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
#include <unochart.hxx>

#include <cmdid.h>              // fuer den dflt - Printer in SetJob


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

using namespace ::com::sun::star;
using namespace ::com::sun::star::document;

const sal_Char sFrmFmtStr[] = "Frameformat";
const sal_Char sEmptyPageStr[] = "Empty Page";
const sal_Char sColumnCntStr[] = "Columncontainer";
const sal_Char sCharFmtStr[] = "Zeichenformat";
const sal_Char sTxtCollStr[] = "Textformatvorlage";
const sal_Char sGrfCollStr[] = "Graphikformatvorlage";

SV_IMPL_PTRARR( SwNumRuleTbl, SwNumRulePtr)
SV_IMPL_PTRARR( SwTxtFmtColls, SwTxtFmtCollPtr)
SV_IMPL_PTRARR( SwGrfFmtColls, SwGrfFmtCollPtr)

/*
 * global functions...
 */
 uno::Reference< linguistic2::XProofreadingIterator > SwDoc::GetGCIterator() const
{
    if (!m_xGCIterator.is() && SvtLinguConfig().HasGrammarChecker())
    {
        uno::Reference< lang::XMultiServiceFactory >  xMgr( utl::getProcessServiceFactory() );
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
 * interne Funktionen
 */
BOOL lcl_DelFmtIndizes( const SwFrmFmtPtr& rpFmt, void* )
{
    SwFmtCntnt &rFmtCntnt = (SwFmtCntnt&)rpFmt->GetCntnt();
    if ( rFmtCntnt.GetCntntIdx() )
        rFmtCntnt.SetNewCntntIdx( 0 );
    SwFmtAnchor &rFmtAnchor = (SwFmtAnchor&)rpFmt->GetAnchor();
    if ( rFmtAnchor.GetCntntAnchor() )
        rFmtAnchor.SetAnchor( 0 );
    return TRUE;
}

/*
 * exportierte Methoden
 */
SwDoc::SwDoc() :
    aNodes( this ),
    aUndoNodes( this ),
    mpAttrPool(new SwAttrPool(this)),
    pMarkManager(new ::sw::mark::MarkManager(*this)),
    m_pMetaFieldManager(new ::sw::MetaFieldManager()),
    pDfltFrmFmt( new SwFrmFmt( GetAttrPool(), sFrmFmtStr, 0 ) ),
    pEmptyPageFmt( new SwFrmFmt( GetAttrPool(), sEmptyPageStr, pDfltFrmFmt ) ),
    pColumnContFmt( new SwFrmFmt( GetAttrPool(), sColumnCntStr, pDfltFrmFmt ) ),
    pDfltCharFmt( new SwCharFmt( GetAttrPool(), sCharFmtStr, 0 ) ),
    pDfltTxtFmtColl( new SwTxtFmtColl( GetAttrPool(), sTxtCollStr ) ),
    pDfltGrfFmtColl( new SwGrfFmtColl( GetAttrPool(), sGrfCollStr ) ),
    pFrmFmtTbl( new SwFrmFmts() ),
    pCharFmtTbl( new SwCharFmts() ),
    pSpzFrmFmtTbl( new SwSpzFrmFmts() ),
    pSectionFmtTbl( new SwSectionFmts() ),
    pTblFrmFmtTbl( new SwFrmFmts() ),
    pTxtFmtCollTbl( new SwTxtFmtColls() ),
    pGrfFmtCollTbl( new SwGrfFmtColls() ),
    pTOXTypes( new SwTOXTypes() ),
    pDefTOXBases( new SwDefTOXBase_Impl() ),
    pLayout( 0 ),                   // Rootframe des spezifischen Layouts.
    pDrawModel( 0 ),
    pUndos( new SwUndos( 0, 20 ) ),
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
    pDocShRef( 0 ),
    pLinkMgr( new sfx2::LinkManager( 0 ) ),
    pACEWord( 0 ),
    pURLStateChgd( 0 ),
    pNumberFormatter( 0 ),
    pNumRuleTbl( new SwNumRuleTbl ),
    maLists(),
    maListStyleLists(),
    pRedlineTbl( new SwRedlineTbl ),
    pAutoFmtRedlnComment( 0 ),
    pUnoCrsrTbl( new SwUnoCrsrTbl( 0, 16 ) ),
    pPgPViewPrtData( 0 ),
    pExtInputRing( 0 ),
    pLayouter( 0 ),
    pStyleAccess( 0 ),
    pLayoutCache( 0 ),
    pUnoCallBack(new SwModify(0)),
    mpGrammarContact( 0 ),
    aChartDataProviderImplRef(),
    pChartControllerHelper( 0 ),
    mpListItemsList( new tImplSortedNodeNumList() ), // #i83479#
    m_pXmlIdRegistry(),
    nUndoPos( 0 ),
    nUndoSavePos( 0 ),
    nUndoCnt( 0 ),
    nUndoSttEnd( 0 ),
    nAutoFmtRedlnCommentNo( 0 ),
    nLinkUpdMode( GLOBALSETTING ),
     eFldUpdMode( AUTOUPD_GLOBALSETTING ),
    eRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE)),
    eChrCmprType( CHARCOMPRESS_NONE ),
    mReferenceCount(0),
    mIdleBlockCount(0),
    nLockExpFld( 0 ),
    mbReadlineChecked(false),
    mbLinksUpdated( sal_False ), //#i38810#
    mbClipBoard( false ),
    mbColumnSelection( false ),
    mbProtectForm(false), // i#78591#
    n32DummyCompatabilityOptions1(0),
    n32DummyCompatabilityOptions2(0),
    mbStartIdleTimer(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::SwDoc" );

    mbGlossDoc =
    mbModified =
    mbDtor =
    mbUndo =
    mbPageNums =
    mbLoaded =
    mbUpdateExpFld =
    mbNewDoc =
    mbCopyIsMove =
    mbNoDrawUndoObj =
    mbBrowseMode =
    mbInReading =
    mbInXMLImport =
    mbUpdateTOX =
    mbInLoadAsynchron =
    mbHTMLMode =
    mbInCallModified =
    mbIsGlobalDoc =
    mbGlblDocSaveLinks =
    mbIsLabelDoc =
    mbIsAutoFmtRedline =
    mbOLEPrtNotifyPending =
    mbAllOLENotify =
    mbIsRedlineMove =
    mbInsOnlyTxtGlssry =
    mbContains_MSVBasic =
    mbKernAsianPunctuation =
#if OSL_DEBUG_LEVEL > 1
    mbXMLExport =
#endif
    mbApplyWorkaroundForB6375613 =
                            false;

    mbGroupUndo =
    mbNewFldLst =
    mbVisibleLinks =
    mbPurgeOLE =
                            true;

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
    mbAddFlyOffsets                         = false;        // hidden
    mbOldNumbering                          = false;        // hidden
    mbUseHiResolutionVirtualDevice          = true;         // hidden
    mbIgnoreFirstLineIndentInNumbering      = false;        // hidden
    mbDoNotJustifyLinesWithManualBreak      = !aOptions.IsExpandWordSpace();
    mbDoNotResetParaAttrsForNumFont         = false;        // hidden
    mbOutlineLevelYieldsOutlineRule         = false;        // hidden
    mbTableRowKeep                          = false;        // hidden
    mbIgnoreTabsAndBlanksForLineCalculation = false;        // hidden
    mbDoNotCaptureDrawObjsOnPage            = false;        // hidden
    mbClipAsCharacterAnchoredWriterFlyFrames= false;        // hidden
    mbUnixForceZeroExtLeading               = false;        // hidden
    mbOldPrinterMetrics                     = false;        // hidden
    mbTabRelativeToIndent                   = true;         // hidden
    mbTabAtLeftIndentForParagraphsInList    = false;        // hidden #i89181#
    mbInvertBorderSpacing                   = false;        // hidden
    mbCollapseEmptyCellPara                 = true;        // hidden

    //
    // COMPATIBILITY FLAGS END
    //

    pMacroTable = new SvxMacroTableDtor;

    mpGrammarContact = ::createGrammarContact();

    /*
     * Defaultformate und DefaultFormatsammlungen (FmtColl)
     * werden an der Position 0 in das jeweilige Array eingetragen.
     * Die Formate der FmtColls sind von den Defaultformaten
     * abgeleitet und stehen auch in der Liste.
     */
    /* Formate */
    pFrmFmtTbl->Insert(pDfltFrmFmt, 0 );
    pCharFmtTbl->Insert(pDfltCharFmt, 0 );

    /* FmtColls */
    // TXT
    pTxtFmtCollTbl->Insert(pDfltTxtFmtColl, 0 );
    // GRF
    pGrfFmtCollTbl->Insert(pDfltGrfFmtColl, 0 );

    // PageDesc, EmptyPageFmt und ColumnFmt anlegen
    if ( !aPageDescs.Count() )
        GetPageDescFromPool( RES_POOLPAGE_STANDARD );

        //Leere Seite Einstellen.
    pEmptyPageFmt->SetFmtAttr( SwFmtFrmSize( ATT_FIX_SIZE ) );
        //BodyFmt fuer Spalten Einstellen.
    pColumnContFmt->SetFmtAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );

    _InitFieldTypes();

    // lege (fuer die Filter) eine Default-OutlineNumRule an
    pOutlineRule = new SwNumRule( String::CreateFromAscii( SwNumRule::GetOutlineRuleName() ),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(pOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    pOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    new SwTxtNode( SwNodeIndex( aUndoNodes.GetEndOfContent() ), pDfltTxtFmtColl );
    new SwTxtNode( SwNodeIndex( aNodes.GetEndOfContent() ),
                    GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    // den eigenen IdleTimer setzen
    aIdleTimer.SetTimeout( 600 );
    aIdleTimer.SetTimeoutHdl( LINK(this, SwDoc, DoIdleJobs) );

    aOLEModifiedTimer.SetTimeout( 1000 );
    aOLEModifiedTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateModifiedOLE ));

    // DBMgr anlegen
    pNewDBMgr = new SwNewDBMgr;

    // create TOXTypes
    InitTOXTypes();

    // pass empty item set containing the paragraph's list attributes
    // as ignorable items to the stype manager.
    {
        SfxItemSet aIgnorableParagraphItems( GetAttrPool(),
                                             RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
                                             0 );
        pStyleAccess = createStyleManager( &aIgnorableParagraphItems );
    }

    ResetModified();
}

/*
 * Besonderheiten: an der Position 0 des Arrays der Formate und
 * der GDI-Objekte befindet sich ein Member der Klasse SwDoc.
 * Dieser darf also keinesfalls durch delete geloescht
 * werden!!!!!!!!!!
 */
SwDoc::~SwDoc()
{
    SwPauseThreadStarting aPauseThreadStarting; // #i73788#

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

    // Undo-Benachrichtigung vom Draw abschalten
    if( pDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    delete pPgPViewPrtData;

    mbUndo = FALSE;         // immer das Undo abschalten !!
    // damit die Fussnotenattribute die Fussnotennodes in Frieden lassen.
    mbDtor = TRUE;

    DELETEZ( pLayout );

    delete pRedlineTbl;
    delete pUnoCrsrTbl;
    delete pAutoFmtRedlnComment;

    if( pUpdtFlds )
        delete pUpdtFlds;

    if( pACEWord )
        delete pACEWord;

    // die BaseLinks freigeben.
    {
        for( USHORT n = pLinkMgr->GetServers().Count(); n; )
            pLinkMgr->GetServers()[ --n ]->Closed();

        if( pLinkMgr->GetLinks().Count() )
            pLinkMgr->Remove( 0, pLinkMgr->GetLinks().Count() );
    }

    // die KapitelNummern / Nummern muessen vor den Vorlage geloescht werden
    // ansonsten wird noch staendig geupdatet !!!
    aNodes.pOutlineNds->Remove( USHORT(0), aNodes.pOutlineNds->Count() );
    aUndoNodes.pOutlineNds->Remove( USHORT(0), aUndoNodes.pOutlineNds->Count() );

    pFtnIdxs->Remove( USHORT(0), pFtnIdxs->Count() );

    pUndos->DeleteAndDestroy( 0, pUndos->Count() ); //Es koennen in den Attributen noch
                                                    //noch indizes angemeldet sein.

    // in den BookMarks sind Indizies auf den Content. Diese muessen vorm
    // loesche der Nodes geloescht werden.
    pMarkManager->clearAllMarks();
    DELETEZ( pMacroTable );

    if( pExtInputRing )
    {
        Ring* pTmp = pExtInputRing;
        pExtInputRing = 0;
        while( pTmp->GetNext() != pTmp )
            delete pTmp->GetNext();
        delete pTmp;
    }

//JP: alt - loeschen ohne Flag ist teuer; Modify wird verschickt!
//  aTOXTypes.DeleteAndDestroy( 0, aTOXTypes.Count() );
    {
        for( USHORT n = pTOXTypes->Count(); n; )
        {
            (*pTOXTypes)[ --n ]->SetInDocDTOR();
            delete (*pTOXTypes)[ n ];
        }
        pTOXTypes->Remove( 0, pTOXTypes->Count() );
    }
    delete pDefTOXBases;

    //Im einen oder anderen FrmFormat koennen noch Indizes angemeldet sein,
    //Diese muessen spaetestens jetzt zerstoert werden.
    pFrmFmtTbl->ForEach( &lcl_DelFmtIndizes, this );
    pSpzFrmFmtTbl->ForEach( &lcl_DelFmtIndizes, this );
    ((SwFrmFmts&)*pSectionFmtTbl).ForEach( &lcl_DelFmtIndizes, this );

    //Die Formate, die hier hinter stehen sind von den DefaultFormaten
    //abhaengig. Erst nach dem Loeschen der FmtIndizes weil der Inhalt von
    //Kopf-/Fussbereichen geloescht wird. Wenn dort noch Indizes von Flys
    //angemeldet sind gibts was an die Ohren.
    aPageDescs.DeleteAndDestroy( 0, aPageDescs.Count() );

    // Inhaltssections loeschen
    // nicht erst durch den SwNodes-DTOR, damit Formate
    // keine Abhaengigen mehr haben.
    aNodes.DelNodes( SwNodeIndex( aNodes ), aNodes.Count() );
    aUndoNodes.DelNodes( SwNodeIndex( aUndoNodes ), aUndoNodes.Count() );

    // Formate loeschen, spaeter mal permanent machen.

    // Delete fuer Collections
    // damit die Abhaengigen wech sind
    SwTxtFmtColl *pFtnColl = pFtnInfo->GetFtnTxtColl();
    if ( pFtnColl ) pFtnColl->Remove(pFtnInfo);
    pFtnColl = pEndNoteInfo->GetFtnTxtColl();
    if ( pFtnColl ) pFtnColl->Remove(pEndNoteInfo);

    OSL_ENSURE( pDfltTxtFmtColl == (*pTxtFmtCollTbl)[0],
            "Default-Text-Collection muss immer am Anfang stehen" );

    // JP 27.01.98: opt.: ausgehend davon, das Standard als 2. im Array
    //              steht, sollte das als letztes geloescht werden, damit
    //              die ganze Umhaengerei der Formate vermieden wird!
    if( 2 < pTxtFmtCollTbl->Count() )
        pTxtFmtCollTbl->DeleteAndDestroy( 2, pTxtFmtCollTbl->Count()-2 );
    pTxtFmtCollTbl->DeleteAndDestroy( 1, pTxtFmtCollTbl->Count()-1 );
    delete pTxtFmtCollTbl;

    OSL_ENSURE( pDfltGrfFmtColl == (*pGrfFmtCollTbl)[0],
            "Default-Grf-Collection muss immer am Anfang stehen" );

    pGrfFmtCollTbl->DeleteAndDestroy( 1, pGrfFmtCollTbl->Count()-1 );
// ergibt sich automatisch - kein _DEL Array!
//  pGrfFmtCollTbl->Remove( 0, n );
    delete pGrfFmtCollTbl;

    /*
     * Defaultformate und DefaultFormatsammlungen (FmtColl)
     * sind an der Position 0 der jeweiligen Arrays eingetragen.
     * Damit sie nicht vom DTOR der Array's zum 2.mal geloescht werden,
     * nehme sie aus dem Array.
     */
    pFrmFmtTbl->Remove( 0 );
    pCharFmtTbl->Remove( 0 );

    // Delete fuer pPrt
    DELETEZ( pPrt );
    DELETEZ( pNewDBMgr );

    // Alle Flys muessen vor dem Drawing Model zerstoert werden,
    // da Flys noch DrawContacts enthalten koennen, wenn wegen
    // eines Lesefehlers kein Layout aufgebaut wurde.
    pSpzFrmFmtTbl->DeleteAndDestroy( 0, pSpzFrmFmtTbl->Count() );

    //Erst jetzt das Model zerstoeren, die Zeichenobjekte - die ja auch
    //im Undo herumlungern - wollen noch ihre Attribute beim Model entfernen.
    //Ausserdem koennen vorher noch DrawContacts existieren.
    ReleaseDrawModel();
    //JP 28.01.99: DrawModel vorm LinkManager zerstoeren, da am DrawModel
    //          dieser immer gesetzt ist.
    DELETEZ( pLinkMgr );

    //Tables vor dem loeschen der Defaults leeren, sonst GPF wegen Def-Abhaengigen.
    //Die Arrays sollten (wegen includes) bei Gelegenheit auch zu Pointern werden.
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
    delete pUndos;
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
    OSL_ENSURE( false, "Printer will be created!" );
#endif

    // wir erzeugen einen default SfxPrinter.
    // Das ItemSet wird vom Sfx geloescht!
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
        pDocShell = pDSh;

        pLinkMgr->SetPersist( pDocShell );
        //JP 27.08.98: Bug 55570 - DocShell Pointer auch am DrawModel setzen
        if( pDrawModel )
        {
            ((SwDrawDocument*)pDrawModel)->SetObjectShell( pDocShell );
            pDrawModel->SetPersist( pDocShell );
            OSL_ENSURE( pDrawModel->GetPersist() == GetPersist(),
                    "draw model's persist is out of sync" );
        }
    }
}

// Convenience-Methode, um uebermaessige Includes von docsh.hxx
// zu vermeiden
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
    BOOL bOldUndo = mbUndo;
    DelAllUndoObj();
    mbUndo = FALSE;         // immer das Undo abschalten !!

    // Undo-Benachrichtigung vom Draw abschalten
    if( pDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    // stehen noch FlyFrames rum, loesche auch diese
    USHORT n;
    while ( 0 != (n = GetSpzFrmFmts()->Count()) )
        DelLayoutFmt((*pSpzFrmFmtTbl)[n-1]);
    OSL_ENSURE( !pDrawModel || !pDrawModel->GetPage(0)->GetObjCount(),
                "not all DrawObjects removed from the page" );

    pRedlineTbl->DeleteAndDestroy( 0, pRedlineTbl->Count() );

    if( pACEWord )
        delete pACEWord;

    // in den BookMarks sind Indizies auf den Content. Diese muessen vorm
    // loesche der Nodes geloescht werden.
    pMarkManager->clearAllMarks();
    InitTOXTypes();

    // create a dummy pagedesc for the layout
    sal_uInt16 nDummyPgDsc = MakePageDesc( String::CreateFromAscii( "?DUMMY?" ));
    SwPageDesc* pDummyPgDsc = aPageDescs[ nDummyPgDsc ];

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // den ersten immer wieder neu anlegen (ohne Attribute/Vorlagen/...)
    SwTxtNode* pFirstNd = GetNodes().MakeTxtNode( aSttIdx, pDfltTxtFmtColl );

    if( pLayout )
    {
        // set the layout to the dummy pagedesc
        pFirstNd->SetAttr( SwFmtPageDesc( pDummyPgDsc ));

        SwPosition aPos( *pFirstNd, SwIndex( pFirstNd ));
        ::PaMCorrAbs( aSttIdx, SwNodeIndex( GetNodes().GetEndOfContent() ),
                         aPos );
    }

    GetNodes().Delete( aSttIdx,
            GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );

    // #i62440#
    // destruction of numbering rules and creation of new outline rule
    // *after* the document nodes are deleted.
    pOutlineRule = NULL;
    pNumRuleTbl->DeleteAndDestroy( 0, pNumRuleTbl->Count() );
    // creation of new outline numbering rule
    pOutlineRule = new SwNumRule( String::CreateFromAscii( SwNumRule::GetOutlineRuleName() ),
                                  // #i89178#
                                  numfunc::GetDefaultPositionAndSpaceMode(),
                                  OUTLINE_RULE );
    AddNumRule(pOutlineRule);
    // Counting of phantoms depends on <IsOldNumbering()>
    pOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );

    //remove the dummy pagedec from the array and delete all the old ones
    aPageDescs.Remove( nDummyPgDsc );
    aPageDescs.DeleteAndDestroy( 0, aPageDescs.Count() );

    // Delete fuer Collections
    // damit die Abhaengigen wech sind
    SwTxtFmtColl* pFtnColl = pFtnInfo->GetFtnTxtColl();
    if( pFtnColl ) pFtnColl->Remove( pFtnInfo );
    pFtnColl = pEndNoteInfo->GetFtnTxtColl();
    if( pFtnColl ) pFtnColl->Remove( pEndNoteInfo );

    // opt.: ausgehend davon, das Standard als 2. im Array
    // steht, sollte das als letztes geloescht werden, damit
    // die ganze Umhaengerei der Formate vermieden wird!
    if( 2 < pTxtFmtCollTbl->Count() )
        pTxtFmtCollTbl->DeleteAndDestroy( 2, pTxtFmtCollTbl->Count()-2 );
    pTxtFmtCollTbl->DeleteAndDestroy( 1, pTxtFmtCollTbl->Count()-1 );
    pGrfFmtCollTbl->DeleteAndDestroy( 1, pGrfFmtCollTbl->Count()-1 );
    pCharFmtTbl->DeleteAndDestroy( 1, pCharFmtTbl->Count()-1 );

    if( pLayout )
    {
        // search the FrameFormat of the root frm. This is not allowed to delete
        pFrmFmtTbl->Remove( pFrmFmtTbl->GetPos( pLayout->GetFmt() ) );
        pFrmFmtTbl->DeleteAndDestroy( 1, pFrmFmtTbl->Count()-1 );
        pFrmFmtTbl->Insert( pLayout->GetFmt(), pFrmFmtTbl->Count() );
    }
    else
        pFrmFmtTbl->DeleteAndDestroy( 1, pFrmFmtTbl->Count()-1 );

    xForbiddenCharsTable.clear();

    pFldTypes->DeleteAndDestroy( INIT_FLDTYPES,
                                pFldTypes->Count() - INIT_FLDTYPES );

    delete pNumberFormatter, pNumberFormatter = 0;

    GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    pFirstNd->ChgFmtColl( GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    nDummyPgDsc = aPageDescs.Count();
    aPageDescs.Insert( pDummyPgDsc, nDummyPgDsc );
    // set the layout back to the new standard pagedesc
    pFirstNd->ResetAllAttr();
    // delete now the dummy pagedesc
    DelPageDesc( nDummyPgDsc );

    mbUndo = bOldUndo;
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
void SwDoc::UpdateLinks( BOOL bUI )
{
    SfxObjectCreateMode eMode;
    USHORT nLinkMode = getLinkUpdateMode( true );
    USHORT nUpdateDocMode = GetDocShell()->GetUpdateDocMode();
    if( GetDocShell() &&
            (nLinkMode != NEVER ||  document::UpdateDocMode::FULL_UPDATE == nUpdateDocMode) &&
        GetLinkManager().GetLinks().Count() &&
        SFX_CREATE_MODE_INTERNAL !=
                    ( eMode = GetDocShell()->GetCreateMode()) &&
        SFX_CREATE_MODE_ORGANIZER != eMode &&
        SFX_CREATE_MODE_PREVIEW != eMode &&
        !GetDocShell()->IsPreview() )
    {
        ViewShell* pVSh = 0;
        BOOL bAskUpdate = nLinkMode == MANUAL;
        BOOL bUpdate = TRUE;
        switch(nUpdateDocMode)
        {
            case document::UpdateDocMode::NO_UPDATE:   bUpdate = FALSE;break;
            case document::UpdateDocMode::QUIET_UPDATE:bAskUpdate = FALSE; break;
            case document::UpdateDocMode::FULL_UPDATE: bAskUpdate = TRUE; break;
        }
        if( bUpdate && (bUI || !bAskUpdate) )
        {
            SfxMedium* pMedium = GetDocShell()->GetMedium();
            SfxFrame* pFrm = pMedium ? pMedium->GetLoadTargetFrame() : 0;
            Window* pDlgParent = pFrm ? &pFrm->GetWindow() : 0;
            if( GetRootFrm() && !GetEditShell( &pVSh ) && !pVSh )
            {
                ViewShell aVSh( *this, 0, 0 );

                SET_CURR_SHELL( &aVSh );
                GetLinkManager().UpdateAllLinks( bAskUpdate , TRUE, FALSE, pDlgParent );
            }
            else
                GetLinkManager().UpdateAllLinks( bAskUpdate, TRUE, FALSE, pDlgParent );
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

void SwDoc::InitTOXTypes()
{
   ShellResource* pShellRes = ViewShell::GetShellRes();
   SwTOXType * pNew = new SwTOXType(TOX_CONTENT,   pShellRes->aTOXContentName        );
   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
   pNew = new SwTOXType(TOX_INDEX,                 pShellRes->aTOXIndexName  );
   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
   pNew = new SwTOXType(TOX_USER,                  pShellRes->aTOXUserName  );
   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
   pNew = new SwTOXType(TOX_ILLUSTRATIONS,         pShellRes->aTOXIllustrationsName );
   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
   pNew = new SwTOXType(TOX_OBJECTS,               pShellRes->aTOXObjectsName       );
   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
   pNew = new SwTOXType(TOX_TABLES,                pShellRes->aTOXTablesName        );
   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
   pNew = new SwTOXType(TOX_AUTHORITIES,           pShellRes->aTOXAuthoritiesName   );
   pTOXTypes->Insert( pNew, pTOXTypes->Count() );
}

SfxObjectShell* SwDoc::CreateCopy(bool bCallInitNew ) const
{
    SwDoc* pRet = new SwDoc;
    //copy settings
    USHORT aRangeOfDefaults[] = {
        RES_FRMATR_BEGIN, RES_FRMATR_END-1,
        RES_CHRATR_BEGIN, RES_CHRATR_END-1,
        RES_PARATR_BEGIN, RES_PARATR_END-1,
        RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
        0
    };

    SfxItemSet aNewDefaults( pRet->GetAttrPool(), aRangeOfDefaults );

    USHORT nWhich;
    USHORT nRange = 0;
    while( aRangeOfDefaults[nRange] != 0)
    {
        for( nWhich = aRangeOfDefaults[nRange]; nWhich < aRangeOfDefaults[nRange + 1]; ++nWhich )
        {
            const SfxPoolItem& rSourceAttr = mpAttrPool->GetDefaultItem( nWhich );
            if( rSourceAttr != pRet->mpAttrPool->GetDefaultItem( nWhich ) )
                aNewDefaults.Put( rSourceAttr );
        }
        nRange += 2;
    }
    if( aNewDefaults.Count() )
        pRet->SetDefault( aNewDefaults );

    pRet->n32DummyCompatabilityOptions1 = n32DummyCompatabilityOptions1;
    pRet->n32DummyCompatabilityOptions2 = n32DummyCompatabilityOptions2;
    pRet->mbParaSpaceMax                          = mbParaSpaceMax                          ;
    pRet->mbParaSpaceMaxAtPages                   = mbParaSpaceMaxAtPages                   ;
    pRet->mbTabCompat                             = mbTabCompat                             ;
    pRet->mbUseVirtualDevice                      = mbUseVirtualDevice                      ;
    pRet->mbAddExternalLeading                    = mbAddExternalLeading                    ;
    pRet->mbOldLineSpacing                        = mbOldLineSpacing                        ;
    pRet->mbAddParaSpacingToTableCells            = mbAddParaSpacingToTableCells            ;
    pRet->mbUseFormerObjectPos                    = mbUseFormerObjectPos                    ;
    pRet->mbUseFormerTextWrapping                 = mbUseFormerTextWrapping                 ;
    pRet->mbConsiderWrapOnObjPos                  = mbConsiderWrapOnObjPos                  ;
    pRet->mbAddFlyOffsets                         = mbAddFlyOffsets                         ;
    pRet->mbOldNumbering                          = mbOldNumbering                          ;
    pRet->mbUseHiResolutionVirtualDevice          = mbUseHiResolutionVirtualDevice          ;
    pRet->mbIgnoreFirstLineIndentInNumbering      = mbIgnoreFirstLineIndentInNumbering      ;
    pRet->mbDoNotJustifyLinesWithManualBreak      = mbDoNotJustifyLinesWithManualBreak      ;
    pRet->mbDoNotResetParaAttrsForNumFont         = mbDoNotResetParaAttrsForNumFont         ;
    pRet->mbOutlineLevelYieldsOutlineRule         = mbOutlineLevelYieldsOutlineRule         ;
    pRet->mbTableRowKeep                          = mbTableRowKeep                          ;
    pRet->mbIgnoreTabsAndBlanksForLineCalculation = mbIgnoreTabsAndBlanksForLineCalculation ;
    pRet->mbDoNotCaptureDrawObjsOnPage            = mbDoNotCaptureDrawObjsOnPage            ;
    pRet->mbClipAsCharacterAnchoredWriterFlyFrames= mbClipAsCharacterAnchoredWriterFlyFrames;
    pRet->mbUnixForceZeroExtLeading               = mbUnixForceZeroExtLeading               ;
    pRet->mbOldPrinterMetrics                     = mbOldPrinterMetrics                     ;
    pRet->mbTabRelativeToIndent                   = mbTabRelativeToIndent                   ;
    pRet->mbTabAtLeftIndentForParagraphsInList    = mbTabAtLeftIndentForParagraphsInList    ;

    //
    // COMPATIBILITY FLAGS END
    //
    pRet->ReplaceStyles( * const_cast< SwDoc*>( this ));
    SfxObjectShellRef aDocShellRef = const_cast< SwDocShell* >( GetDocShell() );
    pRet->SetRefForDocShell( boost::addressof(aDocShellRef) );
    SfxObjectShellRef xRetShell = new SwDocShell( pRet, SFX_CREATE_MODE_STANDARD );
    if( bCallInitNew )
        xRetShell->DoInitNew();
    //copy content
    pRet->Paste( *this );
    pRet->SetRefForDocShell( 0 );
    return xRetShell;
}

/*-- 08.05.2009 10:52:40---------------------------------------------------
    copy document content - code from SwFEShell::Paste( SwDoc* , BOOL  )
  -----------------------------------------------------------------------*/
void SwDoc::Paste( const SwDoc& rSource )
{
//  this has to be empty const USHORT nStartPageNumber = GetPhyPageNum();
    // until the end of the NodesArray
    SwNodeIndex aSourceIdx( rSource.GetNodes().GetEndOfExtras(), 2 );
    SwPaM aCpyPam( aSourceIdx ); //DocStart
    SwNodeIndex aTargetIdx( GetNodes().GetEndOfExtras(), 2 );
    SwPaM aInsertPam( aTargetIdx ); //replaces PCURCRSR from SwFEShell::Paste()


    aCpyPam.SetMark();
    aCpyPam.Move( fnMoveForward, fnGoDoc );

    this->StartUndo( UNDO_INSGLOSSARY, NULL );
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
                aIndexBefore++;
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
        if( /*bIncludingPageFrames && */rSource.GetSpzFrmFmts()->Count() )
        {
            for ( USHORT i = 0; i < rSource.GetSpzFrmFmts()->Count(); ++i )
            {
                BOOL bInsWithFmt = TRUE;
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

    this->EndUndo( UNDO_INSGLOSSARY, NULL );

    UnlockExpFlds();
    UpdateFlds(NULL, false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
