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

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>

#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <algorithm>
#include <assert.h>

#include <AccObject.hxx>
#include <AccEventListener.hxx>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-tokens"
    // "#endif !_MIDL_USE_GUIDDEF_" in midl-generated code
#endif
#include <UAccCOM_i.c>
#if defined __clang__
#pragma clang diagnostic pop
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

constexpr DWORD MSAA_NO_STATE = 0x00000000;

namespace {

/**
 * Map a UNO accessible role to an IAccessible2 role.
 * @param nUnoRole The UNO role (css::accessibility::AccessibleRole).
 * @return IAccessible2 role.
 */
short lcl_mapToIAccessible2Role(sal_Int16 nUnoRole)
{
    switch(nUnoRole)
    {
        case css::accessibility::AccessibleRole::UNKNOWN:
            return IA2_ROLE_UNKNOWN;
        case css::accessibility::AccessibleRole::ALERT:
            return ROLE_SYSTEM_DIALOG;
        case css::accessibility::AccessibleRole::BLOCK_QUOTE:
            return IA2_ROLE_BLOCK_QUOTE;
        case css::accessibility::AccessibleRole::COLUMN_HEADER:
            return ROLE_SYSTEM_COLUMNHEADER;
        case css::accessibility::AccessibleRole::CANVAS:
            return IA2_ROLE_CANVAS;
        case css::accessibility::AccessibleRole::CHECK_BOX:
            return ROLE_SYSTEM_CHECKBUTTON;
        case css::accessibility::AccessibleRole::CHECK_MENU_ITEM:
            return IA2_ROLE_CHECK_MENU_ITEM;
        case css::accessibility::AccessibleRole::COLOR_CHOOSER:
            return IA2_ROLE_COLOR_CHOOSER;
        case css::accessibility::AccessibleRole::COMBO_BOX:
            return ROLE_SYSTEM_COMBOBOX;
        case css::accessibility::AccessibleRole::DATE_EDITOR:
            return IA2_ROLE_DATE_EDITOR;
        case css::accessibility::AccessibleRole::DESKTOP_ICON:
            return IA2_ROLE_DESKTOP_ICON;
        case css::accessibility::AccessibleRole::DESKTOP_PANE:
            return IA2_ROLE_DESKTOP_PANE;
        case css::accessibility::AccessibleRole::DIRECTORY_PANE:
            return IA2_ROLE_DIRECTORY_PANE;
        case css::accessibility::AccessibleRole::DIALOG:
            return ROLE_SYSTEM_DIALOG;
        case css::accessibility::AccessibleRole::DOCUMENT:
            return ROLE_SYSTEM_DOCUMENT;
        case css::accessibility::AccessibleRole::EMBEDDED_OBJECT:
            return IA2_ROLE_EMBEDDED_OBJECT;
        case css::accessibility::AccessibleRole::END_NOTE:
            return IA2_ROLE_ENDNOTE;
        case css::accessibility::AccessibleRole::FILE_CHOOSER:
            return IA2_ROLE_FILE_CHOOSER;
        case css::accessibility::AccessibleRole::FILLER:
            return ROLE_SYSTEM_WHITESPACE;
        case css::accessibility::AccessibleRole::FONT_CHOOSER:
            return IA2_ROLE_FONT_CHOOSER;
        case css::accessibility::AccessibleRole::FOOTER:
            return IA2_ROLE_FOOTER;
        case css::accessibility::AccessibleRole::FOOTNOTE:
            return IA2_ROLE_FOOTNOTE;
        case css::accessibility::AccessibleRole::FRAME:
            return IA2_ROLE_FRAME;
        case css::accessibility::AccessibleRole::GLASS_PANE:
            return IA2_ROLE_GLASS_PANE;
        case css::accessibility::AccessibleRole::GRAPHIC:
            return ROLE_SYSTEM_GRAPHIC;
        case css::accessibility::AccessibleRole::GROUP_BOX:
            return ROLE_SYSTEM_GROUPING;
        case css::accessibility::AccessibleRole::HEADER:
            return IA2_ROLE_HEADER;
        case css::accessibility::AccessibleRole::HEADING:
            return IA2_ROLE_HEADING;
        case css::accessibility::AccessibleRole::HYPER_LINK:
            return ROLE_SYSTEM_TEXT;
        case css::accessibility::AccessibleRole::ICON:
            return IA2_ROLE_ICON;
        case css::accessibility::AccessibleRole::INTERNAL_FRAME:
            return IA2_ROLE_INTERNAL_FRAME;
        case css::accessibility::AccessibleRole::LABEL:
            return ROLE_SYSTEM_STATICTEXT;
        case css::accessibility::AccessibleRole::LAYERED_PANE:
            return IA2_ROLE_LAYERED_PANE;
        case css::accessibility::AccessibleRole::LIST:
            return ROLE_SYSTEM_LIST;
        case css::accessibility::AccessibleRole::LIST_ITEM:
            return ROLE_SYSTEM_LISTITEM;
        case css::accessibility::AccessibleRole::MENU:
            return ROLE_SYSTEM_MENUITEM;
        case css::accessibility::AccessibleRole::MENU_BAR:
            return ROLE_SYSTEM_MENUBAR;
        case css::accessibility::AccessibleRole::MENU_ITEM:
            return ROLE_SYSTEM_MENUITEM;
        case css::accessibility::AccessibleRole::OPTION_PANE:
            return IA2_ROLE_OPTION_PANE;
        case css::accessibility::AccessibleRole::PAGE_TAB:
            return ROLE_SYSTEM_PAGETAB;
        case css::accessibility::AccessibleRole::PAGE_TAB_LIST:
            return ROLE_SYSTEM_PAGETABLIST;
        case css::accessibility::AccessibleRole::PANEL:
            return IA2_ROLE_OPTION_PANE;
        case css::accessibility::AccessibleRole::PARAGRAPH:
            return IA2_ROLE_PARAGRAPH;
        case css::accessibility::AccessibleRole::PASSWORD_TEXT:
            return ROLE_SYSTEM_TEXT;
        case css::accessibility::AccessibleRole::POPUP_MENU:
            return ROLE_SYSTEM_MENUPOPUP;
        case css::accessibility::AccessibleRole::PUSH_BUTTON:
            return ROLE_SYSTEM_PUSHBUTTON;
        case css::accessibility::AccessibleRole::PROGRESS_BAR:
            return ROLE_SYSTEM_PROGRESSBAR;
        case css::accessibility::AccessibleRole::RADIO_BUTTON:
            return ROLE_SYSTEM_RADIOBUTTON;
        case css::accessibility::AccessibleRole::RADIO_MENU_ITEM:
            return IA2_ROLE_RADIO_MENU_ITEM;
        case css::accessibility::AccessibleRole::ROW_HEADER:
            return ROLE_SYSTEM_ROWHEADER;
        case css::accessibility::AccessibleRole::ROOT_PANE:
            return IA2_ROLE_ROOT_PANE;
        case css::accessibility::AccessibleRole::SCROLL_BAR:
            return ROLE_SYSTEM_SCROLLBAR;
        case css::accessibility::AccessibleRole::SCROLL_PANE:
            return IA2_ROLE_SCROLL_PANE;
        case css::accessibility::AccessibleRole::SHAPE:
            return IA2_ROLE_SHAPE;
        case css::accessibility::AccessibleRole::SEPARATOR:
            return ROLE_SYSTEM_SEPARATOR;
        case css::accessibility::AccessibleRole::SLIDER:
            return ROLE_SYSTEM_SLIDER;
        case css::accessibility::AccessibleRole::SPIN_BOX:
            return ROLE_SYSTEM_SPINBUTTON;
        case css::accessibility::AccessibleRole::SPLIT_PANE:
            return IA2_ROLE_SPLIT_PANE;
        case css::accessibility::AccessibleRole::STATUS_BAR:
            return ROLE_SYSTEM_STATUSBAR;
        case css::accessibility::AccessibleRole::TABLE:
            return ROLE_SYSTEM_TABLE;
        case css::accessibility::AccessibleRole::TABLE_CELL:
            return ROLE_SYSTEM_CELL;
        case css::accessibility::AccessibleRole::TEXT:
            return ROLE_SYSTEM_TEXT;
        case css::accessibility::AccessibleRole::TEXT_FRAME:
            return IA2_ROLE_TEXT_FRAME;
        case css::accessibility::AccessibleRole::TOGGLE_BUTTON:
            return ROLE_SYSTEM_PUSHBUTTON;
        case css::accessibility::AccessibleRole::TOOL_BAR:
            return ROLE_SYSTEM_TOOLBAR;
        case css::accessibility::AccessibleRole::TOOL_TIP:
            return ROLE_SYSTEM_TOOLTIP;
        case css::accessibility::AccessibleRole::TREE:
            return ROLE_SYSTEM_OUTLINE;
        case css::accessibility::AccessibleRole::VIEW_PORT:
            return IA2_ROLE_VIEW_PORT;
        case css::accessibility::AccessibleRole::WINDOW:
            return ROLE_SYSTEM_WINDOW;
        case css::accessibility::AccessibleRole::BUTTON_DROPDOWN:
            return ROLE_SYSTEM_BUTTONDROPDOWN;
        case css::accessibility::AccessibleRole::BUTTON_MENU:
            return  ROLE_SYSTEM_BUTTONMENU;
        case css::accessibility::AccessibleRole::CAPTION:
            return  IA2_ROLE_CAPTION;
        case css::accessibility::AccessibleRole::CHART:
            return  IA2_ROLE_SHAPE;
        case css::accessibility::AccessibleRole::EDIT_BAR:
            return  IA2_ROLE_EDITBAR;
        case css::accessibility::AccessibleRole::FORM:
            return  IA2_ROLE_FORM;
        case css::accessibility::AccessibleRole::IMAGE_MAP:
            return  IA2_ROLE_IMAGE_MAP;
        case css::accessibility::AccessibleRole::NOTE:
            return  IA2_ROLE_NOTE;
        case css::accessibility::AccessibleRole::PAGE:
            return  IA2_ROLE_PAGE;
        case css::accessibility::AccessibleRole::RULER:
            return  IA2_ROLE_RULER;
        case css::accessibility::AccessibleRole::SECTION:
            return  IA2_ROLE_SECTION;
        case css::accessibility::AccessibleRole::TREE_ITEM:
            return  ROLE_SYSTEM_OUTLINEITEM;
        case css::accessibility::AccessibleRole::TREE_TABLE:
            return  ROLE_SYSTEM_OUTLINE;
        case css::accessibility::AccessibleRole::COMMENT:
            return  IA2_ROLE_TEXT_FRAME;
        case css::accessibility::AccessibleRole::COMMENT_END:
            return  IA2_ROLE_TEXT_FRAME;
        case css::accessibility::AccessibleRole::DOCUMENT_PRESENTATION:
            return  ROLE_SYSTEM_DOCUMENT;
        case css::accessibility::AccessibleRole::DOCUMENT_SPREADSHEET:
            return  ROLE_SYSTEM_DOCUMENT;
        case css::accessibility::AccessibleRole::DOCUMENT_TEXT:
            return  ROLE_SYSTEM_DOCUMENT;
        case css::accessibility::AccessibleRole::STATIC:
            return  ROLE_SYSTEM_STATICTEXT;
        case css::accessibility::AccessibleRole::NOTIFICATION:
            return  ROLE_SYSTEM_ALERT;
        default:
            SAL_WARN("iacc2", "Unmapped role: " << nUnoRole);
            return IA2_ROLE_UNKNOWN;
    }
}
};


