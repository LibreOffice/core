/*************************************************************************
 *
 *  $RCSfile: drawdoc2.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:55:25 $
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
#ifndef SD_OUTLINE_HXX
#include "Outliner.hxx"
#endif
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
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _EEITEMID_HXX //autogen
#include <svx/eeitemid.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
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
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "::ui:inc:DrawDocShell.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "::ui:inc:FrameView.hxx"
#endif
#include "::ui:inc:cfgids.hxx"
#include "::ui:inc:strings.hrc"
#else
#ifdef UNX
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "../ui/inc/DrawDocShell.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "../ui/inc/FrameView.hxx"
#endif
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"
#else
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "..\ui\inc\DrawDocShell.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "..\ui\inc\FrameView.hxx"
#endif
#include "..\ui\inc\cfgids.hxx"
#include "..\ui\inc\strings.hrc"
#endif
#endif

#endif // !SVX_LIGHT

#include "PageListWatcher.hxx"

using namespace ::sd;

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
    const USHORT nMaxMasterPages = GetMasterPageCount();

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

USHORT SdDrawDocument::GetPageByName(const String& rPgName, BOOL& rbIsMasterPage) const
{
    SdPage* pPage = NULL;
    SdPage* pPageFound = NULL;
    USHORT nPage = 0;
    const USHORT nMaxPages = GetPageCount();
    USHORT nPageNum = SDRPAGE_NOTFOUND;

    rbIsMasterPage = FALSE;

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
            rbIsMasterPage = TRUE;
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
    // #109538#
    return mpDrawPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

USHORT SdDrawDocument::GetSdPageCount(PageKind ePgKind) const
{
    // #109538#
    return (sal_uInt16)mpDrawPageListWatcher->GetSdPageCount(ePgKind);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

SdPage* SdDrawDocument::GetMasterSdPage(USHORT nPgNum, PageKind ePgKind)
{
    // #109538#
    return mpMasterPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

USHORT SdDrawDocument::GetMasterSdPageCount(PageKind ePgKind) const
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
                        ((SdrPageObj*)pObj)->SetReferencedPage(GetPage(nPage - 1));
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

void SdDrawDocument::MovePage(USHORT nPgNum, USHORT nNewPos)
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

void SdDrawDocument::InsertPage(SdrPage* pPage, USHORT nPos)
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

void SdDrawDocument::DeletePage(USHORT nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}

/*************************************************************************
|*
|*  Seite entfernen
|*
\************************************************************************/

SdrPage* SdDrawDocument::RemovePage(USHORT nPgNum)
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

//////////////////////////////////////////////////////////////////////////////
// #107844#
// An undo class which is able to set/unset user calls is needed to handle
// the undo/redo of PresObjs correctly. It can also add/remove the object
// from the PresObjList of that page.

SdrUndoUserCallObj::SdrUndoUserCallObj(SdrObject& rNewObj, SdPage* pNew)
:   SdrUndoObj(rNewObj),
    mpOld((SdPage*)rNewObj.GetUserCall()),
    mpNew(pNew)
{
    if( mpOld )
    {
        meKind = mpOld->GetPresObjKind(&rNewObj);
    }
    else if( mpNew )
    {
        meKind = mpNew->GetPresObjKind(&rNewObj);
    }
    else
    {
        DBG_ERROR( "SdrUndoUserCallObj::SdrUndoUserCallObj() - no page, how shall I restore the user call?" );
        meKind = PRESOBJ_NONE;
    }
}

SdrUndoUserCallObj::SdrUndoUserCallObj(SdrObject& rNewObj, SdPage* pOld, SdPage* pNew)
:   SdrUndoObj(rNewObj),
    mpOld(pOld),
    mpNew(pNew)
{
    if( mpOld )
    {
        meKind = mpOld->GetPresObjKind(&rNewObj);
    }
    else if( mpNew )
    {
        meKind = mpNew->GetPresObjKind(&rNewObj);
    }
    else
    {
        DBG_ERROR( "SdrUndoUserCallObj::SdrUndoUserCallObj() - no page, how shall I restore the user call?" );
        meKind = PRESOBJ_NONE;
    }
}

