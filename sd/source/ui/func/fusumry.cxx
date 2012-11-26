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


#include "fusumry.hxx"
#include <editeng/eeitem.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/printer.hxx>
#include <editeng/outlobj.hxx>

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
#include "DrawViewShell.hxx"

namespace sd {

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

void FuSummaryPage::DoExecute( SfxRequest& )
{
    ::sd::Outliner* pOutl = NULL;
    SdPage* pSummaryPage = NULL;
    sal_uInt32 i = 0;
    sal_uInt32 nFirstPage = SDRPAGE_NOTFOUND;
    sal_uInt32 nSelectedPages = 0;
    sal_uInt32 nCount = mpDoc->GetSdPageCount(PK_STANDARD);

    while (i < nCount && nSelectedPages <= 1)
    {
        /**********************************************************************
        * Wieviele Seiten sind selektiert?
        * Genau eine Seite selektiert: Ab dieser Seite alles zusammenfassen
        * sonst: Nur die selektierten Seiten zusammenfassen
        **********************************************************************/
        SdPage* pActualPage = mpDoc->GetSdPage(i, PK_STANDARD);

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

    bool bBegUndo = false;

    SfxStyleSheet* pStyle = NULL;

    for (i = nFirstPage; i < nCount; i++)
    {
        SdPage* pActualPage = mpDoc->GetSdPage(i, PK_STANDARD);

        if (nSelectedPages <= 1 || pActualPage->IsSelected())
        {
            SdPage* pActualNotesPage = mpDoc->GetSdPage(i, PK_NOTES);
            SdrTextObj* pTextObj = (SdrTextObj*) pActualPage->GetPresObj(PRESOBJ_TITLE);

            if (pTextObj && !pTextObj->IsEmptyPresObj())
            {
                if (!pSummaryPage)
                {
                    /**********************************************************
                    * Inhaltsverzeichnis-Seite einfuegen und Outliner anlegen
                    **********************************************************/
                    const bool bUndo = mpView->IsUndoEnabled();

                    if( bUndo )
                    {
                        mpView->BegUndo(String(SdResId(STR_UNDO_SUMMARY_PAGE)));
                        bBegUndo = true;
                    }

                    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

                    // Seite mit Titel & Gliederung!
                    pSummaryPage = (SdPage*) mpDoc->AllocPage(false);
                    pSummaryPage->SetPageScale(pActualPage->GetPageScale() );
                    pSummaryPage->SetPageBorder(pActualPage->GetLeftPageBorder(),
                                     pActualPage->GetTopPageBorder(),
                                     pActualPage->GetRightPageBorder(),
                                     pActualPage->GetBottomPageBorder() );

                    // Seite hinten einfuegen
                    mpDoc->InsertPage(pSummaryPage, nCount * 2 + 1);
                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pSummaryPage));

                    // MasterPage der aktuellen Seite verwenden
                    pSummaryPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pSummaryPage->SetLayoutName(pActualPage->GetLayoutName());
                    pSummaryPage->SetAutoLayout(AUTOLAYOUT_ENUM, true);
                    pSummaryPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pSummaryPage->setHeaderFooterSettings(pActualPage->getHeaderFooterSettings());

                    // Notiz-Seite
                    SdPage* pNotesPage = (SdPage*) mpDoc->AllocPage(false);
                    pNotesPage->SetPageScale(pActualNotesPage->GetPageScale());
                    pNotesPage->SetPageBorder(pActualNotesPage->GetLeftPageBorder(),
                                          pActualNotesPage->GetTopPageBorder(),
                                          pActualNotesPage->GetRightPageBorder(),
                                          pActualNotesPage->GetBottomPageBorder() );
                    pNotesPage->SetPageKind(PK_NOTES);

                    // Seite hinten einfuegen
                    mpDoc->InsertPage(pNotesPage, nCount * 2 + 2);

                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

                    // MasterPage der aktuellen Seite verwenden
                    pNotesPage->TRG_SetMasterPage(pActualNotesPage->TRG_GetMasterPage());
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), true);
                    pNotesPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pNotesPage->setHeaderFooterSettings(pActualNotesPage->getHeaderFooterSettings());

                    pOutl = new ::sd::Outliner( mpDoc, OUTLINERMODE_OUTLINEOBJECT );
                    pOutl->SetUpdateMode(false);
                    pOutl->EnableUndo(false);

                    if (mpDocSh)
                        pOutl->SetRefDevice(SD_MOD()->GetRefDevice( *mpDocSh ));

                    pOutl->SetDefTab( mpDoc->GetDefaultTabulator() );
                    pOutl->SetStyleSheetPool((SfxStyleSheetPool*) mpDoc->GetStyleSheetPool());
                    pStyle = pSummaryPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
                    pOutl->SetStyleSheet( 0, pStyle );
                }

                /**************************************************************
                * Text hinzufuegen
                **************************************************************/
                OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
                // #118876#, check if the OutlinerParaObject is created successfully
                if( pParaObj )
                {
                    pParaObj->SetOutlinerMode( OUTLINERMODE_OUTLINEOBJECT );
                    pOutl->AddText(*pParaObj);
                }
            }
        }
    }

    if (pSummaryPage)
    {
        SdrTextObj* pTextObj = (SdrTextObj*) pSummaryPage->GetPresObj(PRESOBJ_OUTLINE);

        // Harte Absatz- und Zeichenattribute entfernen
        SfxItemSet aEmptyEEAttr(mpDoc->GetItemPool(), EE_ITEMS_START, EE_ITEMS_END);
        sal_uLong nParaCount = pOutl->GetParagraphCount();

        for (sal_uInt16 nPara = 0; nPara < nParaCount; nPara++)
        {
            pOutl->SetStyleSheet( nPara, pStyle );
            pOutl->QuickRemoveCharAttribs(nPara);
            pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
            pOutl->SetDepth(pOutl->GetParagraph(nPara), 0);
        }

        pTextObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
        pTextObj->SetEmptyPresObj(false);

        // Harte Attribute entfernen (Flag auf true)
        SfxItemSet aAttr(pTextObj->GetObjectItemPool());
        aAttr.Put(XLineStyleItem(XLINE_NONE));
        aAttr.Put(XFillStyleItem(XFILL_NONE));
        pTextObj->SetMergedItemSet(aAttr);

        if( bBegUndo )
            mpView->EndUndo();
        delete pOutl;

        DrawViewShell* pDrawViewShell= dynamic_cast< DrawViewShell* >( mpViewShell );
        if(pDrawViewShell)
        {
            pDrawViewShell->SwitchPage( (pSummaryPage->GetPageNumber() - 1) / 2);
        }
    }
}


} // end of namespace sd
