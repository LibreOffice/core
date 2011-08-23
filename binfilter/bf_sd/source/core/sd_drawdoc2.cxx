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

#ifdef _MSC_VER
#pragma hdrstop
#endif


#ifndef SVX_LIGHT
#ifndef _SFX_PRINTER_HXX
#include <bf_sfx2/printer.hxx>
#endif
#include "sdoutl.hxx"
#else	// SVX_LIGHT
#define SfxPrinter Printer
#endif // !SVX_LIGHT

#ifndef _SVX_PAPERINF_HXX
#include <bf_svx/paperinf.hxx>
#endif
#ifndef _SVDOPAGE_HXX //autogen
#include <bf_svx/svdopage.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <bf_svx/eeitem.hxx>
#endif
#ifndef _EEITEMID_HXX //autogen
#include <bf_svx/eeitemid.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <bf_svtools/itempool.hxx>
#endif


#include <bf_svx/dialogs.hrc>


#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "cusshow.hxx"

#include "bf_sd/docshell.hxx"
#include "bf_sd/frmview.hxx"

namespace binfilter {

SdPage* SdDrawDocument::GetSdPage(USHORT nPgNum, PageKind ePgKind) const
{
    SdPage* pPage = NULL;
    SdPage* pPageFound = NULL;

    USHORT nPage = 0;
    const USHORT nMaxPages = GetPageCount();
    USHORT nPageNum = 0;

    while (nPage < nMaxPages && !pPageFound)
    {
        pPage = (SdPage*) GetPage(nPage);

        if (pPage && pPage->GetPageKind() == ePgKind)
        {
             if (nPageNum == nPgNum)
             {
                 pPageFound = pPage;
             }
 
             nPageNum++;
        }

        nPage++;
    }

    return (pPageFound);
}


USHORT SdDrawDocument::GetSdPageCount(PageKind ePgKind) const
{
    SdPage* pPage = NULL;

    USHORT nPage;
    const USHORT nMaxPages = GetPageCount();
    USHORT nPageNum = 0;

    for (nPage = 0; nPage < nMaxPages; nPage++)
    {
        pPage = (SdPage*) GetPage(nPage);

        if (pPage && pPage->GetPageKind() == ePgKind)
        {
            nPageNum++;
        }
    }

    return (nPageNum);
}


SdPage* SdDrawDocument::GetMasterSdPage(USHORT nPgNum, PageKind ePgKind)
{
    SdPage* pPage = NULL;
    SdPage* pPageFound = NULL;

    USHORT nPage = 0;
    const USHORT nMaxPages = GetMasterPageCount();
    USHORT nPageNum = 0;

    while (nPage < nMaxPages && !pPageFound)
    {
        pPage = (SdPage*) GetMasterPage(nPage);

        if (pPage && pPage->GetPageKind() == ePgKind)
        {
            if (nPageNum == nPgNum)
            {
                pPageFound = pPage;
            }

            nPageNum++;
        }

        nPage++;
    }

     return (pPageFound);
}

USHORT SdDrawDocument::GetMasterSdPageCount(PageKind ePgKind) const
{
    SdPage* pPage = NULL;

    USHORT nPage;
    const USHORT nMaxPages = GetMasterPageCount();
    USHORT nPageNum = 0;

    for (nPage = 0; nPage < nMaxPages; nPage++)
    {
        pPage = (SdPage*) GetMasterPage(nPage);

        if (pPage && pPage->GetPageKind() == ePgKind)
        {
            nPageNum++;
        }
    }

    return (nPageNum);
}

void SdDrawDocument::UpdatePageObjectsInNotes(USHORT nStartPos)
{
    USHORT	nPageCount	= GetPageCount();
    SdPage* pPage		= NULL;

    for (USHORT nPage = nStartPos; nPage < nPageCount; nPage++)
    {
        pPage = (SdPage*)GetPage(nPage);

        // wenn es eine Notizseite ist, Seitenobjekt suchen
        // und Nummer korrigieren
        if (pPage && pPage->GetPageKind() == PK_NOTES)
         {
             ULONG nObjCount = pPage->GetObjCount();
             SdrObject* pObj = NULL;
             for (ULONG nObj = 0; nObj < nObjCount; nObj++)
             {
/*?*/ 				pObj = pPage->GetObj(nObj);
/*?*/ 				if (pObj->GetObjIdentifier() == OBJ_PAGE &&
/*?*/ 					pObj->GetObjInventor() == SdrInventor)
/*?*/ 				{
                    // das Seitenobjekt stellt die vorhergende Seite (also
                    // die Zeichenseite) dar
/*?*/ 					DBG_ASSERTWARNING(nStartPos, "Notizseitenpos. darf nicht 0 sein");

/*?*/ 					DBG_ASSERTWARNING(nPage > 1, "Seitenobjekt darf nicht Handzettel darstellen");

/*?*/ 					if (nStartPos > 0 && nPage > 1)
/*?*/ 						((SdrPageObj*)pObj)->SetPageNum(nPage - 1);
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
    }
}

void SdDrawDocument::InsertPage(SdrPage* pPage, USHORT nPos)
{
    FmFormModel::InsertPage(pPage, nPos);

    UpdatePageObjectsInNotes(nPos);
}

void SdDrawDocument::DeletePage(USHORT nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}

SdrPage* SdDrawDocument::RemovePage(USHORT nPgNum)
{
    SdrPage* pPage = FmFormModel::RemovePage(nPgNum);


    if (pCustomShowList)
    {
        for (ULONG i = 0; i < pCustomShowList->Count(); i++)
        {
            SdCustomShow* pCustomShow = (SdCustomShow*) pCustomShowList->GetObject(i);
            pCustomShow->Remove(pPage);
        }
    }

    UpdatePageObjectsInNotes(nPgNum);

    return pPage;
}

void SdDrawDocument::CreateFirstPages()
{
    USHORT nPageCount = GetPageCount();

    if (nPageCount <= 1)
    {
        Size aDefSize(21000, 29700);   // A4-Hochformat

        BOOL bMasterPage;
        SdPage* pHandoutPage = (SdPage*) AllocPage(bMasterPage=FALSE);

        // Stets Querformat
        if (aDefSize.Height() <= aDefSize.Width())
        {
            pHandoutPage->SetSize(aDefSize);
        }
        else
        {
            pHandoutPage->SetSize( Size(aDefSize.Height(), aDefSize.Width()) );
        }

        pHandoutPage->SetBorder(2000, 2000, 2000, 2000);
        pHandoutPage->SetPageKind(PK_HANDOUT);
        pHandoutPage->SetName( String (SdResId(STR_HANDOUT) ) );
        InsertPage(pHandoutPage, 0);

        SdPage* pHandoutMPage = (SdPage*) AllocPage(bMasterPage=TRUE);
        pHandoutMPage->SetSize( pHandoutPage->GetSize() );
        pHandoutMPage->SetPageKind(PK_HANDOUT);
        pHandoutMPage->SetBorder( pHandoutPage->GetLftBorder(),
                                  pHandoutPage->GetUppBorder(),
                                  pHandoutPage->GetRgtBorder(),
                                  pHandoutPage->GetLwrBorder() );
        InsertMasterPage(pHandoutMPage, 0);
        pHandoutPage->InsertMasterPage( pHandoutMPage->GetPageNum() );

        SdPage* pPage;
        BOOL bClipboard = FALSE;

        if (nPageCount == 0)
        {
            pPage = (SdPage*) AllocPage(bMasterPage=FALSE);

            if (eDocType == DOCUMENT_TYPE_DRAW)
            {
                // Draw: stets Default-Groesse mit Raendern
                pPage->SetSize(aDefSize);
                pPage->SetBorder(0, 0, 0, 0);
            }
            else
            {
                 // Impress: stets Bildschirmformat
                pPage->SetSize( Size(28000, 21000) );   // Bildschirmformat
                pPage->SetBorder(0, 0, 0, 0);
            }

            InsertPage(pPage, 1);
        }
         else
        {
            bClipboard = TRUE;
            pPage = (SdPage*) GetPage(1);
        }

        /**********************************************************************
        * MasterPage einfuegen und an der Seite vermerken
        **********************************************************************/
        SdPage* pMPage = (SdPage*) AllocPage(bMasterPage=TRUE);
        pMPage->SetSize( pPage->GetSize() );
        pMPage->SetBorder( pPage->GetLftBorder(),
                           pPage->GetUppBorder(),
                           pPage->GetRgtBorder(),
                           pPage->GetLwrBorder() );
        InsertMasterPage(pMPage, 1);
        pPage->InsertMasterPage( pMPage->GetPageNum() );
        if( bClipboard )
/*?*/ 			pMPage->SetLayoutName( pPage->GetLayoutName() );

        /**********************************************************************
        * Notizen-Seite einfuegen
        **********************************************************************/
        SdPage* pNotesPage = (SdPage*) AllocPage(bMasterPage=FALSE);

        // Stets Hochformat
        if (aDefSize.Height() >= aDefSize.Width())
        {
            pNotesPage->SetSize(aDefSize);
        }
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pNotesPage->SetSize( Size(aDefSize.Height(), aDefSize.Width()) );
/*?*/ 		}

        pNotesPage->SetBorder(2000, 2000, 2000, 2000);
        pNotesPage->SetPageKind(PK_NOTES);
        InsertPage(pNotesPage, 2);
        if( bClipboard )
/*?*/ 			pNotesPage->SetLayoutName( pPage->GetLayoutName() );

        /**********************************************************************
        * MasterPage einfuegen und an der Notizen-Seite vermerken
        **********************************************************************/
        SdPage* pNotesMPage = (SdPage*) AllocPage(bMasterPage=TRUE);
        pNotesMPage->SetSize( pNotesPage->GetSize() );
        pNotesMPage->SetPageKind(PK_NOTES);
        pNotesMPage->SetBorder( pNotesPage->GetLftBorder(),
                                pNotesPage->GetUppBorder(),
                                pNotesPage->GetRgtBorder(),
                                pNotesPage->GetLwrBorder() );
        InsertMasterPage(pNotesMPage, 2);
        pNotesPage->InsertMasterPage( pNotesMPage->GetPageNum() );
        if( bClipboard )
/*?*/ 			pNotesMPage->SetLayoutName( pPage->GetLayoutName() );

    WorkStartupHdl();

    SetChanged(FALSE);
    }
}

void SdDrawDocument::SetLanguage( const LanguageType eLang, const USHORT nId )
{
    BOOL bChanged = FALSE;

    if( nId == EE_CHAR_LANGUAGE && eLanguage != eLang )
    {
        eLanguage = eLang;
        bChanged = TRUE;
    }
    else if( nId == EE_CHAR_LANGUAGE_CJK && eLanguageCJK != eLang )
    {
         eLanguageCJK = eLang;
         bChanged = TRUE;
     }
     else if( nId == EE_CHAR_LANGUAGE_CTL && eLanguageCTL != eLang )
     {
         eLanguageCTL = eLang;
         bChanged = TRUE;
     }

    if( bChanged )
    {
        GetDrawOutliner().SetDefaultLanguage( Application::GetSettings().GetLanguage() );
        pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguage() );
        pItemPool->SetPoolDefaultItem( SvxLanguageItem( eLang, nId ) );
        SetChanged( bChanged );
    }
}


LanguageType SdDrawDocument::GetLanguage( const USHORT nId ) const
{
    LanguageType eLangType = eLanguage;

    if( nId == EE_CHAR_LANGUAGE_CJK )
        eLangType = eLanguageCJK;
    else if( nId == EE_CHAR_LANGUAGE_CTL )
        eLangType = eLanguageCTL;

    return eLangType;
}


void SdDrawDocument::WorkStartupHdl()
{
    BOOL bChanged = IsChanged();

    SdPage* pHandoutMPage = GetMasterSdPage(0, PK_HANDOUT);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT4, TRUE);

    SdPage* pPage = GetSdPage(0, PK_STANDARD);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, TRUE);

