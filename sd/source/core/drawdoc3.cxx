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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <vcl/wrkwin.hxx>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <sfx2/fcontnr.hxx>
#include <svx/svdopath.hxx>
#include <svx/svditer.hxx>
#include <svl/style.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdundo.hxx>
#include <vcl/msgbox.hxx>
#include <sot/formats.hxx>

#include "glob.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "sdresid.hxx"
#include "sdiocmpt.hxx"
#include "strmname.h"
#include "anminfo.hxx"
#include "customshowlist.hxx"

#include "../ui/inc/unmovss.hxx"
#include "../ui/inc/unchss.hxx"
#include "../ui/inc/unprlout.hxx"
#include "../ui/inc/DrawDocShell.hxx"
#include "../ui/inc/GraphicDocShell.hxx"
#include "../ui/inc/ViewShell.hxx"
#include "../ui/inc/View.hxx"
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"

using namespace ::com::sun::star;

/** Concrete incarnations get called by lcl_IterateBookmarkPages, for
    every page in the bookmark document/list
 */

class InsertBookmarkAsPage_FindDuplicateLayouts
{
public:
    InsertBookmarkAsPage_FindDuplicateLayouts( std::vector<rtl::OUString> &rLayoutsToTransfer )
        : mrLayoutsToTransfer(rLayoutsToTransfer) {}
    void operator()( SdDrawDocument&, SdPage* );
private:
    std::vector<rtl::OUString> &mrLayoutsToTransfer;
};

void InsertBookmarkAsPage_FindDuplicateLayouts::operator()( SdDrawDocument& rDoc, SdPage* pBMMPage )
{
    // now check for duplicate masterpage and layout names
    // ===================================================

    String aFullNameLayout( pBMMPage->GetLayoutName() );
    aFullNameLayout.Erase( aFullNameLayout.SearchAscii( SD_LT_SEPARATOR ));

    rtl::OUString aLayout(aFullNameLayout);

    std::vector<rtl::OUString>::const_iterator pIter =
            find(mrLayoutsToTransfer.begin(),mrLayoutsToTransfer.end(),aLayout);

    bool bFound = pIter != mrLayoutsToTransfer.end();

    const sal_uInt16 nMPageCount = rDoc.GetMasterPageCount();
    for (sal_uInt16 nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
    {
        /**************************************************************
         * Gibt es die Layouts schon im Dokument?
         **************************************************************/
        SdPage* pTestPage = (SdPage*) rDoc.GetMasterPage(nMPage);
        String aFullTest(pTestPage->GetLayoutName());
        aFullTest.Erase( aFullTest.SearchAscii( SD_LT_SEPARATOR ));

        rtl::OUString aTest(aFullTest);

        if (aTest == aLayout)
            bFound = true;
    }

    if (!bFound)
        mrLayoutsToTransfer.push_back(aLayout);
}

/*************************************************************************
|*
|* Fuegt ein Bookmark als Seite ein
|*
\************************************************************************/

void lcl_IterateBookmarkPages( SdDrawDocument &rDoc, SdDrawDocument* pBookmarkDoc,
                               const std::vector<rtl::OUString> &rBookmarkList, sal_uInt16 nBMSdPageCount,
                               InsertBookmarkAsPage_FindDuplicateLayouts& rPageIterator )
{
    //
    // Refactored copy'n'pasted layout name collection from InsertBookmarkAsPage
    //
    int nPos, nEndPos;

    if( rBookmarkList.empty() )
    {
        // no list? whole source document
        nEndPos = nBMSdPageCount;
    }
    else
    {
        // bookmark list? number of entries
        nEndPos = rBookmarkList.size();
    }

    SdPage* pBMPage;

    // iterate over number of pages to insert
    for (nPos = 0; nPos < nEndPos; ++nPos)
    {
        // the master page associated to the nPos'th page to insert
        SdPage* pBMMPage = NULL;

        if( rBookmarkList.empty() )
        {
            // simply take master page of nPos'th page in source document
            pBMMPage = (SdPage*)(&(pBookmarkDoc->GetSdPage((sal_uInt16)nPos, PK_STANDARD)->TRG_GetMasterPage()));
        }
        else
        {
            // fetch nPos'th entry from bookmark list, and determine master page
            String  aBMPgName(rBookmarkList[nPos]);
            sal_Bool  bIsMasterPage;

            sal_uInt16 nBMPage = pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage );

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
                const sal_uInt16 nBMSdPage = (nBMPage - 1) / 2;
                pBMMPage = (SdPage*) (&(pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD)->TRG_GetMasterPage()));
            }
        }

        // successfully determined valid (bookmarked) page?
        if( pBMMPage )
        {
            // yes, call functor
            rPageIterator( rDoc, pBMMPage );
        }
    }
}

/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/

SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(SfxMedium& rMedium)
{
    sal_Bool bOK = sal_True;
    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName = rMedium.GetName();
    const SfxFilter* pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        rMedium.UseInteractionHandler( sal_True );
        SFX_APP()->GetFilterMatcher().GuessFilter( rMedium, &pFilter );
    }

    if ( !pFilter )
    {
        bOK = sal_False;
    }
    else if ( maBookmarkFile != aBookmarkName && aBookmarkName.Len() )
    {
        sal_Bool bCreateGraphicShell = pFilter->GetServiceName() == "com.sun.star.drawing.DrawingDocument";
        sal_Bool bCreateImpressShell = pFilter->GetServiceName() == "com.sun.star.presentation.PresentationDocument";
        if ( bCreateGraphicShell || bCreateImpressShell )
        {
            CloseBookmarkDoc();

            // Es wird eine DocShell erzeugt, da in dem Dokument OLE-Objekte
            // enthalten sein koennten (Persist)
            // Wenn dem nicht so waere, so koennte man auch das Model
            // direkt laden
            if ( bCreateGraphicShell )
                // Draw
                mxBookmarkDocShRef = new ::sd::GraphicDocShell(SFX_CREATE_MODE_STANDARD, sal_True);
            else
                // Impress
                mxBookmarkDocShRef = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD, sal_True);

            bOK = mxBookmarkDocShRef->DoLoad(&rMedium);
            if( bOK )
            {
                maBookmarkFile = aBookmarkName;
                pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            }
        }
    }

    DBG_ASSERT(aBookmarkName.Len(), "Empty document name!");

    if (!bOK)
    {
        ErrorBox aErrorBox( NULL, (WinBits)WB_OK, String(SdResId(STR_READ_DATA_ERROR)));
        aErrorBox.Execute();

        CloseBookmarkDoc();
        pBookmarkDoc = NULL;
    }
    else if (mxBookmarkDocShRef.Is())
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
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

    if (maBookmarkFile != rBookmarkFile && rBookmarkFile.Len())
    {
        SfxMedium* pMedium = new SfxMedium( rBookmarkFile, STREAM_READ );
        pBookmarkDoc = OpenBookmarkDoc(*pMedium);
    }
    else if (mxBookmarkDocShRef.Is())
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
    }

    return(pBookmarkDoc);
}

