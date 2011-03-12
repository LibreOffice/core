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

#ifndef INCLUDED_SLIDESHOW_VIEWMEDIASHAPE_HXX
#define INCLUDED_SLIDESHOW_VIEWMEDIASHAPE_HXX

#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/awt/Point.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "viewlayer.hxx"

class SystemChildWindow;

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
            //------------------------------------------------------------------

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
            //------------------------------------------------------------------

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
            void implInitializeMediaPlayer( const ::rtl::OUString& rMediaURL );
            bool implInitializeVCLBasedPlayerWindow( const ::basegfx::B2DRectangle& rBounds,
                                                     const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rVCLDeviceParams );
            bool implInitializeDXBasedPlayerWindow( const ::basegfx::B2DRectangle& rBounds,
                                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rDXDeviceParams );

            ViewLayerSharedPtr                    mpViewLayer;
            ::std::auto_ptr< SystemChildWindow >    mpMediaWindow;
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

#endif /* INCLUDED_SLIDESHOW_VIEWMEDIASHAPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
