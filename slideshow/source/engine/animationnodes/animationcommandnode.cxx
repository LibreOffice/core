/*************************************************************************
 *
 *  $RCSfile: animationcommandnode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-18 17:11:24 $
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

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTCOMMANDS_HPP_
#include <com/sun/star/presentation/EffectCommands.hpp>
#endif

#include <animationcommandnode.hxx>
#include <delayevent.hxx>
#include <tools.hxx>
#include <nodetools.hxx>
#include <generateevent.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::presentation;

namespace presentation
{
    namespace internal
    {
        AnimationCommandNode::AnimationCommandNode( const uno::Reference< animations::XAnimationNode >& xNode,
                                                const BaseContainerNodeSharedPtr&                   rParent,
                                                const NodeContext&                                  rContext ) :
            BaseNode( xNode, rParent, rContext ),
            mxCommandNode( xNode, uno::UNO_QUERY_THROW )
        {
            ENSURE_AND_THROW( getContext().mxComponentContext.is(),
                              "AnimationCommandNode::AnimationCommandNode(): Invalid component context" );
        }

        void AnimationCommandNode::dispose()
        {
            mxCommandNode.clear();
            BaseNode::dispose();
        }

        bool AnimationCommandNode::activate()
        {
            if( !BaseNode::activate() )
                return false;

            switch( mxCommandNode->getCommand() )
            {
            // the command is user defined
            case EffectCommands::CUSTOM: break;

            // the command is an ole verb.
            case EffectCommands::VERB: break;

            // the command starts playing on a media object
            case EffectCommands::PLAY: break;

            // the command toggles the pause status on a media object
            case EffectCommands::TOGGLEPAUSE: break;

            // the command stops the animation on a media object
            case EffectCommands::STOP: break;

            // the command stops all currently running sound effects
            case EffectCommands::STOPAUDIO:
                getContext().mrEventMultiplexer.notifyCommandStopAudio( getSelf() );
                break;
            }

            getContext().mrEventQueue.addEvent(
                makeEvent( ::boost::bind(&BaseNode::deactivate,
                                            ::boost::cref( getSelf() ) ) ) );
            return true;
        }

        void AnimationCommandNode::deactivate()
        {
            BaseNode::deactivate();
        }

        void AnimationCommandNode::notifyDeactivating( const AnimationNodeSharedPtr& rNotifier )
        {
            // NO-OP for all leaf nodes (which typically don't register nowhere)
        }

        bool AnimationCommandNode::hasPendingAnimation() const
        {
            return false;
        }
    }
}
