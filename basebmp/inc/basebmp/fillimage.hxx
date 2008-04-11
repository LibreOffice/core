/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fillimage.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_BASEBMP_FILLIMAGE_HXX
#define INCLUDED_BASEBMP_FILLIMAGE_HXX

#include <vigra/tuple.hxx>
#include <vigra/iteratortraits.hxx>

namespace basebmp
{

template< class DestIterator, class DestAccessor, typename T >
void fillImage( DestIterator begin,
                DestIterator end,
                DestAccessor ad,
                T            fillVal )
{
    const int width ( end.x - begin.x );
    const int height( end.y - begin.y );

    for( int y=0; y<height; ++y, ++begin.y )
    {
        typename vigra::IteratorTraits<DestIterator>::row_iterator
            rowIter( begin.rowIterator() );
        const typename vigra::IteratorTraits<DestIterator>::row_iterator
            rowEnd( rowIter + width );

        // TODO(P2): Provide specialized span fill methods on the
        // iterator/accessor
        while( rowIter != rowEnd )
            ad.set(fillVal, rowIter++);
    }
}

template< class DestIterator, class DestAccessor, typename T >
inline void fillImage( vigra::triple<DestIterator,DestIterator,DestAccessor> const& src,
                       T                                                            fillVal )
{
    fillImage(src.first,src.second,src.third,fillVal);
}

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_FILLIMAGE_HXX */
