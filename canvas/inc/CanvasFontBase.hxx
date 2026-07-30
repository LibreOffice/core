/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <canvas/canvastoolsdllapi.h>
#include <comphelper/compbase.hxx>

#include <com/sun/star/rendering/XCanvasFont.hpp>

class CANVASTOOLS_DLLPUBLIC CanvasFontBase
    : public comphelper::WeakComponentImplHelper<css::rendering::XCanvasFont>
{
public:
    virtual css::rendering::FontMetrics SAL_CALL getFontMetrics() override;
    virtual css::uno::Sequence<double> SAL_CALL getAvailableSizes() override;
    virtual css::uno::Sequence<css::beans::PropertyValue>
        SAL_CALL getExtraFontProperties() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
