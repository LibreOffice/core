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

#include  <UAccCOM.h>
#include "MAccessible.h"

#include <algorithm>
#include <cstddef>

#include "AccAction.h"
#include "AccRelation.h"
#include "AccComponent.h"
#include "AccText.h"
#include "AccEditableText.h"
#include "AccImage.h"
#include "AccTable.h"
#include "AccTableCell.h"
#include "AccValue.h"
#include "AccHypertext.h"
#include "AccHyperLink.h"

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/accessibility/AccessibleTextAttributeHelper.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <comphelper/AccessibleImplementationHelper.hxx>

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleHyperlink.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleGroupPosition.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;

namespace {

enum class XInterfaceType {
    XI_COMPONENT,
    XI_TEXT,
    XI_TABLE,
    XI_TABLECELL,
    XI_EDITABLETEXT,
    XI_IMAGE,
    XI_SELECTION,
    XI_EXTENDEDCOMP,
    XI_VALUE,
    XI_KEYBINDING,
    XI_ACTION,
    XI_HYPERTEXT,
    XI_HYPERLINK,
    XI_ATTRIBUTE
};

template <class Interface>
bool queryXInterface(XAccessible* pXAcc, XInterface** ppXI)
{
    if (!pXAcc)
        return false;

    Reference<XAccessibleContext> pRContext = pXAcc->getAccessibleContext();
    if (!pRContext.is())
        return false;

    Reference<Interface> pRXI(pRContext, UNO_QUERY);
    if (!pRXI.is())
        return false;

    *ppXI = pRXI.get();
    return true;
}

// Since there's no specific XInterface for table cells, this
// method checks that the accessible's parent is a table
// (implements XAccessibleTable) and pXAcc's context implements
// XAccessibleComponent.
bool queryTableCell(XAccessible* pXAcc, XInterface** ppXI)
{
    XInterface* pXInterface = nullptr;

    const bool bSupportsInterface = queryXInterface<XAccessibleComponent>(pXAcc, &pXInterface);
    if (!bSupportsInterface)
        return false;

    // check whether parent is a table (its accessible context implements XAccessibleTable)
    XInterface* pParentXInterface = nullptr;
    Reference<XAccessible> xParentAcc = pXAcc->getAccessibleContext()->getAccessibleParent();
    const bool bParentIsTable = queryXInterface<XAccessibleTable>(xParentAcc.get(), &pParentXInterface);

    if (!bParentIsTable)
        return false;

    *ppXI = pXInterface;
    return true;
}


void lcl_addIA2State(AccessibleStates& rStates, sal_Int64 nUnoState, sal_Int16 nRole)
{
    switch (nUnoState)
    {
        case css::accessibility::AccessibleStateType::ACTIVE:
            rStates |= IA2_STATE_ACTIVE;
            break;
        case css::accessibility::AccessibleStateType::ARMED:
            rStates |= IA2_STATE_ARMED;
            break;
        case css::accessibility::AccessibleStateType::CHECKABLE:
            // STATE_SYSTEM_PRESSED is used instead of STATE_SYSTEM_CHECKED for these button
            // roles (s. AccObject::GetMSAAStateFromUNO), so don't set CHECKABLE state for them
            if (nRole != AccessibleRole::PUSH_BUTTON && nRole != AccessibleRole::TOGGLE_BUTTON)
                rStates |= IA2_STATE_CHECKABLE;
            break;
        case css::accessibility::AccessibleStateType::DEFUNC:
            rStates |= IA2_STATE_DEFUNCT;
            break;
        case css::accessibility::AccessibleStateType::EDITABLE:
            rStates |= IA2_STATE_EDITABLE;
            break;
        case css::accessibility::AccessibleStateType::HORIZONTAL:
            rStates |= IA2_STATE_HORIZONTAL;
            break;
        case css::accessibility::AccessibleStateType::ICONIFIED:
            rStates |= IA2_STATE_ICONIFIED;
            break;
        case css::accessibility::AccessibleStateType::MANAGES_DESCENDANTS:
            rStates |= IA2_STATE_MANAGES_DESCENDANTS;
            break;
        case css::accessibility::AccessibleStateType::MODAL:
            rStates |= IA2_STATE_MODAL;
            break;
        case css::accessibility::AccessibleStateType::MULTI_LINE:
            rStates |= IA2_STATE_MULTI_LINE;
            break;
        case css::accessibility::AccessibleStateType::OPAQUE:
            rStates |= IA2_STATE_OPAQUE;
            break;
        case css::accessibility::AccessibleStateType::SINGLE_LINE:
            rStates |= IA2_STATE_SINGLE_LINE;
            break;
        case css::accessibility::AccessibleStateType::STALE:
            rStates |= IA2_STATE_STALE;
            break;
        case css::accessibility::AccessibleStateType::TRANSIENT:
            rStates |= IA2_STATE_TRANSIENT;
            break;
        case css::accessibility::AccessibleStateType::VERTICAL:
            rStates |= IA2_STATE_VERTICAL;
            break;
        default:
            // no match
            break;
    }
}

}

AccObjectWinManager* CMAccessible::g_pAccObjectManager = nullptr;

CMAccessible::CMAccessible():
m_pszName(nullptr),
m_pszValue(nullptr),
m_pszActionDescription(nullptr),
m_iRole(0x00),
m_dState(0x00),
m_pIParent(nullptr),
m_dChildID(0x00),
m_dFocusChildID(UACC_NO_FOCUS),
m_hwnd(nullptr),
m_isDestroy(false)
{
    CEnumVariant::Create(&m_pEnumVar);
    m_containedObjects.clear();
}

CMAccessible::~CMAccessible()
{
    SolarMutexGuard g;

    if(m_pszName!=nullptr)
    {
        SysFreeString(std::exchange(m_pszName, nullptr));
    }
    if(m_pszValue!=nullptr)
    {
        SysFreeString(std::exchange(m_pszValue, nullptr));
    }

    if(m_pszActionDescription!=nullptr)
    {
        SysFreeString(std::exchange(m_pszActionDescription, nullptr));
    }

    if(m_pIParent)
    {
        m_pIParent->Release();
        m_pIParent=nullptr;
    }
    m_pEnumVar->Release();
    m_containedObjects.clear();
}