/**
   * Constructor.
   * @param pXAcc Uno XAccessible interface of control.
   * @param pManager The accessible object manager kept in all listeners.
   * @param listener listener that registers in UNO system.
   * @return.
   */
AccObject::AccObject(XAccessible* pAcc, AccObjectWinManager* pManager) :
        m_resID     (0),
        m_pParantID (nullptr),
        m_pIMAcc    (UAccCOMCreateInstance()),
        m_pParentObj(nullptr),
        m_pListener (nullptr),
        m_xAccRef( pAcc )
{
    assert(m_pIMAcc);

    m_xAccContextRef = m_xAccRef->getAccessibleContext();
    m_accRole = m_xAccContextRef -> getAccessibleRole();

    m_pIMAcc->SetXAccessible(reinterpret_cast<hyper>(m_xAccRef.get()));
    m_pIMAcc->Put_XAccObjectManager(reinterpret_cast<hyper>(pManager));
}
/**
   * Destructor.
   * @param
   * @return
   */
AccObject::~AccObject()
{
    m_xAccRef = nullptr;
    m_xAccContextRef = nullptr;
}


/**
   * Insert a child element.
   * @param pChild Child element that should be inserted into child list.
   * @return
   */
void AccObject::InsertChild(AccObject* pChild)
{
    std::vector<AccObject*>::iterator iter;
    iter = std::find(m_childrenList.begin(),m_childrenList.end(),pChild);
    if(iter!=m_childrenList.end())
        return;

    m_childrenList.push_back(pChild);

    pChild->SetParentObj(this);
}