void SdrUndoUserCallObj::Undo()
{
    if(mpNew && (meKind != PRESOBJ_NONE))
    {
        mpNew->RemovePresObj(pObj);
    }

    pObj->SetUserCall(mpOld);

    if(mpOld && (meKind != PRESOBJ_NONE))
    {
        mpOld->InsertPresObj( pObj, meKind );
    }
}

void SdrUndoUserCallObj::Redo()
{
    if(mpOld && (meKind != PRESOBJ_NONE))
    {
        mpOld->RemovePresObj(pObj);
    }

    pObj->SetUserCall(mpNew);

    if(mpNew && (meKind != PRESOBJ_NONE))
    {
        mpNew->InsertPresObj( pObj, meKind );
    }
}

/*************************************************************************
|*
|* anfallenden Undo-Aktionen des SdrModels verwalten
|*
\************************************************************************/

// #107844#
// Inline has no advantages at such big code.
IMPL_LINK( SdDrawDocument, NotifyUndoActionHdl, SfxUndoAction *, pUndoAction )
{
#ifndef SVX_LIGHT
    if (pUndoAction)
    {
        if ( pDeletedPresObjList )
        {
            if( pDeletedPresObjList->Count() )
            {
                // Praesentationsobjekte sollen geloescht werden
                ULONG nCount = pDeletedPresObjList->Count();

                // #107844#
                // SfxUndoAction::Merge() is empty implemented and does
                // nothing. To really add the action here without getting
                // a recursive call to this link the RTTI needs to be used.
                // So, pUndoGroup is filled and used if this worked well.
                SdrUndoGroup* pUndoGroup;

                if(pUndoAction->ISA(SdrUndoGroup))
                {
                    pUndoGroup = (SdrUndoGroup*)pUndoAction;
                }
                else
                {
                    DBG_ASSERT(sal_False, "SdDrawDocument::NotifyUndoActionHdl: Could not add undo action (!)");
                }

                for (ULONG i=0; i < nCount; i++)
                {
                    // #107844# Take the other direction here to create the new placeholders
                    // in the correct order since they were deleted reversely. Else, travelling
                    // over the text objects using CTRL+RETURN was in wrong order.
                    SdrObject* pObj = (SdrObject*) pDeletedPresObjList->GetObject((nCount - 1) - i);

                    // #107844#
                    // Decide between empty and not-empty PresObj later
                    if (pObj)
                    {
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

                                    if(pUndoGroup)
                                    {
                                        // #107844#
                                        // Add new Undo-Action here
                                        pUndoGroup->AddAction(new SdrUndoUserCallObj(*pTextObj, NULL));
                                    }

                                    pPage->RemovePresObj(pTextObj);
                                    pTextObj->SetUserCall(NULL);

                                    if(!pObj->IsEmptyPresObj())
                                    {
                                        // #107844#
                                        // create empty new presobj
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
                                            GetInternalOutliner(TRUE);
                                            pInternalOutliner->SetMinDepth(0);

                                            SdrTextObj* pNewTextObj = (SdrTextObj*)pTextObj->Clone();
                                            pPage->InsertObject(pNewTextObj);

                                            pPage->SetObjText(pNewTextObj, pInternalOutliner, ePresObjKind, aString);
                                            pNewTextObj->NbcSetStyleSheet(pPage->GetStyleSheetForPresObj(ePresObjKind), TRUE);
                                            pNewTextObj->SetEmptyPresObj(TRUE);

                                            if(pUndoGroup)
                                            {
                                                // #107844#
                                                // Add new Undo-Action here
                                                pUndoGroup->AddAction(new SdrUndoUserCallObj(*pNewTextObj, pPage));
                                            }

                                            pNewTextObj->SetUserCall(pPage);
                                            pPage->InsertPresObj(pNewTextObj, ePresObjKind);

                                            if(pUndoGroup)
                                            {
                                                // #107844#
                                                // Add new Undo-Action here
                                                pUndoGroup->AddAction(new SdrUndoNewObj(*pNewTextObj));
                                            }

                                            pInternalOutliner->Clear();
                                            pInternalOutliner->SetMinDepth(0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ClearDeletedPresObjList();
        }

        if (pDocSh)
        {
            pDocSh->GetUndoManager()->AddUndoAction(pUndoAction);
        }
    }

#endif // !SVX_LIGHT
    return 0;
}

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
        if (aDefSize.Height() >= aDefSize.Width())
        {
            pHandoutPage->SetSize(aDefSize);
        }
        else
        {
            pHandoutPage->SetSize( Size(aDefSize.Height(), aDefSize.Width()) );
        }

        pHandoutPage->SetBorder(0, 0, 0, 0);
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
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

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
        pPage->TRG_SetMasterPage( *pMPage );
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

        pNotesPage->SetBorder(0, 0, 0, 0);
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
        pNotesPage->TRG_SetMasterPage( *pNotesMPage );
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
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

        for (USHORT i = 1; i < nPageCount; i = i + 2)
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
|* Language setzen
|*
\************************************************************************/

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


/*************************************************************************
|*
|* Return language
|*
\************************************************************************/

LanguageType SdDrawDocument::GetLanguage( const USHORT nId ) const
{
    LanguageType eLangType = eLanguage;

    if( nId == EE_CHAR_LANGUAGE_CJK )
        eLangType = eLanguageCJK;
    else if( nId == EE_CHAR_LANGUAGE_CTL )
        eLangType = eLanguageCTL;

    return eLangType;
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
#endif

    BOOL bChanged = IsChanged();        // merken

    // Autolayouts initialisieren
    SdPage* pHandoutMPage = GetMasterSdPage(0, PK_HANDOUT);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT6, TRUE, TRUE);
    }

    SdPage* pPage = GetSdPage(0, PK_STANDARD);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, TRUE, TRUE);
    }

    SdPage* pNotesPage = GetSdPage(0, PK_NOTES);

    if (pNotesPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // AutoLayout wurde noch nicht umgesetzt -> Initialisieren
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, TRUE, TRUE);
    }

    SetChanged(bChanged || FALSE);

#ifndef SVX_LIGHT
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
                                              const ::Window& rCmpWnd )
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




