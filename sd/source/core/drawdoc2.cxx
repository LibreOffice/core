/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>

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
#include <svx/dialmgr.hxx>

#include <editeng/outliner.hxx>
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
#include "customshowlist.hxx"

using namespace ::sd;

const long PRINT_OFFSET = 30;       

using namespace com::sun::star;


SdrObject* SdDrawDocument::GetObj(const OUString& rObjName) const
{
    SdrObject* pObj = NULL;
    SdrObject* pObjFound = NULL;
    SdPage* pPage = NULL;

    
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



sal_uInt16 SdDrawDocument::GetPageByName(const OUString& rPgName, sal_Bool& rbIsMasterPage) const
{
    SdPage* pPage = NULL;
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = GetPageCount();
    sal_uInt16 nPageNum = SDRPAGE_NOTFOUND;

    rbIsMasterPage = sal_False;

    
    
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

sal_uInt16 SdDrawDocument::GetActiveSdPageCount() const
{
    return (sal_uInt16)mpDrawPageListWatcher->GetVisibleSdPageCount();
}



void SdDrawDocument::UpdatePageObjectsInNotes(sal_uInt16 nStartPos)
{
    sal_uInt16  nPageCount  = GetPageCount();
    SdPage* pPage       = NULL;

    for (sal_uInt16 nPage = nStartPos; nPage < nPageCount; nPage++)
    {
        pPage = (SdPage*)GetPage(nPage);

        
        
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
                    
                    DBG_ASSERTWARNING(nStartPos, "Position of notes page must not be 0.");

                    DBG_ASSERTWARNING(nPage > 1, "Page object must not be a handout.");

                    if (nStartPos > 0 && nPage > 1)
                        ((SdrPageObj*)pObj)->SetReferencedPage(GetPage(nPage - 1));
                }
            }
        }
    }
}

void SdDrawDocument::UpdatePageRelativeURLs(const OUString& rOldName, const OUString& rNewName)
{
    if (rNewName.isEmpty())
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
                OUString aURL = pURLField->GetURL();

                if (!aURL.isEmpty() && (aURL[0] == 35) && (aURL.indexOf(rOldName, 1) == 1))
                {
                    if (aURL.getLength() == rOldName.getLength() + 1) 
                    {
                        aURL = aURL.replaceAt(1, aURL.getLength() - 1, "");
                        aURL += rNewName;
                        pURLField->SetURL(aURL);
                    }
                    else
                    {
                        const OUString sNotes(SD_RESSTR(STR_NOTES));
                        if (aURL.getLength() == rOldName.getLength() + 2 + sNotes.getLength()
                            && aURL.indexOf(sNotes, rOldName.getLength() + 2) == rOldName.getLength() + 2)
                        {
                            aURL = aURL.replaceAt(1, aURL.getLength() - 1, "");
                            aURL += rNewName + " " + sNotes;
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
                OUString aURL = pURLField->GetURL();

                if (!aURL.isEmpty() && (aURL[0] == 35))
                {
                    OUString aHashSlide("#");
                    aHashSlide += SD_RESSTR(STR_PAGE);

                    if (aURL.startsWith(aHashSlide))
                    {
                        OUString aURLCopy = aURL;
                        const OUString sNotes(SD_RESSTR(STR_NOTES));

                        aURLCopy = aURLCopy.replaceAt(0, aHashSlide.getLength(), "");

                        bool bNotesLink = ( aURLCopy.getLength() >= sNotes.getLength() + 3
                            && aURLCopy.endsWith(sNotes) );

                        if (bNotesLink != bNotes)
                            continue; 

                        if (bNotes)
                            aURLCopy = aURLCopy.replaceAt(aURLCopy.getLength() - sNotes.getLength(), sNotes.getLength(), "");

                        sal_Int32 number = aURLCopy.toInt32();
                        sal_uInt16 realPageNumber = (nPos + 1)/ 2;

                        if ( number >= realPageNumber )
                        {
                            
                            number += nIncrement;
                            aURL = aURL.replaceAt(aHashSlide.getLength() + 1, aURL.getLength() - aHashSlide.getLength() - 1, "");
                            aURL += OUString::number(number);
                            if (bNotes)
                            {
                                aURL += " " + sNotes;
                            }
                            pURLField->SetURL(aURL);
                        }
                    }
                }
            }
        }
    }
}


void SdDrawDocument::MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    FmFormModel::MovePage(nPgNum, nNewPos);

    sal_uInt16 nMin = std::min(nPgNum, nNewPos);

    UpdatePageObjectsInNotes(nMin);
}


void SdDrawDocument::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    bool bLast = (nPos == GetPageCount());

    FmFormModel::InsertPage(pPage, nPos);

    ((SdPage*)pPage)->ConnectLink();

    UpdatePageObjectsInNotes(nPos);

    if (!bLast)
        UpdatePageRelativeURLs(static_cast<SdPage*>( pPage ), nPos, 1);

}