/**
   * Delete a child element
   * @param pChild Child element that should be inserted into child list.
   * @param pos Insert position.
   * @return
   */
void AccObject::DeleteChild( AccObject* pChild )
{
    std::vector<AccObject*>::iterator iter;
    iter = std::find(m_childrenList.begin(),m_childrenList.end(),pChild);
    if(iter!=m_childrenList.end())
    {
        m_childrenList.erase(iter);
        pChild->SetParentObj(nullptr);
    }
}

/**
   * In order to windows API WindowFromAccessibleObject, we sometimes to set a pure
   * top window accessible object created by windows system as top ancestor.
   * @param.
   * @return
   */
void AccObject::UpdateValidWindow() { m_pIMAcc->Put_XAccWindowHandle(m_pParantID); }

/**
   * Update value property to com object.
   * @param
   * @return
   */
void AccObject::UpdateValue()
{
    if (!m_xAccContextRef.is())
    {
        assert(false);
        return ;
    }

    Reference< XAccessibleValue > pRValue(m_xAccContextRef,UNO_QUERY);
    Any pAny;
    if( pRValue.is() )
    {
        pAny = pRValue->getCurrentValue();
    }

    SetValue( pAny );
}

/**
   * Set value property via pAny.
   * @param pAny New value.
   * @return
   */
