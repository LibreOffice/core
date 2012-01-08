/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Fridrich Strba  <fridrich.strba@bluewin.ch>
 *       Thorsten Behrens <tbehrens@novell.com>
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
