/*************************************************************************
 *
 *  $RCSfile: drawdoc2.cxx,v $
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

#pragma hdrstop

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef SVX_LIGHT
#include <so3/iface.hxx>
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#include "sdoutl.hxx"
#else   // SVX_LIGHT
#ifndef _OUTLINER_HXX //autogen wg. Outliner
#include <svx/outliner.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen wg. Outliner
#include <svx/svdoutl.hxx>
#endif
#define SfxPrinter Printer
#endif // !SVX_LIGHT

#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif
#ifndef _SVDOPAGE_HXX //autogen
#include <svx/svdopage.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


#include <svx/linkmgr.hxx>
#include <svx/editdata.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>                  // SVX_RESSTR

#include "eetext.hxx"

#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX //autogen
#include <svtools/imapobj.hxx>
#endif


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

#ifndef SVX_LIGHT
#ifdef MAC
#include "::ui:inc:docshell.hxx"
#include "::ui:inc:frmview.hxx"
#include "::ui:inc:cfgids.hxx"
#include "::ui:inc:strings.hrc"
#else
#ifdef UNX
#include "../ui/inc/docshell.hxx"
#include "../ui/inc/frmview.hxx"
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"
#else
#include "..\ui\inc\docshell.hxx"
#include "..\ui\inc\frmview.hxx"
#include "..\ui\inc\cfgids.hxx"
#include "..\ui\inc\strings.hrc"
#endif
#endif

extern USHORT SlotArray[];
#endif // !SVX_LIGHT

const long PRINT_OFFSET = 30;       // siehe \svx\source\dialog\page.cxx (PB)


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
    USHORT nPage = 0;
    const USHORT nMaxPages = GetPageCount();

    while (nPage < nMaxPages && !pObjFound)
    {
        pPage = (SdPage*) GetPage(nPage);
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if (pObj->GetName() == rObjName)
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
    const USHORT nMaxMasterPages = GetMasterPageCount();

    while (nPage < nMaxMasterPages && !pObjFound)
    {
        pPage = (SdPage*) GetMasterPage(nPage);
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if (pObj->GetName() == rObjName)
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

USHORT SdDrawDocument::GetPageByName(const String& rPgName) const
{
    SdPage* pPage = NULL;
    SdPage* pPageFound = NULL;
    USHORT nPage = 0;
    const USHORT nMaxPages = GetPageCount();
    USHORT nPageNum = SDRPAGE_NOTFOUND;

    /**************************************************************************
    * Zuerst alle Pages durchsuchen
    **************************************************************************/
    while (nPage < nMaxPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = (SdPage*) GetPage(nPage);

        if (pPage && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
        }

        nPage++;
    }

    /**************************************************************************
    * Wenn nicht gefunden, dann alle MasterPages durchsuchen
    **************************************************************************/
    const USHORT nMaxMasterPages = GetMasterPageCount();
    nPage = 0;

    while (nPage < nMaxMasterPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = (SdPage*) GetMasterPage(nPage);

        if (pPage && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
        }

        nPage++;
    }

    return (nPageNum);
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

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

/*************************************************************************
|*
|*  die in den Seitenobjekten der Notizseiten eingetragenen
|*  Seitennummern anpassen
|*
\************************************************************************/