void  AccObject::SetValue( Any pAny )
{
    if (!m_xAccContextRef.is())
    {
        assert(false);
        return ;
    }
    Reference< XAccessibleText > pRText(m_xAccContextRef,UNO_QUERY);
    OUString val;
    switch(m_accRole)
    {
    case AccessibleRole::SPIN_BOX:
        // 3. date editor's msaa value should be the same as spinbox
    case AccessibleRole::DATE_EDITOR:
    case AccessibleRole::TEXT:
    case AccessibleRole::BLOCK_QUOTE:
    case AccessibleRole::PARAGRAPH:
    case AccessibleRole::HEADING:
    case AccessibleRole::TABLE_CELL:

        if(pRText)
        {
            val = pRText->getText();
        }
        m_pIMAcc->Put_XAccValue( o3tl::toW(val.getStr()) );
        break;
    case AccessibleRole::TREE_ITEM:
    //case CHECK_BOX:   //Commented by Li Xing to disable the value for general checkbox
    case AccessibleRole::COMBO_BOX:
    case AccessibleRole::NOTE:
    case AccessibleRole::SCROLL_BAR:
        m_pIMAcc->Put_XAccValue( o3tl::toW(GetMAccessibleValueFromAny(pAny).getStr()) );
        break ;
    // Added by Li Xing, only the checkbox in tree should have the value.
    case AccessibleRole::CHECK_BOX:
        if( ( m_pParentObj !=nullptr ) && (AccessibleRole::TREE == m_pParentObj->m_accRole || AccessibleRole::TREE_ITEM == m_pParentObj->m_accRole ))
            m_pIMAcc->Put_XAccValue( o3tl::toW(GetMAccessibleValueFromAny(pAny).getStr()) );
        break;
    default:
        break;
    }

    return;
}

OUString AccObject::GetMAccessibleValueFromAny(Any pAny)
{
    OUString strValue;

    if(pAny.getValueType() == cppu::UnoType<cppu::UnoUnsignedShortType>::get() )
    {
        sal_uInt16 val;
        if (pAny >>= val)
        {
            strValue=OUString::number(val);

        }
    }
    else if(pAny.getValueType() == cppu::UnoType<OUString>::get())
    {

        pAny >>= strValue ;

    }
    else if(pAny.getValueType() == cppu::UnoType<Sequence< OUString >>::get())
    {
        Sequence< OUString > val;
        if (pAny >>= val)
        {
            for (const OUString& rElem : val)
                strValue += rElem;
        }
    }
    else if(pAny.getValueType() == cppu::UnoType<double>::get())
    {
        double val;
        if (pAny >>= val)
        {
            strValue=OUString::number(val);
        }
    }
    else if(pAny.getValueType() == cppu::UnoType<sal_Int32>::get())
    {
        sal_Int32 val;
        if (pAny >>= val)
        {
            strValue=OUString::number(val);
        }
    }
    else if (pAny.getValueType() == cppu::UnoType<css::accessibility::TextSegment>::get())
    {
        css::accessibility::TextSegment val;
        if (pAny >>= val)
        {
            OUString realVal(val.SegmentText);
            strValue = realVal;

        }
    }

    return strValue;
}

