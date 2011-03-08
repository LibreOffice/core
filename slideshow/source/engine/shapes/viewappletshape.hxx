/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_SLIDESHOW_VIEWAPPLETSHAPE_HXX
#define INCLUDED_SLIDESHOW_VIEWAPPLETSHAPE_HXX

#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/awt/Point.hpp>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "viewlayer.hxx"

namespace com { namespace sun { namespace star {
namespace frame {
    class XSynchronousFrameLoader;
    class XFrame;
}
namespace uno {
    class XComponentContext;
}
namespace drawing {
    class XShape;
}}}}

namespace slideshow
{
    namespace internal
    {
        /** This class is the viewable representation of a draw
            document's applet object, associated to a specific View

            The class is able to render the associated applet on View
            implementations.
         */
        class ViewAppletShape : private boost::noncopyable
        {
        public:
            /** Create a ViewAppletShape for the given View

                @param rViewLayer
                The associated View object.

                @param rxShape
                The associated Shape

                @param rServiceName
                The service name to use, when actually creating the
                viewer component

                @param pPropCopyTable
                Table of plain ASCII property names, to copy from
                xShape to applet.

                @param nNumPropEntries
                Number of property table entries (in pPropCopyTable)
             */
            ViewAppletShape( const ViewLayerSharedPtr&                         rViewLayer,
                             const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::drawing::XShape >&        rxShape,
                             const ::rtl::OUString&                            rServiceName,
                             const char**                                      pPropCopyTable,
                             sal_Size                                          nNumPropEntries,
                             const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XComponentContext >& rxContext );

            /** destroy the object
             */
            virtual ~ViewAppletShape();

            /** Query the associated view layer of this shape
             */
            ViewLayerSharedPtr getViewLayer() const;

            // animation methods
            //------------------------------------------------------------------

            /** Notify the ViewShape that an animation starts now

                This method enters animation mode on the associate
                target view. The shape can be animated in parallel on
                different views.

                @param rBounds
                The current applet shape bounds

                @return whether the mode change finished successfully.
             */
            bool startApplet( const ::basegfx::B2DRectangle& rBounds );

            /** Notify the ViewShape that it is no longer animated

                This methods ends animation mode on the associate
                target view
             */
            void endApplet();

            // render methods
            //------------------------------------------------------------------

            /** Render the ViewShape

                This method renders the ViewAppletShape on the associated view.

                @param rBounds
                The current applet shape bounds

                @return whether the rendering finished successfully.
            */
            bool render( const ::basegfx::B2DRectangle& rBounds ) const;

            /** Resize the ViewShape

                This method resizes the ViewAppletShape on the
                associated view. It does not render.

                @param rBounds
                The current applet shape bounds

                @return whether the resize finished successfully.
            */
            bool resize( const ::basegfx::B2DRectangle& rBounds ) const;

        private:

            ViewLayerSharedPtr                                    mpViewLayer;

            /// the actual viewer component for this applet
            ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XSynchronousFrameLoader> mxViewer;

            /// the frame containing the applet
            ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XFrame>                  mxFrame;
            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext>         mxComponentContext;
        };

        typedef ::boost::shared_ptr< ViewAppletShape > ViewAppletShapeSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWAPPLETSHAPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
