/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docnew.cxx,v $
 *
 *  $Revision: 1.82 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:41:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#define ROLBCK_HISTORY_ONLY     // Der Kampf gegen die CLOOK's

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_PRINTERINDEPENDENTLAYOUT_HPP_
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif

#ifndef _SFXMACITEM_HXX //autogen
    #include <svtools/macitem.hxx>
#endif
#include <svtools/stylepool.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COMPATIBILITY_HXX
#include <svtools/compatibility.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFORDR_HXX
#include <fmtfordr.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _PVPRTDAT_HXX
#include <pvprtdat.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>  //Damit der RootDtor gerufen wird.
#endif
#ifndef _LAYOUTER_HXX
#include <layouter.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> //Damit die PageDescs zerstoert werden koennen.
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SW_PRINTDATA_HXX
#include <printdata.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>           // Undo-Attr, SwHistory
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>          // fuer die Pool-Vorlage
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _ACORRECT_HXX
#include <acorrect.hxx>         // fuer die autom. Aufnahme von Ausnahmen
#endif
#ifndef _VISITURL_HXX
#include <visiturl.hxx>         // fuer die URL-Change Benachrichtigung
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _EXTINPUT_HXX
#include <extinput.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _UNOCLBCK_HXX
#include <unoclbck.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _LAYCACHE_HXX
#include <laycache.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _ISTYLEACCESS_HXX
#include <istyleaccess.hxx>
#endif
#include <swstylemanager.hxx>

#include <unochart.hxx>

#ifndef _CMDID_H
#include <cmdid.h>              // fuer den dflt - Printer in SetJob
#endif


// --> OD 2006-04-19 #b6375613#
#ifndef  _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

using namespace ::com::sun::star;
// <--

// --> OD 2007-03-16 #i73788#
#ifndef _PAUSETHREADSTARTING_HXX
#include <pausethreadstarting.hxx>
#endif
// <--

using namespace ::com::sun::star::document;

const sal_Char __FAR_DATA sFrmFmtStr[] = "Frameformat";
const sal_Char __FAR_DATA sEmptyPageStr[] = "Empty Page";
const sal_Char __FAR_DATA sColumnCntStr[] = "Columncontainer";
const sal_Char __FAR_DATA sCharFmtStr[] = "Zeichenformat";
const sal_Char __FAR_DATA sTxtCollStr[] = "Textformatvorlage";
const sal_Char __FAR_DATA sGrfCollStr[] = "Graphikformatvorlage";

SV_IMPL_PTRARR( SwNumRuleTbl, SwNumRulePtr)
SV_IMPL_PTRARR( SwTxtFmtColls, SwTxtFmtCollPtr)
SV_IMPL_PTRARR( SwGrfFmtColls, SwGrfFmtCollPtr)


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
    aAttrPool( this ),
    pDfltFrmFmt( new SwFrmFmt( aAttrPool, sFrmFmtStr, 0 ) ),
    pEmptyPageFmt( new SwFrmFmt( aAttrPool, sEmptyPageStr, pDfltFrmFmt ) ),
    pColumnContFmt( new SwFrmFmt( aAttrPool, sColumnCntStr, pDfltFrmFmt ) ),
    pDfltCharFmt( new SwCharFmt( aAttrPool, sCharFmtStr, 0 ) ),
    pDfltTxtFmtColl( new SwTxtFmtColl( aAttrPool, sTxtCollStr ) ),
    pDfltGrfFmtColl( new SwGrfFmtColl( aAttrPool, sGrfCollStr ) ),
    pFrmFmtTbl( new SwFrmFmts() ),
    pCharFmtTbl( new SwCharFmts() ),
    pSpzFrmFmtTbl( new SwSpzFrmFmts() ),
    pSectionFmtTbl( new SwSectionFmts() ),
    pTblFrmFmtTbl( new SwFrmFmts() ),
    pTxtFmtCollTbl( new SwTxtFmtColls() ),
    pGrfFmtCollTbl( new SwGrfFmtColls() ),
    pBookmarkTbl( new SwBookmarks( 0, 16 ) ),
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
    pSwgInfo( 0 ),
    pDocShell( 0 ),
    pDocShRef( 0 ),
    pLinkMgr( new SvxLinkManager( 0 ) ),
    pACEWord( 0 ),
    pURLStateChgd( 0 ),
    pNumberFormatter( 0 ),
    pNumRuleTbl( new SwNumRuleTbl ),
    pRedlineTbl( new SwRedlineTbl ),
    pAutoFmtRedlnComment( 0 ),
    pUnoCrsrTbl( new SwUnoCrsrTbl( 0, 16 ) ),
    pPgPViewPrtData( 0 ),
    pExtInputRing( 0 ),
    pLayouter( 0 ),
    pStyleAccess( createStyleManager() ),
    pLayoutCache( 0 ),
    pUnoCallBack(new SwUnoCallBack(0)),
    aChartDataProviderImplRef(),
    pChartControllerHelper( 0 ),
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
    mbWinEncryption(sal_False),
    // --> OD 2005-02-11 #i38810#
    mbLinksUpdated( sal_False ),
    mbClipBoard( false ),
    mbColumnSelection( false ),
    // i#78591#
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
#ifndef PRODUCT
    mbXMLExport =