/*************************************************************************
|*
|* Fuegt ein Bookmark (Seite oder Objekt) ein
|*
\************************************************************************/

sal_Bool SdDrawDocument::InsertBookmark(
    const std::vector<rtl::OUString> &rBookmarkList,    // Liste der Namen der einzufuegenden Bookmarks
    std::vector<rtl::OUString> &rExchangeList,          // Liste der zu verwendenen Namen
    sal_Bool bLink,                                     // Bookmarks sollen als Verknuepfung eingefuegt werden
    sal_Bool bReplace,                                  // Aktuellen Seiten (Standard&Notiz) werden ersetzt
    sal_uInt16 nInsertPos,                              // Einfuegeposition fuer Seiten
    sal_Bool bNoDialogs,                                // Keine Dialoge anzeigen
    ::sd::DrawDocShell* pBookmarkDocSh,                 // Wenn gesetzt, so ist dieses das Source-Dokument
    sal_Bool bCopy,                                     // Seiten werden kopiert
    Point* pObjPos)                                     // Einfuegeposition fuer Objekte
{
    sal_Bool bOK = sal_True;
    sal_Bool bInsertPages = sal_False;

    if (rBookmarkList.empty())
    {
        /**********************************************************************
        * Alle Seiten werden eingefuegt
        **********************************************************************/
        bInsertPages = sal_True;
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
        else if ( mxBookmarkDocShRef.Is() )
        {
            pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            aBookmarkName = maBookmarkFile;
        }
        else
            bOK = sal_False;

        std::vector<rtl::OUString>::const_iterator pIter;
        for ( pIter = rBookmarkList.begin(); bOK && pIter != rBookmarkList.end() && !bInsertPages; ++pIter )
        {
            /******************************************************************
            * Gibt es in der Bookmark-Liste einen Seitennamen?
            ******************************************************************/
            String  aBMPgName(*pIter);
            sal_Bool    bIsMasterPage;

            if( pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // Seite gefunden
                bInsertPages = sal_True;
            }
        }
    }

    sal_Bool bCalcObjCount = !rExchangeList.empty();

    if ( bOK && bInsertPages )
    {
        // Zuerst werden alle Seiten-Bookmarks eingefuegt
        bOK = InsertBookmarkAsPage(rBookmarkList, &rExchangeList, bLink, bReplace,
                                   nInsertPos, bNoDialogs, pBookmarkDocSh, bCopy, sal_True, sal_False);
    }

    if ( bOK && !rBookmarkList.empty() )
    {
        // Es werden alle Objekt-Bookmarks eingefuegt
        bOK = InsertBookmarkAsObject(rBookmarkList, rExchangeList, bLink,
                                     pBookmarkDocSh, pObjPos, bCalcObjCount);
    }

    return bOK;
}

