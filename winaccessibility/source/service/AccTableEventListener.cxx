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
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>

#include "AccTableEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccTableEventListener::AccTableEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccDescendantManagerEventListener(pAcc, Agent)
{}
AccTableEventListener::~AccTableEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccTableEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{
    switch (aEvent.EventId)
    {
    case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
        handleActiveDescendantChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;

    case AccessibleEventId::TABLE_CAPTION_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_CAPTION_CHANGED, pAccessible);
            break;
        }
    case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_COLUMN_DESCRIPTION_CHANGED, pAccessible);
            break;
        }
    case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_COLUMN_HEADER_CHANGED, pAccessible);
            break;
        }
    case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_ROW_HEADER_CHANGED, pAccessible);
            break;
        }
    case AccessibleEventId::TABLE_MODEL_CHANGED:
        {

            handleTableModelChangeEvent(aEvent.NewValue);
            break;
        }
    case AccessibleEventId::TABLE_SUMMARY_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_SUMMARY_CHANGED, pAccessible);
            break;
        }
    case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
        {

            pAgent->NotifyAccEvent(UM_EVENT_TABLE_ROW_DESCRIPTION_CHANGED, pAccessible);
            break;
        }
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
void AccTableEventListener::handleActiveDescendantChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if(newValue >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->InsertAccObj(pAcc,pAccessible);
            pAgent->NotifyAccEvent(UM_EVENT_ACTIVE_DESCENDANT_CHANGED, pAcc);
        }
    }
    else if (oldValue >>= xChild)
    {
        //delete an existing child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->DeleteAccObj( pAcc );
        }
    }

}
void AccTableEventListener::handleTableModelChangeEvent(Any newValue)
{
    AccessibleTableModelChange aModelChange;
    if (newValue >>= aModelChange)
    {
        if( pAccessible )
        {
            //delete all oldValue's existing children
            pAgent->DeleteChildrenAccObj( pAccessible );
            //add all oldValue's existing children
            pAgent->InsertChildrenAccObj( pAccessible );
        }
        pAgent->NotifyAccEvent(UM_EVENT_TABLE_MODEL_CHANGED, pAccessible);
    }
}
