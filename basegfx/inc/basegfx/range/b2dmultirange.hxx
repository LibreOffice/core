/*************************************************************************
 *
 *  $RCSfile: b2dmultirange.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 18:34:43 $
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

#ifndef _BGFX_RANGE_B2DMULTIRANGE_HXX
#define _BGFX_RANGE_B2DMULTIRANGE_HXX

#include <memory>


namespace basegfx
{
    class B2DTuple;
    class B2DRange;
    class B2DPolyPolygon;
    class ImplB2DMultiRange;

    /** Multiple ranges in one object.

        This class combines multiple ranges in one object, providing a
        total, enclosing range for it.

        You can use this class e.g. when updating views containing
        rectangular objects. Add each modified object to a
        B2DMultiRange, then test each viewable object against
        intersection with the multi range.
     */
    class B2DMultiRange
    {
    public:
        B2DMultiRange();
        ~B2DMultiRange();

        /** Create a multi range with exactly one containing range
         */
        explicit B2DMultiRange( const B2DRange& rRange );

        /** Check whether range is empty.

            @return true, if this object either contains no ranges at
            all, or all contained ranges are empty.
         */
        bool            isEmpty() const;

        /** Reset to empty.

            After this call, the object will not contain any ranges,
            and isEmpty() will return true.
         */
        void            reset();

        /** Test whether given tuple is inside one or more of the
            included ranges.
         */
        bool            isInside( const B2DTuple& rTuple ) const;

        /** Test whether given range is inside one or more of the
            included ranges.
         */
        bool            isInside( const B2DRange& rRange ) const;

        /** Test whether given range overlaps one or more of the
            included ranges.
         */
        bool            overlaps( const B2DRange& rRange ) const;

        /** Add given range to the number of contained ranges.
         */
        void            addRange( const B2DRange& rRange );

        /** Get overall bound rect for all included ranges.
         */
        B2DRange        getBounds() const;

        /** Request poly-polygon representing the added ranges.

            This method creates a poly-polygon, consisting exactly out
            of the contained ranges.
         */
        B2DPolyPolygon  getPolyPolygon() const;

    private:
        // TODO(F1): Maybe provide a shared COW implementation here

        // default: disabled copy/assignment
        B2DMultiRange(const B2DMultiRange&);
        B2DMultiRange& operator=( const B2DMultiRange& );

        ::std::auto_ptr< ImplB2DMultiRange >    mpImpl;
    };
}

#endif /* _BGFX_RANGE_B2DMULTIRANGE_HXX */
