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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWMEDIASHAPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWMEDIASHAPE_HXX

#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/awt/Point.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vcl/vclptr.hxx>

#include "viewlayer.hxx"

class SystemChildWindow;
namespace vcl { class Window; }

namespace com { namespace sun { namespace star { namespace drawing {
    class XShape;
}
namespace media {
    class XPlayer;
    class XPlayerWindow;
}
namespace uno {
    class XComponentContext;
}
namespace beans{
    class XPropertySet;
} } } }

namespace slideshow
{
    namespace internal
    {
        /** This class is the viewable representation of a draw
            document's media object, associated to a specific View

            The class is able to render the associated media shape on
            View implementations.
         */
        class ViewMediaShape : private boost::noncopyable
        {
        public:
            /** Create a ViewMediaShape for the given View

                @param rView
                The associated View object.
             */
            ViewMediaShape( const ViewLayerSharedPtr&                          rViewLayer,
                            const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::drawing::XShape >&         rxShape,
                             const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XComponentContext >& rxContext );

            /** destroy the object
             */
            virtual ~ViewMediaShape();

            /** Query the associated view layer of this shape
             */
            ViewLayerSharedPtr getViewLayer() const;

            // animation methods


            /** Notify the ViewShape that an animation starts now

                This method enters animation mode on the associate
                target view. The shape can be animated in parallel on
                different views.

                @return whether the mode change finished successfully.
             */
            bool startMedia();

            /** Notify the ViewShape that it is no longer animated

                This methods ends animation mode on the associate
                target view
             */
            void endMedia();

            /** Notify the ViewShape that it should pause playback

                This methods pauses animation on the associate
                target view. The content stays visible (for video)
             */
            void pauseMedia();

            /** Set current time of media.

            @param fTime
            Local media time that should now be presented, in seconds.
             */
            void setMediaTime(double fTime);

            // render methods


            /** Render the ViewShape

                This method renders the ViewMediaShape on the associated view.

                @param rBounds
                The current media shape bounds

                @return whether the rendering finished successfully.
            */
            bool render( const ::basegfx::B2DRectangle& rBounds ) const;

            /** Resize the ViewShape

                This method updates the ViewMediaShape size on the
                associated view. It does not render.

                @param rBounds
                The current media shape bounds

                @return whether the resize finished successfully.
            */
            bool resize( const ::basegfx::B2DRectangle& rNewBounds ) const;

        private:

            bool implInitialize( const ::basegfx::B2DRectangle& rBounds );
            void implSetMediaProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxProps );
            void implInitializeMediaPlayer( const OUString& rMediaURL, const OUString& rMimeType );
            bool implInitializePlayerWindow( const ::basegfx::B2DRectangle& rBounds,
                                                     const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rVCLDeviceParams,
                                                     const OUString& rMimeType );
            ViewLayerSharedPtr                    mpViewLayer;
            VclPtr< SystemChildWindow >           mpMediaWindow;
            VclPtr< vcl::Window >                 mpEventHandlerParent;
            mutable ::com::sun::star::awt::Point  maWindowOffset;
            mutable ::basegfx::B2DRectangle       maBounds;

            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       mxShape;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >        mxPlayer;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >  mxPlayerWindow;
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> mxComponentContext;
            bool mbIsSoundEnabled;
        };

        typedef ::boost::shared_ptr< ViewMediaShape > ViewMediaShapeSharedPtr;

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWMEDIASHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
