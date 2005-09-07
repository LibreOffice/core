/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationnode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:06:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SLIDESHOW_ANIMATIONNODE_HXX
#define _SLIDESHOW_ANIMATIONNODE_HXX

#include <disposable.hxx>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

namespace presentation
{
    namespace internal
    {
        /* Definition of AnimationNode interface */

        /** This interface is used to mirror every XAnimateNode object
            in the presentation core.
         */
        class AnimationNode : public Disposable
        {
        public:
            /** The current state of this AnimationNode
             */
            enum NodeState
            {
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
                /// Node has completed an active lifecycle, and any effect is removed from the document
                ENDED       =16
            };

            /** Query the corresponding XAnimationNode.
             */
            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::animations::XAnimationNode > getXAnimationNode() const = 0;

            /** Init this node

                If this node is not in state INVALID, init() sets up the
                node state and schedules necessary events. If this
                node has children, they have their init() called, too.
                You will call this method whenever a slide is going to be
                shown.

                @return true, if init was successful
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

                @return true, if a start event was successfully
                scheduled.
             */
            virtual bool resolve() = 0;

            /** Immediately start this node

                This method starts the animation on this node, without
                begin timeout. The node will change its state to ACTIVE.

                @return true, if start was successful. This method
                might return false, if e.g. a restart is not permitted
                on this node.
             */
            virtual bool activate() = 0;

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

                This methods returns the current state of this
                animation node.
             */
            virtual NodeState getState() const = 0;

            /** Register a deactivating listener

                This method registers another AnimationNode as an
                deactivating listener, which gets notified via a
                notifyDeactivating() call. The node calls all
                registered listener, when it leaves the ACTIVE state.

                @param rNotifee
                AnimationNode to notify
             */
            virtual bool registerDeactivatingListener( const ::boost::shared_ptr< AnimationNode >& rNotifee ) = 0;

            /** Called to notify another AnimationNode's deactivation

                @param rNotifier
                The instance who calls this method.
             */
            virtual void notifyDeactivating( const ::boost::shared_ptr< AnimationNode >& rNotifier ) = 0;

            /** Query node whether it has an animation pending.

                @return true, if this node (or at least one of its
                children) has an animation pending.
             */
            virtual bool hasPendingAnimation() const = 0;
        };

        typedef ::boost::shared_ptr< AnimationNode > AnimationNodeSharedPtr;
    }
}

#endif /* _SLIDESHOW_ANIMATIONNODE_HXX */
