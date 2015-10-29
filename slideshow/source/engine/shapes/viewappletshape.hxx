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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWAPPLETSHAPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWAPPLETSHAPE_HXX

#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/awt/Point.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "viewlayer.hxx"

namespace com { namespace sun { namespace star {
namespace frame {
    class XSynchronousFrameLoader;
    class XFrame2;
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
                             const css::uno::Reference< css::drawing::XShape >&        rxShape,
                             const OUString&                                   rServiceName,
                             const char**                                      pPropCopyTable,
                             sal_Size                                          nNumPropEntries,
                             const css::uno::Reference< css::uno::XComponentContext >& rxContext );

            /** destroy the object
             */
            virtual ~ViewAppletShape();

            /** Query the associated view layer of this shape
             */
            ViewLayerSharedPtr getViewLayer() const;

            // animation methods


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
            css::uno::Reference<
                css::frame::XSynchronousFrameLoader> mxViewer;

            /// the frame containing the applet
            css::uno::Reference<
                css::frame::XFrame2>                 mxFrame;
            css::uno::Reference<
                css::uno::XComponentContext>         mxComponentContext;
        };

        typedef ::boost::shared_ptr< ViewAppletShape > ViewAppletShapeSharedPtr;

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWAPPLETSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
