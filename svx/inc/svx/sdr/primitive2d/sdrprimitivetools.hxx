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

#ifndef INCLUDED_SDR_PRIMITIVE2D_PRIMITIVETOOLS_HXX
#define INCLUDED_SDR_PRIMITIVE2D_PRIMITIVETOOLS_HXX

#include <vcl/bitmapex.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////
// helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        // create a 3x3 cross in given color as BitmapEx
        BitmapEx createDefaultCross_3x3(const basegfx::BColor& rBColor);
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_PRIMITIVETOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
