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

#include "AccDescendantManagerEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccDescendantManagerEventListener::AccDescendantManagerEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccComponentEventListener(pAcc, Agent),
        pActiveDescendant(NULL)
{
}

AccDescendantManagerEventListener::~AccDescendantManagerEventListener()
{
}

/**
 * Uno's event notifier when event is captured
 * @param AccessibleEventObject     the event object which contains information about event
 */
void  AccDescendantManagerEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{
    switch (aEvent.EventId)
    {
    case AccessibleEventId::SELECTION_CHANGED:
        handleSelectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::CHILD:
        handleChildChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS:
        handleChildChangedNoFocusEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_ADD:
        handleSelectionChangedAddEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_REMOVE:
        handleSelectionChangedRemoveEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_WITHIN:
        handleSelectionChangedWithinEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    default:
        AccComponentEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the CHILD event
 *  @param  oldValue    the child to be deleted
 *  @param  newValue    the child to be added
 */
void AccDescendantManagerEventListener::handleChildChangedEvent(Any oldValue, Any newValue)
{

    Reference< XAccessible > xChild;
    if( newValue >>= xChild)
    {
        //create a new child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->InsertAccObj( pAcc,pAccessible);
            pAgent->InsertChildrenAccObj(pAcc);

            pAgent->NotifyAccEvent(UM_EVENT_CHILD_ADDED, pAcc);

        }
        else
        {}
    }

    if (oldValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();

            pAgent->NotifyAccEvent(UM_EVENT_CHILD_REMOVED, pAcc);
            pAgent->DeleteChildrenAccObj( pAcc );
            pAgent->DeleteAccObj( pAcc );
        }
        else
        {}
    }

}

/**
 *  handle the SELECTION_CHANGED event
 */
void AccDescendantManagerEventListener::handleSelectionChangedEvent(Any oldValue, Any newValue)
{
    bool bSend =false;
    Reference< XAccessible > xChild;
    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            //if the Role is the SC cell ,don't add the selected state.
            if (pAgent->GetRole(pAcc) != AccessibleRole::TABLE_CELL)
            {
                pAgent->IncreaseState( pAcc, AccessibleStateType::SELECTED);
            }

            pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, pAcc);
            bSend=true;
        }
    }
    if(oldValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->DecreaseState( pAcc, AccessibleStateType::SELECTED);
        }
    }
    if (!bSend)
    {
        pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, pAccessible);
    }
}


void AccDescendantManagerEventListener::handleChildChangedNoFocusEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();

            pAgent->InsertAccObj(pAcc,pAccessible);
            pAgent->InsertChildrenAccObj(pAcc);

            pActiveDescendant= pAcc;
        }
    }
    if (oldValue >>= xChild)
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->DeleteChildrenAccObj( pAcc );
            pAgent->DeleteAccObj( pAcc );
        }
    }
}

bool AccDescendantManagerEventListener::NotifyChildEvent(short nWinEvent,const Any &Value)
{
    Reference< XAccessible > xChild;
    if(Value >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->NotifyAccEvent(nWinEvent, pAcc);

            if (pAgent && pAgent->IsStateManageDescendant(pAccessible) && ( nWinEvent == UM_EVENT_SELECTION_CHANGED_REMOVE) )
            {
                pAgent->DeleteAccObj( pAcc );
            }
            return true;
        }
    }
    return false;
}
void AccDescendantManagerEventListener::handleSelectionChangedAddEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_ADD,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_ADD,pAccessible);
}
void AccDescendantManagerEventListener::handleSelectionChangedRemoveEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_REMOVE,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_REMOVE,pAccessible);
}

void AccDescendantManagerEventListener::handleSelectionChangedWithinEvent(const Any& /*oldValue*/, const Any &newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_WITHIN,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_WITHIN,pAccessible);
}

