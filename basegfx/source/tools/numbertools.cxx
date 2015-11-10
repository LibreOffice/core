/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basegfx/tools/tools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#include <utility>

namespace basegfx { namespace tools
{
    B2DPolyPolygon number2PolyPolygon(double fValue, sal_Int32 nTotalDigits, sal_Int32 nDecPlaces, bool bLitSegments)
    {
        // config here
        // {
        const double fSpace=0.2;
        // }
        // config here

        rtl::OUStringBuffer aNum;
        rtl::math::doubleToUStringBuffer(aNum,
                                         fValue,
                                         rtl_math_StringFormat_F,
                                         nDecPlaces, '.',
                                         nullptr, ',');

        B2DPolyPolygon aRes;
        B2DHomMatrix aMat;
        double fCurrX=std::max(nTotalDigits-aNum.getLength(),
                               sal_Int32(0)) * (1.0+fSpace);
        for( sal_Int32 i=0; i<aNum.getLength(); ++i )
        {
            B2DPolyPolygon aCurr;
            aCurr=createSevenSegmentPolyPolygon(aNum[i],
                                                bLitSegments);

            aMat.identity();
            aMat.translate(fCurrX,0.0);
            aCurr.transform(aMat);

            fCurrX += 1.0+fSpace;

            aRes.append(aCurr);
        }

        return aRes;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
