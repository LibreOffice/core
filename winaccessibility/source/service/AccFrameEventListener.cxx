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

#include "AccFrameEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

#include <vcl/window.hxx>
#include <toolkit/awt/Vclxwindow.hxx>

#ifndef _SV_SYSDATA_HXX
#if defined( WIN ) || defined( WNT ) || defined( OS2 )
typedef sal_Int32 HWND;
typedef sal_Int32 HMENU;
typedef sal_Int32 HDC;
typedef void *PVOID;
typedef PVOID HANDLE;
typedef HANDLE HFONT;
#endif
#include <vcl/sysdata.hxx>
#endif


AccFrameEventListener::AccFrameEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccEventListener(pAcc, Agent)
{
}

AccFrameEventListener::~AccFrameEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccFrameEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{
    switch (aEvent.EventId)
    {
    case AccessibleEventId::CHILD:
        handleChildChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        handleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        handleBoundrectChangedEvent();
        break;
    default:
        AccEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the VISIBLE_DATA_CHANGED event
 */
void AccFrameEventListener::handleVisibleDataChangedEvent()
{
    AccEventListener::handleVisibleDataChangedEvent();
}

/**
 *  handle the BOUNDRECT_CHANGED event
 */
void AccFrameEventListener::handleBoundrectChangedEvent()
{
    AccEventListener::handleBoundrectChangedEvent();
}

/**
 *  handle the CHILD event
 *  @param  oldValue    the child to be deleted
 *  @param  newValue    the child to be added
 */
void AccFrameEventListener::handleChildChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if( newValue >>= xChild)
    {
        //create a new child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();

            VCLXWindow* pvclwindow = (VCLXWindow*)pAccessible;
            Window* window = pvclwindow->GetWindow();
            const SystemEnvData* systemdata=window->GetSystemData();

            //add this child
            pAgent->InsertAccObj( pAcc,pAccessible,(HWND)systemdata->hWnd);
            //add all oldValue's existing children
            pAgent->InsertChildrenAccObj(pAcc);
            pAgent->NotifyAccEvent(UM_EVENT_CHILD_ADDED, pAcc);
        }
        else
        {}
    }
    else if (oldValue >>= xChild)
    {
        //delete a existing child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->NotifyAccEvent(UM_EVENT_CHILD_REMOVED, pAcc);
            //delete all oldValue's existing children
            pAgent->DeleteChildrenAccObj( pAcc );
            //delete this child
            pAgent->DeleteAccObj( pAcc );
        }
        else
        {}
    }

}

/**
 *  set the new state and fire the MSAA event
 *  @param state    new state id
 *  @param enable   true if state is set, false if state is unset
 */
void AccFrameEventListener::setComponentState(short state, bool enable )
{
    // only the following state can be fired state event.
    switch (state)
    {
    case AccessibleStateType::ICONIFIED:
        // no msaa state
        break;
    case AccessibleStateType::VISIBLE:
        // UNO !VISIBLE == MSAA INVISIBLE
        if( enable )
            pAgent->IncreaseState( pAccessible, AccessibleStateType::VISIBLE );
        else
            pAgent->DecreaseState( pAccessible, AccessibleStateType::VISIBLE );
        break;
    case AccessibleStateType::ACTIVE:
        // Only frames should be active
        // no msaa state mapping
        break;
    default:
        break;
    }
}
