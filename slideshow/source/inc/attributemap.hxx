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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ATTRIBUTEMAP_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ATTRIBUTEMAP_HXX

#include <rtl/ustring.hxx>

/* Definition of AttributeMap helper function */

namespace slideshow::internal
    {
        /** Type of to-be-animated attribute.

            This enum describes the type of an animated
            attribute.
        */
        enum class AttributeType
        {
            Invalid,
            CharColor,
            CharFontName,
            CharHeight,
            CharPosture,
            CharUnderline,
            CharWeight,
            Color,
            DimColor,
            FillColor,
            FillStyle,
            Height,
            LineColor,
            LineStyle,
            Opacity,
            Rotate,
            SkewX,
            SkewY,
            Visibility,
            Width,
            PosX,
            PosY
        };

        /** Map attribute name to AttributeType enum

            @returns AttributeType::Invalid, if name was not found in the
            mapping table.
         */
        AttributeType mapAttributeName( const OUString& rAttrName );

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ATTRIBUTEMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
