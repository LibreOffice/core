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


#include <tools/diagnose_ex.h>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/presentation/XShapeEventListener.hpp>

#include "shapemanagerimpl.hxx"

#include <functional>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

ShapeManagerImpl::ShapeManagerImpl( EventMultiplexer&            rMultiplexer,
                                    LayerManagerSharedPtr const& rLayerManager,
                                    CursorManager&               rCursorManager,
                                    const ShapeEventListenerMap& rGlobalListenersMap,
                                    const ShapeCursorMap&        rGlobalCursorMap ):
    mrMultiplexer(rMultiplexer),
    mpLayerManager(rLayerManager),
    mrCursorManager(rCursorManager),
    mrGlobalListenersMap(rGlobalListenersMap),
    mrGlobalCursorMap(rGlobalCursorMap),
    maShapeListenerMap(),
    maShapeCursorMap(),
    maHyperlinkShapes(),
    mbEnabled(false)
{
}

void ShapeManagerImpl::activate()
{
    if( mbEnabled )
        return;

    mbEnabled = true;

    // register this handler on EventMultiplexer.
    // Higher prio (overrides other engine handlers)
    mrMultiplexer.addMouseMoveHandler( shared_from_this(), 2.0 );
    mrMultiplexer.addClickHandler( shared_from_this(), 2.0 );
    mrMultiplexer.addShapeListenerHandler( shared_from_this() );

    // clone listener map
    uno::Reference<presentation::XShapeEventListener> xDummyListener;
    for( const auto& rListener : mrGlobalListenersMap )
        listenerAdded( xDummyListener, rListener.first );

    // clone cursor map
    for( const auto& rListener : mrGlobalCursorMap )
        cursorChanged( rListener.first, rListener.second );

    if( mpLayerManager )
        mpLayerManager->activate();
}

void ShapeManagerImpl::deactivate()
{
    if( !mbEnabled )
        return;

    mbEnabled = false;

    if( mpLayerManager )
        mpLayerManager->deactivate();

    maShapeListenerMap.clear();
    maShapeCursorMap.clear();

    mrMultiplexer.removeShapeListenerHandler( shared_from_this() );
    mrMultiplexer.removeMouseMoveHandler( shared_from_this() );
    mrMultiplexer.removeClickHandler( shared_from_this() );
}

void ShapeManagerImpl::dispose()
{
    // remove listeners (EventMultiplexer holds shared_ptr on us)
    deactivate();

    maHyperlinkShapes.clear();
    maShapeCursorMap.clear();
    maShapeListenerMap.clear();
    mpLayerManager.reset();
}

bool ShapeManagerImpl::handleMousePressed( awt::MouseEvent const& )
{
    // not used here
    return false; // did not handle the event
}

bool ShapeManagerImpl::handleMouseReleased( awt::MouseEvent const& e )
{
    if( !mbEnabled || e.Buttons != awt::MouseButton::LEFT)
        return false;

    basegfx::B2DPoint const aPosition( e.X, e.Y );

    // first check for hyperlinks, because these have
    // highest prio:
    OUString const hyperlink( checkForHyperlink(aPosition) );
    if( !hyperlink.isEmpty() )
    {
        mrMultiplexer.notifyHyperlinkClicked(hyperlink);
        return true; // event consumed
    }

    // find matching shape (scan reversely, to coarsely match
    // paint order)
    auto aCurrBroadcaster = std::find_if(maShapeListenerMap.rbegin(), maShapeListenerMap.rend(),
        [&aPosition](const ShapeToListenersMap::value_type& rBroadcaster) {
            // TODO(F2): Get proper geometry polygon from the
            // shape, to avoid having areas outside the shape
            // react on the mouse
            return rBroadcaster.first->getBounds().isInside( aPosition )
                && rBroadcaster.first->isVisible();
        });
    if (aCurrBroadcaster != maShapeListenerMap.rend())
    {
        // shape hit, and shape is visible. Raise
        // event.

        std::shared_ptr<comphelper::OInterfaceContainerHelper2> const pCont(
            aCurrBroadcaster->second );
        uno::Reference<drawing::XShape> const xShape(
            aCurrBroadcaster->first->getXShape() );

        // DON'T do anything with /this/ after this point!
        pCont->forEach<presentation::XShapeEventListener>(
            [&xShape, &e]( const uno::Reference< presentation::XShapeEventListener >& rListener )
            { return rListener->click( xShape, e ); } );

        return true; // handled this event
    }

    return false; // did not handle this event
}

