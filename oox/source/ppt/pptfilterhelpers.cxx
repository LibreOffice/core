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

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <oox/ppt/pptfilterhelpers.hxx>
#include <rtl/ustrbuf.hxx>

namespace oox { namespace ppt {

    const ImplAttributeNameConversion *getAttributeConversionList()
    {
        static const ImplAttributeNameConversion aList[] =
        {
            { MS_PPT_X,             "ppt_x",                        "X" },
            { MS_PPT_Y,             "ppt_y",                        "Y" },
            { MS_PPT_W,             "ppt_w",                        "Width" },
            { MS_PPT_H,             "ppt_h",                        "Height" },
            { MS_PPT_C,             "ppt_c",                        "DimColor" },
            { MS_R,                 "r",                            "Rotate" },
            { MS_XSHEAR,            "xshear",                       "SkewX" },
            { MS_FILLCOLOR,         "fillColor",                    "FillColor" },
            { MS_FILLCOLOR,         "fillcolor",                    "FillColor" },
            { MS_FILLTYPE,          "fill.type",                    "FillStyle" },
            { MS_STROKECOLOR,       "stroke.color",                 "LineColor" },
            { MS_STROKEON,          "stroke.on",                    "LineStyle" },
            { MS_STYLECOLOR,        "style.color",                  "CharColor" },
            { MS_STYLEROTATION,     "style.rotation",               "Rotate" },
            { MS_FONTWEIGHT,        "style.fontWeight",             "CharWeight" },
            { MS_STYLEUNDERLINE,    "style.textDecorationUnderline","CharUnderline" },
            { MS_STYLEFONTFAMILY,   "style.fontFamily",             "CharFontName" },
            { MS_STYLEFONTSIZE,     "style.fontSize",               "CharHeight" },
            { MS_STYLEFONTSTYLE,    "style.fontStyle",              "CharPosture" },
            { MS_STYLEVISIBILITY,   "style.visibility",             "Visibility" },
            { MS_STYLEOPACITY,      "style.opacity",                "Opacity" },
            { MS_UNKNOWN, NULL, NULL }
        };

        return aList;
    }

    const transition* transition::getList()
    {
        static const transition aList[] =
        {
            { "wipe(up)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::TOPTOBOTTOM, true },
            { "wipe(right)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::LEFTTORIGHT, false },
            { "wipe(left)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::LEFTTORIGHT, true },
            { "wipe(down)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::TOPTOBOTTOM, false },
            { "wheel(1)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::ONEBLADE, true },
            { "wheel(2)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::TWOBLADEVERTICAL, true },
            { "wheel(3)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::THREEBLADE, true },
            { "wheel(4)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::FOURBLADE, true },
            { "wheel(8)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::EIGHTBLADE, true },
            { "strips(downLeft)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALRIGHT, true },
            { "strips(upLeft)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALLEFT, false },
            { "strips(downRight)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALLEFT, true },
            { "strips(upRight)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALRIGHT, false },
            { "barn(inVertical)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::VERTICAL, false },
            { "barn(outVertical)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::VERTICAL, true },
            { "barn(inHorizontal)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::HORIZONTAL, false },
            { "barn(outHorizontal)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "randombar(vertical)", css::animations::TransitionType::RANDOMBARWIPE, css::animations::TransitionSubType::VERTICAL, true},
            { "randombar(horizontal)", css::animations::TransitionType::RANDOMBARWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "checkerboard(down)", css::animations::TransitionType::CHECKERBOARDWIPE, css::animations::TransitionSubType::DOWN, true},
            { "checkerboard(across)", css::animations::TransitionType::CHECKERBOARDWIPE, css::animations::TransitionSubType::ACROSS, true },
            { "plus(out)", css::animations::TransitionType::FOURBOXWIPE, css::animations::TransitionSubType::CORNERSIN, false },
            { "plus(in)", css::animations::TransitionType::FOURBOXWIPE, css::animations::TransitionSubType::CORNERSIN, true },
            { "diamond(out)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::DIAMOND, true },
            { "diamond(in)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::DIAMOND, false },
            { "circle(out)", css::animations::TransitionType::ELLIPSEWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "circle(in)", css::animations::TransitionType::ELLIPSEWIPE, css::animations::TransitionSubType::HORIZONTAL, false },
            { "box(out)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::RECTANGLE, true },
            { "box(in)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::RECTANGLE, false },
            { "wedge", css::animations::TransitionType::FANWIPE, css::animations::TransitionSubType::CENTERTOP, true },
            { "blinds(vertical)", css::animations::TransitionType::BLINDSWIPE, css::animations::TransitionSubType::VERTICAL, true },
            { "blinds(horizontal)", css::animations::TransitionType::BLINDSWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "fade", css::animations::TransitionType::FADE, css::animations::TransitionSubType::CROSSFADE, true },
            { "slide(fromTop)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMTOP, true },
            { "slide(fromRight)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMRIGHT, true },
            { "slide(fromLeft)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMLEFT, true },
            { "slide(fromBottom)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMBOTTOM, true },
            { "dissolve", css::animations::TransitionType::DISSOLVE, css::animations::TransitionSubType::DEFAULT, true },
            { "image", css::animations::TransitionType::DISSOLVE, css::animations::TransitionSubType::DEFAULT, true }, // TODO
            { NULL, 0, 0, false }
        };

        return aList;
    }

    const transition* transition::find( const OUString& rName )
    {
        const transition* p = transition::getList();

        while( p->mpName )
        {
            if( rName.equalsAscii( p->mpName ) )
                return p;

            p++;
        }

        return NULL;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
