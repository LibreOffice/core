/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <utility>
#include <algorithm>
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
#include <sot/storage.hxx>
#include <sot/formats.hxx>

#include <set>
#include <boost/bind.hpp>

#include "glob.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "sdresid.hxx"
#include "sdiocmpt.hxx"
#include "strmname.h"
#include "anminfo.hxx"

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

#define POOL_BUFFER_SIZE        (sal_uInt16)32768
#define BASIC_BUFFER_SIZE       (sal_uInt16)8192
#define DOCUMENT_BUFFER_SIZE    (sal_uInt16)32768

/*************************************************************************
|*
|* Oeffnet ein Bookmark-Dokument
|*
\************************************************************************/
/*
SdStorageListener : public BaseImplHelper1 < lang::XEventListener >
{
    uno::Reference < embed::XStorage >& xStor;
public:
            SdStorageListener ( uno::Reference < embed::XStorage >& rStor )
                : xStor( rStor )
            {}

    void disposing ( const lang::EventObject& aEvent ) throw ( uno::RuntimeException );
};

void SdStorageListener::disposing( const lang::EventObject& aEvent ) throw ( uno::RuntimeException )
{
    xStor = NULL;
}*/

SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(SfxMedium& rMedium)
{
    bool bOK = true;
    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName = rMedium.GetName();
    const SfxFilter* pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        rMedium.UseInteractionHandler( true );
        SFX_APP()->GetFilterMatcher().GuessFilter( rMedium, &pFilter );
    }

    if ( !pFilter )
    {
        bOK = false;
    }
    else if ( maBookmarkFile != aBookmarkName && aBookmarkName.Len() )
    {
        bool bCreateGraphicShell = pFilter->GetServiceName().EqualsAscii( "com.sun.star.drawing.DrawingDocument" );
        bool bCreateImpressShell = pFilter->GetServiceName().EqualsAscii( "com.sun.star.presentation.PresentationDocument" );
        if ( bCreateGraphicShell || bCreateImpressShell )
        {
            CloseBookmarkDoc();

            // Es wird eine DocShell erzeugt, da in dem Dokument OLE-Objekte
            // enthalten sein koennten (Persist)
            // Wenn dem nicht so waere, so koennte man auch das Model
            // direkt laden
            if ( bCreateGraphicShell )
                // Draw
                mxBookmarkDocShRef = new ::sd::GraphicDocShell(SFX_CREATE_MODE_STANDARD, true);
            else
                // Impress
                mxBookmarkDocShRef = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD, true);

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
        SfxMedium* pMedium = new SfxMedium( rBookmarkFile, STREAM_READ, false );
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

bool SdDrawDocument::InsertBookmark(
    List* pBookmarkList,            // Liste der Namen der einzufuegenden Bookmarks
    List* pExchangeList,            // Liste der zu verwendenen Namen
    bool bLink,                     // Bookmarks sollen als Verknuepfung eingefuegt werden
    bool bReplace,                  // Aktuellen Seiten (Standard&Notiz) werden ersetzt
    sal_uInt32 nInsertPos,              // Einfuegeposition fuer Seiten
    bool bNoDialogs,                // Keine Dialoge anzeigen
    ::sd::DrawDocShell* pBookmarkDocSh, // Wenn gesetzt, so ist dieses das Source-Dokument
    bool bCopy,                     // Seiten werden kopiert
    basegfx::B2DPoint* pObjPos)                 // Einfuegeposition fuer Objekte
{
    bool bOK = true;
    bool bInsertPages = false;

    if (!pBookmarkList)
    {
        /**********************************************************************
        * Alle Seiten werden eingefuegt
        **********************************************************************/
        bInsertPages = true;
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
            bOK = false;

        for (sal_uInt16 nPos = 0; bOK && ( nPos < pBookmarkList->Count() ) && !bInsertPages; nPos++)
        {
            /******************************************************************
            * Gibt es in der Bookmark-Liste einen Seitennamen?
            ******************************************************************/
            String  aBMPgName (*(String*) pBookmarkList->GetObject(nPos));
            bool bIsMasterPage;

            if( pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // Seite gefunden
                bInsertPages = true;
            }
        }
    }

    if ( bOK && bInsertPages )
    {
        // Zuerst werden alle Seiten-Bookmarks eingefuegt
        bOK = InsertBookmarkAsPage(pBookmarkList, pExchangeList, bLink, bReplace, nInsertPos, bNoDialogs, pBookmarkDocSh, bCopy, true, false);
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

void SdDrawDocument::IterateBookmarkPages( SdDrawDocument* pBookmarkDoc, List* pBookmarkList, sal_uInt32 nBMSdPageCount,
                                           SdDrawDocument::InsertBookmarkAsPage_PageFunctorBase& rPageIterator )
{
    //
    // #96029# Refactored copy'n'pasted layout name collection from InsertBookmarkAsPage
    //
    sal_uInt32 nPos, nEndPos;

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
            bool    bIsMasterPage;
            const sal_uInt32 nBMPage(pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage ));

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
                const sal_uInt32 nBMSdPage = (nBMPage - 1) / 2;
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
                                               List* pBookmarkList, sal_uInt32 nBMSdPageCount ) :
        mpLayoutsToTransfer(pLayoutsToTransfer), mpBookmarkDoc(pBookmarkDoc),
        mpBookmarkList(pBookmarkList), mnBMSdPageCount(nBMSdPageCount) {}
    virtual ~InsertBookmarkAsPage_FindDuplicateLayouts() {};
    virtual void operator()( SdDrawDocument&, SdPage* );
private:
    List*           mpLayoutsToTransfer;
    SdDrawDocument* mpBookmarkDoc;
    List*           mpBookmarkList;
    sal_uInt32      mnBMSdPageCount;
};

void InsertBookmarkAsPage_FindDuplicateLayouts::operator()( SdDrawDocument& rDoc, SdPage* pBMMPage )
{
    // now check for duplicate masterpage and layout names
    // ===================================================

    String  sFullLayoutName( pBMMPage->GetLayoutName() );
    String* pLayout = new String(sFullLayoutName);
    pLayout->Erase( pLayout->SearchAscii( SD_LT_SEPARATOR ));

    String* pTest = (String*) mpLayoutsToTransfer->First();
    bool bFound = false;

    while (pTest && !bFound)    // found yet?
    {
        if (*pLayout == *pTest)
            bFound = true;
        else
            pTest = (String*)mpLayoutsToTransfer->Next();
    }

    const sal_uInt32 nMPageCount = rDoc.GetMasterPageCount();
    for (sal_uInt32 nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
    {
        /**************************************************************
         * Gibt es die Layouts schon im Dokument?
         **************************************************************/
        SdPage* pTestPage = (SdPage*) rDoc.GetMasterPage(nMPage);
        String aTest(pTestPage->GetLayoutName());
        aTest.Erase( aTest.SearchAscii( SD_LT_SEPARATOR ));

        if (aTest == *pLayout)
            bFound = true;
    }

    if (!bFound)
        mpLayoutsToTransfer->Insert(pLayout, LIST_APPEND);
    else
        delete pLayout;
}

/** Just add one page to the container given to the constructor.
*/
class InsertBookmarkAsPage_AddBookmarkedPages
    : public SdDrawDocument::InsertBookmarkAsPage_PageFunctorBase
{
public:
    InsertBookmarkAsPage_AddBookmarkedPages(::std::vector<SdPage*>& rContainer)
        : mrContainer(rContainer) {}
    ~InsertBookmarkAsPage_AddBookmarkedPages(void) {}
    void operator() (SdDrawDocument&, SdPage* pPage) { mrContainer.push_back(pPage); }
private:
    ::std::vector<SdPage*>& mrContainer;
};


bool SdDrawDocument::InsertBookmarkAsPage(
    List* pBookmarkList,
    List* pExchangeList,            // Liste der zu verwendenen Namen
    bool bLink,
    bool bReplace,
    sal_uInt32 nInsertPos,
    bool bNoDialogs,
    ::sd::DrawDocShell* pBookmarkDocSh,
    bool bCopy,
    bool bMergeMasterPages,
    bool bPreservePageNames)
{
    bool bOK = true;
    bool bContinue = true;
    bool bScaleObjects = false;
    sal_uInt32 nReplacedStandardPages = 0;

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
        return false;
    }

    const sal_uInt32 nSdPageCount = GetSdPageCount(PK_STANDARD);
    const sal_uInt32 nBMSdPageCount = pBookmarkDoc->GetSdPageCount(PK_STANDARD);
    const sal_uInt32 nMPageCount = GetMasterPageCount();

    if (nSdPageCount==0 || nBMSdPageCount==0 || nMPageCount==0)
    {
        bContinue = bOK = false;
        return(bContinue);
    }

    // Store the size and some other properties of the first page and notes
    // page so that inserted pages can be properly scaled even when inserted
    // before the first page.
    // Note that the pointers are used later on as general page pointers.
    SdPage* pRefPage = GetSdPage(0, PK_STANDARD);
    const basegfx::B2DVector& rSize(pRefPage->GetPageScale());
    const double fLeft(pRefPage->GetLeftPageBorder());
    const double fRight(pRefPage->GetRightPageBorder());
    const double fUpper(pRefPage->GetTopPageBorder());
    const double fLower(pRefPage->GetBottomPageBorder());
    const Orientation eOrient(pRefPage->GetOrientation());

    const SdPage* pNPage = GetSdPage(0, PK_NOTES);
    const basegfx::B2DVector aNSize(GetSdPage(0, PK_NOTES)->GetPageScale());
    const double fNLeft(pNPage->GetLeftPageBorder());
    const double fNRight(pNPage->GetRightPageBorder());
    const double fNUpper(pNPage->GetTopPageBorder());
    const double fNLower(pNPage->GetBottomPageBorder());
    const Orientation eNOrient(pRefPage->GetOrientation());

    // Seitengroesse und -raender an die Werte der letzten
    // Seiten anpassen?
    pRefPage = GetSdPage(nSdPageCount - 1, PK_STANDARD);

    if( bNoDialogs )
    {
        if( !pBookmarkList )
            bScaleObjects = pRefPage->IsScaleObjects();
        else
            bScaleObjects = true;
    }
    else
    {
        SdPage* pBMPage = pBookmarkDoc->GetSdPage(0,PK_STANDARD);

        if (!pBMPage->GetPageScale().equal(pRefPage->GetPageScale())         ||
            pBMPage->GetLeftPageBorder()   != pRefPage->GetLeftPageBorder()  ||
            pBMPage->GetRightPageBorder()   != pRefPage->GetRightPageBorder()    ||
            pBMPage->GetTopPageBorder()   != pRefPage->GetTopPageBorder()    ||
            pBMPage->GetBottomPageBorder()   != pRefPage->GetBottomPageBorder())
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
        bMergeMasterPages = true;

    while (pLayout)
    {
        SdStyleSheetVector aCreatedStyles;

        ((SdStyleSheetPool*)GetStyleSheetPool())->CopyLayoutSheets(*pLayout, *pBookmarkStyleSheetPool,aCreatedStyles);

        if(!aCreatedStyles.empty())
        {
            if( pUndoMgr )
            {
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, true);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }

        delete pLayout;

        pLayout = (String*)pLayoutsToTransfer->Next();
    }

    delete pLayoutsToTransfer;

    /**************************************************************************
    * Dokument einfuegen
    **************************************************************************/

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(String(SdResId(STR_UNDO_INSERTPAGES)));

    if (!pBookmarkList)
    {
        if (nInsertPos >= GetPageCount())
        {
            // Seiten werden hinten angefuegt
            nInsertPos = GetPageCount();
        }

        sal_uInt32 nActualInsertPos = nInsertPos;

        List aNameList;
        std::set< sal_uInt32 > aRenameSet;
        sal_uInt32 nBMSdPage;

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pBMPage = pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD);
            String  pName( pBMPage->GetName() );
            bool    bIsMasterPage;

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
              SDRPAGE_NOTFOUND,  // Aber alle anderen
              nActualInsertPos,  // An Position einfuegen
              bMergeMasterPages, // MasterPages mitnehmen
              false,             // Aber nur die benoetigten MasterPages
              true,              // Undo-Aktion erzeugen
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
                // pPage->SetModel(this);
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
            bReplace = false;
            nInsertPos = GetPageCount();
        }

        sal_uInt32 nActualInsertPos = nInsertPos;

        // Collect the bookmarked pages.
        ::std::vector<SdPage*> aBookmarkedPages (pBookmarkList->Count(), NULL);
        for (sal_uInt32 nPos(0); nPos < pBookmarkList->Count(); nPos++)
        {
            String  aPgName(*(String*) pBookmarkList->GetObject(nPos));
            bool    bIsMasterPage;
            const sal_uInt32 nBMPage(pBookmarkDoc->GetPageByName( aPgName, bIsMasterPage ));

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                aBookmarkedPages[nPos] =  dynamic_cast<SdPage*>(pBookmarkDoc->GetPage(nBMPage));
            }
        }

        for (sal_uInt32 nPos(0); nPos < pBookmarkList->Count(); nPos++)
        {
            pBMPage = aBookmarkedPages[nPos];
            const sal_uInt32 nBMPage(pBMPage ? pBMPage->GetPageNumber() : SDRPAGE_NOTFOUND);

            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                /**************************************************************
                * Es muss eine StandardSeite sein
                **************************************************************/
                sal_Bool bMustRename = sal_False;

                // #95991# delay renaming *after* pages are copied (might destroy source otherwise)
                // #67905# don't change name if source and dest model are the same!
                // #96029# avoid renaming if replacing the same page
                String  aPgName(*(String*) pBookmarkList->GetObject(nPos));
                bool bIsMasterPage(false);
                const sal_uInt32 nPageSameName(GetPageByName(aPgName, bIsMasterPage));

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
                      false,             // Aber nur die benoetigten MasterPages
                      true,              // Undo-Aktion erzeugen
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
                    // pPage->SetModel(this);
                }

                if (bReplace)
                {
                    // Seite & Notizseite ausfuegen
                    const sal_uInt32 nDestPageNum(nActualInsertPos + 2);
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
                            // #96029# Take old slide names for inserted pages
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
    sal_uInt32 nNewMPageCount = GetMasterPageCount();

    // rueckwaerts, damit Nummern nicht durcheinander geraten
    for (sal_uInt32 nPage = nNewMPageCount - 1; nPage >= nMPageCount; nPage--)
    {
        pRefPage = (SdPage*) GetMasterPage(nPage);
        String aMPLayout(pRefPage->GetLayoutName());
        PageKind eKind = pRefPage->GetPageKind();

        // gibt's den schon?
        for (sal_uInt32 nTest = 0; nTest < nMPageCount; nTest++)
        {
            SdPage* pTest = (SdPage*) GetMasterPage(nTest);
            String aTest(pTest->GetLayoutName());

            // #96029# nInsertPos > 2 is always true when inserting into non-empty models
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

    // #96029# nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 0)
    {
        sal_uInt32 nSdPageStart = (nInsertPos - 1) / 2;
        sal_uInt32 nSdPageEnd = GetSdPageCount(PK_STANDARD) - nSdPageCount + nSdPageStart - 1;
        const bool bRemoveEmptyPresObj = pBookmarkDoc &&
                (pBookmarkDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) &&
                (GetDocumentType() == DOCUMENT_TYPE_DRAW);

        if( bReplace )
        {
            nSdPageEnd = nSdPageStart + nReplacedStandardPages - 1;
        }

        for (sal_uInt32 nSdPage = nSdPageStart; nSdPage <= nSdPageEnd; nSdPage++)
        {
            pRefPage = GetSdPage(nSdPage, PK_STANDARD);

            if (pExchangeList)
            {
                // Zuverwendener Name aus Exchange-Liste holen
                if (pExchangeList->GetCurObject())
                {
                    String aExchangeName (*(String*) pExchangeList->GetCurObject());
                    pRefPage->SetName(aExchangeName);
                    Broadcast(SdrBaseHint(*pRefPage, HINT_PAGEORDERCHG));
                    SdPage* pNewNotesPage = GetSdPage(nSdPage, PK_NOTES);
                    pNewNotesPage->SetName(aExchangeName);
                    Broadcast(SdrBaseHint(*pNewNotesPage, HINT_PAGEORDERCHG));
                }

                pExchangeList->Next();
            }

            String aLayout(pRefPage->GetLayoutName());
            aLayout.Erase(aLayout.SearchAscii( SD_LT_SEPARATOR ));

            // update layout and referred master page
            pRefPage->SetPresentationLayout(aLayout);
            if( bUndo )
                AddUndo( GetSdrUndoFactory().CreateUndoPageChangeMasterPage( *pRefPage ) );

            if (bScaleObjects)
            {
                pRefPage->ScaleObjects(rSize, fLeft, fUpper, fRight, fLower, true);
            }
            pRefPage->SetPageScale(rSize);
            pRefPage->SetPageBorder(fLeft, fUpper, fRight, fLower);
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
                pRefPage->ScaleObjects(aNSize, fNLeft, fNUpper, fNRight, fNLower, true);
            }

            pRefPage->SetPageScale(aNSize);
            pRefPage->SetPageBorder(fNLeft, fNUpper, fNRight, fNLower);
            pRefPage->SetOrientation( eNOrient );

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();
        }

        for (sal_uInt32 nPage = nMPageCount; nPage < nNewMPageCount; nPage++)
        {
            pRefPage = (SdPage*) GetMasterPage(nPage);
            if (pRefPage->GetPageKind() == PK_STANDARD)
            {
                if (bScaleObjects)
                {
                    pRefPage->ScaleObjects(rSize, fLeft, fUpper, fRight, fLower, true);
                }
                pRefPage->SetPageScale(rSize);
                pRefPage->SetPageBorder(fLeft, fUpper, fRight, fLower);
                pRefPage->SetOrientation( eOrient );
            }
            else        // kann nur noch NOTES sein
            {
                if (bScaleObjects)
                {
                    pRefPage->ScaleObjects(aNSize, fNLeft, fNUpper, fNRight, fNLower, true);
                }
                pRefPage->SetPageScale(aNSize);
                pRefPage->SetPageBorder(fNLeft, fNUpper, fNRight, fNLower);
                pRefPage->SetOrientation( eNOrient );
            }

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();
        }
    }

    // #91146# Make absolutely sure no double masterpages are there
    RemoveUnnecessaryMasterPages(NULL, true, true);

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

bool SdDrawDocument::InsertBookmarkAsObject(
    List* pBookmarkList,
    List* pExchangeList,            // Liste der zu verwendenen Namen
    bool /* bLink */,
    ::sd::DrawDocShell* pBookmarkDocSh,
    basegfx::B2DPoint* pObjPos)
{
    bool bOK = true;
    bool bOLEObjFound = false;
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
        return false;
    }

    if (!pBookmarkList)
    {
        pBMView = new ::sd::View(*pBookmarkDoc, (OutputDevice*) NULL);
        pBMView->EndListening(*pBookmarkDoc);
        pBMView->MarkAll();
    }
    else
    {
        SdrPage* pPage = 0;

        for (sal_uInt16 nPos = 0; nPos < pBookmarkList->Count(); nPos++)
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
                    bOLEObjFound = true;
                }

                if (!pBMView)
                {
                    // View erstmalig erzeugen
                    pBMView = new ::sd::View(*pBookmarkDoc, (OutputDevice*) NULL);
                    pBMView->EndListening(*pBookmarkDoc);
                }

                pPage = pObj->getSdrPageFromSdrObject();

                if(pPage)
                {
                    if (pPage->IsMasterPage())
                    {
                        pBMView->ShowSdrPage(*pBMView->getSdrModelFromSdrView().GetMasterPage(pPage->GetPageNumber()));
                    }
                    else
                    {
                        SdrPageView* pPV = pBMView->GetSdrPageView();

                        if( !pPV || (&pPV->getSdrPageFromSdrPageView() != pPage))
                        {
                            pBMView->ShowSdrPage(*pPage);
                        }
                    }
                }

                pBMView->MarkObj(*pObj, false);
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
                    pPage = &pPV->getSdrPageFromSdrPageView();
                }
                else if (pViewSh->GetActualPage())
                {
                    pPage = pViewSh->GetActualPage();
                }
            }
        }

        basegfx::B2DPoint aObjPos;

        if (pObjPos)
        {
            aObjPos = *pObjPos;
        }
        else
        {
            aObjPos = pPage->GetPageScale() * 0.5;
        }

        sal_uInt32 nCountBefore = 0;

        if (pExchangeList)
        {
            // OrdNums sortieren und Anzahl Objekte vor dem Einfuegen bestimmen
            nCountBefore = pPage->GetObjCount();
        }

        if (bOLEObjFound)
            pBMView->GetDoc()->SetAllocDocSh(true);

        SdDrawDocument* pTmpDoc = (SdDrawDocument*) pBMView->GetAllMarkedModel();
        bOK = pView->Paste(*pTmpDoc, aObjPos, pPage);

        if (bOLEObjFound)
            pBMView->GetDoc()->SetAllocDocSh(false);

        if (!bOLEObjFound)
            delete pTmpDoc;             // Wird ansonsten von der DocShell zerstoert

        delete pView;

        List* pList = pBookmarkList;

        if (pExchangeList)
        {
            // Anzahl Objekte nach dem Einfuegen bestimmen
            sal_uInt32 nCount = pPage->GetObjCount();

            for (sal_uInt32 nObj = nCountBefore; nObj < nCount; nObj++)
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
    return false;
}