/**
* Get role property via pAny
* @param
* @return accessible role
*/
short AccObject::GetRole() const
{
    return m_accRole;
}

/**
   * Get MSAA state from UNO state
   * @Role nState UNO state.
   * @return
   */
DWORD AccObject::GetMSAAStateFromUNO(sal_Int64 nState)
{
    if( !m_xAccContextRef.is() )
    {
        assert(false);
        return MSAA_NO_STATE;
    }

    switch( nState )
    {
    case AccessibleStateType::BUSY:
        return STATE_SYSTEM_BUSY;
    case AccessibleStateType::CHECKED:
        if (m_accRole == AccessibleRole::PUSH_BUTTON || m_accRole == AccessibleRole::TOGGLE_BUTTON)
            return STATE_SYSTEM_PRESSED;
        return STATE_SYSTEM_CHECKED;
    case AccessibleStateType::DEFUNC:
        return STATE_SYSTEM_UNAVAILABLE;
    case AccessibleStateType::EXPANDED:
        return STATE_SYSTEM_EXPANDED;
    case AccessibleStateType::FOCUSABLE:
        return STATE_SYSTEM_FOCUSABLE;
    case AccessibleStateType::FOCUSED:
        return STATE_SYSTEM_FOCUSED;
    case AccessibleStateType::INDETERMINATE:
        return STATE_SYSTEM_MIXED;
    case AccessibleStateType::MULTI_SELECTABLE:
        return STATE_SYSTEM_MULTISELECTABLE;
    case AccessibleStateType::PRESSED:
        return STATE_SYSTEM_PRESSED;
    case AccessibleStateType::RESIZABLE:
        return STATE_SYSTEM_SIZEABLE;
    case AccessibleStateType::SELECTABLE:
        if (m_accRole == AccessibleRole::MENU || m_accRole == AccessibleRole::MENU_ITEM)
            return MSAA_NO_STATE;
        return STATE_SYSTEM_SELECTABLE;
    case AccessibleStateType::SELECTED:
        if (m_accRole == AccessibleRole::MENU || m_accRole == AccessibleRole::MENU_ITEM)
            return MSAA_NO_STATE;
        return STATE_SYSTEM_SELECTED;
    case AccessibleStateType::ARMED:
        return STATE_SYSTEM_FOCUSED;
    case AccessibleStateType::EXPANDABLE:
        {
            if (m_accRole == AccessibleRole::PUSH_BUTTON
                || m_accRole == AccessibleRole::TOGGLE_BUTTON
                || m_accRole == AccessibleRole::BUTTON_DROPDOWN)
                return STATE_SYSTEM_HASPOPUP;

            if (!(m_xAccContextRef->getAccessibleStateSet() & AccessibleStateType::EXPANDED))
                return STATE_SYSTEM_COLLAPSED;

            return MSAA_NO_STATE;
        }
    //Remove the SENSITIVE state mapping. There is no corresponding MSAA state.
    //case  SENSITIVE:
    //    IState = STATE_SYSTEM_PROTECTED;
    case AccessibleStateType::EDITABLE:
        m_pIMAcc->DecreaseState(STATE_SYSTEM_READONLY);
        return MSAA_NO_STATE;
    case AccessibleStateType::OFFSCREEN:
        return STATE_SYSTEM_OFFSCREEN;
    case AccessibleStateType::MOVEABLE:
        return STATE_SYSTEM_MOVEABLE;
    case AccessibleStateType::COLLAPSE:
        return STATE_SYSTEM_COLLAPSED;
        break;
    case AccessibleStateType::DEFAULT:
        return STATE_SYSTEM_DEFAULT;
    default:
        return MSAA_NO_STATE;
    }
}

