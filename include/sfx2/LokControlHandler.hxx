/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/dllapi.h>
#include <svx/svdouno.hxx>
#include <vcl/window.hxx>
#include <tools/UnitConversion.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <toolkit/helper/vclunohelper.hxx>

class LokControlHandler
{
public:
    static constexpr double convertMm100ToPixel(double fNumber) { return fNumber * 96.0 / 2540.0; }

    static Point convertMm100ToPixel(const Point& rPoint)
    {
        return Point(convertMm100ToPixel(rPoint.getX()), convertMm100ToPixel(rPoint.getY()));
    }

    static tools::Rectangle convertMm100ToPixel(const tools::Rectangle& rRectangle)
    {
        return tools::Rectangle(convertMm100ToPixel(rRectangle.TopLeft()),
                                Size(convertMm100ToPixel(rRectangle.GetWidth()),
                                     convertMm100ToPixel(rRectangle.GetHeight())));
    }

    static Point convertTwipToMm100(const Point& rPoint)
    {
        return Point(::convertTwipToMm100(sal_Int64(rPoint.getX())),
                     ::convertTwipToMm100(sal_Int64(rPoint.getY())));
    }

    static tools::Rectangle convertTwipToMm100(const tools::Rectangle& rRectangle)
    {
        return tools::Rectangle(convertTwipToMm100(rRectangle.TopLeft()),
                                Size(::convertTwipToMm100(rRectangle.GetWidth()),
                                     ::convertTwipToMm100(rRectangle.GetHeight())));
    }

    static bool postMouseEvent(SdrPage* pPage, SdrView* pDrawView, vcl::Window const& rMainWindow,
                               int nType, Point aPointHmm, int nCount, int nButtons, int nModifier)
    {
        SdrObjListIter aIterator(pPage, SdrIterMode::Flat);
        while (aIterator.IsMore())
        {
            SdrObject* pObject = aIterator.Next();
            SdrUnoObj* pUnoObect = dynamic_cast<SdrUnoObj*>(pObject);
            if (pUnoObect)
            {
                tools::Rectangle aControlRectHMM = pUnoObect->GetLogicRect();
                if (aControlRectHMM.IsInside(aPointHmm))
                {
                    css::uno::Reference<css::awt::XControl> xControl
                        = pUnoObect->GetUnoControl(*pDrawView, rMainWindow);
                    if (!xControl.is())
                        return false;

                    css::uno::Reference<css::awt::XWindow> xControlWindow(xControl,
                                                                          css::uno::UNO_QUERY);
                    if (!xControlWindow.is())
                        return false;

                    css::uno::Reference<css::awt::XWindowPeer> xWindowPeer(xControl->getPeer());

                    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWindowPeer);
                    if (pWindow)
                    {
                        Point aControlRelativePositionHMM = aPointHmm - aControlRectHMM.TopLeft();
                        Point aControlRelativePosition
                            = convertMm100ToPixel(aControlRelativePositionHMM);

                        LokMouseEventData aMouseEventData(nType, aControlRelativePosition, nCount,
                                                          MouseEventModifiers::SIMPLECLICK,
                                                          nButtons, nModifier);

                        SfxLokHelper::postMouseEventAsync(pWindow, aMouseEventData);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    static void drawUnoControl(SdrView* pDrawView, SdrUnoObj* pUnoObect,
                               vcl::Window const& rMainWindow, VirtualDevice& rDevice,
                               tools::Rectangle const& rTileRectHMM, double scaleX, double scaleY)
    {
        css::uno::Reference<css::awt::XControl> xControl
            = pUnoObect->GetUnoControl(*pDrawView, rMainWindow);
        if (!xControl.is())
            return;

        css::uno::Reference<css::awt::XWindow> xControlWindow(xControl, css::uno::UNO_QUERY);
        if (!xControlWindow.is())
            return;

        css::uno::Reference<css::awt::XGraphics> xGraphics(rDevice.CreateUnoGraphics());
        if (!xGraphics.is())
            return;

        css::uno::Reference<css::awt::XView> xControlView(xControl, css::uno::UNO_QUERY);
        if (!xControlView.is())
            return;

        tools::Rectangle aObjectRectHMM = pUnoObect->GetLogicRect();
        Point aOffsetFromTile = Point(aObjectRectHMM.Left() - rTileRectHMM.Left(),
                                      aObjectRectHMM.Top() - rTileRectHMM.Top());
        tools::Rectangle aRectangleHMM(aOffsetFromTile, aObjectRectHMM.GetSize());
        tools::Rectangle aRectanglePx = convertMm100ToPixel(aRectangleHMM);

        xControlWindow->setPosSize(0, 0, aRectanglePx.GetWidth(), aRectanglePx.GetHeight(),
                                   css::awt::PosSize::POSSIZE);

        xControlView->setGraphics(xGraphics);

        xControlView->draw(aRectanglePx.Left() * scaleX, aRectanglePx.Top() * scaleY);
    }

    static void paintControlTile(SdrPage* pPage, SdrView* pDrawView, vcl::Window const& rMainWindow,
                                 VirtualDevice& rDevice, Size aOutputSize,
                                 tools::Rectangle const& rTileRect)
    {
        tools::Rectangle aTileRectHMM = convertTwipToMm100(rTileRect);

        // Resizes the virtual device so to contain the entries context
        rDevice.SetOutputSizePixel(aOutputSize);

        rDevice.Push(PushFlags::MAPMODE);
        MapMode aDeviceMapMode(rDevice.GetMapMode());

        Fraction scaleX
            = Fraction(aOutputSize.Width(), 96) * Fraction(1440) / Fraction(rTileRect.GetWidth());
        Fraction scaleY
            = Fraction(aOutputSize.Height(), 96) * Fraction(1440) / Fraction(rTileRect.GetHeight());

        aDeviceMapMode.SetScaleX(scaleX);
        aDeviceMapMode.SetScaleY(scaleY);
        rDevice.SetMapMode(aDeviceMapMode);

        SdrObjListIter aIterator(pPage, SdrIterMode::Flat);

        while (aIterator.IsMore())
        {
            SdrObject* pObject = aIterator.Next();
            SdrUnoObj* pUnoObect = dynamic_cast<SdrUnoObj*>(pObject);
            if (pUnoObect)
            {
                tools::Rectangle aObjectRectHMM = pUnoObect->GetLogicRect();

                // Check if we intersect with the tile rectangle and we
                // need to draw the control.
                if (aObjectRectHMM.IsOver(aTileRectHMM))
                {
                    drawUnoControl(pDrawView, pUnoObect, rMainWindow, rDevice, aTileRectHMM,
                                   double(scaleX), double(scaleY));
                }
            }
        }

        rDevice.Pop();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
