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

#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <vcl/wrkwin.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#include "Outliner.hxx"
#include <editeng/paperinf.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/itempool.hxx>
#include <svx/svdpool.hxx>
#include <editeng/flditem.hxx>

#include <sfx2/linkmgr.hxx>
#include <editeng/editdata.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>                  // SVX_RESSTR

#include "eetext.hxx"
#include <svx/svditer.hxx>
#include <svtools/imapobj.hxx>


#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pglink.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "stlpool.hxx"
#include "sdiocmpt.hxx"
#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "cusshow.hxx"
#include "undo/undomanager.hxx"

#include "../ui/inc/DrawDocShell.hxx"
#include "../ui/inc/FrameView.hxx"
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"

#include "PageListWatcher.hxx"
#include <vcl/virdev.hxx>

using namespace ::sd;

const long PRINT_OFFSET = 30;       // siehe \svx\source\dialog\page.cxx (PB)

using namespace com::sun::star;

/*************************************************************************
|*
|* Sucht ein Objekt per Name
|*
\************************************************************************/

SdrObject* SdDrawDocument::GetObj(const String& rObjName) const
{
    SdrObject* pObj = NULL;
    SdrObject* pObjFound = NULL;
    SdPage* pPage = NULL;

    /**************************************************************************
    * Zuerst alle Pages durchsuchen
    **************************************************************************/
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = GetPageCount();

    while (nPage < nMaxPages && !pObjFound)
    {
        pPage = (SdPage*) GetPage(nPage);
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( pObj->GetName().equals(rObjName) ) ||
                ( SdrInventor == pObj->GetObjInventor() &&
                  OBJ_OLE2 == pObj->GetObjIdentifier() &&
                  rObjName == static_cast< SdrOle2Obj* >( pObj )->GetPersistName() ) )
            {
                pObjFound = pObj;
            }
        }

        nPage++;
    }

    /**************************************************************************
    * Wenn nicht gefunden, dann alle MasterPages durchsuchen
    **************************************************************************/
    nPage = 0;
    const sal_uInt16 nMaxMasterPages = GetMasterPageCount();

    while (nPage < nMaxMasterPages && !pObjFound)
    {
        pPage = (SdPage*) GetMasterPage(nPage);
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( pObj->GetName().equals(rObjName) ) ||
                ( SdrInventor == pObj->GetObjInventor() &&
                  OBJ_OLE2 == pObj->GetObjIdentifier() &&
                  rObjName == static_cast< SdrOle2Obj* >( pObj )->GetPersistName() ) )
            {
                pObjFound = pObj;
            }
        }

        nPage++;
    }

    return (pObjFound);
}


/*************************************************************************
|*
|* Sucht die SdPage per Name
|*
\************************************************************************/

sal_uInt16 SdDrawDocument::GetPageByName(const String& rPgName, sal_Bool& rbIsMasterPage) const
{
    SdPage* pPage = NULL;
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = GetPageCount();
    sal_uInt16 nPageNum = SDRPAGE_NOTFOUND;

    rbIsMasterPage = sal_False;

    // Search all regular pages and all notes pages (handout pages are
    // ignored.)
    while (nPage < nMaxPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = const_cast<SdPage*>(static_cast<const SdPage*>(
            GetPage(nPage)));

        if (pPage != NULL
            && pPage->GetPageKind() != PK_HANDOUT
            && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
        }

        nPage++;
    }

    // Search all master pages when not found among non-master pages.
    const sal_uInt16 nMaxMasterPages = GetMasterPageCount();
    nPage = 0;

    while (nPage < nMaxMasterPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = const_cast<SdPage*>(static_cast<const SdPage*>(
            GetMasterPage(nPage)));

        if (pPage && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
            rbIsMasterPage = sal_True;
        }

        nPage++;
    }

    return nPageNum;
}

