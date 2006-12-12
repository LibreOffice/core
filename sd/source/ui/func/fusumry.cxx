/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fusumry.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 15:53:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fusumry.hxx"

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
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif

namespace sd {

TYPEINIT1( FuSummaryPage, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/
FuSummaryPage::FuSummaryPage (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuSummaryPage::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSummaryPage( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSummaryPage::DoExecute( SfxRequest& rReq )
{
    ::sd::Outliner* pOutl = NULL;
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
                    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

                    // Seite mit Titel & Gliederung!
                    pSummaryPage = (SdPage*) pDoc->AllocPage(FALSE);
                    pSummaryPage->SetSize(pActualPage->GetSize() );
                    pSummaryPage->SetBorder(pActualPage->GetLftBorder(),
                                     pActualPage->GetUppBorder(),
                                     pActualPage->GetRgtBorder(),
                                     pActualPage->GetLwrBorder() );

                    // Seite hinten einfuegen
                    pDoc->InsertPage(pSummaryPage, nCount * 2 + 1);
                    pView->AddUndo(pDoc->GetSdrUndoFactory().CreateUndoNewPage(*pSummaryPage));

                    // MasterPage der aktuellen Seite verwenden
                    pSummaryPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pSummaryPage->SetLayoutName(pActualPage->GetLayoutName());
                    pSummaryPage->SetAutoLayout(AUTOLAYOUT_ENUM, TRUE);
                    pSummaryPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pSummaryPage->setHeaderFooterSettings(pActualPage->getHeaderFooterSettings());

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
                    pView->AddUndo(pDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

                    // MasterPage der aktuellen Seite verwenden
                    pNotesPage->TRG_SetMasterPage(pActualNotesPage->TRG_GetMasterPage());
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), TRUE);
                    pNotesPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pNotesPage->setHeaderFooterSettings(pActualNotesPage->getHeaderFooterSettings());

                    pOutl = new ::sd::Outliner( pDoc, OUTLINERMODE_OUTLINEOBJECT );
                    pOutl->SetUpdateMode(FALSE);
                    pOutl->EnableUndo(FALSE);

                    if (pDocSh)
                        pOutl->SetRefDevice(SD_MOD()->GetRefDevice( *pDocSh ));

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
        pTextObj->SetMergedItemSet(aAttr);

        pView->EndUndo();
        delete pOutl;

        DrawViewShell* pDrawViewShell= dynamic_cast< DrawViewShell* >( pViewShell );
        if(pDrawViewShell)
        {
            pDrawViewShell->SwitchPage( (pSummaryPage->GetPageNum() - 1) / 2);
        }
    }
}


} // end of namespace sd