bool ShapeManagerImpl::handleMouseDragged( const awt::MouseEvent& )
{
    // not used here
    return false; // did not handle the event
}

bool ShapeManagerImpl::handleMouseMoved( const awt::MouseEvent& e )
{
    if( !mbEnabled )
        return false;

    // find hit shape in map
    const ::basegfx::B2DPoint aPosition( e.X, e.Y );
    sal_Int16                 nNewCursor(-1);

    if( !checkForHyperlink(aPosition).isEmpty() )
    {
        nNewCursor = awt::SystemPointer::REFHAND;
    }
    else
    {
        // find matching shape (scan reversely, to coarsely match
        // paint order)
        auto aCurrCursor = std::find_if(maShapeCursorMap.rbegin(), maShapeCursorMap.rend(),
            [&aPosition](const ShapeToCursorMap::value_type& rCursor) {
                // TODO(F2): Get proper geometry polygon from the
                // shape, to avoid having areas outside the shape
                // react on the mouse
                return rCursor.first->getBounds().isInside( aPosition )
                    && rCursor.first->isVisible();
            });
        if (aCurrCursor != maShapeCursorMap.rend())
        {
            // shape found, and it's visible. set
            // requested cursor to shape's
            nNewCursor = aCurrCursor->second;
        }
    }

    if( nNewCursor == -1 )
        mrCursorManager.resetCursor();
    else
        mrCursorManager.requestCursor( nNewCursor );

    return false; // we don't /eat/ this event. Lower prio
                  // handler should see it, too.
}

bool ShapeManagerImpl::update()
{
    if( mbEnabled && mpLayerManager )
        return mpLayerManager->update();

    return false;
}

bool ShapeManagerImpl::needsUpdate() const
{
    if( mbEnabled && mpLayerManager )
        return mpLayerManager->isUpdatePending();

    return false;
}

void ShapeManagerImpl::enterAnimationMode( const AnimatableShapeSharedPtr& rShape )
{
    if( mbEnabled && mpLayerManager )
        mpLayerManager->enterAnimationMode(rShape);
}

void ShapeManagerImpl::leaveAnimationMode( const AnimatableShapeSharedPtr& rShape )
{
    if( mbEnabled && mpLayerManager )
        mpLayerManager->leaveAnimationMode(rShape);
}

void ShapeManagerImpl::notifyShapeUpdate( const ShapeSharedPtr& rShape )
{
    if( mbEnabled && mpLayerManager )
        mpLayerManager->notifyShapeUpdate(rShape);
}

ShapeSharedPtr ShapeManagerImpl::lookupShape( uno::Reference< drawing::XShape > const & xShape ) const
{
    if( mpLayerManager )
        return mpLayerManager->lookupShape(xShape);

    return ShapeSharedPtr();
}

void ShapeManagerImpl::addHyperlinkArea( const HyperlinkAreaSharedPtr& rArea )
{
    maHyperlinkShapes.insert(rArea);
}

AttributableShapeSharedPtr ShapeManagerImpl::getSubsetShape( const AttributableShapeSharedPtr& rOrigShape,
                                                             const DocTreeNode&                rTreeNode )
{
    if( mpLayerManager )
        return mpLayerManager->getSubsetShape(rOrigShape,rTreeNode);

    return AttributableShapeSharedPtr();
}

void ShapeManagerImpl::revokeSubset( const AttributableShapeSharedPtr& rOrigShape,
                                     const AttributableShapeSharedPtr& rSubsetShape )
{
    if( mpLayerManager )
        mpLayerManager->revokeSubset(rOrigShape,rSubsetShape);
}