SdPage* SdDrawDocument::GetSdPage(sal_uInt16 nPgNum, PageKind ePgKind) const
{
    return mpDrawPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

sal_uInt16 SdDrawDocument::GetSdPageCount(PageKind ePgKind) const
{
    return (sal_uInt16)mpDrawPageListWatcher->GetSdPageCount(ePgKind);
}

SdPage* SdDrawDocument::GetMasterSdPage(sal_uInt16 nPgNum, PageKind ePgKind)
{
    return mpMasterPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

sal_uInt16 SdDrawDocument::GetMasterSdPageCount(PageKind ePgKind) const
{
    return (sal_uInt16)mpMasterPageListWatcher->GetSdPageCount(ePgKind);
}

/*************************************************************************
|*
|*  die in den Seitenobjekten der Notizseiten eingetragenen
|*  Seitennummern anpassen
|*
\************************************************************************/

void SdDrawDocument::UpdatePageObjectsInNotes(sal_uInt16 nStartPos)
{
    sal_uInt16  nPageCount  = GetPageCount();
    SdPage* pPage       = NULL;

    for (sal_uInt16 nPage = nStartPos; nPage < nPageCount; nPage++)
    {
        pPage = (SdPage*)GetPage(nPage);

        // wenn es eine Notizseite ist, Seitenobjekt suchen
        // und Nummer korrigieren
        if (pPage && pPage->GetPageKind() == PK_NOTES)
        {
            sal_uLong nObjCount = pPage->GetObjCount();
            SdrObject* pObj = NULL;
            for (sal_uLong nObj = 0; nObj < nObjCount; nObj++)
            {
                pObj = pPage->GetObj(nObj);
                if (pObj->GetObjIdentifier() == OBJ_PAGE &&
                    pObj->GetObjInventor() == SdrInventor)
                {
                    // das Seitenobjekt stellt die vorhergende Seite (also
                    // die Zeichenseite) dar
                    DBG_ASSERTWARNING(nStartPos, "Notizseitenpos. darf nicht 0 sein");

                    DBG_ASSERTWARNING(nPage > 1, "Seitenobjekt darf nicht Handzettel darstellen");

                    if (nStartPos > 0 && nPage > 1)
                        ((SdrPageObj*)pObj)->SetReferencedPage(GetPage(nPage - 1));
                }
            }
        }
    }
}

void SdDrawDocument::UpdatePageRelativeURLs(const String& rOldName, const String& rNewName)
{
    if (rNewName.Len() == 0)
        return;

    SfxItemPool& pPool(GetPool());
    sal_uInt32 nCount = pPool.GetItemCount2(EE_FEATURE_FIELD);
    for (sal_uInt32 nOff = 0; nOff < nCount; nOff++)
    {
        const SfxPoolItem *pItem = pPool.GetItem2(EE_FEATURE_FIELD, nOff);
        const SvxFieldItem* pFldItem = dynamic_cast< const SvxFieldItem * > (pItem);

        if(pFldItem)
        {
            SvxURLField* pURLField = const_cast< SvxURLField* >( dynamic_cast<const SvxURLField*>( pFldItem->GetField() ) );

            if(pURLField)
            {
                XubString aURL = pURLField->GetURL();

                if (aURL.Len() && (aURL.GetChar(0) == 35) && (aURL.Search(rOldName, 1) == 1))
                {
                    if (aURL.Len() == rOldName.Len() + 1) // standard page name
                    {
                        aURL.Erase (1, aURL.Len() - 1);
                        aURL += rNewName;
                        pURLField->SetURL(aURL);
                    }
                    else
                    {
                        const XubString sNotes = SdResId(STR_NOTES);
                        if (aURL.Len() == rOldName.Len() + 2 + sNotes.Len() && aURL.Search(sNotes, rOldName.Len() + 2) == rOldName.Len() + 2)
                        {
                            aURL.Erase (1, aURL.Len() - 1);
                            aURL += rNewName;
                            aURL += ' ';
                            aURL += sNotes;
                            pURLField->SetURL(aURL);
                        }
                    }
                }
            }
        }
    }
}

void SdDrawDocument::UpdatePageRelativeURLs(SdPage* pPage, sal_uInt16 nPos, sal_Int32 nIncrement)
{
    bool bNotes = (pPage->GetPageKind() == PK_NOTES);

    SfxItemPool& pPool(GetPool());
    sal_uInt32 nCount = pPool.GetItemCount2(EE_FEATURE_FIELD);
    for (sal_uInt32 nOff = 0; nOff < nCount; nOff++)
    {
        const SfxPoolItem *pItem = pPool.GetItem2(EE_FEATURE_FIELD, nOff);
        const SvxFieldItem* pFldItem;

        if ((pFldItem = dynamic_cast< const SvxFieldItem * > (pItem)) != 0)
        {
            SvxURLField* pURLField = const_cast< SvxURLField* >( dynamic_cast<const SvxURLField*>( pFldItem->GetField() ) );

            if(pURLField)
            {
                XubString aURL = pURLField->GetURL();

                if (aURL.Len() && (aURL.GetChar(0) == 35))
                {
                    XubString aHashSlide = rtl::OUString('#');
                    aHashSlide += SD_RESSTR(STR_PAGE);

                    if (aURL.CompareTo(aHashSlide, aHashSlide.Len()) == COMPARE_EQUAL)
                    {
                        XubString aURLCopy = aURL;
                        const XubString sNotes = SdResId(STR_NOTES);

                        aURLCopy.Erase(0, aHashSlide.Len());

                        bool bNotesLink = (aURLCopy.Len() >= sNotes.Len() + 3 && aURLCopy.Search(sNotes, aURLCopy.Len() - sNotes.Len()) == aURLCopy.Len() - sNotes.Len());

                        if (bNotesLink ^ bNotes)
                            continue; // no compatible link and page

                        if (bNotes)
                            aURLCopy.Erase(aURLCopy.Len() - sNotes.Len(), sNotes.Len());

                        sal_Int32 number = aURLCopy.ToInt32();
                        sal_uInt16 realPageNumber = (nPos + 1)/ 2;

                        if ( number >= realPageNumber )
                        {
                            // update link page number
                            number += nIncrement;
                            aURL.Erase (aHashSlide.Len() + 1, aURL.Len() - aHashSlide.Len() - 1);
                            aURL += XubString::CreateFromInt32(number);
                            if (bNotes)
                            {
                                aURL += ' ';
                                aURL += sNotes;
                            }
                            pURLField->SetURL(aURL);
                        }
                    }
                }
            }
        }
    }
}

/*************************************************************************
|*
|*  Seite verschieben
|*
\************************************************************************/

void SdDrawDocument::MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    // Seite verschieben
    FmFormModel::MovePage(nPgNum, nNewPos);

    sal_uInt16 nMin = Min(nPgNum, nNewPos);

    UpdatePageObjectsInNotes(nMin);
}

/*************************************************************************
|*
|*  Seite einfuegen
|*
\************************************************************************/

void SdDrawDocument::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    bool bLast = (nPos == GetPageCount());

    FmFormModel::InsertPage(pPage, nPos);

    ((SdPage*)pPage)->ConnectLink();

    UpdatePageObjectsInNotes(nPos);

    if (!bLast)
        UpdatePageRelativeURLs(static_cast<SdPage*>( pPage ), nPos, 1);

}

/*************************************************************************
|*
|*  Seite loeschen
|*
\************************************************************************/

void SdDrawDocument::DeletePage(sal_uInt16 nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}

/*************************************************************************
|*
|*  Seite entfernen
|*
\************************************************************************/

