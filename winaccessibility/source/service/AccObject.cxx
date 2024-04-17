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
using namespace com::sun::star::accessibility::AccessibleRole;
using namespace com::sun::star::accessibility::AccessibleStateType;

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
AccObject::AccObject(XAccessible* pAcc, AccObjectWinManager* pManager,
                     AccEventListener* pListener) :
        m_resID     (0),
        m_pParantID (nullptr),
        m_pIMAcc    (UAccCOMCreateInstance()),
        m_pParentObj(nullptr),
        m_pListener (pListener),
        m_xAccRef( pAcc )
{
    ImplInitializeCreateObj();

    m_xAccContextRef = m_xAccRef->getAccessibleContext();
    m_xAccActionRef.set(m_xAccContextRef,UNO_QUERY);
    m_accRole = m_xAccContextRef -> getAccessibleRole();
    if( m_pIMAcc )
    {
        m_pIMAcc->SetXAccessible(reinterpret_cast<hyper>(m_xAccRef.get()));
        m_pIMAcc->Put_XAccObjectManager(reinterpret_cast<hyper>(pManager));
        m_pIMAcc->SetDefaultAction(reinterpret_cast<hyper>(m_xAccActionRef.get()));
    }
}
/**
   * Destructor.
   * @param
   * @return
   */
AccObject::~AccObject()
{
    m_xAccRef = nullptr;
    m_xAccActionRef = nullptr;
    m_xAccContextRef = nullptr;
}


/**
   * Insert a child element.
   * @param pChild Child element that should be inserted into child list.
   * @param pos Insert position.
   * @return
   */
void AccObject::InsertChild( AccObject* pChild,short pos )
{

    std::vector<AccObject*>::iterator iter;
    iter = std::find(m_childrenList.begin(),m_childrenList.end(),pChild);
    if(iter!=m_childrenList.end())
        return;
    if(LAST_CHILD==pos)
    {
        m_childrenList.push_back(pChild);
    }
    else
    {
        iter=m_childrenList.begin()+pos;
        m_childrenList.insert(iter,pChild);
    }

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
        if(m_pIMAcc)
            pChild->SetParentObj(nullptr);
    }
}

/**
   * In order to windows API WindowFromAccessibleObject, we sometimes to set a pure
   * top window accessible object created by windows system as top ancestor.
   * @param.
   * @return
   */
void AccObject::UpdateValidWindow()
{
    if(m_pIMAcc)
        m_pIMAcc->Put_XAccWindowHandle(m_pParantID);
}

/**
   * Translate all UNO basic information into MSAA com information.
   * @param
   * @return If the method is correctly processed.
   */
void AccObject::ImplInitializeCreateObj()
{
    assert(m_pIMAcc);
}

/**
   * Update name property to com object.
   * @param
   * @return
   */
void  AccObject::UpdateName( )
{
    if (!m_pIMAcc)
    {
        return;
    }

    if( ( TEXT_FRAME == m_accRole   ) && ( m_pParentObj !=nullptr )&& ( SCROLL_PANE == m_pParentObj -> m_accRole ) )
        m_pIMAcc->Put_XAccName( o3tl::toW(m_pParentObj->m_xAccContextRef->getAccessibleName().getStr()) );
    //IAccessibility2 Implementation 2009-----
    if (m_accRole == AccessibleRole::PARAGRAPH || m_accRole == AccessibleRole::BLOCK_QUOTE)
    {
        m_pIMAcc->Put_XAccName(L"");
    }
    //-----IAccessibility2 Implementation 2009
    else
        m_pIMAcc->Put_XAccName(o3tl::toW(m_xAccContextRef->getAccessibleName().getStr()));

    return ;
}

/**
   * Update default action property to com object.
   * @param
   * @return
   */
void  AccObject::UpdateAction()
{
    m_xAccActionRef.set(m_xAccContextRef,UNO_QUERY);

    if( m_xAccActionRef.is() && m_pIMAcc )
    {
        if( m_xAccActionRef->getAccessibleActionCount() > 0 )
        {
            UpdateDefaultAction( );
            m_pIMAcc->SetDefaultAction(
                    reinterpret_cast<hyper>(m_xAccActionRef.get()));
        }
    }
}

