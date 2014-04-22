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

#ifndef INCLUDED_OOX_SOURCE_PPT_PPTFILTERHELPERS_HXX
#define INCLUDED_OOX_SOURCE_PPT_PPTFILTERHELPERS_HXX

#include <rtl/ustring.hxx>

namespace oox { namespace ppt {


//BEGIN CUT&PASTE from sd pptanimations.hxx
    // conversion of MS to OOo attributes.
    enum MS_AttributeNames
    {
        MS_PPT_X, MS_PPT_Y, MS_PPT_W, MS_PPT_H, MS_PPT_C, MS_R, MS_XSHEAR, MS_FILLCOLOR, MS_FILLTYPE,
        MS_STROKECOLOR, MS_STROKEON, MS_STYLECOLOR, MS_STYLEROTATION, MS_FONTWEIGHT,
        MS_STYLEUNDERLINE, MS_STYLEFONTFAMILY, MS_STYLEFONTSIZE, MS_STYLEFONTSTYLE,
        MS_STYLEVISIBILITY, MS_STYLEOPACITY, MS_UNKNOWN
    };

    struct ImplAttributeNameConversion
    {
        MS_AttributeNames   meAttribute;
        const char* mpMSName;
        const char* mpAPIName;
    };

    static const ImplAttributeNameConversion gImplConversionList[] =
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
    //END CUT&PASTE


    // BEGIN CUT&PASTE from sd pptanimations.hxx
    struct transition
    {
        const sal_Char* mpName;
        sal_Int16 mnType;
        sal_Int16 mnSubType;
        bool mbDirection; // true: default geometric direction

        static const transition* find( const OUString& rName );
        static const sal_Char* find( const sal_Int16 mnType, const sal_Int16 mnSubType, const bool bDirection );
    };
    // END CUT&PASTE


    // BEGIN CUT&PASTE from sd pptinanimation.cxx
    bool convertMeasure( OUString& rString );
    // END CUT&PASTE from sd pptinanimation.cxx


} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
