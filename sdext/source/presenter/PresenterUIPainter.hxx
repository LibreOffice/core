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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERUIPAINTER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERUIPAINTER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>

namespace sdext::presenter
{
/** Functions for painting UI elements.
*/
class PresenterUIPainter
{
public:
    PresenterUIPainter() = delete;
    PresenterUIPainter(const PresenterUIPainter&) = delete;
    PresenterUIPainter& operator=(const PresenterUIPainter&) = delete;

    static void PaintHorizontalBitmapComposite(
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rRepaintBox, const css::awt::Rectangle& rBoundingBox,
        const css::uno::Reference<css::rendering::XBitmap>& rxLeftBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxRepeatableCenterBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxRightBitmap);

    static void PaintVerticalBitmapComposite(
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rRepaintBox, const css::awt::Rectangle& rBoundingBox,
        const css::uno::Reference<css::rendering::XBitmap>& rxTopBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxRepeatableCenterBitmap,
        const css::uno::Reference<css::rendering::XBitmap>& rxBottomBitmap);
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
