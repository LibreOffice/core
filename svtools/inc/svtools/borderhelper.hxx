/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef _BORDERHELPER_HXX
#define _BORDERHELPER_HXX

#include "svtools/svtdllapi.h"

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <vcl/outdev.hxx>
#include <vector>

namespace svtools
{
    SVT_DLLPUBLIC basegfx::B2DPolyPolygon ApplyLineDashing( const basegfx::B2DPolygon& rPolygon,
            sal_uInt16 nDashing, MapUnit eUnit );

    SVT_DLLPUBLIC void DrawLine( OutputDevice& rDev, const basegfx::B2DPoint& rBeg,
            const basegfx::B2DPoint& rEnd, sal_uInt32 nWidth, sal_uInt16 nDashing );

    SVT_DLLPUBLIC void DrawLine( OutputDevice& rDev, const Point& rBeg,
            const Point& rEnd, sal_uInt32 nWidth, sal_uInt16 nDashing );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
