/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/lokcharthelper.hxx>

#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/ipclient.hxx>
#include <sfx2/lokhelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/fract.hxx>
#include <tools/mapunit.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

#define TWIPS_PER_PIXEL 15

using namespace com::sun::star;

namespace {

Point lcl_TwipsToHMM( const Point& rPoint )
{
    return Point(convertTwipToMm100(rPoint.getX()), convertTwipToMm100(rPoint.getY()));
}

Size lcl_TwipsToHMM( const Size& rSize )
{
    return Size(convertTwipToMm100(rSize.getWidth()), convertTwipToMm100(rSize.getHeight()));
}

} // end anonymous ns

css::uno::Reference<css::frame::XController>& LokChartHelper::GetXController()
{
    if(!mxController.is() && mpViewShell)
    {
        SfxInPlaceClient* pIPClient = mpViewShell->GetIPClient();
        if (pIPClient)
        {
            const css::uno::Reference< ::css::embed::XEmbeddedObject >& xEmbObj = pIPClient->GetObject();
            if( xEmbObj.is() )
            {
                ::css::uno::Reference< ::css::chart2::XChartDocument > xChart( xEmbObj->getComponent(), uno::UNO_QUERY );
                if( xChart.is() )
                {
                    ::css::uno::Reference< ::css::frame::XController > xChartController = xChart->getCurrentController();
                    if( xChartController.is() )
                    {
                        mxController = xChartController;
                    }
                }
            }
        }
    }

    return mxController;
}

css::uno::Reference<css::frame::XDispatch>& LokChartHelper::GetXDispatcher()
{
    if( !mxDispatcher.is() )
    {
        ::css::uno::Reference< ::css::frame::XController >& xChartController = GetXController();
        if( xChartController.is() )
        {
            ::css::uno::Reference< ::css::frame::XDispatch > xDispatcher( xChartController, uno::UNO_QUERY );
            if( xDispatcher.is() )
            {
                mxDispatcher = xDispatcher;
            }
        }
    }

    return mxDispatcher;
}

vcl::Window* LokChartHelper::GetWindow()
{
    if (!mpWindow)
    {
        ::css::uno::Reference< ::css::frame::XController >& xChartController = GetXController();
        if( xChartController.is() )
        {
            ::css::uno::Reference< ::css::frame::XFrame > xFrame = xChartController->getFrame();
            if (xFrame.is())
            {
                ::css::uno::Reference< ::css::awt::XWindow > xDockerWin = xFrame->getContainerWindow();
                vcl::Window* pParent = VCLUnoHelper::GetWindow( xDockerWin ).get();
                if (pParent)
                {
                    sal_uInt16 nTotChildren = pParent->GetChildCount();
                    while (nTotChildren--)
                    {
                        vcl::Window* pChildWin = pParent->GetChild(nTotChildren);
                        if (pChildWin && pChildWin->IsChart())
                        {
                            mpWindow = pChildWin;
                            break;
                        }
                    }
                }
            }
        }
    }

    return mpWindow.get();
}

tools::Rectangle LokChartHelper::GetChartBoundingBox()
{
    tools::Rectangle aBBox;
    if (mpViewShell)
    {
        SfxInPlaceClient* pIPClient = mpViewShell->GetIPClient();
        if (pIPClient)
        {
            vcl::Window* pRootWin = pIPClient->GetEditWin();
            if (pRootWin)
            {
                vcl::Window* pWindow = GetWindow();
                if (pWindow)
                {
                    // In all cases, the following code fragment
                    // returns the chart bounding box in twips.
                    const MapMode& aCWMapMode = pWindow->GetMapMode();
                    double fXScale( aCWMapMode.GetScaleX() );
                    double fYScale( aCWMapMode.GetScaleY() );
                    Point aOffset = pWindow->GetOffsetPixelFrom(*pRootWin);
                    aOffset.setX( aOffset.X() * (TWIPS_PER_PIXEL / fXScale) );
                    aOffset.setY( aOffset.Y() * (TWIPS_PER_PIXEL / fYScale) );
                    Size aSize = pWindow->GetSizePixel();
                    aSize.setWidth( aSize.Width() * (TWIPS_PER_PIXEL / fXScale) );
                    aSize.setHeight( aSize.Height() * (TWIPS_PER_PIXEL / fYScale) );
                    aBBox = tools::Rectangle(aOffset, aSize);
                }
            }
        }
    }
    return aBBox;
}