void SdDrawDocument::UpdatePageObjectsInNotes(USHORT nStartPos)
{
    USHORT  nPageCount  = GetPageCount();
    SdPage* pPage       = NULL;

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
                pObj = pPage->GetObj(nObj);
                if (pObj->GetObjIdentifier() == OBJ_PAGE &&
                    pObj->GetObjInventor() == SdrInventor)
                {
                    // das Seitenobjekt stellt die vorhergende Seite (also
                    // die Zeichenseite) dar
                    DBG_ASSERTWARNING(nStartPos, "Notizseitenpos. darf nicht 0 sein");

                    DBG_ASSERTWARNING(nPage > 1, "Seitenobjekt darf nicht Handzettel darstellen");

                    if (nStartPos > 0 && nPage > 1)
                        ((SdrPageObj*)pObj)->SetPageNum(nPage - 1);
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

void __EXPORT SdDrawDocument::MovePage(USHORT nPgNum, USHORT nNewPos)
{
    // Seite verschieben
    FmFormModel::MovePage(nPgNum, nNewPos);

    USHORT nMin = Min(nPgNum, nNewPos);

    UpdatePageObjectsInNotes(nMin);
}

/*************************************************************************
|*
|*  Seite einfuegen
|*
\************************************************************************/

void __EXPORT SdDrawDocument::InsertPage(SdrPage* pPage, USHORT nPos)
{
    FmFormModel::InsertPage(pPage, nPos);

#ifndef SVX_LIGHT
    ((SdPage*)pPage)->ConnectLink();
#endif

    UpdatePageObjectsInNotes(nPos);
}

/*************************************************************************
|*
|*  Seite loeschen
|*
\************************************************************************/

void __EXPORT SdDrawDocument::DeletePage(USHORT nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}

/*************************************************************************
|*
|*  Seite entfernen
|*
\************************************************************************/

SdrPage* __EXPORT SdDrawDocument::RemovePage(USHORT nPgNum)
{
    SdrPage* pPage = FmFormModel::RemovePage(nPgNum);

#ifndef SVX_LIGHT
    ((SdPage*)pPage)->DisconnectLink();
#endif

    if (pCustomShowList)
    {
        for (ULONG i = 0; i < pCustomShowList->Count(); i++)
        {
            // Ggf. Seite aus der CustomShows ausfuegen
            SdCustomShow* pCustomShow = (SdCustomShow*) pCustomShowList->GetObject(i);
            pCustomShow->Remove(pPage);
        }
    }

    UpdatePageObjectsInNotes(nPgNum);

    return pPage;
}

/*************************************************************************
|*
|* anfallenden Undo-Aktionen des SdrModels verwalten
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SdDrawDocument, NotifyUndoActionHdl, SfxUndoAction *, pUndoAction )
{
#ifndef SVX_LIGHT
    if (pUndoAction)
    {
        if (pDeletedPresObjList && pDeletedPresObjList->Count())
        {
            // Praesentationsobjekte sollen geloescht werden
            ULONG nCount = pDeletedPresObjList->Count();

            for (ULONG i=0; i < nCount; i++)
            {
                SdrObject* pObj = (SdrObject*) pDeletedPresObjList->GetObject(i);

                if (pObj && !pObj->IsEmptyPresObj())
                {
                    // Objekt ist kein leeres Praesentationsobjekt -> austauschen
                    if (pObj->ISA(SdrTextObj))
                    {
                        SdrTextObj* pTextObj = (SdrTextObj*) pObj;

                        if (pTextObj)
                        {
                            String aString;
                            SdPage* pPage = (SdPage*) pTextObj->GetUserCall();

                            if (pPage)
                            {
                                PresObjKind ePresObjKind = pPage->GetPresObjKind(pTextObj);
                                PageKind ePageKind = pPage->GetPageKind();

                                List* pPresObjList = pPage->GetPresObjList();
                                pPresObjList->Remove((void*) pTextObj);
                                pTextObj->SetUserCall(NULL);

                                if (ePresObjKind == PRESOBJ_TITLE)
                                {
                                    if ( pPage->IsMasterPage() )
                                    {
                                        if (ePageKind != PK_NOTES)
                                        {
                                            aString = String ( SdResId( STR_PRESOBJ_MPTITLE ) );
                                        }
                                        else
                                        {
                                            aString = String ( SdResId( STR_PRESOBJ_MPNOTESTITLE ) );
                                        }
                                    }
                                    else
                                    {
                                        aString = String (SdResId(STR_PRESOBJ_TITLE));
                                    }
                                }
                                else if (ePresObjKind == PRESOBJ_OUTLINE)
                                {
                                    ePresObjKind = PRESOBJ_OUTLINE;

                                    if ( pPage->IsMasterPage() )
                                    {
                                        aString = String (SdResId(STR_PRESOBJ_MPOUTLINE));
                                    }
                                    else
                                    {
                                        aString = String (SdResId(STR_PRESOBJ_OUTLINE));
                                    }
                                }
                                else if (ePresObjKind == PRESOBJ_NOTES)
                                {
                                    if ( pPage->IsMasterPage() )
                                    {
                                        aString = String ( SdResId( STR_PRESOBJ_MPNOTESTEXT ) );
                                    }
                                    else
                                    {
                                        aString = String ( SdResId( STR_PRESOBJ_NOTESTEXT ) );
                                    }
                                }
                                else if (ePresObjKind == PRESOBJ_TEXT)
                                {
                                    aString = String ( SdResId( STR_PRESOBJ_TEXT ) );
                                }

                                if ( aString.Len() )
                                {
                                    String aEmptyStr;
                                    GetInternalOutliner(TRUE);
                                    pInternalOutliner->SetMinDepth(0);
                                    pPage->SetObjText( pTextObj, pInternalOutliner, ePresObjKind, aString );
                                    pTextObj->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), TRUE );
                                    OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
                                    pInternalOutliner->SetText(*pParaObj);

                                    pParaObj = pInternalOutliner->CreateParaObject();

                                    SdrTextObj* pNewTextObj = (SdrTextObj*) pTextObj->Clone();
                                    pNewTextObj->SetOutlinerParaObject(pParaObj);
                                    pNewTextObj->SetEmptyPresObj(TRUE);
                                    pNewTextObj->SetUserCall(pPage);
                                    List* pPresObjList = pPage->GetPresObjList();
                                    pPresObjList->Insert(pNewTextObj, LIST_APPEND);
                                    pPage->InsertObject(pNewTextObj);
//                                    pNewTextObj->SetStyleSheet(pTitleSheet, TRUE);
                                    pUndoAction->Merge(new SdrUndoNewObj(*pNewTextObj));

                                    pInternalOutliner->Clear();
                                    pInternalOutliner->SetMinDepth(0);
                                }
                            }
                        }
                    }
                }
            }

            delete pDeletedPresObjList;
            pDeletedPresObjList = NULL;
        }

        if (pDocSh)
        {
            pDocSh->GetUndoManager()->AddUndoAction(pUndoAction);
        }
    }

#endif // !SVX_LIGHT
    return 0;
}
IMPL_LINK_INLINE_END( SdDrawDocument, NotifyUndoActionHdl, SfxUndoAction *, pUndoAction )

/*************************************************************************
|*
|* Seiten selektieren
|*
\************************************************************************/

void SdDrawDocument::SetSelected(SdPage* pPage, BOOL bSelect)
{
    PageKind ePageKind = pPage->GetPageKind();

    if (ePageKind == PK_STANDARD)
    {
        pPage->SetSelected(bSelect);
        SdPage* pNotesPage = (SdPage*) GetPage( pPage->GetPageNum() + 1 );

        if (pNotesPage && pNotesPage->GetPageKind() == PK_NOTES)
            pNotesPage->SetSelected(bSelect);
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

void SdDrawDocument::CreateFirstPages()
{
    /**************************************************************************
    * Wenn noch keine Seite im Model vorhanden ist (Datei-Neu), wird
    * eine neue Seite eingefuegt
    **************************************************************************/
    USHORT nPageCount = GetPageCount();

    if (nPageCount <= 1)
    {
        /**********************************************************************
        * Default-Papiergroesse vom Drucker holen
        **********************************************************************/
        SfxPrinter* pPrinter = NULL;
        Size aDefSize(21000, 29700);   // A4-Hochformat

        if (pDocSh)
        {
            pPrinter = pDocSh->GetPrinter(TRUE);

            if (pPrinter->IsValid())
            {
                // Der Printer gibt leider kein exaktes Format (z.B. A4) zurueck
                aDefSize = pPrinter->GetPaperSize();
                SvxPaper ePaper = SvxPaperInfo::GetSvxPaper(aDefSize, MAP_100TH_MM, TRUE);

                if (ePaper != SVX_PAPER_USER)
                {
                    // Korrekte Size holen
                    aDefSize = SvxPaperInfo::GetPaperSize(ePaper, MAP_100TH_MM);
                }
            }
        }

        /**********************************************************************
        * Handzettel-Seite einfuegen
        **********************************************************************/
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

        /**********************************************************************
        * MasterPage einfuegen und an der Handzettel-Seite vermerken
        **********************************************************************/
        SdPage* pHandoutMPage = (SdPage*) AllocPage(bMasterPage=TRUE);
        pHandoutMPage->SetSize( pHandoutPage->GetSize() );
        pHandoutMPage->SetPageKind(PK_HANDOUT);
        pHandoutMPage->SetBorder( pHandoutPage->GetLftBorder(),
                                  pHandoutPage->GetUppBorder(),
                                  pHandoutPage->GetRgtBorder(),
                                  pHandoutPage->GetLwrBorder() );
        InsertMasterPage(pHandoutMPage, 0);
        pHandoutPage->InsertMasterPage( pHandoutMPage->GetPageNum() );

        /**********************************************************************
        * Seite einfuegen
        * Sofern nPageCount==1 ist, wurde das Model fuers Clipboad erzeugt.
        * Eine Standard-Seite ist daher schon vorhanden.
        **********************************************************************/
        SdPage* pPage;
        BOOL bClipboard = FALSE;

        if (nPageCount == 0)
        {
            pPage = (SdPage*) AllocPage(bMasterPage=FALSE);

            if (eDocType == DOCUMENT_TYPE_DRAW)
            {
                // Draw: stets Default-Groesse mit Raendern
                pPage->SetSize(aDefSize);

                if (pPrinter->IsValid())
                {
                    Size aOutSize(pPrinter->GetOutputSize());
                    Point aPageOffset(pPrinter->GetPageOffset());
                    aPageOffset -= pPrinter->PixelToLogic( Point() );
                    long nOffset = !aPageOffset.X() && !aPageOffset.X() ? 0 : PRINT_OFFSET;

                    ULONG nTop    = aPageOffset.Y();
                    ULONG nLeft   = aPageOffset.X();
                    ULONG nBottom = Max((long)(aDefSize.Height() - aOutSize.Height() - nTop + nOffset), 0L);
                    ULONG nRight  = Max((long)(aDefSize.Width() - aOutSize.Width() - nLeft + nOffset), 0L);

                    pPage->SetBorder(nLeft, nTop, nRight, nBottom);
                }
                else
                {
                    pPage->SetBorder(0, 0, 0, 0);
                }
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
            pMPage->SetLayoutName( pPage->GetLayoutName() );

        /**********************************************************************
        * Notizen-Seite einfuegen
        **********************************************************************/
        SdPage* pNotesPage = (SdPage*) AllocPage(bMasterPage=FALSE);

        // Stets Hochformat
        if (aDefSize.Height() >= aDefSize.Width())
        {
            pNotesPage->SetSize(aDefSize);
        }
        else
        {
            pNotesPage->SetSize( Size(aDefSize.Height(), aDefSize.Width()) );
        }

        pNotesPage->SetBorder(2000, 2000, 2000, 2000);
        pNotesPage->SetPageKind(PK_NOTES);
        InsertPage(pNotesPage, 2);
        if( bClipboard )
            pNotesPage->SetLayoutName( pPage->GetLayoutName() );

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
            pNotesMPage->SetLayoutName( pPage->GetLayoutName() );

        pWorkStartupTimer = new Timer();
        pWorkStartupTimer->SetTimeoutHdl( LINK(this, SdDrawDocument, WorkStartupHdl) );
        pWorkStartupTimer->SetTimeout(2000);
        pWorkStartupTimer->Start();

        SetChanged(FALSE);
    }
}

/*************************************************************************
|*
|* Erzeugt fehlende Notiz und Handzettelseiten (nach PowerPoint-Import)
|* Es wird davon ausgegangen, dass mindestens eine Standard-Seite und
|* eine Standard-MasterPage vorhanden sind.
|*
\************************************************************************/

BOOL SdDrawDocument::CreateMissingNotesAndHandoutPages()
{
    BOOL bOK = FALSE;
    USHORT nPageCount = GetPageCount();

    if (nPageCount != 0)
    {
        /**********************************************************************
        * PageKind setzen
        **********************************************************************/
        SdPage* pHandoutMPage = (SdPage*) GetMasterPage(0);
        pHandoutMPage->SetPageKind(PK_HANDOUT);

        SdPage* pHandoutPage = (SdPage*) GetPage(0);
        pHandoutPage->SetPageKind(PK_HANDOUT);
        pHandoutPage->InsertMasterPage(pHandoutMPage->GetPageNum());

        for (USHORT i = 1; i < nPageCount; i = i + 2)
        {
            SdPage* pPage = (SdPage*) GetPage(i);

            if (pPage->GetMasterPageCount() == 0)
            {
                // Keine MasterPage gesetzt -> erste Standard-MasterPage nehmen
                // (Wenn bei PPT keine Standard-Seite vorhanden war)
                pPage->InsertMasterPage(1);
            }

            SdPage* pNotesPage = (SdPage*) GetPage(i+1);
            pNotesPage->SetPageKind(PK_NOTES);

            // Notiz-MasterPages setzen
            pNotesPage->RemoveMasterPage(0);
            USHORT nNum = pPage->GetMasterPageNum(0) + 1;
            pNotesPage->InsertMasterPage(nNum);
        }

        bOK = TRUE;
        StopWorkStartupDelay();
        SetChanged(FALSE);
    }

    return(bOK);
}

/*************************************************************************
|*
|* - selektierte Seiten hinter genannte Seite schieben
|*   (nTargetPage = (USHORT)-1  --> vor erste Seite schieben)
|* - ergibt TRUE, wenn Seiten verschoben wurden
|*
\************************************************************************/

BOOL SdDrawDocument::MovePages(USHORT nTargetPage)
{
    SdPage* pTargetPage        = NULL;
    SdPage* pPage              = NULL;
    USHORT  nPage;
    USHORT  nNoOfPages         = GetSdPageCount(PK_STANDARD);
    BOOL    bSomethingHappened = FALSE;

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
    if (nPage != (USHORT)-1)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);
        while (nPage > 0 && pPage->IsSelected())
        {
            nPage--;
            pPage = GetSdPage(nPage, PK_STANDARD);
        }

        if (pPage->IsSelected())
        {
            nPage = (USHORT)-1;
        }
    }

    // vor der ersten Seite einfuegen
    if (nPage == (USHORT)-1)
    {
        while (aPageList.Count() > 0)
        {
            aPageList.Last();

            nPage = ( (SdPage*) aPageList.GetCurObject() )->GetPageNum();
            if (nPage != 0)
            {
                SdrPage* pPg = GetPage(nPage);
                AddUndo(new SdrUndoSetPageNum(*pPg, nPage, 1));
                MovePage(nPage, 1);
                pPg = GetPage(nPage+1);
                AddUndo(new SdrUndoSetPageNum(*pPg, nPage+1, 2));
                MovePage(nPage+1, 2);
                bSomethingHappened = TRUE;
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
            nPage = pPage->GetPageNum();
            if (nPage > nTargetPage)
            {
                nTargetPage += 2;        // hinter (!) der Seite einfuegen

                if (nPage != nTargetPage)
                {
                    SdrPage* pPg = GetPage(nPage);
                    AddUndo(new SdrUndoSetPageNum(*pPg, nPage, nTargetPage));
                    MovePage(nPage, nTargetPage);
                    pPg = GetPage(nPage+1);
                    AddUndo(new SdrUndoSetPageNum(*pPg, nPage+1, nTargetPage+1));
                    MovePage(nPage+1, nTargetPage+1);
                    bSomethingHappened = TRUE;
                }
            }
            else
            {
                if (nPage != nTargetPage)
                {
                    SdrPage* pPg = GetPage(nPage+1);
                    AddUndo(new SdrUndoSetPageNum(*pPg, nPage+1, nTargetPage+1));
                    MovePage(nPage+1, nTargetPage+1);
                    pPg = GetPage(nPage);
                    AddUndo(new SdrUndoSetPageNum(*pPg, nPage, nTargetPage));
                    MovePage(nPage, nTargetPage);
                    bSomethingHappened = TRUE;
                }
            }
            aPageList.Remove((ULONG)0);
            nTargetPage = pPage->GetPageNum();
        }
    }

    EndUndo();

    return bSomethingHappened;
}


/*************************************************************************
|*
|* Anzahl der Links im LinkManager zurueckgeben
|*
\************************************************************************/

ULONG SdDrawDocument::GetLinkCount()
{
    return ( pLinkManager->GetLinks().Count() );
}

/*************************************************************************
|*
|* Language zurueckgeben
|*
\************************************************************************/

LanguageType SdDrawDocument::GetLanguage() const
{
    return eLanguage;
}

/*************************************************************************
|*
|* Language setzen
|*
\************************************************************************/

void SdDrawDocument::SetLanguage( LanguageType eNewLang )
{
    if( eLanguage != eNewLang )
    {
        eLanguage = eNewLang;
        GetDrawOutliner().SetDefaultLanguage( eLanguage );
        pHitTestOutliner->SetDefaultLanguage( eLanguage );
        SetChanged( TRUE );
    }
}


/*************************************************************************
|*
|* WorkStartup einleiten
|*
\************************************************************************/

IMPL_LINK( SdDrawDocument, WorkStartupHdl, Timer *, pTimer )
{
#ifndef SVX_LIGHT
    if( pDocSh )
        pDocSh->SetWaitCursor( TRUE );

    BOOL bChanged = IsChanged();        // merken

    // Autolayouts initialisieren
    SdPage* pHandoutMPage = GetMasterSdPage(0, PK_HANDOUT);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT4, TRUE);
    }

    SdPage* pPage = GetSdPage(0, PK_STANDARD);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, TRUE);
    }

    SdPage* pNotesPage = GetSdPage(0, PK_NOTES);

    if (pNotesPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, TRUE);
    }

    SetChanged(bChanged || FALSE);

    // Um die Lupe und nicht die Seite als Bitmap des Zoom-Controllers
    // zu erhalten wird hier in das ehemals static array der Slot fuer
    // die Lupe gemapped. (SlotArray in drviewsc.cxx)
    SlotArray[ 5 ] = SID_ZOOM_OUT; // SID_ZOOM_TOOLBOX;

    if( pDocSh )
        pDocSh->SetWaitCursor( FALSE );
#endif
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
    if (pWorkStartupTimer)
    {
        if ( pWorkStartupTimer->IsActive() )
        {
            // Timer war noch nicht abgelaufen -> WorkStartup wird eingeleitet
            pWorkStartupTimer->Stop();
            WorkStartupHdl(NULL);
        }

        delete pWorkStartupTimer;
        pWorkStartupTimer = NULL;
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
    DBG_ASSERT(pObject, "ohne Objekt keine AnimationsInfo");
    USHORT nUD          = 0;
    USHORT nUDCount     = pObject->GetUserDataCount();
    SdrObjUserData* pUD = NULL;

    // gibt es in den User-Daten eine Animationsinformation?
    for (nUD = 0; nUD < nUDCount; nUD++)
    {
        pUD = pObject->GetUserData(nUD);
        if (pUD->GetInventor() == SdUDInventor   &&
            pUD->GetId() == SD_ANIMATIONINFO_ID)
        {
            return (SdAnimationInfo*)pUD;
        }
    }
    return NULL;
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
    USHORT          nCount = pObject->GetUserDataCount();

    // gibt es in den User-Daten eine IMap-Information?
    for ( USHORT i = 0; i < nCount; i++ )
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
                                              const Point& rWinPoint,
                                              const Window& rCmpWnd )
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
        BOOL                bObjSupported = FALSE;

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
            bObjSupported = TRUE;
        }
        else if ( pObj->ISA( SdrOle2Obj ) ) // OLE-Objekt
        {
#ifndef SVX_LIGHT
            SvInPlaceObjectRef aIPObjRef = ( (SdrOle2Obj*) pObj )->GetObjRef();

            if ( aIPObjRef.Is() )
            {
                aGraphSize = aIPObjRef->GetVisArea().GetSize();
                bObjSupported = TRUE;
            }
#endif // !SVX_LIGHT
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


/*************************************************************************
|*
|*
|*
\************************************************************************/

Graphic SdDrawDocument::GetGraphicFromOle2Obj( const SdrOle2Obj* pOle2Obj )
{
    Graphic             aGraphic;

#ifndef SVX_LIGHT
    SvInPlaceObjectRef  aIPObjRef = pOle2Obj->GetObjRef();

    if ( aIPObjRef.Is() )
    {
        VirtualDevice   aVDev;
        GDIMetaFile     aGDIMtf;
        const MapMode   aMap100( MAP_100TH_MM );
        const Size&     rSize = aIPObjRef->GetVisArea().GetSize();

        aVDev.SetMapMode( aMap100 );
        aGDIMtf.Record( &aVDev );

        aIPObjRef->DoDraw( &aVDev, Point(), rSize, JobSetup() );

        aGDIMtf.Stop();
        aGDIMtf.WindStart();
        aGDIMtf.SetPrefMapMode( aMap100 );
        aGDIMtf.SetPrefSize( rSize );
        aGraphic = Graphic( aGDIMtf );
    }
#endif // !SVX_LIGHT

    return aGraphic;
}


/*************************************************************************
|*
|* Liste fuer zu loeschende Praesentationobjekte (fuer die Seite herausgeben)
|*
\************************************************************************/

List* SdDrawDocument::GetDeletedPresObjList()
{
    if (!pDeletedPresObjList)
    {
        pDeletedPresObjList = new List();
    }

    return pDeletedPresObjList;
}



