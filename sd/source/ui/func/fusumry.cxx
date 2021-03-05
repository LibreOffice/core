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

#include <fusumry.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdundo.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <editeng/outlobj.hxx>
#include <xmloff/autolayout.hxx>

#include <strings.hrc>

#include <pres.hxx>
#include <View.hxx>
#include <sdpage.hxx>
#include <Outliner.hxx>
#include <drawdoc.hxx>
#include <ViewShell.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <DrawViewShell.hxx>

using namespace com::sun::star;

namespace sd {


FuSummaryPage::FuSummaryPage (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuSummaryPage::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuSummaryPage( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSummaryPage::DoExecute( SfxRequest& )
{
    std::unique_ptr<SdOutliner> pOutl;
    rtl::Reference<SdPage> pSummaryPage;
    sal_uInt16 i = 0;
    sal_uInt16 nFirstPage = SDRPAGE_NOTFOUND;
    sal_uInt16 nSelectedPages = 0;
    sal_uInt16 nCount = mpDoc->GetSdPageCount(PageKind::Standard);

    while (i < nCount && nSelectedPages <= 1)
    {
        /* How many pages are selected?
             exactly one: pool everything from this page
             otherwise:   only pool the selected pages  */
        SdPage* pActualPage = mpDoc->GetSdPage(i, PageKind::Standard);

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

    SfxStyleSheet* pStyle = nullptr;

    for (i = nFirstPage; i < nCount; i++)
    {
        SdPage* pActualPage = mpDoc->GetSdPage(i, PageKind::Standard);

        if (nSelectedPages <= 1 || pActualPage->IsSelected())
        {
            SdPage* pActualNotesPage = mpDoc->GetSdPage(i, PageKind::Notes);
            SdrTextObj* pTextObj = static_cast<SdrTextObj*>( pActualPage->GetPresObj(PresObjKind::Title) );

            if (pTextObj && !pTextObj->IsEmptyPresObj())
            {
                if (!pSummaryPage)
                {
                    // insert "table of content"-page and create outliner
                    const bool bUndo = mpView->IsUndoEnabled();

                    if( bUndo )
                    {
                        mpView->BegUndo(SdResId(STR_UNDO_SUMMARY_PAGE));
                        bBegUndo = true;
                    }

                    SdrLayerIDSet aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

                    // page with title & structuring!
                    pSummaryPage = mpDoc->AllocSdPage(false);
                    pSummaryPage->SetSize(pActualPage->GetSize() );
                    pSummaryPage->SetBorder(pActualPage->GetLeftBorder(),
                                     pActualPage->GetUpperBorder(),
                                     pActualPage->GetRightBorder(),
                                     pActualPage->GetLowerBorder() );

                    // insert page at the back
                    mpDoc->InsertPage(pSummaryPage.get(), nCount * 2 + 1);
                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pSummaryPage));

                    // use MasterPage of the current page
                    pSummaryPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pSummaryPage->SetLayoutName(pActualPage->GetLayoutName());
                    pSummaryPage->SetAutoLayout(AUTOLAYOUT_TITLE_CONTENT, true);
                    pSummaryPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pSummaryPage->setHeaderFooterSettings(pActualPage->getHeaderFooterSettings());

                    // notes-page
                    rtl::Reference<SdPage> pNotesPage = mpDoc->AllocSdPage(false);
                    pNotesPage->SetSize(pActualNotesPage->GetSize());
                    pNotesPage->SetBorder(pActualNotesPage->GetLeftBorder(),
                                          pActualNotesPage->GetUpperBorder(),
                                          pActualNotesPage->GetRightBorder(),
                                          pActualNotesPage->GetLowerBorder() );
                    pNotesPage->SetPageKind(PageKind::Notes);

                    // insert page at the back
                    mpDoc->InsertPage(pNotesPage.get(), nCount * 2 + 2);

                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

                    // use MasterPage of the current page
                    pNotesPage->TRG_SetMasterPage(pActualNotesPage->TRG_GetMasterPage());
                    pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                    pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), true);
                    pNotesPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
                    pNotesPage->setHeaderFooterSettings(pActualNotesPage->getHeaderFooterSettings());

                    pOutl.reset(new SdOutliner( mpDoc, OutlinerMode::OutlineObject ));
                    pOutl->SetUpdateMode(false);
                    pOutl->EnableUndo(false);

                    if (mpDocSh)
                        pOutl->SetRefDevice(SD_MOD()->GetVirtualRefDevice());

                    pOutl->SetDefTab( mpDoc->GetDefaultTabulator() );
                    pOutl->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(mpDoc->GetStyleSheetPool()));
                    pStyle = pSummaryPage->GetStyleSheetForPresObj( PresObjKind::Outline );
                    pOutl->SetStyleSheet( 0, pStyle );
                }

                // add text
                OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
                // #118876#, check if the OutlinerParaObject is created successfully
                if( pParaObj )
                {
                    pParaObj->SetOutlinerMode( OutlinerMode::OutlineObject );
                    pOutl->AddText(*pParaObj);
                }
            }
        }
    }

    if (!pSummaryPage)
        return;

    SdrTextObj* pTextObj = static_cast<SdrTextObj*>( pSummaryPage->GetPresObj(PresObjKind::Outline) );

    if (!pTextObj)
        return;

    // remove hard break- and character attributes
    SfxItemSet aEmptyEEAttr(mpDoc->GetPool(), svl::Items<EE_ITEMS_START, EE_ITEMS_END>{});
    sal_Int32 nParaCount = pOutl->GetParagraphCount();

    for (sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
    {
        pOutl->SetStyleSheet( nPara, pStyle );
        pOutl->RemoveCharAttribs(nPara);
        pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
        pOutl->SetDepth(pOutl->GetParagraph(nPara), 0);
    }

    pTextObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
    pTextObj->SetEmptyPresObj(false);

    // remove hard attributes (Flag to sal_True)
    SfxItemSet aAttr(mpDoc->GetPool());
    aAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));
    aAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));
    pTextObj->SetMergedItemSet(aAttr);

    if( bBegUndo )
        mpView->EndUndo();
    pOutl.reset();

    DrawViewShell* pDrawViewShell= dynamic_cast< DrawViewShell* >( mpViewShell );
    if(pDrawViewShell)
    {
        pDrawViewShell->SwitchPage( (pSummaryPage->GetPageNum() - 1) / 2);
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