/**
   * Update value property to com object.
   * @param
   * @return
   */
void AccObject::UpdateValue()
{
    if( nullptr == m_pIMAcc  || !m_xAccContextRef.is() )
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
   * Set special default action description string via UNO role.
   * @param Role UNO role
   * @return
   */
void AccObject::UpdateDefaultAction( )
{
    if(!m_xAccActionRef.is())
        return ;

    switch(m_accRole)
    {
    case PUSH_BUTTON:
    case TOGGLE_BUTTON:
    case RADIO_BUTTON:
    case MENU_ITEM:
    case RADIO_MENU_ITEM:
    case CHECK_MENU_ITEM:
    case LIST_ITEM:
    case CHECK_BOX:
    case TREE_ITEM:
    case BUTTON_DROPDOWN:
        m_pIMAcc->Put_ActionDescription( o3tl::toW(m_xAccActionRef->getAccessibleActionDescription(sal_Int32(0)).getStr()) );
        return;
    }
}

/**
   * Set value property via pAny.
   * @param pAny New value.
   * @return
   */
void  AccObject::SetValue( Any pAny )
{
    if( nullptr == m_pIMAcc || !m_xAccContextRef.is() )
    {
        assert(false);
        return ;
    }
    Reference< XAccessibleText > pRText(m_xAccContextRef,UNO_QUERY);
    OUString val;
    switch(m_accRole)
    {
    case SPIN_BOX:
        // 3. date editor's msaa value should be the same as spinbox
    case DATE_EDITOR:
    case TEXT:
    case BLOCK_QUOTE:
    case PARAGRAPH:
    case HEADING:
    case TABLE_CELL:

        if(pRText)
        {
            val = pRText->getText();
        }
        m_pIMAcc->Put_XAccValue( o3tl::toW(val.getStr()) );
        break;
    case TREE_ITEM:
    //case CHECK_BOX:   //Commented by Li Xing to disable the value for general checkbox
    case COMBO_BOX:
    case NOTE:
    case SCROLL_BAR:
        m_pIMAcc->Put_XAccValue( o3tl::toW(GetMAccessibleValueFromAny(pAny).getStr()) );
        break ;
    // Added by Li Xing, only the checkbox in tree should have the value.
    case CHECK_BOX:
        if( ( m_pParentObj !=nullptr ) && (TREE == m_pParentObj->m_accRole || TREE_ITEM == m_pParentObj->m_accRole ))
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
   * Set name property via pAny.
   * @param pAny New accessible name.
   * @return
   */
void  AccObject::SetName( Any pAny)
{
    if( nullptr == m_pIMAcc )
        return ;

    m_pIMAcc->Put_XAccName( o3tl::toW(GetMAccessibleValueFromAny(pAny).getStr()) );

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
    DWORD IState = UNO_MSAA_UNMAPPING;

    if( !m_xAccContextRef.is() )
    {
        assert(false);
        return IState;
    }

    switch( nState )
    {
    case  BUSY:
        IState = STATE_SYSTEM_BUSY;
        break;
    case  CHECKED:
        if (m_accRole == PUSH_BUTTON || m_accRole == TOGGLE_BUTTON)
        {
            IState = STATE_SYSTEM_PRESSED;
        }
        else
            IState = STATE_SYSTEM_CHECKED;
        break;
    case  DEFUNC:
        IState = STATE_SYSTEM_UNAVAILABLE;
        break;
    case  EXPANDED:
        IState = STATE_SYSTEM_EXPANDED;
        break;
    case  FOCUSABLE:
        IState = STATE_SYSTEM_FOCUSABLE;
        break;
    case  FOCUSED:
        IState = STATE_SYSTEM_FOCUSED;
        break;
    case  INDETERMINATE:
        IState = STATE_SYSTEM_MIXED;
        break;
    case  MULTI_SELECTABLE:
        IState = STATE_SYSTEM_MULTISELECTABLE;
        break;
    case  PRESSED:
        IState = STATE_SYSTEM_PRESSED;
        break;
    case  RESIZABLE:
        IState = STATE_SYSTEM_SIZEABLE;
        break;
    case  SELECTABLE:
        if( m_accRole == MENU || m_accRole == MENU_ITEM)
        {
            IState = UNO_MSAA_UNMAPPING;
        }
        else
        {
            IState = STATE_SYSTEM_SELECTABLE;
        }
        break;
    case  SELECTED:
        if( m_accRole == MENU || m_accRole == MENU_ITEM )
        {
            IState = UNO_MSAA_UNMAPPING;
        }
        else
        {
            IState = STATE_SYSTEM_SELECTED;
        }
        break;
    case  ARMED:
        IState = STATE_SYSTEM_FOCUSED;
        break;
    case  EXPANDABLE:
        {
            sal_Bool isExpanded = true;
            sal_Bool isExpandable = true;
            if (m_accRole == PUSH_BUTTON || m_accRole == TOGGLE_BUTTON  || m_accRole == BUTTON_DROPDOWN)
            {
                IState = STATE_SYSTEM_HASPOPUP;
            }
            else
            {
                GetExpandedState(&isExpandable,&isExpanded);
                if(!isExpanded)
                    IState = STATE_SYSTEM_COLLAPSED;
            }
        }
        break;
    //Remove the SENSITIVE state mapping. There is no corresponding MSAA state.
    //case  SENSITIVE:
    //    IState = STATE_SYSTEM_PROTECTED;
    case EDITABLE:
        if( m_pIMAcc )
        {
            m_pIMAcc->DecreaseState( STATE_SYSTEM_READONLY );
        }
        break;
    case OFFSCREEN:
        IState = STATE_SYSTEM_OFFSCREEN;
        break;
    case MOVEABLE:
        IState = STATE_SYSTEM_MOVEABLE;
        break;
    case COLLAPSE:
        IState = STATE_SYSTEM_COLLAPSED;
        break;
    case DEFAULT:
        IState = STATE_SYSTEM_DEFAULT;
        break;
    default:
        break;
    }

    return IState;
}

/**
   * Decrease state of com object
   * @param xState The lost state.
   * @return
   */
void  AccObject::DecreaseState( sal_Int64 xState )
{
    if( nullptr == m_pIMAcc )
    {
        return;
    }

    if( xState == FOCUSABLE)
    {
        if (m_accRole == MENU_ITEM || m_accRole == RADIO_MENU_ITEM || m_accRole == CHECK_MENU_ITEM)
            return;
        else
        {
            if (m_accRole == TOGGLE_BUTTON || m_accRole == PUSH_BUTTON || m_accRole == BUTTON_DROPDOWN)
            {
                if( ( m_pParentObj !=nullptr ) && (TOOL_BAR == m_pParentObj->m_accRole ) )
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
    if(msState!=UNO_MSAA_UNMAPPING)
        m_pIMAcc->DecreaseState(msState);
}

/**
   * Increase state of com object
   * @param xState The new state.
   * @return
   */
void AccObject::IncreaseState( sal_Int64 xState )
{
    if( nullptr == m_pIMAcc )
    {
        assert(false);
        return;
    }


    if( xState == AccessibleStateType::VISIBLE  )
    {
        m_pIMAcc->DecreaseState( STATE_SYSTEM_INVISIBLE );
    }
    else if( xState == AccessibleStateType::SHOWING )
    {
        m_pIMAcc->DecreaseState( STATE_SYSTEM_OFFSCREEN );
    }


    DWORD msState = GetMSAAStateFromUNO(xState);
    if(msState!=UNO_MSAA_UNMAPPING)
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
   * update action description desc
   * @param
   * @return
   */
void AccObject::UpdateActionDesc()
{
    if (!m_pIMAcc)
        return;

    long Role = m_accRole;

    if(  Role == PUSH_BUTTON || Role == RADIO_BUTTON || Role == MENU_ITEM ||
            Role == LIST_ITEM || Role == CHECK_BOX || Role == TREE_ITEM ||
            Role == CHECK_MENU_ITEM || Role == RADIO_MENU_ITEM )
    {
        UpdateDefaultAction(  );
    }
    else
    {

        if( m_xAccActionRef.is() )
        {
            if( m_xAccActionRef->getAccessibleActionCount() > 0 )
            {
                if (!(Role == SPIN_BOX || Role == COMBO_BOX || Role == DATE_EDITOR ||
                      Role == EDIT_BAR || Role == PASSWORD_TEXT || Role == TEXT))
                {
                    const OUString sActionDesc = m_xAccActionRef->getAccessibleActionDescription(0);
                    // if string is non-empty, action is set.
                    if (!sActionDesc.isEmpty())
                        m_pIMAcc->Put_ActionDescription(o3tl::toW(sActionDesc.getStr()));
                }
            }
        }
    }

}
/**
   * update role information from uno to com
   * @param
   * @return
   */
void AccObject::UpdateRole()
{
    if (!m_pIMAcc)
    {
        return;
    }

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
    if (!m_pIMAcc)
    {
        return;
    }

    XAccessibleContext* pContext  = m_xAccContextRef.get();
    sal_Int64 nRState = pContext->getAccessibleStateSet();

    m_pIMAcc->SetState(0);

    if ( m_accRole == POPUP_MENU )
    {
        return;
    }

    bool isEnable = false;
    bool isShowing = false;
    bool isEditable = false;
    bool isVisible = false;
    bool isFocusable = false;

    for (int i=0; i<63; ++i)
    {
        sal_Int64 nState = sal_Int64(1) << i;
        if (!(nState & nRState))
            continue;
        if (nState == ENABLED)
            isEnable = true;
        else if (nState == SHOWING)
            isShowing = true;
        else if (nState == VISIBLE)
            isVisible = true;
        else if (nState == EDITABLE)
            isEditable = true;
        else if (nState == FOCUSABLE)
            isFocusable = true;
        IncreaseState(nState);
    }

    bool bIsMenuItem = m_accRole == MENU_ITEM || m_accRole == RADIO_MENU_ITEM || m_accRole == CHECK_MENU_ITEM;

    if(bIsMenuItem)
    {
        if(!(isShowing && isVisible) )
        {
            m_pIMAcc->IncreaseState( STATE_SYSTEM_INVISIBLE );
            m_pIMAcc->DecreaseState( STATE_SYSTEM_FOCUSABLE );
        }
    }
    else
    {
        if(!(isShowing || isVisible) )
            m_pIMAcc->IncreaseState( STATE_SYSTEM_INVISIBLE );
    }

    switch(m_accRole)
    {
    case LABEL:
    case STATIC:
    case NOTIFICATION:
        m_pIMAcc->IncreaseState( STATE_SYSTEM_READONLY );
        break;
    case TEXT:
        // 2. editable combobox -> readonly ------ bridge
    case EMBEDDED_OBJECT:
    case END_NOTE:
    case FOOTER:
    case FOOTNOTE:
    case GRAPHIC:
    case HEADER:
    case HEADING:

        //Image Map
    case BLOCK_QUOTE:
    case PARAGRAPH:
    case PASSWORD_TEXT:
    case SHAPE:
    case SPIN_BOX:
    case TABLE:
    case TABLE_CELL:
    case TEXT_FRAME:
    case DATE_EDITOR:
    case DOCUMENT:
    case COLUMN_HEADER:
        {
            if(!isEditable)
                m_pIMAcc->IncreaseState( STATE_SYSTEM_READONLY );
        }
        break;
    default:
        break;
    }

    if( isEnable )
    {

        if (!(m_accRole == FILLER || m_accRole == END_NOTE || m_accRole == FOOTER || m_accRole == FOOTNOTE || m_accRole == GROUP_BOX || m_accRole == RULER
                || m_accRole == HEADER || m_accRole == ICON || m_accRole == INTERNAL_FRAME || m_accRole == LABEL || m_accRole == LAYERED_PANE
                || m_accRole == SCROLL_BAR || m_accRole == SCROLL_PANE || m_accRole == SPLIT_PANE || m_accRole == STATIC || m_accRole == STATUS_BAR
                || m_accRole == TOOL_TIP || m_accRole == NOTIFICATION))
        {
            if (m_accRole == SEPARATOR)
            {
                if( ( m_pParentObj != nullptr ) && ( MENU == m_pParentObj->m_accRole  || POPUP_MENU == m_pParentObj->m_accRole ))
                    IncreaseState( FOCUSABLE );
            }

            else if (m_accRole == TABLE_CELL || m_accRole == TABLE || m_accRole == PANEL || m_accRole == OPTION_PANE ||
                     m_accRole == COLUMN_HEADER)
            {
                if (isFocusable)
                    IncreaseState( FOCUSABLE );
            }
            else
            {
                if(bIsMenuItem)
                {
                    if ( isShowing && isVisible)
                    {
                        IncreaseState( FOCUSABLE );
                    }
                }
                else
                {
                    IncreaseState( FOCUSABLE );
                }
            }
        }
    }
    else
    {
        m_pIMAcc->IncreaseState( STATE_SYSTEM_UNAVAILABLE );
        if( !((m_accRole == MENU_ITEM) ||
                (m_accRole == RADIO_MENU_ITEM) ||
                (m_accRole == CHECK_MENU_ITEM)) )
        {
            if  (m_accRole == TOGGLE_BUTTON || m_accRole == PUSH_BUTTON || m_accRole == BUTTON_DROPDOWN)
            {
                if(( m_pParentObj != nullptr )&& (TOOL_BAR ==  m_pParentObj->m_accRole ) )
                    IncreaseState( FOCUSABLE );
                else
                    DecreaseState( FOCUSABLE );
            }
            else
                DecreaseState( FOCUSABLE );
        }
        else if( isShowing || isVisible )
        {
            IncreaseState( FOCUSABLE );
        }
    }

    switch(m_accRole)
    {
    case POPUP_MENU:
    case MENU:
        if( pContext->getAccessibleChildCount() > 0 )
            m_pIMAcc->IncreaseState( STATE_SYSTEM_HASPOPUP );
        break;
    case PASSWORD_TEXT:
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
    if( nullptr == m_pIMAcc || !m_xAccContextRef.is()  )
    {
        assert(false);
        return false;
    }

    UpdateName();

    UpdateValue();

    UpdateActionDesc();

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
    if(m_pIMAcc)
    {
        IncreaseState(FOCUSED);
        m_pIMAcc->Put_XAccFocus(CHILDID_SELF);

        UpdateRole();
    }
}

/**
   * Unset self from focus object in parent child list.
   * @param
   * @return
   */
void AccObject::unsetFocus()
{
    if(m_pIMAcc)
    {
        DecreaseState( FOCUSED );
        m_pIMAcc->Put_XAccFocus(UACC_NO_FOCUS);
    }
}

void AccObject::GetExpandedState( sal_Bool* isExpandable, sal_Bool* isExpanded)
{
    *isExpanded = false;
    *isExpandable = false;

    if( !m_xAccContextRef.is() )
    {
        return;
    }
    sal_Int64 nRState = m_xAccContextRef->getAccessibleStateSet();

    if (nRState & EXPANDED)
        *isExpanded = true;
    if (nRState & EXPANDABLE)
        *isExpandable = true;
}

void AccObject::NotifyDestroy()
{
    if(m_pIMAcc)
        m_pIMAcc->NotifyDestroy();
}

void AccObject::SetParentObj(AccObject* pParentAccObj)
{
    m_pParentObj = pParentAccObj;

    if(m_pIMAcc)
    {
        if(m_pParentObj)
        {
            m_pIMAcc->Put_XAccParent(m_pParentObj->GetIMAccessible());
        }
        else
        {
            m_pIMAcc->Put_XAccParent(nullptr);
        }
    }
}
//ResID means ChildID in MSAA
void AccObject::SetResID(long id)
{
    m_resID = id;
    if(m_pIMAcc)
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

AccEventListener* AccObject::getListener()
{
    return m_pListener.get();
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
