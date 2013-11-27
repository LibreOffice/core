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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include "AccListEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccListEventListener::AccListEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccDescendantManagerEventListener(pAcc, Agent),
        shouldDeleteChild(true)
{
}

AccListEventListener::~AccListEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccListEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    switch (aEvent.EventId)
    {
    case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
        handleActiveDescendantChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        // Since List items a transient a child events are mostly used
        // to attach/detach listeners, it is save to ignore it here
        //TODO: investigate again
        break;
    case AccessibleEventId::VALUE_CHANGED:
        handleValueChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    default:
        AccDescendantManagerEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the ACTIVE_DESCENDANT_CHANGED event
 *  @param  oldValue    the child to lose active
 *  @param  newValue    the child to get active
 */
void AccListEventListener::handleActiveDescendantChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;

    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();

            // Valueset has cache the child item xacc,Update state if no insert obj
            sal_Bool bHasCache = pAgent->InsertAccObj(pAcc,pAccessible);
            if (!bHasCache)
            {
                pAgent->UpdateState(pAcc);
            }

            pAgent->IncreaseState( pAcc, AccessibleStateType::FOCUSED);

            pAgent->NotifyAccEvent(UM_EVENT_ACTIVE_DESCENDANT_CHANGED, pAcc);
            pActiveDescendant= pAcc;
        }
    }
    if (oldValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->DeleteAccObj( pAcc );
        }
    }
}

/**
 * handle the VALUE_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccListEventListener::handleValueChangedEvent(Any oldValue, Any newValue)
{
    //to enable value changed event
    if(getParentRole() == AccessibleRole::COMBO_BOX)
    {
        XAccessible* pParentAcc = pAgent->GetParentXAccessible(pAccessible);
        pAgent->UpdateValue(pParentAcc);
        pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, pParentAcc);
    }
}
