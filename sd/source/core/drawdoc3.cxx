/*************************************************************************
 *
 *  $RCSfile: drawdoc3.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:55:42 $
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

#include <utility>
#include <algorithm>

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#include <svtools/style.hxx>
#include <svx/linkmgr.hxx>
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#include <set>

#include "glob.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "sdresid.hxx"
#include "sdiocmpt.hxx"
#include "strmname.h"
#include "anminfo.hxx"

#ifdef MAC
#include "::ui:inc:unmovss.hxx"
#include "::ui:inc:unchss.hxx"
#include "::ui:inc:unprlout.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "::ui:inc:DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "::ui:inc:GraphicDocShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "::ui:inc:ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "::ui:inc:View.hxx"
#endif
#include "::ui:inc:cfgids.hxx"
#include "::ui:inc:strings.hrc"
#else
#ifdef UNX
#include "../ui/inc/unmovss.hxx"
#include "../ui/inc/unchss.hxx"
#include "../ui/inc/unprlout.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "../ui/inc/DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "../ui/inc/GraphicDocShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "../ui/inc/ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "../ui/inc/View.hxx"
#endif
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"
#else
#include "..\ui\inc\unmovss.hxx"
#include "..\ui\inc\unchss.hxx"
#include "..\ui\inc\unprlout.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "..\ui\inc\DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "..\ui\inc\GraphicDocShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "..\ui\inc\ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "..\ui\inc\View.hxx"
#endif
#include "..\ui\inc\cfgids.hxx"
#include "..\ui\inc\strings.hrc"
#endif
#endif

using namespace ::com::sun::star;

#define POOL_BUFFER_SIZE        (USHORT)32768
#define BASIC_BUFFER_SIZE       (USHORT)8192
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/

SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(SfxMedium& rMedium)
{
    BOOL bOK = TRUE;
    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName = rMedium.GetName();

    if (aBookmarkFile != aBookmarkName && aBookmarkName.Len() && rMedium.IsStorage())
    {
        DBG_ASSERT( rMedium.IsStorage(), "Kein Storage, keine Banane!" );

        SvStorage* pStorage = rMedium.GetStorage();

        if( !pStorage->IsStream( pStarDrawDoc ) &&
            !pStorage->IsStream( pStarDrawDoc3 ) &&
            !pStorage->IsStream( pStarDrawXMLContent ) &&
            !pStorage->IsStream( pStarDrawOldXMLContent ))
        {
            // Es ist nicht unser Storage
            DBG_ASSERT(bOK, "Nicht unser Storage");
            bOK = FALSE;
        }
        else
        {
            CloseBookmarkDoc();

            aBookmarkFile = aBookmarkName;

            // Es wird eine DocShell erzeugt, da in dem Dokument OLE-Objekte
            // enthalten sein koennten (Persist)
            // Wenn dem nicht so waere, so koennte man auch das Model
            // direkt laden

            if ( pStorage->GetFormat() == SOT_FORMATSTR_ID_STARDRAW_50 )
                // Draw
                xBookmarkDocShRef = new ::sd::GraphicDocShell(SFX_CREATE_MODE_STANDARD, TRUE);
            else
                // Impress
                xBookmarkDocShRef = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD, TRUE);

            if ( bOK = xBookmarkDocShRef->DoLoad(pStorage) )
                pBookmarkDoc = xBookmarkDocShRef->GetDoc();
        }
    }
    else
    {
        // Kein Storage
        bOK = FALSE;
        DBG_ASSERT(bOK, "Kein Storage");
    }

    if (!bOK)
    {
        ErrorBox aErrorBox( NULL, (WinBits)WB_OK, String(SdResId(STR_READ_DATA_ERROR)));
        aErrorBox.Execute();

        CloseBookmarkDoc();
        pBookmarkDoc = NULL;
    }
    else if (xBookmarkDocShRef.Is())
    {
        pBookmarkDoc = xBookmarkDocShRef->GetDoc();
    }

    return(pBookmarkDoc);
}

/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/

SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(const String& rBookmarkFile)
{
    SdDrawDocument* pBookmarkDoc = NULL;

    if (aBookmarkFile != rBookmarkFile && rBookmarkFile.Len())
    {
        // Das Medium muss als read/write geoeffnet werden, da ev.
        // OLE-Objekte geclont werden muessen (innerhalb des Mediums)

        // #70116#: OpenMode is set only to STREAM_READ
        SfxMedium* pMedium = new SfxMedium(rBookmarkFile,
                                           STREAM_READ /*WRITE | STREAM_SHARE_DENYWRITE
                                           | STREAM_NOCREATE */,
                                           FALSE );  // direkt

        if (pMedium->IsStorage())
        {
            if (!pMedium->GetStorage())
            {
                // READ/WRITE hat nicht geklappt, also wieder READ
                pMedium->Close();
                pMedium->SetOpenMode(STREAM_READ | STREAM_NOCREATE,
                                     FALSE);

                // Nun wird eine Kopie angelegt. In diese Kopie darf
                // geschrieben werden
                SfxMedium* pTempMedium = new SfxMedium(*pMedium, TRUE);
                pBookmarkDoc = OpenBookmarkDoc(*pTempMedium);
                delete pTempMedium;
            }
            else
            {
                pBookmarkDoc = OpenBookmarkDoc(*pMedium);
            }
        }

        delete pMedium;
    }
    else if (xBookmarkDocShRef.Is())
    {
        pBookmarkDoc = xBookmarkDocShRef->GetDoc();
    }

    return(pBookmarkDoc);
}

/*************************************************************************
|*
|* Fuegt ein Bookmark (Seite oder Objekt) ein
|*
\************************************************************************/

BOOL SdDrawDocument::InsertBookmark(
    List* pBookmarkList,            // Liste der Namen der einzufuegenden Bookmarks
    List* pExchangeList,            // Liste der zu verwendenen Namen
    BOOL bLink,                     // Bookmarks sollen als Verknuepfung eingefuegt werden
    BOOL bReplace,                  // Aktuellen Seiten (Standard&Notiz) werden ersetzt
    USHORT nInsertPos,              // Einfuegeposition fuer Seiten
    BOOL bNoDialogs,                // Keine Dialoge anzeigen
    ::sd::DrawDocShell* pBookmarkDocSh, // Wenn gesetzt, so ist dieses das Source-Dokument
    BOOL bCopy,                     // Seiten werden kopiert
    Point* pObjPos)                 // Einfuegeposition fuer Objekte
{
    BOOL bOK = TRUE;
    BOOL bInsertPages = FALSE;

    if (!pBookmarkList)
    {
        /**********************************************************************
        * Alle Seiten werden eingefuegt
        **********************************************************************/
        bInsertPages = TRUE;
    }
    else
    {
        SdDrawDocument* pBookmarkDoc = NULL;
        String aBookmarkName;

        if (pBookmarkDocSh)
        {
            pBookmarkDoc = pBookmarkDocSh->GetDoc();
            aBookmarkName = pBookmarkDocSh->GetMedium()->GetName();
        }
        else if ( xBookmarkDocShRef.Is() )
        {
            pBookmarkDoc = xBookmarkDocShRef->GetDoc();
            aBookmarkName = aBookmarkFile;
        }
        else
            bOK = FALSE;

        for (USHORT nPos = 0; bOK && ( nPos < pBookmarkList->Count() ) && !bInsertPages; nPos++)
        {
            /******************************************************************
            * Gibt es in der Bookmark-Liste einen Seitennamen?
            ******************************************************************/
            String  aBMPgName (*(String*) pBookmarkList->GetObject(nPos));
            BOOL    bIsMasterPage;

            if( pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // Seite gefunden
                bInsertPages = TRUE;
            }
        }
    }

    if ( bOK && bInsertPages )
    {
        // Zuerst werden alle Seiten-Bookmarks eingefuegt
        bOK = InsertBookmarkAsPage(pBookmarkList, pExchangeList, bLink, bReplace,
                                   nInsertPos, bNoDialogs, pBookmarkDocSh, bCopy, TRUE, FALSE);
    }

    if ( bOK && pBookmarkList )
    {
        // Es werden alle Objekt-Bookmarks eingefuegt
        bOK = InsertBookmarkAsObject(pBookmarkList, pExchangeList, bLink,
                                     pBookmarkDocSh, pObjPos);
    }

    return bOK;
}

/*************************************************************************
|*
|* Fuegt ein Bookmark als Seite ein
|*
\************************************************************************/

/** Concrete incarnations get called by IterateBookmarkPages, for
    every page in the bookmark document/list
 */
class SdDrawDocument::InsertBookmarkAsPage_PageFunctorBase
{
public:
    virtual ~InsertBookmarkAsPage_PageFunctorBase() = 0;
    virtual void operator()( SdDrawDocument&, SdPage* ) = 0;
};

