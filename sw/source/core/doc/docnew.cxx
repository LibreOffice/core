/*************************************************************************
 *
 *  $RCSfile: docnew.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jp $ $Date: 2001-04-06 17:41:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define ROLBCK_HISTORY_ONLY     // Der Kampf gegen die CLOOK's

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HDL_
#include <com/sun/star/i18n/ForbiddenCharacters.hdl>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif

#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _PVPRTDAT_HXX
#include <pvprtdat.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>  //Damit der RootDtor gerufen wird.
#endif
#ifndef _LAYOUTER_HXX
#include <layouter.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> //Damit die PageDescs zerstoert werden koennen.
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
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
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
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

#ifndef _CMDID_H
#include <cmdid.h>              // fuer den dflt - Printer in SetJob
#endif

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
    aAttrPool( this ),
    aNodes( this ),
    aUndoNodes( this ),
    pFrmFmtTbl( new SwFrmFmts() ),
    pCharFmtTbl( new SwCharFmts() ),
    pSpzFrmFmtTbl( new SwSpzFrmFmts() ),
    pTblFrmFmtTbl( new SwFrmFmts() ),
    pDfltFrmFmt( new SwFrmFmt( aAttrPool, sFrmFmtStr, 0 ) ),
    pEmptyPageFmt( new SwFrmFmt( aAttrPool, sEmptyPageStr, pDfltFrmFmt ) ),
    pColumnContFmt( new SwFrmFmt( aAttrPool, sColumnCntStr, pDfltFrmFmt ) ),
    pDfltCharFmt( new SwCharFmt( aAttrPool, sCharFmtStr, 0 ) ),
    pDfltTxtFmtColl( new SwTxtFmtColl( aAttrPool, sTxtCollStr ) ),
    pTxtFmtCollTbl( new SwTxtFmtColls() ),
    pDfltGrfFmtColl( new SwGrfFmtColl( aAttrPool, sGrfCollStr ) ),
    pGrfFmtCollTbl( new SwGrfFmtColls() ),
    pSectionFmtTbl( new SwSectionFmts() ),
    pFldTypes( new SwFldTypes() ),
    pBookmarkTbl( new SwBookmarks() ),
    pTOXTypes( new SwTOXTypes() ),
    pDefTOXBases( new SwDefTOXBase_Impl() ),
    nLinkCt( 0 ),
    pGlossaryDoc( 0 ),
    nUndoPos( 0 ),
    nUndoSavePos( 0 ),
    nUndoCnt( 0 ),
    nUndoSttEnd( 0 ),
    pOutlineRule( 0 ),
    pLayout( 0 ),                   // Rootframe des spezifischen Layouts.
    pPrt( 0 ),
    pUndos( new SwUndos( 0, 20 ) ),
    pExtInputRing( 0 ),
    pLayouter( 0 ),
    nLockExpFld( 0 ),
    pDocShell( 0 ),
    pDrawModel( 0 ),
    pUpdtFlds( new SwDocUpdtFld() ),
    pLinkMgr( new SvxLinkManager( 0 ) ),
    pSwgInfo( 0 ),
    pDocShRef( 0 ),
    pACEWord( 0 ),
    pURLStateChgd( 0 ),
    pNumberFormatter( 0 ),
    pFtnInfo( new SwFtnInfo ),
    pEndNoteInfo( new SwEndNoteInfo ),
    pLineNumberInfo( new SwLineNumberInfo ),
    pFtnIdxs( new SwFtnIdxs ),
    pDocStat( new SwDocStat ),
    pNumRuleTbl( new SwNumRuleTbl ),
    eRedlineMode( SwRedlineMode(REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE) ),
    pRedlineTbl( new SwRedlineTbl ),
    pUnoCrsrTbl( new SwUnoCrsrTbl( 0, 16 ) ),
    pPgPViewPrtData( 0 ),
    pAutoFmtRedlnComment( 0 ),
    pUnoCallBack(new SwUnoCallBack(0)),
    nAutoFmtRedlnCommentNo( 0 ),
    eChrCmprType( CHARCOMPRESS_NONE ),
    n32Dummy1( 0 ), n32Dummy2( 0 ), n8Dummy1( 0 ), n8Dummy2( 0 ),
    nLinkUpdMode( GLOBALSETTING ),
    nFldUpdMode( AUTOUPD_GLOBALSETTING )
{
    bGlossDoc =
    bModified =
    bDtor =
    bUndo =
    bPageNums =
    bLoaded =
    bUpdateExpFld =
    bNewDoc =
    bCopyIsMove =
    bNoDrawUndoObj =
    bBrowseMode =
    bInReading =
    bUpdateTOX =
    bInLoadAsynchron =
    bHTMLMode =
    bHeadInBrowse =
    bFootInBrowse =
    bInCallModified =
    bIsGlobalDoc =
    bGlblDocSaveLinks =
    bIsLabelDoc =
    bIsAutoFmtRedline =
    bOLEPrtNotifyPending =
    bAllOLENotify =
    bIsRedlineMove =
    bInsOnlyTxtGlssry =
    bContains_MSVBasic =
    bKernAsianPunctuation =
#ifndef PRODUCT
    bXMLExport =
#endif
                                FALSE;

    bGroupUndo =
    bNewFldLst =
    bVisibleLinks =
    bFrmBeepEnabled =
    bPurgeOLE =
                                TRUE;

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
    // aFtnInfo.SetFtnTxtColl(aDfltTxtFmtColl); // jetzt oben in der Liste
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

    // lege (fuer die Filter) eine Default-OullineNumRule an
    pOutlineRule = new SwNumRule( String::CreateFromAscii(
                                        SwNumRule::GetOutlineRuleName() ),
                                    OUTLINE_RULE );

    new SwTxtNode( SwNodeIndex( aUndoNodes.GetEndOfContent() ), pDfltTxtFmtColl );
    new SwTxtNode( SwNodeIndex( aNodes.GetEndOfContent() ),
                    GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    // den eigenen IdleTimer setzen
    aIdleTimer.SetTimeout( 600 );
    aIdleTimer.SetTimeoutHdl( LINK(this, SwDoc, DoIdleJobs) );
    aIdleTimer.Start();

    // den CharTimer setzen
    aChartTimer.SetTimeout( 2000 );
    aChartTimer.SetTimeoutHdl( LINK( this, SwDoc, DoUpdateAllCharts ));

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
    aIdleTimer.Stop();  // den Idltimer abschalten

    delete pUnoCallBack;
    delete pURLStateChgd;

    delete pLayouter;

    // Undo-Benachrichtigung vom Draw abschalten
    if( pDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    delete pPgPViewPrtData;

    bUndo = FALSE;          // immer das Undo abschalten !!
    // damit die Fussnotenattribute die Fussnotennodes in Frieden lassen.
    bDtor = TRUE;

    DELETEZ( pLayout );
    DELETEZ( pOutlineRule );

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
    if( pPrt )
        DELETEZ( pPrt );

    if( pSwgInfo )
        DELETEZ( pSwgInfo );

    if (pNewDBMgr)
        DELETEZ(pNewDBMgr);

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
    delete pCharFmtTbl;
    delete pSectionFmtTbl;
    delete pTblFrmFmtTbl;
    delete pDfltTxtFmtColl;
    delete pDfltGrfFmtColl;
    delete pNumRuleTbl;

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
}


//---------------------------------------------------


void SwDoc::SetJobsetup( const JobSetup &rJobSetup )
{
    BOOL bCheckPageDescs = 0 == pPrt;
    BOOL bDataChanged = FALSE;

    if ( pPrt )
    {
        if ( pPrt->GetName() == rJobSetup.GetPrinterName() )
        {
            if ( pPrt->GetJobSetup() != rJobSetup )
            {
                pPrt->SetJobSetup( rJobSetup );
                bDataChanged = TRUE;
            }
        }
        else
            delete pPrt, pPrt = 0;
    }

    if( !pPrt )
    {
        //Das ItemSet wird vom Sfx geloescht!
        SfxItemSet *pSet = new SfxItemSet( aAttrPool,
                        FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                        SID_HTML_MODE,  SID_HTML_MODE,
                        SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                        0 );
        SfxPrinter *p = new SfxPrinter( pSet, rJobSetup );
        if ( bCheckPageDescs )
            SetPrt( p );
        else
        {
            pPrt = p;
            bDataChanged = TRUE;
        }
    }
    if ( bDataChanged )
        PrtDataChanged();
}

//---------------------------------------------------



const JobSetup* SwDoc::GetJobsetup() const
{
    return pPrt ? &pPrt->GetJobSetup() : 0;
}

//---------------------------------------------------

SfxPrinter* SwDoc::_GetPrt() const
{
    ASSERT( !pPrt, "Don't use _GetPrt(), use GetPrt()!" );
    // wir erzeugen einen default SfxPrinter.
    // Das ItemSet wird vom Sfx geloescht!
    SfxItemSet *pSet = new SfxItemSet( ((SwDoc*)this)->GetAttrPool(),
                    FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                    SID_HTML_MODE,  SID_HTML_MODE,
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );
    SfxPrinter *pNewPrt = new SfxPrinter( pSet );
    ((SwDoc*)this)->SetPrt( pNewPrt );
    return pPrt;
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
            ((SwDrawDocument*)pDrawModel)->SetObjectShell( pDocShell );
    }
}


// Convenience-Methode, um uebermaessige Includes von docsh.hxx
// zu vermeiden



SvStorage* SwDoc::GetDocStorage()
{
    if( pDocShell )
        return pDocShell->GetStorage();
    if( pLinkMgr->GetPersist() )
        return pLinkMgr->GetPersist()->GetStorage();
    return NULL;
}



SvPersist* SwDoc::GetPersist() const
{
    return pDocShell ? pDocShell : pLinkMgr->GetPersist();
}



void SwDoc::SetPersist( SvPersist* pPersist )
{
    if( !pDocShell )
    {
        ASSERT( ( !pPersist && pLinkMgr->GetPersist() ) ||
                ( pPersist && !pLinkMgr->GetPersist() ),
                "doppeltes setzen von Persist-Pointer?" )
        pLinkMgr->SetPersist( pPersist );
    }
#ifndef PRODUCT
    else
        ASSERT( !this, "DocShell existiert schon!" )
#endif
}



const SfxDocumentInfo* SwDoc::GetInfo()
{
    if( !pSwgInfo )
        // Pointer-Members initialisieren
        pSwgInfo  = new SfxDocumentInfo;
    return pSwgInfo;
}

void SwDoc::ClearDoc()
{
    bUndo = FALSE;          // immer das Undo abschalten !!

    // Undo-Benachrichtigung vom Draw abschalten
    if( pDrawModel )
    {
        DrawNotifyUndoHdl();
        ClrContourCache();
    }

    // stehen noch FlyFrames rum, loesche auch diese
    for( USHORT n = 0; n < GetSpzFrmFmts()->Count(); ++n )
        DelLayoutFmt( (*pSpzFrmFmtTbl)[n] );

    pRedlineTbl->DeleteAndDestroy( 0, pRedlineTbl->Count() );

    if( pACEWord )
        delete pACEWord;

    // in den BookMarks sind Indizies auf den Content. Diese muessen vorm
    // loesche der Nodes geloescht werden.
    pBookmarkTbl->DeleteAndDestroy( 0, pBookmarkTbl->Count() );
    pTOXTypes->DeleteAndDestroy( 0, pTOXTypes->Count() );
    aPageDescs.DeleteAndDestroy( 0, aPageDescs.Count() );

    pNumRuleTbl->DeleteAndDestroy( 0, pNumRuleTbl->Count() );

    SwNodeIndex aSttIdx( *GetNodes().GetEndOfContent().StartOfSectionNode(), 1 );
    // den ersten immer wieder neu anlegen (ohne Attribute/Vorlagen/...)
    SwTxtNode* pFirstNd = GetNodes().MakeTxtNode( aSttIdx, pDfltTxtFmtColl );
    GetNodes().Delete( aSttIdx,
            GetNodes().GetEndOfContent().GetIndex() - aSttIdx.GetIndex() );

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
    pFrmFmtTbl->DeleteAndDestroy( 1, pFrmFmtTbl->Count()-1 );
    pCharFmtTbl->DeleteAndDestroy( 1, pCharFmtTbl->Count()-1 );

    ReleaseDrawModel();
    xForbiddenCharsTable.unbind();

    pFldTypes->DeleteAndDestroy( INIT_FLDTYPES,
                                pFldTypes->Count() - INIT_FLDTYPES );

    delete pNumberFormatter, pNumberFormatter = 0;

    GetPageDescFromPool( RES_POOLPAGE_STANDARD );
    pFirstNd->ChgFmtColl( GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
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

/* ------------------------------------------------------------------------*/