SdrPage* SdDrawDocument::RemovePage(sal_uInt16 nPgNum)
{
    SdrPage* pPage = FmFormModel::RemovePage(nPgNum);

    bool bLast = ((nPgNum+1)/2 == (GetPageCount()+1)/2);

    ((SdPage*)pPage)->DisconnectLink();
    ReplacePageInCustomShows( dynamic_cast< SdPage* >( pPage ), 0 );
    UpdatePageObjectsInNotes(nPgNum);

    if (!bLast)
        UpdatePageRelativeURLs((SdPage*)pPage, nPgNum, -1);

    return pPage;
}

// Warning: This is not called for new master pages created from SdrModel::Merge,
// you also have to modify code in SdDrawDocument::Merge!
void SdDrawDocument::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos )
{
    FmFormModel::InsertMasterPage( pPage, nPos );
    if( pPage && pPage->IsMasterPage() && (static_cast<SdPage*>(pPage)->GetPageKind() == PK_STANDARD) )
    {
        // new master page created, add its style family
        SdStyleSheetPool* pStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        if( pStylePool )
            pStylePool->AddStyleFamily( static_cast<SdPage*>(pPage) );
    }
}

SdrPage* SdDrawDocument::RemoveMasterPage(sal_uInt16 nPgNum)
{
    SdPage* pPage = static_cast<SdPage*>(GetMasterPage(nPgNum ));
    if( pPage && pPage->IsMasterPage() && (pPage->GetPageKind() == PK_STANDARD) )
    {
        // master page removed, remove its style family
        SdStyleSheetPool* pStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        if( pStylePool )
            pStylePool->RemoveStyleFamily( pPage );
    }

    return FmFormModel::RemoveMasterPage(nPgNum);
}

/*************************************************************************
|*
|* Seiten selektieren
|*
\************************************************************************/

void SdDrawDocument::SetSelected(SdPage* pPage, sal_Bool bSelect)
{
    PageKind ePageKind = pPage->GetPageKind();

    if (ePageKind == PK_STANDARD)
    {
        pPage->SetSelected(bSelect);

        const sal_uInt16 nDestPageNum(pPage->GetPageNum() + 1);
        SdPage* pNotesPage = 0L;

        if(nDestPageNum < GetPageCount())
        {
            pNotesPage = (SdPage*)GetPage(nDestPageNum);
        }

        if (pNotesPage && pNotesPage->GetPageKind() == PK_NOTES)
        {
            pNotesPage->SetSelected(bSelect);
        }
    }
    else if (ePageKind == PK_NOTES)
    {
        pPage->SetSelected(bSelect);
        SdPage* pStandardPage = (SdPage*) GetPage( pPage->GetPageNum() - 1 );

        if (pStandardPage && pStandardPage->GetPageKind() == PK_STANDARD)
            pStandardPage->SetSelected(bSelect);
    }
}

/*************************************************************************
|*
|* Sofern noch keine Seiten vorhanden sind, werden nun Seiten erzeugt
|*
\************************************************************************/