sal_Bool SdDrawDocument::InsertBookmarkAsPage(
    const std::vector<rtl::OUString> &rBookmarkList,
    std::vector<rtl::OUString> *pExchangeList,            // Liste der zu verwendenen Namen
    sal_Bool bLink,
    sal_Bool bReplace,
    sal_uInt16 nInsertPos,
    sal_Bool bNoDialogs,
    ::sd::DrawDocShell* pBookmarkDocSh,
    sal_Bool bCopy,
    sal_Bool bMergeMasterPages,
    sal_Bool bPreservePageNames)
{
    sal_Bool bOK = sal_True;
    sal_Bool bContinue = sal_True;
    sal_Bool bScaleObjects = sal_False;
    sal_uInt16 nReplacedStandardPages = 0;

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
    else if ( mxBookmarkDocShRef.Is() )
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
        aBookmarkName = maBookmarkFile;
    }
    else
    {
        return sal_False;
    }

    const sal_uInt16 nSdPageCount = GetSdPageCount(PK_STANDARD);
    const sal_uInt16 nBMSdPageCount = pBookmarkDoc->GetSdPageCount(PK_STANDARD);
    const sal_uInt16 nMPageCount = GetMasterPageCount();

    if (nSdPageCount==0 || nBMSdPageCount==0 || nMPageCount==0)
    {
        bContinue = bOK = sal_False;
        return(bContinue);
    }

    // Store the size and some other properties of the first page and notes
    // page so that inserted pages can be properly scaled even when inserted
    // before the first page.
    // Note that the pointers are used later on as general page pointers.
    SdPage* pRefPage = GetSdPage(0, PK_STANDARD);
    Size  aSize(pRefPage->GetSize());
    sal_Int32 nLeft  = pRefPage->GetLftBorder();
    sal_Int32 nRight = pRefPage->GetRgtBorder();
    sal_Int32 nUpper = pRefPage->GetUppBorder();
    sal_Int32 nLower = pRefPage->GetLwrBorder();
    Orientation eOrient = pRefPage->GetOrientation();

    SdPage* pNPage = GetSdPage(0, PK_NOTES);
    Size aNSize(GetSdPage(0, PK_NOTES)->GetSize());
    sal_Int32 nNLeft  = pNPage->GetLftBorder();
    sal_Int32 nNRight = pNPage->GetRgtBorder();
    sal_Int32 nNUpper = pNPage->GetUppBorder();
    sal_Int32 nNLower = pNPage->GetLwrBorder();
    Orientation eNOrient = pRefPage->GetOrientation();

    // Seitengroesse und -raender an die Werte der letzten
    // Seiten anpassen?
    pRefPage = GetSdPage(nSdPageCount - 1, PK_STANDARD);

    if( bNoDialogs )
    {
        if( rBookmarkList.empty() )
            bScaleObjects = pRefPage->IsScaleObjects();
        else
            bScaleObjects = sal_True;
    }
    else
    {
        SdPage* pBMPage = pBookmarkDoc->GetSdPage(0,PK_STANDARD);

        if (pBMPage->GetSize()        != pRefPage->GetSize()         ||
            pBMPage->GetLftBorder()   != pRefPage->GetLftBorder()    ||
            pBMPage->GetRgtBorder()   != pRefPage->GetRgtBorder()    ||
            pBMPage->GetUppBorder()   != pRefPage->GetUppBorder()    ||
            pBMPage->GetLwrBorder()   != pRefPage->GetLwrBorder())
        {
            String aStr(SdResId(STR_SCALE_OBJECTS));
            sal_uInt16 nBut = QueryBox( NULL, WB_YES_NO_CANCEL, aStr).Execute();

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
    ::svl::IUndoManager* pUndoMgr = NULL;
    if( mpDocSh )
    {
        pUndoMgr = mpDocSh->GetUndoManager();
        pUndoMgr->EnterListAction(String(SdResId(STR_UNDO_INSERTPAGES)), String());
    }

    //
    // Refactored copy'n'pasted layout name collection into IterateBookmarkPages
    //
    std::vector<rtl::OUString> aLayoutsToTransfer;
    InsertBookmarkAsPage_FindDuplicateLayouts aSearchFunctor( aLayoutsToTransfer );
    lcl_IterateBookmarkPages( *this, pBookmarkDoc, rBookmarkList, nBMSdPageCount, aSearchFunctor );


    /**************************************************************************
    * Die tatsaechlich benoetigten Vorlagen kopieren
    **************************************************************************/
    SdStyleSheetPool* pBookmarkStyleSheetPool =
    (SdStyleSheetPool*) pBookmarkDoc->GetStyleSheetPool();

    // Wenn Vorlagen kopiert werden muessen, dann muessen auch die
    // MasterPages kopiert werden!
    if( !aLayoutsToTransfer.empty() )
        bMergeMasterPages = sal_True;

    std::vector<rtl::OUString>::const_iterator pIter;
    for ( pIter = aLayoutsToTransfer.begin(); pIter != aLayoutsToTransfer.end(); ++pIter )
    {
        SdStyleSheetVector aCreatedStyles;
        String layoutName = *pIter;

        ((SdStyleSheetPool*)GetStyleSheetPool())->CopyLayoutSheets(layoutName, *pBookmarkStyleSheetPool,aCreatedStyles);

        if(!aCreatedStyles.empty())
        {
            if( pUndoMgr )
            {
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, sal_True);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }
    }

    /**************************************************************************
    * Dokument einfuegen
    **************************************************************************/

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(String(SdResId(STR_UNDO_INSERTPAGES)));

    if (rBookmarkList.empty())
    {
        if (nInsertPos >= GetPageCount())
        {
            // Seiten werden hinten angefuegt
            nInsertPos = GetPageCount();
        }

        sal_uInt16 nActualInsertPos = nInsertPos;

        sal_uInt16 nBMSdPage;
        std::set<sal_uInt16> aRenameSet;
        std::map<sal_uInt16,rtl::OUString> aNameMap;

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pBMPage = pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD);
            String  sName( pBMPage->GetName() );
            sal_Bool    bIsMasterPage;

            if (bLink)
            {
                // Es werden sich die Namen aller Seiten gemerkt
                aNameMap.insert(std::make_pair(nBMSdPage,sName));
            }

            // Have to check for duplicate names here, too
            // don't change name if source and dest model are the same!
            if( pBookmarkDoc != this &&
                GetPageByName(sName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // delay renaming *after* pages are copied (might destroy source otherwise)
                aRenameSet.insert(nBMSdPage);
            }
        }

        Merge(*pBookmarkDoc,
              1,                 // Nicht die Handzettelseite
              0xFFFF,            // Aber alle anderen
              nActualInsertPos,  // An Position einfuegen
              bMergeMasterPages, // MasterPages mitnehmen
              sal_False,             // Aber nur die benoetigten MasterPages
              sal_True,              // Undo-Aktion erzeugen
              bCopy);            // Seiten kopieren (oder mergen)

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pPage       = (SdPage*) GetPage(nActualInsertPos);
            SdPage* pNotesPage  = (SdPage*) GetPage(nActualInsertPos+1);

            // delay renaming *after* pages are copied (might destroy source otherwise)
            if( aRenameSet.find(nBMSdPage) != aRenameSet.end() )
            {
                // Seitenname schon vorhanden -> Defaultname
                // fuer Standard & Notizseite
                pPage->SetName(String());
                pNotesPage->SetName(String());
            }

            if (bLink)
            {
                String aName(aNameMap[nBMSdPage]);

                // Nun werden die Link-Namen zusammengestellt
                pPage->SetFileName(aBookmarkName);
                pPage->SetBookmarkName(aName);
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
            bReplace = sal_False;
            nInsertPos = GetPageCount();
        }

        sal_uInt16 nActualInsertPos = nInsertPos;

        // Collect the bookmarked pages.
        ::std::vector<SdPage*> aBookmarkedPages (rBookmarkList.size(), NULL);
        for ( size_t nPos = 0, n = rBookmarkList.size(); nPos < n; ++nPos)
        {
            String  aPgName(rBookmarkList[nPos]);
            sal_Bool    bIsMasterPage;
            sal_uInt16  nBMPage = pBookmarkDoc->GetPageByName( aPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                aBookmarkedPages[nPos] =  dynamic_cast<SdPage*>(pBookmarkDoc->GetPage(nBMPage));
            }
        }

        for ( size_t nPos = 0, n = rBookmarkList.size(); nPos < n; ++nPos)
        {
            pBMPage = aBookmarkedPages[nPos];
            sal_uInt16 nBMPage = pBMPage!=NULL ? pBMPage->GetPageNum() : SDRPAGE_NOTFOUND;

            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                /**************************************************************
                * Es muss eine StandardSeite sein
                **************************************************************/
                sal_Bool bMustRename = sal_False;

                // delay renaming *after* pages are copied (might destroy source otherwise)
                // don't change name if source and dest model are the same!
                // avoid renaming if replacing the same page
                String  aPgName(rBookmarkList[nPos]);
                sal_Bool    bIsMasterPage;
                sal_uInt16 nPageSameName = GetPageByName(aPgName, bIsMasterPage);
                if( pBookmarkDoc != this &&
                    nPageSameName != SDRPAGE_NOTFOUND &&
                    ( !bReplace ||
                      nPageSameName != nActualInsertPos ) )
                {
                    bMustRename = sal_True;
                }

                SdPage* pBookmarkPage = pBMPage;
                if (bReplace )
                {
                    ReplacePageInCustomShows( dynamic_cast< SdPage* >( GetPage( nActualInsertPos ) ), pBookmarkPage );
                }

                Merge(*pBookmarkDoc,
                      nBMPage,           // Von Seite (Standard)
                      nBMPage+1,         // Bis Seite (Notizen)
                      nActualInsertPos,  // An Position einfuegen
                      bMergeMasterPages, // MasterPages mitnehmen
                      sal_False,             // Aber nur die benoetigten MasterPages
                      sal_True,              // Undo-Aktion erzeugen
                      bCopy);            // Seiten kopieren (oder mergen)

                if( bReplace )
                {
                    if( GetPage( nActualInsertPos ) != pBookmarkPage )
                    {
                        // bookmark page was not moved but cloned, so update custom shows again
                        ReplacePageInCustomShows( pBookmarkPage, dynamic_cast< SdPage* >( GetPage( nActualInsertPos ) ) );
                    }
                }

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
                            // Take old slide names for inserted pages
                            SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                            pPage->SetName( pStandardPage->GetRealName() );
                        }

                        if( bUndo )
                            AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pStandardPage));

                        RemovePage(nDestPageNum);

                        if( !bUndo )
                            delete pStandardPage;
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
                            // Take old slide names for inserted pages
                            SdPage* pNewNotesPage = (SdPage*) GetPage(nActualInsertPos+1);
                            if( pNewNotesPage )
                                pNewNotesPage->SetName( pStandardPage->GetRealName() );
                        }

                        if( bUndo )
                            AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pNotesPage));

                        RemovePage(nDestPageNum);

                        if( !bUndo )
                            delete pNotesPage;
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
    sal_uInt16 nNewMPageCount = GetMasterPageCount();

    // rueckwaerts, damit Nummern nicht durcheinander geraten
    for (sal_uInt16 nPage = nNewMPageCount - 1; nPage >= nMPageCount; nPage--)
    {
        pRefPage = (SdPage*) GetMasterPage(nPage);
        String aMPLayout(pRefPage->GetLayoutName());
        PageKind eKind = pRefPage->GetPageKind();

        // gibt's den schon?
        for (sal_uInt16 nTest = 0; nTest < nMPageCount; nTest++)
        {
            SdPage* pTest = (SdPage*) GetMasterPage(nTest);
            String aTest(pTest->GetLayoutName());

            // nInsertPos > 2 is always true when inserting into non-empty models
            if ( nInsertPos > 2 &&
                 aTest == aMPLayout &&
                 eKind == pTest->GetPageKind() )
            {
                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pRefPage));

                RemoveMasterPage(nPage);

                if( !bUndo )
                    delete pRefPage;
                nNewMPageCount--;
                break;
            }
        }
    }

    // nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 0)
    {
        sal_uInt16 nSdPageStart = (nInsertPos - 1) / 2;
        sal_uInt16 nSdPageEnd = GetSdPageCount(PK_STANDARD) - nSdPageCount +
                            nSdPageStart - 1;
        const bool bRemoveEmptyPresObj = pBookmarkDoc &&
                (pBookmarkDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) &&
                (GetDocumentType() == DOCUMENT_TYPE_DRAW);

        if( bReplace )
        {
            nSdPageEnd = nSdPageStart + nReplacedStandardPages - 1;
        }

        std::vector<rtl::OUString>::iterator pExchangeIter;

        if (pExchangeList)
            pExchangeIter = pExchangeList->begin();

        for (sal_uInt16 nSdPage = nSdPageStart; nSdPage <= nSdPageEnd; nSdPage++)
        {
            pRefPage = GetSdPage(nSdPage, PK_STANDARD);

            if (pExchangeList && pExchangeIter != pExchangeList->end())
            {
                // Zuverwendener Name aus Exchange-Liste holen
                String aExchangeName (*pExchangeIter);
                pRefPage->SetName(aExchangeName);
                SdrHint aHint(HINT_PAGEORDERCHG);
                aHint.SetPage(pRefPage);
                Broadcast(aHint);
                SdPage* pNewNotesPage = GetSdPage(nSdPage, PK_NOTES);
                pNewNotesPage->SetName(aExchangeName);
                aHint.SetPage(pNewNotesPage);
                Broadcast(aHint);

                ++pExchangeIter;
            }

            String aLayout(pRefPage->GetLayoutName());
            aLayout.Erase(aLayout.SearchAscii( SD_LT_SEPARATOR ));

            // update layout and referred master page
            pRefPage->SetPresentationLayout(aLayout);
            if( bUndo )
                AddUndo( GetSdrUndoFactory().CreateUndoPageChangeMasterPage( *pRefPage ) );

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pRefPage->ScaleObjects(aSize, aBorderRect, sal_True);
            }
            pRefPage->SetSize(aSize);
            pRefPage->SetBorder(nLeft, nUpper, nRight, nLower);
            pRefPage->SetOrientation( eOrient );

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();

            pRefPage = GetSdPage(nSdPage, PK_NOTES);

            // update layout and referred master page
            pRefPage->SetPresentationLayout(aLayout);
            if( bUndo )
                AddUndo( GetSdrUndoFactory().CreateUndoPageChangeMasterPage( *pRefPage ) );

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nNLeft, nNUpper, nNRight, nNLower);
                pRefPage->ScaleObjects(aNSize, aBorderRect, sal_True);
            }

            pRefPage->SetSize(aNSize);
            pRefPage->SetBorder(nNLeft, nNUpper, nNRight, nNLower);
            pRefPage->SetOrientation( eNOrient );

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();
        }

        ///Remove processed elements, to avoid doings hacks in InsertBookmarkAsObject
        if ( pExchangeList )
            pExchangeList->erase(pExchangeList->begin(),pExchangeIter);

        for (sal_uInt16 nPage = nMPageCount; nPage < nNewMPageCount; nPage++)
        {
            pRefPage = (SdPage*) GetMasterPage(nPage);
            if (pRefPage->GetPageKind() == PK_STANDARD)
            {
                if (bScaleObjects)
                {
                    Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                    pRefPage->ScaleObjects(aSize, aBorderRect, sal_True);
                }
                pRefPage->SetSize(aSize);
                pRefPage->SetBorder(nLeft, nUpper, nRight, nLower);
                pRefPage->SetOrientation( eOrient );
            }
            else        // kann nur noch NOTES sein
            {
                if (bScaleObjects)
                {
                    Rectangle aBorderRect(nNLeft, nNUpper, nNRight, nNLower);
                    pRefPage->ScaleObjects(aNSize, aBorderRect, sal_True);
                }
                pRefPage->SetSize(aNSize);
                pRefPage->SetBorder(nNLeft, nNUpper, nNRight, nNLower);
                pRefPage->SetOrientation( eNOrient );
            }

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();
        }
    }

    // Make absolutely sure no double masterpages are there
    RemoveUnnecessaryMasterPages(NULL, sal_True, sal_True);

    if( bUndo )
        EndUndo();
    pUndoMgr->LeaveListAction();

    return bContinue;
}

