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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <boost/shared_ptr.hpp>

#include "appletshape.hxx"
#include "externalshapebase.hxx"
#include "vieweventhandler.hxx"
#include "viewappletshape.hxx"
#include "tools.hxx"

#include <boost/bind.hpp>
#include <algorithm>


using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        /** Represents an applet shape.

            This implementation offers support for applet shapes (both
            Java applets, and Netscape plugins). Such shapes need
            special treatment.
         */
        class AppletShape : public ExternalShapeBase
        {
        public:
            /** Create a shape for the given XShape for a applet object

                @param xShape
                The XShape to represent.

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!

                @param rServiceName
                Service name to use, when creating the actual viewer
                component

                @param pPropCopyTable
                Table of plain ASCII property names, to copy from
                xShape to applet.

                @param nNumPropEntries
                Number of property table entries (in pPropCopyTable)
             */
            AppletShape( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::drawing::XShape >& xShape,
                         double                                     nPrio,
                         const ::rtl::OUString&                     rServiceName,
                         const char**                               pPropCopyTable,
                         sal_Size                                   nNumPropEntries,
                         const SlideShowContext&                    rContext ); // throw ShapeLoadFailedException;

        private:

            // View layer methods
            //------------------------------------------------------------------

            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer );
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer );
            virtual bool clearAllViewLayers();


            // ExternalShapeBase methods
            //------------------------------------------------------------------

            virtual bool implRender( const ::basegfx::B2DRange& rCurrBounds ) const;
            virtual void implViewChanged( const UnoViewSharedPtr& rView );
            virtual void implViewsChanged();
            virtual bool implStartIntrinsicAnimation();
            virtual bool implEndIntrinsicAnimation();
            virtual bool implPauseIntrinsicAnimation();
            virtual bool implIsIntrinsicAnimationPlaying() const;
            virtual void implSetIntrinsicAnimationTime(double);

            const ::rtl::OUString                           maServiceName;
            const char**                                    mpPropCopyTable;
            const sal_Size                                  mnNumPropEntries;

            /// the list of active view shapes (one for each registered view layer)
            typedef ::std::vector< ViewAppletShapeSharedPtr > ViewAppletShapeVector;
            ViewAppletShapeVector                           maViewAppletShapes;
            bool                                             mbIsPlaying;
        };

        AppletShape::AppletShape( const uno::Reference< drawing::XShape >& xShape,
                                  double                                   nPrio,
                                  const ::rtl::OUString&                   rServiceName,
                                  const char**                             pPropCopyTable,
                                  sal_Size                                 nNumPropEntries,
                                  const SlideShowContext&                  rContext ) :
            ExternalShapeBase( xShape, nPrio, rContext ),
            maServiceName( rServiceName ),
            mpPropCopyTable( pPropCopyTable ),
            mnNumPropEntries( nNumPropEntries ),
            maViewAppletShapes(),
            mbIsPlaying(false)
        {
        }

        // ---------------------------------------------------------------------

        void AppletShape::implViewChanged( const UnoViewSharedPtr& rView )
        {
            // determine ViewAppletShape that needs update
            ViewAppletShapeVector::const_iterator       aIter(maViewAppletShapes.begin());
            ViewAppletShapeVector::const_iterator const aEnd (maViewAppletShapes.end());
            while( aIter != aEnd )
            {
                if( (*aIter)->getViewLayer()->isOnView(rView) )
                    (*aIter)->resize(getBounds());

                ++aIter;
            }
        }

        // ---------------------------------------------------------------------

        void AppletShape::implViewsChanged()
        {
            // resize all ViewShapes
            ::std::for_each( maViewAppletShapes.begin(),
                             maViewAppletShapes.end(),
                             ::boost::bind(
                                 &ViewAppletShape::resize,
                                 _1,
                                 ::boost::cref( AppletShape::getBounds())) );
        }

        // ---------------------------------------------------------------------

        void AppletShape::addViewLayer( const ViewLayerSharedPtr& rNewLayer,
                                        bool                      bRedrawLayer )
        {
            try
            {
                maViewAppletShapes.push_back(
                    ViewAppletShapeSharedPtr( new ViewAppletShape( rNewLayer,
                                                                   getXShape(),
                                                                   maServiceName,
                                                                   mpPropCopyTable,
                                                                   mnNumPropEntries,
                                                                   mxComponentContext )));

                // push new size to view shape
                maViewAppletShapes.back()->resize( getBounds() );

                // render the Shape on the newly added ViewLayer
                if( bRedrawLayer )
                    maViewAppletShapes.back()->render( getBounds() );
            }
            catch(uno::Exception&)
            {
                // ignore failed shapes - slideshow should run with
                // the remaining content
            }
        }

        // ---------------------------------------------------------------------

        bool AppletShape::removeViewLayer( const ViewLayerSharedPtr& rLayer )
        {
            const ViewAppletShapeVector::iterator aEnd( maViewAppletShapes.end() );

            OSL_ENSURE( ::std::count_if(maViewAppletShapes.begin(),
                                        aEnd,
                                        ::boost::bind<bool>(
                                            ::std::equal_to< ViewLayerSharedPtr >(),
                                            ::boost::bind( &ViewAppletShape::getViewLayer, _1 ),
                                            ::boost::cref( rLayer ) ) ) < 2,
                        "AppletShape::removeViewLayer(): Duplicate ViewLayer entries!" );

            ViewAppletShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewAppletShapes.begin(),
                                         aEnd,
                                         ::boost::bind<bool>(
                                             ::std::equal_to< ViewLayerSharedPtr >(),
                                             ::boost::bind( &ViewAppletShape::getViewLayer,
                                                            _1 ),
                                             ::boost::cref( rLayer ) ) )) == aEnd )
            {
                // view layer seemingly was not added, failed
                return false;
            }

            // actually erase from container
            maViewAppletShapes.erase( aIter, aEnd );

            return true;
        }

        // ---------------------------------------------------------------------

        bool AppletShape::clearAllViewLayers()
        {
            maViewAppletShapes.clear();
            return true;
        }

        // ---------------------------------------------------------------------

        bool AppletShape::implRender( const ::basegfx::B2DRange& rCurrBounds ) const
        {
            // redraw all view shapes, by calling their update() method
            if( ::std::count_if( maViewAppletShapes.begin(),
                                 maViewAppletShapes.end(),
                                 ::boost::bind<bool>(
                                     ::boost::mem_fn( &ViewAppletShape::render ),
                                     _1,
                                     ::boost::cref( rCurrBounds ) ) )
                != static_cast<ViewAppletShapeVector::difference_type>(maViewAppletShapes.size()) )
            {
                // at least one of the ViewShape::update() calls did return
                // false - update failed on at least one ViewLayer
                return false;
            }

            return true;
        }

        // ---------------------------------------------------------------------

        bool AppletShape::implStartIntrinsicAnimation()
        {
            ::std::for_each( maViewAppletShapes.begin(),
                             maViewAppletShapes.end(),
                             ::boost::bind( &ViewAppletShape::startApplet,
                                            _1,
                                            ::boost::cref( getBounds() )));
            mbIsPlaying = true;

            return true;
        }

        // ---------------------------------------------------------------------

        bool AppletShape::implEndIntrinsicAnimation()
        {
            ::std::for_each( maViewAppletShapes.begin(),
                             maViewAppletShapes.end(),
                             ::boost::mem_fn( &ViewAppletShape::endApplet ) );

            mbIsPlaying = false;

            return true;
        }

        // ---------------------------------------------------------------------

        bool AppletShape::implPauseIntrinsicAnimation()
        {
            // TODO(F1): any way of temporarily disabling/deactivating
            // applets?
            return true;
        }

        // ---------------------------------------------------------------------

        bool AppletShape::implIsIntrinsicAnimationPlaying() const
        {
            return mbIsPlaying;
        }

        // ---------------------------------------------------------------------

        void AppletShape::implSetIntrinsicAnimationTime(double)
        {
            // No way of doing this, or?
        }

        boost::shared_ptr<Shape> createAppletShape(
            const uno::Reference< drawing::XShape >& xShape,
            double                                   nPrio,
            const ::rtl::OUString&                   rServiceName,
            const char**                             pPropCopyTable,
            sal_Size                                 nNumPropEntries,
            const SlideShowContext&                  rContext )
        {
            boost::shared_ptr< AppletShape > pAppletShape(
                new AppletShape(xShape,
                                nPrio,
                                rServiceName,
                                pPropCopyTable,
                                nNumPropEntries,
                                rContext) );

            return pAppletShape;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
