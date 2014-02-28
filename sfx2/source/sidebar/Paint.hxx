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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_PAINT_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_PAINT_HXX

#include <tools/color.hxx>
#include <vcl/gradient.hxx>
#include <vcl/wall.hxx>
#include <com/sun/star/awt/Gradient.hpp>

#include <boost/variant.hpp>

namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

/** Abstraction of different ways to fill outlines.
    Can be
     - none (empty: outline is not filled)
     - singular color
     - gradient
*/
class Paint
{
public:
    enum Type
    {
        NoPaint,
        ColorPaint,
        GradientPaint
    };

    // Create a Paint object for an Any that may contain a color, a
    // awt::Gradient, or nothing.
    static Paint Create (const cssu::Any& rValue);

    // Create paint with type NoPaint.
    explicit Paint (void);

    // Create a Paint object for the given color.
    explicit Paint (const Color& rColor);

    // Create a Paint object for the given gradient.
    explicit Paint (const Gradient& rGradient);

    Type GetType (void) const;
    const Color& GetColor (void) const;
    const Gradient& GetGradient (void) const;

    Wallpaper GetWallpaper (void) const;

private:
    Type meType;
    ::boost::variant<
        Color,
        Gradient
    > maValue;
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
