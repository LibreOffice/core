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
#ifndef SFX_SIDEBAR_LAYOUTER_HXX
#define SFX_SIDEBAR_LAYOUTER_HXX

#include "sfx2/dllapi.h"

class Window;

namespace sfx2 { namespace sidebar {

/** Collection of simple helper functions for layouting sidebar panels.
*/
class SFX2_DLLPUBLIC Layouter
{
public:
    /** Make the given control wider by the given value.  Negative
        values would make the control smaller.
        The height and the position of the control remain unchanged.
    */
    static void EnlargeControlHorizontally (
        Window& rControl,
        const sal_Int32 nDeltaX);

    static void SetWidth (
        Window& rControl,
        const sal_Int32 nWidth);

    static void SetRight (
        Window& rControl,
        const sal_Int32 nRight);

    /** Move the given control by the given value to the right.
        A negative value would move the control to the left.
        The y-position and the size of the control remain unchanged.
    */
    static void MoveControlHorizontally (
        Window& rControl,
        const sal_Int32 nDeltaX);

    static void SetHorizontalPosition (
        Window& rControl,
        const sal_Int32 nX);

    /** Set the WB_ELLIPSIS flag at the given control so that when it
        can not be shown completely it is shortened more gracefully
        then just cutting it off.  The ellipsis flag can not be set
        via the resource file.
    */
    static void PrepareForLayouting (
        Window& rControl);

    static sal_Int32 MapX (
        const Window& rControl,
        const sal_Int32 nValue);

    static sal_Int32 MapWidth (
        const Window& rControl,
        const sal_Int32 nValue);
};

} } // end of namespace sfx2::sidebar

#endif