void SdDrawDocument::CreateFirstPages( SdDrawDocument* pRefDocument /* = 0 */ )
{
    /**************************************************************************
    * Wenn noch keine Seite im Model vorhanden ist (Datei-Neu), wird
    * eine neue Seite eingefuegt
    **************************************************************************/
    sal_uInt16 nPageCount = GetPageCount();

    if (nPageCount <= 1)
    {
        // #i57181# Paper size depends on Language, like in Writer
        Size aDefSize = SvxPaperInfo::GetDefaultPaperSize( MAP_100TH_MM );

        /**********************************************************************
        * Handzettel-Seite einfuegen
        **********************************************************************/
        SdPage* pHandoutPage = dynamic_cast< SdPage* >( AllocPage(sal_False) );

        SdPage* pRefPage = NULL;

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_HANDOUT );

        if( pRefPage )
        {
            pHandoutPage->SetSize(pRefPage->GetSize());
            pHandoutPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
        }
        else
        {
            pHandoutPage->SetSize(aDefSize);
            pHandoutPage->SetBorder(0, 0, 0, 0);
        }

        pHandoutPage->SetPageKind(PK_HANDOUT);
        pHandoutPage->SetName( String (SdResId(STR_HANDOUT) ) );
        InsertPage(pHandoutPage, 0);

        /**********************************************************************
        * MasterPage einfuegen und an der Handzettel-Seite vermerken
        **********************************************************************/
        SdPage* pHandoutMPage = (SdPage*) AllocPage(sal_True);
        pHandoutMPage->SetSize( pHandoutPage->GetSize() );
        pHandoutMPage->SetPageKind(PK_HANDOUT);
        pHandoutMPage->SetBorder( pHandoutPage->GetLftBorder(),
                                  pHandoutPage->GetUppBorder(),
                                  pHandoutPage->GetRgtBorder(),
                                  pHandoutPage->GetLwrBorder() );
        InsertMasterPage(pHandoutMPage, 0);
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

        /**********************************************************************
        * Seite einfuegen
        * Sofern nPageCount==1 ist, wurde das Model fuers Clipboad erzeugt.
        * Eine Standard-Seite ist daher schon vorhanden.
        **********************************************************************/
        SdPage* pPage;
        sal_Bool bClipboard = sal_False;

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_STANDARD );

        if (nPageCount == 0)
        {
            pPage = dynamic_cast< SdPage* >( AllocPage(sal_False) );

            if( pRefPage )
            {
                pPage->SetSize( pRefPage->GetSize() );
                pPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
            }
            else if (meDocType == DOCUMENT_TYPE_DRAW)
            {
                // Draw: stets Default-Groesse mit Raendern
                pPage->SetSize(aDefSize);

                SfxPrinter* pPrinter = mpDocSh->GetPrinter(sal_False);
                if (pPrinter && pPrinter->IsValid())
                {
                    Size aOutSize(pPrinter->GetOutputSize());
                    Point aPageOffset(pPrinter->GetPageOffset());
                    aPageOffset -= pPrinter->PixelToLogic( Point() );
                    long nOffset = !aPageOffset.X() && !aPageOffset.X() ? 0 : PRINT_OFFSET;

                    sal_uLong nTop    = aPageOffset.Y();
                    sal_uLong nLeft   = aPageOffset.X();
                    sal_uLong nBottom = Max((long)(aDefSize.Height() - aOutSize.Height() - nTop + nOffset), 0L);
                    sal_uLong nRight  = Max((long)(aDefSize.Width() - aOutSize.Width() - nLeft + nOffset), 0L);

                    pPage->SetBorder(nLeft, nTop, nRight, nBottom);
                }
                else
                {
                    // The printer is not available.  Use a border of 10mm
                    // on each side instead.
                    // This has to be kept synchronized with the border
                    // width set in the
                    // SvxPageDescPage::PaperSizeSelect_Impl callback.
                    pPage->SetBorder(1000, 1000, 1000, 1000);
                }
            }
            else
            {
                // Impress: stets Bildschirmformat, quer
                Size aSz( SvxPaperInfo::GetPaperSize(PAPER_SCREEN_4_3, MAP_100TH_MM) );
                pPage->SetSize( Size( aSz.Height(), aSz.Width() ) );
                pPage->SetBorder(0, 0, 0, 0);
            }

            InsertPage(pPage, 1);
        }
        else
        {
            bClipboard = sal_True;
            pPage = (SdPage*) GetPage(1);
        }

        /**********************************************************************
        * MasterPage einfuegen und an der Seite vermerken
        **********************************************************************/
        SdPage* pMPage = (SdPage*) AllocPage(sal_True);
        pMPage->SetSize( pPage->GetSize() );
        pMPage->SetBorder( pPage->GetLftBorder(),
                           pPage->GetUppBorder(),
                           pPage->GetRgtBorder(),
                           pPage->GetLwrBorder() );
        InsertMasterPage(pMPage, 1);
        pPage->TRG_SetMasterPage( *pMPage );
        if( bClipboard )
            pMPage->SetLayoutName( pPage->GetLayoutName() );

        /**********************************************************************
        * Notizen-Seite einfuegen
        **********************************************************************/
        SdPage* pNotesPage = (SdPage*) AllocPage(sal_False);

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_NOTES );

        if( pRefPage )
        {
            pNotesPage->SetSize( pRefPage->GetSize() );
            pNotesPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
        }
        else
        {
            // Stets Hochformat
            if (aDefSize.Height() >= aDefSize.Width())
            {
                pNotesPage->SetSize(aDefSize);
            }
            else
            {
                pNotesPage->SetSize( Size(aDefSize.Height(), aDefSize.Width()) );
            }

            pNotesPage->SetBorder(0, 0, 0, 0);
        }
        pNotesPage->SetPageKind(PK_NOTES);
        InsertPage(pNotesPage, 2);
        if( bClipboard )
            pNotesPage->SetLayoutName( pPage->GetLayoutName() );

        /**********************************************************************
        * MasterPage einfuegen und an der Notizen-Seite vermerken
        **********************************************************************/
        SdPage* pNotesMPage = (SdPage*) AllocPage(sal_True);
        pNotesMPage->SetSize( pNotesPage->GetSize() );
        pNotesMPage->SetPageKind(PK_NOTES);
        pNotesMPage->SetBorder( pNotesPage->GetLftBorder(),
                                pNotesPage->GetUppBorder(),
                                pNotesPage->GetRgtBorder(),
                                pNotesPage->GetLwrBorder() );
        InsertMasterPage(pNotesMPage, 2);
        pNotesPage->TRG_SetMasterPage( *pNotesMPage );
        if( bClipboard )
            pNotesMPage->SetLayoutName( pPage->GetLayoutName() );


        if( !pRefPage && (meDocType != DOCUMENT_TYPE_DRAW) )
            pPage->SetAutoLayout( AUTOLAYOUT_TITLE, sal_True, sal_True );

        mpWorkStartupTimer = new Timer();
        mpWorkStartupTimer->SetTimeoutHdl( LINK(this, SdDrawDocument, WorkStartupHdl) );
        mpWorkStartupTimer->SetTimeout(2000);
        mpWorkStartupTimer->Start();

        SetChanged(sal_False);
    }
}

/*************************************************************************
|*
|* Erzeugt fehlende Notiz und Handzettelseiten (nach PowerPoint-Import)
|* Es wird davon ausgegangen, dass mindestens eine Standard-Seite und
|* eine Standard-MasterPage vorhanden sind.
|*
\************************************************************************/

sal_Bool SdDrawDocument::CreateMissingNotesAndHandoutPages()
{
    sal_Bool bOK = sal_False;
    sal_uInt16 nPageCount = GetPageCount();

    if (nPageCount != 0)
    {
        /**********************************************************************
        * PageKind setzen
        **********************************************************************/
        SdPage* pHandoutMPage = (SdPage*) GetMasterPage(0);
        pHandoutMPage->SetPageKind(PK_HANDOUT);

        SdPage* pHandoutPage = (SdPage*) GetPage(0);
        pHandoutPage->SetPageKind(PK_HANDOUT);
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

        for (sal_uInt16 i = 1; i < nPageCount; i = i + 2)
        {
            SdPage* pPage = (SdPage*) GetPage(i);

            if(!pPage->TRG_HasMasterPage())
            {
                // Keine MasterPage gesetzt -> erste Standard-MasterPage nehmen
                // (Wenn bei PPT keine Standard-Seite vorhanden war)
                pPage->TRG_SetMasterPage(*GetMasterPage(1));
            }

            SdPage* pNotesPage = (SdPage*) GetPage(i+1);
            pNotesPage->SetPageKind(PK_NOTES);

            // Notiz-MasterPages setzen
            sal_uInt16 nMasterPageAfterPagesMasterPage = (pPage->TRG_GetMasterPage()).GetPageNum() + 1;
            pNotesPage->TRG_SetMasterPage(*GetMasterPage(nMasterPageAfterPagesMasterPage));
        }

        bOK = sal_True;
        StopWorkStartupDelay();
        SetChanged(sal_False);
    }

    return(bOK);
}