/**
   * Decrease state of com object
   * @param xState The lost state.
   * @return
   */
void  AccObject::DecreaseState( sal_Int64 xState )
{
    if (xState == AccessibleStateType::FOCUSABLE)
    {
        if (m_accRole == AccessibleRole::MENU_ITEM || m_accRole == AccessibleRole::RADIO_MENU_ITEM
            || m_accRole == AccessibleRole::CHECK_MENU_ITEM)
            return;
        else
        {
            if (m_accRole == AccessibleRole::TOGGLE_BUTTON
                || m_accRole == AccessibleRole::PUSH_BUTTON
                || m_accRole == AccessibleRole::BUTTON_DROPDOWN)
            {
                if ((m_pParentObj != nullptr)
                    && (AccessibleRole::TOOL_BAR == m_pParentObj->m_accRole))
                    return;
            }
        }
    }

    else if( xState == AccessibleStateType::VISIBLE  )
    {
        m_pIMAcc->IncreaseState( STATE_SYSTEM_INVISIBLE );
    }
    else if( xState == AccessibleStateType::SHOWING )
    {
        m_pIMAcc->IncreaseState( STATE_SYSTEM_OFFSCREEN );
    }

    DWORD msState = GetMSAAStateFromUNO(xState);
    if (msState != MSAA_NO_STATE)
        m_pIMAcc->DecreaseState(msState);
}

/**
   * Increase state of com object
   * @param xState The new state.
   * @return
   */
void AccObject::IncreaseState( sal_Int64 xState )
{
    if( xState == AccessibleStateType::VISIBLE  )
    {
        m_pIMAcc->DecreaseState( STATE_SYSTEM_INVISIBLE );
    }
    else if( xState == AccessibleStateType::SHOWING )
    {
        m_pIMAcc->DecreaseState( STATE_SYSTEM_OFFSCREEN );
    }


    DWORD msState = GetMSAAStateFromUNO(xState);
    if (msState != MSAA_NO_STATE)
        m_pIMAcc->IncreaseState( msState );
}

/**
   * Get next child element
   * @param
   * @return AccObject Object interface.
   */
AccObject* AccObject::NextChild()
{
    IAccChildList::iterator pInd = m_childrenList.begin();
    if( pInd != m_childrenList.end() )
        return  *pInd;
    return nullptr;
}

/**
   * update role information from uno to com
   * @param
   * @return
   */
void AccObject::UpdateRole()
{
    const sal_Int16 nUnoRole = m_xAccContextRef->getAccessibleRole();
    short nIA2Role = lcl_mapToIAccessible2Role(nUnoRole);
    m_pIMAcc->Put_XAccRole(nIA2Role);
}

/**
   * update state information from uno to com
   * @param
   * @return
   */
