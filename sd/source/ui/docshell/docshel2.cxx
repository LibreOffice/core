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

#include <memory>
#include <DrawDocShell.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxdlg.hxx>

#include <helpids.h>
#include <ViewShell.hxx>
#include <FrameView.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <ClientView.hxx>
#include <Window.hxx>
#include <strings.hrc>

#include <sdresid.hxx>
#include <fupoor.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/character.hxx>

namespace sd {

/**
 * Drawing of DocShell (with the helper class SdDrawViewShell)
 */
void DrawDocShell::Draw(OutputDevice* pOut, const JobSetup&, sal_uInt16 nAspect)
{
    if (nAspect == ASPECT_THUMBNAIL)
    {
      // THUMBNAIL: here we may can set the draft mode
    }

    std::unique_ptr<ClientView> pView( new ClientView(this, pOut) );

    pView->SetHlplVisible(false);
    pView->SetGridVisible(false);
    pView->SetBordVisible(false);
    pView->SetPageVisible(false);
    pView->SetGlueVisible(false);

    SdPage* pSelectedPage = nullptr;

    const std::vector<std::unique_ptr<sd::FrameView>> &rViews = mpDoc->GetFrameViewList();
    if( !rViews.empty() )
    {
        sd::FrameView* pFrameView = rViews[0].get();
        if( pFrameView->GetPageKind() == PageKind::Standard )
        {
            sal_uInt16 nSelectedPage = pFrameView->GetSelectedPage();
            pSelectedPage = mpDoc->GetSdPage(nSelectedPage, PageKind::Standard);
        }
    }

    if( nullptr == pSelectedPage )
    {
        SdPage* pPage = nullptr;
        sal_uInt16 nPageCnt = mpDoc->GetSdPageCount(PageKind::Standard);

        for (sal_uInt16 i = 0; i < nPageCnt; i++)
        {
            pPage = mpDoc->GetSdPage(i, PageKind::Standard);

            if ( pPage->IsSelected() )
                pSelectedPage = pPage;
        }

        if( nullptr == pSelectedPage )
            pSelectedPage = mpDoc->GetSdPage(0, PageKind::Standard);
    }

    ::tools::Rectangle aVisArea = GetVisArea(nAspect);
    pOut->IntersectClipRegion(aVisArea);
    pView->ShowSdrPage(pSelectedPage);

    if (pOut->GetOutDevType() == OUTDEV_WINDOW)
        return;

    MapMode aOldMapMode = pOut->GetMapMode();

    if (pOut->GetOutDevType() == OUTDEV_PRINTER)
    {
        MapMode aMapMode = aOldMapMode;
        Point aOrigin = aMapMode.GetOrigin();
        aOrigin.AdjustX(1 );
        aOrigin.AdjustY(1 );
        aMapMode.SetOrigin(aOrigin);
        pOut->SetMapMode(aMapMode);
    }

    vcl::Region aRegion(aVisArea);
    pView->CompleteRedraw(pOut, aRegion);

    if (pOut->GetOutDevType() == OUTDEV_PRINTER)
    {
        pOut->SetMapMode(aOldMapMode);
    }
}

::tools::Rectangle DrawDocShell::GetVisArea(sal_uInt16 nAspect) const
{
    ::tools::Rectangle aVisArea;

    if( ( ASPECT_THUMBNAIL == nAspect ) || ( ASPECT_DOCPRINT == nAspect ) )
    {
        // provide size of first page
        MapMode aSrcMapMode(MapUnit::MapPixel);
        MapMode aDstMapMode(MapUnit::Map100thMM);
        Size aSize = mpDoc->GetSdPage(0, PageKind::Standard)->GetSize();
        aSrcMapMode.SetMapUnit(MapUnit::Map100thMM);

        aSize = Application::GetDefaultDevice()->LogicToLogic(aSize, &aSrcMapMode, &aDstMapMode);
        aVisArea.SetSize(aSize);
    }
    else
    {
        aVisArea = SfxObjectShell::GetVisArea(nAspect);
    }

    if (aVisArea.IsEmpty() && mpViewShell)
    {
        vcl::Window* pWin = mpViewShell->GetActiveWindow();

        if (pWin)
        {
            aVisArea = pWin->PixelToLogic(::tools::Rectangle(Point(0,0), pWin->GetOutputSizePixel()));
        }
    }

    return aVisArea;
}

void DrawDocShell::Connect(ViewShell* pViewSh)
{
    mpViewShell = pViewSh;
}

void DrawDocShell::Disconnect(ViewShell const * pViewSh)
{
    if (mpViewShell == pViewSh)
    {
        mpViewShell = nullptr;
    }
}

FrameView* DrawDocShell::GetFrameView()
{
    FrameView* pFrameView = nullptr;

    if (mpViewShell)
    {
        pFrameView = mpViewShell->GetFrameView();
    }

    return pFrameView;
}

/**
 * Creates a bitmap of an arbitrary page
 */
BitmapEx DrawDocShell::GetPagePreviewBitmap(SdPage* pPage)
{
    const sal_uInt16 nMaxEdgePixel = 90;
    MapMode         aMapMode( MapUnit::Map100thMM );
    const Size      aSize( pPage->GetSize() );
    const Point     aNullPt;
    ScopedVclPtrInstance< VirtualDevice > pVDev( *Application::GetDefaultDevice() );

    pVDev->SetMapMode( aMapMode );

    const Size  aPixSize( pVDev->LogicToPixel( aSize ) );
    const sal_uLong nMaxEdgePix = std::max( aPixSize.Width(), aPixSize.Height() );
    Fraction    aFrac( nMaxEdgePixel, nMaxEdgePix );

    aMapMode.SetScaleX( aFrac );
    aMapMode.SetScaleY( aFrac );
    pVDev->SetMapMode( aMapMode );
    pVDev->SetOutputSize( aSize );

    // that we also get the dark lines at the right and bottom page margin
    aFrac = Fraction( nMaxEdgePixel - 1, nMaxEdgePix );
    aMapMode.SetScaleX( aFrac );
    aMapMode.SetScaleY( aFrac );
    pVDev->SetMapMode( aMapMode );

    std::unique_ptr<ClientView> pView(new ClientView( this, pVDev ));
    FrameView*      pFrameView = GetFrameView();
    pView->ShowSdrPage( pPage );

    if ( GetFrameView() )
    {
        // initialize the drawing-(screen) attributes
        pView->SetGridCoarse( pFrameView->GetGridCoarse() );
        pView->SetGridFine( pFrameView->GetGridFine() );
        pView->SetSnapGridWidth(pFrameView->GetSnapGridWidthX(), pFrameView->GetSnapGridWidthY());
        pView->SetGridVisible( pFrameView->IsGridVisible() );
        pView->SetGridFront( pFrameView->IsGridFront() );
        pView->SetSnapAngle( pFrameView->GetSnapAngle() );
        pView->SetGridSnap( pFrameView->IsGridSnap() );
        pView->SetBordSnap( pFrameView->IsBordSnap() );
        pView->SetHlplSnap( pFrameView->IsHlplSnap() );
        pView->SetOFrmSnap( pFrameView->IsOFrmSnap() );
        pView->SetOPntSnap( pFrameView->IsOPntSnap() );
        pView->SetOConSnap( pFrameView->IsOConSnap() );
        pView->SetDragStripes( pFrameView->IsDragStripes() );
        pView->SetFrameDragSingles( pFrameView->IsFrameDragSingles() );
        pView->SetSnapMagneticPixel( pFrameView->GetSnapMagneticPixel() );
        pView->SetMarkedHitMovesAlways( pFrameView->IsMarkedHitMovesAlways() );
        pView->SetMoveOnlyDragging( pFrameView->IsMoveOnlyDragging() );
        pView->SetSlantButShear( pFrameView->IsSlantButShear() );
        pView->SetNoDragXorPolys( pFrameView->IsNoDragXorPolys() );
        pView->SetCrookNoContortion( pFrameView->IsCrookNoContortion() );
        pView->SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
        pView->SetBigOrtho( pFrameView->IsBigOrtho() );
        pView->SetOrtho( pFrameView->IsOrtho() );

        SdrPageView* pPageView = pView->GetSdrPageView();

        if (pPageView)
        {
            if ( pPageView->GetVisibleLayers() != pFrameView->GetVisibleLayers() )
                pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );

            if ( pPageView->GetPrintableLayers() != pFrameView->GetPrintableLayers() )
                pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

            if ( pPageView->GetLockedLayers() != pFrameView->GetLockedLayers() )
                pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );

            pPageView->SetHelpLines( pFrameView->GetStandardHelpLines() );
        }

