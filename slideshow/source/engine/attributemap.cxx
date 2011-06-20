/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>

#include "attributemap.hxx"
#include "tools.hxx"


namespace slideshow
{
    namespace internal
    {
        typedef ::canvas::tools::ValueMap< AttributeType > AnimateAttributeMap;

        AttributeType mapAttributeName( const ::rtl::OUString& rAttrName )
        {
            /** Maps attribute name to AttributeType enum.

                String entries are all case-insensitive and MUST
                BE STORED lowercase.

                String entries MUST BE SORTED in ascending order!
            */
            static AnimateAttributeMap::MapEntry lcl_attributeMap[] =
                {
                    { "charcolor", ATTRIBUTE_CHAR_COLOR },
                    { "charfontname", ATTRIBUTE_CHAR_FONT_NAME },
                    { "charheight", ATTRIBUTE_CHAR_HEIGHT },
                    { "charposture", ATTRIBUTE_CHAR_POSTURE },
                    // TODO(Q1): This should prolly be changed in PPT import
                    // { "charrotation", ATTRIBUTE_CHAR_ROTATION },
                    { "charrotation", ATTRIBUTE_ROTATE },
                    { "charunderline", ATTRIBUTE_CHAR_UNDERLINE },
                    { "charweight", ATTRIBUTE_CHAR_WEIGHT },
                    { "color", ATTRIBUTE_COLOR },
                    { "dimcolor", ATTRIBUTE_DIMCOLOR },
                    { "fillcolor", ATTRIBUTE_FILL_COLOR },
                    { "fillstyle", ATTRIBUTE_FILL_STYLE },
                    { "height", ATTRIBUTE_HEIGHT },
                    { "linecolor", ATTRIBUTE_LINE_COLOR },
                    { "linestyle", ATTRIBUTE_LINE_STYLE },
                    { "opacity", ATTRIBUTE_OPACITY },
                    { "rotate", ATTRIBUTE_ROTATE },
                    { "skewx", ATTRIBUTE_SKEW_X },
                    { "skewy", ATTRIBUTE_SKEW_Y },
                    { "visibility", ATTRIBUTE_VISIBILITY },
                    { "width", ATTRIBUTE_WIDTH },
                    { "x", ATTRIBUTE_POS_X },
                    { "y", ATTRIBUTE_POS_Y }
                };

            static AnimateAttributeMap aMap( lcl_attributeMap,
                                             sizeof(lcl_attributeMap)/sizeof(*lcl_attributeMap),
                                             false );

            AttributeType eAttributeType = ATTRIBUTE_INVALID;

            // determine the type from the attribute name
            if( !aMap.lookup( rAttrName,
                              eAttributeType ) )
            {
                OSL_TRACE( "mapAttributeName(): attribute name %s not found in map.",
                           ::rtl::OUStringToOString( rAttrName,
                                                     RTL_TEXTENCODING_ASCII_US ).getStr() );
                return ATTRIBUTE_INVALID;
            }

            return eAttributeType;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
