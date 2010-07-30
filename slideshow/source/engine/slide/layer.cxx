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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

#include "layer.hxx"

#include <boost/bind.hpp>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        Layer::Layer( const basegfx::B2DRange& rMaxLayerBounds,
                      Dummy                     ) :
            maViewEntries(),
            maBounds(),
            maNewBounds(),
            maMaxBounds( rMaxLayerBounds ),
            mbBoundsDirty(false),
            mbBackgroundLayer(true),
            mbClipSet(false)
        {
        }

        Layer::Layer( const basegfx::B2DRange& rMaxLayerBounds ) :
            maViewEntries(),
            maBounds(),
            maNewBounds(),
            maMaxBounds( rMaxLayerBounds ),
            mbBoundsDirty(false),
            mbBackgroundLayer(false),
            mbClipSet(false)
        {
        }

        ViewLayerSharedPtr Layer::addView( const ViewSharedPtr& rNewView )
        {
            OSL_ASSERT( rNewView );

            ViewEntryVector::iterator aIter;
            const ViewEntryVector::iterator aEnd( maViewEntries.end() );
            if( (aIter=std::find_if( maViewEntries.begin(),
                                     aEnd,
                                     boost::bind<bool>(
                                         std::equal_to< ViewSharedPtr >(),
                                         boost::bind( &ViewEntry::getView, _1 ),
                                         boost::cref( rNewView )))) != aEnd )
            {
                // already added - just return existing layer
                return aIter->mpViewLayer;

            }

            // not yet added - create new view layer
            ViewLayerSharedPtr pNewLayer;
            if( mbBackgroundLayer )
                pNewLayer = rNewView;
            else
                pNewLayer = rNewView->createViewLayer(maBounds);

            // add to local list
            maViewEntries.push_back(
                ViewEntry( rNewView,
                           pNewLayer ));

            return maViewEntries.back().mpViewLayer;
        }

        ViewLayerSharedPtr Layer::removeView( const ViewSharedPtr& rView )
        {
            OSL_ASSERT( rView );

            ViewEntryVector::iterator       aIter;
            const ViewEntryVector::iterator aEnd( maViewEntries.end() );
            if( (aIter=std::find_if( maViewEntries.begin(),
                                       aEnd,
                                       boost::bind<bool>(
                                           std::equal_to< ViewSharedPtr >(),
                                           boost::bind( &ViewEntry::getView, _1 ),
                                           boost::cref( rView )))) == aEnd )
            {
                // View was not added/is already removed
                return ViewLayerSharedPtr();
            }

            OSL_ENSURE( std::count_if( maViewEntries.begin(),
                                       aEnd,
                                       boost::bind<bool>(
                                           std::equal_to< ViewSharedPtr >(),
                                           boost::bind( &ViewEntry::getView, _1 ),
                                           boost::cref( rView ))) == 1,
                        "Layer::removeView(): view added multiple times" );

            ViewLayerSharedPtr pRet( aIter->mpViewLayer );
            maViewEntries.erase(aIter);

            return pRet;
        }

        void Layer::viewChanged( const ViewSharedPtr& rChangedView )
        {
            ViewEntryVector::iterator aIter;
            const ViewEntryVector::iterator aEnd( maViewEntries.end() );
            if( (aIter=std::find_if( maViewEntries.begin(),
                                     aEnd,
                                     boost::bind<bool>(
                                         std::equal_to< ViewSharedPtr >(),
                                         boost::bind( &ViewEntry::getView, _1 ),
                                         boost::cref( rChangedView )))) !=
                aEnd )
            {
                // adapt size of given ViewLayer - background layer
                // resizes with view.
                if( !mbBackgroundLayer )
                    aIter->mpViewLayer->resize(maBounds);
            }
        }

        void Layer::viewsChanged()
        {
            // adapt size of given ViewLayer - background layer
            // resizes with view.
            if( !mbBackgroundLayer )
            {
                std::for_each( maViewEntries.begin(),
                               maViewEntries.end(),
                               boost::bind( &ViewLayer::resize,
                                            boost::bind( &ViewEntry::getViewLayer,
                                                         _1 ),
                                            boost::cref(maBounds)));
            }
        }

        void Layer::setShapeViews( ShapeSharedPtr const& rShape ) const
        {
            rShape->clearAllViewLayers();

            std::for_each( maViewEntries.begin(),
                           maViewEntries.end(),
                           boost::bind(&Shape::addViewLayer,
                                       boost::cref(rShape),
                                       boost::bind(&ViewEntry::getViewLayer,
                                                   _1),
                                       false ));
        }

        void Layer::setPriority( const ::basegfx::B1DRange& rPrioRange )
        {
            if( !mbBackgroundLayer )
            {
                std::for_each( maViewEntries.begin(),
                               maViewEntries.end(),
                               boost::bind( &ViewLayer::setPriority,
                                            boost::bind( &ViewEntry::getViewLayer,
                                                         _1 ),
                                            boost::cref(rPrioRange)));
            }
        }

        void Layer::addUpdateRange( ::basegfx::B2DRange const& rUpdateRange )
        {
            // TODO(Q1): move this to B2DMultiRange
            if( !rUpdateRange.isEmpty() )
                maUpdateAreas.appendElement( rUpdateRange,
                                             basegfx::ORIENTATION_POSITIVE );
        }

        void Layer::updateBounds( ShapeSharedPtr const& rShape )
        {
            if( !mbBackgroundLayer )
            {
                if( !mbBoundsDirty )
                    maNewBounds.reset();

                maNewBounds.expand( rShape->getUpdateArea() );
            }

            mbBoundsDirty = true;
        }

        bool Layer::commitBounds()
        {
            mbBoundsDirty = false;

            if( mbBackgroundLayer )
                return false;

            if( maNewBounds == maBounds )
                return false;

            maBounds = maNewBounds;
            if( std::count_if( maViewEntries.begin(),
                               maViewEntries.end(),
                               boost::bind( &ViewLayer::resize,
                                            boost::bind( &ViewEntry::getViewLayer,
                                                         _1 ),
                                            boost::cref(maBounds)) ) == 0 )
            {
                return false;
            }

            // layer content invalid, update areas have wrong
            // coordinates/not sensible anymore.
            clearUpdateRanges();

            return true;
        }

        void Layer::clearUpdateRanges()
        {
            maUpdateAreas.clear();
        }

        void Layer::clearContent()
        {
            // clear content on all view layers
            std::for_each( maViewEntries.begin(),
                           maViewEntries.end(),
                           boost::bind(
                               &ViewLayer::clear,
                               boost::bind(
                                   &ViewEntry::getViewLayer,
                                   _1)));

            // layer content cleared, update areas are not sensible
            // anymore.
            clearUpdateRanges();
        }

        class LayerEndUpdate : private boost::noncopyable
        {
        public:
            LayerEndUpdate( LayerSharedPtr const& rLayer ) :
                mpLayer( rLayer )
            {}

            ~LayerEndUpdate() { if(mpLayer) mpLayer->endUpdate(); }

            void dismiss() { mpLayer.reset(); }

        private:
            LayerSharedPtr mpLayer;
        };

        Layer::EndUpdater Layer::beginUpdate()
        {
            if( maUpdateAreas.count() )
            {
                // perform proper layer update. That means, setup proper
                // clipping, and render each shape that intersects with
                // the calculated update area
                ::basegfx::B2DPolyPolygon aClip( maUpdateAreas.solveCrossovers() );
                aClip = ::basegfx::tools::stripNeutralPolygons(aClip);
                aClip = ::basegfx::tools::stripDispensablePolygons(aClip, false);

                // actually, if there happen to be shapes with zero
                // update area in the maUpdateAreas vector, the
                // resulting clip polygon will be empty.
                if( aClip.count() )
                {
                    // set clip to all view layers
                    std::for_each( maViewEntries.begin(),
                                   maViewEntries.end(),
                                   boost::bind(
                                       &ViewLayer::setClip,
                                       boost::bind(
                                           &ViewEntry::getViewLayer,
                                           _1),
                                       boost::cref(aClip)));

                    // clear update area on all view layers
                    std::for_each( maViewEntries.begin(),
                                   maViewEntries.end(),
                                   boost::bind(
                                       &ViewLayer::clear,
                                       boost::bind(
                                           &ViewEntry::getViewLayer,
                                           _1)));

                    mbClipSet = true;
                }
            }

            return EndUpdater(new LayerEndUpdate(shared_from_this()));
        }

        void Layer::endUpdate()
        {
            if( mbClipSet )
            {
                mbClipSet = false;

                basegfx::B2DPolyPolygon aEmptyClip;
                std::for_each( maViewEntries.begin(),
                               maViewEntries.end(),
                               boost::bind(
                                   &ViewLayer::setClip,
                                   boost::bind(
                                       &ViewEntry::getViewLayer,
                                       _1),
                                   boost::cref(aEmptyClip)));
            }

            clearUpdateRanges();
        }

        bool Layer::isInsideUpdateArea( ShapeSharedPtr const& rShape ) const
        {
            return maUpdateAreas.overlaps( rShape->getUpdateArea() );
        }

        LayerSharedPtr Layer::createBackgroundLayer( const basegfx::B2DRange& rMaxLayerBounds )
        {
            return LayerSharedPtr(new Layer( rMaxLayerBounds,
                                             BackgroundLayer ));
        }

        LayerSharedPtr Layer::createLayer( const basegfx::B2DRange& rMaxLayerBounds )
        {
            return LayerSharedPtr( new Layer( rMaxLayerBounds ) );
        }

    }
}