SdDrawDocument::InsertBookmarkAsPage_PageFunctorBase::~InsertBookmarkAsPage_PageFunctorBase()
{
}

void SdDrawDocument::IterateBookmarkPages( SdDrawDocument* pBookmarkDoc, List* pBookmarkList, USHORT nBMSdPageCount,
                                           SdDrawDocument::InsertBookmarkAsPage_PageFunctorBase& rPageIterator )
{
    //
    // #96029# Refactored copy'n'pasted layout name collection from InsertBookmarkAsPage
    //
    int nPos, nEndPos;

    if( !pBookmarkList )
    {
        // no list? whole source document
        nEndPos = nBMSdPageCount;
    }
    else
    {
        // bookmark list? number of entries
        nEndPos = pBookmarkList->Count();
    }

    SdPage* pBMPage;

    // iterate over number of pages to insert
    for (nPos = 0; nPos < nEndPos; ++nPos)
    {
        // the master page associated to the nPos'th page to insert
        SdPage* pBMMPage = NULL;

        if( !pBookmarkList )
        {
            // simply take master page of nPos'th page in source document
            pBMMPage = (SdPage*)(&(pBookmarkDoc->GetSdPage(nPos, PK_STANDARD)->TRG_GetMasterPage()));
        }
        else
        {
            // fetch nPos'th entry from bookmark list, and determine master page
            String  aBMPgName (*(String*) pBookmarkList->GetObject(nPos));
            BOOL    bIsMasterPage;

            USHORT nBMPage = pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                pBMPage = (SdPage*) pBookmarkDoc->GetPage(nBMPage);
            }
            else
            {
                pBMPage = NULL;
            }

            // enforce that bookmarked page is a standard page and not already a master page
            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                const USHORT nBMSdPage = (nBMPage - 1) / 2;
                pBMMPage = (SdPage*) (&(pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD)->TRG_GetMasterPage()));
            }
        }

        // successfully determined valid (bookmarked) page?
        if( pBMMPage )
        {
            // yes, call functor
            rPageIterator( *this, pBMMPage );
        }
    }
}

class InsertBookmarkAsPage_FindDuplicateLayouts : public SdDrawDocument::InsertBookmarkAsPage_PageFunctorBase
{
public:
    InsertBookmarkAsPage_FindDuplicateLayouts( List* pLayoutsToTransfer, SdDrawDocument* pBookmarkDoc,
                                               List* pBookmarkList, USHORT nBMSdPageCount ) :
        mpLayoutsToTransfer(pLayoutsToTransfer), mpBookmarkDoc(pBookmarkDoc),
        mpBookmarkList(pBookmarkList), mnBMSdPageCount(nBMSdPageCount) {}
    virtual ~InsertBookmarkAsPage_FindDuplicateLayouts() {};
    virtual void operator()( SdDrawDocument&, SdPage* );
private:
    List*           mpLayoutsToTransfer;
    SdDrawDocument* mpBookmarkDoc;
    List*           mpBookmarkList;
    USHORT          mnBMSdPageCount;
};

