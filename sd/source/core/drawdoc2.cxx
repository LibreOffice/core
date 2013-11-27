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


#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <vcl/wrkwin.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#ifndef SD_OUTLINE_HXX
#include "Outliner.hxx"
#endif
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
#include <svx/svdlegacy.hxx>

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
    sal_uInt32 nPage = 0;
    const sal_uInt32 nMaxPages = GetPageCount();

    while (nPage < nMaxPages && !pObjFound)
    {
        pPage = (SdPage*) GetPage(nPage);
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( rObjName == pObj->GetName() ) ||
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
    const sal_uInt32 nMaxMasterPages = GetMasterPageCount();

    while (nPage < nMaxMasterPages && !pObjFound)
    {
        pPage = (SdPage*) GetMasterPage(nPage);
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( rObjName == pObj->GetName() ) ||
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

sal_uInt32 SdDrawDocument::GetPageByName(const String& rPgName, bool& rbIsMasterPage) const
{
    SdPage* pPage = NULL;
    sal_uInt32 nPage = 0;
    const sal_uInt32 nMaxPages = GetPageCount();
    sal_uInt32 nPageNum = SDRPAGE_NOTFOUND;

    rbIsMasterPage = false;

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
    const sal_uInt32 nMaxMasterPages = GetMasterPageCount();
    nPage = 0;

    while (nPage < nMaxMasterPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = const_cast<SdPage*>(static_cast<const SdPage*>(
            GetMasterPage(nPage)));

        if (pPage && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
            rbIsMasterPage = true;
        }

        nPage++;
    }

    return nPageNum;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SdPage* SdDrawDocument::GetSdPage(sal_uInt32 nPgNum, PageKind ePgKind) const
{
    // #109538#
    return mpDrawPageListWatcher->GetSdPage(ePgKind, nPgNum);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_uInt32 SdDrawDocument::GetSdPageCount(PageKind ePgKind) const
{
    // #109538#
    return (sal_uInt16)mpDrawPageListWatcher->GetSdPageCount(ePgKind);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

SdPage* SdDrawDocument::GetMasterSdPage(sal_uInt32 nPgNum, PageKind ePgKind)
{
    // #109538#
    return mpMasterPageListWatcher->GetSdPage(ePgKind, nPgNum);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

sal_uInt32 SdDrawDocument::GetMasterSdPageCount(PageKind ePgKind) const
{
    // #109538#
    return (sal_uInt16)mpMasterPageListWatcher->GetSdPageCount(ePgKind);
}

/*************************************************************************
|*
|*  die in den Seitenobjekten der Notizseiten eingetragenen
|*  Seitennummern anpassen
|*
\************************************************************************/

void SdDrawDocument::UpdatePageObjectsInNotes(sal_uInt32 nStartPos)
{
    sal_uInt32 nPageCount   = GetPageCount();
    SdPage* pPage       = NULL;

    for (sal_uInt32 nPage = nStartPos; nPage < nPageCount; nPage++)
    {
        pPage = (SdPage*)GetPage(nPage);

        // wenn es eine Notizseite ist, Seitenobjekt suchen
        // und Nummer korrigieren
        if (pPage && pPage->GetPageKind() == PK_NOTES)
        {
            sal_uInt32 nObjCount = pPage->GetObjCount();
            SdrObject* pObj = 0;
            for (sal_uInt32 nObj = 0; nObj < nObjCount; nObj++)
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

    const sal_uInt32 nCount(GetItemPool().GetItemCount2(EE_FEATURE_FIELD));

    for (sal_uInt32 nOff = 0; nOff < nCount; nOff++)
    {
        const SfxPoolItem *pItem = GetItemPool().GetItem2(EE_FEATURE_FIELD, nOff);
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

void SdDrawDocument::UpdatePageRelativeURLs(SdPage* pPage, sal_uInt32 nPos, sal_Int32 nIncrement)
{
    bool bNotes = (pPage->GetPageKind() == PK_NOTES);
    const sal_uInt32 nCount(GetItemPool().GetItemCount2(EE_FEATURE_FIELD));

    for (sal_uInt32 nOff = 0; nOff < nCount; nOff++)
    {
        const SfxPoolItem *pItem = GetItemPool().GetItem2(EE_FEATURE_FIELD, nOff);
        const SvxFieldItem* pFldItem;

        if ((pFldItem = dynamic_cast< const SvxFieldItem * > (pItem)) != 0)
        {
            SvxURLField* pURLField = const_cast< SvxURLField* >( dynamic_cast<const SvxURLField*>( pFldItem->GetField() ) );

            if(pURLField)
            {
                XubString aURL = pURLField->GetURL();

                if (aURL.Len() && (aURL.GetChar(0) == 35))
                {
                    XubString aHashSlide('#');
                    aHashSlide += SdResId(STR_PAGE);

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
                        sal_uInt32 realPageNumber = (nPos + 1)/ 2;

                        if ( number >= (sal_Int32)realPageNumber )
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

void SdDrawDocument::MovePage(sal_uInt32 nPgNum, sal_uInt32 nNewPos)
{
    // Seite verschieben
    FmFormModel::MovePage(nPgNum, nNewPos);

    sal_uInt32 nMin = Min(nPgNum, nNewPos);

    UpdatePageObjectsInNotes(nMin);
}

/*************************************************************************
|*
|*  Seite einfuegen
|*
\************************************************************************/

void SdDrawDocument::InsertPage(SdrPage* pPage, sal_uInt32 nPos)
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

void SdDrawDocument::DeletePage(sal_uInt32 nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}

/*************************************************************************
|*
|*  Seite entfernen
|*
\************************************************************************/

SdrPage* SdDrawDocument::RemovePage(sal_uInt32 nPgNum)
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
void SdDrawDocument::InsertMasterPage(SdrPage* pPage, sal_uInt32 nPos )
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

SdrPage* SdDrawDocument::RemoveMasterPage(sal_uInt32 nPgNum)
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

void SdDrawDocument::SetSelected(SdPage* pPage, bool bSelect)
{
    PageKind ePageKind = pPage->GetPageKind();

    if (ePageKind == PK_STANDARD)
    {
        pPage->SetSelected(bSelect);

        const sal_uInt32 nDestPageNum(pPage->GetPageNumber() + 1);
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
        SdPage* pStandardPage = (SdPage*) GetPage( pPage->GetPageNumber() - 1 );

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
    sal_uInt32 nPageCount = GetPageCount();

    if (nPageCount <= 1)
    {
        // #i57181# Paper size depends on Language, like in Writer
        Size aDefSize = SvxPaperInfo::GetDefaultPaperSize( MAP_100TH_MM );

        /**********************************************************************
        * Handzettel-Seite einfuegen
        **********************************************************************/
        bool bMasterPage;
        SdPage* pHandoutPage = dynamic_cast< SdPage* >( AllocPage(bMasterPage=false) );

        SdPage* pRefPage = NULL;

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_HANDOUT );

        if( pRefPage )
        {
            pHandoutPage->SetPageScale(pRefPage->GetPageScale());
            pHandoutPage->SetPageBorder(
                pRefPage->GetLeftPageBorder(),
                pRefPage->GetTopPageBorder(),
                pRefPage->GetRightPageBorder(),
                pRefPage->GetBottomPageBorder() );
        }
        else
        {
            pHandoutPage->SetPageScale(basegfx::B2DVector(aDefSize.Width(), aDefSize.Height()));
            pHandoutPage->SetPageBorder(0, 0, 0, 0);
        }

        pHandoutPage->SetPageKind(PK_HANDOUT);
        pHandoutPage->SetName( String (SdResId(STR_HANDOUT) ) );
        InsertPage(pHandoutPage, 0);

        /**********************************************************************
        * MasterPage einfuegen und an der Handzettel-Seite vermerken
        **********************************************************************/
        SdPage* pHandoutMPage = (SdPage*) AllocPage(bMasterPage=true);
        pHandoutMPage->SetPageScale( pHandoutPage->GetPageScale() );
        pHandoutMPage->SetPageKind(PK_HANDOUT);
        pHandoutMPage->SetPageBorder( pHandoutPage->GetLeftPageBorder(),
                                  pHandoutPage->GetTopPageBorder(),
                                  pHandoutPage->GetRightPageBorder(),
                                  pHandoutPage->GetBottomPageBorder() );
        InsertMasterPage(pHandoutMPage, 0);
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

        /**********************************************************************
        * Seite einfuegen
        * Sofern nPageCount==1 ist, wurde das Model fuers Clipboad erzeugt.
        * Eine Standard-Seite ist daher schon vorhanden.
        **********************************************************************/
        SdPage* pPage;
        bool bClipboard = false;

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_STANDARD );

        if (nPageCount == 0)
        {
            pPage = dynamic_cast< SdPage* >( AllocPage(bMasterPage=false) );

            if( pRefPage )
            {
                pPage->SetPageScale( pRefPage->GetPageScale() );
                pPage->SetPageBorder(
                    pRefPage->GetLeftPageBorder(),
                    pRefPage->GetTopPageBorder(),
                    pRefPage->GetRightPageBorder(),
                    pRefPage->GetBottomPageBorder() );
            }
            else if (meDocType == DOCUMENT_TYPE_DRAW)
            {
                // Draw: stets Default-Groesse mit Raendern
                pPage->SetPageScale(basegfx::B2DVector(aDefSize.Width(), aDefSize.Height()));

                SfxPrinter* pPrinter = mpDocSh->GetPrinter(false);
                if (pPrinter && pPrinter->IsValid())
                {
                    Size aOutSize(pPrinter->GetOutputSize());
                    Point aPageOffset(pPrinter->GetPageOffset());
                    aPageOffset -= pPrinter->PixelToLogic( Point() );
                    long nOffset = !aPageOffset.X() && !aPageOffset.X() ? 0 : PRINT_OFFSET;

                    sal_uInt32 nTop    = aPageOffset.Y();
                    sal_uInt32 nLeft   = aPageOffset.X();
                    sal_uInt32 nBottom = Max((long)(aDefSize.Height() - aOutSize.Height() - nTop + nOffset), 0L);
                    sal_uInt32 nRight  = Max((long)(aDefSize.Width() - aOutSize.Width() - nLeft + nOffset), 0L);

                    pPage->SetPageBorder(nLeft, nTop, nRight, nBottom);
                }
                else
                {
                    // The printer is not available.  Use a border of 10mm
                    // on each side instead.
                    // This has to be kept synchronized with the border
                    // width set in the
                    // SvxPageDescPage::PaperSizeSelect_Impl callback.
                    pPage->SetPageBorder(1000, 1000, 1000, 1000);
                }
            }
            else
            {
                // Impress: stets Bildschirmformat, quer
                Size aSz( SvxPaperInfo::GetPaperSize(PAPER_SCREEN, MAP_100TH_MM) );
                pPage->SetPageScale( basegfx::B2DVector( aSz.Height(), aSz.Width() ) );
                pPage->SetPageBorder(0, 0, 0, 0);
            }

            InsertPage(pPage, 1);
        }
        else
        {
            bClipboard = true;
            pPage = (SdPage*) GetPage(1);
        }

        /**********************************************************************
        * MasterPage einfuegen und an der Seite vermerken
        **********************************************************************/
        SdPage* pMPage = (SdPage*) AllocPage(bMasterPage=true);
        pMPage->SetPageScale( pPage->GetPageScale() );
        pMPage->SetPageBorder( pPage->GetLeftPageBorder(),
                           pPage->GetTopPageBorder(),
                           pPage->GetRightPageBorder(),
                           pPage->GetBottomPageBorder() );
        InsertMasterPage(pMPage, 1);
        pPage->TRG_SetMasterPage( *pMPage );
        if( bClipboard )
            pMPage->SetLayoutName( pPage->GetLayoutName() );

        /**********************************************************************
        * Notizen-Seite einfuegen
        **********************************************************************/
        SdPage* pNotesPage = (SdPage*) AllocPage(bMasterPage=false);

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_NOTES );

        if( pRefPage )
        {
            pNotesPage->SetPageScale( pRefPage->GetPageScale() );
            pNotesPage->SetPageBorder(
                pRefPage->GetLeftPageBorder(),
                pRefPage->GetTopPageBorder(),
                pRefPage->GetRightPageBorder(),
                pRefPage->GetBottomPageBorder() );
        }
        else
        {
            // Stets Hochformat
            if (aDefSize.Height() >= aDefSize.Width())
            {
                pNotesPage->SetPageScale(basegfx::B2DVector(aDefSize.Width(), aDefSize.Height()));
            }
            else
            {
                pNotesPage->SetPageScale( basegfx::B2DVector(aDefSize.Height(), aDefSize.Width()) );
            }

            pNotesPage->SetPageBorder(0, 0, 0, 0);
        }
        pNotesPage->SetPageKind(PK_NOTES);
        InsertPage(pNotesPage, 2);
        if( bClipboard )
            pNotesPage->SetLayoutName( pPage->GetLayoutName() );

        /**********************************************************************
        * MasterPage einfuegen und an der Notizen-Seite vermerken
        **********************************************************************/
        SdPage* pNotesMPage = (SdPage*) AllocPage(bMasterPage=true);
        pNotesMPage->SetPageScale( pNotesPage->GetPageScale() );
        pNotesMPage->SetPageKind(PK_NOTES);
        pNotesMPage->SetPageBorder( pNotesPage->GetLeftPageBorder(),
                                pNotesPage->GetTopPageBorder(),
                                pNotesPage->GetRightPageBorder(),
                                pNotesPage->GetBottomPageBorder() );
        InsertMasterPage(pNotesMPage, 2);
        pNotesPage->TRG_SetMasterPage( *pNotesMPage );
        if( bClipboard )
            pNotesMPage->SetLayoutName( pPage->GetLayoutName() );


        if( !pRefPage && (meDocType != DOCUMENT_TYPE_DRAW) )
            pPage->SetAutoLayout( AUTOLAYOUT_TITLE, true, true );

        mpWorkStartupTimer = new Timer();
        mpWorkStartupTimer->SetTimeoutHdl( LINK(this, SdDrawDocument, WorkStartupHdl) );
        mpWorkStartupTimer->SetTimeout(2000);
        mpWorkStartupTimer->Start();

        SetChanged(false);
    }
}

/*************************************************************************
|*
|* Erzeugt fehlende Notiz und Handzettelseiten (nach PowerPoint-Import)
|* Es wird davon ausgegangen, dass mindestens eine Standard-Seite und
|* eine Standard-MasterPage vorhanden sind.
|*
\************************************************************************/

bool SdDrawDocument::CreateMissingNotesAndHandoutPages()
{
    bool bOK = false;
    sal_uInt32 nPageCount = GetPageCount();

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

        for (sal_uInt32 i = 1; i < nPageCount; i = i + 2)
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
            const sal_uInt32 nMasterPageAfterPagesMasterPage = (pPage->TRG_GetMasterPage()).GetPageNumber() + 1;
            pNotesPage->TRG_SetMasterPage(*GetMasterPage(nMasterPageAfterPagesMasterPage));
        }

        bOK = true;
        StopWorkStartupDelay();
        SetChanged(false);
    }

    return(bOK);
}

/*************************************************************************
|*
|* - selektierte Seiten hinter genannte Seite schieben
|*   (nTargetPage = (sal_uInt32)-1  --> vor erste Seite schieben)
|* - ergibt true, wenn Seiten verschoben wurden
|*
\************************************************************************/

bool SdDrawDocument::MovePages(sal_uInt32 nTargetPage)
{
    SdPage* pTargetPage        = NULL;
    SdPage* pPage              = NULL;
    sal_uInt32 nPage;
    const sal_uInt32 nNoOfPages(GetSdPageCount(PK_STANDARD));
    bool    bSomethingHappened = false;

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(String(SdResId(STR_UNDO_MOVEPAGES)));

    // Liste mit selektierten Seiten
    List    aPageList;
    for (nPage = 0; nPage < nNoOfPages; nPage++)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);
        if (pPage->IsSelected())
        {
            aPageList.Insert(pPage, LIST_APPEND);
        }
    }

    // falls noetig, nach vorne hangeln, bis nicht selektierte Seite gefunden
    nPage = nTargetPage;
    if (nPage != (sal_uInt32)-1)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);
        while (nPage > 0 && pPage->IsSelected())
        {
            nPage--;
            pPage = GetSdPage(nPage, PK_STANDARD);
        }

        if (pPage->IsSelected())
        {
            nPage = (sal_uInt32)-1;
        }
    }

    // vor der ersten Seite einfuegen
    if (nPage == (sal_uInt32)-1)
    {
        while (aPageList.Count() > 0)
        {
            aPageList.Last();

            nPage = ( (SdPage*) aPageList.GetCurObject() )->GetPageNumber();
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
                bSomethingHappened = true;
            }
            aPageList.Remove();
        }
    }
    // hinter <nPage> einfuegen
    else
    {
        pTargetPage = GetSdPage(nPage, PK_STANDARD);
        nTargetPage = nPage;
        nTargetPage = 2 * nTargetPage + 1;    // PK_STANDARD --> absolut
        while (aPageList.Count() > 0)
        {
            pPage = (SdPage*)aPageList.GetObject(0);
            nPage = pPage->GetPageNumber();
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
                    bSomethingHappened = true;
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
            aPageList.Remove((sal_uIntPtr)0);
            nTargetPage = pPage->GetPageNumber();
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

sal_uInt32 SdDrawDocument::GetLinkCount()
{
    return ( GetLinkManager()->GetLinks().Count() );
}

/*************************************************************************
|*
|* Language setzen
|*
\************************************************************************/

void SdDrawDocument::SetLanguage( const LanguageType eLang, const sal_uInt16 nId )
{
    bool bChanged = false;

    if( nId == EE_CHAR_LANGUAGE && meLanguage != eLang )
    {
        meLanguage = eLang;
        bChanged = true;
    }
    else if( nId == EE_CHAR_LANGUAGE_CJK && meLanguageCJK != eLang )
    {
        meLanguageCJK = eLang;
        bChanged = true;
    }
    else if( nId == EE_CHAR_LANGUAGE_CTL && meLanguageCTL != eLang )
    {
        meLanguageCTL = eLang;
        bChanged = true;
    }

    if( bChanged )
    {
        GetDrawOutliner().SetDefaultLanguage( Application::GetSettings().GetLanguage() );
        GetItemPool().SetPoolDefaultItem( SvxLanguageItem( eLang, nId ) );
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

IMPL_LINK( SdDrawDocument, WorkStartupHdl, Timer *, EMPTYARG )
{
    if( mpDocSh )
        mpDocSh->SetWaitCursor( true );

    bool bChanged = IsChanged();        // merken

    // Autolayouts initialisieren
    SdPage* pHandoutMPage = GetMasterSdPage(0, PK_HANDOUT);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT6, true, true);
    }

    SdPage* pPage = GetSdPage(0, PK_STANDARD);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, true, true);
    }

    SdPage* pNotesPage = GetSdPage(0, PK_NOTES);

    if (pNotesPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true, true);
    }

    SetChanged(bChanged || false);

    if( mpDocSh )
        mpDocSh->SetWaitCursor( false );
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

SdAnimationInfo* SdDrawDocument::GetShapeUserData(const SdrObject& rObject, bool bCreate /* = false */ )
{
    sal_uInt32 nUD(0);
    const sal_uInt32 nUDCount(rObject.GetUserDataCount());
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
        pRet = new SdAnimationInfo( const_cast< SdrObject& >(rObject) );
        const_cast< SdrObject& >(rObject).InsertUserData( pRet);
    }

    return pRet;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SdIMapInfo* SdDrawDocument::GetIMapInfo( SdrObject* pObject ) const
{
    DBG_ASSERT(pObject, "ohne Objekt keine IMapInfo");

    SdrObjUserData* pUserData = NULL;
    SdIMapInfo*     pIMapInfo = NULL;
    const sal_uInt32 nCount(pObject->GetUserDataCount());

    // gibt es in den User-Daten eine IMap-Information?
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        pUserData = pObject->GetUserData( i );

        if ( ( pUserData->GetInventor() == SdUDInventor ) && ( pUserData->GetId() == SD_IMAPINFO_ID ) )
            pIMapInfo = (SdIMapInfo*) pUserData;
    }

    return pIMapInfo;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMapObject* SdDrawDocument::GetHitIMapObject( SdrObject* pObj,
    const basegfx::B2DPoint& rWinPoint, const ::Window& /* rCmpWnd */ )
{
    SdIMapInfo* pIMapInfo = GetIMapInfo( pObj );
    IMapObject* pIMapObj = NULL;

    if ( pIMapInfo )
    {
        const MapMode       aMap100( MAP_100TH_MM );
        Size                aGraphSize;
        basegfx::B2DPoint aRelPoint( rWinPoint );
        ImageMap&           rImageMap = (ImageMap&) pIMapInfo->GetImageMap();
        const basegfx::B2DRange aLogicRange(sdr::legacy::GetLogicRange(*pObj));
        bool bObjSupported = false;

        // HitTest ausfuehren
        const SdrGrafObj* pGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);

        if ( pGrafObj  ) // einfaches Grafik-Objekt
        {
            // remove shear, mirror and rotation; so just absolute scale and translation get applied
            basegfx::B2DHomMatrix aJustAbsScaleTranslate(pGrafObj->getSdrObjectTransformation());

            aJustAbsScaleTranslate.invert();
            aJustAbsScaleTranslate.scale(basegfx::absolute(pGrafObj->getSdrObjectScale()));
            aJustAbsScaleTranslate.translate(pGrafObj->getSdrObjectTranslate());
            aRelPoint = aJustAbsScaleTranslate * aRelPoint;

            if ( pGrafObj->GetGrafPrefMapMode().GetMapUnit() == MAP_PIXEL )
                aGraphSize = Application::GetDefaultDevice()->PixelToLogic( pGrafObj->GetGrafPrefSize(), aMap100 );
            else
                aGraphSize = OutputDevice::LogicToLogic( pGrafObj->GetGrafPrefSize(),
                                                         pGrafObj->GetGrafPrefMapMode(), aMap100 );

            bObjSupported = true;
        }
        else
        {
            const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);

            if ( pSdrOle2Obj ) // OLE-Objekt
        {
                aGraphSize = pSdrOle2Obj->GetOrigObjSize();
                bObjSupported = true;
            }
        }

        // hat alles geklappt, dann HitTest ausfuehren
        if ( bObjSupported )
        {
            // relativen Mauspunkt berechnen
            aRelPoint -= aLogicRange.getMinimum();

            const Size aLogicRangeSize(basegfx::fround(aLogicRange.getWidth()), basegfx::fround(aLogicRange.getHeight()));
            const Point aOldRelPoint(basegfx::fround(aRelPoint.getX()), basegfx::fround(aRelPoint.getY()));

            pIMapObj = rImageMap.GetHitIMapObject( aGraphSize, aLogicRangeSize, aOldRelPoint );

            // Deaktivierte Objekte wollen wir nicht
            if ( pIMapObj && !pIMapObj->IsActive() )
                pIMapObj = NULL;
        }
    }

    return pIMapObj;
}

//IAccessibility2 Implementation 2009-----
ImageMap* SdDrawDocument::GetImageMapForObject(SdrObject* pObj)
{
    SdIMapInfo* pIMapInfo = GetIMapInfo( pObj );
    if ( pIMapInfo )
    {
        return const_cast<ImageMap*>( &(pIMapInfo->GetImageMap()) );
    }
    return NULL;
}
//-----IAccessibility2 Implementation 2009
/** this method enforces that the masterpages are in the currect order,
    that is at position 1 is a PK_STANDARD masterpage followed by a
    PK_NOTES masterpage and so on. #
*/
void SdDrawDocument::CheckMasterPages()
{
//  RemoveMasterPage(2); // code to test the creation of notes pages

    sal_uInt32 nMaxPages(GetMasterPageCount());

    // we need at least a handout master and one master page
    if( nMaxPages < 2 )
    {
        return;
    }

    SdPage* pPage = 0;
    SdPage* pNotesPage = 0;
    sal_uInt32 nPage;

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
        nPage = 1;
        while( nPage < nMaxPages )
        {
            pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
            if( pPage->GetPageKind() != PK_STANDARD )
            {
                sal_uInt32 nFound = nPage + 1;
                while( nFound < nMaxPages )
                {
                    pPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                    if( PK_STANDARD == pPage->GetPageKind() )
                    {
                        MoveMasterPage( nFound, nPage );
                        // pPage->SetInserted(sal_True);
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
                sal_uInt32 nFound = nPage + 1;
                while( nFound < nMaxPages )
                {
                    pNotesPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                    if( (PK_NOTES == pNotesPage->GetPageKind()) && ( pPage->GetLayoutName() == pNotesPage->GetLayoutName() ) )
                    {
                        MoveMasterPage( nFound, nPage );
                        // pNotesPage->SetInserted(sal_True);
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
                        pNewNotesPage->SetPageScale( pRefNotesPage->GetPageScale() );
                        pNewNotesPage->SetPageBorder( pRefNotesPage->GetLeftPageBorder(),
                                                pRefNotesPage->GetTopPageBorder(),
                                                pRefNotesPage->GetRightPageBorder(),
                                                pRefNotesPage->GetBottomPageBorder() );
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
            RemoveMasterPage( nPage );
            nMaxPages--;
        }
    }
}

sal_uInt32 SdDrawDocument::CreatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    bool bIsPageBack,
    bool bIsPageObj,
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
        sal_uInt32 nNotesPageNum = pPreviousNotesPage->GetPageNumber() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
        eStandardLayout = pPreviousStandardPage->GetAutoLayout();
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        sal_uInt32 nStandardPageNum = pPreviousStandardPage->GetPageNumber() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
        eNotesLayout = pPreviousNotesPage->GetAutoLayout();
    }

    // Create new standard page and set it up.
    pStandardPage = (SdPage*) AllocPage(false);

    // #108658#
    // Set the size here since else the presobj autolayout
    // will be wrong.
    pStandardPage->SetPageScale( pPreviousStandardPage->GetPageScale() );
    pStandardPage->SetPageBorder( pPreviousStandardPage->GetLeftPageBorder(),
                              pPreviousStandardPage->GetTopPageBorder(),
                              pPreviousStandardPage->GetRightPageBorder(),
                              pPreviousStandardPage->GetBottomPageBorder() );

    // Use master page of current page.
    pStandardPage->TRG_SetMasterPage(pPreviousStandardPage->TRG_GetMasterPage());

    // User layout of current standard page.
    pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
    pStandardPage->SetAutoLayout(eStandardLayout, true);
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
    pNotesPage = (SdPage*) AllocPage(false);
    pNotesPage->SetPageKind(PK_NOTES);

    // Use master page of current page.
    pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());

    // Use layout of current notes page.
    pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
    pNotesPage->SetAutoLayout(eNotesLayout, true);
    pNotesPage->setHeaderFooterSettings( pPreviousNotesPage->getHeaderFooterSettings() );

    return InsertPageSet (
        pActualPage,
        ePageKind,
        sStandardPageName,
        sNotesPageName,
        eStandardLayout,
        eNotesLayout,
        bIsPageBack,
        bIsPageObj,
        pStandardPage,
        pNotesPage,
        nInsertPosition);
}




sal_uInt32 SdDrawDocument::DuplicatePage (sal_uInt32 nPageNum)
{
    PageKind ePageKind = PK_STANDARD;

    // Get current page.
    SdPage* pActualPage = GetSdPage(nPageNum, ePageKind);

    // Get background flags.
    SdrLayerAdmin& rLayerAdmin = GetModelLayerAdmin();
    sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), false);
    sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), false);
    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

    // Get layout from current page.
    AutoLayout eAutoLayout = pActualPage->GetAutoLayout();

    return DuplicatePage (
        pActualPage, ePageKind,
        // No names for the new slides.
        String(), String(),
        eAutoLayout, eAutoLayout,
        aVisibleLayers.IsSet(aBckgrnd),
        aVisibleLayers.IsSet(aBckgrndObj));
}




sal_uInt32 SdDrawDocument::DuplicatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    bool bIsPageBack,
    bool bIsPageObj,
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
        sal_uInt32 nNotesPageNum = pPreviousNotesPage->GetPageNumber() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        sal_uInt32 nStandardPageNum = pPreviousStandardPage->GetPageNumber() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
    }

    // Create duplicates of a standard page and the associated notes page.
    pStandardPage = (SdPage*) pPreviousStandardPage->CloneSdrPage();
    pNotesPage = (SdPage*) pPreviousNotesPage->CloneSdrPage();

    return InsertPageSet (
        pActualPage,
        ePageKind,
        sStandardPageName,
        sNotesPageName,
        eStandardLayout,
        eNotesLayout,
        bIsPageBack,
        bIsPageObj,
        pStandardPage,
        pNotesPage,
        nInsertPosition);
}




