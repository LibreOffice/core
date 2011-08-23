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

#include <osl/diagnose.h>

#include <basegfx/point/b2dpoint.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>

#include <rtl/instance.hxx>

#include <boost/scoped_ptr.hpp>
#include <vector>
#include <algorithm>

class ImplB2DEllipse
{
     basegfx::B2DPoint maCenter;
     basegfx::B2DTuple maRadius;

public:
    ImplB2DEllipse()
    :	maCenter(0.0f, 0.0f),
        maRadius(0.0f, 0.0f)
    {}

    ImplB2DEllipse(const ImplB2DEllipse& rToBeCopied)
    :	maCenter(rToBeCopied.maCenter),
        maRadius(rToBeCopied.maRadius)
    {}

    ImplB2DEllipse& operator=( const ImplB2DEllipse& rToBeCopied )
    {
        maCenter = rToBeCopied.maCenter;
        maRadius = rToBeCopied.maRadius;

        return *this;
    }

    bool isEqual(const ImplB2DEllipse& rCandidate) const
    {
        return (maCenter == rCandidate.maCenter)
            && (maRadius == rCandidate.maRadius);
    }

    basegfx::B2DPoint getCenter() const
    {
        return maCenter;
    }

    void setCenter(const basegfx::B2DPoint& rCenter)
    {
        maCenter = rCenter;
    }

    basegfx::B2DTuple getRadius() const
    {
        return maRadius;
    }

    void setRadius(const basegfx::B2DTuple& rRadius)
    {
        maRadius = rRadius;
    }


    void transform(const basegfx::B2DHomMatrix& /* rMatrix */)
    {
    }
};

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{

    B2DEllipse::B2DEllipse()
    {}

    B2DEllipse::B2DEllipse(const basegfx::B2DPoint& rCenter, const basegfx::B2DTuple& rRadius)
    :	maCenter(rCenter), maRadius(rRadius)
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

    void B2DEllipse::setB2DEllipseCenter(const basegfx::B2DPoint& rCenter)
    {
        maCenter = rCenter;
    }

    basegfx::B2DTuple B2DEllipse::getB2DEllipseRadius() const
    {
        return maRadius;
    }

    void B2DEllipse::setB2DEllipseRadius(const basegfx::B2DTuple& rRadius)
    {
        maRadius = rRadius;
    }

    void B2DEllipse::transform(const basegfx::B2DHomMatrix& /* rMatrix */)
    {
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
