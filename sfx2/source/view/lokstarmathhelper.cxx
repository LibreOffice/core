/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/lokcomponenthelpers.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/ipclient.hxx>
#include <sfx2/lokhelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/fract.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

css::uno::Reference<css::frame::XController>& LokStarMathHelper::GetXController()
{
    if (!mxController && mpViewShell)
    {
        if (const SfxInPlaceClient* pIPClient = mpViewShell->GetIPClient())
        {
            if (const auto& xEmbObj = pIPClient->GetObject())
            {
                css::uno::Reference<css::lang::XServiceInfo> xComp(xEmbObj->getComponent(),
                                                                   css::uno::UNO_QUERY);
                if (xComp && xComp->supportsService("com.sun.star.formula.FormulaProperties"))
                    if (css::uno::Reference<css::frame::XModel> xModel{ xComp,
                                                                        css::uno::UNO_QUERY })
                        mxController = xModel->getCurrentController();
            }
        }
    }

    return mxController;
}

css::uno::Reference<css::frame::XDispatch>& LokStarMathHelper::GetXDispatcher()
{
    if (!mxDispatcher)
        mxDispatcher.set(GetXController(), css::uno::UNO_QUERY);

    return mxDispatcher;
}

namespace
{
vcl::Window* FindMathWindow(vcl::Window* pWin)
{
    if (!pWin)
        return nullptr;

    if (pWin->IsStarMath())
        return pWin;

    pWin = pWin->GetWindow(GetWindowType::FirstChild);
    while (pWin)
    {
        if (vcl::Window* pMathWindow = FindMathWindow(pWin))
            return pMathWindow;
        pWin = pWin->GetWindow(GetWindowType::Next);
    }
    return nullptr;
}
}

vcl::Window* LokStarMathHelper::GetWindow()
{
    if (!mpWindow)
    {
        if (const css::uno::Reference<css::frame::XController>& xController = GetXController())
        {
            if (const css::uno::Reference<css::frame::XFrame> xFrame = xController->getFrame())
            {
                css::uno::Reference<css::awt::XWindow> xDockerWin = xFrame->getContainerWindow();
                mpWindow.set(FindMathWindow(VCLUnoHelper::GetWindow(xDockerWin)));
            }
        }
    }

    return mpWindow.get();
}

tools::Rectangle LokStarMathHelper::GetBoundingBox()
{
    tools::Rectangle aBBox;
    if (mpViewShell)
    {
        if (SfxInPlaceClient* pIPClient = mpViewShell->GetIPClient())
        {
            if (vcl::Window* pRootWin = pIPClient->GetEditWin())
            {
                if (vcl::Window* pWindow = GetWindow())
                {
                    // In all cases, the following code fragment
                    // returns the bounding box in twips.
                    const MapMode& aCWMapMode = pWindow->GetMapMode();
                    constexpr auto p
                        = o3tl::getConversionMulDiv(o3tl::Length::px, o3tl::Length::twip);
                    const auto& scaleX = aCWMapMode.GetScaleX();
                    const auto& scaleY = aCWMapMode.GetScaleY();
                    const auto nXNum = p.first * scaleX.GetDenominator();
                    const auto nXDen = p.second * scaleX.GetNumerator();
                    const auto nYNum = p.first * scaleY.GetDenominator();
                    const auto nYDen = p.second * scaleY.GetNumerator();

                    Point aOffset
                        = pWindow->GetOffsetPixelFrom(*pRootWin).scale(nXNum, nXDen, nYNum, nYDen);
                    Size aSize = pWindow->GetSizePixel().scale(nXNum, nXDen, nYNum, nYDen);
                    aBBox = tools::Rectangle(aOffset, aSize);
                }
            }
        }
    }
    return aBBox;
}

void LokStarMathHelper::Invalidate()
{
    mpWindow = nullptr;
    mxDispatcher.clear();
    mxController.clear();
}

bool LokStarMathHelper::Hit(const Point& aPos) { return GetBoundingBox().Contains(aPos); }

bool LokStarMathHelper::HitAny(const Point& aPos)
{
    SfxViewShell* pCurView = SfxViewShell::Current();
    int nPartForCurView = pCurView ? pCurView->getPart() : -1;
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pViewShell->GetDocId() == pCurView->GetDocId()
            && pViewShell->getPart() == nPartForCurView)
        {
            LokStarMathHelper aHelper(pViewShell);
            if (aHelper.Hit(aPos))
                return true;
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
    return false;
}

void LokStarMathHelper::PaintTile(VirtualDevice& rRenderContext, const tools::Rectangle& rTileRect)
{
    vcl::Window* pWindow = GetWindow();
    if (!pWindow)
        return;

    tools::Rectangle aRect = GetBoundingBox();
    tools::Rectangle aTestRect = rTileRect;
    aTestRect.Intersection(aRect);
    if (aTestRect.IsEmpty())
        return;

    Point aOffset(aRect.Left() - rTileRect.Left(), aRect.Top() - rTileRect.Top());
    Point aOffsetFromTile = convertTwipToMm100(aOffset);
    Size aSize = convertTwipToMm100(aRect.GetSize());
    tools::Rectangle aRectangle(Point(0, 0), aSize);

    bool bEnableMapMode = !pWindow->IsMapModeEnabled();
    pWindow->EnableMapMode();
    bool bRenderContextEnableMapMode = !rRenderContext.IsMapModeEnabled();
    rRenderContext.EnableMapMode();

    rRenderContext.Push(vcl::PushFlags::MAPMODE);

    MapMode aCWMapMode = pWindow->GetMapMode();
    aCWMapMode.SetScaleX(rRenderContext.GetMapMode().GetScaleX());
    aCWMapMode.SetScaleY(rRenderContext.GetMapMode().GetScaleY());

    aCWMapMode.SetOrigin(aOffsetFromTile);
    rRenderContext.SetMapMode(aCWMapMode);

    pWindow->Paint(rRenderContext, aRectangle);

    rRenderContext.Pop();

    if (bRenderContextEnableMapMode)
        rRenderContext.EnableMapMode(false);
    if (bEnableMapMode)
        pWindow->EnableMapMode(false);
}

