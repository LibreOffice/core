/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include "pptx-animations-cond.hxx"

using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::uno;

namespace
{
const char* convertEventTrigger(sal_Int16 nTrigger)
{
    const char* pEvent = nullptr;
    switch (nTrigger)
    {
        case EventTrigger::ON_NEXT:
            pEvent = "onNext";
            break;
        case EventTrigger::ON_PREV:
            pEvent = "onPrev";
            break;
        case EventTrigger::BEGIN_EVENT:
            pEvent = "begin";
            break;
        case EventTrigger::END_EVENT:
            pEvent = "end";
            break;
        case EventTrigger::ON_BEGIN:
            pEvent = "onBegin";
            break;
        case EventTrigger::ON_END:
            pEvent = "onEnd";
            break;
        case EventTrigger::ON_CLICK:
            pEvent = "onClick";
            break;
        case EventTrigger::ON_DBL_CLICK:
            pEvent = "onDblClick";
            break;
        case EventTrigger::ON_STOP_AUDIO:
            pEvent = "onStopAudio";
            break;
        case EventTrigger::ON_MOUSE_ENTER:
            pEvent = "onMouseOver"; // not exact?
            break;
        case EventTrigger::ON_MOUSE_LEAVE:
            pEvent = "onMouseOut";
            break;
    }
    return pEvent;
}
}

namespace oox::core
{
Cond::Cond(const Any& rAny, bool bIsMainSeqChild)
    : mpEvent(nullptr)
{
    bool bHasFDelay = false;
    double fDelay = 0;
    Timing eTiming;
    Event aEvent;

    if (rAny >>= eTiming)
    {
        if (eTiming == Timing_INDEFINITE)
            msDelay = "indefinite"_ostr;
    }
    else if (rAny >>= aEvent)
    {
        if (aEvent.Trigger == EventTrigger::ON_NEXT && bIsMainSeqChild)
            msDelay = "indefinite"_ostr;
        else
        {
            mpEvent = convertEventTrigger(aEvent.Trigger);
            if (!(aEvent.Source >>= mxShape))
                aEvent.Source >>= mxNode;

            if (aEvent.Offset >>= fDelay)
                bHasFDelay = true;
        }
    }
    else if (rAny >>= fDelay)
        bHasFDelay = true;

    if (bHasFDelay)
    {
        sal_Int32 nDelay = static_cast<sal_uInt32>(fDelay * 1000.0);
        msDelay = OString::number(nDelay);
    }
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
