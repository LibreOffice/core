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
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <com/sun/star/awt/Gradient.hpp>

using namespace css;

namespace sfx2 { namespace sidebar {

Paint::Paint()
    : meType(NoPaint)
{
}

Paint::Paint (const Color& rColor)
    : meType(ColorPaint),
      maValue(rColor)
{
}

Paint::Paint (const Gradient& rGradient)
    : meType(GradientPaint),
      maValue(rGradient)
{
}

Paint Paint::Create (const css::uno::Any& rValue)
{
    Color aColor (0);
    if (rValue >>= aColor)
        return Paint(aColor);

    awt::Gradient aAwtGradient;
    if (rValue >>= aAwtGradient)
        return Paint(Tools::AwtToVclGradient(aAwtGradient));

    return Paint();
}

const Color& Paint::GetColor() const
{
    if (meType != ColorPaint)
    {
        assert(meType==ColorPaint);
        static const Color aErrorColor;
        return aErrorColor;
    }
    else
        return ::boost::get<Color>(maValue);
}

const Gradient& Paint::GetGradient() const
{
    if (meType != GradientPaint)
    {
        assert(meType==GradientPaint);
        static Gradient aErrorGradient;
        return aErrorGradient;
    }
    else
        return ::boost::get<Gradient>(maValue);
}

Wallpaper Paint::GetWallpaper() const
{
    switch (meType)
    {
        case Paint::NoPaint:
        default:
            return Wallpaper();
            break;

        case Paint::ColorPaint:
            return Wallpaper(GetColor());
            break;

        case Paint::GradientPaint:
            return Wallpaper(GetGradient());
            break;
    }
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