const com::sun::star::i18n::
    ForbiddenCharacters* SwDoc::GetForbiddenCharacters( USHORT nLang,
                            BOOL bLocaleData ) const
{
    const com::sun::star::i18n::ForbiddenCharacters* pRet = 0;
    if( xForbiddenCharsTable.isValid() )
        pRet = xForbiddenCharsTable->GetForbiddenCharacters( nLang, FALSE );
    if( bLocaleData && !pRet && pBreakIt )
        pRet = &pBreakIt->GetForbidden( (LanguageType)nLang );
    return pRet;
}

void SwDoc::SetForbiddenCharacters( USHORT nLang,
                const com::sun::star::i18n::ForbiddenCharacters& rFChars )
{
    if( !xForbiddenCharsTable.isValid() )
    {
        ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > xMSF =
                                    ::comphelper::getProcessServiceFactory();
        xForbiddenCharsTable = new SvxForbiddenCharactersTable( xMSF );
    }
    xForbiddenCharsTable->SetForbiddenCharacters( nLang, rFChars );
    if( pDrawModel )
    {
        pDrawModel->SetForbiddenCharsTable( xForbiddenCharsTable );
        if( !bInReading )
            pDrawModel->ReformatAllTextObjects();
    }

    if( pLayout && !bInReading )
    {
        pLayout->StartAllAction();
        pLayout->InvalidateAllCntnt();
        pLayout->EndAllAction();
    }
}

void SwDoc::ClearForbiddenCharacters( USHORT nLang )
{
    if( xForbiddenCharsTable.isValid() )
    {
        xForbiddenCharsTable->ClearForbiddenCharacters( nLang );
        if( !xForbiddenCharsTable->Count() )
            xForbiddenCharsTable.unbind();

        if( pDrawModel )
        {
            pDrawModel->SetForbiddenCharsTable( xForbiddenCharsTable );
            if( !bInReading )
                pDrawModel->ReformatAllTextObjects();
        }

        if( pLayout && !bInReading )
        {
            pLayout->StartAllAction();
            pLayout->InvalidateAllCntnt();
            pLayout->EndAllAction();
        }
    }
}