void InsertBookmarkAsPage_FindDuplicateLayouts::operator()( SdDrawDocument& rDoc, SdPage* pBMMPage )
{
    // now check for duplicate masterpage and layout names
    // ===================================================

    String  sFullLayoutName( pBMMPage->GetLayoutName() );
    String* pLayout = new String(sFullLayoutName);
    pLayout->Erase( pLayout->SearchAscii( SD_LT_SEPARATOR ));

    String* pTest = (String*) mpLayoutsToTransfer->First();
    BOOL bFound = FALSE;

    while (pTest && !bFound)    // found yet?
    {
        if (*pLayout == *pTest)
            bFound = TRUE;
        else
            pTest = (String*)mpLayoutsToTransfer->Next();
    }

    const USHORT nMPageCount = rDoc.GetMasterPageCount();
    for (USHORT nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
    {
        /**************************************************************
         * Gibt es die Layouts schon im Dokument?
         **************************************************************/
        SdPage* pTest = (SdPage*) rDoc.GetMasterPage(nMPage);
        String aTest(pTest->GetLayoutName());
        aTest.Erase( aTest.SearchAscii( SD_LT_SEPARATOR ));

        if (aTest == *pLayout)
            bFound = TRUE;
    }

    if (!bFound)
        mpLayoutsToTransfer->Insert(pLayout, LIST_APPEND);
    else
        delete pLayout;
}


BOOL SdDrawDocument::InsertBookmarkAsPage(
    List* pBookmarkList,
    List* pExchangeList,            // Liste der zu verwendenen Namen
    BOOL bLink,
    BOOL bReplace,
    USHORT nInsertPos,
    BOOL bNoDialogs,
    ::sd::DrawDocShell* pBookmarkDocSh,
    BOOL bCopy,
    BOOL bMergeMasterPages,
    BOOL bPreservePageNames)
{
    BOOL bOK = TRUE;
    BOOL bContinue = TRUE;
    BOOL bScaleObjects = FALSE;
    USHORT nReplacedStandardPages = 0;

    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName;

    if (pBookmarkDocSh)
    {
        pBookmarkDoc = pBookmarkDocSh->GetDoc();

        if (pBookmarkDocSh->GetMedium())
        {
            aBookmarkName = pBookmarkDocSh->GetMedium()->GetName();
        }
    }
    else if ( xBookmarkDocShRef.Is() )
    {
        pBookmarkDoc = xBookmarkDocShRef->GetDoc();
        aBookmarkName = aBookmarkFile;
    }
    else
    {
        return FALSE;
    }

    const USHORT nSdPageCount = GetSdPageCount(PK_STANDARD);
    const USHORT nBMSdPageCount = pBookmarkDoc->GetSdPageCount(PK_STANDARD);
    const USHORT nMPageCount = GetMasterPageCount();

    if (nSdPageCount==0 || nBMSdPageCount==0 || nMPageCount==0)
    {
        bContinue = bOK = FALSE;
        return(bContinue);
    }

    // Seitengroesse und -raender an die Werte der letzten
    // Seiten anpassen?
    SdPage* pBMPage = pBookmarkDoc->GetSdPage(0,PK_STANDARD);
    SdPage* pPage = GetSdPage(nSdPageCount - 1, PK_STANDARD);

    if( bNoDialogs )
    {
        if( !pBookmarkList )
            bScaleObjects = pPage->IsScaleObjects();
    }
    else
    {
        if (pBMPage->GetSize()        != pPage->GetSize()        ||
            pBMPage->GetLftBorder()   != pPage->GetLftBorder()   ||
            pBMPage->GetRgtBorder()   != pPage->GetRgtBorder()   ||
            pBMPage->GetUppBorder()   != pPage->GetUppBorder()   ||
            pBMPage->GetLwrBorder()   != pPage->GetLwrBorder())
        {
            String aStr(SdResId(STR_SCALE_OBJECTS));
            USHORT nBut = QueryBox( NULL, WB_YES_NO_CANCEL, aStr).Execute();

            bScaleObjects = nBut == RET_YES;
            bContinue     = nBut != RET_CANCEL;

            if (!bContinue)
            {
                return(bContinue);
            }
        }
    }


    /**************************************************************************
    |* Die benoetigten Praesentations-StyleSheets ermitteln und vor
    |* den Seiten transferieren, sonst verlieren die Textobjekte
    |* beim Transfer den Bezug zur Vorlage
    \*************************************************************************/
    SfxUndoManager* pUndoMgr = NULL;
    if( pDocSh )
    {
        pUndoMgr = pDocSh->GetUndoManager();
        pUndoMgr->EnterListAction(String(SdResId(STR_UNDO_INSERTPAGES)), String());
    }

    List* pLayoutsToTransfer = new List;

    //
    // #96029# Refactored copy'n'pasted layout name collection into IterateBookmarkPages
    //
    InsertBookmarkAsPage_FindDuplicateLayouts aSearchFunctor( pLayoutsToTransfer, pBookmarkDoc,
                                                              pBookmarkList, nBMSdPageCount );
    IterateBookmarkPages( pBookmarkDoc, pBookmarkList, nBMSdPageCount, aSearchFunctor );


    /**************************************************************************
    * Die tatsaechlich benoetigten Vorlagen kopieren
    **************************************************************************/
    SdStyleSheetPool* pBookmarkStyleSheetPool =
    (SdStyleSheetPool*) pBookmarkDoc->GetStyleSheetPool();
    String* pLayout = (String*) pLayoutsToTransfer->First();

    // Wenn Vorlagen kopiert werden muessen, dann muessen auch die
    // MasterPages kopiert werden!
    if( pLayout )
        bMergeMasterPages = TRUE;

    while (pLayout)
    {
        List* pCreatedStyles = new List;

        ((SdStyleSheetPool*) GetStyleSheetPool())->
        CopyLayoutSheets(*pLayout, *pBookmarkStyleSheetPool,pCreatedStyles);

        if (pCreatedStyles->Count() > 0)
        {
            if( pUndoMgr )
            {
                SdMoveStyleSheetsUndoAction* pMovStyles =
                    new SdMoveStyleSheetsUndoAction(this, pCreatedStyles, TRUE);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }
        else
        {
            delete pCreatedStyles;
        }

        delete pLayout;

        pLayout = (String*)pLayoutsToTransfer->Next();
    }

    delete pLayoutsToTransfer;

    /**************************************************************************
    * Dokument einfuegen
    **************************************************************************/
    BegUndo(String(SdResId(STR_UNDO_INSERTPAGES)));

    if (!pBookmarkList)
    {
        if (nInsertPos >= GetPageCount())
        {
            // Seiten werden hinten angefuegt
            nInsertPos = GetPageCount();
        }

        USHORT nActualInsertPos = nInsertPos;

        List aNameList;
        std::set<USHORT> aRenameSet;
        USHORT nBMSdPage;

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pBMPage = pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD);
            String  pName( pBMPage->GetName() );
            BOOL    bIsMasterPage;

            if (bLink)
            {
                // Es werden sich die Namen aller Seiten gemerkt
                aNameList.Insert(new String(pName), nBMSdPage);
            }

            // #95677# Have to check for duplicate names here, too
            // #67905# don't change name if source and dest model are the same!
            if( pBookmarkDoc != this &&
                GetPageByName(pName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // #95991# delay renaming *after* pages are copied (might destroy source otherwise)
                aRenameSet.insert(nBMSdPage);
            }
        }

        Merge(*pBookmarkDoc,
              1,                 // Nicht die Handzettelseite
              0xFFFF,            // Aber alle anderen
              nActualInsertPos,  // An Position einfuegen
              bMergeMasterPages, // MasterPages mitnehmen
              FALSE,             // Aber nur die benoetigten MasterPages
              TRUE,              // Undo-Aktion erzeugen
              bCopy);            // Seiten kopieren (oder mergen)

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pPage       = (SdPage*) GetPage(nActualInsertPos);
            SdPage* pNotesPage  = (SdPage*) GetPage(nActualInsertPos+1);
            String* pName       = (String*) aNameList.GetObject(nBMSdPage);

            // #95991# delay renaming *after* pages are copied (might destroy source otherwise)
            if( aRenameSet.find(nBMSdPage) != aRenameSet.end() )
            {
                // Seitenname schon vorhanden -> Defaultname
                // fuer Standard & Notizseite
                pPage->SetName(String());
                pNotesPage->SetName(String());
            }

            if (bLink)
            {
                // Nun werden die Link-Namen zusammengestellt
                pPage->SetFileName(aBookmarkName);
                pPage->SetBookmarkName(*(pName));
                delete pName;
                pPage->SetModel(this);
            }

            nActualInsertPos += 2;
        }
    }
    else
    {
        /**********************************************************************
        * Ausgewaehlte Seiten einfuegen
        **********************************************************************/
        SdPage* pBMPage;

        if (nInsertPos >= GetPageCount())
        {
            // Seiten werden hinten angefuegt
            bReplace = FALSE;
            nInsertPos = GetPageCount();
        }

        USHORT nActualInsertPos = nInsertPos;

        for (USHORT nPos = 0; nPos < pBookmarkList->Count(); nPos++)
        {
            /**************************************************************
            * Namen der Bookmark-Seiten aus Liste holen
            **************************************************************/
            String  aPgName(*(String*) pBookmarkList->GetObject(nPos));
            BOOL    bIsMasterPage;
            USHORT  nBMPage = pBookmarkDoc->GetPageByName( aPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                pBMPage = (SdPage*) pBookmarkDoc->GetPage(nBMPage);
            }
            else
            {
                pBMPage = NULL;
            }

            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                /**************************************************************
                * Es muss eine StandardSeite sein
                **************************************************************/
                sal_Bool bMustRename = sal_False;

                // #95991# delay renaming *after* pages are copied (might destroy source otherwise)
                // #67905# don't change name if source and dest model are the same!
                // #96029# avoid renaming if replacing the same page
                USHORT nPageSameName = GetPageByName(aPgName, bIsMasterPage);
                if( pBookmarkDoc != this &&
                    nPageSameName != SDRPAGE_NOTFOUND &&
                    ( !bReplace ||
                      nPageSameName != nActualInsertPos ) )
                {
                    bMustRename = sal_True;
                }

                Merge(*pBookmarkDoc,
                      nBMPage,           // Von Seite (Standard)
                      nBMPage+1,         // Bis Seite (Notizen)
                      nActualInsertPos,  // An Position einfuegen
                      bMergeMasterPages, // MasterPages mitnehmen
                      FALSE,             // Aber nur die benoetigten MasterPages
                      TRUE,              // Undo-Aktion erzeugen
                      bCopy);            // Seiten kopieren (oder mergen)

                if( bMustRename )
                {
                    // Seitenname schon vorhanden -> Defaultname
                    // fuer Standard & Notizseite
                    SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                    pPage->SetName(String());
                    SdPage* pNotesPage = (SdPage*) GetPage(nActualInsertPos+1);
                    pNotesPage->SetName(String());
                }

                if (bLink)
                {
                    SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                    pPage->SetFileName(aBookmarkName);
                    pPage->SetBookmarkName(aPgName);
                    pPage->SetModel(this);
                }

                if (bReplace)
                {
                    // Seite & Notizseite ausfuegen
                    const sal_uInt16 nDestPageNum(nActualInsertPos + 2);
                    SdPage* pStandardPage = 0L;

                    if(nDestPageNum < GetPageCount())
                    {
                        pStandardPage = (SdPage*)GetPage(nDestPageNum);
                    }

                    if (pStandardPage)
                    {
                        if( bPreservePageNames )
                        {
                            // #96029# Take old slide names for inserted pages
                            SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                            pPage->SetName( pStandardPage->GetName() );
                        }

                        AddUndo(new SdrUndoDelPage(*pStandardPage));
                        RemovePage(nDestPageNum);
                    }

                    SdPage* pNotesPage = 0L;

                    if(nDestPageNum < GetPageCount())
                    {
                        pNotesPage = (SdPage*)GetPage(nDestPageNum);
                    }

                    if (pNotesPage)
                    {
                        if( bPreservePageNames )
                        {
                            // #96029# Take old slide names for inserted pages
                            SdPage* pNotesPage = (SdPage*) GetPage(nActualInsertPos+1);
                            pNotesPage->SetName( pStandardPage->GetName() );
                        }

                        AddUndo(new SdrUndoDelPage(*pNotesPage));
                        RemovePage(nDestPageNum);
                    }

                    nReplacedStandardPages++;
                }

                nActualInsertPos += 2;
            }
        }
    }


    /**************************************************************************
    |* Dabei sind evtl. zu viele Masterpages ruebergekommen, da die
    |* DrawingEngine gleiche Praesentationslayouts nicht erkennen kann.
    |* Ueberzaehlige MasterPages entfernen.
    \*************************************************************************/
    USHORT nNewMPageCount = GetMasterPageCount();

    // rueckwaerts, damit Nummern nicht durcheinander geraten
    for (USHORT nPage = nNewMPageCount - 1; nPage >= nMPageCount; nPage--)
    {
        pPage = (SdPage*) GetMasterPage(nPage);
        String aMPLayout(pPage->GetLayoutName());
        PageKind eKind = pPage->GetPageKind();

        // gibt's den schon?
        for (USHORT nTest = 0; nTest < nMPageCount; nTest++)
        {
            SdPage* pTest = (SdPage*) GetMasterPage(nTest);
            String aTest(pTest->GetLayoutName());

            // #96029# nInsertPos > 2 is always true when inserting into non-empty models
            if ( nInsertPos > 2 &&
                 aTest == aMPLayout &&
                 eKind == pTest->GetPageKind() )
            {
                AddUndo(new SdrUndoDelPage(*pPage));
                RemoveMasterPage(nPage);
                nNewMPageCount--;
                break;
            }
        }
    }

    // #96029# nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 2)
    {
        /**********************************************************************
        |* Nur wenn Vorgaenger-Seiten vorhanden sind:
        |* An allen neuen Seiten nochmal das Praesentationslayout setzen;
        |* dadurch werden die Seiten von den Doppelgaenger-Masterpages
        |* geloest und den richtigen Masterpages zugeordnet. Ausserdem
        |* werden Gliederungstextobjekte wieder Listener der richtigen
        |* StyleSheets.
        |* Seitengroessen und -raender und ggfs. Objekte anpassen.
        |*
        |* Undo unnoetig, da die Seiten dabei sowieso rausfliegen.
        \*********************************************************************/
        pPage = (SdPage*) GetPage(nInsertPos - 2);
        SdPage* pNPage = (SdPage*) GetPage(nInsertPos - 1);

        Size  aSize(pPage->GetSize());
        INT32 nLeft  = pPage->GetLftBorder();
        INT32 nRight = pPage->GetRgtBorder();
        INT32 nUpper = pPage->GetUppBorder();
        INT32 nLower = pPage->GetLwrBorder();
        Orientation eOrient = pPage->GetOrientation();

        Size  aNSize(pNPage->GetSize());
        INT32 nNLeft  = pNPage->GetLftBorder();
        INT32 nNRight = pNPage->GetRgtBorder();
        INT32 nNUpper = pNPage->GetUppBorder();
        INT32 nNLower = pNPage->GetLwrBorder();
        Orientation eNOrient = pPage->GetOrientation();

        ULONG nExchangeListPos = 0;
        USHORT nSdPageStart = (nInsertPos - 1) / 2;
        USHORT nSdPageEnd = GetSdPageCount(PK_STANDARD) - nSdPageCount +
                            nSdPageStart - 1;

        if( bReplace )
        {
            nSdPageEnd = nSdPageStart + nReplacedStandardPages - 1;
        }

        for (USHORT nSdPage = nSdPageStart; nSdPage <= nSdPageEnd; nSdPage++)
        {
            pPage = GetSdPage(nSdPage, PK_STANDARD);

            if (pExchangeList)
            {
                // Zuverwendener Name aus Exchange-Liste holen
                if (pExchangeList->GetCurObject())
                {
                    String aExchangeName (*(String*) pExchangeList->GetCurObject());
                    pPage->SetName(aExchangeName);
                    SdrHint aHint(HINT_PAGEORDERCHG);
                    aHint.SetPage(pPage);
                    Broadcast(aHint);
                    SdPage* pNPage = GetSdPage(nSdPage, PK_NOTES);
                    pNPage->SetName(aExchangeName);
                    aHint.SetPage(pNPage);
                    Broadcast(aHint);
                }

                pExchangeList->Next();
            }

            String aLayout(pPage->GetLayoutName());
            aLayout.Erase(aLayout.SearchAscii( SD_LT_SEPARATOR ));

            // update layout and referred master page
            pPage->SetPresentationLayout(aLayout);

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(aSize, aBorderRect, TRUE);
            }
            pPage->SetSize(aSize);
            pPage->SetBorder(nLeft, nUpper, nRight, nLower);
            pPage->SetOrientation( eOrient );

            pPage = GetSdPage(nSdPage, PK_NOTES);

            // update layout and referred master page
            pPage->SetPresentationLayout(aLayout);

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nNLeft, nNUpper, nNRight, nNLower);
                pPage->ScaleObjects(aNSize, aBorderRect, TRUE);
            }

            pPage->SetSize(aNSize);
            pPage->SetBorder(nNLeft, nNUpper, nNRight, nNLower);
            pPage->SetOrientation( eNOrient );
        }

        for (USHORT nPage = nMPageCount; nPage < nNewMPageCount; nPage++)
        {
            pPage = (SdPage*) GetMasterPage(nPage);
            if (pPage->GetPageKind() == PK_STANDARD)
            {
                if (bScaleObjects)
                {
                    Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                    pPage->ScaleObjects(aSize, aBorderRect, TRUE);
                }
                pPage->SetSize(aSize);
                pPage->SetBorder(nLeft, nUpper, nRight, nLower);
                pPage->SetOrientation( eOrient );
            }
            else        // kann nur noch NOTES sein
            {
                if (bScaleObjects)
                {
                    Rectangle aBorderRect(nNLeft, nNUpper, nNRight, nNLower);
                    pPage->ScaleObjects(aNSize, aBorderRect, TRUE);
                }
                pPage->SetSize(aNSize);
                pPage->SetBorder(nNLeft, nNUpper, nNRight, nNLower);
                pPage->SetOrientation( eNOrient );
            }
        }
    }

    // #91146# Make absolutely sure no double masterpages are there
    RemoveUnnessesaryMasterPages(NULL, TRUE, TRUE);

    EndUndo();
    pUndoMgr->LeaveListAction();

    return bContinue;
}

