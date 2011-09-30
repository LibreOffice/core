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

#ifndef INCLUDED_SVX_SOURCE_ACCESSIBILITY_LOOKUPCOLORNAME_HXX
#define INCLUDED_SVX_SOURCE_ACCESSIBILITY_LOOKUPCOLORNAME_HXX

#include "sal/config.h"

namespace rtl { class OUString; }

namespace accessibility {

/** This is a color name lookup targeted to be used by the accessibility
    <type>DescriptionGenerator</type> class.  It encapsulates a
    <type>com.sun.star.drawing.ColorTable</type> and provides an inverse look
    up of color names for given numerical color descriptions (the RGB values
    encoded as an integer).

    <p>The implementation uses as singleton so that the
    <type>com.sun.star.drawing.ColorTable</type> object needs to be created
    only once.  That singleton instance for now lives until the application
    terminates.  However, the color table from which it takes its values may
    change during this time.  Reacting to these changes remains a task for the
    future.</p>

    @param nColor
    This integer is the sum of the 8 Bit red value shifted left 16 Bits, the
    green value shifted left 8 Bits, and the unshifted blue value.

    @return
    The returned string is either the color name of the specified color or,
    when no name exists, a string of the form "#RRGGBB" with two hexadecimal
    digits for each color component.
*/
rtl::OUString lookUpColorName(long color);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
