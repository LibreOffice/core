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

#include <com/sun/star/animations/TransitionSubType.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include "ellipsewipe.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

::basegfx::B2DPolyPolygon EllipseWipe::operator () ( double t )
{
    ::basegfx::B2DPoint rCenter(0.5,0.5);
    double fRadius = ::basegfx::pruneScaleValue( t * M_SQRT2 / 2.0 );

    if( mnSubType == com::sun::star::animations::TransitionSubType::VERTICAL )
    {
        // oval:
        ::basegfx::B2DPolygon poly (
            ::basegfx::utils::createPolygonFromEllipse( rCenter, fRadius*2, fRadius ) ); //Horizontal Ellipse is rotated by 90 degress
        return ::basegfx::B2DPolyPolygon( poly );
    }
    else
    {
         // circle:
        ::basegfx::B2DPolygon poly(
            ::basegfx::utils::createPolygonFromCircle( rCenter, fRadius ) );
        return ::basegfx::B2DPolyPolygon( poly );
    }
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