void LokStarMathHelper::PaintAllStarMathsOnTile(VirtualDevice& rDevice, int nOutputWidth,
                                                int nOutputHeight, int nTilePosX, int nTilePosY,
                                                tools::Long nTileWidth, tools::Long nTileHeight)
{
    if (comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    // Resizes the virtual device so to contain the entries context
    rDevice.SetOutputSizePixel(Size(nOutputWidth, nOutputHeight));

    rDevice.Push(vcl::PushFlags::MAPMODE);
    MapMode aMapMode(rDevice.GetMapMode());

    // Scaling. Must convert from pixels to twips. We know
    // that VirtualDevices use a DPI of 96.
    const Fraction scale = conversionFract(o3tl::Length::px, o3tl::Length::twip);
    Fraction scaleX = Fraction(nOutputWidth, nTileWidth) * scale;
    Fraction scaleY = Fraction(nOutputHeight, nTileHeight) * scale;
    aMapMode.SetScaleX(scaleX);
    aMapMode.SetScaleY(scaleY);
    rDevice.SetMapMode(aMapMode);

    SfxViewShell* pCurView = SfxViewShell::Current();
    int nPartForCurView = pCurView ? pCurView->getPart() : -1;
    tools::Rectangle aTileRect(Point(nTilePosX, nTilePosY), Size(nTileWidth, nTileHeight));
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pCurView && pViewShell->GetDocId() == pCurView->GetDocId()
            && pViewShell->getPart() == nPartForCurView)
        {
            LokStarMathHelper aHelper(pViewShell);
            aHelper.PaintTile(rDevice, aTileRect);
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
    rDevice.Pop();
}

bool LokStarMathHelper::postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons,
                                       int nModifier, double fScaleX, double fScaleY)
{
    Point aMousePos(nX, nY);
    if (vcl::Window* pWindow = GetWindow())
    {
        tools::Rectangle rBBox = GetBoundingBox();
        if (rBBox.Contains(aMousePos))
        {
            int nWinX = nX - rBBox.Left();
            int nWinY = nY - rBBox.Top();

            // window expects pixels, but the conversion factor
            // can depend on the client zoom
            Point aPos(nWinX * fScaleX, nWinY * fScaleY);

            LokMouseEventData aMouseEventData(nType, aPos, nCount, MouseEventModifiers::SIMPLECLICK,
                                              nButtons, nModifier);
            SfxLokHelper::postMouseEventAsync(pWindow, aMouseEventData);

            return true;
        }
    }
    return false;
}

bool LokStarMathHelper::setTextSelection(int nType, int nX, int nY)
{
    tools::Rectangle rBBox = GetBoundingBox();
    if (rBBox.Contains(Point(nX, nY)))
    {
        if (auto xDispatcher{ GetXDispatcher() })
        {
            int nWinX = nX - rBBox.Left();
            int nWinY = nY - rBBox.Top();

            // no scale here the controller expects twips that are converted to hmm
            css::util::URL aURL;
            aURL.Path = "LOKSetTextSelection";
            css::uno::Sequence<css::beans::PropertyValue> aArgs{
                comphelper::makePropertyValue({}, static_cast<sal_Int32>(nType)), // Why no name?
                comphelper::makePropertyValue({}, static_cast<sal_Int32>(nWinX)),
                comphelper::makePropertyValue({}, static_cast<sal_Int32>(nWinY))
            };
            xDispatcher->dispatch(aURL, aArgs);
        }
        return true;
    }
    return false;
}

bool LokStarMathHelper::setGraphicSelection(int nType, int nX, int nY, double fScaleX,
                                            double fScaleY)
{
    tools::Rectangle rBBox = GetBoundingBox();
    if (rBBox.Contains(Point(nX, nY)))
    {
        int nWinX = nX - rBBox.Left();
        int nWinY = nY - rBBox.Top();

        vcl::Window* pWindow = GetWindow();

        Point aPos(nWinX * fScaleX, nWinY * fScaleY);
        switch (nType)
        {
            case LOK_SETGRAPHICSELECTION_START:
            {
                MouseEvent aClickEvent(aPos, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
                pWindow->MouseButtonDown(aClickEvent);
                MouseEvent aMoveEvent(aPos, 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
                pWindow->MouseMove(aMoveEvent);
            }
            break;
            case LOK_SETGRAPHICSELECTION_END:
            {
                MouseEvent aMoveEvent(aPos, 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
                pWindow->MouseMove(aMoveEvent);
                MouseEvent aClickEvent(aPos, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
                pWindow->MouseButtonUp(aClickEvent);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