    SdPage* pNotesPage = GetSdPage(0, PK_NOTES);

    if (pNotesPage->GetAutoLayout() == AUTOLAYOUT_NONE)
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, TRUE);

    SetChanged(bChanged || FALSE);
}

SdAnimationInfo* SdDrawDocument::GetAnimationInfo(SdrObject* pObject) const
{
    DBG_ASSERT(pObject, "ohne Objekt keine AnimationsInfo");
    USHORT nUD			= 0;
    USHORT nUDCount 	= pObject->GetUserDataCount();
    SdrObjUserData* pUD = NULL;

    // gibt es in den User-Daten eine Animationsinformation?
    for (nUD = 0; nUD < nUDCount; nUD++)
    {
        pUD = pObject->GetUserData(nUD);
        if (pUD->GetInventor() == SdUDInventor	 &&
            pUD->GetId() == SD_ANIMATIONINFO_ID)
        {
            return (SdAnimationInfo*)pUD;
        }
    }
    return NULL;
}

SdIMapInfo* SdDrawDocument::GetIMapInfo( SdrObject* pObject ) const
{
    DBG_ASSERT(pObject, "ohne Objekt keine IMapInfo");

    SdrObjUserData* pUserData = NULL;
    SdIMapInfo* 	pIMapInfo = NULL;
    USHORT			nCount = pObject->GetUserDataCount();

    // gibt es in den User-Daten eine IMap-Information?
    for ( USHORT i = 0; i < nCount; i++ )
    {
        pUserData = pObject->GetUserData( i );

        if ( ( pUserData->GetInventor() == SdUDInventor ) && ( pUserData->GetId() == SD_IMAPINFO_ID ) )
            pIMapInfo = (SdIMapInfo*) pUserData;
    }

    return pIMapInfo;
}

