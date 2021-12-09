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

#include <config_features.h>

#include <officecfg/Office/Common.hxx>
#include <sal/log.hxx>
#include <sfx2/opengrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/linkwarn.hxx>
#include <svx/svxids.hrc>
#include <vcl/graphicfilter.hxx>
#include <svl/stritem.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/GraphicNativeTransform.hxx>
#include <vcl/GraphicNativeMetadata.hxx>
#include <fuinsert.hxx>
#include <tabvwsh.hxx>
#include <drwlayer.hxx>
#include <drawview.hxx>
#include <document.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <globstr.hrc>
#include <comphelper/lok.hxx>

#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/uno/Sequence.hxx>

using namespace css;
using namespace css::uno;

void ScLimitSizeOnDrawPage( Size& rSize, Point& rPos, const Size& rPage )
{
    if ( !rPage.Width() || !rPage.Height() )
        return;

    Size aPageSize = rPage;
    bool bNegative = aPageSize.Width() < 0;
    if ( bNegative )
    {
        //  make everything positive temporarily
        aPageSize.setWidth( -aPageSize.Width() );
        rPos.setX( -rPos.X() - rSize.Width() );
    }

    if ( rSize.Width() > aPageSize.Width() || rSize.Height() > aPageSize.Height() )
    {
        double fX = aPageSize.Width()  / static_cast<double>(rSize.Width());
        double fY = aPageSize.Height() / static_cast<double>(rSize.Height());

        if ( fX < fY )
        {
            rSize.setWidth( aPageSize.Width() );
            rSize.setHeight( static_cast<tools::Long>( rSize.Height() * fX ) );
        }
        else
        {
            rSize.setHeight( aPageSize.Height() );
            rSize.setWidth( static_cast<tools::Long>( rSize.Width() * fY ) );
        }

        if (!rSize.Width())
            rSize.setWidth( 1 );
        if (!rSize.Height())
            rSize.setHeight( 1 );
    }

    if ( rPos.X() + rSize.Width() > aPageSize.Width() )
        rPos.setX( aPageSize.Width() - rSize.Width() );
    if ( rPos.Y() + rSize.Height() > aPageSize.Height() )
        rPos.setY( aPageSize.Height() - rSize.Height() );

    if ( bNegative )
        rPos.setX( -rPos.X() - rSize.Width() );       // back to real position
}

static void lcl_InsertGraphic( const Graphic& rGraphic,
                        const OUString& rFileName, bool bAsLink, bool bApi,
                        ScTabViewShell& rViewSh, const vcl::Window* pWindow, SdrView* pView,
                        ScAnchorType aAnchorType = SCA_CELL )
{
    Graphic& rGraphic1 = const_cast<Graphic &>(rGraphic);
    GraphicNativeMetadata aMetadata;
    if ( aMetadata.read(rGraphic1) )
    {
        const Degree10 aRotation = aMetadata.getRotation();
        if (aRotation)
        {
            GraphicNativeTransform aTransform( rGraphic1 );
            aTransform.rotate( aRotation );
        }
    }
    ScDrawView* pDrawView = rViewSh.GetScDrawView();

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
            const OUString aBeginUndo(ScResId(STR_UNDO_DRAGDROP));

            SdrObject* pResult = pDrawView->ApplyGraphicToObject(
                *pPickObj,
                rGraphic1,
                aBeginUndo,
                bAsLink ? rFileName : OUString());

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
    Point aInsertPos = rViewSh.GetInsertPos();

    ScViewData& rData = rViewSh.GetViewData();
    if ( rData.GetDocument().IsNegativePage( rData.GetTabNo() ) )
        aInsertPos.AdjustX( -(aLogicSize.Width()) );       // move position to left edge

    ScLimitSizeOnDrawPage( aLogicSize, aInsertPos, pPage->GetSize() );

    tools::Rectangle aRect ( aInsertPos, aLogicSize );

    SdrGrafObj* pObj = new SdrGrafObj(
        pView->getSdrModelFromSdrView(), // TTTT pView should be reference
        rGraphic1,
        aRect);

    // calling SetGraphicLink here doesn't work
    // Yes, due to the SdrObject had no SdrModel
    //  Path is no longer used as name for the graphics object

    ScDrawLayer* pLayer = static_cast<ScDrawLayer*>(pView->GetModel());
    OUString aName = pLayer->GetNewGraphicName();                 // "Graphics"
    pObj->SetName(aName);

    if (aAnchorType == SCA_CELL || aAnchorType == SCA_CELL_RESIZE)
        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rData.GetDocument(), rData.GetTabNo(),
                                                 aAnchorType == SCA_CELL_RESIZE);

    //  don't select if from (dispatch) API, to allow subsequent cell operations
    SdrInsertFlags nInsOptions = (bApi && !comphelper::LibreOfficeKit::isActive()) ? SdrInsertFlags::DONTMARK : SdrInsertFlags::NONE;
    bool bSuccess = pView->InsertObjectAtView( pObj, *pPV, nInsOptions );

    // SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
    if (bSuccess && bAsLink)
        pObj->SetGraphicLink( rFileName );
}

