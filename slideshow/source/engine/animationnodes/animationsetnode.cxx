/*************************************************************************
 *
 *  $RCSfile: animationsetnode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:03:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            AnimationBaseNode( xNode, rParent, rContext )
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

        void AnimationSetNode::scheduleDeactivationEvent() const
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
                    makeEvent( ::boost::bind(&BaseNode::deactivate,
                                             ::boost::cref( getSelf() ) ) ) );
            }
        }

#if defined(VERBOSE) && defined(DBG_UTIL)
        const char* AnimationSetNode::getDescription() const
        {
            return "AnimationSetNode";
        }
#endif

        AnimationActivitySharedPtr AnimationSetNode::createSetActivity()
        {
            const ::rtl::OUString& rAttrName( getXAnimateNode()->getAttributeName() );

            const AttributableShapeSharedPtr& rShape( getShape() );

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

                    return makeSetActivity( AnimationFactory::createNumberPropertyAnimation(
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

                    return makeSetActivity( AnimationFactory::createEnumPropertyAnimation(
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

                    return makeSetActivity( AnimationFactory::createColorPropertyAnimation(
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

                    return makeSetActivity( AnimationFactory::createStringPropertyAnimation(
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

                    return makeSetActivity( AnimationFactory::createBoolPropertyAnimation(
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