/*************************************************************************
|*
|* Fuegt ein Bookmark als Objekt ein
|*
\************************************************************************/

sal_Bool SdDrawDocument::InsertBookmarkAsObject(
    const std::vector<rtl::OUString> &rBookmarkList,
    const std::vector<rtl::OUString> &rExchangeList,            // Liste der zu verwendenen Namen
    sal_Bool /* bLink */,
    ::sd::DrawDocShell* pBookmarkDocSh,
    Point* pObjPos, bool bCalcObjCount)
{
    sal_Bool bOK = sal_True;
    sal_Bool bOLEObjFound = sal_False;
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
    else if ( mxBookmarkDocShRef.Is() )
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
        aBookmarkName = maBookmarkFile;
    }
    else
    {
        return sal_False;
    }

    if (rBookmarkList.empty())
    {
        pBMView = new ::sd::View(*pBookmarkDoc, (OutputDevice*) NULL);
        pBMView->EndListening(*pBookmarkDoc);
        pBMView->MarkAll();
    }
    else
    {
        SdrPage* pPage;
        SdrPageView* pPV;

        std::vector<rtl::OUString>::const_iterator pIter;
        for ( pIter = rBookmarkList.begin(); pIter != rBookmarkList.end(); ++pIter )
        {
            /******************************************************************
            * Namen der Bookmarks aus Liste holen
            ******************************************************************/
            String aBMName (*pIter);

            SdrObject* pObj = pBookmarkDoc->GetObj(aBMName);

            if (pObj)
            {
                // Objekt gefunden

                if (pObj->GetObjInventor() == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_OLE2)
                {
                    bOLEObjFound = sal_True;
                }

                if (!pBMView)
                {
                    // View erstmalig erzeugen
                    pBMView = new ::sd::View(*pBookmarkDoc, (OutputDevice*) NULL);
                    pBMView->EndListening(*pBookmarkDoc);
                }

                pPage = pObj->GetPage();

                if (pPage->IsMasterPage())
                {
                    pPV = pBMView->ShowSdrPage(pBMView->GetModel()->GetMasterPage(pPage->GetPageNum()));
                }
                else
                {
                    pPV = pBMView->GetSdrPageView();
                    if( !pPV || (pPV->GetPage() != pPage))
                        pPV = pBMView->ShowSdrPage(pPage);
                }

                pBMView->MarkObj(pObj, pPV, sal_False);
            }
        }
    }

    if (pBMView)
    {
        /**********************************************************************
        * Selektierte Objekte einfuegen
        **********************************************************************/
        ::sd::View* pView = new ::sd::View(*this, (OutputDevice*) NULL);
        pView->EndListening(*this);

        // Seite bestimmen, auf der die Objekte eingefuegt werden sollen
        SdrPage* pPage = GetSdPage(0, PK_STANDARD);

        if (mpDocSh)
        {
            ::sd::ViewShell* pViewSh = mpDocSh->GetViewShell();

            if (pViewSh)
            {
                // Welche Seite wird denn aktuell angezeigt?
                SdrPageView* pPV = pViewSh->GetView()->GetSdrPageView();

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

        sal_uLong nCountBefore = 0;

        if (!rExchangeList.empty() || bCalcObjCount)
        {
            // OrdNums sortieren und Anzahl Objekte vor dem Einfuegen bestimmen
            pPage->RecalcObjOrdNums();
            nCountBefore = pPage->GetObjCount();
        }

        if (bOLEObjFound)
            pBMView->GetDoc().SetAllocDocSh(sal_True);

        SdDrawDocument* pTmpDoc = (SdDrawDocument*) pBMView->GetAllMarkedModel();
        bOK = pView->Paste(*pTmpDoc, aObjPos, pPage);

        if (bOLEObjFound)
            pBMView->GetDoc().SetAllocDocSh(sal_False);

        if (!bOLEObjFound)
            delete pTmpDoc;             // Wird ansonsten von der DocShell zerstoert

        delete pView;

        if (!rExchangeList.empty())
        {
            // Anzahl Objekte nach dem Einfuegen bestimmen
            sal_uLong nCount = pPage->GetObjCount();

            std::vector<rtl::OUString>::const_iterator pIter = rExchangeList.begin();
            for (sal_uLong nObj = nCountBefore; nObj < nCount; nObj++)
            {
                // Zuverwendener Name aus Exchange-Liste holen
                if (pIter != rExchangeList.end())
                {
                    String aExchangeName (*pIter);

                    if (pPage->GetObj(nObj))
                    {
                        pPage->GetObj(nObj)->SetName(aExchangeName);
                    }

                    ++pIter;
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
    if (mxBookmarkDocShRef.Is())
    {
        mxBookmarkDocShRef->DoClose();
    }

    mxBookmarkDocShRef.Clear();
    maBookmarkFile = String();
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

bool SdDrawDocument::IsReadOnly() const
{
    return sal_False;
}


/*************************************************************************
|*
|* In anschliessendem AllocModel() wird eine DocShell erzeugt
|* (xAllocedDocShRef). Eine bereits bestehende DocShell wird ggf. geloescht
|*
\************************************************************************/

void SdDrawDocument::SetAllocDocSh(sal_Bool bAlloc)
{
    mbAllocDocSh = bAlloc;

    if(mxAllocedDocShRef.Is())
    {
        mxAllocedDocShRef->DoClose();
    }

    mxAllocedDocShRef.Clear();
}

/*************************************************************************
|*
|* Liste der CustomShows zurueckgeben (ggf. zuerst erzeugen)
|*
\************************************************************************/

SdCustomShowList* SdDrawDocument::GetCustomShowList(sal_Bool bCreate)
{
    if (!mpCustomShowList && bCreate)
    {
        // Liste erzeugen
        mpCustomShowList = new SdCustomShowList;
    }

    return mpCustomShowList;
}

/*************************************************************************
|*
|* Nicht benutzte MasterPages und Layouts entfernen
|*
\************************************************************************/

void SdDrawDocument::RemoveUnnecessaryMasterPages(SdPage* pMasterPage, sal_Bool bOnlyDuplicatePages, sal_Bool bUndo)
{
    ::sd::View* pView = NULL;
    ::svl::IUndoManager* pUndoMgr = NULL;

    if( bUndo && !IsUndoEnabled() )
        bUndo = sal_False;

    if (mpDocSh)
    {
        pUndoMgr = mpDocSh->GetUndoManager();

        if (mpDocSh->GetViewShell())
            pView = mpDocSh->GetViewShell()->GetView();
    }

    /***********************************************************
    * Alle MasterPages pruefen
    ***********************************************************/
    sal_uInt16 nSdMasterPageCount = GetMasterSdPageCount( PK_STANDARD );
    for (sal_Int32 nMPage = nSdMasterPageCount - 1; nMPage >= 0; nMPage--)
    {
        SdPage* pMaster = pMasterPage;
        SdPage* pNotesMaster = NULL;

        if (!pMaster)
        {
            pMaster = (SdPage*) GetMasterSdPage( (sal_uInt16) nMPage, PK_STANDARD );
            pNotesMaster = (SdPage*) GetMasterSdPage( (sal_uInt16) nMPage, PK_NOTES );
        }
        else
        {
            for ( sal_uInt16 nMPg = 0; nMPg < GetMasterPageCount(); nMPg++ )
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
            // Do not delete master pages that have their precious flag set.
            sal_Bool bDeleteMaster = !pMaster->IsPrecious();
            OUString aLayoutName = pMaster->GetLayoutName();

            if(bOnlyDuplicatePages )
            {
                // remove only duplicate pages
                bDeleteMaster = sal_False;
                for (sal_uInt16 i = 0; i < GetMasterSdPageCount( PK_STANDARD ); i++)
                {
                    SdPage* pMPg = (SdPage*) GetMasterSdPage( i, PK_STANDARD );
                    if( pMPg != pMaster &&
                        pMPg->GetLayoutName() == aLayoutName )
                    {
                        // duplicate page found -> remove it
                        bDeleteMaster = sal_True;
                    }
                }
            }

            if( bDeleteMaster )
            {
                if (pView)
                {
                    // if MasterPage is visible hide on pageview
                    SdrPageView* pPgView = pView->GetSdrPageView();
                    if (pPgView)
                    {
                        SdrPage* pShownPage = pPgView->GetPage();
                        if( (pShownPage == pMaster) || (pShownPage == pNotesMaster) )
                        {
                            pView->HideSdrPage();
                            pView->ShowSdrPage( GetSdPage( 0, PK_STANDARD ) );
                        }
                    }
                }

                if( bUndo )
                {
                    BegUndo();
                    AddUndo( GetSdrUndoFactory().CreateUndoDeletePage( *pNotesMaster ) );
                }

                RemoveMasterPage( pNotesMaster->GetPageNum() );

                if( !bUndo )
                    delete pNotesMaster;

                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pMaster));

                RemoveMasterPage( pMaster->GetPageNum() );

                if( !bUndo )
                    delete pMaster;

                if( bUndo )
                    EndUndo();  // schon hier, damit sich Joes Actions ZWISCHEN unsere eigenen schieben

                // alte Layoutvorlagen loeschen, wenn sie nicht mehr benoetigt werden
                sal_Bool bDeleteOldStyleSheets = sal_True;
                for ( sal_uInt16 nMPg = 0;
                          nMPg < GetMasterPageCount() && bDeleteOldStyleSheets;
                       nMPg++ )
                {
                    SdPage* pMPg = (SdPage*) GetMasterPage(nMPg);
                    if (pMPg->GetLayoutName() == aLayoutName)
                    {
                        bDeleteOldStyleSheets = sal_False;
                    }
                }

                if (bDeleteOldStyleSheets)
                {
                    SdStyleSheetVector aRemove;
                    static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutSheetList( aLayoutName, aRemove );

                    if( bUndo )
                    {
                        // die Liste gehoert der UndoAction
                        SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, aRemove, false );

                        if (pUndoMgr)
                            pUndoMgr->AddUndoAction(pMovStyles);
                    }

                    for( SdStyleSheetVector::iterator iter = aRemove.begin(); iter != aRemove.end(); ++iter )
                        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->Remove((*iter).get());
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

void SdDrawDocument::SetMasterPage(sal_uInt16 nSdPageNum,
                                   const String& rLayoutName,
                                   SdDrawDocument* pSourceDoc,
                                   sal_Bool bMaster,
                                   sal_Bool bCheckMasters)
{
    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_True );

    ::svl::IUndoManager* pUndoMgr = mpDocSh->GetUndoManager();

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        pUndoMgr->EnterListAction(String(SdResId(STR_UNDO_SET_PRESLAYOUT)), String());
    }

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
        std::vector<StyleReplaceData> aReplList; // Liste fuer ersetzte StyleSheets
        sal_Bool bLayoutReloaded = sal_False;   // Wurde ex. Layout wieder geladen?

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
            OUStringBuffer aBuf(rLayoutName);
            aBuf.append(SD_LT_SEPARATOR).append(SdResId(STR_LAYOUT_OUTLINE).toString());
            OUString aSearchFor(aBuf.makeStringAndClear());

            for (sal_uInt16 nMP = 0; nMP < pSourceDoc->GetMasterPageCount(); ++nMP)
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

            if( mpDocSh )
                mpDocSh->SetWaitCursor( sal_False );

            OSL_FAIL( "SdDrawDocument::SetMasterPage() failed!" );

            return;
        }

        if (pSourceDoc != this)
        {
            const sal_uInt16 nMasterPageCount = GetMasterPageCount();
            for ( sal_uInt16 nMPage = 0; nMPage < nMasterPageCount; nMPage++ )
            {
                SdPage* pCheckMaster = (SdPage*)GetMasterPage(nMPage);
                if( pCheckMaster->GetName() == aNewLayoutName )
                {
                    bLayoutReloaded = sal_True;
                    break;
                }
            }

            /*****************************************************************
            |* Praesentationsvorlagen korrigieren bzw. neu anlegen
            \****************************************************************/
            // nur die Praesentationsvorlagen beachten
            String aName;
            SdStyleSheetPool* pSourceStyleSheetPool = (SdStyleSheetPool*) pSourceDoc->GetStyleSheetPool();
            pSourceStyleSheetPool->SetSearchMask(SD_STYLE_FAMILY_MASTERPAGE);
            static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->SetSearchMask(SD_STYLE_FAMILY_MASTERPAGE);

            SdStyleSheetVector aCreatedStyles;          // Liste fuer erzeugte StyleSheets
            SfxStyleSheetBase* pHisSheet = pSourceStyleSheetPool->First();

            while (pHisSheet)
            {
                aName = pHisSheet->GetName();

                if( aName.Search( aNewLayoutName ) == 0 )
                {
                    SfxStyleSheet* pMySheet = static_cast<SfxStyleSheet*>( mxStyleSheetPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE) );

                    if (pMySheet)
                    {
                        // Es ist eine gleichnamige Vorlage vorhanden ist: Inhalte ersetzen
#ifdef DBG_UTIL
                        sal_Bool bTest =
#endif
                            pMySheet->SetName(pHisSheet->GetName());
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
                        // create new style
                        String aHelpFile;
                        pMySheet = static_cast<SfxStyleSheet*>( &mxStyleSheetPool->Make(aName, SD_STYLE_FAMILY_MASTERPAGE, pHisSheet->GetMask()) );
                        pMySheet->SetHelpId( aHelpFile, pHisSheet->GetHelpId(aHelpFile) );
                        pMySheet->GetItemSet().ClearItem(0);  // alle loeschen
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());

                        aCreatedStyles.push_back( SdStyleSheetRef( static_cast< SdStyleSheet* >( pMySheet ) ) );
                    }

                    StyleReplaceData aReplData;
                    aReplData.nNewFamily = pMySheet->GetFamily();
                    aReplData.nFamily    = pMySheet->GetFamily();
                    aReplData.aNewName   = pMySheet->GetName();

                    String aTemp(pMySheet->GetName());
                    sal_uInt16 nPos = aTemp.SearchAscii( SD_LT_SEPARATOR );
                    aTemp.Erase(0, nPos);
                    aTemp.Insert(aOldLayoutName, 0);
                    aReplData.aName = aTemp;
                    aReplList.push_back(aReplData);
                }

                pHisSheet = (SfxStyleSheet*) pSourceStyleSheetPool->Next();
            }

            // wenn neue Vorlagen erzeugt wurden:
            // eventuell bestehende Parent-Verkettung der Itemsets in den
            // Vorlagen wieder aufbauen
            if(!aCreatedStyles.empty())
            {
                std::vector<StyleReplaceData>::iterator pRDataIter;
                for ( pRDataIter = aReplList.begin(); pRDataIter != aReplList.end(); ++pRDataIter )
                {
                    SfxStyleSheetBase* pSOld = mxStyleSheetPool->Find(pRDataIter->aName);
                    SfxStyleSheetBase* pSNew = mxStyleSheetPool->Find(pRDataIter->aNewName);

                    if (pSOld && pSNew)
                    {
                        const String& rParentOfOld = pSOld->GetParent();
                        const String& rParentOfNew = pSNew->GetParent();

                        if (rParentOfOld.Len() > 0 && rParentOfNew.Len() == 0)
                        {
                            std::vector<StyleReplaceData>::iterator pRDIter;
                            for ( pRDIter = aReplList.begin(); pRDIter != aReplList.end(); ++pRDIter )
                            {
                                if ((pRDIter->aName == rParentOfOld) && (pRDIter->aName != pRDIter->aNewName))
                                {
                                    String aParentOfNew(pRDIter->aNewName);
                                    pSNew->SetParent(aParentOfNew);
                                    break;
                                }
                            }
                        }
                    }
                }

                // ab jetzt beim Suchen alle beachten
                pSourceStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_ALL);
                mxStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_ALL);
            }

            if( !aCreatedStyles.empty() )
            {
                // UndoAction fuer das Erzeugen und Einfuegen vorn StyleSheets
                // auf den UndoManager legen
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, aCreatedStyles, sal_True);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }

        // Layoutnamen auf Basis des Seitenlayoutnamens der Masterpage bilden
        String aPageLayoutName(pMaster->GetLayoutName());
        String aLayoutName = aPageLayoutName;
        aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ));

        if (pSourceDoc != this)
        {
            // Aus dem Source-Dokument austragen
            pSourceDoc->RemoveMasterPage(pNotesMaster->GetPageNum());
            pSourceDoc->RemoveMasterPage(pMaster->GetPageNum());
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
            sal_uInt16 nInsertPos = rOldMaster.GetPageNum();
            BegUndo();

            if (!bLayoutReloaded)
                nInsertPos = 0xFFFF;
            InsertMasterPage(pMaster, nInsertPos);
            if( bUndo )
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

            nInsertPos++;
            if (!bLayoutReloaded)
                nInsertPos = 0xFFFF;
            InsertMasterPage(pNotesMaster, nInsertPos);
            if( bUndo )
            {
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pNotesMaster));

                EndUndo(); // schon hier, damit sich Joes Actions ZWISCHEN unsere eigenen schieben
            }
        }

        // Liste mit Seiten fuellen
        std::vector<SdPage*> aPageList;