void SdDrawDocument::ClearDeletedPresObjList (void)
{
    delete pDeletedPresObjList;
    pDeletedPresObjList = NULL;
}




/** this method enforces that the masterpages are in the currect order,
    that is at position 1 is a PK_STANDARD masterpage followed by a
    PK_NOTES masterpage and so on. #
*/
void SdDrawDocument::CheckMasterPages()
{
//  RemoveMasterPage(2); // code to test the creation of notes pages

    USHORT nMaxPages = GetMasterPageCount();

    // we need at least a handout master and one master page
    if( nMaxPages < 2 )
    {
        return;
    }

    SdPage* pPage = NULL;
    SdPage* pNotesPage = NULL;

    USHORT nPage;

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
                    pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True );
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




USHORT SdDrawDocument::CreatePage (USHORT nPageNum)
{
    PageKind ePageKind = PK_STANDARD;

    // Get current page.
    SdPage* pActualPage = GetSdPage(nPageNum, ePageKind);

    // Get background flags.
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

    // Get layout from current page.
    AutoLayout eAutoLayout = pActualPage->GetAutoLayout();

    return CreatePage (
        pActualPage, ePageKind,
        // No names for the new slides.
        String(), String(),
        eAutoLayout, eAutoLayout,
        aVisibleLayers.IsSet(aBckgrnd),
        aVisibleLayers.IsSet(aBckgrndObj));
}




