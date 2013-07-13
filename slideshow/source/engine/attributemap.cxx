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

        AttributeType mapAttributeName( const OUString& rAttrName )
        {
            /** Maps attribute name to AttributeType enum.

                String entries are all case-insensitive and MUST
                BE STORED lowercase.

                String entries MUST BE SORTED in ascending order!
            */
            static const AnimateAttributeMap::MapEntry lcl_attributeMap[] =
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

            static const AnimateAttributeMap aMap( lcl_attributeMap,
                                             SAL_N_ELEMENTS(lcl_attributeMap),
                                             false );

            AttributeType eAttributeType = ATTRIBUTE_INVALID;

            // determine the type from the attribute name
            if( !aMap.lookup( rAttrName,
                              eAttributeType ) )
            {
                OSL_TRACE( "mapAttributeName(): attribute name %s not found in map.",
                           OUStringToOString( rAttrName,
                                                     RTL_TEXTENCODING_ASCII_US ).getStr() );
                return ATTRIBUTE_INVALID;
            }

            return eAttributeType;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
