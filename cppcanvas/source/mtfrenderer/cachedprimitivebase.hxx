/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedprimitivebase.hxx,v $
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

#ifndef _CPPCANVAS_CACHEDPRIMITIVEBASE_HXX
#define _CPPCANVAS_CACHEDPRIMITIVEBASE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>

#include <cppcanvas/canvas.hxx>
#include <boost/utility.hpp>

#include "action.hxx"

namespace basegfx { class B2DHomMatrix; }


/* Definition of internal::CachedPrimitiveBase class */

namespace cppcanvas
{
    namespace internal
    {
        /** Base class providing cached re-rendering, if XCanvas
            returns XCachedPrimitive

            Derive from this class and implement private render()
            method to perform the actual primitive rendering. Return
            cached primitive into given reference. Next time this
            class' public render() method gets called, the cached
            representation is taken.
         */
        class CachedPrimitiveBase : public Action,
                                    private ::boost::noncopyable
        {
        public:
            /** Constructor

                @param rCanvas
                Canvas on which this primitive is to appear

                @param bOnlyRedrawWithSameTransform
                When true, this class only reuses the cached
                primitive, if the overall transformation stays the
                same. Otherwise, repaints are always performed via the
                cached primitive.
             */
            CachedPrimitiveBase( const CanvasSharedPtr& rCanvas,
                                 bool                   bOnlyRedrawWithSameTransform );
            virtual ~CachedPrimitiveBase() {}

            virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;

        protected:
            using Action::render;

        private:
            virtual bool render( ::com::sun::star::uno::Reference<
                                     ::com::sun::star::rendering::XCachedPrimitive >& rCachedPrimitive,
                                 const ::basegfx::B2DHomMatrix&                       rTransformation ) const = 0;

            CanvasSharedPtr                                             mpCanvas;
            mutable ::com::sun::star::uno::Reference<
                    ::com::sun::star::rendering::XCachedPrimitive >     mxCachedPrimitive;
            mutable ::basegfx::B2DHomMatrix                             maLastTransformation;
            const bool                                                  mbOnlyRedrawWithSameTransform;
        };
    }
}

#endif /*_CPPCANVAS_CACHEDPRIMITIVEBASE_HXX */
