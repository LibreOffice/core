/*************************************************************************
 *
 *  $RCSfile: docsh2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-19 13:16:35 $
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
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
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
#include "sfx2/iodlg.hxx"
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
#ifndef _ACMPLWRD_HXX //autogen
#include <acmplwrd.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
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
                String sSW5(String::CreateFromAscii(FILTER_SW5));

                SfxObjectFactory &rFact = GetFactory();
                for( USHORT i = 0; i < rFact.GetFilterCount(); i++ )
                {
                    const SfxFilter* pFlt = rFact.GetFilter( i );
                    if( pFlt && pFlt->IsAllowedAsTemplate() )
                        pDlg->AddFilter( pFlt->GetUIName(),
                                    ((WildCard&)pFlt->GetWildcard())(),
                                    pFlt->GetTypeName() );

                    if( pFlt->GetUserData() == sSW5 )
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
                bText       = pDlg->IsTextStyle();
                bFrame      = pDlg->IsFrameStyle();
                bPage       = pDlg->IsPageStyle();
                bOverwrite  = pDlg->IsOverwrite();
                bNumbering  = pDlg->IsNumbering();
                USHORT nFlags = (USHORT)bText +
                                ((USHORT)bFrame << 1) +
                                ((USHORT)bPage << 2) +
                                ((USHORT)bOverwrite << 3) +
                                ((USHORT)bNumbering << 4);
                delete pDlg;
                if(aFileName.Len())
                {
                    // Create a URL from filename
                    INetURLObject aURLObj;
                    aURLObj.SetSmartProtocol( INET_PROT_FILE );
                    aURLObj.SetURL( aFileName );
                    if( INET_PROT_FILE != aURLObj.GetProtocol() )
                        break;

                    String sURL( aURLObj.GetMainURL() );

                    String sBaseURL( INetURLObject::GetBaseURL() );
                    INetURLObject::SetBaseURL( sURL );

                    SvStorageRef pStor;
                    SvFileStream* pStream = 0;
                    SwRead pRead = 0;
                    SwReader* pReader = 0;
                    if( SvStorage::IsStorageFile( aFileName ))
                    {
                        // Filter bestimmen:
                        const SfxFilter* pFlt = SwIoSystem::GetFileFilter(
                                                    aFileName, aEmptyStr );
                        pStor = new SvStorage( aFileName, STREAM_STD_READ );

                        if( pFlt && pFlt->GetVersion() )
                            pStor->SetVersion( (long)pFlt->GetVersion() );

                        pRead = ReadSw3;
                        pReader = new SwReader(*pStor, sURL,
                                               *pWrtShell->GetCrsr() );
                    }
                    else
                    {
                        const SfxFilter* pFlt = SwIoSystem::GetFileFilter(
                                                    aFileName, aEmptyStr );
                        if( pFlt )
                        {
                            if( pFlt->GetUserData() == String::CreateFromAscii(FILTER_SWG) ||
                                pFlt->GetUserData() == String::CreateFromAscii(FILTER_SWGV) )
                            {
                                pRead = ReadSwg;
                            }
                            else if( pFlt->GetUserData() == String::CreateFromAscii(FILTER_XML) )
                            {
                                pRead = ReadXML;
                            }
                        }

                        if( pRead )
                        {
                            pStream = new SvFileStream( aFileName,
                                                        STREAM_STD_READ );
                            pReader = new SwReader(*pStream, sURL, pDoc);
                        }
                    }
                    ASSERT( pRead, "no reader found" );
                    if( pRead )
                    {
                        pRead->GetReaderOpt().SetAllFmtsOnly();
                        pRead->GetReaderOpt().SetTxtFmts(bText);
                        pRead->GetReaderOpt().SetFrmFmts(bFrame);
                        pRead->GetReaderOpt().SetPageDescs(bPage);
                        pRead->GetReaderOpt().SetNumRules(bNumbering);
                        pRead->GetReaderOpt().SetMerge(!bOverwrite);

                        pWrtShell->StartAllAction();
                        SetError( pReader->Read( *pRead ));
                        pWrtShell->EndAllAction();
                    }


                    delete pReader;
                    delete pStream;

                    INetURLObject::SetBaseURL( sBaseURL );
                }
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
                        TempFile aTempFile;
                        aTempFile.EnableKillingFile();
                        pSrcView->SaveContent(aTempFile.GetName());
                        bDone = TRUE;
                        SetActualSize(pSrcView->GetEditWin().GetSizePixel());
                        SfxEventConfiguration* pEvent = SFX_APP()->GetEventConfig();
                        SvxMacro aMac(aEmptyStr, aEmptyStr, STARBASIC);
                        pEvent->ConfigureEvent(SFX_EVENT_OPENDOC,       aMac, this);
                        pEvent->ConfigureEvent(SFX_EVENT_CLOSEDOC,      aMac, this);
                        pEvent->ConfigureEvent(SFX_EVENT_ACTIVATEDOC,   aMac, this);
                        pEvent->ConfigureEvent(SFX_EVENT_DEACTIVATEDOC, aMac, this);
                        ReloadFromHtml(aTempFile.GetName(), pSrcView);
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
                pWrtShell->GetView().GetViewFrame()->GetBindings().Invalidate( aInva );
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
                    pFlt = SwIoSystem::GetFilterOfFormat( String::CreateFromAscii("HTML"),
                            SwWebDocShell::Factory().GetFilterContainer() );
                    nStrId = STR_LOAD_HTML_DOC;
                }
                else
                {
                    // Fuer Global-Dokumente bieten wir jetzt auch nur
                    // noch den aktuellen an.
                    pFlt = SwIoSystem::GetFilterOfFormat( String::CreateFromAscii(FILTER_SW5),
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

                        SfxViewFrame::Current()->GetDispatcher()->Execute(
                                SID_OPENDOC,
                                SFX_CALLMODE_ASYNCHRON,
                                &aName,
                                &aReferer,
                                0L );

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
    pInfo->aObjName = String::CreateFromAscii("StarWriterDocument");
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
        *pAppName       = String::CreateFromAscii("Swriter 3.1");
        *pLongUserName  = SW_RESSTR(STR_WRITER_DOCUMENT_FULLTYPE_31);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_40)
    {
//        *pClassName       = SvGlobalName(0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61,
//                                     0x04, 0x02, 0x1C, 0x00, 0x70,0x02);
        *pClassName     = SvGlobalName( SO3_SW_CLASSID_40 );

        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITER_40;
        *pAppName       = String::CreateFromAscii("StarWriter 4.0");
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
    SvFileStream aStream(rStreamName, STREAM_READ);

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
                    String aLibName( String::CreateFromAscii("[" ));
                    ((aLibName += GetTitle()) += String::CreateFromAscii("].")) += pBasic->GetName();
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

    SwReader aReader( aStream, rMedname, pDoc );
    aReader.Read( *ReadHTML );
    aStream.Close();
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
    SwView* pTempView = pView ? pView : (SwView*)GetView();
    pTempView->GetViewFrame()->GetBindings().Invalidate(FN_SHADOWCURSOR);
    UpdateFontList();
    if( pTempView )
    {
        if( !GetDoc()->GetPrt( FALSE ) )
            pTempView->SetPrinter( GetDoc()->GetPrt( TRUE ),
                    SFX_PRINTER_PRINTER|SFX_PRINTER_JOBSETUP );

        //Wenn wir die BrowseView einschalten, darf es nur diese eine
        //Sicht auf das Dokument geben, alle anderen werden geschlossen.
        SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this, 0, FALSE);
        do
        {   if ( pTmpFrm != pTempView->GetViewFrame() )
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
        if ( GetDoc()->IsBrowseMode() &&
                SVX_ZOOM_PERCENT !=
                (eType = (SvxZoomType)pTempView->GetWrtShell().GetViewOptions()->GetZoomType()) )
        {
            ((SwView*)GetView())->SetZoom( eType );
        }
        pTempView->InvalidateBorder();
        pTempView->SetNewWindowAllowed(!bSet);
    }
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.3  2000/10/06 13:31:28  jp
    should changes: don't use IniManager

    Revision 1.2  2000/09/26 13:33:18  jp
    SFXDISPATHER removed

    Revision 1.1.1.1  2000/09/18 17:14:31  hr
    initial import

    Revision 1.277  2000/09/18 16:05:10  willem.vandorp
    OpenOffice header added.

    Revision 1.276  2000/09/08 08:33:26  os
    check GetView()

    Revision 1.275  2000/09/08 08:12:50  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.274  2000/09/07 15:59:20  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.273  2000/08/30 16:12:40  os
    #78341# save document before switching to source view

    Revision 1.272  2000/08/14 17:27:15  jp
    Task #77422#: PrintPreView in the same window

    Revision 1.271  2000/08/07 11:48:36  jp
    PagePreView replace the current view

    Revision 1.270  2000/06/09 08:10:13  os
    using tools/tempfile

    Revision 1.269  2000/05/30 13:39:55  os
    #70840# HTML source view: for saved documents only, call save as dialog if needed

    Revision 1.268  2000/05/26 07:21:27  os
    old SW Basic API Slots removed

    Revision 1.267  2000/04/19 12:56:33  os
    include sfx2/filedlg.hxx removed

    Revision 1.266  2000/04/11 08:01:30  os
    UNICODE

    Revision 1.265  2000/03/21 15:47:50  os
    UNOIII

    Revision 1.264  2000/03/08 17:26:34  os
    GetAppWindow() - misuse as parent window eliminated

    Revision 1.263  2000/03/03 16:16:32  pl
    #73771# workaround for c50 intel compiler

    Revision 1.262  2000/03/03 15:16:58  os
    StarView remainders removed

    Revision 1.261  2000/03/03 12:27:49  mib
    Removed JavaScript

    Revision 1.260  2000/02/24 17:01:58  hr
    #73447#: removed temporaries

    Revision 1.259  2000/02/11 14:43:03  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.258  2000/02/02 17:00:33  jp
    Task #72579#: interface of SwReader is changed

    Revision 1.257  2000/01/14 16:29:50  jp
    Task #71894#: new Options for SW-AutoComplete

    Revision 1.256  2000/01/13 21:27:22  jp
    Task #71894#: new Options for SW-AutoComplete

    Revision 1.255  2000/01/11 10:22:04  tl
    #70735# CheckSpellChanges moved to SwModule and called from there

    Revision 1.254  1999/12/14 16:11:08  os
    #70234# Set Browser Mode via API

    Revision 1.253  1999/11/26 10:58:35  mib
    Loading of styles from XML documents

    Revision 1.252  1999/11/11 10:40:03  jp
    SetModified: broadcast the hint DOCCHANGED

    Revision 1.251  1999/11/09 09:44:04  jp
    SetModified: do nothing if EnableSetModified is not set

    Revision 1.250  1999/10/29 11:31:31  os
    support XModifiable

    Revision 1.249  1999/10/25 19:37:25  tl
    ongoing ONE_LINGU implementation

    Revision 1.248  1999/10/21 17:45:27  jp
    have to change - SearchFile with SfxIniManager, dont use SwFinder for this

    Revision 1.247  1999/10/19 14:30:57  hr
    #65293#: sfxecode.hxx now in svtools

    Revision 1.246  1999/08/31 08:33:36  TL
    #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.245   31 Aug 1999 10:33:36   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.244   22 Jun 1999 17:20:30   KZ
   Anpass. fuer Linux

      Rev 1.243   10 Jun 1999 10:52:10   JP
   have to change: no AppWin from SfxApp

      Rev 1.242   04 May 1999 14:58:44   JP
   FilterExportklasse Writer von SvRef abgeleitet, damit sie immer zerstoert wird

      Rev 1.241   20 Apr 1999 17:14:34   MA
   #63638# API-Aenderung PrepareClose

      Rev 1.240   16 Apr 1999 17:22:10   JP
   Bug #64928#: erfrage an der Vorlage, ob sie an einem Node gesetzt ist

      Rev 1.239   07 Apr 1999 14:13:18   OS
   #59615# ReleaseDocument aufrufen

      Rev 1.238   29 Mar 1999 15:57:42   OS
   #59615# InsertDocument am SfxJS rufen

      Rev 1.237   15 Mar 1999 15:07:30   JP
   Task #61405#: AutoCompleteList wurde statisch

      Rev 1.236   11 Mar 1999 23:59:42   JP
   Task #61405#: Optionen setzen

      Rev 1.235   09 Mar 1999 14:10:12   OS
   #62286# SetY2KState an die FormShell weiterleiten

      Rev 1.234   03 Mar 1999 09:44:46   MIB
   #60932#: Indizes vom Organizer nun richtig

      Rev 1.233   02 Mar 1999 16:01:48   AMA
   Fix #62568#: Invalidierungen so sparsam wie moeglich, so gruendlich wie noetig

      Rev 1.232   03 Feb 1999 11:46:48   JP
   Bug #61401#: am Storage die VersionsNummer vom Filter setzen

      Rev 1.231   02 Feb 1999 08:42:44   OS
   #61027# zweistellige Jahreszahlen

      Rev 1.230   01 Feb 1999 08:23:36   OS
   #56371# unnoetigen include wieder raus

      Rev 1.229   27 Jan 1999 09:51:50   OS
   #56371# TF_ONE51

      Rev 1.228   19 Jan 1999 12:05:00   OS
   #59620 vor GetBasicManager mit HasBasic pruefen

      Rev 1.227   14 Jan 1999 10:18:56   MIB
   #47231#: Organizer: Ersetzen von Vorlagen ohne Loeschen

      Rev 1.226   12 Jan 1999 12:49:24   OS
   #60705# AutoFormat-Dialog nicht asynchron aufrufen!

      Rev 1.225   18 Dec 1998 17:18:10   MIB
   #60483#: Poolvorlagen beim D&D im Organizer, Parents und Follows richtig setzen

      Rev 1.224   09 Dec 1998 16:37:54   OM
   #59628# Warnung anzeigen, wenn Dok nicht angelegt werden konnt

      Rev 1.223   09 Dec 1998 14:31:10   OS
   #56371# TF_ONE51

      Rev 1.222   25 Nov 1998 12:42:52   OS
   #59826# temp. Datei in der DocShell loeschen

      Rev 1.221   25 Nov 1998 11:26:56   OS
   #59240##59637# SubInitNew vor dem Import, kein TabStop fuer HTML

      Rev 1.220   17 Nov 1998 22:18:02   JP
   Task #59398#: ClipboardFormatId Umstellungen

      Rev 1.219   05 Nov 1998 10:08:50   OS
   #58996# SourceView: Flag fuers speichern

      Rev 1.218   22 Sep 1998 12:34:00   JP
   Bug #55959#: SetBase mit einer URL und keinem FileNamen - der Dialog liefert keine URL (ist eig. der Bug!)

      Rev 1.217   21 Sep 1998 11:57:36   JP
   Bug #55959#: beim Laden von Vorlagen, muss die BaseURL richtig gesetzt sein

      Rev 1.216   03 Sep 1998 13:27:44   OS
   #55662# AddControl mit NewLine

      Rev 1.215   23 Jul 1998 11:14:36   JP
   Task #52654#: Einfuegen Doc nicht mit einer CrsrShell sondern mit einen PaM

      Rev 1.214   09 Jul 1998 09:52:44   JP
   EmptyStr benutzen

      Rev 1.213   08 Jul 1998 16:21:00   JP
   Bug #52411#: ReturnWert vom Convert korrekt auswerten

      Rev 1.212   02 Jul 1998 14:34:40   MIB
   Fix #52035#: Beim Unschalten aus der Source-View View vor dem Import umschalten

      Rev 1.211   25 Jun 1998 11:52:22   OM
   #51665# Language fuer Zahlenformate

      Rev 1.210   23 Jun 1998 18:13:50   OS
   InitNewDoc - Member des XTextDocuments zuruecksetzen #51535#

      Rev 1.209   18 Jun 1998 11:02:40   OS
   FN_NEW_GLOBAL_DOC: ohne Vorlagennamen Null uebergeben

      Rev 1.208   18 Jun 1998 10:08:08   OS
   Global-Doc erzeugen: Ueberschrift 1 als default #51263#

      Rev 1.207   15 Jun 1998 09:54:50   MIB
   fix #51032#: HTML-Vorlage beim Umsch. aus der Source-View laden und Seitenvorlage setzen

      Rev 1.206   12 Jun 1998 09:45:58   OS
   Appear statt ToTop #50485#

      Rev 1.205   28 May 1998 17:40:52   OM
   Zahlenformat per Basic ermitteln

      Rev 1.204   19 May 1998 12:45:22   OM
   SvxMacro-Umstellung

      Rev 1.203   15 May 1998 17:26:12   OM
   #50043# DirectCursor nicht im OnlineLayout

      Rev 1.202   13 May 1998 14:52:34   OM
   Autokorrektur/Autoformat umgestaltet und zusammengefasst

      Rev 1.201   24 Apr 1998 16:01:04   OM
   Seitenvorschau auch im Html-Mode

      Rev 1.200   23 Apr 1998 15:35:36   OM
   #49689 Aenderung der Seitenanzahl per Basic konfigurierbar

      Rev 1.199   02 Apr 1998 10:09:52   MIB
   Beim Erzeugen eines Global-Dokuments nur noch den 5.0-Filter anbieten

      Rev 1.198   27 Mar 1998 14:12:32   OM
   ChildWindows im Modified-Hdl updaten

      Rev 1.197   20 Mar 1998 14:57:52   OM
   Vorlagentext verbreitert

      Rev 1.196   20 Mar 1998 14:42:42   OM
   Fehlender Include

      Rev 1.195   20 Mar 1998 12:41:58   OM
   Helpid fuer Template-Button

      Rev 1.194   20 Mar 1998 12:36:32   OM
   Globaldokument: Vorlage auswaehlen

      Rev 1.193   19 Mar 1998 17:40:56   OM
   Vorlage auswehlen

      Rev 1.192   17 Mar 1998 13:04:38   JP
   fuers GenerateHTMLDoc: nur den HTML-Filter anbieten

      Rev 1.191   17 Mar 1998 12:23:04   JP
   neu: GenerateHTMLDoc - aquivalent zu GenerateGlobalDoc

      Rev 1.190   12 Mar 1998 13:01:54   OS
   SID_NEWWINDOW nicht mehr ueberladen

      Rev 1.189   02 Feb 1998 13:53:26   OS
   SID_VIEWSH* am eigenen Dispatcher rufen #46862#

      Rev 1.188   19 Jan 1998 11:52:34   MBA
   CreateViewFrame verwenden

      Rev 1.187   07 Jan 1998 18:42:08   MIB
   5.0 Fileformat

      Rev 1.186   09 Dec 1997 08:19:06   OS
   Vorlagen laden: auch Numerierung

      Rev 1.185   29 Nov 1997 15:25:40   MA
   includes

      Rev 1.184   24 Nov 1997 14:22:48   MA
   includes

      Rev 1.183   11 Nov 1997 11:37:22   AMA
   Fix #44514#: Fuer die Druckvorschau braucht man einen Drucker

      Rev 1.182   30 Oct 1997 18:24:44   JP
   Options am Reader nicht mehr public

      Rev 1.181   28 Oct 1997 14:52:34   OS
   SetPool an der SourceView sofort nach AddLink rufen

      Rev 1.180   24 Oct 1997 15:18:58   OS
   ReloadFromHtml nur noch rufen, wenn das Doc im SourceMode veraendert wurde #44971#

      Rev 1.179   13 Oct 1997 17:23:22   MA
   #44630# Keine SwDokInfo fuer SrcView

      Rev 1.178   08 Oct 1997 14:28:20   OS
   PrintLayout funktioniert jetzt auch aus dem Basic

      Rev 1.177   15 Sep 1997 13:26:06   OS
   Events am Doc vor Reload loeschen #42231#

      Rev 1.176   12 Sep 1997 16:04:50   OS
   Vorlagen laden: EAs uebergeben, SW4-Filter als Standard #43699#

      Rev 1.175   01 Sep 1997 13:06:34   OS
   DLL-Umstellung

      Rev 1.174   14 Aug 1997 15:23:36   MA
   fix: DocShell ohne Doc bei OLE-Registration

------------------------------------------------------------------------*/



