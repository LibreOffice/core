/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// AccObjectContainerEventListener.cpp: implementation of the AccContainerEventListener class.
//
//////////////////////////////////////////////////////////////////////

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include "AccObjectContainerEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccObjectContainerEventListener::AccObjectContainerEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccContainerEventListener(pAcc, Agent)
{}
AccObjectContainerEventListener::~AccObjectContainerEventListener()
{
}

/**
 *  handle the STATE_CHANGED event
 */
void AccObjectContainerEventListener::handleStateChangedEvent(Any oldValue, Any newValue)
{
    //set the accessible name before process for there is no NAME_CHANGED event when change
    //the text in drawing objects.
    short newV;
    if( newValue >>= newV)
    {
        if (newV == AccessibleStateType::FOCUSED)
        {
            pAgent->UpdateAccName(pAccessible);

            pAgent->UpdateDescription(pAccessible);
        }
    }
    AccContainerEventListener::handleStateChangedEvent(oldValue, newValue);
}
/**
 *  handle the VISIBLE_DATA_CHANGED event
 *  For SHAPES, the visiable_data_changed event should be mapped to LOCATION_CHANGED event
  */
void AccObjectContainerEventListener::handleVisibleDataChangedEvent()
{
    AccContainerEventListener::handleBoundrectChangedEvent();
}