void LokChartHelper::Invalidate()
{
    mpWindow = nullptr;
    mxDispatcher.clear();
    mxController.clear();
}

bool LokChartHelper::Hit(const Point& aPos)
{
    if (mpViewShell)
    {
        vcl::Window* pChartWindow = GetWindow();
        if (pChartWindow)
        {
            tools::Rectangle rChartBBox = GetChartBoundingBox();
            return rChartBBox.IsInside(aPos);
        }
    }
    return false;
}

bool LokChartHelper::HitAny(const Point& aPos)
{
    SfxViewShell* pCurView = SfxViewShell::Current();
    int nPartForCurView = pCurView ? pCurView->getPart() : -1;
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->getPart() == nPartForCurView)
        {
            LokChartHelper aChartHelper(pViewShell);
            if (aChartHelper.Hit(aPos))
                return true;
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
    return false;
}

void LokChartHelper::PaintTile(VirtualDevice& rRenderContext, const tools::Rectangle& rTileRect)
{
    if (!mpViewShell)
        return;

    vcl::Window* pChartWindow = GetWindow();
    if (!pChartWindow)
        return;

    tools::Rectangle aChartRect = GetChartBoundingBox();
    tools::Rectangle aTestRect = rTileRect;
    aTestRect.Intersection( aChartRect );
    if (aTestRect.IsEmpty())
        return;

    Point aOffset( aChartRect.Left() - rTileRect.Left(), aChartRect.Top() - rTileRect.Top() );
    Point aOffsetFromTile = lcl_TwipsToHMM(aOffset);
    Size aSize = lcl_TwipsToHMM(aChartRect.GetSize());
    tools::Rectangle aRectangle(Point(0,0), aSize);

    bool bEnableMapMode = !pChartWindow->IsMapModeEnabled();
    pChartWindow->EnableMapMode();
    bool bRenderContextEnableMapMode = !rRenderContext.IsMapModeEnabled();
    rRenderContext.EnableMapMode();

    rRenderContext.Push(PushFlags::MAPMODE);

    MapMode aCWMapMode = pChartWindow->GetMapMode();
    aCWMapMode.SetScaleX(rRenderContext.GetMapMode().GetScaleX());
    aCWMapMode.SetScaleY(rRenderContext.GetMapMode().GetScaleY());

    aCWMapMode.SetOrigin(aOffsetFromTile);
    rRenderContext.SetMapMode(aCWMapMode);

    pChartWindow->Paint(rRenderContext, aRectangle);

    rRenderContext.Pop();

    if (bRenderContextEnableMapMode)
        rRenderContext.EnableMapMode(false);
    if (bEnableMapMode)
        pChartWindow->EnableMapMode(false);
}

