/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef OOX_PPT_PPTFILTERHELPERS
#define OOX_PPT_PPTFILTERHELPERS

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
        MS_AttributeNames	meAttribute;
        const char* mpMSName;
        const char* mpAPIName;
    };

    static const ImplAttributeNameConversion gImplConversionList[] = 
    {
        { MS_PPT_X,				"ppt_x",						"X" },
        { MS_PPT_Y,				"ppt_y",						"Y" },
        { MS_PPT_W,				"ppt_w",						"Width" },
        { MS_PPT_H,				"ppt_h",						"Height" },
        { MS_PPT_C,				"ppt_c",						"DimColor" },
        { MS_R,					"r",							"Rotate" },
        { MS_XSHEAR,			"xshear",						"SkewX" },
        { MS_FILLCOLOR,			"fillColor",					"FillColor" },
        { MS_FILLCOLOR,			"fillcolor",					"FillColor" },
        { MS_FILLTYPE,			"fill.type",					"FillStyle" },
        { MS_STROKECOLOR,		"stroke.color",					"LineColor" },
        { MS_STROKEON,			"stroke.on",					"LineStyle" },
        { MS_STYLECOLOR,		"style.color",					"CharColor" },
        { MS_STYLEROTATION,		"style.rotation",				"Rotate" },
        { MS_FONTWEIGHT,		"style.fontWeight",				"CharWeight" },
        { MS_STYLEUNDERLINE,	"style.textDecorationUnderline","CharUnderline" },
        { MS_STYLEFONTFAMILY,	"style.fontFamily",				"CharFontName" },
        { MS_STYLEFONTSIZE,		"style.fontSize",				"CharHeight" },
        { MS_STYLEFONTSTYLE,	"style.fontStyle",				"CharPosture" },
        { MS_STYLEVISIBILITY,	"style.visibility",				"Visibility" },
        { MS_STYLEOPACITY,		"style.opacity",				"Opacity" },
        { MS_UNKNOWN, NULL, NULL }
    };
    //END CUT&PASTE


    // BEGIN CUT&PASTE from sd pptanimations.hxx
    struct transition
    {
        const sal_Char* mpName;
        sal_Int16 mnType;
        sal_Int16 mnSubType;
        sal_Bool mbDirection; // true: default geometric direction
        
        static const transition* find( const rtl::OUString& rName );
        static const sal_Char* find( const sal_Int16 mnType, const sal_Int16 mnSubType, const sal_Bool bDirection );
    };
    // END CUT&PASTE


    // BEGIN CUT&PASTE from sd pptinanimation.cxx
    bool convertMeasure( ::rtl::OUString& rString );
    // END CUT&PASTE from sd pptinanimation.cxx


} }


#endif