/*************************************************************************
|*
|* Fuegt ein Bookmark als Objekt ein
|*
\************************************************************************/

BOOL SdDrawDocument::InsertBookmarkAsObject(
    List* pBookmarkList,
    List* pExchangeList,            // Liste der zu verwendenen Namen
    BOOL bLink,
    ::sd::DrawDocShell* pBookmarkDocSh,
    Point* pObjPos)
{
    BOOL bOK = TRUE;
    BOOL bOLEObjFound = FALSE;
    ::sd::View* pBMView = NULL;

    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName;

    if (pBookmarkDocSh)
    {
        pBookmarkDoc = pBookmarkDocSh->GetDoc();

        if (pBookmarkDocSh->GetMedium())
        {
            aBookmarkName = pBookmarkDocSh->GetMedium()->GetName();
        }
    }
    else if ( xBookmarkDocShRef.Is() )
    {
        pBookmarkDoc = xBookmarkDocShRef->GetDoc();
        aBookmarkName = aBookmarkFile;
    }
    else
    {
        return FALSE;
    }

    if (!pBookmarkList)
    {
        pBMView = new ::sd::View(pBookmarkDoc, (OutputDevice*) NULL);
        pBMView->EndListening(*pBookmarkDoc);
        pBMView->MarkAll();
    }
    else
    {
        SdrPage* pPage;
        SdrPageView* pPV;

        for (USHORT nPos = 0; nPos < pBookmarkList->Count(); nPos++)
        {
            /******************************************************************
            * Namen der Bookmarks aus Liste holen
            ******************************************************************/
            String aBMName (*(String*) pBookmarkList->GetObject(nPos));

            SdrObject* pObj = pBookmarkDoc->GetObj(aBMName);

            if (pObj)
            {
                // Objekt gefunden

                if (pObj->GetObjInventor() == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_OLE2)
                {
                    bOLEObjFound = TRUE;
                }

                if (!pBMView)
                {
                    // View erstmalig erzeugen
                    pBMView = new ::sd::View(pBookmarkDoc, (OutputDevice*) NULL);
                    pBMView->EndListening(*pBookmarkDoc);
                }

                pPage = pObj->GetPage();

                if (pPage->IsMasterPage())
                {
                    pPV = pBMView->ShowMasterPagePgNum(pPage->GetPageNum(), Point(0, 0));
                }
                else
                {
                    pPV = pBMView->GetPageView( pPage );
                    if( !pPV )
                        pPV = pBMView->ShowPage(pPage, Point(0, 0));
                }

                pBMView->MarkObj(pObj, pPV, FALSE);
            }
        }
    }

    if (pBMView)
    {
        /**********************************************************************
        * Selektierte Objekte einfuegen
        **********************************************************************/
        ::sd::View* pView = new ::sd::View(this, (OutputDevice*) NULL);
        pView->EndListening(*this);

        // Seite bestimmen, auf der die Objekte eingefuegt werden sollen
        SdrPage* pPage = GetSdPage(0, PK_STANDARD);

        if (pDocSh)
        {
            ::sd::ViewShell* pViewSh = pDocSh->GetViewShell();

            if (pViewSh)
            {
                // Welche Seite wird denn aktuell angezeigt?
                SdrPageView* pPV = pViewSh->GetView()->GetPageViewPvNum(0);

                if (pPV)
                {
                    pPage = pPV->GetPage();
                }
                else if (pViewSh->GetActualPage())
                {
                    pPage = pViewSh->GetActualPage();
                }
            }
        }

        Point aObjPos;

        if (pObjPos)
        {
            aObjPos = *pObjPos;
        }
        else
        {
            aObjPos = Rectangle(Point(), pPage->GetSize()).Center();
        }

        ULONG nCountBefore = 0;

        if (pExchangeList)
        {
            // OrdNums sortieren und Anzahl Objekte vor dem Einfuegen bestimmen
            pPage->RecalcObjOrdNums();
            nCountBefore = pPage->GetObjCount();
        }

        if (bOLEObjFound)
            pBMView->GetDoc()->SetAllocDocSh(TRUE);

        SdDrawDocument* pTmpDoc = (SdDrawDocument*) pBMView->GetAllMarkedModel();
        bOK = pView->Paste(*pTmpDoc, aObjPos, pPage);

        if (bOLEObjFound)
            pBMView->GetDoc()->SetAllocDocSh(FALSE);

        if (!bOLEObjFound)
            delete pTmpDoc;             // Wird ansonsten von der DocShell zerstoert

        delete pView;

        List* pList = pBookmarkList;

        if (pExchangeList)
        {
            // Anzahl Objekte nach dem Einfuegen bestimmen
            ULONG nCount = pPage->GetObjCount();

            for (ULONG nObj = nCountBefore; nObj < nCount; nObj++)
            {
                // Zuverwendener Name aus Exchange-Liste holen
                if (pExchangeList->GetCurObject())
                {
                    String aExchangeName (*(String*) pExchangeList->GetCurObject());

                    if (pPage->GetObj(nObj))
                    {
                        pPage->GetObj(nObj)->SetName(aExchangeName);
                    }
                }

                pExchangeList->Next();
            }

            pList = pExchangeList;
        }

        if( pList )
        {
            for (USHORT nPos = 0; nPos < pList->Count(); nPos++)
            {
                /******************************************************************
                * Namen der Bookmarks aus Liste holen
                ******************************************************************/
                String aBMName (*(String*) pList->GetObject(nPos));

                SdrObject* pObj = GetObj(aBMName);

                if (pObj)
                {
                    // Objekt gefunden
                    if (bLink && pObj->ISA(SdrObjGroup))
                    {
                        ( (SdrObjGroup*) pObj)->SetGroupLink(aBookmarkName, aBMName);
                    }
                }
            }
        }
    }

    delete pBMView;

    return bOK;
}

