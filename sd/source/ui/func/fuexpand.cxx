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

#include <fuexpand.hxx>

#include <sfx2/viewfrm.hxx>
#include <svx/svdotext.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svdundo.hxx>
#include <sfx2/printer.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdetc.hxx>
#include <xmloff/autolayout.hxx>
#include <sal/log.hxx>

#include <app.hrc>
#include <strings.hrc>
#include <pres.hxx>
#include <View.hxx>
#include <sdpage.hxx>
#include <Outliner.hxx>
#include <drawview.hxx>
#include <drawdoc.hxx>
#include <ViewShell.hxx>
#include <DrawDocShell.hxx>
#include <sdresid.hxx>
#include <optsitem.hxx>
#include <sdmod.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/eeitem.hxx>

using namespace com::sun::star;

namespace sd {


FuExpandPage::FuExpandPage (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuExpandPage::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuExpandPage( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuExpandPage::DoExecute( SfxRequest& )
{
    if ( mpView && mpView->IsTextEdit() )
        mpView->SdrEndTextEdit();

    // find selected page (only standard pages)
    SdPage* pActualPage = nullptr;
    sal_uInt16 i = 0;
    sal_uInt16 nCount = mpDoc->GetSdPageCount(PageKind::Standard);

    while (!pActualPage && i < nCount)
    {
        if (mpDoc->GetSdPage(i, PageKind::Standard)->IsSelected())
        {
            pActualPage = mpDoc->GetSdPage(i, PageKind::Standard);
        }

        i++;
    }

    if (!pActualPage)
        return;

    SdOutliner aOutliner( mpDoc, OutlinerMode::OutlineObject );
    aOutliner.SetUpdateMode(false);
    aOutliner.EnableUndo(false);

    if (mpDocSh)
        aOutliner.SetRefDevice( SD_MOD()->GetVirtualRefDevice() );

    aOutliner.SetDefTab( mpDoc->GetDefaultTabulator() );
    aOutliner.SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(mpDoc->GetStyleSheetPool()));

    SdrLayerIDSet aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();
    sal_uInt16 nActualPageNum = pActualPage->GetPageNum();
    SdPage* pActualNotesPage = static_cast<SdPage*>(mpDoc->GetPage(nActualPageNum + 1));
    SdrTextObj* pActualOutline = static_cast<SdrTextObj*>(pActualPage->GetPresObj(PRESOBJ_OUTLINE));

    if (pActualOutline)
    {
        const bool bUndo = mpView->IsUndoEnabled();

        if( bUndo )
            mpView->BegUndo(SdResId(STR_UNDO_EXPAND_PAGE));

        // set current structuring-object into outliner
        OutlinerParaObject* pParaObj = pActualOutline->GetOutlinerParaObject();
        aOutliner.SetText(*pParaObj);

        // remove hard paragraph- and character attributes
        SfxItemSet aEmptyEEAttr(mpDoc->GetPool(), svl::Items<EE_ITEMS_START, EE_ITEMS_END>{});
        sal_Int32 nParaCount1 = aOutliner.GetParagraphCount();

        for (sal_Int32 nPara = 0; nPara < nParaCount1; nPara++)
        {
            aOutliner.RemoveCharAttribs(nPara);
            aOutliner.SetParaAttribs(nPara, aEmptyEEAttr);
        }

        sal_uInt16 nPos = 2;
        Paragraph* pPara = aOutliner.GetParagraph( 0 );

        while (pPara)
        {
            sal_Int32 nParaPos = aOutliner.GetAbsPos( pPara );
            sal_Int16 nDepth = aOutliner.GetDepth( nParaPos );
            if ( nDepth == 0 )
            {
                // page with title & structuring!
                SdPage* pPage = mpDoc->AllocSdPage(false);
                pPage->SetSize(pActualPage->GetSize() );
                pPage->SetBorder(pActualPage->GetLeftBorder(),
                                 pActualPage->GetUpperBorder(),
                                 pActualPage->GetRightBorder(),
                                 pActualPage->GetLowerBorder() );
                pPage->SetName(OUString());

                // insert page after current page
                mpDoc->InsertPage(pPage, nActualPageNum + nPos);
                nPos++;

                if( bUndo )
                    mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pPage));

                // use MasterPage of the current page
                pPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                pPage->SetLayoutName(pActualPage->GetLayoutName());
                pPage->SetAutoLayout(AUTOLAYOUT_TITLE_CONTENT, true);
                pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

                // notes-page
                SdPage* pNotesPage = mpDoc->AllocSdPage(false);
                pNotesPage->SetSize(pActualNotesPage->GetSize());
                pNotesPage->SetBorder(pActualNotesPage->GetLeftBorder(),
                                      pActualNotesPage->GetUpperBorder(),
                                      pActualNotesPage->GetRightBorder(),
                                      pActualNotesPage->GetLowerBorder() );
                pNotesPage->SetPageKind(PageKind::Notes);
                pNotesPage->SetName(OUString());

                // insert page after current page
                mpDoc->InsertPage(pNotesPage, nActualPageNum + nPos);
                nPos++;

                if( bUndo )
                    mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

                // use MasterPage of the current page
                pNotesPage->TRG_SetMasterPage(pActualNotesPage->TRG_GetMasterPage());
                pNotesPage->SetLayoutName(pActualNotesPage->GetLayoutName());
                pNotesPage->SetAutoLayout(pActualNotesPage->GetAutoLayout(), true);
                pNotesPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

                // create title text objects
                SdrTextObj* pTextObj = static_cast<SdrTextObj*>(pPage->GetPresObj(PRESOBJ_TITLE));
                SAL_WARN_IF(!pTextObj, "sd.core", "worrying lack of PRESOBJ_TITLE object");
                if (!pTextObj)
                    continue;

                std::unique_ptr<OutlinerParaObject> pOutlinerParaObject = aOutliner.CreateParaObject( nParaPos, 1);
                pOutlinerParaObject->SetOutlinerMode(OutlinerMode::TitleObject);

                if( pOutlinerParaObject->GetDepth(0) != -1 )
                {
                    std::unique_ptr<SdrOutliner> pTempOutl = SdrMakeOutliner(OutlinerMode::TitleObject, *mpDoc);

                    pTempOutl->SetText( *pOutlinerParaObject );

                    pOutlinerParaObject.reset();

                    pTempOutl->SetDepth( pTempOutl->GetParagraph( 0 ), -1 );

                    pOutlinerParaObject = pTempOutl->CreateParaObject();
                }

                pTextObj->SetOutlinerParaObject(std::move(pOutlinerParaObject));

                pTextObj->SetEmptyPresObj(false);

                SfxStyleSheet* pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                pTextObj->NbcSetStyleSheet(pSheet, false);

                SdrTextObj* pOutlineObj = nullptr;
                sal_Int32 nChildCount = aOutliner.GetChildCount(pPara);
                if (nChildCount > 0)
                    pOutlineObj = static_cast<SdrTextObj*>( pPage->GetPresObj(PRESOBJ_OUTLINE) );
                if (pOutlineObj)
                {
                    // create structuring text objects
                    std::unique_ptr<OutlinerParaObject> pOPO = aOutliner.CreateParaObject(++nParaPos, nChildCount);

                    std::unique_ptr<SdrOutliner> pTempOutl = SdrMakeOutliner(OutlinerMode::OutlineObject, *mpDoc);
                    pTempOutl->SetText( *pOPO );

                    sal_Int32 nParaCount2 = pTempOutl->GetParagraphCount();
                    sal_Int32 nPara;
                    for( nPara = 0; nPara < nParaCount2; nPara++ )
                    {
                        pTempOutl->SetDepth (
                            pTempOutl->GetParagraph( nPara ),
                            pTempOutl->GetDepth( nPara ) - 1);
                    }

                    pOPO = pTempOutl->CreateParaObject();
                    pTempOutl.reset();

                    pOutlineObj->SetOutlinerParaObject( std::move(pOPO) );
                    pOutlineObj->SetEmptyPresObj(false);

                    // remove hard attributes (Flag to sal_True)
                    SfxItemSet aAttr(mpDoc->GetPool());
                    aAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));
                    aAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));
                    pOutlineObj->SetMergedItemSet(aAttr);
                }
            }

            pPara = aOutliner.GetParagraph( ++nParaPos );
        }

        if( bUndo )
            mpView->EndUndo();
    }

    mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE_PAGE, SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