/*************************************************************************
|*
|* - selektierte Seiten hinter genannte Seite schieben
|*   (nTargetPage = (sal_uInt16)-1  --> vor erste Seite schieben)
|* - ergibt sal_True, wenn Seiten verschoben wurden
|*
\************************************************************************/

sal_Bool SdDrawDocument::MovePages(sal_uInt16 nTargetPage)
{
    SdPage* pPage              = NULL;
    sal_uInt16  nPage;
    sal_uInt16  nNoOfPages         = GetSdPageCount(PK_STANDARD);
    sal_Bool    bSomethingHappened = sal_False;

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(String(SdResId(STR_UNDO_MOVEPAGES)));

    // Liste mit selektierten Seiten
    std::vector<SdPage*> aPageList;
    for (nPage = 0; nPage < nNoOfPages; nPage++)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);

        if (pPage->IsSelected())
            aPageList.push_back(pPage);
    }

    // falls noetig, nach vorne hangeln, bis nicht selektierte Seite gefunden
    nPage = nTargetPage;
    if (nPage != (sal_uInt16)-1)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);
        while (nPage > 0 && pPage->IsSelected())
        {
            nPage--;
            pPage = GetSdPage(nPage, PK_STANDARD);
        }

        if (pPage->IsSelected())
        {
            nPage = (sal_uInt16)-1;
        }
    }

    // vor der ersten Seite einfuegen
    if (nPage == (sal_uInt16)-1)
    {
        std::vector<SdPage*>::reverse_iterator iter;
        for (iter = aPageList.rbegin(); iter != aPageList.rend(); ++iter)
        {
            nPage = (*iter)->GetPageNum();
            if (nPage != 0)
            {
                SdrPage* pPg = GetPage(nPage);
                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage, 1));
                MovePage(nPage, 1);
                pPg = GetPage(nPage+1);
                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage+1, 2));
                MovePage(nPage+1, 2);
                bSomethingHappened = sal_True;
            }
        }
    }
    // hinter <nPage> einfuegen
    else
    {
        nTargetPage = nPage;
        nTargetPage = 2 * nTargetPage + 1;    // PK_STANDARD --> absolut

        std::vector<SdPage*>::iterator iter;
        for (iter = aPageList.begin(); iter != aPageList.end(); ++iter)
        {
            pPage = *iter;
            nPage = pPage->GetPageNum();
            if (nPage > nTargetPage)
            {
                nTargetPage += 2;        // hinter (!) der Seite einfuegen

                if (nPage != nTargetPage)
                {
                    SdrPage* pPg = GetPage(nPage);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage, nTargetPage));
                    MovePage(nPage, nTargetPage);
                    pPg = GetPage(nPage+1);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage+1, nTargetPage+1));
                    MovePage(nPage+1, nTargetPage+1);
                    bSomethingHappened = sal_True;
                }
            }
            else
            {
                if (nPage != nTargetPage)
                {
                    SdrPage* pPg = GetPage(nPage+1);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage+1, nTargetPage+1));
                    MovePage(nPage+1, nTargetPage+1);
                    pPg = GetPage(nPage);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage, nTargetPage));
                    MovePage(nPage, nTargetPage);
                    bSomethingHappened = sal_True;
                }
            }
            nTargetPage = pPage->GetPageNum();
        }
    }

    if( bUndo )
        EndUndo();

    return bSomethingHappened;
}


/*************************************************************************
|*
|* Anzahl der Links im sfx2::LinkManager zurueckgeben
|*
\************************************************************************/

sal_uLong SdDrawDocument::GetLinkCount()
{
    return pLinkManager->GetLinks().size();
}

/*************************************************************************
|*
|* Language setzen
|*
\************************************************************************/

void SdDrawDocument::SetLanguage( const LanguageType eLang, const sal_uInt16 nId )
{
    sal_Bool bChanged = sal_False;

    if( nId == EE_CHAR_LANGUAGE && meLanguage != eLang )
    {
        meLanguage = eLang;
        bChanged = sal_True;
    }
    else if( nId == EE_CHAR_LANGUAGE_CJK && meLanguageCJK != eLang )
    {
        meLanguageCJK = eLang;
        bChanged = sal_True;
    }
    else if( nId == EE_CHAR_LANGUAGE_CTL && meLanguageCTL != eLang )
    {
        meLanguageCTL = eLang;
        bChanged = sal_True;
    }

    if( bChanged )
    {
        GetDrawOutliner().SetDefaultLanguage( Application::GetSettings().GetLanguage() );
        pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguage() );
        pItemPool->SetPoolDefaultItem( SvxLanguageItem( eLang, nId ) );
        SetChanged( bChanged );
    }
}


/*************************************************************************
|*
|* Return language
|*
\************************************************************************/

LanguageType SdDrawDocument::GetLanguage( const sal_uInt16 nId ) const
{
    LanguageType eLangType = meLanguage;

    if( nId == EE_CHAR_LANGUAGE_CJK )
        eLangType = meLanguageCJK;
    else if( nId == EE_CHAR_LANGUAGE_CTL )
        eLangType = meLanguageCTL;

    return eLangType;
}


/*************************************************************************
|*
|* WorkStartup einleiten
|*
\************************************************************************/

IMPL_LINK_NOARG(SdDrawDocument, WorkStartupHdl)
{
    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_True );

    sal_Bool bChanged = IsChanged();        // merken

    // Autolayouts initialisieren
    SdPage* pHandoutMPage = GetMasterSdPage(0, PK_HANDOUT);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT6, sal_True, sal_True);
    }

    SdPage* pPage = GetSdPage(0, PK_STANDARD);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, sal_True, sal_True);
    }

    SdPage* pNotesPage = GetSdPage(0, PK_NOTES);

    if (pNotesPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True);
    }

    SetChanged(bChanged);

    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_False );
    return 0;
}