/**
* Returns the Parent IAccessible interface pointer to AT.
* It should add reference, and the client should release the component.
* It should return E_FAIL when the parent point is null.
* @param    ppdispParent [in,out] used to return the parent interface point.
*           when the point is null, should return null.
* @return   S_OK if successful and E_FAIL if the m_pIParent is NULL.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accParent(IDispatch **ppdispParent)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(ppdispParent == nullptr)
        {
            return E_INVALIDARG;
        }

        if(m_pIParent)
        {
            *ppdispParent = m_pIParent;
            (*ppdispParent)->AddRef();
            return S_OK;
        }
        else if(m_hwnd)
        {
            HRESULT hr = AccessibleObjectFromWindow(m_hwnd, OBJID_WINDOW, IID_IAccessible, reinterpret_cast<void**>(ppdispParent));
            if (!SUCCEEDED(hr) || !*ppdispParent)
            {
                return S_FALSE;
            }
            return S_OK;
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns child count of current COM object.
* @param    pcountChildren [in,out] used to return the children count.
* @return   S_OK if successful.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accChildCount(long *pcountChildren)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pcountChildren == nullptr)
        {
            return E_INVALIDARG;
        }

        if (!m_xAccessible.is())
            return S_FALSE;

        Reference<XAccessibleContext> const pRContext =
            m_xAccessible->getAccessibleContext();
        if( pRContext.is() )
        {
            sal_Int64 nChildCount = pRContext->getAccessibleChildCount();
            if (nChildCount > std::numeric_limits<long>::max())
            {
                SAL_WARN("iacc2", "CMAccessible::get_accChildCount: Child count exceeds maximum long value, "
                                  "returning max long.");
                nChildCount = std::numeric_limits<long>::max();
            }

            *pcountChildren = nChildCount;
        }

        return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
* Returns child interface pointer for AT according to input child ID.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    ppdispChild, [in,out] use to return the child interface point.
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accChild(VARIANT varChild, IDispatch **ppdispChild)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(ppdispChild == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varChild.vt==VT_I4)
        {
            //get child interface pointer due to child ID
            if(varChild.lVal==CHILDID_SELF)
            {
                AddRef();
                *ppdispChild = this;
                return S_OK;
            }
            *ppdispChild = GetChildInterface(varChild.lVal);
            if((*ppdispChild) == nullptr)
                return E_FAIL;
            (*ppdispChild)->AddRef();
            return S_OK;
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the accessible name of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pszName, [in,out] use to return the name of the proper object.
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accName(VARIANT varChild, BSTR *pszName)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pszName == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varChild.vt==VT_I4)
        {
            if(varChild.lVal==CHILDID_SELF)
            {
                SysFreeString(*pszName);
                *pszName = SysAllocString(m_pszName);
                return S_OK;
            }

            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;
            return pChild->get_accName(varChild,pszName);
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the accessible value of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pszValue, [in,out] use to return the value of the proper object.
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accValue(VARIANT varChild, BSTR *pszValue)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if( pszValue == nullptr )
        {
            return E_INVALIDARG;
        }
        if( varChild.vt==VT_I4 )
        {
            if(varChild.lVal==CHILDID_SELF)
            {
                if(m_dState & STATE_SYSTEM_PROTECTED)
                    return E_ACCESSDENIED;

                if ( m_pszValue !=nullptr && wcslen(m_pszValue) == 0 )
                    return S_OK;

                SysFreeString(*pszValue);
                *pszValue = SysAllocString(m_pszValue);
                return S_OK;
            }

            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;
            return pChild->get_accValue(varChild,pszValue);
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the accessible description of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pszDescription, [in,out] use to return the description of the proper object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pszDescription == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varChild.vt==VT_I4)
        {
            if(varChild.lVal==CHILDID_SELF)
            {
                if (!m_xAccessible.is())
                    return S_FALSE;

                Reference<XAccessibleContext> xContext = m_xAccessible->getAccessibleContext();
                if (!xContext.is())
                    return S_FALSE;

                const OUString sDescription = xContext->getAccessibleDescription();
                SysFreeString(*pszDescription);
                *pszDescription = SysAllocString(o3tl::toW(sDescription.getStr()));
                return S_OK;
            }

            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;
            return pChild->get_accDescription(varChild,pszDescription);
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the accessible role of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pvarRole, [in,out] use to return the role of the proper object.
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarRole == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varChild.vt == VT_I4)
        {

            if(varChild.lVal == CHILDID_SELF)
            {
                VariantInit(pvarRole);
                pvarRole->vt = VT_I4;

                if (m_iRole < IA2_ROLE_CAPTION)
                    pvarRole->lVal = m_iRole;
                else
                    pvarRole->lVal = ROLE_SYSTEM_CLIENT;

                return S_OK;
            }


            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;
            return pChild->get_accRole(varChild,pvarRole);
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the accessible state of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pvarState, [in,out] use to return the state of the proper object.
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarState == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varChild.vt==VT_I4)
        {
            if(varChild.lVal == CHILDID_SELF)
            {
                if (m_xAccessible.is())
                {
                    Reference<XAccessibleContext> const pContext =
                        m_xAccessible->getAccessibleContext();
                    if(pContext.is())
                    {
                        // add the STATE_SYSTEM_LINKED state
                        Reference< XAccessibleHypertext > pRHypertext(pContext,UNO_QUERY);
                        if(pRHypertext.is())
                        {
                            if( pRHypertext->getHyperLinkCount() > 0 )
                                m_dState |= STATE_SYSTEM_LINKED;
                            else
                                m_dState &= ~STATE_SYSTEM_LINKED;
                        }
                        else
                            m_dState &= ~STATE_SYSTEM_LINKED;
                    }
                }

                VariantInit(pvarState);
                pvarState->vt = VT_I4;
                pvarState->lVal = m_dState;
                return S_OK;
            }

            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;
            return pChild->get_accState(varChild,pvarState);
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the accessible helpString of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pszHelp, [in,out] use to return the helpString of the proper object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accHelp(VARIANT, BSTR *)
{
    return E_NOTIMPL;
}

/**
* Returns the accessible HelpTopic of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pszHelpFile, [in,out] use to return the HelpTopic of the proper object.
* @param    pidTopic, use to return the HelpTopic ID of the proper object.
* @return   S_OK if successful and E_FAIL if failure.
* Not implemented yet
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accHelpTopic(BSTR *, VARIANT, long *)
{
    return E_NOTIMPL;
}

static bool GetMnemonicChar( const OUString& aStr, sal_Unicode* wStr)
{
    for (sal_Int32 i = 0;; i += 2) {
        i = aStr.indexOf('~', i);
        if (i == -1 || i == aStr.getLength() - 1) {
            return false;
        }
        auto c = aStr[i + 1];
        if (c != '~') {
            *wStr = c;
            return true;
        }
    }
}

/**
* Returns the accessible keyboard shortcut of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pszKeyboardShortcut, [in,out] use to return the kbshortcut of the proper object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
    SolarMutexGuard g;

    try {

        if (m_isDestroy) return S_FALSE;

        if(pszKeyboardShortcut == nullptr)
        {
            return E_INVALIDARG;
        }

        if(varChild.vt==VT_I4)
        {
            if(varChild.lVal == CHILDID_SELF)
            {
                if (m_xAccessible.is())
                {
                    Reference<XAccessibleContext> const pRContext =
                        m_xAccessible->getAccessibleContext();
                    if( !pRContext.is() )
                        return S_FALSE;

                    Reference<XAccessibleAction> pRXI(pRContext,UNO_QUERY);

                    OUString wString;

                    if( pRXI.is() && pRXI->getAccessibleActionCount() >= 1)
                    {
                        Reference< XAccessibleKeyBinding > binding = pRXI->getAccessibleActionKeyBinding(0);
                        if( binding.is() )
                        {
                            long nCount = binding->getAccessibleKeyBindingCount();
                            if(nCount >= 1)
                            {
                                wString = comphelper::GetkeyBindingStrByXkeyBinding( binding->getAccessibleKeyBinding(0) );
                            }
                        }
                    }
                    if(wString.isEmpty())
                    {
                        Reference<XAccessibleRelationSet> pRrelationSet = pRContext->getAccessibleRelationSet();
                        if(!pRrelationSet.is())
                        {
                            return S_FALSE;
                        }

                        long nRelCount = pRrelationSet->getRelationCount();

                        // Modified by Steve Yin, for SODC_1552
                        if( /*nRelCount <= 0 &&*/ m_iRole == ROLE_SYSTEM_TEXT )
                        {
                            VARIANT varParentRole;
                            VariantInit( &varParentRole );

                            if (m_pIParent
                                && SUCCEEDED(m_pIParent->get_accRole(varChild, &varParentRole))
                                && varParentRole.lVal == ROLE_SYSTEM_COMBOBOX) // edit in comboBox
                            {
                                m_pIParent->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
                                return S_OK;
                            }
                        }

                        AccessibleRelation *paccRelation = nullptr;
                        AccessibleRelation accRelation;
                        for(int i=0; i<nRelCount ; i++)
                        {
                            if (pRrelationSet->getRelation(i).RelationType == AccessibleRelationType::LABELED_BY)
                            {
                                accRelation = pRrelationSet->getRelation(i);
                                paccRelation = &accRelation;
                            }
                        }

                        if(paccRelation == nullptr)
                            return S_FALSE;

                        Sequence< Reference< XInterface > > xTargets = paccRelation->TargetSet;
                        Reference<XInterface> pRAcc = xTargets[0];

                        XAccessible* pXAcc = static_cast<XAccessible*>(pRAcc.get());

                        Reference<XAccessibleContext> xLabelContext = pXAcc->getAccessibleContext();
                        if (!xLabelContext.is())
                            return S_FALSE;

                        pRrelationSet = xLabelContext->getAccessibleRelationSet();
                        nRelCount = pRrelationSet->getRelationCount();

                        paccRelation = nullptr;
                        for(int j=0; j<nRelCount ; j++)
                        {
                            if (pRrelationSet->getRelation(j).RelationType == AccessibleRelationType::LABEL_FOR)
                            {
                                accRelation = pRrelationSet->getRelation(j);
                                paccRelation = &accRelation;
                            }
                        }

                        if(paccRelation)
                        {
                            xTargets = paccRelation->TargetSet;
                            pRAcc = xTargets[0];
                            if (m_xAccessible.get() != static_cast<XAccessible*>(pRAcc.get()))
                                return S_FALSE;
                        }

                        Reference<XAccessibleExtendedComponent> pRXIE(xLabelContext, UNO_QUERY);
                        if(!pRXIE.is())
                            return S_FALSE;

                        OUString ouStr = pRXIE->getTitledBorderText();
                        sal_Unicode key;
                        if(GetMnemonicChar(ouStr, &key))
                        {
                            wString = "Alt+" + OUStringChar(key);
                        }
                        else
                            return S_FALSE;
                    }

                    SysFreeString(*pszKeyboardShortcut);
                    *pszKeyboardShortcut = SysAllocString(o3tl::toW(wString.getStr()));

                    return S_OK;
                }
                else
                {
                    return S_FALSE;
                }
            }

            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;

            return pChild->get_accKeyboardShortcut(varChild,pszKeyboardShortcut);
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the current focused child to AT.
* @param    pvarChild, [in,out] vt member of pvarChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accFocus(VARIANT *pvarChild)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarChild == nullptr)
        {
            return E_INVALIDARG;
        }
        if( m_dFocusChildID==UACC_NO_FOCUS )
        {
            pvarChild->vt = VT_EMPTY;//no focus on the object and its children
            return S_OK;
        }
        //if the descendant of current object has focus indicated by m_dFocusChildID, return the IDispatch of this focused object
        else
        {
            IMAccessible* pIMAcc = g_pAccObjectManager->GetIAccessibleFromResID(m_dFocusChildID);
            if (pIMAcc == nullptr)
            {
                return E_FAIL;
            }
            pIMAcc->AddRef();
            pvarChild->vt = VT_DISPATCH;
            pvarChild->pdispVal = pIMAcc;

        }
        return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the selection of the current COM object to AT.
