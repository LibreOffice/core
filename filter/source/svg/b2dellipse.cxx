/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "b2dellipse.hxx"

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

namespace basegfx
{
    B2DEllipse::B2DEllipse(const basegfx::B2DPoint& rCenter, const basegfx::B2DTuple& rRadius)
    :   maCenter(rCenter), maRadius(rRadius)
    {
    }

    B2DEllipse::~B2DEllipse()
    {
    }

    bool B2DEllipse::operator==(const B2DEllipse& rEllipse) const
    {
        return (maCenter == rEllipse.maCenter) && (maRadius == rEllipse.maRadius);
    }

    bool B2DEllipse::operator!=(const B2DEllipse& rEllipse) const
    {
        return !(*this == rEllipse);
    }

    basegfx::B2DPoint B2DEllipse::getB2DEllipseCenter() const
    {
        return maCenter;
    }

    basegfx::B2DTuple B2DEllipse::getB2DEllipseRadius() const
    {
        return maRadius;
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