void SdDrawDocument::CheckMasterPages()
{
    USHORT nMaxPages = GetMasterPageCount();

    if( nMaxPages < 2 )
    {
        return;
    }

    SdPage* pPage = NULL;
    SdPage* pNotesPage = NULL;

    USHORT nPage;

    for( nPage = 1; nPage < nMaxPages; nPage++ )
    {
        pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
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
                USHORT nFound = nPage + 1;
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

                USHORT nFound = nPage + 1;
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

                    SdPage* pNotesPage = static_cast<SdPage*>(AllocPage(sal_True));
                    pNotesPage->SetPageKind(PK_NOTES);
                    if( pRefNotesPage )
                    {
                        pNotesPage->SetSize( pRefNotesPage->GetSize() );
                        pNotesPage->SetBorder( pRefNotesPage->GetLftBorder(),
                                                pRefNotesPage->GetUppBorder(),
                                                pRefNotesPage->GetRgtBorder(),
                                                pRefNotesPage->GetLwrBorder() );
                    }
                    InsertMasterPage(pNotesPage,  nPage );
                    pNotesPage->SetLayoutName( pPage->GetLayoutName() );
                    pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True);
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
            DBG_ERROR( "master pages where in a wrong order" );
            RecalcPageNums( sal_True);
        }
    }
}

}