* @param    pvarChildren,[in,out]
* if selection num is 0,return VT_EMPTY for vt,
* if selection num is 1,return VT_I4 for vt,and child index for lVal
* if selection num >1,return VT_UNKNOWN for vt, and IEnumVariant* for punkVal
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_accSelection(VARIANT *pvarChildren)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarChildren == nullptr)
        {
            return E_INVALIDARG;
        }
        switch(m_pEnumVar->GetCountOfElements())
        {
        case 0:
            pvarChildren->vt = VT_EMPTY;
            break;
        case 1:
            VARIANT varTmp[1];
            ULONG count;
            VariantInit(&varTmp[0]);
            m_pEnumVar->Next(1,varTmp,&count);
            if(count!=1)
                return S_FALSE;
            pvarChildren->vt = VT_DISPATCH;
            pvarChildren->pdispVal = varTmp[0].pdispVal;
            pvarChildren->pdispVal->AddRef();
            VariantClear(&varTmp[0]);
            m_pEnumVar->Reset();
            break;
        default:
            pvarChildren->vt = VT_UNKNOWN;
            IEnumVARIANT* pClone;
            m_pEnumVar->Clone(&pClone);
            pClone->Reset();
            pvarChildren->punkVal = pClone;
            break;
        }
        return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the location of the current COM object self or its one child to AT.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    pxLeft, [in,out] use to return the x-coordination of the proper object.
* @param    pyTop,  [in,out] use to return the y-coordination of the proper object.
* @param    pcxWidth, [in,out] use to return the x-coordination width of the proper object.
* @param    pcyHeight, [in,out] use to return the y-coordination height of the proper object.
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pxLeft == nullptr || pyTop == nullptr || pcxWidth == nullptr || pcyHeight == nullptr)
        {
            return E_INVALIDARG;
        }

        if(varChild.vt==VT_I4)
        {
            if(varChild.lVal==CHILDID_SELF)
            {
                if (!m_xAccessible.is())
                    return S_FALSE;

                Reference<XAccessibleContext> const pRContext =
                    m_xAccessible->getAccessibleContext();
                if( !pRContext.is() )
                    return S_FALSE;
                Reference< XAccessibleComponent > pRComponent(pRContext,UNO_QUERY);
                if( !pRComponent.is() )
                    return S_FALSE;

                css::awt::Point pCPoint = pRComponent->getLocationOnScreen();
                css::awt::Size pCSize = pRComponent->getSize();
                *pxLeft = pCPoint.X;
                *pyTop =  pCPoint.Y;
                *pcxWidth = pCSize.Width;
                *pcyHeight = pCSize.Height;
                return S_OK;
            }
        }
        return S_FALSE;

    } catch(...) { return E_FAIL; }
}

/**
* Returns the current focused child to AT.
* @param    navDir, the direction flag of the navigation.
* @param    varStart, the start child id of this navigation action.
* @param    pvarEndUpAt, [in,out] the end up child of this navigation action.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarEndUpAt == nullptr)
        {
            return E_INVALIDARG;
        }
        HRESULT ret = E_FAIL;
        switch (navDir)
        {
        case NAVDIR_FIRSTCHILD:
            ret = GetFirstChild(varStart,pvarEndUpAt);
            break;
        case NAVDIR_LASTCHILD:
            ret = GetLastChild(varStart,pvarEndUpAt);
            break;
        case NAVDIR_NEXT:
            ret = GetNextSibling(varStart,pvarEndUpAt);
            break;
        case NAVDIR_PREVIOUS:
            ret = GetPreSibling(varStart,pvarEndUpAt);
            break;
        case NAVDIR_DOWN://do not implement temporarily
            break;
        case NAVDIR_UP://do not implement temporarily
            break;
        case NAVDIR_LEFT://do not implement temporarily
            break;
        case NAVDIR_RIGHT://do not implement temporarily
            break;
        default:
            break;
        }
        return ret;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
    SolarMutexGuard g;

    if (m_isDestroy)
        return S_FALSE;

    if (!pvarChild)
        return E_INVALIDARG;

    try
    {
        pvarChild->vt = VT_EMPTY;

        Reference<XAccessibleContext> xContext = GetContextByXAcc(m_xAccessible.get());
        Reference<XAccessibleComponent> xComponent(xContext, UNO_QUERY);
        if (!xComponent.is())
            return S_FALSE;

        // convert from screen to object-local coordinates
        css::awt::Point aTopLeft = xComponent->getLocationOnScreen();
        css::awt::Point aPoint(xLeft - aTopLeft.X, yTop - aTopLeft.Y);

        Reference<XAccessible> xAccAtPoint = xComponent->getAccessibleAtPoint(aPoint);
        if (!xAccAtPoint.is())
            return S_FALSE;

        IAccessible* pRet = get_IAccessibleFromXAccessible(xAccAtPoint.get());
        if (!pRet)
        {
            g_pAccObjectManager->InsertAccObj(xAccAtPoint.get(), m_xAccessible.get(), m_hwnd);
            pRet = get_IAccessibleFromXAccessible(xAccAtPoint.get());
        }
        if (!pRet)
            return S_FALSE;

        pvarChild->vt = VT_DISPATCH;
        pvarChild->pdispVal = pRet;
        pRet->AddRef();

        return S_OK;
    } catch(...) { return E_FAIL; }
}

/**
* Get The other Interface from CMAccessible.
* @param    guidService, must be IID_IAccessible here.
* @param    riid, the IID interface .
* @return   S_OK if successful and S_FALSE if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::QueryService(REFGUID guidService, REFIID riid, void** ppvObject)
{
    if( InlineIsEqualGUID(guidService, IID_IAccessible) )
        return QueryInterface(riid, ppvObject);
    return S_FALSE;
}

/**
* No longer supported according to IAccessible doc.
* Servers should return E_NOTIMPL
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::put_accName(VARIANT, BSTR)
{
    return E_NOTIMPL;
}

/**
* Set the accessible value of the current COM object self or its one child from UNO.
* @param    varChild, vt member of varChild must be VT_I4,and lVal member stores the child ID,
* the child ID specify child index from 0 to children count, 0 stands for object self.
* @param    szValue, the value used to set the value of the proper object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::put_accValue(VARIANT varChild, BSTR szValue)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;
        if(varChild.vt==VT_I4)
        {
            if(varChild.lVal==CHILDID_SELF)
            {
                SysFreeString(m_pszValue);
                m_pszValue=SysAllocString(szValue);
                return S_OK;
            }

            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;
            return pChild->put_accValue(varChild,szValue);
        }
        return E_FAIL;

        } catch(...) { return E_FAIL; }
}

/**
* Set the accessible name of the current COM object self from UNO.
* @param    pszName, the name value used to set the name of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccName(const OLECHAR __RPC_FAR *pszName)
{
    // internal IMAccessible - no mutex meeded

    try {
        if (m_isDestroy) return S_FALSE;

        if(pszName == nullptr)
        {
            return E_INVALIDARG;
        }

        SysFreeString(m_pszName);
        m_pszName = SysAllocString(pszName);
        if(m_pszName==nullptr)
            return E_FAIL;
        return S_OK;

        } catch(...) { return E_FAIL; }
}

/**
* Set the accessible role of the current COM object self from UNO.
* @param    pRole, the role value used to set the role of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccRole(unsigned short pRole)
{
    // internal IMAccessible - no mutex meeded

    m_iRole = pRole;
    return S_OK;
}

/**
* Add one state into the current state set for the current COM object from UNO.
* @param    pXSate, the state used to set the name of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::DecreaseState(DWORD pXSate)
{
    // internal IMAccessible - no mutex meeded

    m_dState &= (~pXSate);
    return S_OK;
}

/**
* Delete one state into the current state set for the current COM object from UNO.
* @param    pXSate, the state used to set the name of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::IncreaseState(DWORD pXSate)
{
    // internal IMAccessible - no mutex meeded

    m_dState |= pXSate;
    return S_OK;
}

/**
* Set state into the current state set for the current COM object from UNO.
* @param    pXSate, the state used to set the name of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::SetState(DWORD pXSate)
{
    // internal IMAccessible - no mutex meeded

    m_dState = pXSate;
    return S_OK;
}

/**
* Set the accessible value of the current COM object self from UNO.
* @param    pszAccValue, the name used to set the value of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccValue(const OLECHAR __RPC_FAR *pszAccValue)
{
    // internal IMAccessible - no mutex meeded

    try {
        if (m_isDestroy) return S_FALSE;

        if(pszAccValue == nullptr)
        {
            return E_INVALIDARG;
        }
        SysFreeString(m_pszValue);
        m_pszValue = SysAllocString(pszAccValue);
        if(m_pszValue==nullptr)
            return E_FAIL;
        return S_OK;

        } catch(...) { return E_FAIL; }
}

/**
* Set the HWND value of the current COM object self from UNO. It should set the parent IAccessible
* Object through the method AccessibleObjectFromWindow(...).
* @param    hwnd, the HWND used to set the value of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccWindowHandle(HWND hwnd)
{
    // internal IMAccessible - no mutex meeded

    try {
        if (m_isDestroy) return S_FALSE;
        m_hwnd = hwnd;
        return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
* Set accessible focus by specifying child ID
* @param    dChildID, the child id identifies the focus child.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccFocus(long dChildID)
{
    // internal IMAccessible - no mutex meeded

    try {
        if (m_isDestroy) return S_FALSE;

        if(dChildID==CHILDID_SELF)
        {
            if(m_pIParent)
            {
                m_pIParent->Put_XAccFocus(m_dChildID);
            }
        }
        else
        {
            m_dFocusChildID = dChildID;
            //traverse all ancestors to set the focused child ID so that when the get_accFocus is called on
            //any of the ancestors, this id can be used to get the IAccessible of focused object.
            if(m_pIParent)
            {
                m_pIParent->Put_XAccFocus(dChildID);
            }
        }
        return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
* Set accessible parent object for the current COM object if
* the current object is a child of some COM object
* @param    pIParent, the parent of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccParent(IMAccessible __RPC_FAR *pIParent)
{
    // internal IMAccessible - no mutex meeded

    this->m_pIParent = pIParent;

    if(pIParent)
        m_pIParent->AddRef();

    return S_OK;
}

/**
* Set unique child id to COM
* @param    dChildID, the id of the current object.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccChildID(long dChildID)
{
    // internal IMAccessible - no mutex meeded

    this->m_dChildID = dChildID;
    return S_OK;
}

/**
* Set AccObjectWinManager object pointer to COM
* @param    pManager, the AccObjectWinManager pointer.
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_XAccObjectManager(hyper pManager)
{
    // internal IMAccessible - no mutex meeded

    g_pAccObjectManager = reinterpret_cast<AccObjectWinManager*>(pManager);
    return S_OK;
}

/**
* When a UNO control disposing, it disposes its listeners,
* then notify AccObject in bridge management, then notify
* COM that the XAccessible is invalid, so set m_xAccessible as NULL
* @return   S_OK if successful and E_FAIL if failure.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::NotifyDestroy()
{
    // internal IMAccessible - no mutex meeded

    m_isDestroy = true;
    m_xAccessible.clear();
    return S_OK;
}

/**
*private methods that help implement public functions
*/

