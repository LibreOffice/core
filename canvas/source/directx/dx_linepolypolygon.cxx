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

#include <sal/config.h>

#include <basegfx/utils/canvastools.hxx>

#include "dx_linepolypolygon.hxx"


using namespace ::com::sun::star;

namespace dxcanvas
{
    LinePolyPolygon::LinePolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly ) :
        ::basegfx::unotools::UnoPolyPolygon( rPolyPoly ),
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        mpPath()
    {
    }

    GraphicsPathSharedPtr LinePolyPolygon::getGraphicsPath( bool bNoLineJoin ) const
    {
        // generate GraphicsPath only on demand (gets deleted as soon
        // as any of the modifying methods above touches the
        // B2DPolyPolygon).
        if( !mpPath )
        {
            mpPath = tools::graphicsPathFromB2DPolyPolygon( getPolyPolygonUnsafe(), bNoLineJoin );
            mpPath->SetFillMode( const_cast<LinePolyPolygon*>(this)->getFillRule() == rendering::FillRule_EVEN_ODD ?
                                 Gdiplus::FillModeAlternate : Gdiplus::FillModeWinding );
        }

        return mpPath;
    }

    void LinePolyPolygon::modifying() const
    {
        mpPath.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
