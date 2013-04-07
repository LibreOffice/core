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
OUString lookUpColorName(long color);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