USHORT SdDrawDocument::CreatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    BOOL bIsPageBack,
    BOOL bIsPageObj)
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
        USHORT nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
        eStandardLayout = pPreviousStandardPage->GetAutoLayout();
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        USHORT nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
        eNotesLayout = pPreviousNotesPage->GetAutoLayout();
    }

    // Create new standard page and set it up.
    pStandardPage = (SdPage*) AllocPage(FALSE);

    // #108658#
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
    pStandardPage->SetAutoLayout(eStandardLayout, TRUE);
    pStandardPage->getHeaderFooterSettings() = pPreviousStandardPage->getHeaderFooterSettings();

    // Create new notes page and set it up.
    pNotesPage = (SdPage*) AllocPage(FALSE);
    pNotesPage->SetPageKind(PK_NOTES);

    // Use master page of current page.
    pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());

    // Use layout of current notes page.
    pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
    pNotesPage->SetAutoLayout(eNotesLayout, TRUE);
    pNotesPage->getHeaderFooterSettings() = pPreviousNotesPage->getHeaderFooterSettings();

    return InsertPageSet (
        pActualPage, ePageKind,
        sStandardPageName,
        sNotesPageName,
        eStandardLayout,
        eNotesLayout,
        bIsPageBack,
        bIsPageObj,

        pStandardPage,
        pNotesPage);
}




USHORT SdDrawDocument::DuplicatePage (USHORT nPageNum)
{
    PageKind ePageKind = PK_STANDARD;

    // Get current page.
    SdPage* pActualPage = GetSdPage(nPageNum, ePageKind);

    // Get background flags.
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
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




USHORT SdDrawDocument::DuplicatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    BOOL bIsPageBack,
    BOOL bIsPageObj)
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
        USHORT nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = (SdPage*) GetPage(nNotesPageNum - 3);
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        USHORT nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
    }

    // Create duplicates of a standard page and the associated notes page.
    pStandardPage = (SdPage*) pPreviousStandardPage->Clone();
    pNotesPage = (SdPage*) pPreviousNotesPage->Clone();

    return InsertPageSet (
        pActualPage, ePageKind,
        sStandardPageName,
        sNotesPageName,
        eStandardLayout,
        eNotesLayout,
        bIsPageBack,
        bIsPageObj,

        pStandardPage,
        pNotesPage);
}




USHORT SdDrawDocument::InsertPageSet (
    SdPage* pActualPage,
    PageKind ePageKind,
    const String& sStandardPageName,
    const String& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    BOOL bIsPageBack,
    BOOL bIsPageObj,

    SdPage* pStandardPage,
    SdPage* pNotesPage)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    USHORT nStandardPageNum;
    USHORT nNotesPageNum;
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
        eStandardLayout = pPreviousStandardPage->GetAutoLayout();
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = (SdPage*) GetPage(nStandardPageNum - 1);
        nNotesPageNum = nStandardPageNum + 1;
        aNotesPageName = aStandardPageName;
        eNotesLayout = pPreviousNotesPage->GetAutoLayout();
    }


    // Set up and insert the standard page.
    SetupNewPage (
        pPreviousStandardPage,
        pStandardPage,
        aStandardPageName,
        nStandardPageNum,
        bIsPageBack,
        bIsPageObj);

    // Set up and insert the notes page.
    pNotesPage->SetPageKind(PK_NOTES);
    SetupNewPage (
        pPreviousNotesPage,
        pNotesPage,
        aNotesPageName,
        nNotesPageNum,
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
    USHORT nInsertionPoint,
    BOOL bIsPageBack,
    BOOL bIsPageObj)
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
        BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
        BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
        SetOfByte aVisibleLayers = pPreviousPage->TRG_GetMasterPageVisibleLayers();
        aVisibleLayers.Set(aBckgrnd, bIsPageBack);
        aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
        pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }
}

// eof