/*************************************************************************
|*
|* In anschliessendem AllocModel() wird eine DocShell erzeugt
|* (xAllocedDocShRef). Eine bereits bestehende DocShell wird ggf. geloescht
|*
\************************************************************************/

void SdDrawDocument::SetAllocDocSh(bool bAlloc)
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

List* SdDrawDocument::GetCustomShowList(bool bCreate)
{
    if (!mpCustomShowList && bCreate)
    {
        // Liste erzeugen
        mpCustomShowList = new List();
    }

    return(mpCustomShowList);
}

/*************************************************************************
|*
|* Document-Stream herausgeben (fuer load-on-demand Graphiken)
|*
\************************************************************************/

SvStream* SdDrawDocument::GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const
{
    uno::Reference < embed::XStorage > xStor;
    if (mpDocSh)
        xStor = mpDocSh->GetStorage();
    SvStream*   pRet = NULL;

    if( xStor.is() )
    {
        //TODO/MBA: binary format removed, needs testing
        if( rStreamInfo.maUserData.Len() &&
            ( rStreamInfo.maUserData.GetToken( 0, ':' ) ==
              String( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package" ) ) ) )
        {
            const String aPicturePath( rStreamInfo.maUserData.GetToken( 1, ':' ) );

            // graphic from picture stream in picture storage in XML package
            if( aPicturePath.GetTokenCount( '/' ) == 2 ) try
            {
                const String aPictureStreamName( aPicturePath.GetToken( 1, '/' ) );
                const String aPictureStorageName( aPicturePath.GetToken( 0, '/' ) );
                if( xStor->isStorageElement( aPictureStorageName )  )
                {
                    uno::Reference < embed::XStorage > xPictureStorage =
                            xStor->openStorageElement( aPictureStorageName, embed::ElementModes::READ );
                    try
                    {
                        if( xPictureStorage.is() && xPictureStorage->isStreamElement( aPictureStreamName ) )
                        {
                            uno::Reference < io::XStream > xStream = xPictureStorage->openStreamElement( aPictureStreamName, embed::ElementModes::READ );
                            if( xStream.is() )
                                pRet = ::utl::UcbStreamHelper::CreateStream( xStream );
                        }
                    }
                    catch( container::NoSuchElementException& )
                    {
                    }
                }
            }
            catch( uno::Exception& e )
            {
                (void)e;
                DBG_ERROR(
                    (rtl::OString("sd::SdDrawDocument::GetDocumentStream(), "
                            "exception caught: ") +
                    rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ) +
                        rtl::OString("\r\nATTENTION: Graphics may get lost now, please inform CL or KA!") ).getStr() );
            }

            rStreamInfo.mbDeleteAfterUse = ( pRet != NULL );
        }
    }

#if OSL_DEBUG_LEVEL > 1
    if( pRet )
    {
        // try to get some information from stream
        const sal_uInt32 nStartPos = pRet->Tell();
        const sal_uInt32 nEndPos = pRet->Seek( STREAM_SEEK_TO_END );
        const sal_uInt32 nStmLen = nEndPos - nStartPos;
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
|* Nicht benutzte MasterPages und Layouts entfernen
|*
\************************************************************************/

void SdDrawDocument::RemoveUnnecessaryMasterPages(SdPage* pMasterPage, bool bOnlyDuplicatePages, bool bUndo)
{
    ::sd::View* pView = NULL;
    ::svl::IUndoManager* pUndoMgr = NULL;

    if( bUndo && !IsUndoEnabled() )
        bUndo = false;

    if (mpDocSh)
    {
        pUndoMgr = mpDocSh->GetUndoManager();

        if (mpDocSh->GetViewShell())
            pView = mpDocSh->GetViewShell()->GetView();
    }

    /***********************************************************
    * Alle MasterPages pruefen
    ***********************************************************/
    sal_uInt32 nSdMasterPageCount = GetMasterSdPageCount( PK_STANDARD );
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
            for ( sal_uInt32 nMPg = 0; nMPg < GetMasterPageCount(); nMPg++ )
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
            bool bDeleteMaster = !pMaster->IsPrecious();
            String aLayoutName = pMaster->GetLayoutName();

            if(bOnlyDuplicatePages )
            {
                // remove only duplicate pages
                bDeleteMaster = false;
                for (sal_uInt32 i = 0; i < GetMasterSdPageCount( PK_STANDARD ); i++)
                {
                    SdPage* pMPg = (SdPage*) GetMasterSdPage( i, PK_STANDARD );
                    if( pMPg != pMaster &&
                        pMPg->GetLayoutName() == aLayoutName )
                    {
                        // duplicate page found -> remove it
                        bDeleteMaster = true;
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
                        SdrPage& rShownPage = pPgView->getSdrPageFromSdrPageView();

                        if( (&rShownPage == pMaster) || (&rShownPage == pNotesMaster) )
                        {
                            pView->ShowSdrPage( *GetSdPage( 0, PK_STANDARD ) );
                        }
                    }
                }

                if( bUndo )
                {
                    BegUndo();
                    AddUndo( GetSdrUndoFactory().CreateUndoDeletePage( *pNotesMaster ) );
                }

                RemoveMasterPage( pNotesMaster->GetPageNumber() );

                if( !bUndo )
                    delete pNotesMaster;

                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pMaster));

                RemoveMasterPage( pMaster->GetPageNumber() );

                if( !bUndo )
                    delete pMaster;

                if( bUndo )
                    EndUndo();  // schon hier, damit sich Joes Actions ZWISCHEN unsere eigenen schieben

                // alte Layoutvorlagen loeschen, wenn sie nicht mehr benoetigt werden
                bool bDeleteOldStyleSheets = true;
                for ( sal_uInt32 nMPg = 0;
                          nMPg < GetMasterPageCount() && bDeleteOldStyleSheets;
                       nMPg++ )
                {
                    SdPage* pMPg = (SdPage*) GetMasterPage(nMPg);
                    if (pMPg->GetLayoutName() == aLayoutName)
                    {
                        bDeleteOldStyleSheets = false;
                    }
                }

                if (bDeleteOldStyleSheets)
                {
                    SdStyleSheetVector aRemove;
                    GetSdStyleSheetPool()->CreateLayoutSheetList( aLayoutName, aRemove );

                    if( bUndo )
                    {
                        // die Liste gehoert der UndoAction
                        SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, aRemove, false );

                        if (pUndoMgr)
                            pUndoMgr->AddUndoAction(pMovStyles);
                    }

                    for( SdStyleSheetVector::iterator iter = aRemove.begin(); iter != aRemove.end(); iter++ )
                        GetSdStyleSheetPool()->Remove((*iter).get());
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

// #121863# factored out functionality
bool isMasterPageLayoutNameUnique(const SdDrawDocument& rDoc, const String& rCandidate)
{
    if(!rCandidate.Len())
    {
        return false;
    }

    const sal_uInt32 nPageCount(rDoc.GetMasterPageCount());

    for(sal_uInt32 a(0); a < nPageCount; a++)
    {
        const SdrPage* pCandidate = rDoc.GetMasterPage(a);
        String aPageLayoutName(pCandidate->GetLayoutName());
        aPageLayoutName.Erase(aPageLayoutName.SearchAscii(SD_LT_SEPARATOR));

        if(aPageLayoutName == rCandidate)
        {
            return false;
        }
    }

    return true;
}

// #121863# factored out functinality
String createNewMasterPageLayoutName(const SdDrawDocument& rDoc)
{
    const String aBaseName(SdResId(STR_LAYOUT_DEFAULT_NAME));
    String aRetval;
    sal_uInt16 nCount(0);

    while(!aRetval.Len())
    {
        aRetval = aBaseName;

        if(nCount)
        {
            aRetval += String::CreateFromInt32(nCount);
        }

        nCount++;

        if(!isMasterPageLayoutNameUnique(rDoc, aRetval))
        {
            aRetval.Erase();
        }
    }

    return aRetval;
}

void SdDrawDocument::SetMasterPage(
    sal_uInt32 nSdPageNum,
    const String& rLayoutName,
    SdDrawDocument* pSourceDoc,
    bool bMaster,
    bool bCheckMasters)
{
    if( mpDocSh )
        mpDocSh->SetWaitCursor( true );

    ::svl::IUndoManager* pUndoMgr = mpDocSh->GetUndoManager();

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        pUndoMgr->EnterListAction(String(SdResId(STR_UNDO_SET_PRESLAYOUT)), String());
    }

    SdPage* pSelectedPage   = GetSdPage(nSdPageNum, PK_STANDARD);
    SdPage* pNotes          = (SdPage*) GetPage(pSelectedPage->GetPageNumber()+1);
    SdPage& rOldMaster      = (SdPage&)pSelectedPage->TRG_GetMasterPage();
    SdPage& rOldNotesMaster = (SdPage&)pNotes->TRG_GetMasterPage();
    SdPage* pMaster         = NULL;
    SdPage* pNotesMaster    = NULL;
    SdPage* pPage           = NULL;
    String aOldPageLayoutName(pSelectedPage->GetLayoutName());
    String aOldLayoutName(aOldPageLayoutName);
    aOldLayoutName.Erase(aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ));

    if (pSourceDoc)
    {
        List* pReplList = NULL;
        bool bLayoutReloaded = false;   // Wurde ex. Layout wieder geladen?

        /*********************************************************************
        |* LayoutName, Page and Notespage
        \*********************************************************************/
        if (rLayoutName.Len() == 0)
        {
            // No LayoutName: take first MasterPage
            pMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_STANDARD);
            pNotesMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_NOTES);
        }
        else
        {
            String aSearchFor(rLayoutName);
            aSearchFor.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
            aSearchFor.Append( String(SdResId(STR_LAYOUT_OUTLINE))) ;

            for (sal_uInt32 nMP = 0; nMP < pSourceDoc->GetMasterPageCount(); nMP++)
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
            }
        }

        // we should never reach this, but one never knows....
        if( (pMaster == NULL) || (pNotesMaster == NULL) )
        {
            pUndoMgr->LeaveListAction();

            if( mpDocSh )
                mpDocSh->SetWaitCursor( false );

            DBG_ERROR( "SdDrawDocument::SetMasterPage() failed!" );

            return;
        }

        const String aOriginalNewLayoutName( pMaster->GetName() );
        String aTargetNewLayoutName(aOriginalNewLayoutName);

        if (pSourceDoc != this)
        {
            // #121863# clone masterpages, they are from another model (!)
            SdPage* pNewNotesMaster = dynamic_cast< SdPage* >(pNotesMaster->CloneSdrPage(this));
            SdPage* pNewMaster = dynamic_cast< SdPage* >(pMaster->CloneSdrPage(this));

            if(!pNewNotesMaster || !pNewMaster)
            {
                delete pNewNotesMaster;
                delete pNewMaster;
                OSL_ASSERT("SdDrawDocument::SetMasterPage() cloning of MasterPage/NoteAmsterPage failed!" );
                return;
            }

            pNotesMaster = pNewNotesMaster;
            pMaster = pNewMaster;

            // layout name needs to be unique
            aTargetNewLayoutName = pMaster->GetLayoutName();
            aTargetNewLayoutName.Erase(aTargetNewLayoutName.SearchAscii(SD_LT_SEPARATOR));

            if(!isMasterPageLayoutNameUnique(*this, aTargetNewLayoutName))
            {
                aTargetNewLayoutName = createNewMasterPageLayoutName(*this);

                String aTemp(aTargetNewLayoutName);
                aTemp.AppendAscii(RTL_CONSTASCII_STRINGPARAM(SD_LT_SEPARATOR));
                aTemp.Append(String(SdResId(STR_LAYOUT_OUTLINE)));

                pMaster->SetName(aTargetNewLayoutName);
                pMaster->SetLayoutName(aTemp);

                pNotesMaster->SetName(aTargetNewLayoutName);
                pNotesMaster->SetLayoutName(aTemp);
            }
        }

        if (pSourceDoc != this)
        {
            const sal_uInt32 nMasterPageCount = GetMasterPageCount();
            for ( sal_uInt32 nMPage = 0; nMPage < nMasterPageCount; nMPage++ )
            {
                SdPage* pCheckMaster = (SdPage*)GetMasterPage(nMPage);
                if( pCheckMaster->GetName() == aTargetNewLayoutName )
                {
                    bLayoutReloaded = true;
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
            GetSdStyleSheetPool()->SetSearchMask(SD_STYLE_FAMILY_MASTERPAGE);

            pReplList = new List;           // Liste fuer ersetzte StyleSheets
            SdStyleSheetVector aCreatedStyles;          // Liste fuer erzeugte StyleSheets

            SfxStyleSheetBase* pHisSheet = pSourceStyleSheetPool->First();

            while (pHisSheet)
            {
                aName = pHisSheet->GetName();

                // #121863# search in source styles with original style name from source of
                // evtl. cloned master (not-cloned, renamed for uniqueness)
                if( aName.Search( aOriginalNewLayoutName ) == 0 )
                {
                    // #121863# build name of evtl. cloned master style to search for
                    if(aOriginalNewLayoutName != aTargetNewLayoutName)
                    {
                        const sal_uInt16 nPos(aName.SearchAscii(SD_LT_SEPARATOR));
                        aName.Erase(0, nPos);
                        aName.Insert(aTargetNewLayoutName, 0);
                    }

                    // TTTT: before or after upper replace?
                    SfxStyleSheet* pMySheet = static_cast<SfxStyleSheet*>(GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_MASTERPAGE) );

                    if (pMySheet)
                    {
                        // Es ist eine gleichnamige Vorlage vorhanden ist: Inhalte ersetzen
#ifdef DBG_UTIL
                        bool bTest =
#endif
                            pMySheet->SetName(pHisSheet->GetName());
                        DBG_ASSERT(bTest, "StyleSheet-Umbenennung fehlgeschlagen");
                        pMySheet->GetItemSet().ClearItem(0);  // alle loeschen

                        StyleSheetUndoAction* pUndoChStyle = new StyleSheetUndoAction(*this, *pMySheet, pHisSheet->GetItemSet());
                        pUndoMgr->AddUndoAction(pUndoChStyle);
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());
                        pMySheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    }
                    else
                    {
                        // create new style
                        String aHelpFile;
                        pMySheet = static_cast<SfxStyleSheet*>( &GetStyleSheetPool()->Make(aName, SD_STYLE_FAMILY_MASTERPAGE, pHisSheet->GetMask()) );
                        pMySheet->SetHelpId( aHelpFile, pHisSheet->GetHelpId(aHelpFile) );
                        pMySheet->GetItemSet().ClearItem(0);  // alle loeschen
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());

                        aCreatedStyles.push_back( SdStyleSheetRef( static_cast< SdStyleSheet* >( pMySheet ) ) );
                    }

                    StyleReplaceData* pReplData = new StyleReplaceData;
                    pReplData->nNewFamily = pMySheet->GetFamily();
                    pReplData->nFamily    = pMySheet->GetFamily();
                    pReplData->aNewName   = pMySheet->GetName();

                    // #121863# re-create original name of styte used at page where to replace with
                    // this new style
                    String aTemp(pMySheet->GetName());
                    const sal_uInt16 nPos(aTemp.SearchAscii(SD_LT_SEPARATOR));
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
            if(!aCreatedStyles.empty())
            {
                StyleReplaceData* pRData = (StyleReplaceData*)pReplList->First();

                while (pRData)
                {
                    SfxStyleSheetBase* pSOld = GetStyleSheetPool()->Find(pRData->aName);
                    SfxStyleSheetBase* pSNew = GetStyleSheetPool()->Find(pRData->aNewName);

                    if (pSOld && pSNew)
                    {
                        const String& rParentOfOld = pSOld->GetParent();
                        const String& rParentOfNew = pSNew->GetParent();

                        if (rParentOfOld.Len() > 0 && rParentOfNew.Len() == 0)
                        {

                            for (sal_uInt32 i = 0; i < pReplList->Count(); i++)
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
                GetStyleSheetPool()->SetSearchMask(SFX_STYLE_FAMILY_ALL);
            }

            if( !aCreatedStyles.empty() )
            {
                // UndoAction fuer das Erzeugen und Einfuegen vorn StyleSheets
                // auf den UndoManager legen
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, aCreatedStyles, true);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }

        // Layoutnamen auf Basis des Seitenlayoutnamens der Masterpage bilden
        String aPageLayoutName(pMaster->GetLayoutName());
        String aLayoutName = aPageLayoutName;
        aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ));

        // #121863# Do *not* remove from original document any longer, it is potentially used there
        // and would lead to crashes. Rely on the automatic process of removing unused masterpages
        // (see RemoveUnnecessaryMasterPages)
        //if (pSourceDoc != this)
        //{
        //  // Aus dem Source-Dokument austragen
        //  SdrPage* pTest = NULL;
        //  pTest = pSourceDoc->RemoveMasterPage(pNotesMaster->GetPageNumber());
        //  pTest = pSourceDoc->RemoveMasterPage(pMaster->GetPageNumber());
        //}

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
            sal_uInt32 nInsertPos = rOldMaster.GetPageNumber();
            BegUndo();

            if (!bLayoutReloaded)
                nInsertPos = 0xffffffff;
            InsertMasterPage(pMaster, nInsertPos);
            if( bUndo )
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

            nInsertPos++;
            if (!bLayoutReloaded)
                nInsertPos = 0xffffffff;
            InsertMasterPage(pNotesMaster, nInsertPos);
            if( bUndo )
            {
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pNotesMaster));

                EndUndo(); // schon hier, damit sich Joes Actions ZWISCHEN unsere eigenen schieben
            }
        }

        // Liste mit Seiten fuellen
        List* pPageList = new List;