/**
* Return child interface pointer by child ID,note: need to call AddRef()
* @param    lChildID, specify child index,which AT(such as Inspect32) gives.
* @return  IMAccessible*, pointer to the corresponding child object.
*/
IMAccessible* CMAccessible::GetChildInterface(long dChildID)//for test
{
    if(dChildID<0)
    {
        if(g_pAccObjectManager)
        {
            IMAccessible* pIMAcc = g_pAccObjectManager->GetIAccessibleFromResID(dChildID);
            return pIMAcc;
        }
        return nullptr;
    }
    else
    {
        if (!m_xAccessible.is())
            return nullptr;

        Reference<XAccessibleContext> const pRContext =
            m_xAccessible->getAccessibleContext();
        if( !pRContext.is() )
            return nullptr;

        if(dChildID<1 || dChildID>pRContext->getAccessibleChildCount())
            return nullptr;

        Reference< XAccessible > pXChild = pRContext->getAccessibleChild(dChildID-1);
        IAccessible* pChild = get_IAccessibleFromXAccessible(pXChild.get());

        if(!pChild)
        {
            g_pAccObjectManager->InsertAccObj(pXChild.get(), m_xAccessible.get(), m_hwnd);
            pChild = get_IAccessibleFromXAccessible(pXChild.get());
        }

        if (pChild)
        {
            IMAccessible* pIMAcc =  static_cast<IMAccessible*>(pChild);
            return pIMAcc;
        }
    }

    return nullptr;
}

/**
* for descendantmanager circumstance,provide child interface when navigate
* @param    varCur, the current child.
* @param    flags, the navigation direction.
* @return  IMAccessible*, the child of the end up node.
*/
IMAccessible* CMAccessible::GetNavigateChildForDM(VARIANT varCur, short flags)
{

    XAccessibleContext* pXContext = GetContextByXAcc(m_xAccessible.get());
    if(pXContext==nullptr)
    {
        return nullptr;
    }

    sal_Int64 count = pXContext->getAccessibleChildCount();
    if(count<1)
    {
        return nullptr;
    }

    IMAccessible* pCurChild = nullptr;
    union {
        XAccessible* pChildXAcc;
        hyper nHyper = 0;
    };
    Reference<XAccessible> pRChildXAcc;
    XAccessibleContext* pChildContext = nullptr;
    sal_Int64 index = 0, delta = 0;
    switch(flags)
    {
    case DM_FIRSTCHILD:
        pRChildXAcc = pXContext->getAccessibleChild(0);
        break;
    case DM_LASTCHILD:
        pRChildXAcc = pXContext->getAccessibleChild(count-1);
        break;
    case DM_NEXTCHILD:
    case DM_PREVCHILD:
        pCurChild = GetChildInterface(varCur.lVal);
        if(pCurChild==nullptr)
        {
            return nullptr;
        }
        pCurChild->GetUNOInterface(&nHyper);
        if(pChildXAcc==nullptr)
        {
            return nullptr;
        }
        pChildContext = GetContextByXAcc(pChildXAcc);
        if(pChildContext == nullptr)
        {
            return nullptr;
        }
        delta = (flags==DM_NEXTCHILD)?1:-1;
        //currently, getAccessibleIndexInParent is error in UNO for
        //some kind of List,such as ValueSet, the index will be less 1 than
        //what should be, need to fix UNO code
        index = pChildContext->getAccessibleIndexInParent()+delta;
        if((index>=0)&&(index<=count-1))
        {
            pRChildXAcc = pXContext->getAccessibleChild(index);
        }
        break;
    default:
        break;
    }

    if(!pRChildXAcc.is())
    {
        return nullptr;
    }
    pChildXAcc = pRChildXAcc.get();
    g_pAccObjectManager->InsertAccObj(pChildXAcc, m_xAccessible.get());
    return g_pAccObjectManager->GetIAccessibleFromXAccessible(pChildXAcc);
}

/**
*the following 4 private methods are for accNavigate implementation
*/

/**
* Return first child for parent container, process differently according
* to whether it is descendant manage
* @param    varStart, the start child id of this navigation action.
* @param    pvarEndUpAt, [in,out] the end up child of this navigation action.
* @return   S_OK if successful and E_FAIL if failure.
*/
HRESULT CMAccessible::GetFirstChild(VARIANT varStart,VARIANT* pvarEndUpAt)
{

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarEndUpAt == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varStart.vt != VT_I4)
        {
            pvarEndUpAt->vt = VT_EMPTY;
            return E_INVALIDARG;
        }

        pvarEndUpAt->pdispVal = GetNavigateChildForDM(varStart, DM_FIRSTCHILD);
        if(pvarEndUpAt->pdispVal)
        {
            pvarEndUpAt->pdispVal->AddRef();
            pvarEndUpAt->vt = VT_DISPATCH;
            return S_OK;
        }

        pvarEndUpAt->vt = VT_EMPTY;
        return E_FAIL;

        } catch(...) { return E_FAIL; }
}

/**
* Return last child for parent container, process differently according
* to whether it is descendant manage
* @param    varStart, the start child id of this navigation action.
* @param    pvarEndUpAt, [in,out] the end up child of this navigation action.
* @return   S_OK if successful and E_FAIL if failure.
*/
HRESULT CMAccessible::GetLastChild(VARIANT varStart,VARIANT* pvarEndUpAt)
{

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarEndUpAt == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varStart.vt != VT_I4)
        {
            pvarEndUpAt->vt = VT_EMPTY;
            return E_INVALIDARG;
        }

        pvarEndUpAt->pdispVal = GetNavigateChildForDM(varStart, DM_LASTCHILD);
        if(pvarEndUpAt->pdispVal)
        {
            pvarEndUpAt->pdispVal->AddRef();
            pvarEndUpAt->vt = VT_DISPATCH;
            return S_OK;
        }
        pvarEndUpAt->vt = VT_EMPTY;
        return E_FAIL;

        } catch(...) { return E_FAIL; }
}

/**
* The method GetNextSibling is general, whatever it is descendant manage or not
* Get the next sibling object.
* @param    varStart, the start child id of this navigation action.
* @param    pvarEndUpAt, [in,out] the end up child of this navigation action.
* @return   S_OK if successful and E_FAIL if failure.
*/
HRESULT CMAccessible::GetNextSibling(VARIANT varStart,VARIANT* pvarEndUpAt)
{

    try {
        if (m_isDestroy) return S_FALSE;
        if(varStart.vt != VT_I4)
        {
            pvarEndUpAt->vt = VT_EMPTY;
            return E_INVALIDARG;
        }

        Reference<XAccessibleContext> const pRContext =
            GetContextByXAcc(m_xAccessible.get());
        if(pRContext.is())
        {
            varStart.iVal = sal_Int16(pRContext->getAccessibleIndexInParent() + 2);
            if(m_pIParent)
                if( m_pIParent->get_accChild(varStart,&pvarEndUpAt->pdispVal) == S_OK)
                {
                    pvarEndUpAt->vt = VT_DISPATCH;
                    return S_OK;
                }
        }
        pvarEndUpAt->vt = VT_EMPTY;
        return E_FAIL;

        } catch(...) { return E_FAIL; }
}

