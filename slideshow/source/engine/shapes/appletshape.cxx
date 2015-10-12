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


#include <canvas/canvastools.hxx>

#include <boost/shared_ptr.hpp>

#include "appletshape.hxx"
#include "externalshapebase.hxx"
#include "vieweventhandler.hxx"
#include "viewappletshape.hxx"
#include "tools.hxx"

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
                         const OUString&                     rServiceName,
                         const char**                               pPropCopyTable,
                         sal_Size                                   nNumPropEntries,
                         const SlideShowContext&                    rContext ); // throw ShapeLoadFailedException;

        private:

            // View layer methods


            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer ) override;
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer ) override;
            virtual bool clearAllViewLayers() override;


            // ExternalShapeBase methods


            virtual bool implRender( const ::basegfx::B2DRange& rCurrBounds ) const override;
            virtual void implViewChanged( const UnoViewSharedPtr& rView ) override;
            virtual void implViewsChanged() override;
            virtual bool implStartIntrinsicAnimation() override;
            virtual bool implEndIntrinsicAnimation() override;
            virtual bool implPauseIntrinsicAnimation() override;
            virtual bool implIsIntrinsicAnimationPlaying() const override;
            virtual void implSetIntrinsicAnimationTime(double) override;

            const OUString                           maServiceName;
            const char**                                    mpPropCopyTable;
            const sal_Size                                  mnNumPropEntries;

            /// the list of active view shapes (one for each registered view layer)
            typedef ::std::vector< ViewAppletShapeSharedPtr > ViewAppletShapeVector;
            ViewAppletShapeVector                           maViewAppletShapes;
            bool                                             mbIsPlaying;
        };

        AppletShape::AppletShape( const uno::Reference< drawing::XShape >& xShape,
                                  double                                   nPrio,
                                  const OUString&                   rServiceName,
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



        void AppletShape::implViewChanged( const UnoViewSharedPtr& rView )
        {
            const ::basegfx::B2DRectangle& rBounds = getBounds();
            // determine ViewAppletShape that needs update
            for( const auto& pViewAppletShape : maViewAppletShapes )
            {
                if( pViewAppletShape->getViewLayer()->isOnView( rView ) )
                    pViewAppletShape->resize( rBounds );
            }
        }



        void AppletShape::implViewsChanged()
        {
            // resize all ViewShapes
            const ::basegfx::B2DRectangle& rBounds = getBounds();
            for( const auto& pViewAppletShape : maViewAppletShapes )
                pViewAppletShape->resize( rBounds );
        }



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



        bool AppletShape::removeViewLayer( const ViewLayerSharedPtr& rLayer )
        {
            const ViewAppletShapeVector::iterator aEnd( maViewAppletShapes.end() );

            OSL_ENSURE( ::std::count_if(maViewAppletShapes.begin(),
                                        aEnd,
                                        [&rLayer]
                                        ( const ViewAppletShapeSharedPtr& pShape )
                                        { return rLayer == pShape->getViewLayer(); } ) < 2,
                        "AppletShape::removeViewLayer(): Duplicate ViewLayer entries!" );

            ViewAppletShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewAppletShapes.begin(),
                                         aEnd,
                                         [&rLayer]
                                         ( const ViewAppletShapeSharedPtr& pShape )
                                         { return rLayer == pShape->getViewLayer(); } ) ) == aEnd )
            {
                // view layer seemingly was not added, failed
                return false;
            }

            // actually erase from container
            maViewAppletShapes.erase( aIter, aEnd );

            return true;
        }



        bool AppletShape::clearAllViewLayers()
        {
            maViewAppletShapes.clear();
            return true;
        }



        bool AppletShape::implRender( const ::basegfx::B2DRange& rCurrBounds ) const
        {
            // redraw all view shapes, by calling their update() method
            if( ::std::count_if( maViewAppletShapes.begin(),
                                 maViewAppletShapes.end(),
                                 [&rCurrBounds]
                                 ( const ViewAppletShapeSharedPtr& pShape )
                                 { return pShape->render( rCurrBounds ); } )
                != static_cast<ViewAppletShapeVector::difference_type>(maViewAppletShapes.size()) )
            {
                // at least one of the ViewShape::update() calls did return
                // false - update failed on at least one ViewLayer
                return false;
            }

            return true;
        }



        bool AppletShape::implStartIntrinsicAnimation()
        {
            const ::basegfx::B2DRectangle& rBounds = getBounds();
            for( const auto& pViewAppletShape : maViewAppletShapes )
                pViewAppletShape->startApplet( rBounds );

            mbIsPlaying = true;

            return true;
        }



        bool AppletShape::implEndIntrinsicAnimation()
        {
            for( const auto& pViewAppletShape : maViewAppletShapes )
                pViewAppletShape->endApplet();

            mbIsPlaying = false;

            return true;
        }



        bool AppletShape::implPauseIntrinsicAnimation()
        {
            // TODO(F1): any way of temporarily disabling/deactivating
            // applets?
            return true;
        }



        bool AppletShape::implIsIntrinsicAnimationPlaying() const
        {
            return mbIsPlaying;
        }



        void AppletShape::implSetIntrinsicAnimationTime(double)
        {
            // No way of doing this, or?
        }

        boost::shared_ptr<Shape> createAppletShape(
            const uno::Reference< drawing::XShape >& xShape,
            double                                   nPrio,
            const OUString&                   rServiceName,
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
