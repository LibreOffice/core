/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

TYPEINIT1( FuSummaryPage, FuPoor );

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
    sal_uInt16 i = 0;
    sal_uInt16 nFirstPage = SDRPAGE_NOTFOUND;
    sal_uInt16 nSelectedPages = 0;
    sal_uInt16 nCount = mpDoc->GetSdPageCount(PK_STANDARD);

    while (i < nCount && nSelectedPages <= 1)
    {
        /* How many pages are selected?
             exactly one: pool everything from this page
             otherwise:   only pool the selected pages  */
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
                    // insert "table of content"-page and create outliner
                    const bool bUndo = mpView->IsUndoEnabled();

                    if( bUndo )
                    {
                        mpView->BegUndo(SD_RESSTR(STR_UNDO_SUMMARY_PAGE));
                        bBegUndo = true;
                    }

                    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

                    // page with title & structuring!
                    pSummaryPage = (SdPage*) mpDoc->AllocPage(sal_False);
                    pSummaryPage->SetSize(pActualPage->GetSize() );
                    pSummaryPage->SetBorder(pActualPage->GetLftBorder(),
                                     pActualPage->GetUppBorder(),
                                     pActualPage->GetRgtBorder(),
                                     pActualPage->GetLwrBorder() );

                    // insert page at the back
                    mpDoc->InsertPage(pSummaryPage, nCount * 2 + 1);
                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pSummaryPage));

                    // use MasterPage of the current page
                    pSummaryPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pSummaryPage->SetLayoutName(pActualPage->GetLayoutName());
                    pSummaryPage->SetAutoLayout(AUTOLAYOUT_ENUM, sal_True);
                    pSummaryPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pSummaryPage->setHeaderFooterSettings(pActualPage->getHeaderFooterSettings());

                    // notes-page
                    SdPage* pNotesPage = (SdPage*) mpDoc->AllocPage(sal_False);
                    pNotesPage->SetSize(pActualNotesPage->GetSize());
                    pNotesPage->SetBorder(pActualNotesPage->GetLftBorder(),
                                          pActualNotesPage->GetUppBorder(),
                                          pActualNotesPage->GetRgtBorder(),
                                          pActualNotesPage->GetLwrBorder() );
                    pNotesPage->SetPageKind(PK_NOTES);

                    // insert page at the back
                    mpDoc->InsertPage(pNotesPage, nCount * 2 + 2);

                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

                    // use MasterPage of the current page
                    pNotesPage->TRG_SetMasterPage(pActualNotesPage->TRG_GetMasterPage());
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), sal_True);
                    pNotesPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pNotesPage->setHeaderFooterSettings(pActualNotesPage->getHeaderFooterSettings());

                    pOutl = new ::sd::Outliner( mpDoc, OUTLINERMODE_OUTLINEOBJECT );
                    pOutl->SetUpdateMode(sal_False);
                    pOutl->EnableUndo(sal_False);

                    if (mpDocSh)
                        pOutl->SetRefDevice(SD_MOD()->GetRefDevice( *mpDocSh ));

                    pOutl->SetDefTab( mpDoc->GetDefaultTabulator() );
                    pOutl->SetStyleSheetPool((SfxStyleSheetPool*) mpDoc->GetStyleSheetPool());
                    pStyle = pSummaryPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
                    pOutl->SetStyleSheet( 0, pStyle );
                }

                // add text
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

        // remove hard break- and character attributes
        SfxItemSet aEmptyEEAttr(mpDoc->GetPool(), EE_ITEMS_START, EE_ITEMS_END);
        sal_Int32 nParaCount = pOutl->GetParagraphCount();

        for (sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
        {
            pOutl->SetStyleSheet( nPara, pStyle );
            pOutl->QuickRemoveCharAttribs(nPara);
            pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
            pOutl->SetDepth(pOutl->GetParagraph(nPara), 0);
        }

        pTextObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
        pTextObj->SetEmptyPresObj(sal_False);

        // remove hard attributes (Flag to sal_True)
        SfxItemSet aAttr(mpDoc->GetPool());
        aAttr.Put(XLineStyleItem(XLINE_NONE));
        aAttr.Put(XFillStyleItem(XFILL_NONE));
        pTextObj->SetMergedItemSet(aAttr);

        if( bBegUndo )
            mpView->EndUndo();
        delete pOutl;

        DrawViewShell* pDrawViewShell= dynamic_cast< DrawViewShell* >( mpViewShell );
        if(pDrawViewShell)
        {
            pDrawViewShell->SwitchPage( (pSummaryPage->GetPageNum() - 1) / 2);
        }
    }
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
