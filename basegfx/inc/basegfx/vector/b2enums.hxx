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
