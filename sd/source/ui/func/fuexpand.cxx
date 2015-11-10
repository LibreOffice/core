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

using namespace com::sun::star;

namespace sd {

TYPEINIT1( FuExpandPage, FuPoor );

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
    sal_uInt16 nCount = mpDoc->GetSdPageCount(PK_STANDARD);

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
        pOutl->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(mpDoc->GetStyleSheetPool()));

        SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();
        sal_uInt16 nActualPageNum = pActualPage->GetPageNum();
        SdPage* pActualNotesPage = static_cast<SdPage*>(mpDoc->GetPage(nActualPageNum + 1));
        SdrTextObj* pActualOutline = static_cast<SdrTextObj*>(pActualPage->GetPresObj(PRESOBJ_OUTLINE));

        if (pActualOutline)
        {
            const bool bUndo = mpView->IsUndoEnabled();

            if( bUndo )
                mpView->BegUndo(SD_RESSTR(STR_UNDO_EXPAND_PAGE));

            // set current structuring-object into outliner
            OutlinerParaObject* pParaObj = pActualOutline->GetOutlinerParaObject();
            pOutl->SetText(*pParaObj);

            // remove hard paragraph- and character attributes
            SfxItemSet aEmptyEEAttr(mpDoc->GetPool(), EE_ITEMS_START, EE_ITEMS_END);
            sal_Int32 nParaCount1 = pOutl->GetParagraphCount();

            for (sal_Int32 nPara = 0; nPara < nParaCount1; nPara++)
            {
                pOutl->RemoveCharAttribs(nPara);
                pOutl->SetParaAttribs(nPara, aEmptyEEAttr);
            }

            sal_uInt16 nPos = 2;
            Paragraph* pPara = pOutl->GetParagraph( 0 );

            while (pPara)
            {
                sal_Int32 nParaPos = pOutl->GetAbsPos( pPara );
                sal_Int16 nDepth = pOutl->GetDepth( nParaPos );
                if ( nDepth == 0 )
                {
                    // page with title & structuring!
                    SdPage* pPage = mpDoc->AllocSdPage(false);
                    pPage->SetSize(pActualPage->GetSize() );
                    pPage->SetBorder(pActualPage->GetLftBorder(),
                                     pActualPage->GetUppBorder(),
                                     pActualPage->GetRgtBorder(),
                                     pActualPage->GetLwrBorder() );
                    pPage->SetName(OUString());

                    // insert page after current page
                    mpDoc->InsertPage(pPage, nActualPageNum + nPos);
                    nPos++;

                    if( bUndo )
                        mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewPage(*pPage));

                    // use MasterPage of the current page
                    pPage->TRG_SetMasterPage(pActualPage->TRG_GetMasterPage());
                    pPage->SetLayoutName(pActualPage->GetLayoutName());
                    pPage->SetAutoLayout(AUTOLAYOUT_ENUM, true);
                    pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

                    // notes-page
                    SdPage* pNotesPage = mpDoc->AllocSdPage(false);
                    pNotesPage->SetSize(pActualNotesPage->GetSize());
                    pNotesPage->SetBorder(pActualNotesPage->GetLftBorder(),
                                          pActualNotesPage->GetUppBorder(),
                                          pActualNotesPage->GetRgtBorder(),
                                          pActualNotesPage->GetLwrBorder() );
                    pNotesPage->SetPageKind(PK_NOTES);
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

                    OutlinerParaObject* pOutlinerParaObject = pOutl->CreateParaObject( nParaPos, 1);
                    pOutlinerParaObject->SetOutlinerMode(OUTLINERMODE_TITLEOBJECT);

                    if( pOutlinerParaObject->GetDepth(0) != -1 )
                    {
                        SdrOutliner* pTempOutl = SdrMakeOutliner(OUTLINERMODE_TITLEOBJECT, *mpDoc);

                        pTempOutl->SetText( *pOutlinerParaObject );

                        delete pOutlinerParaObject;

                        pTempOutl->SetDepth( pTempOutl->GetParagraph( 0 ), -1 );

                        pOutlinerParaObject = pTempOutl->CreateParaObject();
                        delete pTempOutl;
                    }

                    pTextObj->SetOutlinerParaObject(pOutlinerParaObject);

                    pTextObj->SetEmptyPresObj(false);

                    SfxStyleSheet* pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                    pTextObj->NbcSetStyleSheet(pSheet, false);

                    SdrTextObj* pOutlineObj = nullptr;
                    sal_Int32 nChildCount = pOutl->GetChildCount(pPara);
                    if (nChildCount > 0)
                        pOutlineObj = static_cast<SdrTextObj*>( pPage->GetPresObj(PRESOBJ_OUTLINE) );
                    if (pOutlineObj)
                    {
                        // create structuring text objects
                        OutlinerParaObject* pOPO = pOutl->CreateParaObject(++nParaPos, nChildCount);

                        SdrOutliner* pTempOutl = SdrMakeOutliner(OUTLINERMODE_OUTLINEOBJECT, *mpDoc);
                        pTempOutl->SetText( *pOPO );

                        sal_Int32 nParaCount2 = pTempOutl->GetParagraphCount();
                        sal_Int32 nPara;
                        for( nPara = 0; nPara < nParaCount2; nPara++ )
                        {
                            pTempOutl->SetDepth (
                                pTempOutl->GetParagraph( nPara ),
                                pTempOutl->GetDepth( nPara ) - 1);
                        }

                        delete pOPO;
                        pOPO = pTempOutl->CreateParaObject();
                        delete pTempOutl;

                        pOutlineObj->SetOutlinerParaObject( pOPO );
                        pOutlineObj->SetEmptyPresObj(false);

                        // remove hard attributes (Flag to sal_True)
                        SfxItemSet aAttr(mpDoc->GetPool());
                        aAttr.Put(XLineStyleItem(drawing::LineStyle_NONE));
                        aAttr.Put(XFillStyleItem(drawing::FillStyle_NONE));
                        pOutlineObj->SetMergedItemSet(aAttr);
                    }
                }

                pPara = pOutl->GetParagraph( ++nParaPos );
            }

            if( bUndo )
                mpView->EndUndo();
        }

        delete pOutl;

        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE_PAGE, SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