static void lcl_InsertMedia( const OUString& rMediaURL, bool bApi,
                      ScTabViewShell* pViewSh, const vcl::Window* pWindow, SdrView* pView,
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
            aSize = pWindow->PixelToLogic(rPrefSize, MapMode(MapUnit::Map100thMM));
        else
            aSize = Application::GetDefaultDevice()->PixelToLogic(rPrefSize, MapMode(MapUnit::Map100thMM));
    }
    else
        aSize = Size( 5000, 5000 );

    ScLimitSizeOnDrawPage( aSize, aInsertPos, pPage->GetSize() );

    if( rData.GetDocument().IsNegativePage( rData.GetTabNo() ) )
        aInsertPos.AdjustX( -(aSize.Width()) );

    OUString realURL;
    if (bLink)
    {
        realURL = rMediaURL;
    }
    else
    {
        uno::Reference<frame::XModel> const xModel(
                rData.GetDocument().GetDocumentShell()->GetModel());
#if HAVE_FEATURE_AVMEDIA
        bool const bRet = ::avmedia::EmbedMedia(xModel, rMediaURL, realURL);
        if (!bRet) { return; }
#else
        return;
#endif
    }

    SdrMediaObj* pObj = new SdrMediaObj(
        *rData.GetDocument().GetDrawLayer(),
        tools::Rectangle(aInsertPos, aSize));

    pObj->setURL( realURL, ""/*TODO?*/ );
    pView->InsertObjectAtView( pObj, *pPV, bApi ? SdrInsertFlags::DONTMARK : SdrInsertFlags::NONE );
}

FuInsertGraphic::FuInsertGraphic( ScTabViewShell&   rViewSh,
                                  vcl::Window*      pWin,
                                  ScDrawView*       pViewP,
                                  SdrModel*         pDoc,
                                  SfxRequest&       rReq )
       : FuPoor(rViewSh, pWin, pViewP, pDoc, rReq)
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
            lcl_InsertGraphic( aGraphic, aFileName, bAsLink, true, rViewSh, pWindow, pView );
        }
    }
    else
    {
        SvxOpenGraphicDialog aDlg(ScResId(STR_INSERTGRAPHIC), pWin ? pWin->GetFrameWeld() : nullptr,
                                  ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR);

        Reference<ui::dialogs::XFilePickerControlAccess> xCtrlAcc = aDlg.GetFilePickerControlAccess();
        sal_Int16 nSelect = 0;
        Sequence<OUString> aListBoxEntries {
            ScResId(STR_ANCHOR_TO_CELL),
            ScResId(STR_ANCHOR_TO_CELL_RESIZE),
            ScResId(STR_ANCHOR_TO_PAGE)
        };
        try
        {
            Any aTemplates(&aListBoxEntries, cppu::UnoType<decltype(aListBoxEntries)>::get());

            xCtrlAcc->setValue(ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR,
                ui::dialogs::ListboxControlActions::ADD_ITEMS, aTemplates);

            Any aSelectPos(&nSelect, cppu::UnoType<decltype(nSelect)>::get());
            xCtrlAcc->setValue(ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR,
                ui::dialogs::ListboxControlActions::SET_SELECT_ITEM, aSelectPos);
        }
        catch (const Exception&)
        {
            SAL_WARN("sc", "control access failed");
        }

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            Graphic aGraphic;
            ErrCode nError = aDlg.GetGraphic(aGraphic);
            if( nError == ERRCODE_NONE )
            {
                OUString aFileName = aDlg.GetPath();
                const OUString& aFilterName = aDlg.GetDetectedFilter();
                bool bAsLink = aDlg.IsAsLink();

                // really store as link only?
                if( bAsLink && officecfg::Office::Common::Misc::ShowLinkWarningDialog::get() )
                {
                    SvxLinkWarningDialog aWarnDlg(pWin ? pWin->GetFrameWeld() : nullptr, aFileName);
                    if (aWarnDlg.run() != RET_OK)
                        bAsLink = false; // don't store as link
                }

                // Anchor to cell or to page?
                Any aAnchorValue = xCtrlAcc->getValue(
                    ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR,
                    ui::dialogs::ListboxControlActions::GET_SELECTED_ITEM );
                OUString sAnchor;
                aAnchorValue >>= sAnchor;

                ScAnchorType aAnchorType;
                if (sAnchor == ScResId(STR_ANCHOR_TO_CELL))
                    aAnchorType = SCA_CELL;
                else if (sAnchor == ScResId(STR_ANCHOR_TO_CELL_RESIZE))
                    aAnchorType = SCA_CELL_RESIZE;
                else if (sAnchor == ScResId(STR_ANCHOR_TO_PAGE))
                    aAnchorType = SCA_PAGE;
                else
                    aAnchorType = SCA_DONTKNOW;

                lcl_InsertGraphic( aGraphic, aFileName, bAsLink, false, rViewSh, pWindow, pView, aAnchorType );

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

FuInsertMedia::FuInsertMedia( ScTabViewShell&   rViewSh,
                              vcl::Window*      pWin,
                              ScDrawView*       pViewP,
                              SdrModel*         pDoc,
                              const SfxRequest& rReq ) :
    FuPoor(rViewSh, pWin, pViewP, pDoc, rReq)
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
    bool bInsertMedia = bAPI;
#if HAVE_FEATURE_AVMEDIA
    if (!bInsertMedia)
        bInsertMedia = ::avmedia::MediaWindow::executeMediaURLDialog(pWin ? pWin->GetFrameWeld() : nullptr, aURL, &bLink);
#endif
    if (!bInsertMedia)
        return;

    Size aPrefSize;

    if( pWin )
        pWin->EnterWait();

#if HAVE_FEATURE_AVMEDIA
    if( !::avmedia::MediaWindow::isMediaURL( aURL, ""/*TODO?*/, true, &aPrefSize ) )
    {
        if( pWin )
            pWin->LeaveWait();

        if( !bAPI )
            ::avmedia::MediaWindow::executeFormatErrorBox(pWindow ? pWindow->GetFrameWeld() : nullptr);
    }
    else
#endif
    {
        lcl_InsertMedia( aURL, bAPI, &rViewSh, pWindow, pView, aPrefSize,
                bLink );

        if( pWin )
            pWin->LeaveWait();
    }
}

FuInsertMedia::~FuInsertMedia()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