//      #98456, this has to be removed according to CL (KA 07/08/2002)
//      #109884# but we need them again to restore the styles of the presentation objects while undo
        pPageList->Insert(pMaster, LIST_APPEND);
        pPageList->Insert(pNotesMaster, LIST_APPEND);

        if (bMaster || bLayoutReloaded)
        {
            for (sal_uInt32 nPage = 1; nPage < GetPageCount(); nPage++)
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

            if( bUndo )
            {
                SdPresentationLayoutUndoAction * pPLUndoAction =
                    new SdPresentationLayoutUndoAction
                        (this,
                        pPage->IsMasterPage() ? aLayoutName : aOldLayoutName,
                        aLayoutName,
                         eAutoLayout, eAutoLayout, false, pPage);
                pUndoMgr->AddUndoAction(pPLUndoAction);
            }
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
            basegfx::B2DVector aPageScale(rOldMaster.GetPageScale());
            pMaster->ScaleObjects(aPageScale, rOldMaster.GetLeftPageBorder(), rOldMaster.GetTopPageBorder(),
                                  rOldMaster.GetRightPageBorder(), rOldMaster.GetBottomPageBorder(), true);
            pMaster->SetPageScale(aPageScale);
            pMaster->SetPageBorder(rOldMaster.GetLeftPageBorder(),
                               rOldMaster.GetTopPageBorder(),
                               rOldMaster.GetRightPageBorder(),
                               rOldMaster.GetBottomPageBorder());
            pMaster->SetOrientation( rOldMaster.GetOrientation() );
            pMaster->SetAutoLayout(pMaster->GetAutoLayout());

            aPageScale = rOldNotesMaster.GetPageScale();
            pNotesMaster->ScaleObjects(aPageScale, rOldNotesMaster.GetLeftPageBorder(), rOldNotesMaster.GetTopPageBorder(),
                                       rOldNotesMaster.GetRightPageBorder(), rOldNotesMaster.GetBottomPageBorder(), true);
            pNotesMaster->SetPageScale(aPageScale);
            pNotesMaster->SetPageBorder(rOldNotesMaster.GetLeftPageBorder(),
                                    rOldNotesMaster.GetTopPageBorder(),
                                    rOldNotesMaster.GetRightPageBorder(),
                                    rOldNotesMaster.GetBottomPageBorder());
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
        String aName(createNewMasterPageLayoutName(*this)); // TTTT: Compare for security with orig
        String aPageLayoutName(aName);
        aPageLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
        aPageLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

        /*********************************************************************
        |* Neue StyleSheets erzeugen
        \********************************************************************/
        GetSdStyleSheetPool()->CreateLayoutStyleSheets(aName);
        SdStyleSheetVector aCreatedStyles;
        GetSdStyleSheetPool()->CreateLayoutSheetList(aName, aCreatedStyles);

        if( bUndo )
        {
            SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, true);
            pUndoMgr->AddUndoAction(pMovStyles);
        }

        /*********************************************************************
        |* Neue MasterPages erzeugen und ins Dokument eintragen
        \********************************************************************/

        if( bUndo )
            BegUndo();

        pMaster = (SdPage*) AllocPage(true);
        pMaster->SetPageScale(pSelectedPage->GetPageScale());
        pMaster->SetPageBorder(pSelectedPage->GetLeftPageBorder(),
                           pSelectedPage->GetTopPageBorder(),
                           pSelectedPage->GetRightPageBorder(),
                           pSelectedPage->GetBottomPageBorder() );
        pMaster->SetName(aName);
        pMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pMaster);

        if( bUndo )
            AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

        pMaster->SetAutoLayout(AUTOLAYOUT_NONE, true, true);

        pNotesMaster = (SdPage*) AllocPage(true);
        pNotesMaster->SetPageKind(PK_NOTES);
        pNotesMaster->SetPageScale(pNotes->GetPageScale());
        pNotesMaster->SetPageBorder(pNotes->GetLeftPageBorder(),
                                pNotes->GetTopPageBorder(),
                                pNotes->GetRightPageBorder(),
                                pNotes->GetBottomPageBorder() );
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
        List* pPageList = new List;
        if (bMaster)
        {
            for (sal_uInt32 nPage = 1; nPage < GetPageCount(); nPage++)
            {
                pPage = (SdPage*) GetPage(nPage);
                const String s(pPage->GetLayoutName());
                if(s == aOldPageLayoutName)
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

            if( bUndo )
            {
                SdPresentationLayoutUndoAction * pPLUndoAction =
                    new SdPresentationLayoutUndoAction
                            (this, aOldLayoutName, aName,
                             eOldAutoLayout, eNewAutoLayout, true,
                             pPage);
                pUndoMgr->AddUndoAction(pPLUndoAction);
            }

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
        mpDocSh->SetWaitCursor( false );
}



void SdDrawDocument::Merge(SdrModel& rSourceModel,
               sal_uInt32 nFirstPageNum, sal_uInt32 nLastPageNum,
               sal_uInt32 nDestPos,
               bool bMergeMasterPages, bool bAllMasterPages,
               bool bUndo, bool bTreadSourceAsConst)
{
    sal_uInt32 nMasterPageCount = GetMasterPageCount();
    SdrModel::Merge( rSourceModel, nFirstPageNum, nLastPageNum, nDestPos, bMergeMasterPages, bAllMasterPages, bUndo, bTreadSourceAsConst );

    // add style family for each new master page
    for( sal_uInt32 nMaster = nMasterPageCount; nMaster < GetMasterPageCount(); nMaster++ )
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
