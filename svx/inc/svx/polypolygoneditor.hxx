/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SDR_POLYPOLYGONEDITOR_HXX_
#define _SDR_POLYPOLYGONEDITOR_HXX_

#include "svx/svxdllapi.h"
#include "svx/ipolypolygoneditorcontroller.hxx"
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <set>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    /** this namespace implements some helper functions to edit a B2DPolyPolygon */
    namespace PolyPolygonEditor
{
    /** returns true if the B2DPolyPolygon was changed.
        Warning: B2DPolyPolygon can be empty after this operation!
    */
        SVX_DLLPUBLIC bool DeletePoints(basegfx::B2DPolyPolygon& o_rCandidate, const sdr::selection::Indices& rAbsPoints);

    /** returns true if the B2DPolyPolygon was changed.
    */
        SVX_DLLPUBLIC bool SetSegmentsKind(basegfx::B2DPolyPolygon& o_rCandidate, SdrPathSegmentKind eKind, const sdr::selection::Indices& rAbsPoints);

    /** returns true if the B2DPolyPolygon was changed.
    */
        SVX_DLLPUBLIC bool SetPointsSmooth(basegfx::B2DPolyPolygon& o_rCandidate, basegfx::B2VectorContinuity eFlags, const sdr::selection::Indices& rAbsPoints);

    /** Outputs the realative position ( polygon number and point number in that polygon ) from the absolut point number.
        False is returned if the given absolute point is greater not inside this B2DPolyPolygon
    */
        SVX_DLLPUBLIC bool GetRelativePolyPoint(const basegfx::B2DPolyPolygon& rPoly, sal_uInt32 nAbsPnt, sal_uInt32& rPolyNum, sal_uInt32& rPointNum);
    } // end of namespace PolyPolygonEditor
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