/**
*the method GetPreSibling is general, whatever it is descendant manage or not
* @param    varStart, the start child id of this navigation action.
* @param    pvarEndUpAt, [in,out] the end up child of this navigation action.
* @return   S_OK if successful and E_FAIL if failure.
*/
HRESULT CMAccessible::GetPreSibling(VARIANT varStart,VARIANT* pvarEndUpAt)
{

    try {
        if (m_isDestroy) return S_FALSE;

        if(pvarEndUpAt == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varStart.vt != VT_I4)
        {
            pvarEndUpAt->vt = VT_EMPTY;
            return E_INVALIDARG;
        }

        Reference<XAccessibleContext> const pRContext =
            GetContextByXAcc(m_xAccessible.get());
        if(pRContext.is())
        {
            varStart.iVal = sal_Int16(pRContext->getAccessibleIndexInParent());
            if(m_pIParent && varStart.iVal > 0)
                if( m_pIParent->get_accChild(varStart,&pvarEndUpAt->pdispVal) == S_OK)
                {
                    pvarEndUpAt->vt = VT_DISPATCH;
                    return S_OK;
                }
        }
        pvarEndUpAt->vt = VT_EMPTY;
        return E_FAIL;

        } catch(...) { return E_FAIL; }
}

/**
* For IAccessible2 implementation methods
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_nRelations( long __RPC_FAR *nRelations)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(nRelations == nullptr)
        {
            return E_INVALIDARG;
        }

        *nRelations = 0;

        if (!m_xContext.is())
            return E_FAIL;
        Reference<XAccessibleRelationSet> pRrelationSet =
            m_xContext->getAccessibleRelationSet();
        if(!pRrelationSet.is())
        {
            *nRelations = 0;
            return S_OK;
        }

        *nRelations = pRrelationSet->getRelationCount();
        return S_OK;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_relation( long relationIndex, IAccessibleRelation __RPC_FAR *__RPC_FAR *relation)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(relation == nullptr)
        {
            return E_INVALIDARG;
        }

        if (!m_xContext.is())
            return E_FAIL;


        long nMax = 0;
        get_nRelations(&nMax);

        *relation = static_cast<IAccessibleRelation*>(::CoTaskMemAlloc(sizeof(IAccessibleRelation)));

        // #CHECK Memory Allocation#
        if(*relation == nullptr)
        {
            return E_FAIL;
        }

        if( relationIndex < nMax )
        {
            Reference<XAccessibleRelationSet> const pRrelationSet =
                m_xContext->getAccessibleRelationSet();
            if(!pRrelationSet.is())
            {

                return E_FAIL;
            }

            IAccessibleRelation* pRelation = nullptr;
            HRESULT hr = createInstance<CAccRelation>(IID_IAccessibleRelation,
                            &pRelation);
            if(SUCCEEDED(hr))
            {
                IUNOXWrapper* wrapper = nullptr;
                hr = pRelation->QueryInterface(IID_IUNOXWrapper, reinterpret_cast<void**>(&wrapper));
                if(SUCCEEDED(hr))
                {
                    AccessibleRelation accRelation = pRrelationSet->getRelation(relationIndex);
                    wrapper->put_XSubInterface(
                            reinterpret_cast<hyper>(&accRelation));
                    wrapper->Release();
                    *relation = pRelation;
                    return S_OK;
                }

            }
        }

        return E_FAIL;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_relations( long, IAccessibleRelation __RPC_FAR *__RPC_FAR *relation, long __RPC_FAR *nRelations)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(relation == nullptr || nRelations == nullptr)
        {
            return E_INVALIDARG;
        }

        if (!m_xContext.is())
            return E_FAIL;

        Reference<XAccessibleRelationSet> const pRrelationSet =
            m_xContext->getAccessibleRelationSet();
        if(!pRrelationSet.is())
        {
            *nRelations = 0;
            return S_OK;
        }

        long nCount = pRrelationSet->getRelationCount();

        *relation = static_cast<IAccessibleRelation*>(::CoTaskMemAlloc(nCount*sizeof(IAccessibleRelation)));

        // #CHECK Memory Allocation#
        if(*relation == nullptr)
        {
            return E_FAIL;
        }

        for(int i=0; i<nCount ; i++)
        {
            IAccessibleRelation* pRelation = nullptr;
            HRESULT hr = createInstance<CAccRelation>(IID_IAccessibleRelation,
                            &pRelation);
            if(SUCCEEDED(hr))
            {
                IUNOXWrapper* wrapper = nullptr;
                hr = pRelation->QueryInterface(IID_IUNOXWrapper, reinterpret_cast<void**>(&wrapper));
                if(SUCCEEDED(hr))
                {
                    AccessibleRelation accRelation = pRrelationSet->getRelation(i);
                    wrapper->put_XSubInterface(
                            reinterpret_cast<hyper>(&accRelation));
                    wrapper->Release();
                }
                relation[i] = pRelation;
            }
        }

        *nRelations = nCount;
        return S_OK;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::role(long __RPC_FAR *role)
{
    SolarMutexGuard g;

    try {

        (*role) = m_iRole;

        return S_OK;

    } catch(...) { return E_FAIL; }
}


COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_nActions(long __RPC_FAR *nActions)
{
    SolarMutexGuard g;

    try
    {
        if (m_isDestroy) return S_FALSE;

        if(nActions == nullptr)
        {
            return E_INVALIDARG;
        }
        *nActions = 0;
        IAccessibleAction* pAcc = nullptr;
        HRESULT hr = QueryInterface(IID_IAccessibleAction, reinterpret_cast<void**>(&pAcc));
        if( hr == S_OK )
        {
            pAcc->nActions(nActions);
            pAcc->Release();
        }

        return S_OK;
    }
    catch(...)
    {
        *nActions = 0;
        return S_OK;
    }
}


COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::scrollToPoint(enum IA2CoordinateType, long, long)
{
    return E_NOTIMPL;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::scrollTo(enum IA2ScrollType)
{
    return E_NOTIMPL;
}

static XAccessible* getTheParentOfMember(XAccessible* pXAcc)
{
    if(pXAcc == nullptr)
    {
        return nullptr;
    }
    Reference<XAccessibleContext> pRContext = pXAcc->getAccessibleContext();
    Reference<XAccessibleRelationSet> pRrelationSet = pRContext->getAccessibleRelationSet();
    sal_Int32 nRelations = pRrelationSet->getRelationCount();
    for(sal_Int32 i=0 ; i<nRelations ; i++)
    {
        AccessibleRelation accRelation = pRrelationSet->getRelation(i);
        if (accRelation.RelationType == AccessibleRelationType::MEMBER_OF)
        {
            Sequence< Reference< XInterface > > xTargets = accRelation.TargetSet;
            return static_cast<XAccessible*>(xTargets[0].get());
        }
    }
    return nullptr;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_groupPosition(long __RPC_FAR *groupLevel,long __RPC_FAR *similarItemsInGroup,long __RPC_FAR *positionInGroup)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(groupLevel == nullptr || similarItemsInGroup == nullptr || positionInGroup == nullptr)
        {
            return E_INVALIDARG;
        }

        if (!m_xAccessible.is())
            return E_FAIL;

        Reference<XAccessibleContext> const pRContext =
            m_xAccessible->getAccessibleContext();
        if(!pRContext.is())
            return E_FAIL;
        const sal_Int16 nRole = pRContext->getAccessibleRole();

        *groupLevel = 0;
        *similarItemsInGroup = 0;
        *positionInGroup = 0;

        if (nRole != AccessibleRole::DOCUMENT && nRole != AccessibleRole::DOCUMENT_PRESENTATION &&
                nRole != AccessibleRole::DOCUMENT_SPREADSHEET && nRole != AccessibleRole::DOCUMENT_TEXT)
        {
            Reference< XAccessibleGroupPosition > xGroupPosition( pRContext, UNO_QUERY );
            if ( xGroupPosition.is() )
            {
                Sequence< sal_Int32 > rSeq = xGroupPosition->getGroupPosition( Any( pRContext ) );
                if (rSeq.getLength() >= 3)
                {
                    *groupLevel = rSeq[0];
                    *similarItemsInGroup = rSeq[1];
                    *positionInGroup = rSeq[2];
                    return S_OK;
                }
                return S_OK;
            }
        }

        Reference< XAccessible> pParentAcc = pRContext->getAccessibleParent();
        if( !pParentAcc.is() )
        {
            return S_OK;
        }

        Reference<XAccessibleContext> pRParentContext = pParentAcc->getAccessibleContext();

        if (nRole == AccessibleRole::RADIO_BUTTON)
        {
            int index = 0;
            int number = 0;
            Reference<XAccessibleRelationSet> pRrelationSet = pRContext->getAccessibleRelationSet();
            long nRel = pRrelationSet->getRelationCount();
            for(int i=0 ; i<nRel ; i++)
            {
                AccessibleRelation accRelation = pRrelationSet->getRelation(i);
                if (accRelation.RelationType == AccessibleRelationType::MEMBER_OF)
                {
                    Sequence< Reference< XInterface > > xTargets = accRelation.TargetSet;

                    Reference<XInterface> pRAcc = xTargets[0];
                    sal_Int64 nChildCount = pRParentContext->getAccessibleChildCount();
                    assert(nChildCount < std::numeric_limits<long>::max());
                    for (sal_Int64 j = 0; j< nChildCount; j++)
                    {
                        if( getTheParentOfMember(pRParentContext->getAccessibleChild(j).get())
                            == static_cast<XAccessible*>(pRAcc.get()) &&
                            pRParentContext->getAccessibleChild(j)->getAccessibleContext()->getAccessibleRole() == AccessibleRole::RADIO_BUTTON)
                            number++;
                        if (pRParentContext->getAccessibleChild(j).get() == m_xAccessible.get())
                            index = number;
                    }
                }
            }
            *groupLevel = 1;
            *similarItemsInGroup = number;
            *positionInGroup = index;
            return S_OK;
        }

        else if (nRole == AccessibleRole::COMBO_BOX)
        {
            *groupLevel = 1;
            *similarItemsInGroup = 0;
            *positionInGroup = -1;

            if (pRContext->getAccessibleChildCount() != 2)
            {
                return S_OK;
            }
            Reference<XAccessible> xList=pRContext->getAccessibleChild(1);
            if (!xList.is())
            {
                return S_OK;
            }
            Reference<XAccessibleContext> xListContext(xList,UNO_QUERY);
            if (!xListContext.is())
            {
                return S_OK;
            }
            Reference<XAccessibleSelection> xListSel(xList,UNO_QUERY);
            if (!xListSel.is())
            {
                return S_OK;
            }
            sal_Int64 nChildCount = xListContext->getAccessibleChildCount();
            assert(nChildCount < std::numeric_limits<long>::max());
            *similarItemsInGroup = nChildCount;
            if (*similarItemsInGroup > 0 )
            {
                try
                {
                    Reference<XAccessible> xChild = xListSel->getSelectedAccessibleChild(0);
                    if (xChild.is())
                    {
                        Reference<XAccessibleContext> xChildContext(xChild,UNO_QUERY);
                        if (xChildContext.is())
                        {
                            *positionInGroup=xChildContext->getAccessibleIndexInParent() + 1 ;
                            return S_OK;
                        }
                    }
                }
                catch(...)
                {}
            }
            return S_OK;
        }
        else if (nRole == AccessibleRole::PAGE_TAB)
        {
            *groupLevel = 1;
            sal_Int64 nChildCount = pRParentContext->getAccessibleChildCount();
            assert(nChildCount < std::numeric_limits<long>::max());
            *similarItemsInGroup = nChildCount;
            if (*similarItemsInGroup > 0 )
            {
                *positionInGroup=pRContext->getAccessibleIndexInParent() + 1 ;
            }
            else
            {
                *positionInGroup = -1;
            }
            return S_OK;
        }

        int level = 0;
        bool isFound = false;
        while( pParentAcc.is() && !isFound)
        {
            level++;
            pRParentContext = pParentAcc->getAccessibleContext();
            const sal_Int16 nParentRole = pRParentContext->getAccessibleRole();
            if ((nParentRole == AccessibleRole::TREE) || (nParentRole == AccessibleRole::LIST))
                isFound = true;
            pParentAcc = pRParentContext->getAccessibleParent();
        }

        if( isFound )
        {
            Reference< XAccessible> pTempAcc = pRContext->getAccessibleParent();
            pRParentContext = pTempAcc->getAccessibleContext();
            *groupLevel = level;
            sal_Int64 nChildCount = pRParentContext->getAccessibleChildCount();
            assert(nChildCount < std::numeric_limits<long>::max());
            *similarItemsInGroup = nChildCount;
            *positionInGroup = pRContext->getAccessibleIndexInParent() + 1;
        }
        else
        {
            *groupLevel = 0;
            *similarItemsInGroup = 0;
            *positionInGroup = 0;
        }
        return S_OK;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_extendedStates(long, BSTR __RPC_FAR *__RPC_FAR *, long __RPC_FAR *)
{
    return E_NOTIMPL;
}


COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_uniqueID(long __RPC_FAR *uniqueID)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(uniqueID == nullptr)
        {
            return E_INVALIDARG;
        }
        *uniqueID = m_dChildID;
        return S_OK;

        } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_windowHandle(HWND __RPC_FAR *windowHandle)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(windowHandle == nullptr)
        {
            return E_INVALIDARG;
        }

        HWND nHwnd = m_hwnd;
        IAccessible* pParent = m_pIParent;
        while((nHwnd==nullptr) && pParent)
        {
            if (CMAccessible* pChild = dynamic_cast<CMAccessible*>(pParent))
            {
                pParent = pChild->m_pIParent;
                nHwnd = pChild->m_hwnd;
            }
            else
                pParent = nullptr;
        }

        *windowHandle = nHwnd;
        return S_OK;

        } catch(...) { return E_FAIL; }
}

/**
* Get XAccessibleContext directly from UNO by the stored XAccessible pointer
* @param    pXAcc, UNO XAccessible object point.
* @return   XAccessibleContext*, the context of the pXAcc.
*/
XAccessibleContext* CMAccessible::GetContextByXAcc( XAccessible* pXAcc )
{
    Reference< XAccessibleContext > pRContext;
    if( pXAcc == nullptr)
        return nullptr;

    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return nullptr;
    return pRContext.get();
}

