/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewmediashape.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:55:39 $
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

#ifndef INCLUDED_SLIDESHOW_VIEWMEDIASHAPE_HXX
#define INCLUDED_SLIDESHOW_VIEWMEDIASHAPE_HXX

#include <basegfx/range/b2drectangle.hxx>
#include <com/sun/star/awt/Point.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "viewlayer.hxx"

class JavaChildWindow;

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
            ::std::auto_ptr< JavaChildWindow >    mpMediaWindow;
            mutable ::com::sun::star::awt::Point  maWindowOffset;
            mutable ::basegfx::B2DRectangle       maBounds;

            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       mxShape;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >        mxPlayer;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >  mxPlayerWindow;
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> mxComponentContext;
        };

        typedef ::boost::shared_ptr< ViewMediaShape > ViewMediaShapeSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWMEDIASHAPE_HXX */