void SdDrawDocument::DeletePage(sal_uInt16 nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}


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



void SdDrawDocument::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos )
{
    FmFormModel::InsertMasterPage( pPage, nPos );
    if( pPage->IsMasterPage() && (static_cast<SdPage*>(pPage)->GetPageKind() == PK_STANDARD) )
    {
        
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
        
        SdStyleSheetPool* pStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
        if( pStylePool )
            pStylePool->RemoveStyleFamily( pPage );
    }

    return FmFormModel::RemoveMasterPage(nPgNum);
}


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


void SdDrawDocument::CreateFirstPages( SdDrawDocument* pRefDocument /* = 0 */ )
{
    
    sal_uInt16 nPageCount = GetPageCount();

    if (nPageCount <= 1)
    {
        
        Size aDefSize = SvxPaperInfo::GetDefaultPaperSize( MAP_100TH_MM );

        
        SdPage* pHandoutPage = dynamic_cast< SdPage* >( AllocPage(false) );

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
        pHandoutPage->SetName( SD_RESSTR(STR_HANDOUT) );
        InsertPage(pHandoutPage, 0);

        
        SdPage* pHandoutMPage = (SdPage*) AllocPage(true);
        pHandoutMPage->SetSize( pHandoutPage->GetSize() );
        pHandoutMPage->SetPageKind(PK_HANDOUT);
        pHandoutMPage->SetBorder( pHandoutPage->GetLftBorder(),
                                  pHandoutPage->GetUppBorder(),
                                  pHandoutPage->GetRgtBorder(),
                                  pHandoutPage->GetLwrBorder() );
        InsertMasterPage(pHandoutMPage, 0);
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

        
        
        
        SdPage* pPage;
        sal_Bool bClipboard = sal_False;

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_STANDARD );

        if (nPageCount == 0)
        {
            pPage = dynamic_cast< SdPage* >( AllocPage(false) );

            if( pRefPage )
            {
                pPage->SetSize( pRefPage->GetSize() );
                pPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
            }
            else if (meDocType == DOCUMENT_TYPE_DRAW)
            {
                
                pPage->SetSize(aDefSize);

                SfxPrinter* pPrinter = mpDocSh->GetPrinter(sal_False);
                if (pPrinter && pPrinter->IsValid())
                {
                    Size aOutSize(pPrinter->GetOutputSize());
                    Point aPageOffset(pPrinter->GetPageOffset());
                    aPageOffset -= pPrinter->PixelToLogic( Point() );
                    long nOffset = !aPageOffset.X() && !aPageOffset.Y() ? 0 : PRINT_OFFSET;

                    sal_uLong nTop    = aPageOffset.Y();
                    sal_uLong nLeft   = aPageOffset.X();
                    sal_uLong nBottom = std::max((long)(aDefSize.Height() - aOutSize.Height() - nTop + nOffset), 0L);
                    sal_uLong nRight  = std::max((long)(aDefSize.Width() - aOutSize.Width() - nLeft + nOffset), 0L);

                    pPage->SetBorder(nLeft, nTop, nRight, nBottom);
                }
                else
                {
                    
                    
                    
                    
                    
                    pPage->SetBorder(1000, 1000, 1000, 1000);
                }
            }
            else
            {
                
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

        
        SdPage* pMPage = (SdPage*) AllocPage(true);
        pMPage->SetSize( pPage->GetSize() );
        pMPage->SetBorder( pPage->GetLftBorder(),
                           pPage->GetUppBorder(),
                           pPage->GetRgtBorder(),
                           pPage->GetLwrBorder() );
        InsertMasterPage(pMPage, 1);
        pPage->TRG_SetMasterPage( *pMPage );
        if( bClipboard )
            pMPage->SetLayoutName( pPage->GetLayoutName() );

        
        SdPage* pNotesPage = (SdPage*) AllocPage(false);

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_NOTES );

        if( pRefPage )
        {
            pNotesPage->SetSize( pRefPage->GetSize() );
            pNotesPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
        }
        else
        {
            
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

        
        SdPage* pNotesMPage = (SdPage*) AllocPage(true);
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

        SetChanged(false);
    }
}




sal_Bool SdDrawDocument::CreateMissingNotesAndHandoutPages()
{
    sal_Bool bOK = sal_False;
    sal_uInt16 nPageCount = GetPageCount();

    if (nPageCount != 0)
    {
        
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
                
                
                pPage->TRG_SetMasterPage(*GetMasterPage(1));
            }

            SdPage* pNotesPage = (SdPage*) GetPage(i+1);
            pNotesPage->SetPageKind(PK_NOTES);

            
            sal_uInt16 nMasterPageAfterPagesMasterPage = (pPage->TRG_GetMasterPage()).GetPageNum() + 1;
            pNotesPage->TRG_SetMasterPage(*GetMasterPage(nMasterPageAfterPagesMasterPage));
        }

        bOK = sal_True;
        StopWorkStartupDelay();
        SetChanged(false);
    }

    return(bOK);
}




