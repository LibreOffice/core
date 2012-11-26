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


#include "fuexpand.hxx"

#include <sfx2/viewfrm.hxx>
#include <svx/svdotext.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/printer.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdetc.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "pres.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "Outliner.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "sdmod.hxx"
#include <sfx2/dispatch.hxx>
#include <editeng/eeitem.hxx>

namespace sd {

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
    if ( mpView && mpView->IsTextEdit() )
        mpView->SdrEndTextEdit();

    // Selektierte Seite finden (nur Standard-Seiten)
    SdPage* pActualPage = NULL;
    sal_uInt32 i = 0;
    sal_uInt32 nCount = mpDoc->GetSdPageCount(PK_STANDARD);

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
        pOutl->SetUpdateMode(false);
        pOutl->EnableUndo(false);

        if (mpDocSh)
            pOutl->SetRefDevice( SD_MOD()->GetRefDevice( *mpDocSh ) );

        pOutl->SetDefTab( mpDoc->GetDefaultTabulator() );
        pOutl->SetStyleSheetPool((SfxStyleSheetPool*) mpDoc->GetStyleSheetPool());

        SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();
        sal_uInt32 nActualPageNum = pActualPage->GetPageNumber();
        SdPage* pActualNotesPage = (SdPage*) mpDoc->GetPage(nActualPageNum + 1);
        SdrTextObj* pActualOutline = (SdrTextObj*) pActualPage->GetPresObj(PRESOBJ_OUTLINE);

        if (pActualOutline)
        {
            const bool bUndo = mpView->IsUndoEnabled();

            if( bUndo )
                mpView->BegUndo(String(SdResId(STR_UNDO_EXPAND_PAGE)));

            // Aktuelles Gliederungsobjekt in Outliner setzen
            OutlinerParaObject* pParaObj = pActualOutline->GetOutlinerParaObject();
            pOutl->SetText(*pParaObj);

            // Harte Absatz- und Zeichenattribute entfernen
            SfxItemSet aEmptyEEAttr(mpDoc->GetItemPool(), EE_ITEMS_START, EE_ITEMS_END);
            sal_uLong nParaCount1 = pOutl->GetParagraphCount();

            for (sal_uInt16 nPara = 0; nPara < nParaCount1; nPara++)
            {
                pOutl->QuickRemoveCharAttribs(nPara);
                pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
            }

            sal_uInt32 nPos = 2;
            Paragraph* pPara = pOutl->GetParagraph( 0 );

            while (pPara)
            {
                sal_uLong nParaPos = pOutl->GetAbsPos( pPara );
                sal_Int16 nDepth = pOutl->GetDepth( (sal_uInt16) nParaPos );
                if ( nDepth == 0 )
                {
                    // Seite mit Titel & Gliederung!
                    SdPage* pPage = (SdPage*) mpDoc->AllocPage(false);
                    pPage->SetPageScale(pActualPage->GetPageScale() );
                    pPage->SetPageBorder(pActualPage->GetLeftPageBorder(),
                                     pActualPage->GetTopPageBorder(),
                                     pActualPage->GetRightPageBorder(),
                                     pActualPage->GetBottomPageBorder() );
                    pPage->SetName(String());

                    // Seite hinter aktueller Seite einfuegen
                    mpDoc->InsertPage(pPage, nActualPageNum + nPos);
                    nPos++;

                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pPage));

                    // MasterPage der aktuellen Seite verwenden
                    pPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pPage->SetLayoutName(pActualPage->GetLayoutName());
                    pPage->SetAutoLayout(AUTOLAYOUT_ENUM, true);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

                    // Notiz-Seite
                    SdPage* pNotesPage = (SdPage*) mpDoc->AllocPage(false);
                    pNotesPage->SetPageScale(pActualNotesPage->GetPageScale());
                    pNotesPage->SetPageBorder(pActualNotesPage->GetLeftPageBorder(),
                                          pActualNotesPage->GetTopPageBorder(),
                                          pActualNotesPage->GetRightPageBorder(),
                                          pActualNotesPage->GetBottomPageBorder() );
                    pNotesPage->SetPageKind(PK_NOTES);
                    pNotesPage->SetName(String());

                    // Seite hinter aktueller Seite einfuegen
                    mpDoc->InsertPage(pNotesPage, nActualPageNum + nPos);
                    nPos++;

                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

                    // MasterPage der aktuellen Seite verwenden
                    pNotesPage->TRG_SetMasterPage(pActualNotesPage->TRG_GetMasterPage());
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), true);
                    pNotesPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

                    // Title-Textobjekt erstellen
                    SdrTextObj* pTextObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_TITLE);

//
                    OutlinerParaObject* pOutlinerParaObject = pOutl->CreateParaObject( (sal_uInt16) nParaPos, 1);
                    pOutlinerParaObject->SetOutlinerMode(OUTLINERMODE_TITLEOBJECT);

                    if( pOutlinerParaObject->GetDepth(0) != -1 )
                    {
                        SdrOutliner* pTempOutl = SdrMakeOutliner( OUTLINERMODE_TITLEOBJECT, mpDoc );

                        pTempOutl->SetText( *pOutlinerParaObject );

                        delete pOutlinerParaObject;

                        pTempOutl->SetDepth( pTempOutl->GetParagraph( 0 ), -1 );

                        pOutlinerParaObject = pTempOutl->CreateParaObject();
                        delete pTempOutl;
                    }

                    pTextObj->SetOutlinerParaObject(pOutlinerParaObject);

                    pTextObj->SetEmptyPresObj(false);

                    SfxStyleSheet* pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                    pTextObj->SetStyleSheet(pSheet, false);

                    sal_uLong nChildCount = pOutl->GetChildCount(pPara);

                    if (nChildCount > 0)
                    {
                        // Gliederungs-Textobjekt erstellen
                        SdrTextObj* pOutlineObj = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_OUTLINE);
                        pPara = pOutl->GetParagraph( ++nParaPos );

                        OutlinerParaObject* pOPO = pOutl->CreateParaObject( (sal_uInt16) nParaPos, (sal_uInt16) nChildCount);

// --
                        SdrOutliner* pTempOutl = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, mpDoc );
                        pTempOutl->SetText( *pOPO );

                        sal_uLong nParaCount2 = pTempOutl->GetParagraphCount();
                        sal_uLong nPara;
                        for( nPara = 0; nPara < nParaCount2; nPara++ )
                        {
                            pTempOutl->SetDepth (
                                pTempOutl->GetParagraph( nPara ),
                                pTempOutl->GetDepth((sal_uInt16) nPara ) - 1);
                        }

                        delete pOPO;
                        pOPO = pTempOutl->CreateParaObject();
                        delete pTempOutl;

// --
                        pOutlineObj->SetOutlinerParaObject( pOPO );
                        pOutlineObj->SetEmptyPresObj(false);

                        // Harte Attribute entfernen (Flag auf true)
                        SfxItemSet aAttr(pOutlineObj->GetObjectItemPool());
                        aAttr.Put(XLineStyleItem(XLINE_NONE));
                        aAttr.Put(XFillStyleItem(XFILL_NONE));
                        pOutlineObj->SetMergedItemSet(aAttr);
                    }
                }

                pPara = pOutl->GetParagraph( ++nParaPos );
            }

            if( bUndo )
                mpView->EndUndo();
        }

        delete pOutl;

        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE_PAGE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
    }
}

} // end of namespace sd
