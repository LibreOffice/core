/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <sfx2/opengrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/linkwarn.hxx>
#include <svtools/filter.hxx>
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
#include "sc.hrc"


using namespace ::com::sun::star;

//------------------------------------------------------------------------

void SC_DLLPUBLIC ScLimitSizeOnDrawPage( Size& rSize, Point& rPos, const Size& rPage )
{
    if ( !rPage.Width() || !rPage.Height() )
        return;

    Size aPageSize = rPage;
    sal_Bool bNegative = aPageSize.Width() < 0;
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

//------------------------------------------------------------------------

static void lcl_InsertGraphic( const Graphic& rGraphic,
                        const String& rFileName, const String& rFilterName, sal_Bool bAsLink, sal_Bool bApi,
                        ScTabViewShell* pViewSh, Window* pWindow, SdrView* pView )
{
    //  set the size so the graphic has its original pixel size
    //  at 100% view scale (as in SetMarkedOriginalSize),
    //  instead of respecting the current view scale

    ScDrawView* pDrawView = pViewSh->GetScDrawView();
    MapMode aSourceMap = rGraphic.GetPrefMapMode();
    MapMode aDestMap( MAP_100TH_MM );
    if ( aSourceMap.GetMapUnit() == MAP_PIXEL && pDrawView )
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

    ScViewData* pData = pViewSh->GetViewData();
    if ( pData->GetDocument()->IsNegativePage( pData->GetTabNo() ) )
        aInsertPos.X() -= aLogicSize.Width();       // move position to left edge

    ScLimitSizeOnDrawPage( aLogicSize, aInsertPos, pPage->GetSize() );

    Rectangle aRect ( aInsertPos, aLogicSize );

    SdrGrafObj* pObj = new SdrGrafObj( rGraphic, aRect );

    // calling SetGraphicLink here doesn't work

    //  Path is no longer used as name for the graphics object

    ScDrawLayer* pLayer = (ScDrawLayer*) pView->GetModel();
    String aName = pLayer->GetNewGraphicName();                 // "Grafik x"
    pObj->SetName(aName);

    //  don't select if from (dispatch) API, to allow subsequent cell operations
    sal_uLong nInsOptions = bApi ? SDRINSERT_DONTMARK : 0;
    pView->InsertObjectAtView( pObj, *pPV, nInsOptions );

    // SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
    if ( bAsLink )
        pObj->SetGraphicLink( rFileName, rFilterName );
}

//------------------------------------------------------------------------

static void lcl_InsertMedia( const ::rtl::OUString& rMediaURL, bool bApi,
                      ScTabViewShell* pViewSh, Window* pWindow, SdrView* pView,
                      const Size& rPrefSize, bool const bLink )
{
    SdrPageView*    pPV  = pView->GetSdrPageView();
    SdrPage*        pPage = pPV->GetPage();
    ScViewData*     pData = pViewSh->GetViewData();
    Point           aInsertPos( pViewSh->GetInsertPos() );
    Size            aSize;

    if( rPrefSize.Width() && rPrefSize.Height() )
    {
        if( pWindow )
            aSize = pWindow->PixelToLogic( rPrefSize, MAP_100TH_MM );
        else
            aSize = Application::GetDefaultDevice()->PixelToLogic( rPrefSize, MAP_100TH_MM );
    }
    else
        aSize = Size( 5000, 5000 );

    ScLimitSizeOnDrawPage( aSize, aInsertPos, pPage->GetSize() );

    if( pData->GetDocument()->IsNegativePage( pData->GetTabNo() ) )
        aInsertPos.X() -= aSize.Width();

    ::rtl::OUString realURL;
    if (bLink)
    {
        realURL = rMediaURL;
    }
    else
    {
        uno::Reference<frame::XModel> const xModel(
                pData->GetDocument()->GetDocumentShell()->GetModel());
        bool const bRet = ::avmedia::EmbedMedia(xModel, rMediaURL, realURL);
        if (!bRet) { return; }
    }

    SdrMediaObj* pObj = new SdrMediaObj( Rectangle( aInsertPos, aSize ) );

    pObj->SetModel(pData->GetDocument()->GetDrawLayer()); // set before setURL
    pObj->setURL( realURL );
    pView->InsertObjectAtView( pObj, *pPV, bApi ? SDRINSERT_DONTMARK : 0 );
}

/*************************************************************************
|*
|* FuInsertGraphic::Konstruktor
|*
\************************************************************************/

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

FuInsertGraphic::FuInsertGraphic( ScTabViewShell*   pViewSh,
                                  Window*           pWin,
                                  ScDrawView*       pViewP,
                                  SdrModel*         pDoc,
                                  SfxRequest&       rReq )
       : FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    if ( pReqArgs &&
         pReqArgs->GetItemState( SID_INSERT_GRAPHIC, sal_True, &pItem ) == SFX_ITEM_SET )
    {
        String aFileName = ((const SfxStringItem*)pItem)->GetValue();

        String aFilterName;
        if ( pReqArgs->GetItemState( FN_PARAM_FILTER, sal_True, &pItem ) == SFX_ITEM_SET )
            aFilterName = ((const SfxStringItem*)pItem)->GetValue();

        sal_Bool bAsLink = false;
        if ( pReqArgs->GetItemState( FN_PARAM_1, sal_True, &pItem ) == SFX_ITEM_SET )
            bAsLink = ((const SfxBoolItem*)pItem)->GetValue();

        Graphic aGraphic;
        int nError = GraphicFilter::LoadGraphic( aFileName, aFilterName, aGraphic, &GraphicFilter::GetGraphicFilter() );
        if ( nError == GRFILTER_OK )
        {
            lcl_InsertGraphic( aGraphic, aFileName, aFilterName, bAsLink, sal_True, pViewSh, pWindow, pView );
        }
    }
    else
    {
        SvxOpenGraphicDialog aDlg(ScResId(STR_INSERTGRAPHIC));

        if( aDlg.Execute() == GRFILTER_OK )
        {
            Graphic aGraphic;
            int nError = aDlg.GetGraphic(aGraphic);
            if( nError == GRFILTER_OK )
            {
                String aFileName = aDlg.GetPath();
                String aFilterName = aDlg.GetCurrentFilter();
                sal_Bool bAsLink = aDlg.IsAsLink();

                // really store as link only?
                if( bAsLink && SvtMiscOptions().ShowLinkWarningDialog() )
                {
                    SvxLinkWarningDialog aWarnDlg(pWin,aFileName);
                    if( aWarnDlg.Execute() != RET_OK )
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

/*************************************************************************
|*
|* FuInsertGraphic::Destruktor
|*
\************************************************************************/

FuInsertGraphic::~FuInsertGraphic()
{
}

/*************************************************************************
|*
|* FuInsertGraphic::Function aktivieren
|*
\************************************************************************/

void FuInsertGraphic::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* FuInsertGraphic::Function deaktivieren
|*
\************************************************************************/

void FuInsertGraphic::Deactivate()
{
    FuPoor::Deactivate();
}

/*************************************************************************
|*
|* FuInsertMedia::Konstruktor
|*
\************************************************************************/

FuInsertMedia::FuInsertMedia( ScTabViewShell*   pViewSh,
                              Window*           pWin,
                              ScDrawView*       pViewP,
                              SdrModel*         pDoc,
                              SfxRequest&       rReq ) :
    FuPoor(pViewSh, pWin, pViewP, pDoc, rReq)
{
    ::rtl::OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    bool                bAPI = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = PTR_CAST( SfxStringItem, &pReqArgs->Get( rReq.GetSlot() ) );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = aURL.getLength();
        }
    }

    bool bLink(true);
    if (bAPI ||
        ::avmedia::MediaWindow::executeMediaURLDialog(pWindow, aURL, & bLink))
    {
        Size aPrefSize;

        if( pWin )
            pWin->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, true, &aPrefSize ) )
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

/*************************************************************************
|*
|* FuInsertMedia::Destruktor
|*
\************************************************************************/

FuInsertMedia::~FuInsertMedia()
{
}

/*************************************************************************
|*
|* FuInsertMedia::Function aktivieren
|*
\************************************************************************/

void FuInsertMedia::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* FuInsertMedia::Function deaktivieren
|*
\************************************************************************/

void FuInsertMedia::Deactivate()
{
    FuPoor::Deactivate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