#endif
    // --> OD 2006-03-21 #b6375613#
    mbApplyWorkaroundForB6375613 =
    // <--
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

    //
    // COMPATIBILITY FLAGS END
    //

    pMacroTable = new SvxMacroTableDtor;

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
    pEmptyPageFmt->SetAttr( SwFmtFrmSize( ATT_FIX_SIZE ) );
        //BodyFmt fuer Spalten Einstellen.
    pColumnContFmt->SetAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );

    _InitFieldTypes();

    // lege (fuer die Filter) eine Default-OutlineNumRule an
    pOutlineRule = new SwNumRule( String::CreateFromAscii(
                                        SwNumRule::GetOutlineRuleName() ),
                                        OUTLINE_RULE );
    // #115901#
    AddNumRule(pOutlineRule);
    // --> OD 2005-10-21 - counting of phantoms depends on <IsOldNumbering()>
    pOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );
    // <--

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
    // --> OD 2007-03-16 #i73788#
    SwPauseThreadStarting aPauseThreadStarting;
    // <--

    // clean up chart related structures...
    // Note: the chart data provider gets already diposed in ~SwDocShell
    // since all UNO API related functionality requires an existing SwDocShell
    // this assures that dipose gets called if there is need for it.
    aChartDataProviderImplRef.reset();
    delete pChartControllerHelper;

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
    // --> OD 2005-09-05 #125370#
    pLayouter = 0L;
    // <--

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
    pBookmarkTbl->DeleteAndDestroy( 0, pBookmarkTbl->Count() );
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

    ASSERT( pDfltTxtFmtColl == (*pTxtFmtCollTbl)[0],
            "Default-Text-Collection muss immer am Anfang stehen" );

    // JP 27.01.98: opt.: ausgehend davon, das Standard als 2. im Array
    //              steht, sollte das als letztes geloescht werden, damit
    //              die ganze Umhaengerei der Formate vermieden wird!
    if( 2 < pTxtFmtCollTbl->Count() )
        pTxtFmtCollTbl->DeleteAndDestroy( 2, pTxtFmtCollTbl->Count()-2 );
    pTxtFmtCollTbl->DeleteAndDestroy( 1, pTxtFmtCollTbl->Count()-1 );
    delete pTxtFmtCollTbl;

    ASSERT( pDfltGrfFmtColl == (*pGrfFmtCollTbl)[0],
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
    DELETEZ( pSwgInfo );
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

    delete pPrtData;
    delete pBookmarkTbl;
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

}

//---------------------------------------------------

VirtualDevice& SwDoc::CreateVirtualDevice_() const
{
    VirtualDevice* pNewVir = new VirtualDevice( 1 );

    // <--
    pNewVir->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );

    // --> FME 2006-10-09 #i60945# External leading compatibility for unix systems.
    if ( get(IDocumentSettingAccess::UNIX_FORCE_ZERO_EXT_LEADING ) )
        pNewVir->Compat_ZeroExtleadBug();
    // <--

    MapMode aMapMode( pNewVir->GetMapMode() );
    aMapMode.SetMapUnit( MAP_TWIP );
    pNewVir->SetMapMode( aMapMode );

    const_cast<SwDoc*>(this)->setVirtualDevice( pNewVir, true, true );
    return *pVirDev;
}

//---------------------------------------------------

