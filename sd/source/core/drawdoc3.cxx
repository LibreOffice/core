/*************************************************************************
 *
 *  $RCSfile: drawdoc3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
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

#ifndef SVX_LIGHT
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
#endif // !SVX_LIGHT

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
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif


#include "glob.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "sdresid.hxx"
#include "sdiocmpt.hxx"
#include "strmname.h"

#ifndef SVX_LIGHT
#ifdef MAC
#include "::ui:inc:unmovss.hxx"
#include "::ui:inc:unchss.hxx"
#include "::ui:inc:unprlout.hxx"
#include "::ui:inc:docshell.hxx"
#include "::ui:inc:grdocsh.hxx"
#include "::ui:inc:viewshel.hxx"
#include "::ui:inc:sdview.hxx"
#include "::ui:inc:cfgids.hxx"
#include "::ui:inc:strings.hrc"
#else
#ifdef UNX
#include "../ui/inc/unmovss.hxx"
#include "../ui/inc/unchss.hxx"
#include "../ui/inc/unprlout.hxx"
#include "../ui/inc/docshell.hxx"
#include "../ui/inc/grdocsh.hxx"
#include "../ui/inc/viewshel.hxx"
#include "../ui/inc/sdview.hxx"
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"
#else
#include "..\ui\inc\unmovss.hxx"
#include "..\ui\inc\unchss.hxx"
#include "..\ui\inc\unprlout.hxx"
#include "..\ui\inc\docshell.hxx"
#include "..\ui\inc\grdocsh.hxx"
#include "..\ui\inc\viewshel.hxx"
#include "..\ui\inc\sdview.hxx"
#include "..\ui\inc\cfgids.hxx"
#include "..\ui\inc\strings.hrc"
#endif
#endif
#endif // !SVX_LIGHT



#define POOL_BUFFER_SIZE        (USHORT)32768
#define BASIC_BUFFER_SIZE       (USHORT)8192
#define DOCUMENT_BUFFER_SIZE    (USHORT)32768

/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/

#ifndef SVX_LIGHT
SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(SfxMedium& rMedium)
{
    BOOL bOK = TRUE;
    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName = rMedium.GetName();

    if (aBookmarkFile != aBookmarkName && aBookmarkName.Len() && rMedium.IsStorage())
    {
        DBG_ASSERT( rMedium.IsStorage(), "Kein Storage, keine Banane!" );

        SvStorage* pStorage = rMedium.GetStorage();

        if (!pStorage->IsStream(pStarDrawDoc) &&
            !pStorage->IsStream(pStarDrawDoc3))
        {
            // Es ist nicht unser Storage
            bOK = FALSE;
            DBG_ASSERT(bOK, "Nicht unser Storage");
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
                xBookmarkDocShRef = new SdGraphicDocShell(SFX_CREATE_MODE_STANDARD, TRUE);
            else
                // Impress
                xBookmarkDocShRef = new SdDrawDocShell(SFX_CREATE_MODE_STANDARD, TRUE);

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
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/

#ifndef SVX_LIGHT
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
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* Fuegt ein Bookmark (Seite oder Objekt) ein
|*
\************************************************************************/

#ifndef SVX_LIGHT
BOOL SdDrawDocument::InsertBookmark(
    List* pBookmarkList,            // Liste der Namen der einzufuegenden Bookmarks
    List* pExchangeList,            // Liste der zu verwendenen Namen
    BOOL bLink,                     // Bookmarks sollen als Verknuepfung eingefuegt werden
    BOOL bReplace,                  // Aktuellen Seiten (Standard&Notiz) werden ersetzt
    USHORT nInsertPos,              // Einfuegeposition fuer Seiten
    BOOL bNoDialogs,                // Keine Dialoge anzeigen
    SdDrawDocShell* pBookmarkDocSh, // Wenn gesetzt, so ist dieses das Source-Dokument
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
            String aBMPgName (*(String*) pBookmarkList->GetObject(nPos));

            if (pBookmarkDoc->GetPageByName(aBMPgName) != SDRPAGE_NOTFOUND)
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
                                   nInsertPos, bNoDialogs, pBookmarkDocSh, bCopy, TRUE);
    }

    if ( bOK && pBookmarkList )
    {
        // Es werden alle Objekt-Bookmarks eingefuegt
        bOK = InsertBookmarkAsObject(pBookmarkList, pExchangeList, bLink,
                                     pBookmarkDocSh, pObjPos);
    }

    return bOK;
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Fuegt ein Bookmark als Seite ein
|*
\************************************************************************/

