/*************************************************************************
 *
 *  $RCSfile: fuexpand.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:02:19 $
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

#include "fuexpand.hxx"

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

#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif

#include "app.hrc"
#include "strings.hrc"
#include "pres.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "sdmod.hxx"

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

namespace sd {

TYPEINIT1( FuExpandPage, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuExpandPage::FuExpandPage (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
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
        ::sd::Outliner* pOutl =
              new ::sd::Outliner( pDoc, OUTLINERMODE_OUTLINEOBJECT );
        pOutl->SetUpdateMode(FALSE);
        pOutl->EnableUndo(FALSE);

        if (pDocSh)
            pOutl->SetRefDevice( SD_MOD()->GetRefDevice( *pDocSh ) );

        pOutl->SetDefTab( pDoc->GetDefaultTabulator() );
        pOutl->SetStyleSheetPool((SfxStyleSheetPool*) pDoc->GetStyleSheetPool());
        pOutl->SetMinDepth(0);

        SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();
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
                USHORT nDepth = pOutl->GetDepth( (USHORT) nParaPos );
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
                    pPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pPage->SetLayoutName(pActualPage->GetLayoutName());
                    pPage->SetAutoLayout(AUTOLAYOUT_ENUM, TRUE);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

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
                    pNotesPage->TRG_SetMasterPage(pActualNotesPage->TRG_GetMasterPage());
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), TRUE);
                    pNotesPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

                    // Title-Textobjekt erstellen
                    SdrTextObj* pTextObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_TITLE);

//
                    OutlinerParaObject* pOutlinerParaObject = pOutl->CreateParaObject( (USHORT) nParaPos, 1);
                    pOutlinerParaObject->SetOutlinerMode(OUTLINERMODE_TITLEOBJECT);

                    if( pOutlinerParaObject->GetDepth(0) != 0 )
                    {
                        SdrOutliner* pTempOutl = SdrMakeOutliner( OUTLINERMODE_TITLEOBJECT, pDoc );

                        pTempOutl->SetText( *pOutlinerParaObject );
                        pTempOutl->SetMinDepth(0);

                        delete pOutlinerParaObject;

                        pTempOutl->SetDepth( pTempOutl->GetParagraph( 0 ), 0 );

                        pOutlinerParaObject = pTempOutl->CreateParaObject();
                        delete pTempOutl;
                    }

                    pTextObj->SetOutlinerParaObject(pOutlinerParaObject);

                    pTextObj->SetEmptyPresObj(FALSE);

                    SfxStyleSheet* pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                    pTextObj->NbcSetStyleSheet(pSheet, FALSE);

                    ULONG nChildCount = pOutl->GetChildCount(pPara);

                    if (nChildCount > 0)
                    {
                        // Gliederungs-Textobjekt erstellen
                        SdrTextObj* pTextObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_OUTLINE);
                        pPara = pOutl->GetParagraph( ++nParaPos );

                        OutlinerParaObject* pOutlinerParaObject = pOutl->CreateParaObject( (USHORT) nParaPos, (USHORT) nChildCount);

// --
                        SdrOutliner* pTempOutl = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, pDoc );
                        pTempOutl->SetText( *pOutlinerParaObject );

                        ULONG nParaCount = pTempOutl->GetParagraphCount();
                        ULONG nPara;
                        for( nPara = 0; nPara < nParaCount; nPara++ )
                        {
                            pTempOutl->SetDepth (
                                pTempOutl->GetParagraph( nPara ),
                                pTempOutl->GetDepth((USHORT) nPara ) - 1);
                        }

                        delete pOutlinerParaObject;
                        pOutlinerParaObject = pTempOutl->CreateParaObject();
                        delete pTempOutl;

// --
                        pTextObj->SetOutlinerParaObject( pOutlinerParaObject );
                        pTextObj->SetEmptyPresObj(FALSE);

                        // Harte Attribute entfernen (Flag auf TRUE)
                        SfxItemSet aAttr(pDoc->GetPool());
                        aAttr.Put(XLineStyleItem(XLINE_NONE));
                        aAttr.Put(XFillStyleItem(XFILL_NONE));
                        pTextObj->SetMergedItemSet(aAttr);
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

} // end of namespace sd
