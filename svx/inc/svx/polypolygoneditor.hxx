/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygoneditor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 13:17:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SDR_POLYPOLYGONEDITOR_HXX_
#define _SDR_POLYPOLYGONEDITOR_HXX_

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
