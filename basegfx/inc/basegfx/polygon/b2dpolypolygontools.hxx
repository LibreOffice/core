/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygontools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-06 16:30:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#define _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        // predefinitions
        class B2DPolyPolygon;
    } // end of namespace polygon

    namespace polygon
    {
        namespace tools
        {
            // B2DPolyPolygon tools

            // Check and evtl. correct orientations of all contained Polygons so that
            // the orientations of contained polygons will variate to express areas and
            // holes
            void correctOrientations(::basegfx::polygon::B2DPolyPolygon& rCandidate);

            // Remove all intersections, the self intersections and the in-between
            // polygon intersections. After this operation there are no more intersections
            // in the given PolyPolygon. Only closed polygons are handled. The non-closed
            // polygons or the ones with less than 3 points are preserved, but not
            // computed.
            // bForceOrientation: If sal_True, the orientations of all contained polygons
            // is changed to ORIENTATION_POSITIVE before computing.
            // bInvertRemove: if sal_True, created polygons which are inside others and
            // have the same orientation are removed (cleanup).
            void removeIntersections(::basegfx::polygon::B2DPolyPolygon& rCandidate,
                sal_Bool bForceOrientation = sal_True, sal_Bool bInvertRemove = sal_False);

        } // end of namespace tools
    } // end of namespace polygon
} // end of namespace basegfx

#endif //   _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