        if ( pView->GetActiveLayer() != pFrameView->GetActiveLayer() )
            pView->SetActiveLayer( pFrameView->GetActiveLayer() );
    }

    pView->CompleteRedraw( pVDev, vcl::Region(::tools::Rectangle(aNullPt, aSize)) );

    // IsRedrawReady() always gives sal_True while ( !pView->IsRedrawReady() ) {}
    pView.reset();

    pVDev->SetMapMode( MapMode() );

    BitmapEx aPreview( pVDev->GetBitmapEx( aNullPt, pVDev->GetOutputSizePixel() ) );

    DBG_ASSERT(!!aPreview, "Preview-Bitmap could not be generated");

    return aPreview;
}

/**
 * Checks if the page exists. If so, we force the user to enter a not yet used
 * name.
 * @return sal_False if the user cancels the action.
 */
bool DrawDocShell::CheckPageName(weld::Window* pWin, OUString& rName)
{
    const OUString aStrForDlg( rName );
    bool bIsNameValid = IsNewPageNameValid( rName, true );

    if( ! bIsNameValid )
    {
        OUString aDesc( SdResId( STR_WARN_PAGE_EXISTS ) );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

        ScopedVclPtr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog(pWin, aStrForDlg, aDesc));
        aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

        aNameDlg->SetCheckNameHdl( LINK( this, DrawDocShell, RenameSlideHdl ) );

        rtl::Reference<FuPoor> xFunc( mpViewShell->GetCurrentFunction() );
        if( xFunc.is() )
            xFunc->cancel();

        if( aNameDlg->Execute() == RET_OK )
        {
            aNameDlg->GetName( rName );
            bIsNameValid = IsNewPageNameValid( rName );
        }
    }

    return bIsNameValid;
}

