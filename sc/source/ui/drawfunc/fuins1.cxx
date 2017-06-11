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

#include <sfx2/opengrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/linkwarn.hxx>
#include <vcl/graphicfilter.hxx>
#include <svl/stritem.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/msgbox.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>

#include "fuinsert.hxx"
#include "tabvwsh.hxx"
#include "drwlayer.hxx"
#include "drawview.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "progress.hxx"
#include "strings.hrc"
#include "globstr.hrc"

using namespace ::com::sun::star;

void ScLimitSizeOnDrawPage( Size& rSize, Point& rPos, const Size& rPage )
{
    if ( !rPage.Width() || !rPage.Height() )
        return;

    Size aPageSize = rPage;
    bool bNegative = aPageSize.Width() < 0;
    if ( bNegative )
    {
        //  make everything positive temporarily
        aPageSize.Width() = -aPageSize.Width();
        rPos.X() = -rPos.X() - rSize.Width();
    }

    if ( rSize.Width() > aPageSize.Width() || rSize.Height() > aPageSize.Height() )
    {
        double fX = aPageSize.Width()  / (double) rSize.Width();
        double fY = aPageSize.Height() / (double) rSize.Height();

        if ( fX < fY )
        {
            rSize.Width()  = aPageSize.Width();
            rSize.Height() = (long) ( rSize.Height() * fX );
        }
        else
        {
            rSize.Height() = aPageSize.Height();
            rSize.Width()  = (long) ( rSize.Width() * fY );
        }

        if (!rSize.Width())
            rSize.Width() = 1;
        if (!rSize.Height())
            rSize.Height() = 1;
    }

    if ( rPos.X() + rSize.Width() > aPageSize.Width() )
        rPos.X() = aPageSize.Width() - rSize.Width();
    if ( rPos.Y() + rSize.Height() > aPageSize.Height() )
        rPos.Y() = aPageSize.Height() - rSize.Height();

    if ( bNegative )
        rPos.X() = -rPos.X() - rSize.Width();       // back to real position
}

static void lcl_InsertGraphic( const Graphic& rGraphic,
                        const OUString& rFileName, const OUString& rFilterName, bool bAsLink, bool bApi,
                        ScTabViewShell* pViewSh, vcl::Window* pWindow, SdrView* pView )
{
    ScDrawView* pDrawView = pViewSh->GetScDrawView();

    // #i123922# check if an existing object is selected; if yes, evtl. replace
    // the graphic for a SdrGraphObj (including link state updates) or adapt the fill
    // style for other objects
    if(pDrawView && 1 == pDrawView->GetMarkedObjectCount())
    {
        SdrObject* pPickObj = pDrawView->GetMarkedObjectByIndex(0);

        if(pPickObj)
        {
            //sal_Int8 nAction(DND_ACTION_MOVE);
            //Point aPos;
            const OUString aBeginUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));

            SdrObject* pResult = pDrawView->ApplyGraphicToObject(
                *pPickObj,
                rGraphic,
                aBeginUndo,
                bAsLink ? rFileName : OUString(),
                bAsLink ? rFilterName : OUString());

            if(pResult)
            {
                // we are done; mark the modified/new object
                pDrawView->MarkObj(pResult, pDrawView->GetSdrPageView());
                return;
            }
        }
    }

    //  set the size so the graphic has its original pixel size
    //  at 100% view scale (as in SetMarkedOriginalSize),
    //  instead of respecting the current view scale
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MapUnit::Map100thMM );
    if ( aSourceMap.GetMapUnit() == MapUnit::MapPixel && pDrawView )
    {
        Fraction aScaleX, aScaleY;
        pDrawView->CalcNormScale( aScaleX, aScaleY );
        aDestMap.SetScaleX(aScaleX);
        aDestMap.SetScaleY(aScaleY);
    }
    Size aLogicSize = pWindow->LogicToLogic(
                            rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );

    //  Limit size

    SdrPageView* pPV  = pView->GetSdrPageView();
    SdrPage* pPage = pPV->GetPage();
    Point aInsertPos = pViewSh->GetInsertPos();

    ScViewData& rData = pViewSh->GetViewData();
    if ( rData.GetDocument()->IsNegativePage( rData.GetTabNo() ) )
        aInsertPos.X() -= aLogicSize.Width();       // move position to left edge

    ScLimitSizeOnDrawPage( aLogicSize, aInsertPos, pPage->GetSize() );

    tools::Rectangle aRect ( aInsertPos, aLogicSize );

    SdrGrafObj* pObj = new SdrGrafObj( rGraphic, aRect );

    // calling SetGraphicLink here doesn't work

    //  Path is no longer used as name for the graphics object

    ScDrawLayer* pLayer = static_cast<ScDrawLayer*>(pView->GetModel());
    OUString aName = pLayer->GetNewGraphicName();                 // "Graphics"
    pObj->SetName(aName);

    //  don't select if from (dispatch) API, to allow subsequent cell operations
    SdrInsertFlags nInsOptions = bApi ? SdrInsertFlags::DONTMARK : SdrInsertFlags::NONE;
    pView->InsertObjectAtView( pObj, *pPV, nInsOptions );

    // SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
    if ( bAsLink )
        pObj->SetGraphicLink( rFileName, ""/*TODO?*/, rFilterName );
}

