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

#ifndef _SDR_POLYPOLYGONEDITOR_HXX_
#define _SDR_POLYPOLYGONEDITOR_HXX_

#include "svx/svxdllapi.h"

#include "svx/ipolypolygoneditorcontroller.hxx"

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <set>

class SdrUShortCont;

namespace sdr
{

/** this class implements some helper functions to edit a B2DPolyPolygon */
class SVX_DLLPUBLIC PolyPolygonEditor
{
public:
    PolyPolygonEditor( const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed );

    const basegfx::B2DPolyPolygon& GetPolyPolygon() const { return maPolyPolygon; }

    /** returns true if the B2DPolyPolygon was changed.
        Warning: B2DPolyPolygon can be empty after this operation!
    */
    bool DeletePoints( const std::set< sal_uInt16 >& rAbsPoints );

    /** returns true if the B2DPolyPolygon was changed.
    */
    bool SetSegmentsKind(SdrPathSegmentKind eKind, const std::set< sal_uInt16 >& rAbsPoints);

    /** returns true if the B2DPolyPolygon was changed.
    */
    bool SetPointsSmooth( basegfx::B2VectorContinuity eFlags, const std::set< sal_uInt16 >& rAbsPoints);

    /** Outputs the realative position ( polygon number and point number in that polygon ) from the absolut point number.
        False is returned if the given absolute point is greater not inside this B2DPolyPolygon
    */
    static bool GetRelativePolyPoint( const basegfx::B2DPolyPolygon& rPoly, sal_uInt32 nAbsPnt, sal_uInt32& rPolyNum, sal_uInt32& rPointNum );

private:
    basegfx::B2DPolyPolygon maPolyPolygon;
    bool mbIsClosed;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