sal_Bool SdDrawDocument::MovePages(sal_uInt16 nTargetPage)
{
    SdPage* pPage              = NULL;
    sal_uInt16  nPage;
    sal_uInt16  nNoOfPages         = GetSdPageCount(PK_STANDARD);
    sal_Bool    bSomethingHappened = sal_False;

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(SD_RESSTR(STR_UNDO_MOVEPAGES));

    
    std::vector<SdPage*> aPageList;
    for (nPage = 0; nPage < nNoOfPages; nPage++)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);

        if (pPage->IsSelected())
            aPageList.push_back(pPage);
    }

    
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
    
    else
    {
        nTargetPage = nPage;
        nTargetPage = 2 * nTargetPage + 1;    

        std::vector<SdPage*>::iterator iter;
        for (iter = aPageList.begin(); iter != aPageList.end(); ++iter)
        {
            pPage = *iter;
            nPage = pPage->GetPageNum();
            if (nPage > nTargetPage)
            {
                nTargetPage += 2;        

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



sal_uLong SdDrawDocument::GetLinkCount()
{
    return pLinkManager->GetLinks().size();
}


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
        GetDrawOutliner().SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
        pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
        pItemPool->SetPoolDefaultItem( SvxLanguageItem( eLang, nId ) );
        SetChanged( bChanged );
    }
}



LanguageType SdDrawDocument::GetLanguage( const sal_uInt16 nId ) const
{
    LanguageType eLangType = meLanguage;

    if( nId == EE_CHAR_LANGUAGE_CJK )
        eLangType = meLanguageCJK;
    else if( nId == EE_CHAR_LANGUAGE_CTL )
        eLangType = meLanguageCTL;

    return eLangType;
}



IMPL_LINK_NOARG(SdDrawDocument, WorkStartupHdl)
{
    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_True );

    sal_Bool bChanged = IsChanged();        

    
    SdPage* pHandoutMPage = GetMasterSdPage(0, PK_HANDOUT);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT6, sal_True, sal_True);
    }

    SdPage* pPage = GetSdPage(0, PK_STANDARD);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, sal_True, sal_True);
    }

    SdPage* pNotesPage = GetSdPage(0, PK_NOTES);

    if (pNotesPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True);
    }

    SetChanged(bChanged);

    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_False );
    return 0;
}





void SdDrawDocument::StopWorkStartupDelay()
{
    if (mpWorkStartupTimer)
    {
        if ( mpWorkStartupTimer->IsActive() )
        {
            
            mpWorkStartupTimer->Stop();
            WorkStartupHdl(NULL);
        }

        delete mpWorkStartupTimer;
        mpWorkStartupTimer = NULL;
    }
}




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
    DBG_ASSERT(pObject, "Without an object there is no IMapInfo");

    SdrObjUserData* pUserData = NULL;
    SdIMapInfo*     pIMapInfo = NULL;
    sal_uInt16          nCount = pObject->GetUserDataCount();

    
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

        
        if ( pObj->ISA( SdrGrafObj )  ) 
        {
            const SdrGrafObj*   pGrafObj = (const SdrGrafObj*) pObj;
            const GeoStat&      rGeo = pGrafObj->GetGeoStat();
            SdrGrafObjGeoData*  pGeoData = (SdrGrafObjGeoData*) pGrafObj->GetGeoData();

            
            if ( rGeo.nDrehWink )
                RotatePoint( aRelPoint, rRect.TopLeft(), -rGeo.nSin, rGeo.nCos );

            
            if ( pGeoData->bMirrored )
                aRelPoint.X() = rRect.Right() + rRect.Left() - aRelPoint.X();

            
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
        else if ( pObj->ISA( SdrOle2Obj ) ) 
        {
            aGraphSize = ( (SdrOle2Obj*) pObj )->GetOrigObjSize();
            bObjSupported = sal_True;
        }

        
        if ( bObjSupported )
        {
            
            aRelPoint -= rRect.TopLeft();
            pIMapObj = rImageMap.GetHitIMapObject( aGraphSize, rRect.GetSize(), aRelPoint );

            
            if ( pIMapObj && !pIMapObj->IsActive() )
                pIMapObj = NULL;
        }
    }

    return pIMapObj;
}

ImageMap* SdDrawDocument::GetImageMapForObject(SdrObject* pObj)
{
    SdIMapInfo* pIMapInfo = GetIMapInfo( pObj );
    if ( pIMapInfo )
    {
        return const_cast<ImageMap*>( &(pIMapInfo->GetImageMap()) );
    }
    return NULL;
}

