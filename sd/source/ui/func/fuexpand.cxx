/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuexpand.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:17:46 $
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


#include "fuexpand.hxx"

#include <sfx2/viewfrm.hxx>

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
}

FunctionReference FuExpandPage::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuExpandPage( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuExpandPage::DoExecute( SfxRequest& )
{
    // Selektierte Seite finden (nur Standard-Seiten)
    SdPage* pActualPage = NULL;
    USHORT i = 0;
    USHORT nCount = mpDoc->GetSdPageCount(PK_STANDARD);

    while (!pActualPage && i < nCount)
    {
        if (mpDoc->GetSdPage(i, PK_STANDARD)->IsSelected())
        {
            pActualPage = mpDoc->GetSdPage(i, PK_STANDARD);
        }

        i++;
    }

    if (pActualPage)
    {
        ::sd::Outliner* pOutl =
              new ::sd::Outliner( mpDoc, OUTLINERMODE_OUTLINEOBJECT );
        pOutl->SetUpdateMode(FALSE);
        pOutl->EnableUndo(FALSE);

        if (mpDocSh)
            pOutl->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        pOutl->SetDefTab( mpDoc->GetDefaultTabulator() );
        pOutl->SetStyleSheetPool((SfxStyleSheetPool*) mpDoc->GetStyleSheetPool());
        pOutl->SetMinDepth(0);

        SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();
        USHORT nActualPageNum = pActualPage->GetPageNum();
        SdPage* pActualNotesPage = (SdPage*) mpDoc->GetPage(nActualPageNum + 1);
        SdrTextObj* pActualOutline = (SdrTextObj*) pActualPage->GetPresObj(PRESOBJ_OUTLINE);

        if (pActualOutline)
        {
            mpView->BegUndo(String(SdResId(STR_UNDO_EXPAND_PAGE)));

            // Aktuelles Gliederungsobjekt in Outliner setzen
            OutlinerParaObject* pParaObj = pActualOutline->GetOutlinerParaObject();
            pOutl->SetText(*pParaObj);

            // Harte Absatz- und Zeichenattribute entfernen
            SfxItemSet aEmptyEEAttr(mpDoc->GetPool(), EE_ITEMS_START, EE_ITEMS_END);
            ULONG nParaCount1 = pOutl->GetParagraphCount();

            for (USHORT nPara = 0; nPara < nParaCount1; nPara++)
            {
                pOutl->QuickRemoveCharAttribs(nPara);
                pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
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
                    SdPage* pPage = (SdPage*) mpDoc->AllocPage(FALSE);
                    pPage->SetSize(pActualPage->GetSize() );
                    pPage->SetBorder(pActualPage->GetLftBorder(),
                                     pActualPage->GetUppBorder(),
                                     pActualPage->GetRgtBorder(),
                                     pActualPage->GetLwrBorder() );
                    pPage->SetName(String());

                    // Seite hinter aktueller Seite einfuegen
                    mpDoc->InsertPage(pPage, nActualPageNum + nPos);
                    nPos++;
                    mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pPage));

                    // MasterPage der aktuellen Seite verwenden
                    pPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pPage->SetLayoutName(pActualPage->GetLayoutName());
                    pPage->SetAutoLayout(AUTOLAYOUT_ENUM, TRUE);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

                    // Notiz-Seite
                    SdPage* pNotesPage = (SdPage*) mpDoc->AllocPage(FALSE);
                    pNotesPage->SetSize(pActualNotesPage->GetSize());
                    pNotesPage->SetBorder(pActualNotesPage->GetLftBorder(),
                                          pActualNotesPage->GetUppBorder(),
                                          pActualNotesPage->GetRgtBorder(),
                                          pActualNotesPage->GetLwrBorder() );
                    pNotesPage->SetPageKind(PK_NOTES);
                    pNotesPage->SetName(String());

                    // Seite hinter aktueller Seite einfuegen
                    mpDoc->InsertPage(pNotesPage, nActualPageNum + nPos);
                    nPos++;
                    mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

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
                        SdrOutliner* pTempOutl = SdrMakeOutliner( OUTLINERMODE_TITLEOBJECT, mpDoc );

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
                        SdrTextObj* pOutlineObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_OUTLINE);
                        pPara = pOutl->GetParagraph( ++nParaPos );

                        OutlinerParaObject* pOPO = pOutl->CreateParaObject( (USHORT) nParaPos, (USHORT) nChildCount);

// --
                        SdrOutliner* pTempOutl = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, mpDoc );
                        pTempOutl->SetText( *pOPO );

                        ULONG nParaCount2 = pTempOutl->GetParagraphCount();
                        ULONG nPara;
                        for( nPara = 0; nPara < nParaCount2; nPara++ )
                        {
                            pTempOutl->SetDepth (
                                pTempOutl->GetParagraph( nPara ),
                                pTempOutl->GetDepth((USHORT) nPara ) - 1);
                        }

                        delete pOPO;
                        pOPO = pTempOutl->CreateParaObject();
                        delete pTempOutl;

// --
                        pOutlineObj->SetOutlinerParaObject( pOPO );
                        pOutlineObj->SetEmptyPresObj(FALSE);

                        // Harte Attribute entfernen (Flag auf TRUE)
                        SfxItemSet aAttr(mpDoc->GetPool());
                        aAttr.Put(XLineStyleItem(XLINE_NONE));
                        aAttr.Put(XFillStyleItem(XFILL_NONE));
                        pOutlineObj->SetMergedItemSet(aAttr);
                    }
                }

                pPara = pOutl->GetParagraph( ++nParaPos );
            }

            mpView->EndUndo();
        }

        delete pOutl;

        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE_PAGE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
    }
}

} // end of namespace sd