bool DrawDocShell::IsNewPageNameValid( OUString & rInOutPageName, bool bResetStringIfStandardName /* = false */ )
{
    bool bCanUseNewName = false;

    // check if name is something like 'Slide n'
    OUString aStrPage(SdResId(STR_SD_PAGE) + " ");

    bool bIsStandardName = false;

    // prevent also _future_ slide names of the form "'STR_SD_PAGE' + ' ' + '[0-9]+|[a-z]|[A-Z]|[CDILMVX]+|[cdilmvx]+'"
    // (arabic, lower- and upper case single letter, lower- and upper case roman numbers)
    if (rInOutPageName.startsWith(aStrPage) &&
        rInOutPageName.getLength() > aStrPage.getLength())
    {
        sal_Int32 nIdx{ aStrPage.getLength() };
        OUString sRemainder = rInOutPageName.getToken(0, ' ', nIdx);
        if (sRemainder[0] >= '0' && sRemainder[0] <= '9')
        {
            // check for arabic numbering

            sal_Int32 nIndex = 1;
            // skip all following numbers
            while (nIndex < sRemainder.getLength() &&
                   sRemainder[nIndex] >= '0' && sRemainder[nIndex] <= '9')
            {
                nIndex++;
            }

            // EOL? Reserved name!
            if (nIndex >= sRemainder.getLength())
            {
                bIsStandardName = true;
            }
        }
        else if (sRemainder.getLength() == 1 &&
                 rtl::isAsciiLowerCase(sRemainder[0]))
        {
            // lower case, single character: reserved
            bIsStandardName = true;
        }
        else if (sRemainder.getLength() == 1 &&
                 rtl::isAsciiUpperCase(sRemainder[0]))
        {
            // upper case, single character: reserved
            bIsStandardName = true;
        }
        else
        {
            // check for upper/lower case roman numbering
            OUString sReserved("cdilmvx");

            // skip all following characters contained in one reserved class
            if (sReserved.indexOf(sRemainder[0]) == -1)
                sReserved = sReserved.toAsciiUpperCase();

            sal_Int32 nIndex = 0;
            while (nIndex < sRemainder.getLength() &&
                   sReserved.indexOf(sRemainder[nIndex]) != -1)
            {
                nIndex++;
            }

            // EOL? Reserved name!
            if (nIndex >= sRemainder.getLength())
            {
                bIsStandardName = true;
            }
        }
    }

    if( bIsStandardName )
    {
        if( bResetStringIfStandardName )
        {
            // this is for insertion of slides from other files with standard
            // name.  They get a new standard name, if the string is set to an
            // empty one.
            rInOutPageName.clear();
            bCanUseNewName = true;
        }
        else
            bCanUseNewName = false;
    }
    else
    {
        if (!rInOutPageName.isEmpty())
        {
            bool   bOutDummy;
            sal_uInt16 nExistingPageNum = mpDoc->GetPageByName( rInOutPageName, bOutDummy );
            bCanUseNewName = ( nExistingPageNum == SDRPAGE_NOTFOUND );
        }
        else
            bCanUseNewName = false;
    }

    return bCanUseNewName;
}

bool DrawDocShell::IsPageNameUnique( const OUString & rPageName ) const
{
    return mpDoc->IsPageNameUnique(rPageName);
}

IMPL_LINK( DrawDocShell, RenameSlideHdl, AbstractSvxNameDialog&, rDialog, bool )
{
    OUString aNewName;
    rDialog.GetName( aNewName );
    return IsNewPageNameValid( aNewName );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
