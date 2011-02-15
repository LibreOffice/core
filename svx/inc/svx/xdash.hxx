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

#ifndef _SVX_XDASH_HXX
#define _SVX_XDASH_HXX

//-------------
// class XDash
//-------------

#include <svx/xenum.hxx>
#include "svx/svxdllapi.h"

#include <vector>

class SVX_DLLPUBLIC XDash
{
protected:
    XDashStyle  eDash;
    sal_uInt16      nDots;
    sal_uIntPtr       nDotLen;
    sal_uInt16      nDashes;
    sal_uIntPtr       nDashLen;
    sal_uIntPtr       nDistance;

public:
          XDash(XDashStyle eDash = XDASH_RECT,
                sal_uInt16 nDots = 1, sal_uIntPtr nDotLen = 20,
                sal_uInt16 nDashes = 1, sal_uIntPtr nDashLen = 20, sal_uIntPtr nDistance = 20);

    bool operator==(const XDash& rDash) const;

    void SetDashStyle(XDashStyle eNewStyle) { eDash = eNewStyle; }
    void SetDots(sal_uInt16 nNewDots)           { nDots = nNewDots; }
    void SetDotLen(sal_uIntPtr nNewDotLen)        { nDotLen = nNewDotLen; }
    void SetDashes(sal_uInt16 nNewDashes)       { nDashes = nNewDashes; }
    void SetDashLen(sal_uIntPtr nNewDashLen)      { nDashLen = nNewDashLen; }
    void SetDistance(sal_uIntPtr nNewDistance)    { nDistance = nNewDistance; }

    XDashStyle  GetDashStyle() const        { return eDash; }
    sal_uInt16      GetDots() const             { return nDots; }
    sal_uIntPtr       GetDotLen() const           { return nDotLen; }
    sal_uInt16      GetDashes() const           { return nDashes; }
    sal_uIntPtr       GetDashLen() const          { return nDashLen; }
    sal_uIntPtr       GetDistance() const         { return nDistance; }

    // XDash is translated into an array of doubles which describe the lengths of the
    // dashes, dots and empty passages. It returns the complete length of the full DashDot
    // sequence and fills the given vetor of doubles accordingly (also resizing, so deleting it).
    double CreateDotDashArray(::std::vector< double >& rDotDashArray, double fLineWidth) const;
};

#endif