bool ShapeManagerImpl::listenerAdded(
    const uno::Reference<presentation::XShapeEventListener>& /*xListener*/,
    const uno::Reference<drawing::XShape>&                   xShape )
{
    ShapeEventListenerMap::const_iterator aIter;
    if( (aIter = mrGlobalListenersMap.find( xShape )) ==
        mrGlobalListenersMap.end() )
    {
        ENSURE_OR_RETURN_FALSE(false,
                          "ShapeManagerImpl::listenerAdded(): global "
                          "shape listener map inconsistency!");
    }

    // is this one of our shapes? other shapes are ignored.
    ShapeSharedPtr pShape( lookupShape(xShape) );
    if( pShape )
    {
        maShapeListenerMap.emplace(pShape, aIter->second);
    }

    return true;
}

bool ShapeManagerImpl::listenerRemoved(
    const uno::Reference<presentation::XShapeEventListener>& /*xListener*/,
    const uno::Reference<drawing::XShape>&                   xShape )
{
    // shape really erased from map? maybe there are other listeners
    // for the same shape pending...
    if( mrGlobalListenersMap.find(xShape) == mrGlobalListenersMap.end() )
    {
        // is this one of our shapes? other shapes are ignored.
        ShapeSharedPtr pShape( lookupShape(xShape) );
        if( pShape )
            maShapeListenerMap.erase(pShape);
    }

    return true;
}

void ShapeManagerImpl::cursorChanged( const uno::Reference<drawing::XShape>&   xShape,
                                      sal_Int16                                nCursor )
{
    ShapeSharedPtr pShape( lookupShape(xShape) );

    // is this one of our shapes? other shapes are ignored.
    if( !pShape )
        return;

    if( mrGlobalCursorMap.find(xShape) == mrGlobalCursorMap.end() )
    {
        // erased from global map - erase locally, too
        maShapeCursorMap.erase(pShape);
    }
    else
    {
        // included in global map - update local one
        ShapeToCursorMap::iterator aIter;
        if( (aIter = maShapeCursorMap.find(pShape))
            == maShapeCursorMap.end() )
        {
            maShapeCursorMap.emplace(pShape, nCursor);
        }
        else
        {
            aIter->second = nCursor;
        }
    }
}

OUString ShapeManagerImpl::checkForHyperlink( basegfx::B2DPoint const& hitPos ) const
{
    // find matching region (scan reversely, to coarsely match
    // paint order): set is ordered by priority
    AreaSet::const_reverse_iterator iPos( maHyperlinkShapes.rbegin() );
    AreaSet::const_reverse_iterator const iEnd( maHyperlinkShapes.rend() );
    for( ; iPos != iEnd; ++iPos )
    {
        HyperlinkAreaSharedPtr const& pArea = *iPos;

        HyperlinkArea::HyperlinkRegions const linkRegions(
            pArea->getHyperlinkRegions() );

        for( std::size_t i = linkRegions.size(); i--; )
        {
            basegfx::B2DRange const& region = linkRegions[i].first;
            if( region.isInside(hitPos) )
                return linkRegions[i].second;
        }
    }

    return OUString();
}

void ShapeManagerImpl::addIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler )
{
    maIntrinsicAnimationEventHandlers.add( rHandler );
}

void ShapeManagerImpl::removeIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler )
{
    maIntrinsicAnimationEventHandlers.remove( rHandler );
}

void ShapeManagerImpl::notifyIntrinsicAnimationsEnabled()
{
    maIntrinsicAnimationEventHandlers.applyAll(
        std::mem_fn(&IntrinsicAnimationEventHandler::enableAnimations));
}

void ShapeManagerImpl::notifyIntrinsicAnimationsDisabled()
{
    maIntrinsicAnimationEventHandlers.applyAll(
        std::mem_fn(&IntrinsicAnimationEventHandler::disableAnimations));
}


} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
