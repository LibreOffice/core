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

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/Event.hpp>

#include "generateevent.hxx"
#include "shape.hxx"
#include "subsettableshapemanager.hxx"
#include "usereventqueue.hxx"
#include "slideshowcontext.hxx"
#include "delayevent.hxx"

namespace slideshow {
namespace internal {

using namespace com::sun::star;

EventSharedPtr generateEvent(
    uno::Any const& rEventDescription,
    Delay::FunctorT const& rFunctor,
    SlideShowContext const& rContext,
    double nAdditionalDelay )
{
    EventSharedPtr pEvent;

    if (! rEventDescription.hasValue())
        return pEvent;

    animations::Timing eTiming;
    animations::Event aEvent;
    uno::Sequence<uno::Any> aSequence;
    double nDelay1 = 0;

    if (rEventDescription >>= eTiming) {
        switch (eTiming) {
        case animations::Timing_INDEFINITE:
            break; // don't schedule no event
        case animations::Timing_MEDIA:
            OSL_FAIL( "MEDIA timing not yet implemented!" );
            break;
        default:
            ENSURE_OR_THROW( false, "unexpected case!" );
        }
    }
    else if (rEventDescription >>= aEvent) {

        // try to extract additional event delay
        double nDelay2 = 0.0;
        if (aEvent.Offset.hasValue() && !(aEvent.Offset >>= nDelay2)) {
            OSL_FAIL( "offset values apart from DOUBLE not "
                        "recognized in animations::Event!" );
        }

        // common vars used inside switch
        uno::Reference<animations::XAnimationNode> xNode;
        uno::Reference<drawing::XShape> xShape;
        ShapeSharedPtr pShape;

        // TODO(F1): Respect aEvent.Repeat value

        switch (aEvent.Trigger) {
        default:
            ENSURE_OR_THROW( false, "unexpected event trigger!" );
        case animations::EventTrigger::NONE:
            // no event at all
            break;
        case animations::EventTrigger::ON_BEGIN:
            OSL_FAIL( "event trigger ON_BEGIN not yet implemented!" );
            break;
        case animations::EventTrigger::ON_END:
            OSL_FAIL( "event trigger ON_END not yet implemented!" );
            break;
        case animations::EventTrigger::BEGIN_EVENT:
            // try to extract XAnimationNode event source
            if (aEvent.Source >>= xNode) {
                pEvent = makeDelay( rFunctor,
                                    nDelay2 + nAdditionalDelay,
                                    "generateEvent, BEGIN_EVENT");
                rContext.mrUserEventQueue.registerAnimationStartEvent(
                    pEvent, xNode );
            }
            else {
                OSL_FAIL("could not extract source XAnimationNode "
                           "for BEGIN_EVENT!" );
            }
            break;
        case animations::EventTrigger::END_EVENT:
            // try to extract XAnimationNode event source
            if (aEvent.Source >>= xNode) {
                pEvent = makeDelay( rFunctor,
                                    nDelay2 + nAdditionalDelay,
                                    "generateEvent, END_EVENT");
                rContext.mrUserEventQueue.registerAnimationEndEvent(
                    pEvent, xNode );
            }
            else {
                OSL_FAIL( "could not extract source XAnimationNode "
                            "for END_EVENT!" );
            }
            break;
        case animations::EventTrigger::ON_CLICK:
            // try to extract XShape event source
            if ((aEvent.Source >>= xShape) &&
                (pShape = rContext.mpSubsettableShapeManager->lookupShape(xShape)).get())
            {
                pEvent = makeDelay( rFunctor,
                                    nDelay2 + nAdditionalDelay,
                                    "generateEvent, ON_CLICK");
                rContext.mrUserEventQueue.registerShapeClickEvent(
                    pEvent, pShape );
            }
            else {
                OSL_FAIL( "could not extract source XAnimationNode "
                            "for ON_CLICK!" );
            }
            break;
        case animations::EventTrigger::ON_DBL_CLICK:
            // try to extract XShape event source
            if ((aEvent.Source >>= xShape) &&
                (pShape = rContext.mpSubsettableShapeManager->lookupShape(xShape)).get())
            {
                pEvent = makeDelay( rFunctor,
                                    nDelay2 + nAdditionalDelay,
                                    "generateEvent, ON_DBL_CLICK");
                rContext.mrUserEventQueue.registerShapeDoubleClickEvent(
                    pEvent, pShape );
            }
            else {
                OSL_FAIL( "could not extract source XAnimationNode "
                            "for ON_DBL_CLICK!" );
            }
            break;
        case animations::EventTrigger::ON_MOUSE_ENTER:
            // try to extract XShape event source
            if ((aEvent.Source >>= xShape) &&
                (pShape = rContext.mpSubsettableShapeManager->lookupShape(xShape)).get())
            {
                pEvent = makeDelay( rFunctor,
                                    nDelay2 + nAdditionalDelay,
                                    "generateEvent, ON_MOUSE_ENTER");
                rContext.mrUserEventQueue.registerMouseEnterEvent(
                    pEvent, pShape );
            }
            else {
                OSL_FAIL( "could not extract source XAnimationNode "
                            "for ON_MOUSE_ENTER!" );
            }
            break;
        case animations::EventTrigger::ON_MOUSE_LEAVE:
            // try to extract XShape event source
            if ((aEvent.Source >>= xShape) &&
                (pShape = rContext.mpSubsettableShapeManager->lookupShape(xShape)).get())
            {
                pEvent = makeDelay( rFunctor,
                                    nDelay2 + nAdditionalDelay,
                                    "generateEvent, ON_MOUSE_LEAVE");
                rContext.mrUserEventQueue.registerMouseLeaveEvent(
                    pEvent, pShape );
            }
            else {
                OSL_FAIL( "could not extract source XAnimationNode "
                            "for ON_MOUSE_LEAVE!" );
            }
            break;
        case animations::EventTrigger::ON_PREV:
            OSL_FAIL( "event trigger ON_PREV not yet implemented, "
                        "mapped to ON_NEXT!" );
            SAL_FALLTHROUGH;
        case animations::EventTrigger::ON_NEXT:
            pEvent = makeDelay( rFunctor,
                                nDelay2 + nAdditionalDelay,
                                "generateEvent, ON_NEXT");
            rContext.mrUserEventQueue.registerNextEffectEvent( pEvent );
            break;
        case animations::EventTrigger::ON_STOP_AUDIO:
            // try to extract XAnimationNode event source
            if (aEvent.Source >>= xNode) {
                pEvent = makeDelay( rFunctor,
                                    nDelay2 + nAdditionalDelay,
                                    "generateEvent, ON_STOP_AUDIO");
                rContext.mrUserEventQueue.registerAudioStoppedEvent(
                    pEvent, xNode );
            }
            else {
                OSL_FAIL( "could not extract source XAnimationNode "
                            "for ON_STOP_AUDIO!" );
            }
            break;
        case animations::EventTrigger::REPEAT:
            OSL_FAIL( "event trigger REPEAT not yet implemented!" );
            break;
        }
    }
    else if (rEventDescription >>= aSequence) {
        OSL_FAIL( "sequence of timing primitives "
                    "not yet implemented!" );
    }
    else if (rEventDescription >>= nDelay1) {
        pEvent = makeDelay( rFunctor,
                            nDelay1 + nAdditionalDelay,
                            "generateEvent with delay");
        // schedule delay event
        rContext.mrEventQueue.addEvent( pEvent );
    }

    return pEvent;
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
