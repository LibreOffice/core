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

#ifndef INCLUDED_BASEBMP_INC_TOOLS_HXX
#define INCLUDED_BASEBMP_INC_TOOLS_HXX

#include <basegfx/range/b2ibox.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <vigra/tuple.hxx>
#include <vigra/diff2d.hxx>

namespace basebmp
{
    inline vigra::Diff2D topLeft( const basegfx::B2IBox& rRange )
    { return vigra::Diff2D(rRange.getMinX(),rRange.getMinY()); }

    inline vigra::Diff2D bottomRight( const basegfx::B2IBox& rRange )
    { return vigra::Diff2D(rRange.getMaxX(),rRange.getMaxY()); }

    template< class Iterator, class Accessor >
    inline vigra::triple<Iterator,Iterator,Accessor>
    destIterRange(Iterator const&          begin,
                  Accessor const&          accessor,
                  const basegfx::B2IBox& rRange)
    {
        return vigra::triple<Iterator,Iterator,Accessor>(
            begin + topLeft(rRange),
            begin + bottomRight(rRange),
            accessor);
    }

    template< class Iterator, class Accessor >
    inline vigra::triple<Iterator,Iterator,Accessor>
    srcIterRange(Iterator const&          begin,
                 Accessor const&          accessor,
                 const basegfx::B2IBox& rRange)
    {
        return vigra::triple<Iterator,Iterator,Accessor>(
            begin + topLeft(rRange),
            begin + bottomRight(rRange),
            accessor);
    }

    template< class Iterator, class Accessor >
    inline vigra::pair<Iterator,Accessor>
    destIter(Iterator const&          begin,
             Accessor const&          accessor,
             const basegfx::B2IPoint& rPt)
    {
        return vigra::pair<Iterator,Accessor>(
            begin + vigra::Diff2D(rPt.getX(),rPt.getY()),
            accessor);
    }
}

#endif /* INCLUDED_BASEBMP_INC_TOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
