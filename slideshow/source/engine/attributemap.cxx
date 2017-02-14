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
                    { "charcolor", AttributeType::CharColor },
                    { "charfontname", AttributeType::CharFontName },
                    { "charheight", AttributeType::CharHeight },
                    { "charposture", AttributeType::CharPosture },
                    // TODO(Q1): This should prolly be changed in PPT import
                    // { "charrotation", AttributeType::CharRotation },
                    { "charrotation", AttributeType::Rotate },
                    { "charunderline", AttributeType::CharUnderline },
                    { "charweight", AttributeType::CharWeight },
                    { "color", AttributeType::Color },
                    { "dimcolor", AttributeType::DimColor },
                    { "fillcolor", AttributeType::FillColor },
                    { "fillstyle", AttributeType::FillStyle },
                    { "height", AttributeType::Height },
                    { "linecolor", AttributeType::LineColor },
                    { "linestyle", AttributeType::LineStyle },
                    { "opacity", AttributeType::Opacity },
                    { "rotate", AttributeType::Rotate },
                    { "skewx", AttributeType::SkewX },
                    { "skewy", AttributeType::SkewY },
                    { "visibility", AttributeType::Visibility },
                    { "width", AttributeType::Width },
                    { "x", AttributeType::PosX },
                    { "y", AttributeType::PosY }
                };

            static const AnimateAttributeMap aMap( lcl_attributeMap,
                                             SAL_N_ELEMENTS(lcl_attributeMap),
                                             false );

            AttributeType eAttributeType = AttributeType::Invalid;

            // determine the type from the attribute name
            if( !aMap.lookup( rAttrName,
                              eAttributeType ) )
            {
                SAL_WARN("slideshow", "mapAttributeName(): attribute name not found in map: " << rAttrName);
                return AttributeType::Invalid;
            }

            return eAttributeType;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