void LokChartHelper::PaintAllChartsOnTile(VirtualDevice& rDevice,
                                          int nOutputWidth, int nOutputHeight,
                                          int nTilePosX, int nTilePosY,
                                          long nTileWidth, long nTileHeight)
{
    if (comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    // Resizes the virtual device so to contain the entries context
    rDevice.SetOutputSizePixel(Size(nOutputWidth, nOutputHeight));

    rDevice.Push(PushFlags::MAPMODE);
    MapMode aMapMode(rDevice.GetMapMode());

    // Scaling. Must convert from pixels to twips. We know
    // that VirtualDevices use a DPI of 96.
    Fraction scaleX = Fraction(nOutputWidth, 96) * Fraction(1440) / Fraction(nTileWidth);
    Fraction scaleY = Fraction(nOutputHeight, 96) * Fraction(1440) / Fraction(nTileHeight);
    aMapMode.SetScaleX(scaleX);
    aMapMode.SetScaleY(scaleY);
    rDevice.SetMapMode(aMapMode);

    SfxViewShell* pCurView = SfxViewShell::Current();
    int nPartForCurView = pCurView ? pCurView->getPart() : -1;
    tools::Rectangle aTileRect(Point(nTilePosX, nTilePosY), Size(nTileWidth, nTileHeight));
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->getPart() == nPartForCurView)
        {
            LokChartHelper aChartHelper(pViewShell);
            aChartHelper.PaintTile(rDevice, aTileRect);
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
    rDevice.Pop();
}

bool LokChartHelper::postMouseEvent(int nType, int nX, int nY,
                                    int nCount, int nButtons, int nModifier,
                                    double fScaleX, double fScaleY)
{
    Point aMousePos(nX, nY);
    vcl::Window* pChartWindow = GetWindow();
    if (pChartWindow)
    {
        tools::Rectangle rChartBBox = GetChartBoundingBox();
        if (rChartBBox.IsInside(aMousePos))
        {
            int nChartWinX = nX - rChartBBox.Left();
            int nChartWinY = nY - rChartBBox.Top();

            // chart window expects pixels, but the conversion factor
            // can depend on the client zoom
            Point aPos(nChartWinX * fScaleX, nChartWinY * fScaleY);
            SfxLokHelper::postMouseEventAsync(pChartWindow, nType, aPos, nCount,
                                              MouseEventModifiers::SIMPLECLICK,
                                              nButtons, nModifier);

            return true;
        }
    }
    return false;
}

bool LokChartHelper::setTextSelection(int nType, int nX, int nY)
{
    tools::Rectangle rChartBBox = GetChartBoundingBox();
    if (rChartBBox.IsInside(Point(nX, nY)))
    {
        css::uno::Reference<css::frame::XDispatch> xDispatcher = GetXDispatcher();
        if (xDispatcher.is())
        {
            int nChartWinX = nX - rChartBBox.Left();
            int nChartWinY = nY - rChartBBox.Top();

            // no scale here the chart controller expects twips
            // that are converted to hmm
            util::URL aURL;
            aURL.Path = "LOKSetTextSelection";
            uno::Sequence< beans::PropertyValue > aArgs(3);
            aArgs[0].Value <<= static_cast<sal_Int32>(nType);
            aArgs[1].Value <<= static_cast<sal_Int32>(nChartWinX);
            aArgs[2].Value <<= static_cast<sal_Int32>(nChartWinY);
            xDispatcher->dispatch(aURL, aArgs);
        }
        return true;
    }
    return false;
}

bool LokChartHelper::setGraphicSelection(int nType, int nX, int nY,
                                         double fScaleX, double fScaleY)
{
    tools::Rectangle rChartBBox = GetChartBoundingBox();
    if (rChartBBox.IsInside(Point(nX, nY)))
    {
        int nChartWinX = nX - rChartBBox.Left();
        int nChartWinY = nY - rChartBBox.Top();

        vcl::Window* pChartWindow = GetWindow();

        Point aPos(nChartWinX * fScaleX, nChartWinY * fScaleY);
        switch (nType)
        {
        case LOK_SETGRAPHICSELECTION_START:
            {
                MouseEvent aClickEvent(aPos, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
                pChartWindow->MouseButtonDown(aClickEvent);
                MouseEvent aMoveEvent(aPos, 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
                pChartWindow->MouseMove(aMoveEvent);
            }
            break;
        case LOK_SETGRAPHICSELECTION_END:
            {
                MouseEvent aMoveEvent(aPos, 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
                pChartWindow->MouseMove(aMoveEvent);
                MouseEvent aClickEvent(aPos, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
                pChartWindow->MouseButtonUp(aClickEvent);
            }
            break;
        default:
            assert(false);
            break;
        }
        return true;
    }
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