/*************************************************************************
|*
|* Beendet das Einfuegen von Bookmarks
|*
\************************************************************************/

void SdDrawDocument::CloseBookmarkDoc()
{
    if (xBookmarkDocShRef.Is())
    {
        xBookmarkDocShRef->DoClose();
    }

    xBookmarkDocShRef.Clear();
    aBookmarkFile = String();
}

/*************************************************************************
|*
|* Dokument laden (fuer gelinkte Objekte)
|*
\************************************************************************/

const SdrModel* SdDrawDocument::LoadModel(const String& rFileName)
{
    return ( OpenBookmarkDoc(rFileName) );
}

/*************************************************************************
|*
|* Dokument schliessen (fuer gelinkte Objekte)
|*
\************************************************************************/

void SdDrawDocument::DisposeLoadedModels()
{
    CloseBookmarkDoc();
}

/*************************************************************************
|*
|* Ist das Dokument read-only?
|*
\************************************************************************/

FASTBOOL SdDrawDocument::IsReadOnly() const
{
    BOOL bReadOnly = FALSE;

    if (pDocSh)
    {
        // bReadOnly = TRUE wuerde dazu fuehren, dass das Dokument nicht
        // bearbeitet werden kann. Dieser Effekt ist jedoch z.Z. nicht
        // gewuenscht, daher auskommentiert:
//        bReadOnly = pDocSh->IsReadOnly();
    }

    return (bReadOnly);
}


/*************************************************************************
|*
|* In anschliessendem AllocModel() wird eine DocShell erzeugt
|* (xAllocedDocShRef). Eine bereits bestehende DocShell wird ggf. geloescht
|*
\************************************************************************/

void SdDrawDocument::SetAllocDocSh(BOOL bAlloc)
{
    bAllocDocSh = bAlloc;

    if (xAllocedDocShRef.Is())
    {
        xAllocedDocShRef->DoClose();
    }

    xAllocedDocShRef.Clear();
}

/*************************************************************************
|*
|* Liste der CustomShows zurueckgeben (ggf. zuerst erzeugen)
|*
\************************************************************************/

List* SdDrawDocument::GetCustomShowList(BOOL bCreate)
{
    if (!pCustomShowList && bCreate)
    {
        // Liste erzeugen
        pCustomShowList = new List();
    }

    return(pCustomShowList);
}

/*************************************************************************
|*
|* Document-Stream herausgeben (fuer load-on-demand Graphiken)
|*
\************************************************************************/

