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

#ifndef _SDR_POLYPOLYGONEDITOR_HXX_
#define _SDR_POLYPOLYGONEDITOR_HXX_

#include "svx/svxdllapi.h"

#include "svx/ipolypolygoneditorcontroller.hxx"

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <set>

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
