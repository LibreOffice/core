/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scaleimage.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-14 14:22:58 $
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

#ifndef INCLUDED_BASEBIMAGE_SCALEIMAGE_HXX
#define INCLUDED_BASEBIMAGE_SCALEIMAGE_HXX

#include <osl/diagnose.h>

#include <vigra/tuple.hxx>
#include <vigra/basicimage.hxx>
#include <vigra/iteratortraits.hxx>

namespace basebmp
{

template< class SourceIter, class SourceAcc,
          class DestIter, class DestAcc >
inline void scaleLine( SourceIter      s_begin,
                       SourceIter      s_end,
                       SourceAcc       s_acc,
                       DestIter        d_begin,
                       DestIter        d_end,
                       DestAcc         d_acc )
{
    const int src_width  = s_end - s_begin;
    const int dest_width = d_end - d_begin;

    OSL_ASSERT( src_width > 0 && dest_width > 0 );

    if( src_width >= dest_width )
    {
        // shrink
        int rem = 0;
        while( s_begin != s_end )
        {
            if( rem >= 0 )
            {
                d_acc.set( s_acc(s_begin), d_begin );

                rem -= src_width;
                ++d_begin;
            }

            rem += dest_width;
            ++s_begin;
        }
    }
    else
    {
        // enlarge
        int rem = -dest_width;
        while( d_begin != d_end )
        {
            if( rem >= 0 )
            {
                rem -= dest_width;
                ++s_begin;
            }

            d_acc.set( s_acc(s_begin), d_begin );

            rem += src_width;
            ++d_begin;
        }
    }
}

template< class SourceIter, class SourceAcc,
          class DestIter, class DestAcc >
inline void scaleImage( SourceIter      s_begin,
                        SourceIter      s_end,
                        SourceAcc       s_acc,
                        DestIter        d_begin,
                        DestIter        d_end,
                        DestAcc         d_acc )
{
    const int src_width ( s_end.x - s_begin.x );
    const int src_height( s_end.y - s_begin.y );

    const int dest_width ( d_end.x - d_begin.x );
    const int dest_height( d_end.y - d_begin.y );

    typedef vigra::BasicImage<typename SourceAcc::value_type> TmpImage;
    typedef typename TmpImage::traverser TmpImageIter;

    TmpImage     tmp_image(src_width,
                           dest_height);
    TmpImageIter t_begin = tmp_image.upperLeft();

    // scale in y direction
    for( int x=0; x<src_width; ++x, ++s_begin.x, ++t_begin.x )
    {
        typename SourceIter::column_iterator   s_cbegin = s_begin.columnIterator();
        typename TmpImageIter::column_iterator t_cbegin = t_begin.columnIterator();

        scaleLine(s_cbegin, s_cbegin+src_height, s_acc,
                  t_cbegin, t_cbegin+dest_height, tmp_image.accessor());
    }

    t_begin = tmp_image.upperLeft();

    // scale in x direction
    for( int y=0; y<dest_height; ++y, ++d_begin.y, ++t_begin.y )
    {
        typename DestIter::row_iterator     d_rbegin = d_begin.rowIterator();
        typename TmpImageIter::row_iterator t_rbegin = t_begin.rowIterator();

        scaleLine(t_rbegin, t_rbegin+src_width, tmp_image.accessor(),
                  d_rbegin, d_rbegin+dest_width, d_acc);
    }
}

template< class SourceIter, class SourceAcc,
          class DestIter, class DestAcc >
inline void scaleImage( vigra::triple<SourceIter,SourceIter,SourceAcc> const& src,
                        vigra::triple<DestIter,DestIter,DestAcc> const&       dst )
{
    scaleImage(src.first,src.second,src.third,
               dst.first,dst.second,dst.third);
}

}

#endif /* INCLUDED_BASEBIMAGE_SCALEIMAGE_HXX */
