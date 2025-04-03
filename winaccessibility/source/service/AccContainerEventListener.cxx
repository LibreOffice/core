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

#include <vcl/svapp.hxx>

#include <AccContainerEventListener.hxx>
#include <AccObjectWinManager.hxx>
#include <unomsaaevent.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

AccContainerEventListener::AccContainerEventListener(css::accessibility::XAccessible* pAcc, AccObjectWinManager& rManager)
        :AccEventListener(pAcc, rManager)
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
{
    SolarMutexGuard g;

    switch (aEvent.EventId)
    {
    case AccessibleEventId::SELECTION_CHANGED:
        HandleSelectionChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
    case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        HandleAllChildrenChangedEvent();
        break;
    case AccessibleEventId::TEXT_CHANGED:
        HandleTextChangedEvent(aEvent.OldValue, aEvent.NewValue);
        break;
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
    sal_Int64 State;
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
 * handle the SELECTION_CHANGED event
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccContainerEventListener::HandleSelectionChangedEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if (NotifyChildEvent(UnoMSAAEvent::SELECTION_CHANGED, newValue))
    {
        return ;
    }

    //menu bar does not process selection change event,just same as word behavior
    if (GetRole()!=AccessibleRole::MENU_BAR)
        m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED);
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
        m_rObjManager.DeleteChildrenAccObj(m_xAccessible.get());
        //add all oldValue's existing children
        m_rObjManager.InsertChildrenAccObj(m_xAccessible.get());
        m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_REORDER);
    }
}

/**
 *  handle the TEXT_CHANGED event
 */
void AccContainerEventListener::HandleTextChangedEvent(Any, Any newValue)
{
    m_rObjManager.SetValue(m_xAccessible.get(), newValue);
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_TEXTCHANGE);
}

/**
 * set the new state and fire the MSAA event
 * @param   state   new state id
 * @param   enable  true if state is set, false if state is unset
 */
void AccContainerEventListener::SetComponentState(sal_Int64 state, bool enable )
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
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            m_rObjManager.UpdateState(m_xAccessible.get());

            UpdateAllChildrenState(m_xAccessible.get());
        }
        else
        {
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::DEFUNC);
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSABLE);
            m_rObjManager.UpdateState(m_xAccessible.get());

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
                m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
            }

            else    /* lose the active state */
            {
                m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
            }
        }
        break;

    case AccessibleStateType::EXPANDED:
    case AccessibleStateType::COLLAPSE:
    case AccessibleStateType::CHECKED:
        {
            m_rObjManager.UpdateState(m_xAccessible.get());
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_BUSY);
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
void AccContainerEventListener::FireStatePropertyChange(sal_Int64 state, bool set)
{
    if( set )
    {
        // new value
        switch(state)
        {
        case AccessibleStateType::SELECTED:
            m_rObjManager.IncreaseState(m_xAccessible.get(), state);
            break;
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::BUSY:
        case AccessibleStateType::FOCUSABLE:
        case AccessibleStateType::OFFSCREEN:
            m_rObjManager.IncreaseState(m_xAccessible.get(), state);
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_BUSY);
            break;
        case AccessibleStateType::SHOWING:
            // UNO !SHOWING == MSAA OFFSCREEN
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING);
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
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
            m_rObjManager.DecreaseState(m_xAccessible.get(), state);
            break;
        case AccessibleStateType::BUSY:
        case AccessibleStateType::INDETERMINATE:
        case AccessibleStateType::FOCUSABLE:
        case AccessibleStateType::OFFSCREEN:
            m_rObjManager.DecreaseState(m_xAccessible.get(), state);
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_BUSY);
            break;
        case AccessibleStateType::SHOWING:
            // UNO !SHOWING == MSAA OFFSCREEN
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::SHOWING);
            break;
        case AccessibleStateType::VISIBLE:
            // UNO !VISIBLE == MSAA INVISIBLE
            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::VISIBLE);
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
        m_rObjManager.IncreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
        // if the acc role is MENU_BAR, UnoMSAAEvent::MENU_START event should be sent
        // if the acc role is POPUP_MENU, UnoMSAAEvent::MENUPOPUPSTART event should be sent
        short role = GetRole();
        if(role == AccessibleRole::MENU_BAR)
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::MENU_START);
        }
        else if (role == AccessibleRole::POPUP_MENU)
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::MENUPOPUPSTART);
        //Disable the focused event on option_pane and Panel.
        //only disable option_pane for toolbar has panel to get focus
        else if (role == AccessibleRole::PANEL || role == AccessibleRole::OPTION_PANE )
        {
            //don't send focused event on PANEL & OPTION_PANE if the parent is not toolbar
            short parentRole = GetParentRole();
            if (parentRole == AccessibleRole::TOOL_BAR
                || parentRole == AccessibleRole::SCROLL_PANE // sidebar
                || parentRole == AccessibleRole::PANEL) // sidebar
                m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_FOCUSED);
        }
        else if (role == AccessibleRole::COMBO_BOX )
        {
            //for editable combobox, send focus event on only edit control,
            bool bSendFocusOnCombobox = true;
            //send focused event to the first text child
            Reference<XAccessibleContext> mxContext = m_xAccessible->getAccessibleContext();
            if(mxContext.is())
            {
                Reference<XAccessible> mxChild = mxContext->getAccessibleChild(0);
                if(mxChild.is())
                {
                    Reference<XAccessibleContext> mxChildContext = mxChild->getAccessibleContext();
                    short childrole = mxChildContext->getAccessibleRole();
                    if (childrole == AccessibleRole::TEXT)
                    {
                        if (IsEditable(mxChildContext))
                        {
                            m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
                            m_rObjManager.IncreaseState( mxChild.get(), AccessibleStateType::FOCUSED);
                            m_rObjManager.NotifyAccEvent(mxChild.get(), UnoMSAAEvent::STATE_FOCUSED);
                            bSendFocusOnCombobox = false;
                        }
                    }
                }
            }
            if (bSendFocusOnCombobox)
                m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_FOCUSED);
        }
        else
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::STATE_FOCUSED);
    }
    else
    {
        m_rObjManager.DecreaseState(m_xAccessible.get(), AccessibleStateType::FOCUSED);
        // if the acc role is MENU_BAR, UnoMSAAEvent::MENU_END event should be sent
        // if the acc role is POPUP_MENU, UnoMSAAEvent::MENUPOPUPEND event should be sent
        if (GetRole() == AccessibleRole::MENU_BAR)
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::MENU_END);
        }
        else if (GetRole() == AccessibleRole::POPUP_MENU)
        {
            m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::MENUPOPUPEND);
        }
    }
}

