/*************************************************************************
 *
 *  $RCSfile: docsh2.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-06 09:21:17 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _LCKBITEM_HXX
#include <svtools/lckbitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFX_DINFDLG_HXX //autogen
#include <sfx2/dinfdlg.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXNEW_HXX //autogen
#include <sfx2/new.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX //autogen
#include <sfx2/evntconf.hxx>
#endif
#ifndef _SFXECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_OFAITEM_HXX //autogen
#include <offmgr/ofaitem.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif

#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _SWEVENT_HXX //autogen
#include <swevent.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>         // fuer die aktuelle Sicht
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>        // Dokumenterzeugung
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _DOCSTDLG_HXX
#include <docstdlg.hxx>     // fuer Dokument Style
#endif
#ifndef _TMPLDLG_HXX
#include <tmpldlg.hxx>
#endif
#ifndef _FLDWRAP_HXX
#include <fldwrap.hxx>
#endif
#ifndef _REDLNDLG_HXX
#include <redlndlg.hxx>
#endif
#ifndef _ABSTRACT_HXX
#include <abstract.hxx>     // SwInsertAbstractDialog
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _LTMPDLG_HXX
#include <ltmpdlg.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _UNOTXDOC_HXX
#include <unotxdoc.hxx>
#endif
#ifndef _ACMPLWRD_HXX
#include <acmplwrd.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_H
#include <globals.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

extern FASTBOOL FindPhyStyle( SwDoc& , const String& , SfxStyleFamily );


/*--------------------------------------------------------------------
    Beschreibung:   DocInfo setzen am SFX
 --------------------------------------------------------------------*/


BOOL SwDocShell::SetDocumentInfo(const SfxDocumentInfo& rInfo)
{
    SfxDocumentInfo& rOldInfo = GetDocInfo();
    BOOL bRet = !(rOldInfo == rInfo);
    if( bRet )
    {
        rOldInfo = rInfo;
        Broadcast( SfxDocumentInfoHint( &rOldInfo ) );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   DocInfo kreieren (virtuell)
 --------------------------------------------------------------------*/


SfxDocumentInfoDialog* SwDocShell::CreateDocumentInfoDialog(
                                Window *pParent, const SfxItemSet &rSet)
{
    SfxDocumentInfoDialog* pDlg = new SfxDocumentInfoDialog(pParent, rSet);
//  const SfxDocumentInfoItem& rItem = (const SfxDocumentInfoItem&)rSet.Get(SID_DOCINFO);
//  if(rItem.IsOwnFormat())
    //nur mit Statistik, wenn dieses Doc auch angezeigt wird, nicht
    //aus dem Doc-Manager
    SwDocShell* pDocSh = (SwDocShell*) SfxObjectShell::Current();
    if( pDocSh == this )
    {
        //Nicht fuer SourceView.
        SfxViewShell *pVSh = SfxViewShell::Current();
        if ( pVSh && !pVSh->ISA(SwSrcView) )
            pDlg->AddTabPage(TP_DOC_STAT, SW_RESSTR(STR_DOC_STAT),
                                                    SwDocStatPage::Create, 0);
    }
    return pDlg;
}

/*--------------------------------------------------------------------
    Beschreibung:   Benachrichtigung bei geaenderter DocInfo
 --------------------------------------------------------------------*/


void SwDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if( !pDoc )
    {
//MA: Kommt bei der OLE-Registration vor!
//      ASSERT( !this, "DocShell ist nicht richtig initialisiert!" );
        return ;
    }

    if( rHint.ISA(SfxDocumentInfoHint) )
    {
        if( pWrtShell )
        {
            pWrtShell->LockView( TRUE );    //Sichtbaren bereich Locken.
            pWrtShell->StartAllAction();
        }
        pDoc->DocInfoChgd( *((SfxDocumentInfoHint&)rHint).GetObject() );
        if( pWrtShell )
        {
            pWrtShell->EndAllAction();
            pWrtShell->LockView( FALSE );
        }
    }
    else if ( rHint.ISA(SfxSimpleHint) )
    {
        switch( ((SfxSimpleHint&) rHint).GetId() )
        {
            case SFX_HINT_TITLECHANGED:
                if ( GetMedium() )
                {
                    if( pWrtShell )
                    {
                        pWrtShell->LockView( TRUE );
                        pWrtShell->StartAllAction();
                    }
                    pDoc->GetSysFldType( RES_FILENAMEFLD )->UpdateFlds();
                    if( pWrtShell )
                    {
                        pWrtShell->EndAllAction();
                        pWrtShell->LockView( FALSE );
                    }
                }
                break;
        }
    }

}

/*--------------------------------------------------------------------
    Beschreibung:   Benachrichtigung Doc schliessen
 --------------------------------------------------------------------*/


USHORT SwDocShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    USHORT nRet = SfxObjectShell::PrepareClose( bUI, bForBrowsing );

    if( TRUE == nRet ) //Unbedingt auf TRUE abfragen! (RET_NEWTASK)
        EndListening( *this );

    return nRet;
}


