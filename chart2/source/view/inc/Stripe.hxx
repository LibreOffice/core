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
#pragma once

#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/uno/Any.h>

namespace chart
{

/** A Stripe represents a 2 dimensional foursquare plane in a 3 dimensional room.

@todo could: it is not necessary to have 4 point members here; it would be sufficient to have one point and 2 directions
*/

class Stripe
{
public:
    Stripe( const css::drawing::Position3D& rPoint1
        , const css::drawing::Direction3D& rDirectionToPoint2
        , const css::drawing::Direction3D& rDirectionToPoint4 );

    Stripe( const css::drawing::Position3D& rPoint1
        , const css::drawing::Position3D& rPoint2
        , double fDepth );

    Stripe( const css::drawing::Position3D& rPoint1
        , const css::drawing::Position3D& rPoint2
        , const css::drawing::Position3D& rPoint3
        , const css::drawing::Position3D& rPoint4 );

    void SetManualNormal( const css::drawing::Direction3D& rNormal );
    css::drawing::Direction3D getNormal() const;

    void InvertNormal( bool bInvertNormal );

    css::uno::Any         getPolyPolygonShape3D() const;
    css::uno::Any         getNormalsPolygon() const;
    static css::uno::Any  getTexturePolygon( short nRotatedTexture ); //0 to 7 are the different possibilities

private:
    css::drawing::Position3D m_aPoint1;
    css::drawing::Position3D m_aPoint2;
    css::drawing::Position3D m_aPoint3;
    css::drawing::Position3D m_aPoint4;

    bool m_bInvertNormal;
    bool m_bManualNormalSet;
    css::drawing::Direction3D m_aManualNormal;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
