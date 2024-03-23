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
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    class B2DRange;
    class B2DPolyPolygon;
    class B2DHomMatrix;
}

namespace basegfx::utils
{
        class ImplB2DClipState;

        /** This class provides an optimized, symbolic clip state for graphical output

            Having a current 'clip' state is a common attribute of
            almost all graphic output APIs, most of which internally
            represent it via a list of rectangular bands. In contrast,
            this implementation purely uses symbolic clips, but in a
            quite efficient manner, deferring actual evaluation until
            a clip representation is requested, and using faster code
            paths for common special cases (like all-rectangle clips)
         */
        class BASEGFX_DLLPUBLIC B2DClipState
        {
        public:
            typedef o3tl::cow_wrapper< ImplB2DClipState > ImplType;

        private:
            ImplType mpImpl;

        public:
            /// Init clip, in 'cleared' state - everything is visible
            B2DClipState();
            ~B2DClipState();
            B2DClipState( const B2DClipState& );
            B2DClipState( B2DClipState&& );
            explicit B2DClipState( const B2DPolyPolygon& );
            B2DClipState& operator=( const B2DClipState& );
            B2DClipState& operator=( B2DClipState&& );

            /// Set clip to 'null' - nothing is visible
            void makeNull();

            /// returns true when clip is 'cleared' - everything is visible
            bool isCleared() const;

            bool operator==(const B2DClipState&) const;

            void unionRange(const B2DRange& );
            void unionPolyPolygon(const B2DPolyPolygon& );

            void intersectRange(const B2DRange& );
            void intersectPolyPolygon(const B2DPolyPolygon& );

            void subtractRange(const B2DRange& );
            void subtractPolyPolygon(const B2DPolyPolygon& );

            void xorRange(const B2DRange& );
            void xorPolyPolygon(const B2DPolyPolygon& );

            void transform(const B2DHomMatrix& );

            B2DPolyPolygon const & getClipPoly() const;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
