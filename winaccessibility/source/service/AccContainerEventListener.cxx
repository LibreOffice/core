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

#include "AccContainerEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccContainerEventListener::AccContainerEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
        :AccEventListener(pAcc, Agent)
{
}

AccContainerEventListener::~AccContainerEventListener()
{
}

/**
 *  Uno's event notifier when event is captured
 *
 *  @param AccessibleEventObject    the event object which contains information about event
 */
void  AccContainerEventListener::notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{
    short role = getRole();
    switch (aEvent.EventId)
    {
    case AccessibleEventId::CHILD:
        handleChildChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED:
        handleSelectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        handleAllChildrenChangedEvent();
        break;
    case AccessibleEventId::TEXT_CHANGED:
        handleTextChangedEvent(aEvent.OldValue, aEvent.NewValue);
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        handleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        handleBoundrectChangedEvent();
        break;
    case AccessibleEventId::STATE_CHANGED:
        handleStateChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::VALUE_CHANGED:
        handleValueChangedEvent(aEvent.OldValue, aEvent.NewValue);
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
    case AccessibleEventId::PAGE_CHANGED:
        handlePageChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SECTION_CHANGED:
        handleSectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::COLUMN_CHANGED:
        handleColumnChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    default:
        AccEventListener::notifyEvent(aEvent);
        break;
    }
}

/**
 *  handle the VISIBLE_DATA_CHANGED event
 */
void AccContainerEventListener::handleVisibleDataChangedEvent()
{
    AccEventListener::handleVisibleDataChangedEvent();
}

/**
 *  handle the BOUNDRECT_CHANGED event
 */
void AccContainerEventListener::handleBoundrectChangedEvent()
{
    AccEventListener::handleBoundrectChangedEvent();
}

void AccContainerEventListener::handleStateChangedEvent(Any oldValue, Any newValue)
{
    short State;
    if( newValue >>= State)
    {
        setComponentState( State,true);
    }
    else if (oldValue >>= State)
    {
        setComponentState( State,false);
    }

}

/**
 * handle the CHILD event
 * @param   oldValue    the child to be deleted
 * @param   newValue    the child to be added
 */
void AccContainerEventListener::handleChildChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if( newValue >>= xChild)
    {
        //create a new child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            //add this child

            if(pAgent->InsertAccObj( pAcc,pAccessible))
            {
                //add all oldValue's existing children
                pAgent->InsertChildrenAccObj(pAcc);
                pAgent->NotifyAccEvent(UM_EVENT_CHILD_ADDED, pAcc);
            }
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
 * handle the SELECTION_CHANGED event
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccContainerEventListener::handleSelectionChangedEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED,newValue))
    {
        return ;
    }

    //menu bar does not process selection change event,just same as word behavior
    if(getRole()!=AccessibleRole::MENU_BAR)
        pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, pAccessible);
}

/**
 *  handle the INVALIDATE_ALL_CHILDREN event
 */
void AccContainerEventListener::handleAllChildrenChangedEvent()
{
    //TODO: update all the children
    if( pAccessible )
    {
        //delete all oldValue's existing children
        pAgent->DeleteChildrenAccObj( pAccessible );
        //add all oldValue's existing children
        pAgent->InsertChildrenAccObj( pAccessible );
        pAgent->NotifyAccEvent(UM_EVENT_OBJECT_REORDER , pAccessible);
    }
}

/**
 *  handle the TEXT_CHANGED event
 */
void AccContainerEventListener::handleTextChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(pAccessible, newValue);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_TEXTCHANGE, pAccessible);
}

/**
 * set the new state and fire the MSAA event
 * @param   state   new state id
 * @param   enable  true if state is set, false if state is unset
 */
void AccContainerEventListener::setComponentState(short state, bool enable )
{
    // only the following state can be fired state event.

    switch (state)
    {
    case AccessibleStateType::SELECTED:
    case AccessibleStateType::BUSY:
    case AccessibleStateType::INDETERMINATE:
    case AccessibleStateType::OFFSCREEN:
    case AccessibleStateType::FOCUSABLE:
    case AccessibleStateType::SHOWING:
    case AccessibleStateType::VISIBLE:
        fireStatePropertyChange(state, enable);
        break;
    case AccessibleStateType::FOCUSED:
        fireStateFocusdChange(enable);
        break;
    case AccessibleStateType::ENABLED:
        if(enable)
        {
            pAgent->DecreaseState( pAccessible, AccessibleStateType::DEFUNC);
            pAgent->IncreaseState( pAccessible, AccessibleStateType::FOCUSABLE);
            pAgent->UpdateState(pAccessible);

            UpdateAllChildrenState(pAccessible);
        }
        else
        {
            pAgent->IncreaseState( pAccessible, AccessibleStateType::DEFUNC);
            pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSABLE);
            pAgent->UpdateState(pAccessible);

            UpdateAllChildrenState(pAccessible);
        }
        break;
    case AccessibleStateType::ACTIVE:
        // Only frames should be active
        // no msaa state mapping
        //for PAGE_TAB_LIST, there will be ACTIVE state, then it should be converted to FOCUSED event.
        if(getRole() == AccessibleRole::PAGE_TAB_LIST)
        {
            if (!enable) /* get the active state */
            {
                pAgent->IncreaseState( pAccessible, AccessibleStateType::FOCUSED);
            }

            else    /* lose the active state */
            {
                pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSED);
            }
        }
        break;

    case AccessibleStateType::EXPANDED:
    case AccessibleStateType::COLLAPSE:
    case AccessibleStateType::CHECKED:
        {
            pAgent->UpdateState(pAccessible);
            pAgent->NotifyAccEvent(UM_EVENT_STATE_BUSY, pAccessible);
            break;
        }

    default:
        break;
    }
}