SvStream* SdDrawDocument::GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const
{
    SotStorage* pStor = pDocSh ? pDocSh->GetMedium()->GetStorage() : NULL;
    SvStream*   pRet = NULL;

    if( pStor )
    {
        if( rStreamInfo.maUserData.Len() &&
            ( rStreamInfo.maUserData.GetToken( 0, ':' ) ==
              String( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package" ) ) ) )
        {
            const String aPicturePath( rStreamInfo.maUserData.GetToken( 1, ':' ) );

            // graphic from picture stream in picture storage in XML package
            if( aPicturePath.GetTokenCount( '/' ) == 2 )
            {
                const String aPictureStreamName( aPicturePath.GetToken( 1, '/' ) );

                if( !xPictureStorage.Is() )
                {
                    const String aPictureStorageName( aPicturePath.GetToken( 0, '/' ) );

                    if( pStor->IsContained( aPictureStorageName ) &&
                        pStor->IsStorage( aPictureStorageName )  )
                    {
                        // cast away const
                        ((SdDrawDocument*)this)->xPictureStorage = pStor->OpenUCBStorage( aPictureStorageName, STREAM_READ );
                    }
                }

                if( xPictureStorage.Is() &&
                    xPictureStorage->IsContained( aPictureStreamName ) &&
                    xPictureStorage->IsStream( aPictureStreamName ) )
                {
                    pRet = xPictureStorage->OpenSotStream( aPictureStreamName, STREAM_READ );

                    if( pRet )
                    {
                        pRet->SetVersion( xPictureStorage->GetVersion() );
                        pRet->SetKey( xPictureStorage->GetKey() );
                    }
                }
            }

            rStreamInfo.mbDeleteAfterUse = ( pRet != NULL );
        }
        else
        {
            // graphic from plain binary document stream
            if( !pDocStor )
            {
                if( pStor->IsStream( pStarDrawDoc ) )
                {
                    BOOL bOK = pStor->Rename(pStarDrawDoc, pStarDrawDoc3);
                    DBG_ASSERT(bOK, "Umbenennung des Streams gescheitert");
                }

                SotStorageStreamRef docStream = pStor->OpenSotStream( pStarDrawDoc3, STREAM_READ );
                docStream->SetVersion( pStor->GetVersion() );
                docStream->SetKey( pStor->GetKey() );

                // cast away const (should be regarded logical constness)
                ((SdDrawDocument*)this)->xDocStream = docStream;
                ((SdDrawDocument*)this)->pDocStor = pStor;
            }

            pRet = xDocStream;
            rStreamInfo.mbDeleteAfterUse = FALSE;
        }
    }

#if OSL_DEBUG_LEVEL > 1
    if( pRet )
    {
        // try to get some information from stream
        const ULONG nStartPos = pRet->Tell();
        const ULONG nEndPos = pRet->Seek( STREAM_SEEK_TO_END );
        const ULONG nStmLen = nEndPos - nStartPos;
        sal_uChar   aTestByte;

        // try to read one byte
        if( nStmLen )
            *pRet >> aTestByte;

        pRet->Seek( nStartPos );
    }
#endif

    return pRet;
}


/*************************************************************************
|*
|* Release doc stream, if no longer valid
|*
\************************************************************************/

void SdDrawDocument::HandsOff()
{
    xPictureStorage = SotStorageRef();
    pDocStor = NULL;
}


/*************************************************************************
|*
|* Nicht benutzte MasterPages und Layouts entfernen
|*
\************************************************************************/

void SdDrawDocument::RemoveUnnessesaryMasterPages(SdPage* pMasterPage, BOOL bOnlyDuplicatePages, BOOL bUndo)
{
    ::sd::View* pView = NULL;
    SfxUndoManager* pUndoMgr = NULL;

    if (pDocSh)
    {
        pUndoMgr = pDocSh->GetUndoManager();

        if (pDocSh->GetViewShell())
            pView = pDocSh->GetViewShell()->GetView();
    }

    /***********************************************************
    * Alle MasterPages pruefen
    ***********************************************************/
    USHORT nSdMasterPageCount = GetMasterSdPageCount( PK_STANDARD );
    for (sal_Int32 nMPage = nSdMasterPageCount - 1; nMPage >= 0; nMPage--)
    {
        SdPage* pMaster = pMasterPage;
        SdPage* pNotesMaster = NULL;

        if (!pMaster)
        {
            pMaster = (SdPage*) GetMasterSdPage( (USHORT) nMPage, PK_STANDARD );
            pNotesMaster = (SdPage*) GetMasterSdPage( (USHORT) nMPage, PK_NOTES );
        }
        else
        {
            for ( USHORT nMPg = 0; nMPg < GetMasterPageCount(); nMPg++ )
            {
                if ( pMaster == GetMasterPage( nMPg ) )
                {
                    pNotesMaster = (SdPage*) GetMasterPage( ++nMPg );
                    break;
                }
            }
        }

        DBG_ASSERT( pMaster->GetPageKind() == PK_STANDARD, "wrong page kind" );

        if ( pMaster->GetPageKind() == PK_STANDARD &&
             GetMasterPageUserCount( pMaster ) == 0 &&
             pNotesMaster )
        {
            BOOL bDeleteMaster = TRUE;
            String aLayoutName = pMaster->GetLayoutName();

            if( bOnlyDuplicatePages )
            {
                // remove only duplicate pages
                bDeleteMaster = FALSE;
                for (USHORT i = 0; i < GetMasterSdPageCount( PK_STANDARD ); i++)
                {
                    SdPage* pMPg = (SdPage*) GetMasterSdPage( i, PK_STANDARD );
                    if( pMPg != pMaster &&
                        pMPg->GetLayoutName() == aLayoutName )
                    {
                        // duplicate page found -> remove it
                        bDeleteMaster = TRUE;
                    }
                }
            }

            if( bDeleteMaster )
            {
                if (pView)
                {
                    // falls MasterPage sichtbar: erst PageView abmelden, dann loeschen
                    SdrPageView* pPgView = pView->GetPageView(pNotesMaster);
                    if (pPgView)
                        pView->HidePage(pPgView);

                    pPgView = pView->GetPageView(pMaster);
                    if (pPgView)
                        pView->HidePage(pPgView);
                }

                if( bUndo )
                {
                    BegUndo();
                    AddUndo( new SdrUndoDelPage( *pNotesMaster ) );
                }

                RemoveMasterPage( pNotesMaster->GetPageNum() );

                if( bUndo )
                    AddUndo(new SdrUndoDelPage(*pMaster));

                RemoveMasterPage( pMaster->GetPageNum() );

                if( bUndo )
                    EndUndo();  // schon hier, damit sich Joes Actions ZWISCHEN unsere eigenen schieben

                // alte Layoutvorlagen loeschen, wenn sie nicht mehr benoetigt werden
                BOOL bDeleteOldStyleSheets = TRUE;
                for ( USHORT nMPg = 0;
                          nMPg < GetMasterPageCount() && bDeleteOldStyleSheets;
                       nMPg++ )
                {
                    SdPage* pMPg = (SdPage*) GetMasterPage(nMPg);
                    if (pMPg->GetLayoutName() == aLayoutName)
                    {
                        bDeleteOldStyleSheets = FALSE;
                    }
                }

                if (bDeleteOldStyleSheets)
                {
                    List* pRemove = ((SdStyleSheetPool*) pStyleSheetPool)->CreateLayoutSheetList( aLayoutName );

                    if( bUndo )
                    {
                        // die Liste gehoert der UndoAction
                        SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, pRemove, FALSE );

                        if (pUndoMgr)
                            pUndoMgr->AddUndoAction(pMovStyles);
                    }

                    for ( SfxStyleSheet* pSheet = (SfxStyleSheet*)pRemove->First();
                           pSheet;
                           pSheet = (SfxStyleSheet*)pRemove->Next() )
                    {
                        ((SdStyleSheetPool*) pStyleSheetPool)->Remove(pSheet);
                    }
                }
            }
        }

        if (pMasterPage)
            break;                      // Nur diese eine MasterPage!
    }
}


/*************************************************************************
|*
|* MasterPage austauschen
|*
|* Entweder erhaelt nSdPageNum eine neue, eigene MasterPage, oder die MasterPage
|* wird komplett ausgetauscht (gilt dann fuer alle Seiten).
|*
|* nSdPageNum   : Nummer der Seite, welche die neue MasterPage erhalten soll
|* rLayoutName  : LayoutName der neuen MasterPage
|* pSourceDoc   : Dokument (Vorlage) aus dem die MasterPage geholt wird
|* bMaster      : Die MasterPage von nSdPageNum soll ausgetauscht werden
|* bCheckMasters: Nicht benutzte MasterPages sollen entfernt werden
|*
|* Ist pSourceDoc == NULL, so wird eine leere MasterPage zugewiesen.
|* Ist rLayoutName leer, so wird die erste MasterPage genommen
\************************************************************************/

void SdDrawDocument::SetMasterPage(USHORT nSdPageNum,
                                   const String& rLayoutName,
                                   SdDrawDocument* pSourceDoc,
                                   BOOL bMaster,
                                   BOOL bCheckMasters)
{
    if( pDocSh )
        pDocSh->SetWaitCursor( TRUE );

    SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    pUndoMgr->EnterListAction(String(SdResId(STR_UNDO_SET_PRESLAYOUT)), String());

    SdPage* pSelectedPage   = GetSdPage(nSdPageNum, PK_STANDARD);
    SdPage* pNotes          = (SdPage*) GetPage(pSelectedPage->GetPageNum()+1);
    SdPage& rOldMaster      = (SdPage&)pSelectedPage->TRG_GetMasterPage();
    SdPage& rOldNotesMaster = (SdPage&)pNotes->TRG_GetMasterPage();
    SdPage* pMaster         = NULL;
    SdPage* pNotesMaster    = NULL;
    SdPage* pPage           = NULL;
    String aOldPageLayoutName(pSelectedPage->GetLayoutName());
    String aOldLayoutName(aOldPageLayoutName);
    aOldLayoutName.Erase(aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ));

    String aNewLayoutName( rLayoutName );

    if (pSourceDoc)
    {
        List* pReplList = NULL;
        BOOL bLayoutReloaded = FALSE;   // Wurde ex. Layout wieder geladen?

        /*********************************************************************
        |* LayoutName, Page and Notespage
        \*********************************************************************/
        if (rLayoutName.Len() == 0)
        {
            // No LayoutName: take first MasterPage
            pMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_STANDARD);
            pNotesMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_NOTES);
            aNewLayoutName = pMaster->GetName();
        }
        else
        {
            String aSearchFor(rLayoutName);
            aSearchFor.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
            aSearchFor.Append( String(SdResId(STR_LAYOUT_OUTLINE))) ;

            for (USHORT nMP = 0; nMP < pSourceDoc->GetMasterPageCount(); nMP++)
            {
                SdPage* pMP = (SdPage*) pSourceDoc->GetMasterPage(nMP);

                if (pMP->GetLayoutName() == aSearchFor)
                {
                    if (pMP->GetPageKind() == PK_STANDARD)
                        pMaster = pMP;
                    if (pMP->GetPageKind() == PK_NOTES)
                        pNotesMaster = pMP;
                }
                if (pMaster && pNotesMaster)
                    break;
            }
            DBG_ASSERT(pMaster, "MasterPage (Standard page) not found");
            DBG_ASSERT(pNotesMaster, "MasterPage (Notes page) not found");

            // this should not happen, but looking at crashreports, it does
            if( (pMaster == NULL) || (pNotesMaster == NULL) )
            {
                // so take the first MasterPage
                pMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_STANDARD);
                pNotesMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_NOTES);
                aNewLayoutName = pMaster->GetName();
            }
        }

        // we should never reach this, but one never knows....
        if( (pMaster == NULL) || (pNotesMaster == NULL) )
        {
            pUndoMgr->LeaveListAction();

            if( pDocSh )
                pDocSh->SetWaitCursor( FALSE );

            DBG_ERROR( "SdDrawDocument::SetMasterPage() failed!" );

            return;
        }

        if (pSourceDoc != this)
        {
            const USHORT nMasterPageCount = GetMasterPageCount();
            for ( USHORT nMPage = 0; nMPage < nMasterPageCount; nMPage++ )
            {
                SdPage* pMaster = (SdPage*)GetMasterPage(nMPage);
                if( pMaster->GetName() == aNewLayoutName )
                {
                    bLayoutReloaded = TRUE;
                    break;
                }
            }

            /*****************************************************************
            |* Praesentationsvorlagen korrigieren bzw. neu anlegen
            \****************************************************************/
            // nur die Praesentationsvorlagen beachten
            String aName;
            SdStyleSheetPool* pSourceStyleSheetPool = (SdStyleSheetPool*) pSourceDoc->GetStyleSheetPool();
            pSourceStyleSheetPool->SetSearchMask(SD_LT_FAMILY);
            ((SdStyleSheetPool*) pStyleSheetPool)->SetSearchMask(SD_LT_FAMILY);

            pReplList = new List;           // Liste fuer ersetzte StyleSheets
            List* pCreatedStyles = new List;// Liste fuer erzeugte StyleSheets

            SfxStyleSheetBase* pHisSheet = pSourceStyleSheetPool->First();

            while (pHisSheet)
            {
                aName = pHisSheet->GetName();

                if( aName.Search( aNewLayoutName ) == 0 )
                {
                    SfxStyleSheet* pMySheet = (SfxStyleSheet*) pStyleSheetPool->Find(
                                                aName, SD_LT_FAMILY);

                    if (pMySheet)
                    {
                        // Es ist eine gleichnamige Vorlage vorhanden ist: Inhalte ersetzen
                        BOOL bTest = pMySheet->SetName(pHisSheet->GetName());
                        DBG_ASSERT(bTest, "StyleSheet-Umbenennung fehlgeschlagen");
                        pMySheet->GetItemSet().ClearItem(0);  // alle loeschen

                        StyleSheetUndoAction* pUndoChStyle = new StyleSheetUndoAction(this,
                                                                 pMySheet, &pHisSheet->GetItemSet());
                        pUndoMgr->AddUndoAction(pUndoChStyle);
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());
                        pMySheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    }
                    else
                    {
                       // So eine Vorlage erzeugen
                        pMySheet = (SfxStyleSheet*)&pStyleSheetPool->Make(
                                            aName, SD_LT_FAMILY, pHisSheet->GetMask());
                        pMySheet->GetItemSet().ClearItem(0);  // alle loeschen
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());
                        pCreatedStyles->Insert(pMySheet, LIST_APPEND);
                    }

                    StyleReplaceData* pReplData = new StyleReplaceData;
                    pReplData->nNewFamily = pMySheet->GetFamily();
                    pReplData->nFamily    = pMySheet->GetFamily();
                    pReplData->aNewName   = pMySheet->GetName();

                    String aTemp(pMySheet->GetName());
                    USHORT nPos = aTemp.SearchAscii( SD_LT_SEPARATOR );
                    aTemp.Erase(0, nPos);
                    aTemp.Insert(aOldLayoutName, 0);
                    pReplData->aName = aTemp;
                    pReplList->Insert(pReplData, LIST_APPEND);
                }

                pHisSheet = (SfxStyleSheet*) pSourceStyleSheetPool->Next();
            }

            // wenn neue Vorlagen erzeugt wurden:
            // eventuell bestehende Parent-Verkettung der Itemsets in den
            // Vorlagen wieder aufbauen
            if (pCreatedStyles->Count())
            {
                StyleReplaceData* pRData = (StyleReplaceData*)pReplList->First();

                while (pRData)
                {
                    SfxStyleSheetBase* pSOld = pStyleSheetPool->Find(pRData->aName);
                    SfxStyleSheetBase* pSNew = pStyleSheetPool->Find(pRData->aNewName);

                    if (pSOld && pSNew)
                    {
                        const String& rParentOfOld = pSOld->GetParent();
                        const String& rParentOfNew = pSNew->GetParent();

                        if (rParentOfOld.Len() > 0 && rParentOfNew.Len() == 0)
                        {

                            for (ULONG i = 0; i < pReplList->Count(); i++)
                            {
                                StyleReplaceData* pRD = (StyleReplaceData*)pReplList->
                                                                        GetObject(i);
                                if ((pRD->aName == rParentOfOld) && (pRD->aName != pRD->aNewName))
                                {
                                    String aParentOfNew(pRD->aNewName);
                                    pSNew->SetParent(aParentOfNew);
                                    break;
                                }
                            }
                        }
                    }
                    pRData = (StyleReplaceData*) pReplList->Next();
                }

                // ab jetzt beim Suchen alle beachten
                pSourceStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_ALL);
                pStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_ALL);
            }

            if (pCreatedStyles->Count() > 0)
            {
                // UndoAction fuer das Erzeugen und Einfuegen vorn StyleSheets
                // auf den UndoManager legen
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(
                                                              this, pCreatedStyles, TRUE);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
            else
            {
                // Liste zerstoeren
                delete pCreatedStyles;
            }
        }

        // Layoutnamen auf Basis des Seitenlayoutnamens der Masterpage bilden
        String aPageLayoutName(pMaster->GetLayoutName());
        String aLayoutName = aPageLayoutName;
        aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ));

        if (pSourceDoc != this)
        {
            // Aus dem Source-Dokument austragen
            SdrPage* pTest = NULL;
            pTest = pSourceDoc->RemoveMasterPage(pNotesMaster->GetPageNum());
            pTest = pSourceDoc->RemoveMasterPage(pMaster->GetPageNum());
        }

        /*********************************************************************
        |* Neue MasterPages ins Dokument eintragen und den Standard- und
        |* Notizseiten das Praesentationslayout ueberbraten
        \********************************************************************/
        if (pSourceDoc != this)
        {
            // Die Masterpages einfuegen:
            // Masterpages von neuen Layouts hinten anhaengen; wird ein Layout
            // dagegen ersetzt, so muss vor der Position der alten Masterpage
            // eingefuegt werden, damit ab jetzt beim Suchen (z. B. SdPage::
            // SetPresentationLayout) die neue Masterpage zuerst gefunden wird
            USHORT nInsertPos = rOldMaster.GetPageNum();
            BegUndo();

            if (!bLayoutReloaded)
                nInsertPos = 0xFFFF;
            InsertMasterPage(pMaster, nInsertPos);
            AddUndo(new SdrUndoNewPage(*pMaster));

            nInsertPos++;
            if (!bLayoutReloaded)
                nInsertPos = 0xFFFF;
            InsertMasterPage(pNotesMaster, nInsertPos);
            AddUndo(new SdrUndoNewPage(*pNotesMaster));

            EndUndo(); // schon hier, damit sich Joes Actions ZWISCHEN unsere eigenen schieben
        }

        // Liste mit Seiten fuellen
        List* pPageList = new List;