SfxPrinter& SwDoc::CreatePrinter_() const
{
    ASSERT( ! pPrt, "Do not call CreatePrinter_(), call getPrinter() instead" )

#if OSL_DEBUG_LEVEL > 1
    ASSERT( false, "Printer will be created!" )
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
//---------------------------------------------------

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
            ASSERT( pDrawModel->GetPersist() == GetPersist(),
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



const SfxDocumentInfo* SwDoc::GetpInfo() const
{
    return pSwgInfo;
}

SfxDocumentInfo* SwDoc::GetDocumentInfo()
{
    if( !pSwgInfo )
        pSwgInfo = new SfxDocumentInfo;
    return pSwgInfo;
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
    ASSERT( !pDrawModel || !pDrawModel->GetPage(0)->GetObjCount(),
                "not all DrawObjects removed from the page" );

    pRedlineTbl->DeleteAndDestroy( 0, pRedlineTbl->Count() );

    if( pACEWord )
        delete pACEWord;

    // in den BookMarks sind Indizies auf den Content. Diese muessen vorm
    // loesche der Nodes geloescht werden.
    pBookmarkTbl->DeleteAndDestroy( 0, pBookmarkTbl->Count() );
    pTOXTypes->DeleteAndDestroy( 0, pTOXTypes->Count() );

    // create a dummy pagedesc for the layout
    sal_uInt16 nDummyPgDsc = MakePageDesc( String::CreateFromAscii( "?DUMMY?" ));
    SwPageDesc* pDummyPgDsc = aPageDescs[ nDummyPgDsc ];

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // den ersten immer wieder neu anlegen (ohne Attribute/Vorlagen/...)
    SwTxtNode* pFirstNd = GetNodes().MakeTxtNode( aSttIdx, pDfltTxtFmtColl );

    if( pLayout )
    {
        // set the layout to the dummy pagedesc
        pFirstNd->SwCntntNode::SetAttr( SwFmtPageDesc( pDummyPgDsc ));

        SwPosition aPos( *pFirstNd, SwIndex( pFirstNd ));
        ::PaMCorrAbs( aSttIdx, SwNodeIndex( GetNodes().GetEndOfContent() ),
                         aPos );
    }

    GetNodes().Delete( aSttIdx,
            GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );

    // --> OD 2006-02-28 #i62440#
    // destruction of numbering rules and creation of new outline rule
    // *after* the document nodes are deleted.
    pOutlineRule = NULL;
    pNumRuleTbl->DeleteAndDestroy( 0, pNumRuleTbl->Count() );
    // creation of new outline numbering rule
    pOutlineRule = new SwNumRule( String::CreateFromAscii(
                                      SwNumRule::GetOutlineRuleName() ),
                                  OUTLINE_RULE );
    AddNumRule(pOutlineRule);
    // --> OD 2005-10-21 - counting of phantoms depends on <IsOldNumbering()>
    pOutlineRule->SetCountPhantoms( !get(IDocumentSettingAccess::OLD_NUMBERING) );
    // <--
    // <--

    //remove the dummy pagedec from the array and delete all the old ones
    aPageDescs.Remove( nDummyPgDsc );
    aPageDescs.DeleteAndDestroy( 0, aPageDescs.Count() );

    // Delete fuer Collections
    // damit die Abhaengigen wech sind
    SwTxtFmtColl* pFtnColl = pFtnInfo->GetFtnTxtColl();
    if( pFtnColl ) pFtnColl->Remove( pFtnInfo );
    pFtnColl = pEndNoteInfo->GetFtnTxtColl();
    if( pFtnColl ) pFtnColl->Remove( pEndNoteInfo );

    // JP 27.01.98: opt.: ausgehend davon, das Standard als 2. im Array
    //              steht, sollte das als letztes geloescht werden, damit
    //              die ganze Umhaengerei der Formate vermieden wird!
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

    xForbiddenCharsTable.unbind();

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
/* -----------------------------06.01.00 14:03--------------------------------

 ---------------------------------------------------------------------------*/
SwModify*   SwDoc::GetUnoCallBack() const
{
    return pUnoCallBack;
}

/*-----------------28.5.2001 10:06------------------
 * SwDoc:
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

// --> FME 2005-02-25 #i42634# Moved common code of SwReader::Read() and
// SwDocShell::UpdateLinks() to new SwDoc::UpdateLinks():
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
// <--
// --> OD 2006-04-19 #b6375613#
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
                            rtl::OUString::createFromAscii("WorkaroundForB6375613Applied"),
                            beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::REMOVABLE,
                            uno::makeAny( false ) );
                    }
                    else
                    {
                        xDocInfo->removeProperty( rtl::OUString::createFromAscii("WorkaroundForB6375613Applied") );
                    }
                }
                catch( uno::Exception& )
                {
                }
            }
        }
    }
}
// <--