/*************************************************************************
|*
|* Wenn der WorkStartupTimer erzeugt worden ist (das erfolgt ausschliesslich
|* in SdDrawViewShell::Consruct() ), so wird der Timer ggf. gestoppt und
|* das WorkStartup eingeleitet
|*
\************************************************************************/

void SdDrawDocument::StopWorkStartupDelay()
{
    if (mpWorkStartupTimer)
    {
        if ( mpWorkStartupTimer->IsActive() )
        {
            // Timer war noch nicht abgelaufen -> WorkStartup wird eingeleitet
            mpWorkStartupTimer->Stop();
            WorkStartupHdl(NULL);
        }

        delete mpWorkStartupTimer;
        mpWorkStartupTimer = NULL;
    }
}

/*************************************************************************
|*
|* Wenn der WorkStartupTimer erzeugt worden ist (das erfolgt ausschliesslich
|* in SdDrawViewShell::Consruct() ), so wird der Timer ggf. gestoppt und
|* das WorkStartup eingeleitet
|*
\************************************************************************/

SdAnimationInfo* SdDrawDocument::GetAnimationInfo(SdrObject* pObject) const
{
    DBG_ASSERT(pObject, "sd::SdDrawDocument::GetAnimationInfo(), invalid argument!");
    if( pObject )
        return GetShapeUserData( *pObject, false );
    else
        return 0;
}

SdAnimationInfo* SdDrawDocument::GetShapeUserData(SdrObject& rObject, bool bCreate /* = false */ )
{
    sal_uInt16 nUD          = 0;
    sal_uInt16 nUDCount     = rObject.GetUserDataCount();
    SdrObjUserData* pUD = 0;
    SdAnimationInfo* pRet = 0;

    // gibt es in den User-Daten eine Animationsinformation?
    for (nUD = 0; nUD < nUDCount; nUD++)
    {
        pUD = rObject.GetUserData(nUD);
        if((pUD->GetInventor() == SdUDInventor) && (pUD->GetId() == SD_ANIMATIONINFO_ID))
        {
            pRet = dynamic_cast<SdAnimationInfo*>(pUD);
            break;
        }
    }

    if( (pRet == 0) && bCreate )
    {
        pRet = new SdAnimationInfo( rObject );
        rObject.AppendUserData( pRet);
    }

    return pRet;
}

SdIMapInfo* SdDrawDocument::GetIMapInfo( SdrObject* pObject ) const
{
    DBG_ASSERT(pObject, "ohne Objekt keine IMapInfo");

    SdrObjUserData* pUserData = NULL;
    SdIMapInfo*     pIMapInfo = NULL;
    sal_uInt16          nCount = pObject->GetUserDataCount();

    // gibt es in den User-Daten eine IMap-Information?
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        pUserData = pObject->GetUserData( i );

        if ( ( pUserData->GetInventor() == SdUDInventor ) && ( pUserData->GetId() == SD_IMAPINFO_ID ) )
            pIMapInfo = (SdIMapInfo*) pUserData;
    }

    return pIMapInfo;
}

IMapObject* SdDrawDocument::GetHitIMapObject( SdrObject* pObj,
                                              const Point& rWinPoint,
                                              const ::Window& /* rCmpWnd */ )
{
    SdIMapInfo* pIMapInfo = GetIMapInfo( pObj );
    IMapObject* pIMapObj = NULL;

    if ( pIMapInfo )
    {
        const MapMode       aMap100( MAP_100TH_MM );
        Size                aGraphSize;
        Point               aRelPoint( rWinPoint );
        ImageMap&           rImageMap = (ImageMap&) pIMapInfo->GetImageMap();
        const Rectangle&    rRect = pObj->GetLogicRect();
        sal_Bool                bObjSupported = sal_False;

        // HitTest ausfuehren
        if ( pObj->ISA( SdrGrafObj )  ) // einfaches Grafik-Objekt
        {
            const SdrGrafObj*   pGrafObj = (const SdrGrafObj*) pObj;
            const GeoStat&      rGeo = pGrafObj->GetGeoStat();
            SdrGrafObjGeoData*  pGeoData = (SdrGrafObjGeoData*) pGrafObj->GetGeoData();

            // Drehung rueckgaengig
            if ( rGeo.nDrehWink )
                RotatePoint( aRelPoint, rRect.TopLeft(), -rGeo.nSin, rGeo.nCos );

            // Spiegelung rueckgaengig
            if ( pGeoData->bMirrored )
                aRelPoint.X() = rRect.Right() + rRect.Left() - aRelPoint.X();

            // ggf. Unshear:
            if ( rGeo.nShearWink )
                ShearPoint( aRelPoint, rRect.TopLeft(), -rGeo.nTan );

            if ( pGrafObj->GetGrafPrefMapMode().GetMapUnit() == MAP_PIXEL )
                aGraphSize = Application::GetDefaultDevice()->PixelToLogic( pGrafObj->GetGrafPrefSize(), aMap100 );
            else
                aGraphSize = OutputDevice::LogicToLogic( pGrafObj->GetGrafPrefSize(),
                                                         pGrafObj->GetGrafPrefMapMode(), aMap100 );

            delete pGeoData;
            bObjSupported = sal_True;
        }
        else if ( pObj->ISA( SdrOle2Obj ) ) // OLE-Objekt
        {
            aGraphSize = ( (SdrOle2Obj*) pObj )->GetOrigObjSize();
            bObjSupported = sal_True;
        }

        // hat alles geklappt, dann HitTest ausfuehren
        if ( bObjSupported )
        {
            // relativen Mauspunkt berechnen
            aRelPoint -= rRect.TopLeft();
            pIMapObj = rImageMap.GetHitIMapObject( aGraphSize, rRect.GetSize(), aRelPoint );

            // Deaktivierte Objekte wollen wir nicht
            if ( pIMapObj && !pIMapObj->IsActive() )
                pIMapObj = NULL;
        }
    }

    return pIMapObj;
}