/**
 * fire the MSAA state changed event
 * @param   state   the state id
 * @param   set     true if state is set, false if state is unset
 */
void AccContainerEventListener::fireStatePropertyChange(short state, bool set)
{
    if( set )
    {
        // new value
        switch(state)
        {
        case AccessibleStateType::SELECTED:
            pAgent->IncreaseState( pAccessible, state);
            break;
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::BUSY:
        case AccessibleStateType::FOCUSABLE:
        case AccessibleStateType::OFFSCREEN:
            pAgent->IncreaseState( pAccessible, state);
            pAgent->NotifyAccEvent(UM_EVENT_STATE_BUSY, pAccessible);
            break;
        case AccessibleStateType::SHOWING:
            // UNO !SHOWING == MSAA OFFSCREEN
            pAgent->IncreaseState( pAccessible, AccessibleStateType::SHOWING );
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            pAgent->IncreaseState( pAccessible, AccessibleStateType::VISIBLE );
            break;
        default:
            break;
        }
    }
    else
    {
        // old value
        switch(state)
        {
        case AccessibleStateType::SELECTED:
            pAgent->DecreaseState( pAccessible, state );
            break;
        case AccessibleStateType::BUSY:
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::FOCUSABLE:
        case AccessibleStateType::OFFSCREEN:
            pAgent->DecreaseState( pAccessible, state);
            pAgent->NotifyAccEvent(UM_EVENT_STATE_BUSY, pAccessible);
            break;
        case AccessibleStateType::SHOWING:
            // UNO !SHOWING == MSAA OFFSCREEN
            pAgent->DecreaseState( pAccessible, AccessibleStateType::SHOWING );
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            pAgent->DecreaseState( pAccessible, AccessibleStateType::VISIBLE );
            break;
        default:
            break;
        }
    }
}

/**
 * handle the focused event
 * @param   enable  true if get focus, false if lose focus
 */
void AccContainerEventListener::fireStateFocusdChange(bool enable)
{
    if(enable)
    {
        pAgent->IncreaseState( pAccessible, AccessibleStateType::FOCUSED);
        //if the acc role is MENU_BAR, MSAA UM_EVENT_MENU_START event should be sent
        //if the acc role is POPUP_MENU, MSAA UM_EVENT_MENUPOPUPSTART event should be sent
        short role = getRole();
        if(role == AccessibleRole::MENU_BAR)
        {
            pAgent->NotifyAccEvent(UM_EVENT_MENU_START, pAccessible);
        }
        else if (role == AccessibleRole::POPUP_MENU)
            pAgent->NotifyAccEvent(UM_EVENT_MENUPOPUPSTART, pAccessible);
        //Disable the focused event on option_pane and Panel.
        //only disable option_pane for toolbar has panel to get focus
        else if (role == AccessibleRole::PANEL || role == AccessibleRole::OPTION_PANE )
        {
            //don't send focused event on PANEL & OPTION_PANE if the parent is not toolbar
            short parentRole = getParentRole();
            if (parentRole == AccessibleRole::TOOL_BAR
                || parentRole == AccessibleRole::SCROLL_PANE // sidebar
                || parentRole == AccessibleRole::PANEL) // sidebar
                pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, pAccessible);
        }
        //to update ComboBox's description
        else if (role == AccessibleRole::COMBO_BOX )
        {
            pAgent->UpdateDescription(pAccessible);
            //for editable combobox, send focus event on only edit control,
            bool bSendFocusOnCombobox = true;
            //send focused event to the first text child
            Reference<XAccessibleContext> mxContext(pAccessible->getAccessibleContext(),UNO_QUERY);
            if(mxContext.is())
            {
                Reference<XAccessible> mxChild = mxContext->getAccessibleChild(0);
                if(mxChild.is())
                {
                    Reference<XAccessibleContext> mxChildContext(mxChild->getAccessibleContext(),UNO_QUERY);
                    short childrole = mxChildContext->getAccessibleRole();
                    if (childrole == AccessibleRole::TEXT)
                    {
                        if (IsEditable(mxChildContext))
                        {
                            pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSED);
                            pAgent->IncreaseState( mxChild.get(), AccessibleStateType::FOCUSED);
                            pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, mxChild.get());
                            bSendFocusOnCombobox = false;
                        }
                    }
                }
            }
            if (bSendFocusOnCombobox)
                pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, pAccessible);
        }
        else
            pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, pAccessible);
    }
    else
    {
        pAgent->DecreaseState( pAccessible, AccessibleStateType::FOCUSED);
        //if the acc role is MENU_BAR, MSAA UM_EVENT_MENU_END event should be sent
        //if the acc role is POPUP_MENU, MSAA UM_EVENT_MENUPOPUPEND event should be sent
        if(getRole() == AccessibleRole::MENU_BAR)
        {
            pAgent->NotifyAccEvent(UM_EVENT_MENU_END, pAccessible);
        }
        else if (getRole() == AccessibleRole::POPUP_MENU)
        {
            pAgent->NotifyAccEvent(UM_EVENT_MENUPOPUPEND, pAccessible);
        }
    }
}