//      #98456, this has to be removed according to CL (KA 07/08/2002)
//      #109884# but we need them again to restore the styles of the presentation objects while undo
        pPageList->Insert(pMaster, LIST_APPEND);
        pPageList->Insert(pNotesMaster, LIST_APPEND);

        if (bMaster || bLayoutReloaded)
        {
            for (USHORT nPage = 1; nPage < GetPageCount(); nPage++)
            {
                pPage = (SdPage*) GetPage(nPage);
                String aTest = pPage->GetLayoutName();
                if (aTest == aOldPageLayoutName)
                {
                    pPageList->Insert(pPage, LIST_APPEND);
                }
            }

        }
        else
        {
            pPageList->Insert(pSelectedPage, LIST_APPEND);
            pPageList->Insert(pNotes, LIST_APPEND);
        }

        pPage = (SdPage*)pPageList->First();
        while (pPage)
        {
            AutoLayout eAutoLayout = pPage->GetAutoLayout();

            SdPresentationLayoutUndoAction * pPLUndoAction =
                new SdPresentationLayoutUndoAction
                    (this, aOldLayoutName, aLayoutName,
                     eAutoLayout, eAutoLayout, FALSE, pPage);
            pUndoMgr->AddUndoAction(pPLUndoAction);
            pPage->SetPresentationLayout(aLayoutName);
            pPage->SetAutoLayout(eAutoLayout);

            pPage = (SdPage*)pPageList->Next();
        }
        delete pPageList;

        /*********************************************************************
        |* Neue Masterpages angleichen
        \********************************************************************/
        if (pSourceDoc != this)
        {
            // die Masterpages angleichen
            Size aSize(rOldMaster.GetSize());
            Rectangle aBorderRect(rOldMaster.GetLftBorder(),
                                  rOldMaster.GetUppBorder(),
                                  rOldMaster.GetRgtBorder(),
                                  rOldMaster.GetLwrBorder());
            pMaster->ScaleObjects(aSize, aBorderRect, TRUE);
            pMaster->SetSize(aSize);
            pMaster->SetBorder(rOldMaster.GetLftBorder(),
                               rOldMaster.GetUppBorder(),
                               rOldMaster.GetRgtBorder(),
                               rOldMaster.GetLwrBorder());
            pMaster->SetOrientation( rOldMaster.GetOrientation() );
            pMaster->SetAutoLayout(pMaster->GetAutoLayout());

            aSize = rOldNotesMaster.GetSize();
            Rectangle aNotesBorderRect(rOldNotesMaster.GetLftBorder(),
                                       rOldNotesMaster.GetUppBorder(),
                                       rOldNotesMaster.GetRgtBorder(),
                                       rOldNotesMaster.GetLwrBorder());
            pNotesMaster->ScaleObjects(aSize, aNotesBorderRect, TRUE);
            pNotesMaster->SetSize(aSize);
            pNotesMaster->SetBorder(rOldNotesMaster.GetLftBorder(),
                                    rOldNotesMaster.GetUppBorder(),
                                    rOldNotesMaster.GetRgtBorder(),
                                    rOldNotesMaster.GetLwrBorder());
            pNotesMaster->SetOrientation( rOldNotesMaster.GetOrientation() );
            pNotesMaster->SetAutoLayout(pNotesMaster->GetAutoLayout());

            // Liste der ersetzten Vorlagen mit Inhalt loeschen
            StyleReplaceData* pReplData = (StyleReplaceData*)pReplList->First();
            while (pReplData)
            {
                delete pReplData;
                pReplData = (StyleReplaceData*)pReplList->Next();
            }
            delete pReplList;
       }
    }
    else
    {
        /*********************************************************************
        |* Einen neuen Layoutnamen ausdenken
        \********************************************************************/
        String aName        = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
        String aTest;
        BOOL   bNotANewName = TRUE;
        USHORT nCount       = 0;
        USHORT nMPgCount    = GetMasterPageCount();

        for (nCount = 0; bNotANewName; nCount++)
        {
            // Testnamen bilden
            aTest = aName;              // Standard, Standard1, Standard2, ...
            if (nCount > 0)
                aTest += String::CreateFromInt32( nCount );

            // gibt's schon eine, die so heisst?
            bNotANewName = FALSE;
            for (USHORT nMPg = 1; nMPg < nMPgCount; nMPg++)
            {
                const SdrPage* pTest = GetMasterPage(nMPg);
                String aPageLayoutName(pTest->GetLayoutName());
                aPageLayoutName.Erase( aPageLayoutName.SearchAscii( SD_LT_SEPARATOR ));

                if (aPageLayoutName == aTest)
                    bNotANewName = TRUE;
            }
        }
        aName = aTest;
        String aPageLayoutName(aName);
        aPageLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
        aPageLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

        /*********************************************************************
        |* Neue StyleSheets erzeugen
        \********************************************************************/
        ((SdStyleSheetPool*) pStyleSheetPool)->CreateLayoutStyleSheets(aName);
        List* pCreatedStyles = ((SdStyleSheetPool*) pStyleSheetPool)->CreateLayoutSheetList(aName);
        SdMoveStyleSheetsUndoAction* pMovStyles =
            new SdMoveStyleSheetsUndoAction(this, pCreatedStyles, TRUE);
        pUndoMgr->AddUndoAction(pMovStyles);

        /*********************************************************************
        |* Neue MasterPages erzeugen und ins Dokument eintragen
        \********************************************************************/
        BegUndo();
        pMaster = (SdPage*) AllocPage(TRUE);
        pMaster->SetSize(pSelectedPage->GetSize());
        pMaster->SetBorder(pSelectedPage->GetLftBorder(),
                           pSelectedPage->GetUppBorder(),
                           pSelectedPage->GetRgtBorder(),
                           pSelectedPage->GetLwrBorder() );
        pMaster->SetName(aName);
        pMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pMaster);
        AddUndo(new SdrUndoNewPage(*pMaster));
        pMaster->SetAutoLayout(AUTOLAYOUT_NONE, true, true);

        pNotesMaster = (SdPage*) AllocPage(TRUE);
        pNotesMaster->SetPageKind(PK_NOTES);
        pNotesMaster->SetSize(pNotes->GetSize());
        pNotesMaster->SetBorder(pNotes->GetLftBorder(),
                                pNotes->GetUppBorder(),
                                pNotes->GetRgtBorder(),
                                pNotes->GetLwrBorder() );
        pNotesMaster->SetName(aName);
        pNotesMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pNotesMaster);
        AddUndo(new SdrUndoNewPage(*pNotesMaster));
        pNotesMaster->SetAutoLayout(AUTOLAYOUT_NOTES, true, true);
        EndUndo();

        /*********************************************************************
        |* Liste der betroffenen Standard- und Notizseiten erstellen
        \********************************************************************/
        List* pPageList = new List;
        if (bMaster)
        {
            for (USHORT nPage = 1; nPage < GetPageCount(); nPage++)
            {
                pPage = (SdPage*) GetPage(nPage);
                String aTest = pPage->GetLayoutName();
                if (aTest == aOldPageLayoutName)
                {
                    pPageList->Insert(pPage, LIST_APPEND);
                }
            }
        }
        else
        {
            pPageList->Insert(pSelectedPage, LIST_APPEND);
            pPageList->Insert(pNotes, LIST_APPEND);
        }

        /*********************************************************************
        |* An den betroffenen Seiten Praesentations- und Autolayout setzen
        \********************************************************************/
        pPage = (SdPage*)pPageList->First();
        while(pPage)
        {
            AutoLayout eOldAutoLayout = pPage->GetAutoLayout();
            AutoLayout eNewAutoLayout =
                pPage->GetPageKind() == PK_STANDARD ? AUTOLAYOUT_NONE : AUTOLAYOUT_NOTES;

            SdPresentationLayoutUndoAction * pPLUndoAction =
                new SdPresentationLayoutUndoAction
                        (this, aOldLayoutName, aName,
                         eOldAutoLayout, eNewAutoLayout, TRUE,
                         pPage);
            pUndoMgr->AddUndoAction(pPLUndoAction);

            pPage->SetPresentationLayout(aName);
            pPage->SetAutoLayout(eNewAutoLayout);

            pPage = (SdPage*)pPageList->Next();
        }

        // Seitenliste loeschen
        delete pPageList;
    }

    /*********************************************************************
    |* falls die alten Masterpages nicht mehr benoetigt werden,
    |* muessen sie und die entsprechenden Praesentationsvorlagen
    |* entfernt werden
    \********************************************************************/
    if (bCheckMasters)
    {
        // Alle pruefen
        RemoveUnnessesaryMasterPages();
    }
    else
    {
        // Nur die ausgetauschte MasterPage pruefen
        RemoveUnnessesaryMasterPages(&rOldMaster);
    }

    pUndoMgr->LeaveListAction();

    if( pDocSh )
        pDocSh->SetWaitCursor( FALSE );
}



