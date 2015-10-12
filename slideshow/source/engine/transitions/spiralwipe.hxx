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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_SPIRALWIPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_SPIRALWIPE_HXX

#include "parametricpolypolygon.hxx"
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>


namespace slideshow {
namespace internal {

/// Generates a topLeftClockWise or
/// bottomLeftCounterClockWise (flipOnYAxis=true) spiral wipe:
class SpiralWipe : public ParametricPolyPolygon
{
public:
    SpiralWipe( sal_Int32 nElements, bool flipOnYAxis = false );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t ) override;
protected:
    ::basegfx::B2DPolyPolygon calcNegSpiral( double t ) const;

    const sal_Int32 m_elements;
    const sal_Int32 m_sqrtElements;
    const bool m_flipOnYAxis;
};

/// Generates a twoBoxLeft or fourBoxHorizontal wipe:
class BoxSnakesWipe : public SpiralWipe
{
public:
    BoxSnakesWipe( sal_Int32 nElements, bool fourBox = false )
        : SpiralWipe(nElements), m_fourBox(fourBox) {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t ) override;
private:
    const bool m_fourBox;
};

}
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_TRANSITIONS_SPIRALWIPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
