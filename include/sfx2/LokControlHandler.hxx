/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/lokhelper.hxx>
#include <svx/svdouno.hxx>
#include <svx/svditer.hxx>
#include <vcl/virdev.hxx>
#include <vcl/DocWindow.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/UnitConversion.hxx>

#include <optional>

class LokControlHandler
{
public:
    static bool postMouseEvent(const SdrPage* pPage, const SdrView* pDrawView,
                               vcl::DocWindow& rMainWindow, int nType, Point aPointHmm, int nCount,
                               int nButtons, int nModifier)
    {
        static std::optional<PointerStyle> eDocPointerStyle;

        o3tl::Length eControlUnitLength = MapToO3tlLength(rMainWindow.GetMapMode().GetMapUnit());
        SdrObjListIter aIterator(pPage, SdrIterMode::Flat);
        while (aIterator.IsMore())
        {
            SdrObject* pObject = aIterator.Next();
            SdrUnoObj* pUnoObect = dynamic_cast<SdrUnoObj*>(pObject);
            if (pUnoObect)
            {
                tools::Rectangle aControlRect = pUnoObect->GetLogicRect();
                tools::Rectangle aControlRectHMM
                    = o3tl::convert(aControlRect, eControlUnitLength, o3tl::Length::mm100);

                if (aControlRectHMM.Contains(aPointHmm))
                {
                    css::uno::Reference<css::awt::XControl> xControl
                        = pUnoObect->GetUnoControl(*pDrawView, *rMainWindow.GetOutDev());
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
                        tools::Rectangle aControlRectPx
                            = o3tl::convert(aControlRectHMM, o3tl::Length::mm100, o3tl::Length::px);
                        // used by Control::LogicInvalidate
                        pWindow->SetPosPixel(aControlRectPx.TopLeft());

                        // when entering into control area save current pointer style
                        // and set pointer style to arrow
                        if (!eDocPointerStyle)
                        {
                            eDocPointerStyle = rMainWindow.GetPointer();
                            rMainWindow.SetPointer(pWindow->GetPointer());
                        }

                        Point aControlRelativePositionHMM = aPointHmm - aControlRectHMM.TopLeft();
                        Point aControlRelativePosition = o3tl::convert(
                            aControlRelativePositionHMM, o3tl::Length::mm100, o3tl::Length::px);

                        LokMouseEventData aMouseEventData(nType, aControlRelativePosition, nCount,
                                                          MouseEventModifiers::SIMPLECLICK,
                                                          nButtons, nModifier);
                        SfxLokHelper::postMouseEventAsync(pWindow, aMouseEventData);
                        return true;
                    }
                }
            }
        }

        // when exiting from control area restore document pointer style
        if (eDocPointerStyle)
        {
            rMainWindow.SetPointer(*eDocPointerStyle);
            eDocPointerStyle.reset();
        }
        return false;
    }

    static void drawUnoControl(const SdrView* pDrawView, const SdrUnoObj* pUnoObect,
                               vcl::Window const& rMainWindow, VirtualDevice& rDevice,
                               tools::Rectangle const& rTileRectHMM, double scaleX, double scaleY)
    {
        css::uno::Reference<css::awt::XControl> xControl
            = pUnoObect->GetUnoControl(*pDrawView, *rMainWindow.GetOutDev());
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

        o3tl::Length eControlUnitLength = MapToO3tlLength(rMainWindow.GetMapMode().GetMapUnit());
        tools::Rectangle aControlRect = pUnoObect->GetLogicRect();
        tools::Rectangle aObjectRectHMM
            = o3tl::convert(aControlRect, eControlUnitLength, o3tl::Length::mm100);
        tools::Rectangle aControltRectPx
            = o3tl::convert(aObjectRectHMM, o3tl::Length::mm100, o3tl::Length::px);

        Point aOffsetFromTile(aObjectRectHMM.Left() - rTileRectHMM.Left(),
                              aObjectRectHMM.Top() - rTileRectHMM.Top());
        tools::Rectangle aRectangleHMM(aOffsetFromTile, aObjectRectHMM.GetSize());
        tools::Rectangle aRectanglePx
            = o3tl::convert(aRectangleHMM, o3tl::Length::mm100, o3tl::Length::px);

        xControlWindow->setPosSize(aControltRectPx.Left(), aControltRectPx.Top(),
                                   aRectanglePx.GetWidth(), aRectanglePx.GetHeight(),
                                   css::awt::PosSize::POSSIZE);

        xControlView->setGraphics(xGraphics);
        // required for getting text label rendered with the correct scale
        xControlView->setZoom(1, 1);

        xControlView->draw(aRectanglePx.Left() * scaleX, aRectanglePx.Top() * scaleY);
    }

    static void paintControlTile(const SdrPage* pPage, const SdrView* pDrawView,
                                 vcl::Window const& rMainWindow, VirtualDevice& rDevice,
                                 Size aOutputSize, tools::Rectangle const& rTileRect)
    {
        tools::Rectangle aTileRectHMM
            = o3tl::convert(rTileRect, o3tl::Length::twip, o3tl::Length::mm100);

        // Resizes the virtual device so to contain the entries context
        rDevice.SetOutputSizePixel(aOutputSize, /*bErase*/ false);

        rDevice.Push(vcl::PushFlags::MAPMODE);
        MapMode aDeviceMapMode(rDevice.GetMapMode());

        const Fraction scale = conversionFract(o3tl::Length::px, o3tl::Length::twip);
        Fraction scaleX = Fraction(aOutputSize.Width(), rTileRect.GetWidth()) * scale;
        Fraction scaleY = Fraction(aOutputSize.Height(), rTileRect.GetHeight()) * scale;
        aDeviceMapMode.SetScaleX(scaleX);
        aDeviceMapMode.SetScaleY(scaleY);
        aDeviceMapMode.SetMapUnit(MapUnit::MapPixel);
        rDevice.SetMapMode(aDeviceMapMode);

        o3tl::Length eControlUnitLength = MapToO3tlLength(rMainWindow.GetMapMode().GetMapUnit());
        SdrObjListIter aIterator(pPage, SdrIterMode::Flat);

        while (aIterator.IsMore())
        {
            SdrObject* pObject = aIterator.Next();
            SdrUnoObj* pUnoObect = dynamic_cast<SdrUnoObj*>(pObject);
            if (pUnoObect)
            {
                tools::Rectangle aControlRect = pUnoObect->GetLogicRect();
                tools::Rectangle aObjectRectHMM
                    = o3tl::convert(aControlRect, eControlUnitLength, o3tl::Length::mm100);

                // Check if we intersect with the tile rectangle and we
                // need to draw the control.
                if (aObjectRectHMM.Overlaps(aTileRectHMM))
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
