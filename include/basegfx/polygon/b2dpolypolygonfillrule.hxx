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

#ifndef INCLUDED_BASEGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX
#define INCLUDED_BASEGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX

#include <sal/types.h>



namespace basegfx
{
    /** Fill rule to use for poly-polygon filling.

        The fill rule determines which areas are inside, and which are
        outside the poly-polygon.
     */
    enum FillRule
    {
        /** Areas, for which a scanline has crossed an odd number of
            vertices, are regarded 'inside', the remainder 'outside'
            of the poly-polygon.
         */
        FillRule_EVEN_ODD,

        /** For each edge a scanline crosses, a current winding number
            is updated. Downward edges count +1, upward edges count
            -1. If the total accumulated winding number for one area
            is not zero, this area is regarded 'inside', otherwise,
            'outside'.
         */
        FillRule_NONZERO_WINDING_NUMBER
    };
}

#endif // INCLUDED_BASEGFX_POLYGON_B2DPOLYPOLYGONFILLRULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
