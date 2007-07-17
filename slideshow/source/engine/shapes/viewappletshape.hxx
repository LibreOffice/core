/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewappletshape.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:54:55 $
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
