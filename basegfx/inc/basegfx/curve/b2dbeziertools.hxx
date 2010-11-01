/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _BGFX_CURVE_B2DBEZIERTOOLS_HXX
#define _BGFX_CURVE_B2DBEZIERTOOLS_HXX

#include <sal/types.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx
{
    class B2DCubicBezier;
}

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DCubicBezierHelper
    {
    private:
        ::std::vector< double >         maLengthArray;
        sal_uInt32                      mnEdgeCount;

    public:
        B2DCubicBezierHelper(const B2DCubicBezier& rBase, sal_uInt32 nDivisions = 9);

        double getLength() const { if(maLengthArray.size()) return maLengthArray[maLengthArray.size() - 1]; else return 0.0; }
        double distanceToRelative(double fDistance) const;
        double relativeToDistance(double fRelative) const;
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_CURVE_B2DBEZIERTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