void SwDoc::SetInfo( const SfxDocumentInfo& rInfo )
{
    if( pDocShell )
        pDocShell->SetDocumentInfo( rInfo );

    // sollte nur beim "Konvertieren" von Dokumenten hier ankommen!
    else
    {
        // dann setzen wir uns die DocInfo. Nach dem Konvertieren wird diese
        // am Medium gesetzt. Erst dann ist die DocShell bekannt.
        delete pSwgInfo;
        pSwgInfo = new SfxDocumentInfo( rInfo );

// wenn beim Einlesen, dann kein Modify verschicken, diese sollten dann
// richtig eingelesen werden oder spaetestens beim Expandieren die richtigen
// Werte finden.
//      GetSysFldType( RES_DOCINFOFLD )->UpdateFlds();
//      GetSysFldType( RES_TEMPLNAMEFLD )->UpdateFlds();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Organizer
 --------------------------------------------------------------------*/


BOOL SwDocShell::Insert( SfxObjectShell &rSource,
    USHORT  nSourceIdx1,        // SourcePool: oberste Inhaltsebene (Vorlagen/Makros)
    USHORT  nSourceIdx2,        // Index in den Inhalt
    USHORT  nSourceIdx3,        // Index in die Inhaltsebene
    USHORT &rIdx1,              // und das gleiche fuer den DestinationPool
    USHORT &rIdx2,              //      ""
    USHORT &rIdx3,              //      ""
    USHORT &rRemovedIdx )       // falls doppelte geloescht werden, Pos zurueck
{
    BOOL bRet = FALSE;

    if (INDEX_IGNORE == rIdx1 && CONTENT_STYLE == nSourceIdx1)
        rIdx1 = CONTENT_STYLE;

    if (CONTENT_STYLE == nSourceIdx1 && CONTENT_STYLE == rIdx1)
    {
        SfxStyleSheetBasePool* pHisPool  = rSource.GetStyleSheetPool();
        SwDocStyleSheetPool* pMyPool =
            (SwDocStyleSheetPool*)GetStyleSheetPool();

        //  wir koennen nicht in uns selbst hin und her moven
        if( pHisPool == pMyPool )
            return FALSE;

        if( INDEX_IGNORE == rIdx2 )
            rIdx2 = pMyPool->Count();

        // erstmal auf die Such-Maske "positionieren"
        pHisPool->First();
        SfxStyleSheetBase* pHisSheet = (*pHisPool)[nSourceIdx2];

        // wenn so eine Vorlage schon existiert: loeschen!
        const String& rOldName = pHisSheet->GetName();
        SfxStyleFamily eOldFamily( pHisSheet->GetFamily() );

        // dflt. PageDesc und StandardZeichenvorlage nie loeschen !!!
        if( ( SFX_STYLE_FAMILY_PAGE == eOldFamily &&
              pDoc->GetPageDesc(0).GetName() == rOldName ) ||
              ( SFX_STYLE_FAMILY_CHAR == eOldFamily &&
                rOldName == *pDoc->GetTextNmArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] ))
            return FALSE;

        SfxStyleFamily eMyOldFamily( pMyPool->GetSearchFamily() );
        USHORT nMySrchMask = pMyPool->GetSearchMask();

        SfxStyleSheetBase* pExist;
        if( ::FindPhyStyle( *pDoc, rOldName, eOldFamily ) )
        {
            // Bug 20365: nur uebernehmen, wenn das gewuenscht ist!
            if( ERRCODE_BUTTON_OK != ErrorHandler::HandleError(
                *new MessageInfo( ERRCODE_SFXMSG_STYLEREPLACE, rOldName )) )
            {
                return FALSE;
            }

            // Da Replace den aStyleSheet-Member selbst benoetigt, muss
            // das Ergebnis vom Find kopiert werden (s.u.))
            SwDocStyleSheet aExist(
                    *(SwDocStyleSheet*)pMyPool->Find( rOldName, eOldFamily ) );
            pMyPool->Replace( *pHisSheet, aExist );

            // An der Reihenfolge der Vorlagen im Pool aendert sich nichts.
            rIdx2 = rIdx1 = INDEX_IGNORE;

            GetDoc()->SetModified();

            return TRUE;
        }

        pMyPool->SetSearchMask( eOldFamily, nMySrchMask );

        // MIB 18.12.98: SwDocStyleSheetPool::Make liefert einen
        // Pointer auf SwDocStyleSheetPool::aStyleSheet zurueck.
        // Der gleiche Member von SwDocStyleSheetPool::Find benutzt.
        // Deshalb muss hier mit einer Kopie gearbeitet werden.
        // Vorsicht: SfxStyleSheetBase::SetFollow ruft seinerseits
        // ein SwDocStyleSheetPool::Find auf, do dass es nicht genuegt
        // die Find-Aufrufe in dieser Methode zu eleminieren.

        SwDocStyleSheet aNewSheet( (SwDocStyleSheet&)pMyPool
                ->Make(rOldName, eOldFamily, pHisSheet->GetMask() ) );
        if( SFX_STYLE_FAMILY_PAGE == eOldFamily && rSource.ISA(SwDocShell) )
        {
            // gesondert behandeln!!
            SwPageDesc* pDestDsc = (SwPageDesc*)aNewSheet.GetPageDesc();
            SwPageDesc* pCpyDsc = (SwPageDesc*)((SwDocStyleSheet*)pHisSheet)->GetPageDesc();
            pDoc->CopyPageDesc( *pCpyDsc, *pDestDsc );
        }
        else
            // die neue Vorlage mit den Attributen fuellen
            aNewSheet.SetItemSet( pHisSheet->GetItemSet() );

        pMyPool->SetSearchMask( SFX_STYLE_FAMILY_ALL, nMySrchMask );

        if( aNewSheet.IsUserDefined() || aNewSheet.IsUsed() )
        {
            // Benutzte und Benutzer-definierte Vorlagen werden angezeigt.
            // Dshalb muss hier der Index der neuen Vorlage im Pool
            // ermittelt werden.
            pExist = pMyPool->First();
            USHORT nIdx = 0;
            while( pExist )
            {
                if( pExist->GetName() == rOldName &&
                   eOldFamily == pExist->GetFamily() )
                {
                    rIdx2 = nIdx;
                    break;
                }
                pExist = pMyPool->Next();
                nIdx++;
            }
        }
        else
        {
            // Andere Vorlagen werden nicht angezeigt.
            rIdx1 = rIdx2 = INDEX_IGNORE;
        }

/*        pMyPool->Count();       //interne Liste neu fuellen lassen!!!!!

        // suchen, um die richige Insert-Position returnen zu koennen
        pMyPool->Find( rOldName, SFX_STYLE_FAMILY_ALL, nMySrchMask );
        rIdx2 = pMyPool->GetFindPos();*/

        // wer bekommt den Neuen als Parent? wer benutzt den Neuen als Follow?
        // (immer nur ueber die Instanziierten!!!)
        pMyPool->SetSearchMask( eOldFamily, nMySrchMask );
        pMyPool->SetOrganizerMode( TRUE );
        SfxStyleSheetBase* pTestSheet = pMyPool->First();
        while (pTestSheet)
        {
            if (pTestSheet->GetFamily() == eOldFamily &&
                pTestSheet->HasParentSupport()        &&
                pTestSheet->GetParent() == rOldName)
            {
                pTestSheet->SetParent(rOldName); // Verknuepfung neu aufbauen
            }

            if (pTestSheet->GetFamily() == eOldFamily &&
                pTestSheet->HasFollowSupport()        &&
                pTestSheet->GetFollow() == rOldName)
            {
                pTestSheet->SetFollow(rOldName); // Verknuepfung neu aufbauen
            }

            pTestSheet = pMyPool->Next();
        }
        pMyPool->SetOrganizerMode( SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );

        // hat der Neue einen Parent? wenn ja, mit gleichem Namen bei uns suchen
        if (pHisSheet->HasParentSupport())
        {
            const String& rParentName = pHisSheet->GetParent();
            if (0 != rParentName.Len())
            {
                SfxStyleSheetBase* pParentOfNew = pMyPool->Find(rParentName,
                                                                eOldFamily);
                if (pParentOfNew)
                {
                    aNewSheet.SetParent(rParentName);
                }
                pMyPool->SetSearchMask( eOldFamily, nMySrchMask );
            }
        }

        // hat der Neue einen Follow? wenn ja, mit gleichem Namen bei uns suchen
        if (pHisSheet->HasFollowSupport())
        {
            const String& rFollowName = pHisSheet->GetFollow();
            if (0 != rFollowName.Len())
            {
                SfxStyleSheetBase* pFollowOfNew = pMyPool->Find(rFollowName,
                                                                eOldFamily);
                if (pFollowOfNew)
                {
                    aNewSheet.SetFollow(rFollowName);
                }
                pMyPool->SetSearchMask( eOldFamily, nMySrchMask );
            }
        }

        // Bug 27347: alte Einstellung wieder setzen
        pMyPool->SetSearchMask( eMyOldFamily, nMySrchMask );

        // Model geaendert
        ASSERT(pDoc, "Doc fehlt");
        GetDoc()->SetModified();

        bRet = TRUE;
    }
    else
        bRet = SfxObjectShell::Insert( rSource,
                    nSourceIdx1,
                    nSourceIdx2,
                    nSourceIdx3,
                    rIdx1,
                    rIdx2,
                    rIdx3,
                    rRemovedIdx);

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Vorlagen Remove
 --------------------------------------------------------------------*/


BOOL SwDocShell::Remove(USHORT nIdx1,       // siehe Insert
                        USHORT nIdx2,
                        USHORT nIdx3)
{
    BOOL bRet = FALSE;

    if (CONTENT_STYLE == nIdx1)
    {
        SwDocStyleSheetPool* pMyPool = (SwDocStyleSheetPool*)GetStyleSheetPool();

        pMyPool->First();       // vorm Zugriff Pool aktualisieren!!
        SfxStyleSheetBase* pMySheet = (*pMyPool)[nIdx2];

        String aName( pMySheet->GetName() );
        SfxStyleFamily eFamily( pMySheet->GetFamily() );

        // dflt. PageDesc und StandardZeichenvorlage nie loeschen !!!
        if( ( SFX_STYLE_FAMILY_PAGE == eFamily &&
              pDoc->GetPageDesc(0).GetName() == aName ) ||
              ( SFX_STYLE_FAMILY_CHAR == eFamily &&
                aName == *pDoc->GetTextNmArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] ))
            return FALSE;

        // also loeschen
        pMyPool->Erase( pMySheet );

        // jetzt noch die Parents/Follows aller Instanziierten korrigieren
        pMyPool->SetOrganizerMode( TRUE );
        SfxStyleSheetBase* pTestSheet = pMyPool->First();
        while (pTestSheet)
        {
            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasParentSupport()     &&
                pTestSheet->GetParent() == aName)
            {
                pTestSheet->SetParent( aEmptyStr ); // Verknuepfung aufloesen
            }

            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasFollowSupport()        &&
                pTestSheet->GetFollow() == aName)
            {
                pTestSheet->SetFollow( aEmptyStr ); // Verknuepfung aufloesen
            }

            pTestSheet = pMyPool->Next();
        }
        pMyPool->SetOrganizerMode( SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );

        bRet = TRUE;
    }
    else
        bRet = SfxObjectShell::Remove(  nIdx1,
                                        nIdx2,
                                        nIdx3 );


    // Model geaendert
    ASSERT(pDoc, "Doc fehlt");
    GetDoc()->SetModified();

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDocShell::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    USHORT nWhich = rReq.GetSlot();
    BOOL bDone = FALSE;
    switch ( nWhich )
    {
        case FN_AUTO_CORRECT_DLG:
        {
            OfficeApplication *pOApp = OFF_APP();

            SfxBoolItem aSwOptions( SID_AUTO_CORRECT_DLG, TRUE );
            SvxSwAutoFmtFlags* pAFlags = &pOApp->GetAutoCorrect()->GetSwFlags();
            SwAutoCompleteWord& rACW = SwDoc::GetAutoCompleteWords();

            BOOL bOldLocked = rACW.IsLockWordLstLocked(),
                 bOldAutoCmpltCollectWords = pAFlags->bAutoCmpltCollectWords;

            rACW.SetLockWordLstLocked( TRUE );

            SvStringsISortDtor aTmpLst;
            aTmpLst.Insert( &rACW.GetWordList() );
            pAFlags->pAutoCmpltList = &aTmpLst;

            SfxViewShell* pViewShell = GetView()
                                            ? (SfxViewShell*)GetView()
                                            : SfxViewShell::Current();
            pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                SID_AUTO_CORRECT_DLG, SFX_CALLMODE_SYNCHRON, &aSwOptions, 0L);

            rACW.SetLockWordLstLocked( bOldLocked );

            SwEditShell::SetAutoFmtFlags( pAFlags );
            rACW.SetMinWordLen( pAFlags->nAutoCmpltWordLen );
            rACW.SetMaxCount( pAFlags->nAutoCmpltListLen );
            if( pAFlags->pAutoCmpltList )  // any changes?
            {
                rACW.CheckChangedList( aTmpLst );
                // clear the temp WordList pointer
                pAFlags->pAutoCmpltList = 0;
            }
            // remove all pointer we never delete the strings
            aTmpLst.Remove( (USHORT)0, aTmpLst.Count() );

            if( !bOldAutoCmpltCollectWords && bOldAutoCmpltCollectWords !=
                pAFlags->bAutoCmpltCollectWords )
            {
                // call on all Docs the idle formatter to start
                // the collection of Words
                TypeId aType = TYPE(SwDocShell);
                for( SwDocShell *pDocSh = (SwDocShell*)SfxObjectShell::GetFirst(&aType);
                     pDocSh;
                     pDocSh = (SwDocShell*)SfxObjectShell::GetNext( *pDocSh, &aType ) )
                {
                    SwDoc* pTmp = pDocSh->GetDoc();
                    if ( pTmp->GetRootFrm() )
                        pTmp->InvalidateAutoCompleteFlag();
                }
            }
        }
        break;

        case SID_PRINTPREVIEW:
            {
                BOOL bSet = FALSE, bFound = FALSE, bOnly = TRUE;
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this);
                SfxViewShell* pViewShell = SfxViewShell::Current();
                SwView* pView = PTR_CAST( SwView, pViewShell );
                BOOL bCurrent = IS_TYPE( SwPagePreView, pViewShell );

                while( pTmpFrm )    // search PreView
                {
                    if( IS_TYPE( SwView, pTmpFrm->GetViewShell()) )
                        bOnly = FALSE;
                    else if( IS_TYPE( SwPagePreView, pTmpFrm->GetViewShell()))
                    {
                        pTmpFrm->GetFrame()->Appear();
                        bFound = TRUE;
                    }
                    if( bFound && !bOnly )
                        break;
                    pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, this);
                }

                if( pArgs && SFX_ITEM_SET ==
                    pArgs->GetItemState( SID_PRINTPREVIEW, FALSE, &pItem ))
                    bSet = ((SfxBoolItem*)pItem)->GetValue();
                else
                    bSet = !bCurrent;

                USHORT nSlotId = 0;
                if( bSet && !bFound )   // Keine gefunden, daher neue Preview anlegen
                {
                    //Keine neue anlegen fuer BrowseView!
                    if( !GetDoc()->IsBrowseMode() )
                        nSlotId = SID_VIEWSHELL1;
                }
                else if( bFound && !bSet )
                    nSlotId = bOnly ? SID_VIEWSHELL0 : SID_VIEWSHELL1;

                if( nSlotId )
                {
                    if( pView && pView->GetDocShell() == this )
                        pTmpFrm = pView->GetViewFrame();
                    else
                        pTmpFrm = SfxViewFrame::GetFirst( this );

                    pTmpFrm->GetBindings().Execute( nSlotId, NULL, 0,
                                                    SFX_CALLMODE_ASYNCHRON );
                }
                rReq.SetReturnValue(SfxBoolItem(SID_PRINTPREVIEW, bSet ));
            }
            break;
        case SID_TEMPLATE_LOAD:
            {
                String aFileName;
                BOOL bText = FALSE;
                BOOL bFrame = FALSE;
                BOOL bPage = FALSE;
                BOOL bOverwrite = FALSE;
                BOOL bNumbering = FALSE;
                USHORT nRet = USHRT_MAX;
                SvtPathOptions aPathOpt;
                SwLoadTemplateDlg* pDlg = new SwLoadTemplateDlg(0);
                pDlg->SetPath( aPathOpt.GetWorkPath() );

                SfxObjectFactory &rFact = GetFactory();
                for( USHORT i = 0; i < rFact.GetFilterCount(); i++ )
                {
                    const SfxFilter* pFlt = rFact.GetFilter( i );
                    if( pFlt && pFlt->IsAllowedAsTemplate() )
                        pDlg->AddFilter( pFlt->GetUIName(),
                                    ((WildCard&)pFlt->GetWildcard())(),
                                    pFlt->GetTypeName() );

                    if( pFlt->GetUserData().EqualsAscii( FILTER_SW5 ))
                        pDlg->SetCurFilter( pFlt->GetUIName() );
                }

                nRet = pDlg->Execute();
                if( nRet == RET_OK )
                {
                    aFileName = pDlg->GetPath();
                }
                else if( nRet == RET_TEMPLATE )
                {
                    SfxNewFileDialog* pNewDlg = pDlg->GetNewFileDlg();

                    nRet = pNewDlg->Execute();
                    if( nRet == RET_OK )
                        aFileName = pNewDlg->GetTemplateFileName();
                }

                SwgReaderOption aOpt;
                aOpt.SetFrmFmts( pDlg->IsFrameStyle() );
                aOpt.SetTxtFmts( pDlg->IsTextStyle() );
                aOpt.SetPageDescs( pDlg->IsPageStyle() );
                aOpt.SetNumRules( pDlg->IsNumbering() );
                aOpt.SetMerge( !pDlg->IsOverwrite() );

                delete pDlg;
                if( aFileName.Len() )
                    SetError( LoadStylesFromFile( aFileName, aOpt, FALSE ));
            }
            break;
            case SID_SOURCEVIEW:
            {
                SfxViewShell* pViewShell = GetView()
                                            ? (SfxViewShell*)GetView()
                                            : SfxViewShell::Current();
                SfxViewFrame*  pViewFrm = pViewShell->GetViewFrame();
                SwSrcView* pSrcView = PTR_CAST(SwSrcView, pViewShell);
                if(!pSrcView)
                {
                    if(!HasName())
                    {
                        const SfxBoolItem* pBool = (const SfxBoolItem*)
                                pViewFrm->GetDispatcher()->Execute(
                                        SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON );
                        if(!pBool || !pBool->GetValue())
                            break;
                    }
                    else
                    {
                        pViewFrm->GetDispatcher()->Execute(
                                        SID_SAVEDOC, SFX_CALLMODE_SYNCHRON );
                    }
                }
#ifdef DBG_UTIL
                {
                    BOOL bWeb = 0 != PTR_CAST(SwWebDocShell, this);
                    DBG_ASSERT(bWeb == TRUE, "SourceView nur in der WebDocShell")
                }
#endif
                // die SourceView ist fuer die SwWebDocShell die 1
                USHORT nSlot = SID_VIEWSHELL1;
                BOOL bSetModified = FALSE;
                SfxPrinter* pSavePrinter = 0;
                if( 0 != pSrcView)
                {
                    SfxPrinter* pTemp = GetDoc()->GetPrt(FALSE);
                    if(pTemp)
                        pSavePrinter = new SfxPrinter(*pTemp);
                    bSetModified = IsModified() || pSrcView->IsModified();
                    if(pSrcView->IsModified()||pSrcView->HasSourceSaved())
                    {
                        utl::TempFile aTempFile;
                        aTempFile.EnableKillingFile();
                        pSrcView->SaveContent(aTempFile.GetFileName());
                        bDone = TRUE;
                        SetActualSize(pSrcView->GetEditWin().GetSizePixel());
                        SfxEventConfiguration* pEvent = SFX_APP()->GetEventConfig();
                        SvxMacro aMac(aEmptyStr, aEmptyStr, STARBASIC);
                        pEvent->ConfigureEvent(SFX_EVENT_OPENDOC,       aMac, this);
                        pEvent->ConfigureEvent(SFX_EVENT_CLOSEDOC,      aMac, this);
                        pEvent->ConfigureEvent(SFX_EVENT_ACTIVATEDOC,   aMac, this);
                        pEvent->ConfigureEvent(SFX_EVENT_DEACTIVATEDOC, aMac, this);
                        ReloadFromHtml(aTempFile.GetFileName(), pSrcView);
                        nSlot = 0;
                    }
                    else
                    {
                        nSlot = SID_VIEWSHELL0;
                    }
                }
/*   OS 10.04.97 14.20: Im Web-Writer gibt es keinen Unterschied zwischen
                        Export in den SourceMode und dem Speichern des Docs
                else if(IsModified())
                {
                    USHORT nRet = QueryBox( SFX_APPWINDOW ,SW_RES(MSG_SAVE_HTML_QUERY)).Execute();
                    if(RET_YES == nRet)
                    {
                        S F X _DISPATCHER().Execute(SID_SAVEDOC,
                                    SFX_CALLMODE_SYNCHRON);
                        // der ReturnValue von SID_SAVEDOC ist etwas schwer verstaendlich
                        if(IsModified())
                            nSlot = 0;
                    }
                    else if( RET_CANCEL == nRet )
                    {
                        nSlot = 0;
                    }
                }*/
                if(nSlot)
                    pViewFrm->GetDispatcher()->Execute(nSlot, SFX_CALLMODE_SYNCHRON);
                if(bSetModified)
                    GetDoc()->SetModified();
                if(pSavePrinter)
                {
                    GetDoc()->SetPrt(pSavePrinter);
                    //pSavePrinter darf nicht wieder geloescht werden
                }
                pViewFrm->GetBindings().SetState(SfxBoolItem(SID_SOURCEVIEW, nSlot == SID_VIEWSHELL2));
            }
            break;
            case SID_GET_COLORTABLE:
            {
                SvxColorTableItem* pColItem = (SvxColorTableItem*)GetItem(SID_COLOR_TABLE);
                XColorTable* pTable = pColItem->GetColorTable();
                rReq.SetReturnValue(OfaPtrItem(SID_GET_COLORTABLE, pTable));
            }
            break;
        case FN_ABSTRACT_STARIMPRESS:
        case FN_ABSTRACT_NEWDOC:
        {
            SwInsertAbstractDlg* pDlg = new SwInsertAbstractDlg(0);
            if(RET_OK == pDlg->Execute())
            {
                BYTE nLevel = pDlg->GetLevel();
                BYTE nPara = pDlg->GetPara();
                SwDoc* pDoc = new SwDoc();
                SfxObjectShellRef xDocSh( new SwDocShell( pDoc, SFX_CREATE_MODE_STANDARD));
                xDocSh->DoInitNew( 0 );
                BOOL bImpress = FN_ABSTRACT_STARIMPRESS == nWhich;
                pWrtShell->Summary( pDoc, nLevel, nPara, bImpress );
                if( bImpress )
                {
                    WriterRef xWrt;
                    ::GetRTFWriter( aEmptyStr, xWrt );
                    SvMemoryStream *pStrm = new SvMemoryStream();
                    pStrm->SetBufferSize( 16348 );
                    SwWriter aWrt( *pStrm, *pDoc );
                    ErrCode eErr = aWrt.Write( xWrt );
                    if( !ERRCODE_TOERROR( eErr ) )
                    {
                        pStrm->Seek( STREAM_SEEK_TO_END );
                        *pStrm << '\0';
                        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                        //Die Lockbytes werden owner des Stream*
                        SvLockBytes *pLockBytes = new SvLockBytes( pStrm, TRUE );
                        //Das Item wird owner des LockBytes*
                        SfxLockBytesItem aItem( SID_OUTLINE_TO_IMPRESS, pLockBytes );
                        SfxModule *pMod = (*(SfxModule**)GetAppData(SHL_DRAW))->Load();
                        SfxItemSet aSet( pMod->GetPool(), SID_OUTLINE_TO_IMPRESS,
                                                            SID_OUTLINE_TO_IMPRESS );
                        aSet.Put( aItem );
                        SfxAllItemSet aArgs( pMod->GetPool() );
                        aArgs.Put( aSet );
                        SfxRequest aReq( SID_OUTLINE_TO_IMPRESS, 0, aArgs );
                        pMod->ExecuteSlot( aReq );
                    }
                    else
                        ErrorHandler::HandleError(ErrCode( eErr ));
                }
                else
                {
                    // Neues Dokument erzeugen.
                    SfxViewFrame *pFrame = SFX_APP()->CreateViewFrame( *xDocSh, 0 );
                    SwView      *pView = (SwView*) pFrame->GetViewShell();

                    // Dokumenttitel setzen
                    String aTmp( SW_RES(STR_ABSTRACT_TITLE) );
                    aTmp += GetTitle();
                    xDocSh->SetTitle( aTmp );
                    pView->GetWrtShell().SetNewDoc();
                    pFrame->Show();
                    pDoc->SetModified();
                }

            }
            delete pDlg;
        }
        break;
        case FN_OUTLINE_TO_CLIPBOARD:
        case FN_OUTLINE_TO_IMPRESS:
            {
                WriterRef xWrt;
                ::GetRTFWriter( 'O', xWrt );
                SvMemoryStream *pStrm = new SvMemoryStream();
                pStrm->SetBufferSize( 16348 );
                SwWriter aWrt( *pStrm, *GetDoc() );
                ErrCode eErr = aWrt.Write( xWrt );
                if( !ERRCODE_TOERROR( eErr ) )
                {
                    pStrm->Seek( STREAM_SEEK_TO_END );
                    *pStrm << '\0';
                    pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                    if ( nWhich == FN_OUTLINE_TO_IMPRESS )
                    {
                        //Die Lockbytes werden owner des Stream*
                        SvLockBytes *pLockBytes = new SvLockBytes( pStrm, TRUE );
                        //Das Item wird owner des LockBytes*
                        SfxLockBytesItem aItem( SID_OUTLINE_TO_IMPRESS, pLockBytes );
                        SfxModule *pMod = (*(SfxModule**)GetAppData(SHL_DRAW))->Load();
                        SfxItemSet aSet( pMod->GetPool(), SID_OUTLINE_TO_IMPRESS,
                                                          SID_OUTLINE_TO_IMPRESS );
                        aSet.Put( aItem );
                        SfxAllItemSet aArgs( pMod->GetPool() );
                        aArgs.Put( aSet );
                        SfxRequest aReq( SID_OUTLINE_TO_IMPRESS, 0, aArgs );
                        pMod->ExecuteSlot( aReq );
                    }
                    else
                    {
                        Clipboard::Clear();
                        Clipboard::CopyData( pStrm->GetData(), pStrm->GetSize(),
                                             FORMAT_RTF );
                        delete pStrm;
                    }
                }
                else
                    ErrorHandler::HandleError(ErrCode( eErr ));
            }
            break;
            case SID_SPELLCHECKER_CHANGED:
                //! FALSE, TRUE, TRUE is on the save side but a probably overdone
                SW_MOD()->CheckSpellChanges(FALSE, TRUE, TRUE );
            break;

            case SID_BROWSER_MODE:
            case FN_PRINT_LAYOUT:   //Fuer Web, genau umgekehrt zum BrowserMode
            {
                int eState = STATE_TOGGLE;
                BOOL bSet;
                const SfxPoolItem* pAttr;
                if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nWhich , FALSE, &pAttr ))
                {
                    bSet = ((SfxBoolItem*)pAttr)->GetValue();
                    if ( nWhich == FN_PRINT_LAYOUT )
                        bSet = !bSet;
                    eState = bSet ? STATE_ON : STATE_OFF;
                }

                if ( STATE_TOGGLE == eState )
                    bSet = !GetDoc()->IsBrowseMode();

                ToggleBrowserMode(bSet, 0);

                // OS: numerische Reihenfolge beachten!
                static USHORT __READONLY_DATA aInva[] =
                                    {
                                        SID_NEWWINDOW,
                                        SID_RULER_BORDERS, SID_RULER_PAGE_POS,
                                        SID_ATTR_LONG_LRSPACE,
                                        SID_HTML_MODE,
                                        SID_RULER_PROTECT,
                                        SID_AUTOSPELL_CHECK,
                                        SID_AUTOSPELL_MARKOFF,
                                        FN_RULER,       /*20211*/
                                        FN_VIEW_GRAPHIC,    /*20313*/
                                        FN_VIEW_BOUNDS,     /**/
                                        FN_VIEW_FIELDS,     /**/
                                        FN_VLINEAL,             /**/
                                        FN_VSCROLLBAR,      /**/
                                        FN_HSCROLLBAR,      /**/
                                        FN_VIEW_META_CHARS, /**/
                                        FN_VIEW_MARKS,      /**/
                                        FN_VIEW_FIELDNAME,  /**/
                                        FN_VIEW_TABLEGRID,  /**/
                                        FN_QRY_MERGE,   /*20364*/
                                        FN_STAT_HYPERLINKS, /*21186*/
                                        FN_INSERT_HEADER, /*21405*/
                                        FN_INSERT_FOOTER, /*21406*/
                                        0
                                    };
                // the view must not exist!
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst( this );
                if( pTmpFrm )
                    pTmpFrm->GetBindings().Invalidate( aInva );
            }
            break;

        case SID_MAIL_PREPAREEXPORT:
            {
                pWrtShell->StartAllAction();
                pDoc->UpdateFlds( 0 );
                pDoc->EmbedAllLinks();
                pDoc->RemoveInvisibleContent();
                pWrtShell->EndAllAction();
            }
            break;

        case FN_NEW_HTML_DOC:
        case FN_NEW_GLOBAL_DOC:
            {
                bDone = FALSE;
                String aFileName, sTemplateName;

                const SwTxtFmtColl* pSplitColl = 0;

                SfxFileDialog* pFileDlg =
                    new SfxFileDialog( 0, WB_SAVEAS|WB_SVLOOK );
                const SfxFilter* pFlt;
                USHORT nStrId;

                if( FN_NEW_HTML_DOC == nWhich )
                {
                    // fuer HTML gibts es nur einen Filter!!
                    pFlt = SwIoSystem::GetFilterOfFormat(
                            String::CreateFromAscii("HTML"),
                            SwWebDocShell::Factory().GetFilterContainer() );
                    nStrId = STR_LOAD_HTML_DOC;
                }
                else
                {
                    // Fuer Global-Dokumente bieten wir jetzt auch nur
                    // noch den aktuellen an.
                    pFlt = SwIoSystem::GetFilterOfFormat(
                            String::CreateFromAscii(FILTER_SW5),
                            SwGlobalDocShell::Factory().GetFilterContainer() );
                    nStrId = STR_LOAD_GLOBAL_DOC;
                }

                if( pFlt )
                {
                    pFileDlg->AddFilter( pFlt->GetUIName(),
                                ((WildCard&)pFlt->GetWildcard())(),
                                pFlt->GetTypeName() );

                    pFileDlg->SetCurFilter( pFlt->GetUIName() );
                }

                SvtPathOptions aPathOpt;
                Window *pTemplateFT = AddTemplateBtn(pFileDlg);
                pFileDlg->SetText( SW_RESSTR( nStrId ));
                pFileDlg->SetPath( aPathOpt.GetWorkPath() );

                if( RET_OK == pFileDlg->Execute() )
                {
                    aFileName = pFileDlg->GetPath();
                    sTemplateName = pTemplateFT->GetText();
                    sTemplateName.Erase(0, String(SW_RESSTR(STR_FDLG_TEMPLATE_NAME)).Len());

                    if (sTemplateName.Len())
                        pSplitColl = pDoc->FindTxtFmtCollByName(sTemplateName);
                }

                delete pFileDlg;

                if( aFileName.Len() )
                {
                    if( PrepareClose( FALSE ) &&
                        ( FN_NEW_HTML_DOC == nWhich
                            ? pWrtShell->GenerateHTMLDoc( aFileName, pSplitColl )
                            : pWrtShell->GenerateGlobalDoc( aFileName, pSplitColl )) )
                    {
                        bDone = TRUE;

                        SfxStringItem aName( SID_FILE_NAME, aFileName );
                        SfxStringItem aReferer( SID_REFERER, aEmptyStr );
                        SfxFrameItem* pFrameItem = 0;

                        SfxViewShell* pViewShell;
                        if( GetView() )
                        {
                            pViewShell = (SfxViewShell*)GetView();
                            pFrameItem = new SfxFrameItem( SID_DOCFRAME,
                                                pViewShell->GetViewFrame() );
                        }
                        else
                            pViewShell = SfxViewShell::Current();

                        pViewShell->GetDispatcher()->Execute(
                                SID_OPENDOC,
                                SFX_CALLMODE_ASYNCHRON,
                                &aName,
                                &aReferer,
                                pFrameItem, 0L );

                        if( pFrameItem )
                            delete pFrameItem;
                        else
                            DoClose();
                    }
                    else if( !rReq.IsAPI() )
                    {
                        InfoBox( 0, SW_RESSTR( STR_CANTCREATE )).Execute();
                    }
                }
            }
            rReq.SetReturnValue(SfxBoolItem( nWhich, bDone ));
            break;
        case SID_ATTR_YEAR2000:
            if ( pArgs && SFX_ITEM_SET == pArgs->GetItemState( nWhich , FALSE, &pItem ))
            {
                DBG_ASSERT(pItem->ISA(SfxUInt16Item), "falsches Item")
                USHORT nYear2K = ((SfxUInt16Item*)pItem)->GetValue();
                //ueber Views iterieren und den State an den FormShells setzen

                SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( this );
                SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
                SwView* pView = PTR_CAST(SwView, pViewShell);
                while(pView)
                {
                    FmFormShell* pFormShell = pView->GetFormShell();
                    if(pFormShell)
                        pFormShell->SetY2KState(nYear2K);
                    pVFrame = SfxViewFrame::GetNext( *pVFrame, this );
                    pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
                    pView = PTR_CAST(SwView, pViewShell);
                }
                pDoc->GetNumberFormatter(TRUE)->SetYear2000(nYear2K);
            }
        break;

        default: DBG_ERROR("falscher Dispatcher");
    }
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long SwDocShell::DdeGetData( const String& rItem, SvData& rData )
{
    return pDoc->GetData( rItem, rData );
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long SwDocShell::DdeSetData( const String& rItem, const SvData& rData )
{
    return pDoc->ChangeData( rItem, rData );
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SvPseudoObject* SwDocShell::DdeCreateHotLink( const String& rItem )
{
    return pDoc->CreateHotLink( rItem );
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDocShell::FillRegInfo( SvEmbeddedRegistryInfo*  pInfo )
{
    SfxInPlaceObject::FillRegInfo( pInfo );
    pInfo->aObjName.AssignAscii( "StarWriterDocument" );
    pInfo->nMajorVers = 5;
    pInfo->nMinorVers = 0;
    pInfo->aHumanShortTypeName = SW_RESSTR(STR_HUMAN_SWDOC_NAME);
    pInfo->nIconResId = REG_ICON_RESID_WRITER;
/*

JP 25.11.96: das sollte der SFX erledigen!

    SwFilterNms& rFltNms = *GetFilterNames();
    pInfo->aFilterList.Insert( SvFilter( Exchange::RegisterFormatName(
                                    rFltNms.GetSW4FltName()->aFilterName )) );

    SvFilter aFlt( Exchange::RegisterFormatName(
                                    rFltNms.GetSW3FltName()->aFilterName ));
    aFlt.SetExport( FALSE );
    pInfo->aFilterList.Insert( aFlt );
*/
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDocShell::FillClass( SvGlobalName * pClassName,
                                   ULONG * pClipFormat,
                                   String * pAppName,
                                   String * pLongUserName,
                                   String * pUserName,
                                   long nVersion ) const
{
    SfxInPlaceObject::FillClass(pClassName, pClipFormat, pAppName, pLongUserName,
                                pUserName, nVersion);

    if (nVersion == SOFFICE_FILEFORMAT_31)
    {
//        *pClassName       = SvGlobalName(0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61,
//                                     0x04, 0x02, 0x1C, 0x00, 0x70,0x02);
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_30 );

        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITER_30;
        pAppName->AssignAscii( "Swriter 3.1" );
        *pLongUserName  = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE_31);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_40)
    {
//        *pClassName       = SvGlobalName(0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61,
//                                     0x04, 0x02, 0x1C, 0x00, 0x70,0x02);
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_40 );

        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITER_40;
        pAppName->AssignAscii( "StarWriter 4.0" );
        *pLongUserName  = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE_40);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_NOW)
    {
        *pLongUserName = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE);
    }
    *pUserName = SW_RESSTR(STR_HUMAN_SWDOC_NAME);
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDocShell::SetModified( BOOL bSet )
{
    SfxInPlaceObject::SetModified( bSet );
    if( IsEnableSetModified() && !pDoc->IsInCallModified() )
    {
        EnableSetModified( FALSE );

        if( bSet )
        {
            BOOL bOld = pDoc->IsModified();
            pDoc->SetModified();
            if( !bOld )
                pDoc->SetUndoNoResetModified();
        }
        else
            pDoc->ResetModified();

        EnableSetModified( TRUE );
    }
    UpdateChildWindows();
    Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDocShell::UpdateChildWindows()
{
    // Flddlg ggf neu initialisieren (z.B. fuer TYP_SETVAR)
    if(!GetView())
        return;
    SfxViewFrame* pVFrame = GetView()->GetViewFrame();
    SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->
            GetChildWindow( SwFldDlgWrapper::GetChildWindowId() );
    if( pWrp )
        pWrp->ReInitDlg( this );

    // RedlineDlg ggf neu initialisieren
    SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)pVFrame->
            GetChildWindow( SwRedlineAcceptChild::GetChildWindowId() );
    if( pRed )
        pRed->ReInitDlg( this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDocShell::ReloadFromHtml( const String& rStreamName, SwSrcView* pSrcView )
{
    BOOL bModified = IsModified();

    // MIB 23.6.97: Die HTTP-Header-Felder muessen geloescht werden,
    // sonst gibt es welche aus Meta-Tags hinter doppelt und dreifach.
    ClearHeaderAttributesForSourceViewHack();

    // MIB 24.07.97: Das Dokument-Basic muss auch noch d'ran glauben ...
    // Ein EnterBasicCall braucht man hier nicht, weil man nichts ruft und
    // in HTML-Dokument kein Dok-Basic vorhanden sein kann, das noch nicht
    // geladen wurde.
    OfficeApplication* pOffApp = OFF_APP();
    OfaHtmlOptions* pHtmlOptions = pOffApp->GetHtmlOptions();
    //#59620# HasBasic() zeigt an, dass es schon einen BasicManager an der DocShell
    //          gibt. Der wurde im HTML-Import immer angelegt, wenn Macros im Quelltext
    //          vorhanden sind.
    if( pHtmlOptions && pHtmlOptions->IsStarBasic() && HasBasic())
    {
        BasicManager *pBasicMan = GetBasicManager();
        if( pBasicMan && (pBasicMan != pOffApp->GetBasicManager()) )
        {
            USHORT nLibCount = pBasicMan->GetLibCount();
            while( nLibCount )
            {
                StarBASIC *pBasic = pBasicMan->GetLib( --nLibCount );
                if( pBasic )
                {
                    // Die IDE benachrichtigen
                    String aLibName( '[' );
                    ((aLibName += GetTitle()).AppendAscii( "]." ))
                            += pBasic->GetName();
                    SfxStringItem aStrItem( SID_BASICIDE_ARG_LIBNAME, aLibName );

                    pSrcView->GetViewFrame()->GetDispatcher()->Execute(
                                            SID_BASICIDE_LIBREMOVED,
                                            SFX_CALLMODE_SYNCHRON,
                                            &aStrItem, 0L );

                    // Aus der Standard-Lib werden nur die Module geloescht
                    if( nLibCount )
                        pBasicMan->RemoveLib( nLibCount, TRUE );
                    else
                        pBasic->Clear();
                }
            }

            ASSERT( pBasicMan->GetLibCount() <= 1,
                    "Loschen des Basics hat nicht geklappt" );
        }
    }

    RemoveLink();
    delete pIo;
    pIo = 0;
    //jetzt muss auch das UNO-Model ueber das neue Doc informiert werden #51535#
    uno::Reference<text::XTextDocument> xDoc(GetBaseModel(), uno::UNO_QUERY);
    text::XTextDocument* pxDoc = xDoc.get();
    ((SwXTextDocument*)pxDoc)->InitNewDoc();


    AddLink();
    pSrcView->SetPool(&GetPool());


    String sBaseURL = INetURLObject::GetBaseURL();
    const String& rMedname = GetMedium()->GetName();
    INetURLObject::SetBaseURL( rMedname );

    // fix #51032#: Die HTML-Vorlage muss noch gesetzt werden
    SetHTMLTemplate( *GetDoc() );   //Styles aus HTML.vor

    SfxViewShell* pViewShell = GetView() ? (SfxViewShell*)GetView()
                                         : SfxViewShell::Current();
    SfxViewFrame*  pViewFrm = pViewShell->GetViewFrame();
    pViewFrm->GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_SYNCHRON );

    SubInitNew();

    SfxMedium aMed( rStreamName, STREAM_READ, FALSE );
    SwReader aReader( aMed, rMedname, pDoc );
    aReader.Read( *ReadHTML );

    INetURLObject::SetBaseURL(sBaseURL);


    // MIB 23.6.97: Die HTTP-Header-Attribute wieder in die DokInfo
    // uebernehmen. Die Base-URL ist hier egal, da TLX zum absolutieren die
    // vom Dokument nimmt.
    SetHeaderAttributesForSourceViewHack();

    if(bModified && !IsReadOnly())
        SetModified();
    else
        pDoc->ResetModified();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

Window *SwDocShell::AddTemplateBtn(SfxFileDialog* pFileDlg)
{
    PushButton *pTemplateBtn = new PushButton( pFileDlg );
    pTemplateBtn->SetText( SW_RESSTR(STR_FDLG_TEMPLATE_BUTTON) );
    pTemplateBtn->SetClickHdl(LINK(this, SwDocShell, SelTemplateHdl));
    pTemplateBtn->SetHelpId(HID_SELECT_TEMPLATE);
#if (SUPD>503)
    pFileDlg->AddControl( pTemplateBtn, TRUE );
#else
    pFileDlg->AddControl( pTemplateBtn );
#endif
    pTemplateBtn->Show();

    FixedText *pTemplateFT = new FixedText( pFileDlg );

    String sName( SW_RES( STR_FDLG_TEMPLATE_NAME ));
    sName += SwSelTemplateDlg::GetStartTemplate( *pWrtShell );
    pTemplateFT->SetText(sName);

    pFileDlg->AddControl( pTemplateFT );
    pTemplateFT->SetSizePixel(Size(pTemplateFT->LogicToPixel(Size(160, 1), MAP_APPFONT).Width(), pTemplateFT->GetSizePixel().Height()));
    pTemplateFT->Show();

    pTemplateBtn->SetData(pTemplateFT); // Verweis auf den Template-Namen

    // Die hinzugefuegten Controls werden spaeter vom FileDlg geloescht
    return pTemplateFT;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwDocShell, SelTemplateHdl, PushButton *, pBtn )
{
    if (pWrtShell)
    {
        FixedText *pFT = (FixedText*)pBtn->GetData();
        String sFixName(SW_RES(STR_FDLG_TEMPLATE_NAME));

        String sName(pFT->GetText());
        sName.Erase(0, sFixName.Len());

        SwSelTemplateDlg aDlg( pBtn, pWrtShell, sName);

        if( aDlg.Execute() == RET_OK )
        {
            sFixName += sName;
            pFT->SetText(sFixName);
        }
    }

    return 0;
}
/* -----------------------------14.12.99 16:52--------------------------------

 ---------------------------------------------------------------------------*/
void    SwDocShell::ToggleBrowserMode(BOOL bSet, SwView* pView )
{
    GetDoc()->SetBrowseMode( bSet );
    UpdateFontList();
    SwView* pTempView = pView ? pView : (SwView*)GetView();
    if( pTempView )
    {
        pTempView->GetViewFrame()->GetBindings().Invalidate(FN_SHADOWCURSOR);
        if( !GetDoc()->GetPrt( FALSE ) )
            pTempView->SetPrinter( GetDoc()->GetPrt( TRUE ),
                    SFX_PRINTER_PRINTER|SFX_PRINTER_JOBSETUP );

        //Wenn wir die BrowseView einschalten, darf es nur diese eine
        //Sicht auf das Dokument geben, alle anderen werden geschlossen.
        SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this, 0, FALSE);
        do {
            if( pTmpFrm != pTempView->GetViewFrame() )
            {
                pTmpFrm->DoClose();
                pTmpFrm = SfxViewFrame::GetFirst(this, 0, FALSE);
            }
            else
                pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, this, 0, FALSE);

        } while ( pTmpFrm );

        pTempView->GetWrtShell().CheckBrowseView( TRUE );
        pTempView->CheckVisArea();

        SvxZoomType eType;
        if( GetDoc()->IsBrowseMode() &&
              SVX_ZOOM_PERCENT != (eType = (SvxZoomType)pTempView->
                            GetWrtShell().GetViewOptions()->GetZoomType()) )
        {
            ((SwView*)GetView())->SetZoom( eType );
        }
        pTempView->InvalidateBorder();
        pTempView->SetNewWindowAllowed(!bSet);
    }
}

