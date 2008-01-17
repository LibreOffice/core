/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptfilterhelpers.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