/**
* When COM is created, UNO set XAccessible pointer to it
* in order to COM can operate UNO information
* @param    pXAcc, the XAccessible object of current object.
* @return  S_OK if successful.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::SetXAccessible(hyper pXAcc)
{
    // internal IMAccessible - no mutex meeded

    m_xAccessible = reinterpret_cast<XAccessible*>(pXAcc);
    m_pEnumVar->PutSelection(/*XAccessibleSelection*/
            reinterpret_cast<hyper>(m_xAccessible.get()));

    m_xContext = m_xAccessible->getAccessibleContext();

    return S_OK;
}

/**
* accSelect method has many optional flags, needs to process comprehensively
* Mozilla and Microsoft do not implement SELFLAG_EXTENDSELECTION flag.
* The implementation of this flag is a little trouble-shooting,so we also
* do not implement it now
* @param    flagsSelect, the selection flag of the select action.
* @param    varChild, the child object pointer of current action.
* @return  S_OK if successful.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::accSelect(long flagsSelect, VARIANT varChild)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;
        if( (flagsSelect&SELFLAG_ADDSELECTION) &&
            (SELFLAG_REMOVESELECTION&flagsSelect) )
            return E_INVALIDARG;

        if ( (flagsSelect&SELFLAG_TAKESELECTION) &&
             (
                 (flagsSelect&SELFLAG_ADDSELECTION) ||
                 (flagsSelect&SELFLAG_REMOVESELECTION) ||
                 (flagsSelect&SELFLAG_EXTENDSELECTION )
             )
            )
            return E_INVALIDARG;

        if ( varChild.vt != VT_I4 )
            return E_INVALIDARG;

        IMAccessible* pSelectAcc;
        if( varChild.lVal == CHILDID_SELF )
        {
            pSelectAcc = this;
            pSelectAcc->AddRef();
        }
        else
        {
            pSelectAcc = GetChildInterface(varChild.lVal);
        }

        if( pSelectAcc == nullptr )
            return E_INVALIDARG;

        if( flagsSelect&SELFLAG_TAKEFOCUS )
        {
            union {
                XAccessible* pTempUNO;
                hyper nHyper = 0;
            };
            pSelectAcc->GetUNOInterface(&nHyper);

            if( pTempUNO == nullptr )
                return 0;

            Reference<XAccessibleContext> pRContext = pTempUNO->getAccessibleContext();
            Reference< XAccessibleComponent > pRComponent(pRContext,UNO_QUERY);
            Reference< XAccessible > pRParentXAcc = pRContext->getAccessibleParent();
            Reference< XAccessibleContext > pRParentContext = pRParentXAcc->getAccessibleContext();
            Reference< XAccessibleComponent > pRParentComponent(pRParentContext,UNO_QUERY);
            Reference< XAccessibleSelection > pRParentSelection(pRParentContext,UNO_QUERY);


            pRComponent->grabFocus();

            if( flagsSelect & SELFLAG_TAKESELECTION )
            {
                pRParentSelection->clearAccessibleSelection();
                pRParentSelection->selectAccessibleChild( pRContext->getAccessibleIndexInParent() );
            }

            if( flagsSelect & SELFLAG_ADDSELECTION  )
            {
                pRParentSelection->selectAccessibleChild( pRContext->getAccessibleIndexInParent() );
            }

            if( flagsSelect & SELFLAG_REMOVESELECTION )
            {
                pRParentSelection->deselectAccessibleChild( pRContext->getAccessibleIndexInParent() );
            }

            if( flagsSelect & SELFLAG_EXTENDSELECTION  )
            {
                sal_Int64 indexInParrent = pRContext->getAccessibleIndexInParent();

                if( pRParentSelection->isAccessibleChildSelected( indexInParrent + 1 ) ||
                    pRParentSelection->isAccessibleChildSelected( indexInParrent - 1 ) )
                {
                    pRParentSelection->selectAccessibleChild( indexInParrent );
                }
            }

        }

        pSelectAcc->Release();
        return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
* Return XAccessible interface pointer when needed
* @param pXAcc, [in, out] the Uno interface of the current object.
* @return S_OK if successful.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::GetUNOInterface(hyper * pXAcc)
{
    // internal IMAccessible - no mutex meeded

    if(pXAcc == nullptr)
        return E_INVALIDARG;

    *pXAcc = reinterpret_cast<hyper>(m_xAccessible.get());
    return S_OK;
}

/**
* Helper method for Implementation of get_accDefaultAction
* @param pAction, the default action point of the current object.
* @return S_OK if successful.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::SetDefaultAction(hyper pAction)
{
    // internal IMAccessible - no mutex meeded

    m_xAction = reinterpret_cast<XAccessibleAction*>(pAction);
    return S_OK;
}

/**
* This method is called when AT open some UI elements initially
* the UI element takes the default action defined here
* @param varChild, the child id of the defaultaction.
* @param pszDefaultAction,[in/out] the description of the current action.
* @return S_OK if successful.
*/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE CMAccessible::get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if(pszDefaultAction == nullptr)
        {
            return E_INVALIDARG;
        }
        if(varChild.vt==VT_I4)
        {
            if(varChild.lVal==CHILDID_SELF)
            {
                if (!m_xAction.is())
                    return DISP_E_MEMBERNOTFOUND;
                SysFreeString(*pszDefaultAction);
                *pszDefaultAction = SysAllocString(m_pszActionDescription);
                return S_OK;
            }

            long lVal = varChild.lVal;
            varChild.lVal = CHILDID_SELF;
            IMAccessible *pChild = this->GetChildInterface(lVal);
            if(!pChild)
                return E_FAIL;
            return pChild->get_accDefaultAction(varChild,pszDefaultAction);
        }
        return S_FALSE;

        } catch(...) { return E_FAIL; }
}

