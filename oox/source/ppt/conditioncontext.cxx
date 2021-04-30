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

#include "conditioncontext.hxx"

#include <com/sun/star/animations/AnimationEndSync.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>

#include <oox/helper/attributelist.hxx>
#include <oox/ppt/animationspersist.hxx>
#include "animationtypes.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

#include "timetargetelementcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::animations;

namespace oox::ppt {

    CondContext::CondContext( FragmentHandler2 const & rParent, const Reference< XFastAttributeList >& xAttribs,
                const TimeNodePtr & pNode, AnimationCondition & aValue )
        :  TimeNodeContext( rParent, PPT_TOKEN( cond ), pNode )
        , maCond( aValue )
    {
        maEvent.Trigger =  EventTrigger::NONE;
        maEvent.Repeat = 0;

        AttributeList attribs( xAttribs );
        if( attribs.hasAttribute( XML_evt ) )
        {
            sal_Int32 nEvent = xAttribs->getOptionalValueToken( XML_evt, 0 );
            switch( nEvent )
            {
            case XML_onBegin:
                maEvent.Trigger = EventTrigger::ON_BEGIN;
                break;
            case XML_onEnd:
                maEvent.Trigger = EventTrigger::ON_END;
                break;
            case XML_begin:
                maEvent.Trigger = EventTrigger::BEGIN_EVENT;
                break;
            case XML_end:
                maEvent.Trigger = EventTrigger::END_EVENT;
                break;
            case XML_onClick:
                maEvent.Trigger = EventTrigger::ON_CLICK;
                break;
            case XML_onDblClick:
                maEvent.Trigger = EventTrigger::ON_DBL_CLICK;
                break;
            case XML_onMouseOver:
                maEvent.Trigger = EventTrigger::ON_MOUSE_ENTER;
                break;
            case XML_onMouseOut:
                maEvent.Trigger = EventTrigger::ON_MOUSE_LEAVE;
                break;
            case XML_onNext:
                maEvent.Trigger = EventTrigger::ON_NEXT;
                break;
            case XML_onPrev:
                maEvent.Trigger = EventTrigger::ON_PREV;
                break;
            case XML_onStopAudio:
                maEvent.Trigger = EventTrigger::ON_STOP_AUDIO;
                break;
            default:
                break;
            }
        }
        if( attribs.hasAttribute( XML_delay ) || ( maEvent.Trigger == EventTrigger::NONE ) )
        {
            maEvent.Offset = GetTime( xAttribs->getOptionalValue( XML_delay ) );
        }
    }

    CondContext::~CondContext( ) noexcept
    {
        if( maCond.mnType == 0 || maCond.mnType == PPT_TOKEN(tn))
        {
            maCond.maValue = (maEvent.Trigger == EventTrigger::NONE) ? maEvent.Offset : makeAny( maEvent );
        }
    }

    ::oox::core::ContextHandlerRef CondContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( rtn ):
        {
            // ST_TLTriggerRuntimeNode { first, last, all }
            sal_Int32 aTok;
            sal_Int16 nEnum;
            aTok = rAttribs.getToken( XML_val, XML_first );
            switch( aTok )
            {
            case XML_first:
                nEnum = AnimationEndSync::FIRST;
                break;
            case XML_last:
                nEnum = AnimationEndSync::LAST;
                break;
            case XML_all:
                nEnum = AnimationEndSync::ALL;
                break;
            default:
                break;
            }
            maCond.mnType = aElementToken;
            maCond.maValue <<= nEnum;
            return this;
        }
        case PPT_TOKEN( tn ):
        {
            maCond.mnType = aElementToken;
            // Convert the node id string to XAnimationNode later
            maEvent.Source <<= rAttribs.getString(XML_val, OUString());
            return this;
        }
        case PPT_TOKEN( tgtEl ):
            // CT_TLTimeTargetElement
            return new TimeTargetElementContext( *this, maCond.getTarget() );
        default:
            break;
        }

        return this;

    }

    /** CT_TLTimeConditionList */
    CondListContext::CondListContext(
            FragmentHandler2 const & rParent, sal_Int32  aElement,
            const TimeNodePtr & pNode,
            AnimationConditionList & aCond )
        : TimeNodeContext( rParent, aElement, pNode )
        , maConditions( aCond )
    {
    }

    CondListContext::~CondListContext( )
        noexcept
    {
    }

    ::oox::core::ContextHandlerRef CondListContext::onCreateContext( sal_Int32 aElement, const AttributeList& rAttribs )
    {
        switch( aElement )
        {
        case PPT_TOKEN( cond ):
            // add a condition to the list
            maConditions.emplace_back( );
            return new CondContext( *this, rAttribs.getFastAttributeList(), mpNode, maConditions.back() );
        default:
            break;
        }
        return this;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
