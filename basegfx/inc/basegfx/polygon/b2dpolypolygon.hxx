/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygon.hxx,v $
 *
 *  $Revision: 1.3 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGON_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

// predeclarations
class ImplB2DPolyPolygon;

namespace basegfx
{
    namespace polygon
    {
        class B2DPolygon;
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        class B2DPolyPolygon
        {
        private:
            ImplB2DPolyPolygon*                         mpPolyPolygon;

            // internal method to force a ref-counted instance to be copied
            // to a modifyable unique copy.
            void implForceUniqueCopy();

        public:
            B2DPolyPolygon();
            B2DPolyPolygon(const B2DPolyPolygon& rPolyPolygon);
            ~B2DPolyPolygon();

            // assignment operator
            B2DPolyPolygon& operator=(const B2DPolyPolygon& rPolyPolygon);

            // compare operators
            sal_Bool operator==(const B2DPolyPolygon& rPolyPolygon) const;
            sal_Bool operator!=(const B2DPolyPolygon& rPolyPolygon) const;

            // polygon interface
            sal_uInt32 count() const;

            B2DPolygon getPolygon(sal_uInt32 nIndex) const;
            void setPolygon(sal_uInt32 nIndex, const B2DPolygon& rPolygon);

            // insert/append single polygon
            void insert(sal_uInt32 nIndex, const B2DPolygon& rPolygon, sal_uInt32 nCount = 1);
            void append(const B2DPolygon& rPolygon, sal_uInt32 nCount = 1);

            // insert/append multiple polygons
            void insert(sal_uInt32 nIndex, const B2DPolyPolygon& rPolyPolygon);
            void append(const B2DPolyPolygon& rPolyPolygon);

            // remove
            void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

            // reset to empty state
            void clear();

            // closed state
            sal_Bool isClosed() const;
            void setClosed(sal_Bool bNew);

            // flip polygon direction
            void flip();

            // test if PolyPolygon has double points
            sal_Bool hasDoublePoints() const;

            // remove double points, at the begin/end and follow-ups, too
            void removeDoublePoints();
        };
    } // end of namespace polygon
} // end of namespace basegfx

#endif //   _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
