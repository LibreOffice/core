/*************************************************************************
 *
 *  $RCSfile: fuexpand.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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

#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"
#include "fuexpand.hxx"
#include "pres.hxx"
#include "sdview.hxx"
#include "sdpage.hxx"
#include "sdoutl.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "viewshel.hxx"
#include "docshell.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

TYPEINIT1( FuExpandPage, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuExpandPage::FuExpandPage(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                 SdDrawDocument* pDoc, SfxRequest& rReq)
       : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    // Selektierte Seite finden (nur Standard-Seiten)
    SdPage* pActualPage = NULL;
    USHORT i = 0;
    USHORT nCount = pDoc->GetSdPageCount(PK_STANDARD);

    while (!pActualPage && i < nCount)
    {
        if (pDoc->GetSdPage(i, PK_STANDARD)->IsSelected())
        {
            pActualPage = pDoc->GetSdPage(i, PK_STANDARD);
        }

        i++;
    }

    if (pActualPage)
    {
        SdOutliner* pOutl = new SdOutliner( pDoc, OUTLINERMODE_OUTLINEOBJECT );
        pOutl->SetUpdateMode(FALSE);
        pOutl->EnableUndo(FALSE);

        if (pDocSh)
            pOutl->SetRefDevice(pDocSh->GetPrinter(TRUE));

        pOutl->SetDefTab( pDoc->GetDefaultTabulator() );
        pOutl->SetStyleSheetPool((SfxStyleSheetPool*) pDoc->GetStyleSheetPool());
        pOutl->SetMinDepth(0);

        SetOfByte aVisibleLayers = pActualPage->GetMasterPageVisibleLayers(0);
        USHORT nActualPageNum = pActualPage->GetPageNum();
        SdPage* pActualNotesPage = (SdPage*) pDoc->GetPage(nActualPageNum + 1);
        SdrTextObj* pActualOutline = (SdrTextObj*) pActualPage->GetPresObj(PRESOBJ_OUTLINE);

        if (pActualOutline)
        {
            pView->BegUndo(String(SdResId(STR_UNDO_EXPAND_PAGE)));

            // Aktuelles Gliederungsobjekt in Outliner setzen
            OutlinerParaObject* pParaObj = pActualOutline->GetOutlinerParaObject();
            pOutl->SetText(*pParaObj);

            // Harte Absatz- und Zeichenattribute entfernen
            SfxItemSet aEmptyEEAttr(pDoc->GetPool(), EE_ITEMS_START, EE_ITEMS_END);
            ULONG nParaCount = pOutl->GetParagraphCount();

            for (USHORT nPara = 0; nPara < nParaCount; nPara++)
            {
                pOutl->QuickRemoveCharAttribs(nPara);
                pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
                Paragraph* pPara = pOutl->GetParagraph(nPara);
            }

            USHORT nPos = 2;
            Paragraph* pPara = pOutl->GetParagraph( 0 );

            while (pPara)
            {
                ULONG nParaPos = pOutl->GetAbsPos( pPara );
                USHORT nDepth = pOutl->GetDepth( nParaPos );
                if ( nDepth == 1 )
                {
                    // Seite mit Titel & Gliederung!
                    SdPage* pPage = (SdPage*) pDoc->AllocPage(FALSE);
                    pPage->SetSize(pActualPage->GetSize() );
                    pPage->SetBorder(pActualPage->GetLftBorder(),
                                     pActualPage->GetUppBorder(),
                                     pActualPage->GetRgtBorder(),
                                     pActualPage->GetLwrBorder() );
                    pPage->SetName(String());

                    // Seite hinter aktueller Seite einfuegen
                    pDoc->InsertPage(pPage, nActualPageNum + nPos);
                    nPos++;
                    pView->AddUndo(new SdrUndoNewPage(*pPage));

                    // MasterPage der aktuellen Seite verwenden
                    USHORT nPgNum = pActualPage->GetMasterPageNum(0);
                    pPage->InsertMasterPage(nPgNum);
                    pPage->SetLayoutName(pActualPage->GetLayoutName());
                    pPage->SetAutoLayout(AUTOLAYOUT_ENUM, TRUE);
                    pPage->SetMasterPageVisibleLayers(aVisibleLayers, 0);

                    // Notiz-Seite
                    SdPage* pNotesPage = (SdPage*) pDoc->AllocPage(FALSE);
                    pNotesPage->SetSize(pActualNotesPage->GetSize());
                    pNotesPage->SetBorder(pActualNotesPage->GetLftBorder(),
                                          pActualNotesPage->GetUppBorder(),
                                          pActualNotesPage->GetRgtBorder(),
                                          pActualNotesPage->GetLwrBorder() );
                    pNotesPage->SetPageKind(PK_NOTES);
                    pNotesPage->SetName(String());

                    // Seite hinter aktueller Seite einfuegen
                    pDoc->InsertPage(pNotesPage, nActualPageNum + nPos);
                    nPos++;
                    pView->AddUndo(new SdrUndoNewPage(*pNotesPage));

                    // MasterPage der aktuellen Seite verwenden
                    nPgNum = pActualNotesPage->GetMasterPageNum(0);
                    pNotesPage->InsertMasterPage(nPgNum);
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), TRUE);
                    pNotesPage->SetMasterPageVisibleLayers(aVisibleLayers, 0);

                    // Title-Textobjekt erstellen
                    SdrTextObj* pTextObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_TITLE);
                    pTextObj->SetOutlinerParaObject(pOutl->CreateParaObject( nParaPos, 1));
                    pTextObj->SetEmptyPresObj(FALSE);

                    SfxStyleSheet* pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                    pTextObj->NbcSetStyleSheet(pSheet, FALSE);

                    ULONG nChildCount = pOutl->GetChildCount(pPara);

                    if (nChildCount > 0)
                    {
                        // Gliederungs-Textobjekt erstellen
                        SdrTextObj* pTextObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_OUTLINE);
                        pPara = pOutl->GetParagraph( ++nParaPos );

                        pTextObj->SetOutlinerParaObject(pOutl->CreateParaObject( nParaPos, nChildCount) );
                        pTextObj->SetEmptyPresObj(FALSE);

                        // Harte Attribute entfernen (Flag auf TRUE)
                        SfxItemSet aAttr(pDoc->GetPool());
                        aAttr.Put(XLineStyleItem(XLINE_NONE));
                        aAttr.Put(XFillStyleItem(XFILL_NONE));
                        pTextObj->NbcSetAttributes(aAttr, TRUE);
                    }
                }

                pPara = pOutl->GetParagraph( ++nParaPos );
            }

            pView->EndUndo();
        }

        delete pOutl;

        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE_PAGE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
    }
}

