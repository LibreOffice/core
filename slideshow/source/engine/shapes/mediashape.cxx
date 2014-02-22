/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <com/sun/star/drawing/XShape.hpp>

#include "mediashape.hxx"
#include "viewmediashape.hxx"
#include "externalshapebase.hxx"
#include "slideshowcontext.hxx"
#include "shape.hxx"
#include "tools.hxx"

#include <boost/bind.hpp>
#include <algorithm>


using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
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
            MediaShape( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::drawing::XShape >& xShape,
                        double                                      nPrio,
                        const SlideShowContext&                     rContext ); 

        private:

            
            

            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer );
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer );
            virtual bool clearAllViewLayers();


            
            

            virtual bool implRender( const ::basegfx::B2DRange& rCurrBounds ) const;
            virtual void implViewChanged( const UnoViewSharedPtr& rView );
            virtual void implViewsChanged();
            virtual bool implStartIntrinsicAnimation();
            virtual bool implEndIntrinsicAnimation();
            virtual bool implPauseIntrinsicAnimation();
            virtual bool implIsIntrinsicAnimationPlaying() const;
            virtual void implSetIntrinsicAnimationTime(double);

            
            typedef ::std::vector< ViewMediaShapeSharedPtr > ViewMediaShapeVector;
            ViewMediaShapeVector                             maViewMediaShapes;
            bool                                             mbIsPlaying;
        };


        MediaShape::MediaShape( const uno::Reference< drawing::XShape >& xShape,
                                  double                                     nPrio,
                                const SlideShowContext&                  rContext ) :
            ExternalShapeBase( xShape, nPrio, rContext ),
            maViewMediaShapes(),
            mbIsPlaying(false)
        {
        }

        

        void MediaShape::implViewChanged( const UnoViewSharedPtr& rView )
        {
            
            ViewMediaShapeVector::const_iterator       aIter(maViewMediaShapes.begin());
            ViewMediaShapeVector::const_iterator const aEnd (maViewMediaShapes.end());
            while( aIter != aEnd )
            {
                if( (*aIter)->getViewLayer()->isOnView(rView) )
                    (*aIter)->resize(getBounds());

                ++aIter;
            }
        }

        

        void MediaShape::implViewsChanged()
        {
            
            ::std::for_each( maViewMediaShapes.begin(),
                             maViewMediaShapes.end(),
                             ::boost::bind(
                                 &ViewMediaShape::resize,
                                 _1,
                                 ::boost::cref( getBounds())) );
        }

        

        void MediaShape::addViewLayer( const ViewLayerSharedPtr& rNewLayer,
                                       bool                      bRedrawLayer )
        {
            maViewMediaShapes.push_back(
                ViewMediaShapeSharedPtr( new ViewMediaShape( rNewLayer,
                                                             getXShape(),
                                                             mxComponentContext )));

            
            maViewMediaShapes.back()->resize( getBounds() );

            
            if( bRedrawLayer )
                maViewMediaShapes.back()->render( getBounds() );
        }

        

        bool MediaShape::removeViewLayer( const ViewLayerSharedPtr& rLayer )
        {
            const ViewMediaShapeVector::iterator aEnd( maViewMediaShapes.end() );

            OSL_ENSURE( ::std::count_if(maViewMediaShapes.begin(),
                                        aEnd,
                                        ::boost::bind<bool>(
                                            ::std::equal_to< ViewLayerSharedPtr >(),
                                            ::boost::bind( &ViewMediaShape::getViewLayer, _1 ),
                                            ::boost::cref( rLayer ) ) ) < 2,
                        "MediaShape::removeViewLayer(): Duplicate ViewLayer entries!" );

            ViewMediaShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewMediaShapes.begin(),
                                         aEnd,
                                         ::boost::bind<bool>(
                                             ::std::equal_to< ViewLayerSharedPtr >(),
                                             ::boost::bind( &ViewMediaShape::getViewLayer,
                                                            _1 ),
                                             ::boost::cref( rLayer ) ) )) == aEnd )
            {
                
                return false;
            }

            
            maViewMediaShapes.erase( aIter, aEnd );

            return true;
        }

        

        bool MediaShape::clearAllViewLayers()
        {
            maViewMediaShapes.clear();
            return true;
        }

        

        bool MediaShape::implRender( const ::basegfx::B2DRange& rCurrBounds ) const
        {
            
            if( ::std::count_if( maViewMediaShapes.begin(),
                                 maViewMediaShapes.end(),
                                 ::boost::bind<bool>(
                                     ::boost::mem_fn( &ViewMediaShape::render ),
                                     _1,
                                     ::boost::cref( rCurrBounds ) ) )
                != static_cast<ViewMediaShapeVector::difference_type>(maViewMediaShapes.size()) )
            {
                
                
                return false;
            }

            return true;
        }

        

        bool MediaShape::implStartIntrinsicAnimation()
        {
            ::std::for_each( maViewMediaShapes.begin(),
                             maViewMediaShapes.end(),
                             ::boost::mem_fn( &ViewMediaShape::startMedia ) );

            mbIsPlaying = true;

            return true;
        }

        

        bool MediaShape::implEndIntrinsicAnimation()
        {
            ::std::for_each( maViewMediaShapes.begin(),
                             maViewMediaShapes.end(),
                             ::boost::mem_fn( &ViewMediaShape::endMedia ) );

            mbIsPlaying = false;

            return true;
        }

        

        bool MediaShape::implPauseIntrinsicAnimation()
        {
            ::std::for_each( maViewMediaShapes.begin(),
                             maViewMediaShapes.end(),
                             ::boost::mem_fn( &ViewMediaShape::pauseMedia ) );

            mbIsPlaying = false;

            return true;
        }

        

        bool MediaShape::implIsIntrinsicAnimationPlaying() const
        {
            return mbIsPlaying;
        }

        

        void MediaShape::implSetIntrinsicAnimationTime(double fTime)
        {
            ::std::for_each( maViewMediaShapes.begin(),
                             maViewMediaShapes.end(),
                             ::boost::bind( &ViewMediaShape::setMediaTime,
                                            _1, boost::cref(fTime)) );
        }

        

        ShapeSharedPtr createMediaShape(
            const uno::Reference< drawing::XShape >& xShape,
            double                                   nPrio,
            const SlideShowContext&                  rContext)
        {
            boost::shared_ptr< MediaShape > pMediaShape(
                new MediaShape(xShape, nPrio, rContext));

            return pMediaShape;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
