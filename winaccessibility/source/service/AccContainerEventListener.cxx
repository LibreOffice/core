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

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include <vcl/svapp.hxx>

#include "AccContainerEventListener.hxx"
#include "AccObjectManagerAgent.hxx"
#include "unomsaaevent.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccContainerEventListener::AccContainerEventListener(css::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent)
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
void  AccContainerEventListener::notifyEvent( const css::accessibility::AccessibleEventObject& aEvent )
throw (css::uno::RuntimeException)
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::CHILD:
        HandleChildChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED:
        HandleSelectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        HandleAllChildrenChangedEvent();
        break;
    case AccessibleEventId::TEXT_CHANGED:
        HandleTextChangedEvent(aEvent.OldValue, aEvent.NewValue);
    case AccessibleEventId::VISIBLE_DATA_CHANGED:
        HandleVisibleDataChangedEvent();
        break;
    case AccessibleEventId::BOUNDRECT_CHANGED:
        HandleBoundrectChangedEvent();
        break;
    case AccessibleEventId::STATE_CHANGED:
        HandleStateChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::VALUE_CHANGED:
        HandleValueChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_ADD:
        HandleSelectionChangedAddEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_REMOVE:
        HandleSelectionChangedRemoveEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SELECTION_CHANGED_WITHIN:
        HandleSelectionChangedWithinEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::PAGE_CHANGED:
        HandlePageChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::SECTION_CHANGED:
        HandleSectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::COLUMN_CHANGED:
        HandleColumnChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    default:
        AccEventListener::notifyEvent(aEvent);
        break;
    }
}

void AccContainerEventListener::HandleStateChangedEvent(Any oldValue, Any newValue)
{
    short State;
    if( newValue >>= State)
    {
        SetComponentState(State, true);
    }
    else if (oldValue >>= State)
    {
        SetComponentState(State, false);
    }

}

/**
 * handle the CHILD event
 * @param   oldValue    the child to be deleted
 * @param   newValue    the child to be added
 */
void AccContainerEventListener::HandleChildChangedEvent(Any oldValue, Any newValue)
{
    Reference< XAccessible > xChild;
    if( newValue >>= xChild)
    {
        //create a new child
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            //add this child

            if (pAgent->InsertAccObj(pAcc, m_xAccessible.get()))
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
void AccContainerEventListener::HandleSelectionChangedEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED,newValue))
    {
        return ;
    }

    //menu bar does not process selection change event,just same as word behavior
    if (GetRole()!=AccessibleRole::MENU_BAR)
        pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED, m_xAccessible.get());
}

/**
 *  handle the INVALIDATE_ALL_CHILDREN event
 */
void AccContainerEventListener::HandleAllChildrenChangedEvent()
{
    //TODO: update all the children
    if (m_xAccessible.is())
    {
        //delete all oldValue's existing children
        pAgent->DeleteChildrenAccObj(m_xAccessible.get());
        //add all oldValue's existing children
        pAgent->InsertChildrenAccObj(m_xAccessible.get());
        pAgent->NotifyAccEvent(UM_EVENT_OBJECT_REORDER , m_xAccessible.get());
    }
}

/**
 *  handle the TEXT_CHANGED event
 */
void AccContainerEventListener::HandleTextChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(m_xAccessible.get(), newValue);
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_TEXTCHANGE, m_xAccessible.get());
}

/**
 * set the new state and fire the MSAA event
 * @param   state   new state id
 * @param   enable  true if state is set, false if state is unset
 */
