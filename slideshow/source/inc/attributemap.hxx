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

#ifndef INCLUDED_SLIDESHOW_ATTRIBUTEMAP_HXX
#define INCLUDED_SLIDESHOW_ATTRIBUTEMAP_HXX

#include <rtl/ustring.hxx>

/* Definition of AttributeMap helper function */

namespace slideshow
{
    namespace internal
    {
        /** Type of to-be-animated attribute.

            This enum describes the type of an animated
            attribute.
        */
        enum AttributeType
        {
            ATTRIBUTE_INVALID,
            ATTRIBUTE_CHAR_COLOR,
            ATTRIBUTE_CHAR_FONT_NAME,
            ATTRIBUTE_CHAR_HEIGHT,
            ATTRIBUTE_CHAR_POSTURE,
            ATTRIBUTE_CHAR_ROTATION,
            ATTRIBUTE_CHAR_UNDERLINE,
            ATTRIBUTE_CHAR_WEIGHT,
            ATTRIBUTE_COLOR,
            ATTRIBUTE_DIMCOLOR,
            ATTRIBUTE_FILL_COLOR,
            ATTRIBUTE_FILL_STYLE,
            ATTRIBUTE_HEIGHT,
            ATTRIBUTE_LINE_COLOR,
            ATTRIBUTE_LINE_STYLE,
            ATTRIBUTE_OPACITY,
            ATTRIBUTE_ROTATE,
            ATTRIBUTE_SKEW_X,
            ATTRIBUTE_SKEW_Y,
            ATTRIBUTE_VISIBILITY,
            ATTRIBUTE_WIDTH,
            ATTRIBUTE_POS_X,
            ATTRIBUTE_POS_Y
        };

        /** Map attribute name to AttributeType enum

            @returns ATTRIBUTE_INVALID, if name was not found in the
            mapping table.
         */
        AttributeType mapAttributeName( const ::rtl::OUString& rAttrName );
    }
}

#endif /* INCLUDED_SLIDESHOW_ATTRIBUTEMAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
