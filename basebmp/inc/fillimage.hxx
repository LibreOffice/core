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

#ifndef INCLUDED_BASEBMP_INC_FILLIMAGE_HXX
#define INCLUDED_BASEBMP_INC_FILLIMAGE_HXX

#include <vigra/tuple.hxx>
#include <vigra/iteratortraits.hxx>

namespace basebmp
{

template< class DestIterator, class DestAccessor, typename T >
void fillImage( DestIterator begin,
                const DestIterator& end,
                DestAccessor ad,
                const T&     fillVal )
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
                       const T& fillVal )
{
    fillImage(src.first,src.second,src.third,fillVal);
}

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_FILLIMAGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