void AccContainerEventListener::SetComponentState(short state, bool enable )
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
        FireStatePropertyChange(state, enable);
        break;
    case AccessibleStateType::FOCUSED:
        FireStateFocusedChange(enable);
        break;
    case AccessibleStateType::ENABLED:
        if(enable)
        {
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            pAgent->UpdateState(m_xAccessible.get());

            UpdateAllChildrenState(m_xAccessible.get());
        }
        else
        {
            pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            pAgent->UpdateState(m_xAccessible.get());

            UpdateAllChildrenState(m_xAccessible.get());
        }
        break;
    case AccessibleStateType::ACTIVE:
        // Only frames should be active
        // no msaa state mapping
        //for PAGE_TAB_LIST, there will be ACTIVE state, then it should be converted to FOCUSED event.
        if (GetRole() == AccessibleRole::PAGE_TAB_LIST)
        {
            if (!enable) /* get the active state */
            {
                pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
            }

            else    /* lose the active state */
            {
                pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
            }
        }
        break;

    case AccessibleStateType::EXPANDED:
    case AccessibleStateType::COLLAPSE:
    case AccessibleStateType::CHECKED:
        {
            pAgent->UpdateState(m_xAccessible.get());
            pAgent->NotifyAccEvent(UM_EVENT_STATE_BUSY, m_xAccessible.get());
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
void AccContainerEventListener::FireStatePropertyChange(short state, bool set)
{
    if( set )
    {
        // new value
        switch(state)
        {
        case AccessibleStateType::SELECTED:
            pAgent->IncreaseState(m_xAccessible.get(), state);
            break;
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::BUSY:
        case AccessibleStateType::FOCUSABLE:
        case AccessibleStateType::OFFSCREEN:
            pAgent->IncreaseState(m_xAccessible.get(), state);
            pAgent->NotifyAccEvent(UM_EVENT_STATE_BUSY, m_xAccessible.get());
            break;
        case AccessibleStateType::SHOWING:
            // UNO !SHOWING == MSAA OFFSCREEN
            pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING);
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
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
            pAgent->DecreaseState(m_xAccessible.get(), state);
            break;
        case AccessibleStateType::BUSY:
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::FOCUSABLE:
        case AccessibleStateType::OFFSCREEN:
            pAgent->DecreaseState(m_xAccessible.get(), state);
            pAgent->NotifyAccEvent(UM_EVENT_STATE_BUSY, m_xAccessible.get());
            break;
        case AccessibleStateType::SHOWING:
            // UNO !SHOWING == MSAA OFFSCREEN
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING);
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
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
void AccContainerEventListener::FireStateFocusedChange(bool enable)
{
    if(enable)
    {
        pAgent->IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
        //if the acc role is MENU_BAR, MSAA UM_EVENT_MENU_START event should be sent
        //if the acc role is POPUP_MENU, MSAA UM_EVENT_MENUPOPUPSTART event should be sent
        short role = GetRole();
        if(role == AccessibleRole::MENU_BAR)
        {
            pAgent->NotifyAccEvent(UM_EVENT_MENU_START, m_xAccessible.get());
        }
        else if (role == AccessibleRole::POPUP_MENU)
            pAgent->NotifyAccEvent(UM_EVENT_MENUPOPUPSTART, m_xAccessible.get());
        //Disable the focused event on option_pane and Panel.
        //only disable option_pane for toolbar has panel to get focus
        else if (role == AccessibleRole::PANEL || role == AccessibleRole::OPTION_PANE )
        {
            //don't send focused event on PANEL & OPTION_PANE if the parent is not toolbar
            short parentRole = GetParentRole();
            if (parentRole == AccessibleRole::TOOL_BAR
                || parentRole == AccessibleRole::SCROLL_PANE // sidebar
                || parentRole == AccessibleRole::PANEL) // sidebar
                pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, m_xAccessible.get());
        }
        //to update ComboBox's description
        else if (role == AccessibleRole::COMBO_BOX )
        {
            pAgent->UpdateDescription(m_xAccessible.get());
            //for editable combobox, send focus event on only edit control,
            bool bSendFocusOnCombobox = true;
            //send focused event to the first text child
            Reference<XAccessibleContext> mxContext(m_xAccessible.get()->getAccessibleContext(), UNO_QUERY);
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
                            pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
                            pAgent->IncreaseState( mxChild.get(), AccessibleStateType::FOCUSED);
                            pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, mxChild.get());
                            bSendFocusOnCombobox = false;
                        }
                    }
                }
            }
            if (bSendFocusOnCombobox)
                pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, m_xAccessible.get());
        }
        else
            pAgent->NotifyAccEvent(UM_EVENT_STATE_FOCUSED, m_xAccessible.get());
    }
    else
    {
        pAgent->DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
        //if the acc role is MENU_BAR, MSAA UM_EVENT_MENU_END event should be sent
        //if the acc role is POPUP_MENU, MSAA UM_EVENT_MENUPOPUPEND event should be sent
        if (GetRole() == AccessibleRole::MENU_BAR)
        {
            pAgent->NotifyAccEvent(UM_EVENT_MENU_END, m_xAccessible.get());
        }
        else if (GetRole() == AccessibleRole::POPUP_MENU)
        {
            pAgent->NotifyAccEvent(UM_EVENT_MENUPOPUPEND, m_xAccessible.get());
        }
    }
}

/**
 * handle the VALUE_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccContainerEventListener::HandleValueChangedEvent(Any oldValue, Any newValue)
{
    pAgent->UpdateValue(m_xAccessible.get());
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_VALUECHANGE, m_xAccessible.get());
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

void AccContainerEventListener::HandleSelectionChangedAddEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_ADD,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_ADD, m_xAccessible.get());
}

void AccContainerEventListener::HandleSelectionChangedRemoveEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_REMOVE,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_REMOVE, m_xAccessible.get());
}

void AccContainerEventListener::HandleSelectionChangedWithinEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if(NotifyChildEvent(UM_EVENT_SELECTION_CHANGED_WITHIN,newValue))
    {
        return ;
    }
    pAgent->NotifyAccEvent(UM_EVENT_SELECTION_CHANGED_WITHIN, m_xAccessible.get());
}

void AccContainerEventListener::UpdateAllChildrenState(XAccessible* pXAccessible)
{
    Reference<css::accessibility::XAccessibleContext> xContext(pXAccessible->getAccessibleContext(),UNO_QUERY);
    if(!xContext.is())
    {
        return;
    }
    css::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();
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
        Reference<css::accessibility::XAccessible> mxAccessible
        = pAccessibleContext->getAccessibleChild(i);

        css::accessibility::XAccessible* mpAccessible = mxAccessible.get();
        if(mpAccessible != NULL)
        {
            pAgent->UpdateState(mpAccessible);
            UpdateAllChildrenState(mpAccessible);
        }
    }
}

void AccContainerEventListener::HandlePageChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/)
{
    pAgent->NotifyAccEvent(UM_EVENT_OBJECT_PAGECHANGED, m_xAccessible.get());
}

void AccContainerEventListener::HandleSectionChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/ )
{
    pAgent->NotifyAccEvent(UM_EVENT_SECTION_CHANGED, m_xAccessible.get());
}

void AccContainerEventListener::HandleColumnChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/)
{
    pAgent->NotifyAccEvent(UM_EVENT_COLUMN_CHANGED, m_xAccessible.get());
}

void  AccContainerEventListener::HandleNameChangedEvent( Any name )
{
    if (GetRole() == AccessibleRole::COMBO_BOX)
    {
        Reference<XAccessibleContext> mxContext(m_xAccessible->getAccessibleContext());
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
    AccEventListener::HandleNameChangedEvent(name);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
