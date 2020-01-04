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


#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <osl/diagnose.h>

#include "layer.hxx"

using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        Layer::Layer( Dummy                     ) :
            maViewEntries(),
            maBounds(),
            maNewBounds(),
            mbBoundsDirty(false),
            mbBackgroundLayer(true),
            mbClipSet(false)
        {
        }

        Layer::Layer() :
            maViewEntries(),
            maBounds(),
            maNewBounds(),
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
                                     [&rNewView]( const ViewEntry& rViewEntry )
                                     { return rViewEntry.getView() == rNewView; } ) ) != aEnd )
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
            maViewEntries.emplace_back( rNewView,
                           pNewLayer );

            return maViewEntries.back().mpViewLayer;
        }

        ViewLayerSharedPtr Layer::removeView( const ViewSharedPtr& rView )
        {
            OSL_ASSERT( rView );

            ViewEntryVector::iterator       aIter;
            const ViewEntryVector::iterator aEnd( maViewEntries.end() );
            if( (aIter=std::find_if( maViewEntries.begin(),
                                       aEnd,
                                       [&rView]( const ViewEntry& rViewEntry )
                                       { return rViewEntry.getView() == rView; } ) ) == aEnd )
            {
                // View was not added/is already removed
                return ViewLayerSharedPtr();
            }

            OSL_ENSURE( std::count_if( maViewEntries.begin(),
                                       aEnd,
                                       [&rView]( const ViewEntry& rViewEntry )
                                       { return rViewEntry.getView() == rView; } ) == 1,
                        "Layer::removeView(): view added multiple times" );

            ViewLayerSharedPtr pRet( aIter->mpViewLayer );
            maViewEntries.erase(aIter);

            return pRet;
        }

        void Layer::setShapeViews( ShapeSharedPtr const& rShape ) const
        {
            rShape->clearAllViewLayers();

            for( const auto& rViewEntry : maViewEntries )
                rShape->addViewLayer( rViewEntry.getViewLayer(), false );
        }

        void Layer::setPriority( const ::basegfx::B1DRange& rPrioRange )
        {
            if( !mbBackgroundLayer )
            {
                for( const auto& rViewEntry : maViewEntries )
                    rViewEntry.getViewLayer()->setPriority( rPrioRange );
            }
        }

        void Layer::addUpdateRange( ::basegfx::B2DRange const& rUpdateRange )
        {
            // TODO(Q1): move this to B2DMultiRange
            if( !rUpdateRange.isEmpty() )
                maUpdateAreas.appendElement( rUpdateRange,
                                             basegfx::B2VectorOrientation::Positive );
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
                               [this]( const ViewEntry& rViewEntry )
                               { return rViewEntry.getViewLayer()->resize( this->maBounds ); }
                               ) == 0 )
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
            for( const auto& rViewEntry : maViewEntries )
                rViewEntry.getViewLayer()->clearAll();

            // layer content cleared, update areas are not sensible
            // anymore.
            clearUpdateRanges();
        }

        class LayerEndUpdate
        {
        public:
            LayerEndUpdate( const LayerEndUpdate& ) = delete;
            LayerEndUpdate& operator=( const LayerEndUpdate& ) = delete;
            explicit LayerEndUpdate( LayerSharedPtr const& rLayer ) :
                mpLayer( rLayer )
            {}

            ~LayerEndUpdate() { if(mpLayer) mpLayer->endUpdate(); }

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
                aClip = ::basegfx::utils::stripNeutralPolygons(aClip);
                aClip = ::basegfx::utils::stripDispensablePolygons(aClip);

                // actually, if there happen to be shapes with zero
                // update area in the maUpdateAreas vector, the
                // resulting clip polygon will be empty.
                if( aClip.count() )
                {
                    for( const auto& rViewEntry : maViewEntries )
                    {
                        const ViewLayerSharedPtr& pViewLayer = rViewEntry.getViewLayer();

                        // set clip to all view layers and
                        pViewLayer->setClip( aClip );

                        // clear update area on all view layers
                        pViewLayer->clear();
                    }

                    mbClipSet = true;
                }
            }

            return std::make_shared<LayerEndUpdate>(shared_from_this());
        }

        void Layer::endUpdate()
        {
            if( mbClipSet )
            {
                mbClipSet = false;

                basegfx::B2DPolyPolygon aEmptyClip;
                for( const auto& rViewEntry : maViewEntries )
                    rViewEntry.getViewLayer()->setClip( aEmptyClip );
            }

            clearUpdateRanges();
        }

        bool Layer::isInsideUpdateArea( ShapeSharedPtr const& rShape ) const
        {
            return maUpdateAreas.overlaps( rShape->getUpdateArea() );
        }

        LayerSharedPtr Layer::createBackgroundLayer()
        {
            return LayerSharedPtr(new Layer( BackgroundLayer ));
        }

        LayerSharedPtr Layer::createLayer( )
        {
            return LayerSharedPtr( new Layer );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