/**
 * handle the VALUE_CHANGED event
 *
 * @param   oldValue    the old value of the source of event
 * @param   newValue    the new value of the source of event
 */
void AccContainerEventListener::HandleValueChangedEvent(Any, Any)
{
    m_rObjManager.UpdateValue(m_xAccessible.get());
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_VALUECHANGE);
}

bool AccContainerEventListener::IsEditable(Reference<XAccessibleContext> const & xContext)
{
    sal_Int64 nRState = xContext->getAccessibleStateSet();
    return nRState & AccessibleStateType::EDITABLE;
}

bool AccContainerEventListener::NotifyChildEvent(UnoMSAAEvent eWinEvent, const Any& Value)
{
    Reference< XAccessible > xChild;
    if(Value >>= xChild )
    {
        if(xChild.is())
        {
            XAccessible* pAcc = xChild.get();
            m_rObjManager.NotifyAccEvent(pAcc, eWinEvent);
            return true;
        }
    }
    return false;
}

void AccContainerEventListener::HandleSelectionChangedAddEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if (NotifyChildEvent(UnoMSAAEvent::SELECTION_CHANGED_ADD, newValue))
    {
        return ;
    }
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED_ADD);
}

void AccContainerEventListener::HandleSelectionChangedRemoveEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if (NotifyChildEvent(UnoMSAAEvent::SELECTION_CHANGED_REMOVE, newValue))
    {
        return ;
    }
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED_REMOVE);
}

void AccContainerEventListener::HandleSelectionChangedWithinEvent(const Any& /*oldValue*/, const Any& newValue)
{
    if (NotifyChildEvent(UnoMSAAEvent::SELECTION_CHANGED_WITHIN, newValue))
    {
        return ;
    }
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SELECTION_CHANGED_WITHIN);
}

void AccContainerEventListener::UpdateAllChildrenState(XAccessible* pXAccessible)
{
    Reference<css::accessibility::XAccessibleContext> xContext = pXAccessible->getAccessibleContext();
    if(!xContext.is())
    {
        return;
    }
    css::accessibility::XAccessibleContext* pAccessibleContext = xContext.get();
    if(pAccessibleContext == nullptr)
    {
        return;
    }

    if (AccObjectWinManager::IsStateManageDescendant(pXAccessible))
    {
        return;
    }

    const sal_Int64 nCount = pAccessibleContext->getAccessibleChildCount();
    for (sal_Int64 i = 0; i < nCount; i++)
    {
        Reference<css::accessibility::XAccessible> mxAccessible
        = pAccessibleContext->getAccessibleChild(i);

        css::accessibility::XAccessible* mpAccessible = mxAccessible.get();
        if(mpAccessible != nullptr)
        {
            m_rObjManager.UpdateState(mpAccessible);
            UpdateAllChildrenState(mpAccessible);
        }
    }
}

void AccContainerEventListener::HandlePageChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/)
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::OBJECT_PAGECHANGED);
}

void AccContainerEventListener::HandleSectionChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/ )
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::SECTION_CHANGED);
}

void AccContainerEventListener::HandleColumnChangedEvent(const Any& /*oldValue*/, const Any& /*newValue*/)
{
    m_rObjManager.NotifyAccEvent(m_xAccessible.get(), UnoMSAAEvent::COLUMN_CHANGED);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
