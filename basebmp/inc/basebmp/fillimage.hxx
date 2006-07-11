/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillimage.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-11 11:38:55 $
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

#ifndef INCLUDED_BASEBMP_FILLIMAGE_HXX
#define INCLUDED_BASEBMP_FILLIMAGE_HXX

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

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_FILLIMAGE_HXX */
