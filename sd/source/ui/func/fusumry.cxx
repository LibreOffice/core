/*************************************************************************
 *
 *  $RCSfile: fusumry.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:36 $
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


#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif

#include "strings.hrc"

#include "pres.hxx"
#include "sdview.hxx"
#include "sdpage.hxx"
#include "sdoutl.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "viewshel.hxx"
#include "docshell.hxx"
#include "fusumry.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "drviewsh.hxx"

TYPEINIT1( FuSummaryPage, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/
FuSummaryPage::FuSummaryPage(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq)
 : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    SdOutliner* pOutl = NULL;
    SdPage* pSummaryPage = NULL;
    USHORT i = 0;
    USHORT nFirstPage = SDRPAGE_NOTFOUND;
    USHORT nSelectedPages = 0;
    USHORT nCount = pDoc->GetSdPageCount(PK_STANDARD);

    while (i < nCount && nSelectedPages <= 1)
    {
        /**********************************************************************
        * Wieviele Seiten sind selektiert?
        * Genau eine Seite selektiert: Ab dieser Seite alles zusammenfassen
        * sonst: Nur die selektierten Seiten zusammenfassen
        **********************************************************************/
        SdPage* pActualPage = pDoc->GetSdPage(i, PK_STANDARD);

        if (pActualPage->IsSelected())
        {
            if (nFirstPage == SDRPAGE_NOTFOUND)
            {
                nFirstPage = i;
            }

            nSelectedPages++;
        }

        i++;
    }

    SfxStyleSheet* pStyle = NULL;

    for (i = nFirstPage; i < nCount; i++)
    {
        SdPage* pActualPage = pDoc->GetSdPage(i, PK_STANDARD);

        if (nSelectedPages <= 1 || pActualPage->IsSelected())
        {
            SdPage* pActualNotesPage = pDoc->GetSdPage(i, PK_NOTES);
            SdrTextObj* pTextObj = (SdrTextObj*) pActualPage->GetPresObj(PRESOBJ_TITLE);

            if (pTextObj && !pTextObj->IsEmptyPresObj())
            {
                if (!pSummaryPage)
                {
                    /**********************************************************
                    * Inhaltsverzeichnis-Seite einfuegen und Outliner anlegen
                    **********************************************************/
                    pView->BegUndo(String(SdResId(STR_UNDO_SUMMARY_PAGE)));
                    SetOfByte aVisibleLayers = pActualPage->GetMasterPageVisibleLayers(0);

                    // Seite mit Titel & Gliederung!
                    pSummaryPage = (SdPage*) pDoc->AllocPage(FALSE);
                    pSummaryPage->SetSize(pActualPage->GetSize() );
                    pSummaryPage->SetBorder(pActualPage->GetLftBorder(),
                                     pActualPage->GetUppBorder(),
                                     pActualPage->GetRgtBorder(),
                                     pActualPage->GetLwrBorder() );

                    // Seite hinten einfuegen
                    pDoc->InsertPage(pSummaryPage, nCount * 2 + 1);
                    pView->AddUndo(new SdrUndoNewPage(*pSummaryPage));

                    // MasterPage der aktuellen Seite verwenden
                    USHORT nPgNum = pActualPage->GetMasterPageNum(0);
                    pSummaryPage->InsertMasterPage(nPgNum);
                    pSummaryPage->SetLayoutName(pActualPage->GetLayoutName());
                    pSummaryPage->SetAutoLayout(AUTOLAYOUT_ENUM, TRUE);
                    pSummaryPage->SetMasterPageVisibleLayers(aVisibleLayers, 0);

                    // Notiz-Seite
                    SdPage* pNotesPage = (SdPage*) pDoc->AllocPage(FALSE);
                    pNotesPage->SetSize(pActualNotesPage->GetSize());
                    pNotesPage->SetBorder(pActualNotesPage->GetLftBorder(),
                                          pActualNotesPage->GetUppBorder(),
                                          pActualNotesPage->GetRgtBorder(),
                                          pActualNotesPage->GetLwrBorder() );
                    pNotesPage->SetPageKind(PK_NOTES);

                    // Seite hinten einfuegen
                    pDoc->InsertPage(pNotesPage, nCount * 2 + 2);
                    pView->AddUndo(new SdrUndoNewPage(*pNotesPage));

                    // MasterPage der aktuellen Seite verwenden
                    nPgNum = pActualNotesPage->GetMasterPageNum(0);
                    pNotesPage->InsertMasterPage(nPgNum);
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), TRUE);
                    pNotesPage->SetMasterPageVisibleLayers(aVisibleLayers, 0);

                    pOutl = new SdOutliner( pDoc, OUTLINERMODE_OUTLINEOBJECT );
                    pOutl->SetUpdateMode(FALSE);
                    pOutl->EnableUndo(FALSE);

                    if (pDocSh)
                        pOutl->SetRefDevice(pDocSh->GetPrinter(TRUE));

                    pOutl->SetDefTab( pDoc->GetDefaultTabulator() );
                    pOutl->SetStyleSheetPool((SfxStyleSheetPool*) pDoc->GetStyleSheetPool());
                    pStyle = pSummaryPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
                    pOutl->SetStyleSheet( 0, pStyle );
                    pOutl->SetMinDepth(0);
                }

                /**************************************************************
                * Text hinzufuegen
                **************************************************************/
                OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
                pParaObj->SetOutlinerMode( OUTLINERMODE_OUTLINEOBJECT );
                pOutl->AddText(*pParaObj);
            }
        }
    }

    if (pSummaryPage)
    {
        SdrTextObj* pTextObj = (SdrTextObj*) pSummaryPage->GetPresObj(PRESOBJ_OUTLINE);

        // Harte Absatz- und Zeichenattribute entfernen
        SfxItemSet aEmptyEEAttr(pDoc->GetPool(), EE_ITEMS_START, EE_ITEMS_END);
        ULONG nParaCount = pOutl->GetParagraphCount();

        for (USHORT nPara = 0; nPara < nParaCount; nPara++)
        {
            pOutl->SetStyleSheet( nPara, pStyle );
            pOutl->QuickRemoveCharAttribs(nPara);
            pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
            pOutl->SetDepth(pOutl->GetParagraph(nPara), 1);
        }

        pTextObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
        pTextObj->SetEmptyPresObj(FALSE);

        // Harte Attribute entfernen (Flag auf TRUE)
        SfxItemSet aAttr(pDoc->GetPool());
        aAttr.Put(XLineStyleItem(XLINE_NONE));
        aAttr.Put(XFillStyleItem(XFILL_NONE));
        pTextObj->NbcSetAttributes(aAttr, TRUE);

        pView->EndUndo();
        delete pOutl;

        if (pViewSh->ISA(SdDrawViewShell))
        {
            ((SdDrawViewShell*) pViewSh)->SwitchPage((pSummaryPage->GetPageNum() - 1) / 2);
        }
    }
}


