/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationcommandnode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:30:55 $
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

// must be first
#include "canvas/debug.hxx"
#include "canvas/verbosetrace.hxx"
#include "com/sun/star/presentation/EffectCommands.hpp"
#include "animationcommandnode.hxx"
#include "delayevent.hxx"
#include "tools.hxx"
#include "nodetools.hxx"

namespace presentation {
namespace internal {

void AnimationCommandNode::dispose()
{
    mxCommandNode.clear();
    BaseNode::dispose();
}

void AnimationCommandNode::activate_()
{
    namespace EffectCommands = com::sun::star::presentation::EffectCommands;

    switch( mxCommandNode->getCommand() ) {
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

    // deactivate ASAP:
    scheduleDeactivationEvent(
        makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ) ) );
}

bool AnimationCommandNode::hasPendingAnimation() const
{
    return false;
}

} // namespace internal
} // namespace presentation