void SdDrawDocument::Merge(SdrModel& rSourceModel,
               USHORT nFirstPageNum, USHORT nLastPageNum,
               USHORT nDestPos,
               FASTBOOL bMergeMasterPages, FASTBOOL bAllMasterPages,
               FASTBOOL bUndo, FASTBOOL bTreadSourceAsConst)
{
    SdrModel::Merge( rSourceModel, nFirstPageNum, nLastPageNum, nDestPos, bMergeMasterPages, bAllMasterPages, bUndo, bTreadSourceAsConst );

    if( &rSourceModel == this )
        return;

    // #55912# fix animation at path
    if( nLastPageNum >= rSourceModel.GetPageCount() )
        nLastPageNum = rSourceModel.GetPageCount()-1;

    USHORT nSrcPage;
    USHORT nDstPage;
    for( nSrcPage = nFirstPageNum, nDstPage = nDestPos; (nSrcPage < nLastPageNum) && (nDstPage < GetPageCount()); nSrcPage++, nDstPage++ )
    {
        const SdrPage* pSrcPage = rSourceModel.GetPage( nSrcPage );
        const SdrPage* pDstPage = GetPage( nDstPage );

        if( pSrcPage && pDstPage )
        {
            SdrObjListIter  aSrcIter( *pSrcPage, IM_DEEPWITHGROUPS );
            SdrObjListIter  aDstIter( *pDstPage, IM_DEEPWITHGROUPS );

            SdrObject* pSrcObj;
            SdrObject* pDstObj;
            for( pSrcObj = aSrcIter.Next(), pDstObj = aDstIter.Next();
                 pSrcObj && pDstObj;
                 pSrcObj = aSrcIter.Next(), pDstObj = aDstIter.Next() )
            {
                SdAnimationInfo* pInfo = static_cast< SdDrawDocument* >(&rSourceModel)->GetAnimationInfo(const_cast<SdrObject*>(pSrcObj));
                if( pInfo && pInfo->eEffect == presentation::AnimationEffect_PATH && pInfo->pPathObj)
                {
                    SdrObjListIter  aSrcPathIter( *pSrcPage, IM_DEEPWITHGROUPS );
                    SdrObjListIter  aDstPathIter( *pDstPage, IM_DEEPWITHGROUPS );
                    SdrObject* pSrcPathObj;
                    SdrObject* pDstPathObj;
                    for( pSrcPathObj = aSrcPathIter.Next(), pDstPathObj = aDstPathIter.Next();
                         pSrcPathObj && pDstPathObj;
                         pSrcPathObj = aSrcPathIter.Next(), pDstPathObj = aDstPathIter.Next() )
                    {
                        if( pSrcPathObj == pInfo->pPathObj )
                        {
                            if( PTR_CAST( SdrPathObj, pDstPathObj ) )
                            {
                                SdAnimationInfo* pInfo = GetAnimationInfo(pDstObj);
                                if( pInfo == NULL )
                                {
                                    pInfo = new SdAnimationInfo(this);
                                    pDstObj->InsertUserData( pInfo );
                                }

                                pInfo->eEffect = presentation::AnimationEffect_PATH;
                                pInfo->pPathObj = PTR_CAST(SdrPathObj, pDstPathObj );
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}
