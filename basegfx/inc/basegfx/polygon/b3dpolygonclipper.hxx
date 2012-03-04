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

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX
#define _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX

#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predefinitions
    class B3DRange;
    class B2DRange;

    namespace tools
    {
        /** define for deciding one of X,Y,Z directions
        */
        enum B3DOrientation
        {
            B3DORIENTATION_X,       // X-Axis
            B3DORIENTATION_Y,       // Y-Axis
            B3DORIENTATION_Z        // Z-Axis
        };

    } // end of namespace tools
} // end of namespace basegfx

#endif /* _BGFX_POLYPOLYGON_B3DPOLYGONCLIPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