//      #98456, this has to be removed according to CL (KA 07/08/2002)
//      #109884# but we need them again to restore the styles of the presentation objects while undo
        aPageList.push_back(pMaster);
        aPageList.push_back(pNotesMaster);

        if (bMaster || bLayoutReloaded)
        {
            for (sal_uInt16 nPage = 1; nPage < GetPageCount(); nPage++)
            {
                pPage = (SdPage*) GetPage(nPage);
                String aTest = pPage->GetLayoutName();
                if (aTest == aOldPageLayoutName)
                {
                    aPageList.push_back(pPage);
                }
            }

        }
        else
        {
            aPageList.push_back(pSelectedPage);
            aPageList.push_back(pNotes);
        }

        for (std::vector<SdPage*>::iterator pIter = aPageList.begin(); pIter != aPageList.end(); ++pIter)
        {
            AutoLayout eAutoLayout = (*pIter)->GetAutoLayout();

            if( bUndo )
            {
                SdPresentationLayoutUndoAction * pPLUndoAction =
                    new SdPresentationLayoutUndoAction
                        (this,
                        ( pPage && pPage->IsMasterPage() ) ? aLayoutName : aOldLayoutName,
                        aLayoutName,
                         eAutoLayout, eAutoLayout, sal_False, *pIter);
                pUndoMgr->AddUndoAction(pPLUndoAction);
            }
            (*pIter)->SetPresentationLayout(aLayoutName);
            (*pIter)->SetAutoLayout(eAutoLayout);
        }

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
            pMaster->ScaleObjects(aSize, aBorderRect, sal_True);
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
            pNotesMaster->ScaleObjects(aSize, aNotesBorderRect, sal_True);
            pNotesMaster->SetSize(aSize);
            pNotesMaster->SetBorder(rOldNotesMaster.GetLftBorder(),
                                    rOldNotesMaster.GetUppBorder(),
                                    rOldNotesMaster.GetRgtBorder(),
                                    rOldNotesMaster.GetLwrBorder());
            pNotesMaster->SetOrientation( rOldNotesMaster.GetOrientation() );
            pNotesMaster->SetAutoLayout(pNotesMaster->GetAutoLayout());

            if( (pSourceDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) &&
                (GetDocumentType() == DOCUMENT_TYPE_DRAW) )
            {
                pMaster->RemoveEmptyPresentationObjects();
                pNotesMaster->RemoveEmptyPresentationObjects();
            }
        }
    }
    else
    {
        /*********************************************************************
        |* Einen neuen Layoutnamen ausdenken
        \********************************************************************/
        String aName        = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
        String aTest;
        sal_Bool   bNotANewName = sal_True;
        sal_uInt16 nCount       = 0;
        sal_uInt16 nMPgCount    = GetMasterPageCount();

        for (nCount = 0; bNotANewName; nCount++)
        {
            // Testnamen bilden
            aTest = aName;              // Standard, Standard1, Standard2, ...
            if (nCount > 0)
                aTest += String::CreateFromInt32( nCount );

            // gibt's schon eine, die so heisst?
            bNotANewName = sal_False;
            for (sal_uInt16 nMPg = 1; nMPg < nMPgCount; nMPg++)
            {
                const SdrPage* pTest = GetMasterPage(nMPg);
                String aPageLayoutName(pTest->GetLayoutName());
                aPageLayoutName.Erase( aPageLayoutName.SearchAscii( SD_LT_SEPARATOR ));

                if (aPageLayoutName == aTest)
                    bNotANewName = sal_True;
            }
        }
        aName = aTest;
        String aPageLayoutName(aName);
        aPageLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
        aPageLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

        /*********************************************************************
        |* Neue StyleSheets erzeugen
        \********************************************************************/
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutStyleSheets(aName);
        SdStyleSheetVector aCreatedStyles;
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutSheetList(aName, aCreatedStyles);

        if( bUndo )
        {
            SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, sal_True);
            pUndoMgr->AddUndoAction(pMovStyles);
        }

        /*********************************************************************
        |* Neue MasterPages erzeugen und ins Dokument eintragen
        \********************************************************************/

        if( bUndo )
            BegUndo();

        pMaster = (SdPage*) AllocPage(sal_True);
        pMaster->SetSize(pSelectedPage->GetSize());
        pMaster->SetBorder(pSelectedPage->GetLftBorder(),
                           pSelectedPage->GetUppBorder(),
                           pSelectedPage->GetRgtBorder(),
                           pSelectedPage->GetLwrBorder() );
        pMaster->SetName(aName);
        pMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pMaster);

        if( bUndo )
            AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

        pMaster->SetAutoLayout(AUTOLAYOUT_NONE, true, true);

        pNotesMaster = (SdPage*) AllocPage(sal_True);
        pNotesMaster->SetPageKind(PK_NOTES);
        pNotesMaster->SetSize(pNotes->GetSize());
        pNotesMaster->SetBorder(pNotes->GetLftBorder(),
                                pNotes->GetUppBorder(),
                                pNotes->GetRgtBorder(),
                                pNotes->GetLwrBorder() );
        pNotesMaster->SetName(aName);
        pNotesMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pNotesMaster);

        if( bUndo )
            AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pNotesMaster));

        pNotesMaster->SetAutoLayout(AUTOLAYOUT_NOTES, true, true);

        if( bUndo )
            EndUndo();

        /*********************************************************************
        |* Liste der betroffenen Standard- und Notizseiten erstellen
        \********************************************************************/
        std::vector<SdPage*> aPageList;
        if (bMaster)
        {
            for (sal_uInt16 nPage = 1; nPage < GetPageCount(); nPage++)
            {
                pPage = (SdPage*) GetPage(nPage);
                const String s(pPage->GetLayoutName());
                if(s == aOldPageLayoutName)
                {
                    aPageList.push_back(pPage);
                }
            }
        }
        else
        {
            aPageList.push_back(pSelectedPage);
            aPageList.push_back(pNotes);
        }

        /*********************************************************************
        |* An den betroffenen Seiten Praesentations- und Autolayout setzen
        \********************************************************************/
        for ( std::vector<SdPage*>::iterator pIter = aPageList.begin(); pIter != aPageList.end(); ++pIter )
        {
            AutoLayout eOldAutoLayout = (*pIter)->GetAutoLayout();
            AutoLayout eNewAutoLayout =
                (*pIter)->GetPageKind() == PK_STANDARD ? AUTOLAYOUT_NONE : AUTOLAYOUT_NOTES;

            if( bUndo )
            {
                SdPresentationLayoutUndoAction * pPLUndoAction =
                    new SdPresentationLayoutUndoAction
                            (this, aOldLayoutName, aName,
                             eOldAutoLayout, eNewAutoLayout, sal_True,
                             *pIter);
                pUndoMgr->AddUndoAction(pPLUndoAction);
            }

            (*pIter)->SetPresentationLayout(aName);
            (*pIter)->SetAutoLayout(eNewAutoLayout);
        }
    }

    /*********************************************************************
    |* falls die alten Masterpages nicht mehr benoetigt werden,
    |* muessen sie und die entsprechenden Praesentationsvorlagen
    |* entfernt werden
    \********************************************************************/
    if (bCheckMasters)
    {
        // Alle pruefen
        RemoveUnnecessaryMasterPages();
    }
    else
    {
        // Nur die ausgetauschte MasterPage pruefen
        RemoveUnnecessaryMasterPages(&rOldMaster);
    }

    if( bUndo )
        pUndoMgr->LeaveListAction();

    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_False );
}



void SdDrawDocument::Merge(SdrModel& rSourceModel,
               sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
               sal_uInt16 nDestPos,
               bool bMergeMasterPages, bool bAllMasterPages,
               bool bUndo, bool bTreadSourceAsConst)
{
    sal_uInt16 nMasterPageCount = GetMasterPageCount();
    SdrModel::Merge( rSourceModel, nFirstPageNum, nLastPageNum, nDestPos, bMergeMasterPages, bAllMasterPages, bUndo, bTreadSourceAsConst );

    // add style family for each new master page
    for( sal_uInt16 nMaster = nMasterPageCount; nMaster < GetMasterPageCount(); nMaster++ )
    {
        SdPage* pPage = static_cast< SdPage* >( GetMasterPage( nMaster ) );
        if( pPage && pPage->IsMasterPage() && (pPage->GetPageKind() == PK_STANDARD) )
        {
            // new master page created, add its style family
            SdStyleSheetPool* pStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
            if( pStylePool )
                pStylePool->AddStyleFamily( pPage );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