/**
 * handle the VALUE_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccContainerEventListener::handleValueChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(pAccessible);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, pAccessible);
}

bool AccContainerEventListener::IsEditable(Reference<XAccessibleContext> xContext)
{
    bool ret = false;
    Reference< XAccessibleStateSet > pRState = xContext->getAccessibleStateSet();
    if( !pRState.is() )
        return false;

    Sequence<short> pStates = pRState->getStates();
    int count = pStates.getLength();
    for( int iIndex = 0;iIndex < count;iIndex++ )
    {
        if(pStates[iIndex] == AccessibleStateType::EDITABLE)
            return true;
    }
    return ret;
}

bool AccContainerEventListener::NotifyChildEvent(short nWinEvent,const Any &Value)
{
    Reference< XAccessible > xChild;
    if(Value >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            pAgent->NotifyAccEvent(nWinEvent, pAcc);
            return true;
        }
    }
    return false;
}

void AccContainerEventListener::handleSelectionChangedAddEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_ADD,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_ADD,pAccessible);
}
void AccContainerEventListener::handleSelectionChangedRemoveEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_REMOVE,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_REMOVE,pAccessible);
}
void AccContainerEventListener::handleSelectionChangedWithinEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_WITHIN,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_WITHIN,pAccessible);
}

void SAL_CALL AccContainerEventListener::UpdateAllChildrenState( com::sun::star::accessibility::XAccessible* pXAccessible )
{
    Reference<com::sun::star::accessibility::XAccessibleContext> xContext(pXAccessible->getAccessibleContext(),UNO_QUERY);
    if(!xContext.is())
    {
        return;
    }
    com::sun::star::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();
    if(pAccessibleContext == NULL)
    {
        return;
    }

    if (pAgent && pAgent->IsStateManageDescendant(pXAccessible))
    {
        return;
    }

    int count = pAccessibleContext->getAccessibleChildCount();
    for (int i=0;i<count;i++)
    {
        Reference<com::sun::star::accessibility::XAccessible> mxAccessible
        = pAccessibleContext->getAccessibleChild(i);

        com::sun::star::accessibility::XAccessible* mpAccessible = mxAccessible.get();
        if(mpAccessible != NULL)
        {
            pAgent->UpdateState(mpAccessible);
            UpdateAllChildrenState(mpAccessible);
        }
    }
}


void AccContainerEventListener::handlePageChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/)
{
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_PAGECHANGED, pAccessible);
}

void AccContainerEventListener::handleSectionChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/ )
{
    pAgent->NotifyAccEvent(UM_EVENT_SECTION_CHANGED, pAccessible);
}

void AccContainerEventListener::handleColumnChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/)
{
    pAgent->NotifyAccEvent(UM_EVENT_COLUMN_CHANGED, pAccessible);
}

//IAccessibility2 Implementation 2009-----
void  AccContainerEventListener::handleNameChangedEvent( Any name )
{
    if (getRole() == AccessibleRole::COMBO_BOX)
    {
        Reference<XAccessibleContext> mxContext(pAccessible->getAccessibleContext(),UNO_QUERY);
        if(mxContext.is())
        {
            Reference<XAccessible> mxChild = mxContext->getAccessibleChild(0);
            if(mxChild.is())
            {
                Reference<XAccessibleContext> mxChildContext(mxChild->getAccessibleContext(),UNO_QUERY);
                short childrole = mxChildContext->getAccessibleRole();
                if (childrole == AccessibleRole::TEXT)
                {
                    pAgent->UpdateAccName(mxChild.get(), name);
                }
            }
        }
    }
    AccEventListener::handleNameChangedEvent(name);
}
//-----IAccessibility2 Implementation 2009
