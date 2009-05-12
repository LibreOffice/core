/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: action.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _CPPCANVAS_ACTION_HXX
#define _CPPCANVAS_ACTION_HXX

#include <sal/types.h>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

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
            virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
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

        typedef ::boost::shared_ptr< Action > ActionSharedPtr;

    }
}

#endif /* _CPPCANVAS_ACTION_HXX */
