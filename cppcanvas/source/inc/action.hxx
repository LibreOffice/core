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

#ifndef INCLUDED_CPPCANVAS_SOURCE_INC_ACTION_HXX
#define INCLUDED_CPPCANVAS_SOURCE_INC_ACTION_HXX

#include <sal/types.h>
#include <memory>

namespace basegfx
{
    class B2DHomMatrix;
    class B2DRange;
}


/* Definition of Action interface */

namespace cppcanvas
{
    namespace internal
    {
        /** Interface for internal render actions

            This interface is implemented by all objects generated
            from the metafile renderer, and corresponds roughly to the
            VCL meta action.
         */
        class Action
        {
        public:
            /** Used for rendering action subsets

                There are several cases where an Action might have
                subsettable content, e.g. text, or referenced
                metafiles, like the transparent action.

                Generally, at the metafile renderer, all actions are
                'flattened' out, i.e. a meta action rendering the
                string "Hello" counts five indices, and a transparent
                action containing a metafile with 100 actions counts
                at least 100 indices (contained transparency or text
                actions recursively add to this value). From the
                outside, the subset to render is referenced via this
                flat index range
             */
            struct Subset
            {
                /** Denotes start of the subset.

                    The index given here specifies the first subaction
                    to render.
                 */
                sal_Int32   mnSubsetBegin;

                /** Denotes end of the subset

                    The index given here specifies the first subaction
                    <em>not<em> to render, i.e. one action behind the
                    subset to be rendered
                 */
                sal_Int32   mnSubsetEnd;
            };

            virtual ~Action() {}

            /** Render this action to the associated canvas

                @param rTransformation
                Transformation matrix to apply before rendering

                @return true, if rendering was successful. If
                rendering failed, false is returned.
             */
            virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const = 0;

            /** Render the given part of the action to the associated
                canvas.

                @param rTransformation
                Transformation matrix to apply before rendering

                @param rSubset
                Subset of the action to render. See Subset description
                for index semantics.

                @return true, if rendering was successful. If the
                specified subset is invalid for this action, or if
                rendering failed for other reasons, false is returned.
             */
            virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                       const Subset&                  rSubset ) const = 0;

            /** Query bounds of this action on the associated canvas

                @param rTransformation
                Transformation matrix to apply

                @return the bounds for this action in device
                coordinate space.
             */
            virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const = 0;

            /** Query bounds for the given part of the action on the
                associated canvas.

                @param rTransformation
                Transformation matrix to apply.

                @param rSubset
                Subset of the action to query. See Subset description
                for index semantics.

                @return the bounds for the given subset in device
                coordinate space.
             */
            virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                   const Subset&                    rSubset ) const = 0;

            /** Query action count.

                This method returns the number of subset actions
                contained in this action. The render( Subset ) method
                must accept subset ranges up to the value returned
                here.

                @return the number of subset actions
             */
            virtual sal_Int32 getActionCount() const = 0;
        };

        typedef std::shared_ptr< Action > ActionSharedPtr;

    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_INC_ACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