ULONG SwDocShell::LoadStylesFromFile( const String& rURL,
                    SwgReaderOption& rOpt, BOOL bUnoCall )
{
    ULONG nErr = 0;

    // Create a URL from filename
    INetURLObject aURLObj( rURL );
    String sURL( aURLObj.GetMainURL() );

    String sBaseURL( INetURLObject::GetBaseURL() );
    INetURLObject::SetBaseURL( sURL );

    SwRead pRead = 0;
    SwReader* pReader;
    SwPaM* pPam = 0;

    // Filter bestimmen:
    const SfxFilter* pFlt = SwIoSystem::GetFileFilter( rURL, aEmptyStr );
    SfxMedium aMed( rURL, STREAM_STD_READ, FALSE );
    if( aMed.IsStorage() )
    {
        if( pFlt && pFlt->GetVersion() )
            aMed.GetStorage()->SetVersion( (long)pFlt->GetVersion() );
        pRead = ReadSw3;
        // the SW3IO - Reader need the pam/wrtshell, because only then he
        // insert the styles!
        if( bUnoCall )
        {
            SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfContent(), -1 );
            pPam = new SwPaM( aIdx );
            pReader = new SwReader( aMed, rURL, *pPam );
        }
        else
            pReader = new SwReader( aMed, rURL, *pWrtShell->GetCrsr() );
    }
    else if( pFlt )
    {
        if( pFlt->GetUserData().EqualsAscii( FILTER_SWG ) ||
            pFlt->GetUserData().EqualsAscii( FILTER_SWGV ))
            pRead = ReadSwg;
        else if( pFlt->GetUserData().EqualsAscii( FILTER_XML ))
            pRead = ReadXML;
        pReader = new SwReader( aMed, rURL, pDoc );
    }

    ASSERT( pRead, "no reader found" );
    if( pRead )
    {
        pRead->GetReaderOpt().SetTxtFmts( rOpt.IsTxtFmts() );
        pRead->GetReaderOpt().SetFrmFmts( rOpt.IsFrmFmts() );
        pRead->GetReaderOpt().SetPageDescs( rOpt.IsPageDescs() );
        pRead->GetReaderOpt().SetNumRules( rOpt.IsNumRules() );
        pRead->GetReaderOpt().SetMerge( rOpt.IsMerge() );

        if( bUnoCall )
        {
            UnoActionContext aAction( pDoc );
            nErr = pReader->Read( *pRead );
        }
        else
        {
            pWrtShell->StartAllAction();
            nErr = pReader->Read( *pRead );
            pWrtShell->EndAllAction();
        }
    }
    delete pPam;
    delete pReader;
    INetURLObject::SetBaseURL( sBaseURL );
    return nErr;
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.8  2000/11/01 10:13:28  jp
    new method LoadStyleFromFile for docshell and uno

    Revision 1.7  2000/10/31 20:32:32  jp
    change usage of filestream to medium

    Revision 1.6  2000/10/26 11:23:10  jp
    Bug #75694#: use replace instead close & open

    Revision 1.5  2000/10/23 18:12:37  jp
    ToggleBrowserMode without GPF

    Revision 1.4  2000/10/19 13:16:35  jp
    DocSh:Execute: call DoClose after the new load is call

    Revision 1.3  2000/10/06 13:31:28  jp
    should changes: don't use IniManager

    Revision 1.2  2000/09/26 13:33:18  jp
    SFXDISPATHER removed

    Revision 1.1.1.1  2000/09/18 17:14:31  hr
    initial import

------------------------------------------------------------------------*/



