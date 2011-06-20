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

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/presentation/XShapeEventListener.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/awt/MouseButton.hpp>

#include "shapemanagerimpl.hxx"

#include <boost/bind.hpp>

#include <o3tl/compat_functional.hxx>

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

void ShapeManagerImpl::activate( bool bSlideBackgoundPainted )
{
    if( !mbEnabled )
    {
        mbEnabled = true;

        // register this handler on EventMultiplexer.
        // Higher prio (overrides other engine handlers)
        mrMultiplexer.addMouseMoveHandler( shared_from_this(), 2.0 );
        mrMultiplexer.addClickHandler( shared_from_this(), 2.0 );
        mrMultiplexer.addShapeListenerHandler( shared_from_this() );

        // clone listener map
        uno::Reference<presentation::XShapeEventListener> xDummyListener;
        std::for_each( mrGlobalListenersMap.begin(),
                       mrGlobalListenersMap.end(),
                       boost::bind( &ShapeManagerImpl::listenerAdded,
                                    this,
                                    boost::cref(xDummyListener),
                                    boost::bind(
                                        o3tl::select1st<ShapeEventListenerMap::value_type>(),
                                        _1 )));

        // clone cursor map
        std::for_each( mrGlobalCursorMap.begin(),
                       mrGlobalCursorMap.end(),
                       boost::bind( &ShapeManagerImpl::cursorChanged,
                                    this,
                                    boost::bind(
                                        o3tl::select1st<ShapeCursorMap::value_type>(),
                                        _1 ),
                                    boost::bind(
                                        o3tl::select2nd<ShapeCursorMap::value_type>(),
                                        _1 )));

        if( mpLayerManager )
            mpLayerManager->activate( bSlideBackgoundPainted );
    }
}

void ShapeManagerImpl::deactivate()
{
    if( mbEnabled )
    {
        mbEnabled = false;

        if( mpLayerManager )
            mpLayerManager->deactivate();

        maShapeListenerMap.clear();
        maShapeCursorMap.clear();

        mrMultiplexer.removeShapeListenerHandler( shared_from_this() );
        mrMultiplexer.removeMouseMoveHandler( shared_from_this() );
        mrMultiplexer.removeClickHandler( shared_from_this() );
    }
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
    rtl::OUString const hyperlink( checkForHyperlink(aPosition) );
    if( hyperlink.getLength() > 0 )
    {
        mrMultiplexer.notifyHyperlinkClicked(hyperlink);
        return true; // event consumed
    }

    // find matching shape (scan reversely, to coarsely match
    // paint order)
    ShapeToListenersMap::reverse_iterator aCurrBroadcaster(
        maShapeListenerMap.rbegin() );
    ShapeToListenersMap::reverse_iterator const aEndBroadcasters(
        maShapeListenerMap.rend() );
    while( aCurrBroadcaster != aEndBroadcasters )
    {
        // TODO(F2): Get proper geometry polygon from the
        // shape, to avoid having areas outside the shape
        // react on the mouse
        if( aCurrBroadcaster->first->getBounds().isInside( aPosition ) &&
            aCurrBroadcaster->first->isVisible() )
        {
            // shape hit, and shape is visible. Raise
            // event.

            boost::shared_ptr<cppu::OInterfaceContainerHelper> const pCont(
                aCurrBroadcaster->second );
            uno::Reference<drawing::XShape> const xShape(
                aCurrBroadcaster->first->getXShape() );

            // DON'T do anything with /this/ after this point!
            pCont->forEach<presentation::XShapeEventListener>(
                boost::bind( &presentation::XShapeEventListener::click,
                             _1,
                             boost::cref(xShape),
                             boost::cref(e) ));

            return true; // handled this event
        }

        ++aCurrBroadcaster;
    }

    return false; // did not handle this event
}

bool ShapeManagerImpl::handleMouseEntered( const awt::MouseEvent& )
{
    // not used here
    return false; // did not handle the event
}

bool ShapeManagerImpl::handleMouseExited( const awt::MouseEvent& )
{
    // not used here
    return false; // did not handle the event
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

    if( checkForHyperlink(aPosition).getLength() > 0 )
    {
        nNewCursor = awt::SystemPointer::REFHAND;
    }
    else
    {
        // find matching shape (scan reversely, to coarsely match
        // paint order)
        ShapeToCursorMap::reverse_iterator aCurrCursor(
            maShapeCursorMap.rbegin() );
        ShapeToCursorMap::reverse_iterator const aEndCursors(
            maShapeCursorMap.rend() );
        while( aCurrCursor != aEndCursors )
        {
            // TODO(F2): Get proper geometry polygon from the
            // shape, to avoid having areas outside the shape
            // react on the mouse
            if( aCurrCursor->first->getBounds().isInside( aPosition ) &&
                aCurrCursor->first->isVisible() )
            {
                // shape found, and it's visible. set
                // requested cursor to shape's
                nNewCursor = aCurrCursor->second;
                break;
            }

            ++aCurrCursor;
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

bool ShapeManagerImpl::update( ViewSharedPtr const& /*rView*/ )
{
    // am not doing view-specific updates here.
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

void ShapeManagerImpl::removeHyperlinkArea( const HyperlinkAreaSharedPtr& rArea )
{
    maHyperlinkShapes.erase(rArea);
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
        maShapeListenerMap.insert(
            ShapeToListenersMap::value_type(
                pShape,
                aIter->second));
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

bool ShapeManagerImpl::cursorChanged( const uno::Reference<drawing::XShape>&   xShape,
                                      sal_Int16                                nCursor )
{
    ShapeSharedPtr pShape( lookupShape(xShape) );

    // is this one of our shapes? other shapes are ignored.
    if( !pShape )
        return false;

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
            maShapeCursorMap.insert(
                ShapeToCursorMap::value_type(
                    pShape,
                    nCursor ));
        }
        else
        {
            aIter->second = nCursor;
        }
    }

    return true;
}

rtl::OUString ShapeManagerImpl::checkForHyperlink( basegfx::B2DPoint const& hitPos ) const
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

    return rtl::OUString();
}

void ShapeManagerImpl::addIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler )
{
    maIntrinsicAnimationEventHandlers.add( rHandler );
}

void ShapeManagerImpl::removeIntrinsicAnimationHandler( const IntrinsicAnimationEventHandlerSharedPtr& rHandler )
{
    maIntrinsicAnimationEventHandlers.remove( rHandler );
}

bool ShapeManagerImpl::notifyIntrinsicAnimationsEnabled()
{
    return maIntrinsicAnimationEventHandlers.applyAll(
        boost::mem_fn(&IntrinsicAnimationEventHandler::enableAnimations));
}

bool ShapeManagerImpl::notifyIntrinsicAnimationsDisabled()
{
    return maIntrinsicAnimationEventHandlers.applyAll(
        boost::mem_fn(&IntrinsicAnimationEventHandler::disableAnimations));
}



} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
