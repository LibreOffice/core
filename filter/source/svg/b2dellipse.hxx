/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _BASEGFX_B2DELLIPSE_HXX
#define _BASEGFX_B2DELLIPSE_HXX

#include <sal/types.h>

#include <o3tl/cow_wrapper.hxx>

#include <basegfx/point/b2dpoint.hxx>

#include <basegfx/tuple/b2dtuple.hxx>

#include <basegfx/vector/b2enums.hxx>

namespace basegfx
{
    class B2DPoint;
} // end of namespace basegfx

namespace basegfx
{
    class B2DEllipse
    {
    private:
        const basegfx::B2DPoint maCenter;
        const basegfx::B2DTuple maRadius;

    public:
        B2DEllipse(const B2DEllipse& rEllipse);
        B2DEllipse(const basegfx::B2DPoint& rCenter, const basegfx::B2DTuple& rRadius);
        ~B2DEllipse();

        // assignment operator
        B2DEllipse& operator=(const B2DEllipse& rEllipse);

        // compare operators
        bool operator==(const B2DEllipse& rEllipse) const;
        bool operator!=(const B2DEllipse& rEllipse) const;

        // Coordinate interface
        basegfx::B2DPoint getB2DEllipseCenter() const;
        basegfx::B2DTuple getB2DEllipseRadius() const;
    };
} // end of namespace basegfx

#endif /* _BASEGFX_B2DELLIPSE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