/** this method enforces that the masterpages are in the currect order,
    that is at position 1 is a PK_STANDARD masterpage followed by a
    PK_NOTES masterpage and so on. #
*/
void SdDrawDocument::CheckMasterPages()
{
    sal_uInt16 nMaxPages = GetMasterPageCount();

    
    if( nMaxPages < 2 )
    {
        return;
    }

    SdPage* pPage = NULL;
    SdPage* pNotesPage = NULL;

    sal_uInt16 nPage;

    
    for( nPage = 1; nPage < nMaxPages; nPage++ )
    {
        pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
        
        if( ((1 == (nPage & 1)) && (pPage->GetPageKind() != PK_STANDARD) ) ||
            ((0 == (nPage & 1)) && (pPage->GetPageKind() != PK_NOTES) ) )
            break; 
    }

    if( nPage < nMaxPages )
    {
        
        
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
                        pPage->SetInserted(true);
                        break;

                    }

                    nFound++;
                }

                
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
                        pNotesPage->SetInserted(true);
                        break;
                    }

                    nFound++;
                }

                
                if( nMaxPages == nFound )
                {
                    

                    
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

                    SdPage* pNewNotesPage = static_cast<SdPage*>(AllocPage(true));
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

        
        while( nPage < nMaxPages )
        {
            bChanged = sal_True;

            RemoveMasterPage( nPage );
            nMaxPages--;
        }

        if( bChanged )
        {
            OSL_FAIL( "master pages where in a wrong order" );
            RecalcPageNums( true);
        }
    }
}

sal_uInt16 SdDrawDocument::CreatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const OUString& sStandardPageName,
    const OUString& sNotesPageName,
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

    
    pStandardPage = (SdPage*) AllocPage(false);

    
    
    pStandardPage->SetSize( pPreviousStandardPage->GetSize() );
    pStandardPage->SetBorder( pPreviousStandardPage->GetLftBorder(),
                              pPreviousStandardPage->GetUppBorder(),
                              pPreviousStandardPage->GetRgtBorder(),
                              pPreviousStandardPage->GetLwrBorder() );

    
    pStandardPage->TRG_SetMasterPage(pPreviousStandardPage->TRG_GetMasterPage());

    
    pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
    pStandardPage->SetAutoLayout(eStandardLayout, sal_True);
    pStandardPage->setHeaderFooterSettings( pPreviousStandardPage->getHeaderFooterSettings() );

    
    pStandardPage->setTransitionType( pPreviousStandardPage->getTransitionType() );
    pStandardPage->setTransitionSubtype( pPreviousStandardPage->getTransitionSubtype() );
    pStandardPage->setTransitionDirection( pPreviousStandardPage->getTransitionDirection() );
    pStandardPage->setTransitionFadeColor( pPreviousStandardPage->getTransitionFadeColor() );
    pStandardPage->setTransitionDuration( pPreviousStandardPage->getTransitionDuration() );

    
    pStandardPage->SetPresChange( pPreviousStandardPage->GetPresChange() );
    pStandardPage->SetTime( pPreviousStandardPage->GetTime() );

    
    pNotesPage = (SdPage*) AllocPage(false);
    pNotesPage->SetPageKind(PK_NOTES);

    
    pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());

    
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

    
    SdPage* pActualPage = GetSdPage(nPageNum, ePageKind);

    
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
    sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

    return DuplicatePage (
        pActualPage, ePageKind,
        
        OUString(), OUString(),
        aVisibleLayers.IsSet(aBckgrnd),
        aVisibleLayers.IsSet(aBckgrndObj));
}




sal_uInt16 SdDrawDocument::DuplicatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const OUString& sStandardPageName,
    const OUString& sNotesPageName,
    sal_Bool bIsPageBack,
    sal_Bool bIsPageObj,
    const sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    SdPage* pStandardPage;
    SdPage* pNotesPage;

    
    
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
    const OUString& sStandardPageName,
    const OUString& sNotesPageName,
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
    OUString aStandardPageName(sStandardPageName);
    OUString aNotesPageName(sNotesPageName);

    
    
    
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

    
    SetupNewPage (
        pPreviousStandardPage,
        pStandardPage,
        aStandardPageName,
        nInsertPosition,
        bIsPageBack,
        bIsPageObj);

    
    pNotesPage->SetPageKind(PK_NOTES);
    SetupNewPage (
        pPreviousNotesPage,
        pNotesPage,
        aNotesPageName,
        nInsertPosition+1,
        bIsPageBack,
        bIsPageObj);

    
    
    return pStandardPage->GetPageNum() / 2;
}




void SdDrawDocument::SetupNewPage (
    SdPage* pPreviousPage,
    SdPage* pPage,
    const OUString& sPageName,
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
        sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
        sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
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
