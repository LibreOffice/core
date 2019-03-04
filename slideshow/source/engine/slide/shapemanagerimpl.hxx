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
#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_SHAPEMANAGERIMPL_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_SHAPEMANAGERIMPL_HXX

#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/presentation/XShapeEventListener.hpp>

#include <shape.hxx>
#include <subsettableshapemanager.hxx>
#include <eventmultiplexer.hxx>
#include "layermanager.hxx"
#include <viewupdate.hxx>
#include <shapemaps.hxx>
#include <cursormanager.hxx>
#include <hyperlinkarea.hxx>
#include <listenercontainer.hxx>
#include <shapelistenereventhandler.hxx>
#include <mouseeventhandler.hxx>

#include <set>
#include <map>
#include <memory>

namespace slideshow {
namespace internal {

/** Listener class for shape events

    This helper class registers itself on each view, and
    broadcasts the XShapeEventListener events. The mouse motion
    events are needed for setting the shape cursor.
*/
class ShapeManagerImpl : public SubsettableShapeManager,
                         public ShapeListenerEventHandler,
                         public MouseEventHandler,
                         public ViewUpdate,
                         public std::enable_shared_from_this<ShapeManagerImpl>
{
public:
    /** Create a shape event broadcaster

        @param rEventMultiplexer
        The slideshow-global event source, where this class
        registers its event handlers.
    */
    ShapeManagerImpl( EventMultiplexer&            rMultiplexer,
                      LayerManagerSharedPtr const& rLayerManager,
                      CursorManager&               rCursorManager,
                      const ShapeEventListenerMap& rGlobalListenersMap,
                      const ShapeCursorMap&        rGlobalCursorMap );

    /// Forbid copy construction
    ShapeManagerImpl(const ShapeManagerImpl&) = delete;

    /// Forbid copy assignment
    ShapeManagerImpl& operator=(const ShapeManagerImpl&) = delete;

    /** Enables event listening.

        The initial slide content on the background layer
        is already rendered (e.g. from a previous slide
        transition).
     */
    void activate();

    /** Disables event listening.
     */
    void deactivate();

    // Disposable interface


    virtual void dispose() override;

private:

    // MouseEventHandler interface


    virtual bool handleMousePressed(
        css::awt::MouseEvent const& evt ) override;
    virtual bool handleMouseReleased(
        css::awt::MouseEvent const& evt ) override;
    virtual bool handleMouseDragged(
        css::awt::MouseEvent const& evt ) override;
    virtual bool handleMouseMoved(
        css::awt::MouseEvent const& evt ) override;


    // ViewUpdate interface


    virtual bool update() override;
    virtual bool needsUpdate() const override;


    // ShapeManager interface


    virtual void enterAnimationMode( const AnimatableShapeSharedPtr& rShape ) override;
    virtual void leaveAnimationMode( const AnimatableShapeSharedPtr& rShape ) override;
    virtual void notifyShapeUpdate( const ShapeSharedPtr& rShape ) override;
    virtual ShapeSharedPtr lookupShape(
        css::uno::Reference< css::drawing::XShape > const & xShape ) const override;
    virtual void addHyperlinkArea( const HyperlinkAreaSharedPtr& rArea ) override;


    // SubsettableShapeManager interface


    virtual AttributableShapeSharedPtr getSubsetShape(
        const AttributableShapeSharedPtr& rOrigShape,
        const DocTreeNode&                rTreeNode ) override;
    virtual void revokeSubset(
        const AttributableShapeSharedPtr& rOrigShape,
        const AttributableShapeSharedPtr& rSubsetShape ) override;

    virtual void addIntrinsicAnimationHandler(
        const IntrinsicAnimationEventHandlerSharedPtr& rHandler ) override;
    virtual void removeIntrinsicAnimationHandler(
        const IntrinsicAnimationEventHandlerSharedPtr& rHandler ) override;
    virtual void notifyIntrinsicAnimationsEnabled() override;
    virtual void notifyIntrinsicAnimationsDisabled() override;


    // ShapeListenerEventHandler


    virtual bool listenerAdded( const css::uno::Reference< css::presentation::XShapeEventListener>& xListener,
                                const css::uno::Reference< css::drawing::XShape>&                   xShape ) override;

    virtual bool listenerRemoved( const css::uno::Reference< css::presentation::XShapeEventListener>& xListener,
                                  const css::uno::Reference< css::drawing::XShape>&                   xShape ) override;

    void cursorChanged( const css::uno::Reference< css::drawing::XShape>&   xShape,
                              sal_Int16                                     nCursor );


    OUString checkForHyperlink( ::basegfx::B2DPoint const& hitPos )const;


    typedef std::map<ShapeSharedPtr,
                     std::shared_ptr< ::comphelper::OInterfaceContainerHelper2 >,
                     Shape::lessThanShape>        ShapeToListenersMap;
    typedef std::map<ShapeSharedPtr, sal_Int16,
                       Shape::lessThanShape>      ShapeToCursorMap;
    typedef std::set<HyperlinkAreaSharedPtr,
                     HyperlinkArea::lessThanArea> AreaSet;

    typedef ThreadUnsafeListenerContainer<
        IntrinsicAnimationEventHandlerSharedPtr,
        std::vector<IntrinsicAnimationEventHandlerSharedPtr> > ImplIntrinsicAnimationEventHandlers;

    EventMultiplexer&                   mrMultiplexer;
    LayerManagerSharedPtr               mpLayerManager;
    CursorManager&                      mrCursorManager;
    const ShapeEventListenerMap&        mrGlobalListenersMap;
    const ShapeCursorMap&               mrGlobalCursorMap;
    ShapeToListenersMap                 maShapeListenerMap;
    ShapeToCursorMap                    maShapeCursorMap;
    AreaSet                             maHyperlinkShapes;
    ImplIntrinsicAnimationEventHandlers maIntrinsicAnimationEventHandlers;
    bool                                mbEnabled;
};

} // namespace internal
} // namespace presentation

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_SHAPEMANAGERIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
