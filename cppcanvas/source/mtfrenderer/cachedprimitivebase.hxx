/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedprimitivebase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:40:01 $
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