static void lcl_InsertMedia( const OUString& rMediaURL, bool bApi,
                      ScTabViewShell* pViewSh, vcl::Window* pWindow, SdrView* pView,
                      const Size& rPrefSize, bool const bLink )
{
    SdrPageView*    pPV  = pView->GetSdrPageView();
    SdrPage*        pPage = pPV->GetPage();
    ScViewData&     rData = pViewSh->GetViewData();
    Point           aInsertPos( pViewSh->GetInsertPos() );
    Size            aSize;

    if( rPrefSize.Width() && rPrefSize.Height() )
    {
        if( pWindow )
            aSize = pWindow->PixelToLogic( rPrefSize, MapUnit::Map100thMM );
        else
            aSize = Application::GetDefaultDevice()->PixelToLogic( rPrefSize, MapUnit::Map100thMM );
    }
    else
        aSize = Size( 5000, 5000 );

    ScLimitSizeOnDrawPage( aSize, aInsertPos, pPage->GetSize() );

    if( rData.GetDocument()->IsNegativePage( rData.GetTabNo() ) )
        aInsertPos.X() -= aSize.Width();

    OUString realURL;
    if (bLink)
    {
        realURL = rMediaURL;
    }
    else
    {
        uno::Reference<frame::XModel> const xModel(
                rData.GetDocument()->GetDocumentShell()->GetModel());
        bool const bRet = ::avmedia::EmbedMedia(xModel, rMediaURL, realURL);
        if (!bRet) { return; }
    }

    SdrMediaObj* pObj = new SdrMediaObj( tools::Rectangle( aInsertPos, aSize ) );

    pObj->SetModel(rData.GetDocument()->GetDrawLayer()); // set before setURL
    pObj->setURL( realURL, ""/*TODO?*/ );
    pView->InsertObjectAtView( pObj, *pPV, bApi ? SdrInsertFlags::DONTMARK : SdrInsertFlags::NONE );
}

FuInsertGraphic::FuInsertGraphic( ScTabViewShell*   pViewSh,
                                  vcl::Window*           pWin,
                                  ScDrawView*       pViewP,
                                  SdrModel*         pDoc,
                                  SfxRequest&       rReq )
       : FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if ( pReqArgs &&
         pReqArgs->GetItemState( SID_INSERT_GRAPHIC, true, &pItem ) == SfxItemState::SET )
    {
        OUString aFileName = static_cast<const SfxStringItem*>(pItem)->GetValue();

        OUString aFilterName;
        if ( pReqArgs->GetItemState( FN_PARAM_FILTER, true, &pItem ) == SfxItemState::SET )
            aFilterName = static_cast<const SfxStringItem*>(pItem)->GetValue();

        bool bAsLink = false;
        if ( pReqArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SfxItemState::SET )
            bAsLink = static_cast<const SfxBoolItem*>(pItem)->GetValue();

        Graphic aGraphic;
        ErrCode nError = GraphicFilter::LoadGraphic( aFileName, aFilterName, aGraphic, &GraphicFilter::GetGraphicFilter() );
        if ( nError == ERRCODE_NONE )
        {
            lcl_InsertGraphic( aGraphic, aFileName, aFilterName, bAsLink, true, pViewSh, pWindow, pView );
        }
    }
    else
    {
        SvxOpenGraphicDialog aDlg(ScResId(STR_INSERTGRAPHIC));

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            Graphic aGraphic;
            ErrCode nError = aDlg.GetGraphic(aGraphic);
            if( nError == ERRCODE_NONE )
            {
                OUString aFileName = aDlg.GetPath();
                OUString aFilterName = aDlg.GetCurrentFilter();
                bool bAsLink = aDlg.IsAsLink();

                // really store as link only?
                if( bAsLink && SvtMiscOptions().ShowLinkWarningDialog() )
                {
                    ScopedVclPtrInstance< SvxLinkWarningDialog > aWarnDlg(pWin,aFileName);
                    if( aWarnDlg->Execute() != RET_OK )
                        bAsLink = false; // don't store as link
                }

                lcl_InsertGraphic( aGraphic, aFileName, aFilterName, bAsLink, false, pViewSh, pWindow, pView );

                //  append items for recording
                rReq.AppendItem( SfxStringItem( SID_INSERT_GRAPHIC, aFileName ) );
                rReq.AppendItem( SfxStringItem( FN_PARAM_FILTER, aFilterName ) );
                rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bAsLink ) );
                rReq.Done();
            }
            else
            {
                //  error is handled in SvxOpenGraphicDialog::GetGraphic
            }
        }
    }
}

FuInsertGraphic::~FuInsertGraphic()
{
}

FuInsertMedia::FuInsertMedia( ScTabViewShell*   pViewSh,
                              vcl::Window*           pWin,
                              ScDrawView*       pViewP,
                              SdrModel*         pDoc,
                              SfxRequest&       rReq ) :
    FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    bool                bAPI = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>( &pReqArgs->Get( rReq.GetSlot() )  );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = aURL.getLength();
        }
    }

    bool bLink(true);
    if (bAPI ||
        ::avmedia::MediaWindow::executeMediaURLDialog(aURL, & bLink))
    {
        Size aPrefSize;

        if( pWin )
            pWin->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, ""/*TODO?*/, true, &aPrefSize ) )
        {
            if( pWin )
                pWin->LeaveWait();

            if( !bAPI )
                ::avmedia::MediaWindow::executeFormatErrorBox( pWindow );
        }
        else
        {
            lcl_InsertMedia( aURL, bAPI, pViewSh, pWindow, pView, aPrefSize,
                    bLink );

            if( pWin )
                pWin->LeaveWait();
        }
    }
}

FuInsertMedia::~FuInsertMedia()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