void AccObject::UpdateState()
{
    XAccessibleContext* pContext  = m_xAccContextRef.get();
    sal_Int64 nRState = pContext->getAccessibleStateSet();

    m_pIMAcc->SetState(0);

    if (m_accRole == AccessibleRole::POPUP_MENU)
    {
        return;
    }

    bool isEnable = false;
    bool isShowing = false;
    bool isEditable = false;
    bool isVisible = false;

    for (int i=0; i<63; ++i)
    {
        sal_Int64 nState = sal_Int64(1) << i;
        if (!(nState & nRState))
            continue;
        if (nState == AccessibleStateType::ENABLED)
            isEnable = true;
        else if (nState == AccessibleStateType::SHOWING)
            isShowing = true;
        else if (nState == AccessibleStateType::VISIBLE)
            isVisible = true;
        else if (nState == AccessibleStateType::EDITABLE)
            isEditable = true;
        IncreaseState(nState);
    }

    bool bIsMenuItem = m_accRole == AccessibleRole::MENU_ITEM
                       || m_accRole == AccessibleRole::RADIO_MENU_ITEM
                       || m_accRole == AccessibleRole::CHECK_MENU_ITEM;

    if(bIsMenuItem)
    {
        if(!(isShowing && isVisible) )
        {
            m_pIMAcc->IncreaseState( STATE_SYSTEM_INVISIBLE );
        }
    }
    else
    {
        if(!(isShowing || isVisible) )
            m_pIMAcc->IncreaseState( STATE_SYSTEM_INVISIBLE );
    }

    switch(m_accRole)
    {
    case AccessibleRole::LABEL:
    case AccessibleRole::STATIC:
    case AccessibleRole::NOTIFICATION:
    case AccessibleRole::TEXT:
        // 2. editable combobox -> readonly ------ bridge
    case AccessibleRole::EMBEDDED_OBJECT:
    case AccessibleRole::END_NOTE:
    case AccessibleRole::FOOTER:
    case AccessibleRole::FOOTNOTE:
    case AccessibleRole::GRAPHIC:
    case AccessibleRole::HEADER:
    case AccessibleRole::HEADING:

        //Image Map
    case AccessibleRole::BLOCK_QUOTE:
    case AccessibleRole::PARAGRAPH:
    case AccessibleRole::PASSWORD_TEXT:
    case AccessibleRole::SHAPE:
    case AccessibleRole::SPIN_BOX:
    case AccessibleRole::TABLE:
    case AccessibleRole::TABLE_CELL:
    case AccessibleRole::TEXT_FRAME:
    case AccessibleRole::DATE_EDITOR:
    case AccessibleRole::DOCUMENT:
    case AccessibleRole::COLUMN_HEADER:
        {
            if(!isEditable)
                m_pIMAcc->IncreaseState( STATE_SYSTEM_READONLY );
        }
        break;
    default:
        break;
    }

    if (!isEnable)
        m_pIMAcc->IncreaseState( STATE_SYSTEM_UNAVAILABLE );

    switch(m_accRole)
    {
    case AccessibleRole::POPUP_MENU:
    case AccessibleRole::MENU:
        if( pContext->getAccessibleChildCount() > 0 )
            m_pIMAcc->IncreaseState( STATE_SYSTEM_HASPOPUP );
        break;
    case AccessibleRole::PASSWORD_TEXT:
        m_pIMAcc->IncreaseState( STATE_SYSTEM_PROTECTED );
        break;
    default:
        break;
    }
}

/**
   * Public method to mapping information between MSAA and UNO.
   * @param
   * @return If the method is correctly processed.
   */
bool AccObject::UpdateAccessibleInfoFromUnoToMSAA()
{
    if (!m_xAccContextRef.is())
    {
        assert(false);
        return false;
    }

    UpdateValue();

    UpdateRole();

    UpdateState();

    return true;
}


/**
   * Set self to focus object in parent child list
   * @param
   * @return
   */
void AccObject::setFocus()
{
    IncreaseState(AccessibleStateType::FOCUSED);
    m_pIMAcc->Put_XAccFocus(CHILDID_SELF);

    UpdateRole();
}

/**
   * Unset self from focus object in parent child list.
   * @param
   * @return
   */
void AccObject::unsetFocus()
{
    DecreaseState(AccessibleStateType::FOCUSED);
    m_pIMAcc->Put_XAccFocus(UACC_NO_FOCUS);
}

void AccObject::NotifyDestroy() { m_pIMAcc->NotifyDestroy(); }

void AccObject::SetParentObj(AccObject* pParentAccObj)
{
    m_pParentObj = pParentAccObj;

    if (m_pParentObj)
        m_pIMAcc->Put_XAccParent(m_pParentObj->GetIMAccessible());
    else
        m_pIMAcc->Put_XAccParent(nullptr);
}
//ResID means ChildID in MSAA
void AccObject::SetResID(long id)
{
    m_resID = id;
    m_pIMAcc->Put_XAccChildID(m_resID);
}
//return COM interface in acc object
IMAccessible*  AccObject::GetIMAccessible()
{
    return m_pIMAcc;
}

Reference<XAccessible> const& AccObject::GetXAccessible()
{
    return m_xAccRef;
}

void AccObject::SetParentHWND(HWND hWnd)
{
    m_pParantID = hWnd;
}

rtl::Reference<AccEventListener> AccObject::SetListener(rtl::Reference<AccEventListener> const& pListener)
{
    rtl::Reference<AccEventListener> pRet(m_pListener);
    m_pListener = pListener;
    return pRet;
}

long AccObject::GetResID()
{
    return m_resID;
}

HWND AccObject::GetParentHWND()
{
    return m_pParantID;
}

AccObject* AccObject::GetParentObj()
{
    return m_pParentObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
