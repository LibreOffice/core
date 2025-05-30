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


#include <com/sun/star/drawing/XShape.hpp>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>

#include "mediashape.hxx"
#include "viewmediashape.hxx"
#include "externalshapebase.hxx"
#include <slideshowcontext.hxx>
#include <shape.hxx>
#include <tools.hxx>

#include <algorithm>


using namespace ::com::sun::star;


namespace slideshow::internal
{
        namespace {

        /** Represents a media shape.

            This implementation offers support for media shapes.
            Such shapes need special treatment.
         */
        class MediaShape : public ExternalShapeBase
        {
        public:
            /** Create a shape for the given XShape for a media object

                @param xShape
                The XShape to represent.

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!
             */
            MediaShape( const css::uno::Reference< css::drawing::XShape >& xShape,
                        double                                      nPrio,
                        const SlideShowContext&                     rContext ); // throw ShapeLoadFailedException;

        private:

            // View layer methods


            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer ) override;
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer ) override;
            virtual void clearAllViewLayers() override;


            // ExternalShapeBase methods


            virtual bool implRender( const ::basegfx::B2DRange& rCurrBounds ) const override;
            virtual void implViewChanged( const UnoViewSharedPtr& rView ) override;
            virtual void implViewsChanged() override;
            virtual bool implStartIntrinsicAnimation() override;
            virtual bool implEndIntrinsicAnimation() override;
            virtual void implPauseIntrinsicAnimation() override;
            virtual bool implIsIntrinsicAnimationPlaying() const override;
            virtual void implSetIntrinsicAnimationTime(double) override;
            void implSetLooping(bool bLooping) override;

            /// the list of active view shapes (one for each registered view layer)
            typedef ::std::vector< ViewMediaShapeSharedPtr > ViewMediaShapeVector;
            ViewMediaShapeVector                             maViewMediaShapes;
            bool                                             mbIsPlaying;
            OUString                                         maFallbackDir;
        };

        }

        MediaShape::MediaShape( const uno::Reference< drawing::XShape >& xShape,
                                  double                                     nPrio,
                                const SlideShowContext&                  rContext ) :
            ExternalShapeBase( xShape, nPrio, rContext ),
            maViewMediaShapes(),
            mbIsPlaying(false),
            maFallbackDir(rContext.maFallbackDir)
        {
        }


        void MediaShape::implViewChanged( const UnoViewSharedPtr& rView )
        {
            const ::basegfx::B2DRectangle aBounds = getBounds();
            // determine ViewMediaShape that needs update
            for( const auto& pViewMediaShape : maViewMediaShapes )
                if( pViewMediaShape->getViewLayer()->isOnView( rView ) )
                    pViewMediaShape->resize( aBounds );
        }


        void MediaShape::implViewsChanged()
        {
            const ::basegfx::B2DRectangle aBounds = getBounds();
            // resize all ViewShapes
            for( const auto& pViewMediaShape : maViewMediaShapes )
                pViewMediaShape->resize( aBounds );
        }


        void MediaShape::addViewLayer( const ViewLayerSharedPtr& rNewLayer,
                                       bool                      bRedrawLayer )
        {
            maViewMediaShapes.push_back(
                std::make_shared<ViewMediaShape>( rNewLayer,
                                                  getXShape(),
                                                  mxComponentContext,
                                                  maFallbackDir ));

            // push new size to view shape
            maViewMediaShapes.back()->resize( getBounds() );

            // render the Shape on the newly added ViewLayer
            if( bRedrawLayer )
                maViewMediaShapes.back()->render( getBounds() );
        }


        bool MediaShape::removeViewLayer( const ViewLayerSharedPtr& rLayer )
        {
            const ViewMediaShapeVector::iterator aEnd( maViewMediaShapes.end() );

            OSL_ENSURE( ::std::count_if(maViewMediaShapes.begin(),
                                        aEnd,
                                        [&rLayer]
                                        ( const ViewMediaShapeSharedPtr& pShape )
                                        { return rLayer == pShape->getViewLayer(); } ) < 2,
                        "MediaShape::removeViewLayer(): Duplicate ViewLayer entries!" );

// TODO : needed for the moment since ANDROID doesn't know size_t return from std::erase_if
#if defined ANDROID
            ViewMediaShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewMediaShapes.begin(),
                                         aEnd,
                                         [&rLayer]
                                         ( const ViewMediaShapeSharedPtr& pShape )
                                         { return rLayer == pShape->getViewLayer(); } ) ) == aEnd )
            {
                // view layer seemingly was not added, failed
                return false;
            }

            // actually erase from container
            maViewMediaShapes.erase( aIter, aEnd );

            return true;
#else
            size_t nb = std::erase_if(maViewMediaShapes,
                                         [&rLayer]
                                         ( const ViewMediaShapeSharedPtr& pShape )
                                         { return rLayer == pShape->getViewLayer(); } );
            // if nb == 0, it means view media shape seemingly was not added, failed
            return (nb != 0);
#endif
        }


        void MediaShape::clearAllViewLayers()
        {
            maViewMediaShapes.clear();
        }


        bool MediaShape::implRender( const ::basegfx::B2DRange& rCurrBounds ) const
        {
            // redraw all view shapes, by calling their update() method
            if( o3tl::make_unsigned(::std::count_if( maViewMediaShapes.begin(),
                                 maViewMediaShapes.end(),
                                 [&rCurrBounds]
                                 ( const ViewMediaShapeSharedPtr& pShape )
                                 { return pShape->render( rCurrBounds ); } ))
                != maViewMediaShapes.size() )
            {
                // at least one of the ViewShape::update() calls did return
                // false - update failed on at least one ViewLayer
                return false;
            }

            return true;
        }


        bool MediaShape::implStartIntrinsicAnimation()
        {
            for( const auto& pViewMediaShape : maViewMediaShapes )
                pViewMediaShape->startMedia();

            mbIsPlaying = true;

            return true;
        }


        bool MediaShape::implEndIntrinsicAnimation()
        {
            for( const auto& pViewMediaShape : maViewMediaShapes )
                pViewMediaShape->endMedia();

            mbIsPlaying = false;

            return true;
        }


        void MediaShape::implPauseIntrinsicAnimation()
        {
            for( const auto& pViewMediaShape : maViewMediaShapes )
                pViewMediaShape->pauseMedia();

            mbIsPlaying = false;
        }


        bool MediaShape::implIsIntrinsicAnimationPlaying() const
        {
            return mbIsPlaying;
        }


        void MediaShape::implSetIntrinsicAnimationTime(double fTime)
        {
            for( const auto& pViewMediaShape : maViewMediaShapes )
                pViewMediaShape->setMediaTime( fTime );
        }

        void MediaShape::implSetLooping(bool bLooping)
        {
            for (const auto& pViewMediaShape : maViewMediaShapes)
            {
                pViewMediaShape->setLooping(bLooping);
            }
        }

        ShapeSharedPtr createMediaShape(
            const uno::Reference< drawing::XShape >& xShape,
            double                                   nPrio,
            const SlideShowContext&                  rContext)
        {
            return std::make_shared<MediaShape>(xShape, nPrio, rContext);
        }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
