/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_LOKCHARTHELPER_HXX
#define INCLUDED_SFX2_LOKCHARTHELPER_HXX

#include <sfx2/dllapi.h>
#include <tools/gen.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>

class SfxViewShell;
class VirtualDevice;

class SFX2_DLLPUBLIC LokChartHelper
{
private:
    SfxViewShell* mpViewShell;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::Reference<css::frame::XDispatch> mxDispatcher;
    VclPtr<vcl::Window> mpWindow;

public:
    LokChartHelper(SfxViewShell* pViewShell)
        : mpViewShell(pViewShell)
        , mpWindow(nullptr)
    {}

    css::uno::Reference<css::frame::XController>& GetXController();
    css::uno::Reference<css::frame::XDispatch>& GetXDispatcher();
    vcl::Window* GetWindow();
    tools::Rectangle GetChartBoundingBox();
    void Invalidate();

    bool Hit(const Point& aPos);
    static bool HitAny(const Point& aPos);
    void PaintTile(VirtualDevice& rRenderContext, const tools::Rectangle& rTileRect);
    static void PaintAllChartsOnTile(VirtualDevice& rDevice,
                                     int nOutputWidth, int nOutputHeight,
                                     int nTilePosX, int nTilePosY,
                                     long nTileWidth, long nTileHeight);
    bool postMouseEvent(int nType, int nX, int nY,
                        int nCount, int nButtons, int nModifier,
                        double fScaleX = 1.0, double fScaleY = 1.0);
    bool setTextSelection(int nType, int nX, int nY);
    bool setGraphicSelection(int nType, int nX, int nY,
                             double fScaleX = 1.0, double fScaleY = 1.0);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