/** this method enforces that the masterpages are in the currect order,
    that is at position 1 is a PK_STANDARD masterpage followed by a
    PK_NOTES masterpage and so on. #
*/
void SdDrawDocument::CheckMasterPages()
{
//  RemoveMasterPage(2); // code to test the creation of notes pages

    sal_uInt16 nMaxPages = GetMasterPageCount();

    // we need at least a handout master and one master page
    if( nMaxPages < 2 )
    {
        return;
    }

    SdPage* pPage = NULL;
    SdPage* pNotesPage = NULL;

    sal_uInt16 nPage;

    // first see if the page order is correct
    for( nPage = 1; nPage < nMaxPages; nPage++ )
    {
        pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
        // if an odd page is not a standard page or an even page is not a notes page
        if( ((1 == (nPage & 1)) && (pPage->GetPageKind() != PK_STANDARD) ) ||
            ((0 == (nPage & 1)) && (pPage->GetPageKind() != PK_NOTES) ) )
            break; // then we have a fatal error
    }

    if( nPage < nMaxPages )
    {
        // there is a fatal error in the master page order,
        // we need to repair the document
        sal_Bool bChanged = sal_False;

        nPage = 1;
        while( nPage < nMaxPages )
        {
            pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
            if( pPage->GetPageKind() != PK_STANDARD )
            {
                bChanged = sal_True;
                sal_uInt16 nFound = nPage + 1;
                while( nFound < nMaxPages )
                {
                    pPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                    if( PK_STANDARD == pPage->GetPageKind() )
                    {
                        MoveMasterPage( nFound, nPage );
                        pPage->SetInserted(sal_True);
                        break;

                    }

                    nFound++;
                }

                // if we don't have any more standard pages, were done
                if( nMaxPages == nFound )
                    break;
            }

            nPage++;

            if( nPage < nMaxPages )
                pNotesPage = static_cast<SdPage*>(GetMasterPage( nPage ));
            else
                pNotesPage = NULL;

            if( (NULL == pNotesPage) || (pNotesPage->GetPageKind() != PK_NOTES) || ( pPage->GetLayoutName() != pNotesPage->GetLayoutName() ) )
            {
                bChanged = sal_True;

                sal_uInt16 nFound = nPage + 1;
                while( nFound < nMaxPages )
                {
                    pNotesPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                    if( (PK_NOTES == pNotesPage->GetPageKind()) && ( pPage->GetLayoutName() == pNotesPage->GetLayoutName() ) )
                    {
                        MoveMasterPage( nFound, nPage );
                        pNotesPage->SetInserted(sal_True);
                        break;
                    }

                    nFound++;
                }

                // looks like we lost a notes page
                if( nMaxPages == nFound )
                {
                    // so create one

                    // first find a reference notes page for size
                    SdPage* pRefNotesPage = NULL;
                    nFound = 0;
                    while( nFound < nMaxPages )
                    {
                        pRefNotesPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                        if( PK_NOTES == pRefNotesPage->GetPageKind() )
                            break;
                        nFound++;
                    }
                    if( nFound == nMaxPages )
                        pRefNotesPage = NULL;

                    SdPage* pNewNotesPage = static_cast<SdPage*>(AllocPage(sal_True));
                    pNewNotesPage->SetPageKind(PK_NOTES);
                    if( pRefNotesPage )
                    {
                        pNewNotesPage->SetSize( pRefNotesPage->GetSize() );
                        pNewNotesPage->SetBorder( pRefNotesPage->GetLftBorder(),
                                                pRefNotesPage->GetUppBorder(),
                                                pRefNotesPage->GetRgtBorder(),
                                                pRefNotesPage->GetLwrBorder() );
                    }
                    InsertMasterPage(pNewNotesPage,  nPage );
                    pNewNotesPage->SetLayoutName( pPage->GetLayoutName() );
                    pNewNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True );
                    nMaxPages++;
                }
            }

            nPage++;
        }

        // now remove all remaining and unused non PK_STANDARD slides
        while( nPage < nMaxPages )
        {
            bChanged = sal_True;

            RemoveMasterPage( nPage );
            nMaxPages--;
        }

        if( bChanged )
        {
            OSL_FAIL( "master pages where in a wrong order" );
            RecalcPageNums( sal_True);
        }
    }
}