sal_uInt32 SdDrawDocument::InsertPageSet (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    bool bIsPageBack,
    bool bIsPageObj,
    SdPage* pStandardPage,
    SdPage* pNotesPage,
    sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    sal_uInt32 nStandardPageNum;
    sal_uInt32 nNotesPageNum;
    String aStandardPageName = sStandardPageName;
    String aNotesPageName = sNotesPageName;

    // Gather some information about the standard page and the notes page
    // that are to be inserted.  This makes sure that there is allways one
    // standard page followed by one notes page.
    if (ePageKind == PK_NOTES)
    {
        pPreviousNotesPage = pActualPage;
        nNotesPageNum = pPreviousNotesPage->GetPageNumber() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
        nStandardPageNum = nNotesPageNum - 1;
        eStandardLayout = pPreviousStandardPage->GetAutoLayout();
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        nStandardPageNum = pPreviousStandardPage->GetPageNumber() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
        nNotesPageNum = nStandardPageNum + 1;
        aNotesPageName = aStandardPageName;
        eNotesLayout = pPreviousNotesPage->GetAutoLayout();
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
    return pStandardPage->GetPageNumber() / 2;
}




void SdDrawDocument::SetupNewPage (
    SdPage* pPreviousPage,
    SdPage* pPage,
    const String& sPageName,
    sal_uInt32 nInsertionPoint,
    bool bIsPageBack,
    bool bIsPageObj)
{
    if (pPreviousPage != NULL)
    {
        pPage->SetPageScale( pPreviousPage->GetPageScale() );
        pPage->SetPageBorder( pPreviousPage->GetLeftPageBorder(),
            pPreviousPage->GetTopPageBorder(),
            pPreviousPage->GetRightPageBorder(),
            pPreviousPage->GetBottomPageBorder() );
    }
    pPage->SetName(sPageName);

    InsertPage(pPage, nInsertionPoint);

    if (pPreviousPage != NULL)
    {
        SdrLayerAdmin& rLayerAdmin = GetModelLayerAdmin();
        sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), false);
        sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), false);
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

// eof
