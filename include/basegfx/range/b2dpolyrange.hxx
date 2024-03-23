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

#pragma once

#include <o3tl/cow_wrapper.hxx>
#include <tuple>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B2DRange;
    class B2DPolyPolygon;
    class B2DHomMatrix;
    class ImplB2DPolyRange;

    /** Multiple ranges in one object.

        This class combines multiple ranges in one object, providing a
        total, enclosing range for it.

        You can use this class e.g. when updating views containing
        rectangular objects. Add each modified object to a
        B2DMultiRange, then test each viewable object against
        intersection with the multi range.

        Similar in spirit to the poly-polygon vs. polygon relationship.

        Note that comparable to polygons, a poly-range can also
        contain 'holes' - this is encoded via polygon orientation at
        the poly-polygon, and via explicit flags for the poly-range.
     */
    class BASEGFX_DLLPUBLIC B2DPolyRange
    {
    public:
        typedef std::tuple<B2DRange, B2VectorOrientation> ElementType;

        B2DPolyRange();
        ~B2DPolyRange();

        /** Create a multi range with exactly one containing range
         */
        B2DPolyRange( const B2DPolyRange& );
        B2DPolyRange& operator=( const B2DPolyRange& );

        bool operator==(const B2DPolyRange&) const;

        /// Number of included ranges
        sal_uInt32 count() const;

        ElementType getElement(sal_uInt32 nIndex) const;

        // insert/append a single range
        void appendElement(const B2DRange& rRange, B2VectorOrientation eOrient);

        void clear();

        /** Test whether given range overlaps one or more of the
            included ranges. Does *not* use overall range, but checks
            individually.
         */
        bool overlaps( const B2DRange& rRange ) const;

        /** Request a poly-polygon with solved cross-overs
         */
        B2DPolyPolygon solveCrossovers() const;

        void transform(const B2DHomMatrix& rTranslate);

    private:
        o3tl::cow_wrapper< ImplB2DPolyRange > mpImpl;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