sal_uInt16 SdDrawDocument::CreatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    sal_Bool bIsPageBack,
    sal_Bool bIsPageObj,
    const sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    SdPage* pStandardPage;
    SdPage* pNotesPage;

    // From the given page determine the standard page and notes page of which
    // to take the layout and the position where to insert the new pages.
    if (ePageKind == PK_NOTES)
    {
        pPreviousNotesPage = pActualPage;
        sal_uInt16 nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
        eStandardLayout = pPreviousStandardPage->GetAutoLayout();
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        sal_uInt16 nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
        eNotesLayout = pPreviousNotesPage->GetAutoLayout();
    }

    // Create new standard page and set it up.
    pStandardPage = (SdPage*) AllocPage(sal_False);

    // Set the size here since else the presobj autolayout
    // will be wrong.
    pStandardPage->SetSize( pPreviousStandardPage->GetSize() );
    pStandardPage->SetBorder( pPreviousStandardPage->GetLftBorder(),
                              pPreviousStandardPage->GetUppBorder(),
                              pPreviousStandardPage->GetRgtBorder(),
                              pPreviousStandardPage->GetLwrBorder() );

    // Use master page of current page.
    pStandardPage->TRG_SetMasterPage(pPreviousStandardPage->TRG_GetMasterPage());

    // User layout of current standard page.
    pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
    pStandardPage->SetAutoLayout(eStandardLayout, sal_True);
    pStandardPage->setHeaderFooterSettings( pPreviousStandardPage->getHeaderFooterSettings() );

    // transition settings of current page
    pStandardPage->setTransitionType( pPreviousStandardPage->getTransitionType() );
    pStandardPage->setTransitionSubtype( pPreviousStandardPage->getTransitionSubtype() );
    pStandardPage->setTransitionDirection( pPreviousStandardPage->getTransitionDirection() );
    pStandardPage->setTransitionFadeColor( pPreviousStandardPage->getTransitionFadeColor() );
    pStandardPage->setTransitionDuration( pPreviousStandardPage->getTransitionDuration() );

    // apply previous animation timing
    pStandardPage->SetPresChange( pPreviousStandardPage->GetPresChange() );
    pStandardPage->SetTime( pPreviousStandardPage->GetTime() );

    // Create new notes page and set it up.
    pNotesPage = (SdPage*) AllocPage(sal_False);
    pNotesPage->SetPageKind(PK_NOTES);

    // Use master page of current page.
    pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());

    // Use layout of current notes page.
    pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
    pNotesPage->SetAutoLayout(eNotesLayout, sal_True);
    pNotesPage->setHeaderFooterSettings( pPreviousNotesPage->getHeaderFooterSettings() );

    return InsertPageSet (
        pActualPage,
        ePageKind,
        sStandardPageName,
        sNotesPageName,
        bIsPageBack,
        bIsPageObj,
        pStandardPage,
        pNotesPage,
        nInsertPosition);
}




sal_uInt16 SdDrawDocument::DuplicatePage (sal_uInt16 nPageNum)
{
    PageKind ePageKind = PK_STANDARD;

    // Get current page.
    SdPage* pActualPage = GetSdPage(nPageNum, ePageKind);

    // Get background flags.
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False);
    sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False);
    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

    return DuplicatePage (
        pActualPage, ePageKind,
        // No names for the new slides.
        String(), String(),
        aVisibleLayers.IsSet(aBckgrnd),
        aVisibleLayers.IsSet(aBckgrndObj));
}




sal_uInt16 SdDrawDocument::DuplicatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    sal_Bool bIsPageBack,
    sal_Bool bIsPageObj,
    const sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    SdPage* pStandardPage;
    SdPage* pNotesPage;

    // From the given page determine the standard page and the notes page
    // of which to make copies.
    if (ePageKind == PK_NOTES)
    {
        pPreviousNotesPage = pActualPage;
        sal_uInt16 nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        sal_uInt16 nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
    }

    // Create duplicates of a standard page and the associated notes page.
    pStandardPage = (SdPage*) pPreviousStandardPage->Clone();
    pNotesPage = (SdPage*) pPreviousNotesPage->Clone();

    return InsertPageSet (
        pActualPage,
        ePageKind,
        sStandardPageName,
        sNotesPageName,
        bIsPageBack,
        bIsPageObj,
        pStandardPage,
        pNotesPage,
        nInsertPosition);
}




sal_uInt16 SdDrawDocument::InsertPageSet (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    sal_Bool bIsPageBack,
    sal_Bool bIsPageObj,
    SdPage* pStandardPage,
    SdPage* pNotesPage,
    sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    sal_uInt16 nStandardPageNum;
    sal_uInt16 nNotesPageNum;
    String aStandardPageName = sStandardPageName;
    String aNotesPageName = sNotesPageName;

    // Gather some information about the standard page and the notes page
    // that are to be inserted.  This makes sure that there is allways one
    // standard page followed by one notes page.
    if (ePageKind == PK_NOTES)
    {
        pPreviousNotesPage = pActualPage;
        nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
        nStandardPageNum = nNotesPageNum - 1;
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
        nNotesPageNum = nStandardPageNum + 1;
        aNotesPageName = aStandardPageName;
    }

    OSL_ASSERT(nNotesPageNum==nStandardPageNum+1);
    if (nInsertPosition < 0)
        nInsertPosition = nStandardPageNum;

    // Set up and insert the standard page.
    SetupNewPage (
        pPreviousStandardPage,
        pStandardPage,
        aStandardPageName,
        nInsertPosition,
        bIsPageBack,
        bIsPageObj);

    // Set up and insert the notes page.
    pNotesPage->SetPageKind(PK_NOTES);
    SetupNewPage (
        pPreviousNotesPage,
        pNotesPage,
        aNotesPageName,
        nInsertPosition+1,
        bIsPageBack,
        bIsPageObj);

    // Return an index that allows the caller to access the newly inserted
    // pages by using GetSdPage().
    return pStandardPage->GetPageNum() / 2;
}




void SdDrawDocument::SetupNewPage (
    SdPage* pPreviousPage,
    SdPage* pPage,
    const String& sPageName,
    sal_uInt16 nInsertionPoint,
    sal_Bool bIsPageBack,
    sal_Bool bIsPageObj)
{
    if (pPreviousPage != NULL)
    {
        pPage->SetSize( pPreviousPage->GetSize() );
        pPage->SetBorder( pPreviousPage->GetLftBorder(),
            pPreviousPage->GetUppBorder(),
            pPreviousPage->GetRgtBorder(),
            pPreviousPage->GetLwrBorder() );
    }
    pPage->SetName(sPageName);

    InsertPage(pPage, nInsertionPoint);

    if (pPreviousPage != NULL)
    {
        SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
        sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False);
        sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False);
        SetOfByte aVisibleLayers = pPreviousPage->TRG_GetMasterPageVisibleLayers();
        aVisibleLayers.Set(aBckgrnd, bIsPageBack);
        aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
        pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }
}

sd::UndoManager* SdDrawDocument::GetUndoManager() const
{
    return mpDocSh ? dynamic_cast< sd::UndoManager* >(mpDocSh->GetUndoManager()) : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
