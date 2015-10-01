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

#ifndef INCLUDED_BASEBMP_INC_SCALEIMAGE_HXX
#define INCLUDED_BASEBMP_INC_SCALEIMAGE_HXX

#include <osl/diagnose.h>

#include <vigra/tuple.hxx>
#include <vigra/copyimage.hxx>
#include <vigra/basicimage.hxx>
#include <vigra/iteratortraits.hxx>

namespace basebmp
{

template< class SourceIter, class SourceAcc,
          class DestIter, class DestAcc >
void scaleLine( SourceIter      s_begin,
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

/** Scale an image using zero order interpolation (pixel replication)

    Source and destination range must be at least one pixel wide and
    high.

    @param s_begin
    Start iterator for source image

    @param s_end
    End iterator for source image

    @param s_acc
    Source accessor

    @param d_begin
    Start iterator for destination image

    @param d_end
    End iterator for destination image

    @param d_acc
    Destination accessor

    @param bMustCopy
    When true, scaleImage always copies source, even when doing 1:1
    copy
 */
template< class SourceIter, class SourceAcc,
          class DestIter, class DestAcc >
void scaleImage( SourceIter      s_begin,
                 SourceIter      s_end,
                 SourceAcc       s_acc,
                 DestIter        d_begin,
                 DestIter        d_end,
                 DestAcc         d_acc,
                 bool            bMustCopy=false )
{
    const int src_width ( s_end.x - s_begin.x );
    const int src_height( s_end.y - s_begin.y );

    const int dest_width ( d_end.x - d_begin.x );
    const int dest_height( d_end.y - d_begin.y );

    if( !bMustCopy &&
        src_width == dest_width &&
        src_height == dest_height )
    {
        // no scaling involved, can simply copy
        vigra::copyImage( s_begin, s_end, s_acc,
                          d_begin, d_acc );
        return;
    }

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

/** Scale an image, range tuple version

    @param bMustCopy
    When true, scaleImage always copies source, even when doing 1:1
    copy
 */
template< class SourceIter, class SourceAcc,
          class DestIter, class DestAcc >
inline void scaleImage( vigra::triple<SourceIter,SourceIter,SourceAcc> const& src,
                        vigra::triple<DestIter,DestIter,DestAcc> const&       dst,
                        bool                                                  bMustCopy=false )
{
    scaleImage(src.first,src.second,src.third,
               dst.first,dst.second,dst.third,
               bMustCopy);
}

}

#endif /* INCLUDED_BASEBMP_INC_SCALEIMAGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
