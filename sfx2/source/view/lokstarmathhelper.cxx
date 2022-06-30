/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sfx2/ipclient.hxx>
#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/lokhelper.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/fract.hxx>
#include <vcl/layout.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/embed/XEmbeddedObject.hpp>
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

void LokStarMathHelper::Dispatch(const OUString& cmd,
                                 const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    if (const css::uno::Reference<css::frame::XController>& xController = GetXController())
        if (const css::uno::Reference<css::frame::XFrame> xFrame = xController->getFrame())
            comphelper::dispatchCommand(cmd, xFrame, rArguments);
}

namespace
{
// Find a child SmGraphicWindow*
vcl::Window* FindSmGraphicWindow(vcl::Window* pWin)
{
    if (!pWin)
        return nullptr;

    if (pWin->IsStarMath())
        return pWin;

    pWin = pWin->GetWindow(GetWindowType::FirstChild);
    while (pWin)
    {
        if (vcl::Window* pSmGraphicWindow = FindSmGraphicWindow(pWin))
            return pSmGraphicWindow;
        pWin = pWin->GetWindow(GetWindowType::Next);
    }
    return nullptr;
}

// Find a child window that corresponds to SmGraphicWidget
vcl::Window* FindChildSmGraphicWidgetWindow(vcl::Window* pWin)
{
    if (!pWin)
        return nullptr;

    // The needed window is a VclDrawingArea
    if (dynamic_cast<VclDrawingArea*>(pWin))
        return pWin;

    pWin = pWin->GetWindow(GetWindowType::FirstChild);
    while (pWin)
    {
        if (vcl::Window* pSmGraphicWidgetWindow = FindChildSmGraphicWidgetWindow(pWin))
            return pSmGraphicWidgetWindow;
        pWin = pWin->GetWindow(GetWindowType::Next);
    }
    return nullptr;
}

vcl::Window* FindSmGraphicWidgetWindow(vcl::Window* pWin)
{
    return FindChildSmGraphicWidgetWindow(FindSmGraphicWindow(pWin));
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
                mpWindow.set(FindSmGraphicWidgetWindow(VCLUnoHelper::GetWindow(xDockerWin)));
            }
        }
    }

    return mpWindow.get();
}

tools::Rectangle LokStarMathHelper::GetBoundingBox()
{
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
                    const MapMode& aMapMode = pWindow->GetMapMode();
                    const auto & [ m, d ]
                        = o3tl::getConversionMulDiv(o3tl::Length::px, o3tl::Length::twip);
                    const Fraction& scaleX = aMapMode.GetScaleX();
                    const Fraction& scaleY = aMapMode.GetScaleY();
                    const auto nXNum = m * scaleX.GetDenominator();
                    const auto nXDen = d * scaleX.GetNumerator();
                    const auto nYNum = m * scaleY.GetDenominator();
                    const auto nYDen = d * scaleY.GetNumerator();

                    Point aOffset
                        = pWindow->GetOffsetPixelFrom(*pRootWin).scale(nXNum, nXDen, nYNum, nYDen);
                    Size aSize = pWindow->GetSizePixel().scale(nXNum, nXDen, nYNum, nYDen);
                    return { aOffset, aSize };
                }
            }
        }
    }
    return {};
}

bool LokStarMathHelper::postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons,
                                       int nModifier, double fScaleX, double fScaleY)
{
    if (vcl::Window* pWindow = GetWindow())
    {
        Point aMousePos(nX, nY);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