#ifndef SVX_LIGHT
BOOL SdDrawDocument::InsertBookmarkAsPage(
    List* pBookmarkList,
    List* pExchangeList,            // Liste der zu verwendenen Namen
    BOOL bLink,
    BOOL bReplace,
    USHORT nInsertPos,
    BOOL bNoDialogs,
    SdDrawDocShell* pBookmarkDocSh,
    BOOL bCopy,
    BOOL bMergeMasterPages)
{
    BOOL bOK = TRUE;
    BOOL bContinue = TRUE;
    BOOL bScaleObjects = TRUE;
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

    USHORT nSdPageCount = GetSdPageCount(PK_STANDARD);
    USHORT nBMSdPageCount = pBookmarkDoc->GetSdPageCount(PK_STANDARD);
    USHORT nMPageCount = GetMasterPageCount();

    if (nSdPageCount==0 || nBMSdPageCount==0 || nMPageCount==0)
    {
        bContinue = bOK = FALSE;
        return(bContinue);
    }

    // Seitengroesse und -raender an die Werte der letzten
    // Seiten anpassen?
    SdPage* pBMPage = pBookmarkDoc->GetSdPage(0,PK_STANDARD);
    SdPage* pPage = GetSdPage(nSdPageCount - 1, PK_STANDARD);

    if (bNoDialogs)
    {
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
    SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    pUndoMgr->EnterListAction(String(SdResId(STR_UNDO_INSERTPAGES)), String());

    List* pLayoutsToTransfer = new List;
    USHORT nBMSdPage;

    if (!pBookmarkList)
    {
        /**********************************************************************
        * Alle Seiten sollen einfuegt werden: Layouts pruefen
        **********************************************************************/

        for (nBMSdPage = 0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            /**************************************************************
            * Welche Layouts gibt im Bookmark-Dokument?
            **************************************************************/
            SdPage* pBMMPage = (SdPage*) pBookmarkDoc->
                             GetSdPage(nBMSdPage, PK_STANDARD)->GetMasterPage(0);
            String* pLayout = new String(pBMMPage->GetLayoutName());
            pLayout->Erase( pLayout->SearchAscii( SD_LT_SEPARATOR ));

            String* pTest = (String*) pLayoutsToTransfer->First();
            BOOL bFound = FALSE;

            while (pTest && !bFound)    // schon gefunden?
            {
                if (*pLayout == *pTest)
                    bFound = TRUE;
                else
                    pTest = (String*)pLayoutsToTransfer->Next();
            }

            for (USHORT nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
            {
                /**************************************************************
                * Gibt es die Layouts schon im Dokument?
                **************************************************************/
                SdPage* pTest = (SdPage*) GetMasterPage(nMPage);
                String aTest(pTest->GetLayoutName());
                aTest.Erase( aTest.SearchAscii( SD_LT_SEPARATOR ));

                if (aTest == *pLayout)
                    bFound = TRUE;
            }

            if (!bFound)
                pLayoutsToTransfer->Insert(pLayout, LIST_APPEND);
            else
                delete pLayout;
        }
    }
    else
    {
        /**********************************************************************
        * Ausgewaehlte Seiten sollen einfuegt werden: Layouts pruefen
        **********************************************************************/

        SdPage* pBMPage;

        for (USHORT nPos = 0; nPos < pBookmarkList->Count(); nPos++)
        {
            /******************************************************************
            * Namen der Bookmark-Seiten aus Liste holen
            ******************************************************************/
            String aBMPgName (*(String*) pBookmarkList->GetObject(nPos));

            USHORT nBMPage = pBookmarkDoc->GetPageByName(aBMPgName);

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
                USHORT nBMSdPage = (nBMPage - 1) / 2;

                SdPage* pMPage = (SdPage*) pBookmarkDoc->
                                 GetSdPage(nBMSdPage, PK_STANDARD)->GetMasterPage(0);
                String* pLayout = new String(pMPage->GetLayoutName());
                pLayout->Erase( pLayout->SearchAscii( SD_LT_SEPARATOR ));

                String* pTest = (String*) pLayoutsToTransfer->First();
                BOOL bFound = FALSE;

                while (pTest && !bFound)    // schon gefunden?
                {
                    if (*pLayout == *pTest)
                        bFound = TRUE;
                    else
                        pTest = (String*)pLayoutsToTransfer->Next();
                }

                for (USHORT nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
                {
                    /**********************************************************
                    * Gibt es die Layouts schon im Dokument?
                    **********************************************************/
                    SdPage* pTest = (SdPage*) GetMasterPage(nMPage);
                    String aTest(pTest->GetLayoutName());
                    aTest.Erase( aTest.SearchAscii( SD_LT_SEPARATOR ));

                    if (aTest == *pLayout)
                        bFound = TRUE;
                }

                if (!bFound)
                    pLayoutsToTransfer->Insert(pLayout, LIST_APPEND);
                else
                    delete pLayout;
            }
        }
    }

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
            SdMoveStyleSheetsUndoAction* pMovStyles =
            new SdMoveStyleSheetsUndoAction(this, pCreatedStyles, TRUE);
            pUndoMgr->AddUndoAction(pMovStyles);
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

        if (bLink)
        {
            // Es werden sich die Namen aller Seiten gemerkt
            for (USHORT nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
            {
                SdPage* pBMPage = pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD);
                String* pName = new String(pBMPage->GetName());
                aNameList.Insert(pName, nBMSdPage);

                if (GetPageByName(*(pName)) != SDRPAGE_NOTFOUND)
                {
                    // Seitenname schon vorhanden -> Defaultname
                    // fuer Standard & Notizseite
                    pBMPage->SetName(String());
                    SdPage* pBMNotesPage = pBookmarkDoc->GetSdPage(nBMSdPage, PK_NOTES);
                    pBMNotesPage->SetName(String());
                }
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

        if (bLink)
        {
            for (USHORT nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
            {
                // Nun werden die Link-Namen zusammengestellt
                SdPage* pBMPage = pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD);
                SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                String* pName = (String*) aNameList.GetObject(nBMSdPage);
                pPage->SetFileName(aBookmarkName);
                pPage->SetBookmarkName(*(pName));
                pPage->SetModel(this);
                delete pName;
                nActualInsertPos += 2;
            }
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
            String aPgName(*(String*) pBookmarkList->GetObject(nPos));

            USHORT nBMPage = pBookmarkDoc->GetPageByName(aPgName);

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
                if (GetPageByName(aPgName) != SDRPAGE_NOTFOUND)
                {
                    // Seitenname schon vorhanden -> Defaultname
                    // fuer Standard & Notizseite
                    pBMPage->SetName(String());
                    SdPage* pBMNotesPage = (SdPage*) pBookmarkDoc->GetPage(nBMPage+1);
                    pBMNotesPage->SetName(String());
                }

                Merge(*pBookmarkDoc,
                      nBMPage,           // Von Seite (Standard)
                      nBMPage+1,         // Bis Seite (Notizen)
                      nActualInsertPos,  // An Position einfuegen
                      bMergeMasterPages, // MasterPages mitnehmen
                      FALSE,             // Aber nur die benoetigten MasterPages
                      TRUE,              // Undo-Aktion erzeugen
                      bCopy);            // Seiten kopieren (oder mergen)

                if( pBookmarkDoc == this )
                {
                    pBMPage->SetName(aPgName);
                    SdPage* pBMNotesPage = (SdPage*) pBookmarkDoc->GetPage(nBMPage+1);
                    pBMNotesPage->SetName(aPgName);
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
                    SdPage* pStandardPage = (SdPage*) GetPage(nActualInsertPos+2);

                    if (pStandardPage)
                    {
                        AddUndo(new SdrUndoDelPage(*pStandardPage));
                        RemovePage(nActualInsertPos+2);
                    }

                    SdPage* pNotesPage = (SdPage*) GetPage(nActualInsertPos+2);

                    if (pNotesPage)
                    {
                        AddUndo(new SdrUndoDelPage(*pNotesPage));
                        RemovePage(nActualInsertPos+2);
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

    EndUndo();
    pUndoMgr->LeaveListAction();

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

        Size  aNSize(pNPage->GetSize());
        INT32 nNLeft  = pNPage->GetLftBorder();
        INT32 nNRight = pNPage->GetRgtBorder();
        INT32 nNUpper = pNPage->GetUppBorder();
        INT32 nNLower = pNPage->GetLwrBorder();

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

            pPage->SetPresentationLayout(aLayout);

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(aSize, aBorderRect, TRUE);
            }
            pPage->SetSize(aSize);
            pPage->SetBorder(nLeft, nUpper, nRight, nLower);

            pPage = GetSdPage(nSdPage, PK_NOTES);
            pPage->SetPresentationLayout(aLayout);

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nNLeft, nNUpper, nNRight, nNLower);
                pPage->ScaleObjects(aNSize, aBorderRect, TRUE);
            }

            pPage->SetSize(aNSize);
            pPage->SetBorder(nNLeft, nNUpper, nNRight, nNLower);
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
            }
        }
    }

    return bContinue;
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Fuegt ein Bookmark als Objekt ein
|*
\************************************************************************/

#ifndef SVX_LIGHT
BOOL SdDrawDocument::InsertBookmarkAsObject(
    List* pBookmarkList,
    List* pExchangeList,            // Liste der zu verwendenen Namen
    BOOL bLink,
    SdDrawDocShell* pBookmarkDocSh,
    Point* pObjPos)
{
    BOOL bOK = TRUE;
    BOOL bOLEObjFound = FALSE;
    SdView* pBMView = NULL;

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
        pBMView = new SdView(pBookmarkDoc, (OutputDevice*) NULL);
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
                    pBMView = new SdView(pBookmarkDoc, (OutputDevice*) NULL);
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
        SdView* pView = new SdView(this, (OutputDevice*) NULL);
        pView->EndListening(*this);

        // Seite bestimmen, auf der die Objekte eingefuegt werden sollen
        SdrPage* pPage = GetSdPage(0, PK_STANDARD);

        if (pDocSh)
        {
            SdViewShell* pViewSh = pDocSh->GetViewShell();

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
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Beendet das Einfuegen von Bookmarks
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::CloseBookmarkDoc()
{
    if (xBookmarkDocShRef.Is())
    {
        xBookmarkDocShRef->DoClose();
    }

    xBookmarkDocShRef.Clear();
    aBookmarkFile = String();
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Dokument laden (fuer gelinkte Objekte)
|*
\************************************************************************/

#ifndef SVX_LIGHT
const SdrModel* __EXPORT SdDrawDocument::LoadModel(const String& rFileName)
{
    return ( OpenBookmarkDoc(rFileName) );
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Dokument schliessen (fuer gelinkte Objekte)
|*
\************************************************************************/

#ifndef SVX_LIGHT
void __EXPORT SdDrawDocument::DisposeLoadedModels()
{
    CloseBookmarkDoc();
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Ist das Dokument read-only?
|*
\************************************************************************/

FASTBOOL __EXPORT SdDrawDocument::IsReadOnly() const
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

#ifndef SVX_LIGHT
void SdDrawDocument::SetAllocDocSh(BOOL bAlloc)
{
    bAllocDocSh = bAlloc;

    if (xAllocedDocShRef.Is())
    {
        xAllocedDocShRef->DoClose();
    }

    xAllocedDocShRef.Clear();
}
#endif // !SVX_LIGHT


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

SvStream* SdDrawDocument::GetDocumentStream(FASTBOOL& rbDeleteAfterUse) const
{
    SvStream* pStream = NULL;

    if (pDocSh)
    {
        pStream = ((SdDrawDocShell*) pDocSh)->GetDocumentStream(rbDeleteAfterUse);
    }

    return pStream;
}


/*************************************************************************
|*
|* Nicht benutzte MasterPages und Layouts entfernen
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::RemoveUnnessesaryMasterPages(SdPage* pMasterPage, BOOL bOnlyDuplicatePages, BOOL bUndo)
{
    SdView* pView = NULL;
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
            pMaster = (SdPage*) GetMasterSdPage( nMPage, PK_STANDARD );
            pNotesMaster = (SdPage*) GetMasterSdPage( nMPage, PK_NOTES );
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
#endif // !SVX_LIGHT



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

#ifndef SVX_LIGHT
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
    SdPage* pOldMaster      = (SdPage*) pSelectedPage->GetMasterPage(0);
    SdPage* pOldNotesMaster = (SdPage*) pNotes->GetMasterPage(0);
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
            USHORT nInsertPos = pOldMaster->GetPageNum();
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
            pPage->SetAutoLayout(eAutoLayout, TRUE);

            pPage = (SdPage*)pPageList->Next();
        }
        delete pPageList;

        /*********************************************************************
        |* Neue Masterpages angleichen
        \********************************************************************/
        if (pSourceDoc != this)
        {
            // die Masterpages angleichen
            Size aSize(pOldMaster->GetSize());
            Rectangle aBorderRect(pOldMaster->GetLftBorder(),
                                  pOldMaster->GetUppBorder(),
                                  pOldMaster->GetRgtBorder(),
                                  pOldMaster->GetLwrBorder());
            pMaster->ScaleObjects(aSize, aBorderRect, TRUE);
            pMaster->SetSize(aSize);
            pMaster->SetBorder(pOldMaster->GetLftBorder(),
                               pOldMaster->GetUppBorder(),
                               pOldMaster->GetRgtBorder(),
                               pOldMaster->GetLwrBorder());
            pMaster->SetAutoLayout(pMaster->GetAutoLayout());

            aSize = pOldNotesMaster->GetSize();
            Rectangle aNotesBorderRect(pOldNotesMaster->GetLftBorder(),
                                       pOldNotesMaster->GetUppBorder(),
                                       pOldNotesMaster->GetRgtBorder(),
                                       pOldNotesMaster->GetLwrBorder());
            pNotesMaster->ScaleObjects(aSize, aNotesBorderRect, TRUE);
            pNotesMaster->SetSize(aSize);
            pNotesMaster->SetBorder(pOldNotesMaster->GetLftBorder(),
                                    pOldNotesMaster->GetUppBorder(),
                                    pOldNotesMaster->GetRgtBorder(),
                                    pOldNotesMaster->GetLwrBorder());
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
                aTest += nCount;

            // gibt's schon eine, die so heisst?
            bNotANewName = FALSE;
            for (USHORT nMPg = 0; nMPg < nMPgCount; nMPg++)
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
        pMaster->SetAutoLayout(AUTOLAYOUT_NONE, TRUE);

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
        pNotesMaster->SetAutoLayout(AUTOLAYOUT_NOTES, TRUE);
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
            pPage->SetAutoLayout(eNewAutoLayout, TRUE);

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
        RemoveUnnessesaryMasterPages(pOldMaster);
    }

    pUndoMgr->LeaveListAction();

    if( pDocSh )
        pDocSh->SetWaitCursor( FALSE );
}
#endif // !SVX_LIGHT



