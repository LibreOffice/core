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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERCHARTRENDER_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERCHARTRENDER_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class OutputDevice;

namespace drawinglayer { namespace primitive2d { class ChartPrimitive2D; }}
namespace drawinglayer { namespace geometry { class ViewInformation2D; }}

//////////////////////////////////////////////////////////////////////////////
// support chart PrettyPrinter usage from primitives

namespace drawinglayer
{
    // #i101811#
    // Added current ViewInformation2D to take evtl. changed
    // ObjectTransformation into account
    bool renderChartPrimitive2D(
        const primitive2d::ChartPrimitive2D& rChartCandidate,
        OutputDevice& rOutputDevice,
        const geometry::ViewInformation2D& rViewInformation2D);

} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_HELPERCHARTRENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
