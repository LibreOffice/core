/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationsetnode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:41:57 $
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

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <animationfactory.hxx>
#include <setactivity.hxx>
#include <animationsetnode.hxx>
#include <nodetools.hxx>
#include <tools.hxx>
#include <delayevent.hxx>


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        AnimationSetNode::AnimationSetNode( const uno::Reference< animations::XAnimationNode >& xNode,
                                            const BaseContainerNodeSharedPtr&                   rParent,
                                            const NodeContext&                                  rContext ) :
            ActivityAnimationBaseNode( xNode, rParent, rContext )
        {
        }

        bool AnimationSetNode::init()
        {
            if( !AnimationBaseNode::init() )
                return false;

            try
            {
                // TODO(F2): For restart functionality, we must regenerate activities,
                // since they are not able to reset their state (or implement _that_)
                getActivity() = createSetActivity();
            }
            catch( uno::Exception& )
            {
                // catch and ignore. We later handle empty activities, but for
                // other nodes to function properly, the core functionality of
                // this node must remain up and running.
            }

            return true;
        }

#if defined(VERBOSE) && defined(DBG_UTIL)
        const char* AnimationSetNode::getDescription() const
        {
            return "AnimationSetNode";
        }
#endif

        void AnimationSetNode::implScheduleDeactivationEvent() const
        {
            if( !isIndefiniteTiming( getXNode()->getDuration() ) )
            {
                AnimationBaseNode::scheduleDeactivationEvent();
            }
            else
            {
                // set node has indefinite duration - SMIL spec demands that
                // this is equivalent to duration '0'

                // schedule delay event (note: this looks misleading, but
                // since BaseNode::deactivate() is virtual, we actually call
                // AnimationBaseNode::deactivate() with this functor)
                getContext().mrEventQueue.addEvent(
                    makeEvent( boost::bind( &BaseNode::deactivate,
                                            getSelf() ) ) );
            }
        }

        AnimationActivitySharedPtr AnimationSetNode::createSetActivity()
        {
            ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );

            const ::rtl::OUString& rAttrName( getXAnimateNode()->getAttributeName() );

            const AttributableShapeSharedPtr& rShape( getShape() );

            // make deactivation a two-step procedure. Normally, we
            // could solely rely on
            // BaseNode::scheduleDeactivationEvent() to deactivate()
            // us. Unfortunately, that method on the one hand ignores
            // indefinite timing, on the other hand generates
            // zero-timeout delays, which might get fired _before_ our
            // set activity has taken place. Therefore, we enforce
            // sequentiality by letting only the set activity schedule
            // the deactivation event (and ActivityAnimationBaseNode
            // takes care for the fact when mpActivity should be zero).
            AnimationSetNodeSharedPtr pSelf(
                ::boost::dynamic_pointer_cast< AnimationSetNode >(getSelf()) );

            ENSURE_AND_THROW( pSelf.get(),
                              "AnimationSetNode::createSetActivity(): cannot cast getSelf() to my type!" );

            aParms.mpEndEvent =
                makeEvent(
                    ::boost::bind( &AnimationSetNode::implScheduleDeactivationEvent,
                                   pSelf ) );

            switch( AnimationFactory::classifyAttributeName( rAttrName ) )
            {
                default:
                case AnimationFactory::CLASS_UNKNOWN_PROPERTY:
                    ENSURE_AND_THROW( false,
                                      "AnimationSetNode::createSetActivity(): Unexpected attribute class" );
                    break;

                case AnimationFactory::CLASS_NUMBER_PROPERTY:
                {
                    NumberAnimation::ValueType aValue;

                    ENSURE_AND_THROW( extractValue( aValue,
                                                    getXAnimateNode()->getTo(),
                                                    rShape, getContext().mpLayerManager ),
                                      "AnimationSetNode::createSetActivity(): Could not import numeric to value" );

                    return makeSetActivity( aParms,
                                            AnimationFactory::createNumberPropertyAnimation(
                                                rAttrName,
                                                rShape,
                                                getContext().mpLayerManager,
                                                AnimationFactory::FLAG_NO_SPRITE ),
                                            aValue );
                }

                case AnimationFactory::CLASS_ENUM_PROPERTY:
                {
                    EnumAnimation::ValueType aValue;

                    ENSURE_AND_THROW( extractValue( aValue,
                                                    getXAnimateNode()->getTo(),
                                                    rShape,
                                                    getContext().mpLayerManager ),
                                      "AnimationSetNode::createSetActivity(): Could not import enum to value" );

                    return makeSetActivity( aParms,
                                            AnimationFactory::createEnumPropertyAnimation(
                                                rAttrName,
                                                rShape,
                                                getContext().mpLayerManager,
                                                AnimationFactory::FLAG_NO_SPRITE ),
                                            aValue );
                }

                case AnimationFactory::CLASS_COLOR_PROPERTY:
                {
                    ColorAnimation::ValueType aValue;

                    ENSURE_AND_THROW( extractValue( aValue,
                                                    getXAnimateNode()->getTo(),
                                                    rShape,
                                                    getContext().mpLayerManager ),
                                      "AnimationSetNode::createSetActivity(): Could not import color to value" );

                    return makeSetActivity( aParms,
                                            AnimationFactory::createColorPropertyAnimation(
                                                rAttrName,
                                                rShape,
                                                getContext().mpLayerManager,
                                                AnimationFactory::FLAG_NO_SPRITE ),
                                            aValue );
                }

                case AnimationFactory::CLASS_STRING_PROPERTY:
                {
                    StringAnimation::ValueType aValue;

                    ENSURE_AND_THROW( extractValue( aValue,
                                                    getXAnimateNode()->getTo(),
                                                    rShape,
                                                    getContext().mpLayerManager ),
                                      "AnimationSetNode::createSetActivity(): Could not import string to value" );

                    return makeSetActivity( aParms,
                                            AnimationFactory::createStringPropertyAnimation(
                                                rAttrName,
                                                rShape,
                                                getContext().mpLayerManager,
                                                AnimationFactory::FLAG_NO_SPRITE ),
                                            aValue );
                }

                case AnimationFactory::CLASS_BOOL_PROPERTY:
                {
                    BoolAnimation::ValueType aValue;

                    ENSURE_AND_THROW( extractValue( aValue,
                                                    getXAnimateNode()->getTo(),
                                                    rShape,
                                                    getContext().mpLayerManager ),
                                      "AnimationSetNode::createSetActivity(): Could not import bool to value" );

                    return makeSetActivity( aParms,
                                            AnimationFactory::createBoolPropertyAnimation(
                                                rAttrName,
                                                rShape,
                                                getContext().mpLayerManager,
                                                AnimationFactory::FLAG_NO_SPRITE ),
                                            aValue );
                }
            }

            return AnimationActivitySharedPtr();
        }
    }
}
