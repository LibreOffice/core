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
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONNODE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONNODE_HXX

#include "disposable.hxx"

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <memory>


namespace slideshow {
namespace internal {

class AnimationNode;
typedef ::std::shared_ptr< AnimationNode > AnimationNodeSharedPtr;

/** This interface is used to mirror every XAnimateNode object
    in the presentation core.
*/
class AnimationNode : public Disposable
{
public:
    /** The current state of this AnimationNode
     */
    enum NodeState {
        /// Invalid state, node is disposed or otherwise invalid
        INVALID      =0,
        /// Unresolved start time
        UNRESOLVED   =1,
        /// Resolved start time, node will start eventually
        RESOLVED     =2,
        /// Node is active
        ACTIVE       =4,
        /// Node is frozen (no longer active, but changes remain in place)
        FROZEN       =8,
        /// Node has completed an active lifecycle,
        /// and any effect is removed from the document
        ENDED       =16
    };

    /** Query the corresponding XAnimationNode.
     */
    virtual css::uno::Reference< css::animations::XAnimationNode >
    getXAnimationNode() const = 0;

    /** Init this node

        If this node is not in state INVALID, init() sets up the
        node state and schedules necessary events.
        If this node has children, they have their init() called, too.
        You will call this method whenever a slide is going to be
        shown.

        @return true, if init was successful; state has changed to UNRESOLVED
    */
    virtual bool init() = 0;

    /** Resolve node start time

        Nodes can have unresolved start times, i.e. indefinite
        start time for container nodes, or child nodes whose
        parent has not yet started. Calling this method fixes
        the node's start time. This does not mean that this
        node immediately starts its animations, that is only
        the case for begin=0.0. The node will change its state
        to RESOLVED.

        @return true, if a start event was successfully scheduled.
    */
    virtual bool resolve() = 0;

    /** Immediately start this node

        This method starts the animation on this node, without
        begin timeout. The node will change its state to ACTIVE.
    */
    virtual void activate() = 0;

    /** Immediately stop this node

        This method stops the animation on this node. The node
        will change its state to either ENDED or FROZEN,
        depending on XAnimationNode attributes.
    */
    virtual void deactivate() = 0;

    /** End the animation on this node

        This method force-ends animation on this node. Parents
        may call this for their children, if their active
        duration ends. An ended animation will no longer have
        any effect on the shape attributes. The node will
        change its state to ENDED.
    */
    virtual void end() = 0;

    /** Query node state

        @return the current state of this animation node.
    */
    virtual NodeState getState() const = 0;

    /** Register a deactivating listener

        This method registers another AnimationNode as an
        deactivating listener, which gets notified via a
        notifyDeactivating() call. The node calls all
        registered listener, when it leaves the ACTIVE state.

        @param rNotifee AnimationNode to notify
    */
    virtual bool registerDeactivatingListener(
        const AnimationNodeSharedPtr& rNotifee ) = 0;

    /** Called to notify another AnimationNode's deactivation

        @param rNotifier The instance who calls this method.
    */
    virtual void notifyDeactivating(
        const AnimationNodeSharedPtr& rNotifier ) = 0;

    /** Query node whether it has an animation pending.

        @return true, if this node (or at least one of its children)
        has an animation pending. Used to determine if the main
        sequence is actually empty, or contains effects
    */
    virtual bool hasPendingAnimation() const = 0;
};

} // namespace internal
} // namespace presentation

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
