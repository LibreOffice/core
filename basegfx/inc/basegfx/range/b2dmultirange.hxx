/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dmultirange.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _BGFX_RANGE_B2DMULTIRANGE_HXX
#define _BGFX_RANGE_B2DMULTIRANGE_HXX

#include <o3tl/cow_wrapper.hxx>
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

        B2DMultiRange( const B2DMultiRange& );
        B2DMultiRange& operator=( const B2DMultiRange& );

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
        o3tl::cow_wrapper< ImplB2DMultiRange > mpImpl;
    };
}

#endif /* _BGFX_RANGE_B2DMULTIRANGE_HXX */
