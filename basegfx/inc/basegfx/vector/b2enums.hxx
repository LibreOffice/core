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

#ifndef _BGFX_VECTOR_B2ENUMS_HXX
#define _BGFX_VECTOR_B2ENUMS_HXX

#include <sal/types.h>

namespace basegfx
{
    /** Descriptor for the mathematical orientations of two 2D Vectors
    */
    enum B2VectorOrientation
    {
        /// mathematically positive oriented
        ORIENTATION_POSITIVE = 0,

        /// mathematically negative oriented
        ORIENTATION_NEGATIVE,

        /// mathematically neutral, thus parallel
        ORIENTATION_NEUTRAL
    };

    /** Descriptor for the mathematical continuity of two 2D Vectors
    */
    enum B2VectorContinuity
    {
        /// none
        CONTINUITY_NONE = 0,

        /// mathematically negative oriented
        CONTINUITY_C1,

        /// mathematically neutral, thus parallel
        CONTINUITY_C2
    };

    /** Descriptor for possible line joins between two line segments
    */
    enum B2DLineJoin
    {
        B2DLINEJOIN_NONE,       // no rounding
        B2DLINEJOIN_MIDDLE,     // calc middle value between joints
        B2DLINEJOIN_BEVEL,      // join edges with line
        B2DLINEJOIN_MITER,      // extend till cut
        B2DLINEJOIN_ROUND       // create arc
    };

} // end of namespace basegfx

#endif /* _BGFX_VECTOR_B2ENUMS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