/**
* AT call this method to operate application
* @param varChild, the child id of the action object.
* @return S_OK if successful.
*/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE CMAccessible::accDoDefaultAction(VARIANT varChild)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;
        if( varChild.vt != VT_I4 )
            return E_INVALIDARG;
        if (!m_xAction.is())
            return E_FAIL;
        if (m_xAction->getAccessibleActionCount() == 0)
            return E_FAIL;

        if(varChild.lVal==CHILDID_SELF)
        {
            if (m_xAction->getAccessibleActionCount() > 0)
                m_xAction->doAccessibleAction(0);
            return S_OK;
        }

        long lVal = varChild.lVal;
        varChild.lVal = CHILDID_SELF;
        IMAccessible *pChild = this->GetChildInterface(lVal);
        if(!pChild)
            return E_FAIL;
        return pChild->accDoDefaultAction( varChild );

    } catch(...) { return E_FAIL; }
}

/**
* UNO set description information for action to COM.
* @param szAction, the action description of the current object.
* @return S_OK if successful.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::Put_ActionDescription( const OLECHAR* szAction)
{
    // internal IMAccessible - no mutex meeded

    try {
        if (m_isDestroy) return S_FALSE;

        if(szAction == nullptr)
        {
            return E_INVALIDARG;
        }
        SysFreeString(m_pszActionDescription );
        m_pszActionDescription = SysAllocString( szAction );
        return S_OK;

        } catch(...) { return E_FAIL; }
}

bool CMAccessible::GetXInterfaceFromXAccessible(XAccessible* pXAcc, XInterface** ppXI, XInterfaceType eType)
{
    switch(eType)
    {
    case XInterfaceType::XI_COMPONENT:
        return queryXInterface<XAccessibleComponent>(pXAcc, ppXI);
    case XInterfaceType::XI_TEXT:
        return queryXInterface<XAccessibleText>(pXAcc, ppXI);
    case XInterfaceType::XI_EDITABLETEXT:
        return queryXInterface<XAccessibleEditableText>(pXAcc, ppXI);
    case XInterfaceType::XI_TABLE:
        return queryXInterface<XAccessibleTable>(pXAcc, ppXI);
    case XInterfaceType::XI_TABLECELL:
        // needs specific handling, since there's no XInterface for table cells
        return queryTableCell(pXAcc, ppXI);
    case XInterfaceType::XI_SELECTION:
        return queryXInterface<XAccessibleSelection>(pXAcc, ppXI);
    case XInterfaceType::XI_EXTENDEDCOMP:
        return queryXInterface<XAccessibleExtendedComponent>(pXAcc, ppXI);
    case XInterfaceType::XI_KEYBINDING:
        return queryXInterface<XAccessibleKeyBinding>(pXAcc, ppXI);
    case XInterfaceType::XI_ACTION:
        return queryXInterface<XAccessibleAction>(pXAcc, ppXI);
    case XInterfaceType::XI_VALUE:
        return queryXInterface<XAccessibleValue>(pXAcc, ppXI);
    case XInterfaceType::XI_HYPERTEXT:
        return queryXInterface<XAccessibleHypertext>(pXAcc, ppXI);
    case XInterfaceType::XI_HYPERLINK:
        return queryXInterface<XAccessibleHyperlink>(pXAcc, ppXI);
    case XInterfaceType::XI_IMAGE:
        return queryXInterface<XAccessibleImage>(pXAcc, ppXI);
    default:
        return false;
    }
}

template<typename T> static HRESULT
createAggInstance(CMAccessible &rOuter, void ** ppvObject)
{
    // Note: CComAggObject has special handling for IUnknown - must
    // query for that when creating it! Otherwise we get a T member of it
    // which will redirect QueryInterface back to CMAccessible infinitely.
    // (CComAggObject has its own ref-count too which is not a problem
    //  since it is inserted in m_containedObjects.)
    return CComCreator< CComAggObject<T> >::CreateInstance(
            rOuter.GetControllingUnknown(), IID_IUnknown, ppvObject);
}

typedef HRESULT (AggCreatorFunc)(CMAccessible &, void **);

namespace {

struct AggMapEntry
{
    const IID* piid;
    AggCreatorFunc* pfnCreateInstance;
    const XInterfaceType eXInterfaceType;
};

}

static AggMapEntry g_CMAccessible_AggMap[] = {
    { &IID_IAccessibleComponent, &createAggInstance<CAccComponent>, XInterfaceType::XI_COMPONENT },
    { &IID_IAccessibleText, &createAggInstance<CAccText>, XInterfaceType::XI_TEXT },
    { &IID_IAccessibleEditableText, &createAggInstance<CAccEditableText>, XInterfaceType::XI_EDITABLETEXT },
    { &IID_IAccessibleImage, &createAggInstance<CAccImage>, XInterfaceType::XI_IMAGE },
    { &IID_IAccessibleTable, &createAggInstance<CAccTable>, XInterfaceType::XI_TABLE },
    { &IID_IAccessibleTable2, &createAggInstance<CAccTable>, XInterfaceType::XI_TABLE },
    { &IID_IAccessibleTableCell, &createAggInstance<CAccTableCell>, XInterfaceType::XI_TABLECELL },
    { &IID_IAccessibleAction, &createAggInstance<CAccAction>, XInterfaceType::XI_ACTION },
    { &IID_IAccessibleValue, &createAggInstance<CAccValue>, XInterfaceType::XI_VALUE },
    { &IID_IAccessibleHypertext, &createAggInstance<CAccHypertext>, XInterfaceType::XI_HYPERTEXT },
    { &IID_IAccessibleHyperlink, &createAggInstance<CAccHyperLink>, XInterfaceType::XI_HYPERLINK }
};


HRESULT WINAPI CMAccessible::SmartQI(void* /*pv*/, REFIID iid, void** ppvObject)
{
    try {

    if (m_isDestroy) return S_FALSE;
    if (InlineIsEqualGUID(iid,IID_IAccIdentity) ||
        InlineIsEqualGUID(iid,IID_IStdMarshalInfo) ||
        InlineIsEqualGUID(iid,IID_IMarshal) ||
        InlineIsEqualGUID(iid,IID_IExternalConnection)||
        InlineIsEqualGUID(iid,IID_IOleWindow))
    {
        return E_FAIL;
    }

    for (const AggMapEntry& rEntry : g_CMAccessible_AggMap)
    {
        if (InlineIsEqualGUID(iid, *rEntry.piid))
        {
            SolarMutexGuard g;

            XInterface* pXI = nullptr;
            bool bFound = GetXInterfaceFromXAccessible(m_xAccessible.get(),
                                &pXI, rEntry.eXInterfaceType);
            if(!bFound)
            {
                return E_FAIL;
            }

            XGUIDToComObjHash::iterator pIndTemp = m_containedObjects.find( iid );
            if ( pIndTemp != m_containedObjects.end() )
            {
                return pIndTemp->second.p->QueryInterface( iid, ppvObject );
            }
            else
            {
                HRESULT hr = rEntry.pfnCreateInstance(*this, ppvObject);
                assert(hr == S_OK);
                if(hr == S_OK)
                {
                    m_containedObjects.emplace(*rEntry.piid, static_cast<IUnknown*>(*ppvObject));
                    IUNOXWrapper* wrapper = nullptr;
                    static_cast<IUnknown*>(*ppvObject)->QueryInterface(IID_IUNOXWrapper, reinterpret_cast<void**>(&wrapper));
                    if(wrapper)
                    {
                        wrapper->put_XInterface(
                                reinterpret_cast<hyper>(m_xAccessible.get()));
                        wrapper->Release();
                    }
                    return S_OK;
                }
            }
            return E_FAIL;
        }
    }
    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

IAccessible* CMAccessible::get_IAccessibleFromXAccessible(XAccessible* pXAcc)
{
    try
    {
        if (g_pAccObjectManager)
            return g_pAccObjectManager->GetIAccessibleFromXAccessible(pXAcc);
    }
    catch(...)
    {
    }
    return nullptr;
}

void CMAccessible::ConvertAnyToVariant(const css::uno::Any &rAnyVal, VARIANT *pvData)
{
    if(rAnyVal.hasValue())
    {
        // Clear VARIANT variable.
        VariantClear(pvData);

        // Set value according to value type.
        switch(rAnyVal.getValueTypeClass())
        {
        case TypeClass_CHAR:
            pvData->vt = VT_UI1;
            memcpy(&pvData->bVal, rAnyVal.getValue(), sizeof(char));
            break;

        case TypeClass_BOOLEAN:
        {
            bool bBoolean(false);
            rAnyVal >>= bBoolean;
            pvData->vt = VT_BOOL;
            pvData->boolVal = VARIANT_BOOL(bBoolean); // boolVal is a VARIANT_BOOL, a 16bit field
            break;
        }
        case TypeClass_BYTE:
            pvData->vt = VT_UI1;
            memcpy(&pvData->bVal, rAnyVal.getValue(), sizeof(sal_Int8));
            break;

        case TypeClass_SHORT:
            pvData->vt = VT_I2;
            memcpy(&pvData->iVal, rAnyVal.getValue(), sizeof(sal_Int16));
            break;

        case TypeClass_UNSIGNED_SHORT:
            pvData->vt = VT_I2;
            memcpy(&pvData->iVal, rAnyVal.getValue(), sizeof(sal_uInt16));
            break;

        case TypeClass_LONG:
            pvData->vt = VT_I4;
            memcpy(&pvData->lVal, rAnyVal.getValue(), sizeof(sal_Int32));
            break;

        case TypeClass_UNSIGNED_LONG:
            pvData->vt = VT_I4;
            memcpy(&pvData->lVal, rAnyVal.getValue(), sizeof(sal_uInt32));
            break;

        case TypeClass_FLOAT:
            pvData->vt = VT_R4;
            memcpy(&pvData->fltVal, rAnyVal.getValue(), sizeof(float));
            break;

        case TypeClass_DOUBLE:
            pvData->vt = VT_R8;
            memcpy(&pvData->dblVal, rAnyVal.getValue(), sizeof(double));
            break;

        case TypeClass_STRING:
            {
                pvData->vt = VT_BSTR;
                OUString val;
                rAnyVal >>= val;
                pvData->bstrVal = SysAllocString(o3tl::toW(val.getStr()));
                break;
            }

        case TypeClass_VOID:
        case TypeClass_HYPER:
        case TypeClass_UNSIGNED_HYPER:
        case TypeClass_TYPE:
        case TypeClass_ANY:
        case TypeClass_ENUM:
        case TypeClass_TYPEDEF:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        case TypeClass_SEQUENCE:
        case TypeClass_INTERFACE:
            {
                Reference< XAccessible > pXAcc;
                if(rAnyVal >>= pXAcc)
                {
                    if(pXAcc.is())
                    {
                        IAccessible* pIAcc = get_IAccessibleFromXAccessible(pXAcc.get());
                        if(pIAcc == nullptr)
                        {
                            Reference< XAccessibleContext > pXAccContext = pXAcc->getAccessibleContext();
                            g_pAccObjectManager->InsertAccObj(pXAcc.get(),pXAccContext->getAccessibleParent().get());
                            pIAcc = get_IAccessibleFromXAccessible(pXAcc.get());
                        }
                        if(pIAcc)
                        {
                            pIAcc->AddRef();

                            pvData->vt = VT_UNKNOWN;
                            pvData->pdispVal = pIAcc;
                            break;
                        }
                    }
                }
                [[fallthrough]];
            }
        case TypeClass_SERVICE:
        case TypeClass_MODULE:
        case TypeClass_INTERFACE_METHOD:
        case TypeClass_INTERFACE_ATTRIBUTE:
        case TypeClass_UNKNOWN:
        case TypeClass_PROPERTY:
        case TypeClass_CONSTANT:
        case TypeClass_CONSTANTS:
        case TypeClass_SINGLETON:
        case TypeClass::TypeClass_MAKE_FIXED_SIZE:
            // Output the type string, if there is other uno value type.
            pvData->vt = VT_BSTR;
            pvData->bstrVal = SysAllocString(o3tl::toW(rAnyVal.getValueTypeName().getStr()));
            break;

        default:
            break;
        }
    }
    else
    {
        VariantClear(pvData);
    }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_states(AccessibleStates __RPC_FAR *states)
{
    SolarMutexGuard g;

    try {
    if (m_isDestroy) return S_FALSE;

    if (!m_xContext.is())
        return E_FAIL;

    sal_Int64 const nRStateSet =
        m_xContext->getAccessibleStateSet();

    *states = 0x0;
    for (int i = 0; i < 63; ++i)
    {
        sal_Int64 nUnoState = sal_Int64(1) << i;
        if (nRStateSet & nUnoState)
            lcl_addIA2State(*states, nUnoState, m_xContext->getAccessibleRole());
    }

    return S_OK;


    } catch(...) { return E_FAIL; }
}

// return the UNO roles
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_extendedRole(BSTR __RPC_FAR *)
{
    return E_NOTIMPL;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_localizedExtendedRole(BSTR __RPC_FAR *)
{
    return E_NOTIMPL;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_nExtendedStates(long __RPC_FAR *)
{
    return E_NOTIMPL;
}


COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_localizedExtendedStates(long, BSTR __RPC_FAR *__RPC_FAR *, long __RPC_FAR *)
{
    return E_NOTIMPL;
}


COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_indexInParent(long __RPC_FAR *accParentIndex)
{
    try {
        if (m_isDestroy) return S_FALSE;

        if(accParentIndex == nullptr)
            return E_INVALIDARG;

        if (!m_xContext.is())
            return E_FAIL;

        sal_Int64 nIndex = m_xContext->getAccessibleIndexInParent();
        if (nIndex > std::numeric_limits<long>::max())
        {
            SAL_WARN("iacc2", "CMAccessible::get_indexInParent: Child index exceeds maximum long value, "
                              "returning max long.");
            nIndex = std::numeric_limits<long>::max();
        }
        *accParentIndex = nIndex;
        return S_OK;


    } catch(...) { return E_FAIL; }
}
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_locale( IA2Locale __RPC_FAR *locale  )
{
    try {
        if (m_isDestroy) return S_FALSE;
        if(locale == nullptr)
            return E_INVALIDARG;

        if (!m_xContext.is())
            return E_FAIL;

        css::lang::Locale unoLoc = m_xContext->getLocale();
        locale->language = SysAllocString(o3tl::toW(unoLoc.Language.getStr()));
        locale->country = SysAllocString(o3tl::toW(unoLoc.Country.getStr()));
        locale->variant = SysAllocString(o3tl::toW(unoLoc.Variant.getStr()));

        return S_OK;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_appName(BSTR __RPC_FAR *name)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;
        if(name == nullptr)
            return E_INVALIDARG;

        static const OUString sAppName = utl::ConfigManager::getProductName();
        *name = SysAllocString(o3tl::toW(sAppName.getStr()));
        return S_OK;
    } catch(...) { return E_FAIL; }
}
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_appVersion(BSTR __RPC_FAR *version)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;
        if(version == nullptr)
            return E_INVALIDARG;
        static const OUString sVersion = utl::ConfigManager::getProductVersion();
        *version=SysAllocString(o3tl::toW(sVersion.getStr()));
        return S_OK;
    } catch(...) { return E_FAIL; }
}
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_toolkitName(BSTR __RPC_FAR *name)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;
        if(name == nullptr)
            return E_INVALIDARG;
        *name = SysAllocString(OLESTR("VCL"));
        return S_OK;
    } catch(...) { return E_FAIL; }
}
COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_toolkitVersion(BSTR __RPC_FAR *version)
{
    return get_appVersion(version);
}


COM_DECLSPEC_NOTHROW STDMETHODIMP CMAccessible::get_attributes(/*[out]*/ BSTR *pAttr)
{
    SolarMutexGuard g;

    try {
        if (m_isDestroy) return S_FALSE;

        if (!m_xAccessible.is())
            return E_FAIL;

        Reference<XAccessibleContext> pRContext = m_xAccessible->getAccessibleContext();
        if( !pRContext.is() )
        {
            return E_FAIL;
        }

        OUString sAttributes;
        Reference<XAccessibleExtendedAttributes> pRXI(pRContext,UNO_QUERY);
        if (pRXI.is())
        {
            css::uno::Reference<css::accessibility::XAccessibleExtendedAttributes> pRXAttr;
            pRXAttr = pRXI.get();
            css::uno::Any  anyVal = pRXAttr->getExtendedAttributes();

            OUString val;
            anyVal >>= val;
            sAttributes += val;
        }

        // some text-specific IAccessible2 object attributes (like text alignment
        // of a paragraph) are handled as text attributes in LibreOffice
        Reference<XAccessibleText> xText(pRContext, UNO_QUERY);
        if (xText.is())
        {
            sal_Int32 nStartOffset = 0;
            sal_Int32 nEndOffset = 0;
            sAttributes += AccessibleTextAttributeHelper::GetIAccessible2TextAttributes(
                xText, IA2AttributeType::ObjectAttributes, 0, nStartOffset, nEndOffset);
        }

        if (*pAttr)
            SysFreeString(*pAttr);
        *pAttr = SysAllocString(o3tl::toW(sAttributes.getStr()));

        return S_OK;
    } catch(...) { return E_FAIL; }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
