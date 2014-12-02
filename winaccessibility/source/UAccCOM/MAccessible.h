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

#ifndef INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_MACCESSIBLE_H
#define INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_MACCESSIBLE_H

#include "resource.h"       // main symbols
#include <map>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <AccObjectManagerAgent.hxx>
#include "EnumVariant.h"
#include "acccommon.h"
#include <rtl/ustring.hxx>

#define     OBJID_WINDOW        ((LONG)0x00000000)
#define     OBJID_SYSMENU       ((LONG)0xFFFFFFFF)
#define     OBJID_TITLEBAR      ((LONG)0xFFFFFFFE)
#define     OBJID_MENU          ((LONG)0xFFFFFFFD)
#define     OBJID_CLIENT        ((LONG)0xFFFFFFFC)
#define     OBJID_VSCROLL       ((LONG)0xFFFFFFFB)
#define     OBJID_HSCROLL       ((LONG)0xFFFFFFFA)
#define     OBJID_SIZEGRIP      ((LONG)0xFFFFFFF9)
#define     OBJID_CARET         ((LONG)0xFFFFFFF8)
#define     OBJID_CURSOR        ((LONG)0xFFFFFFF7)
#define     OBJID_ALERT         ((LONG)0xFFFFFFF6)
#define     OBJID_SOUND         ((LONG)0xFFFFFFF5)
#define     OBJID_QUERYCLASSNAMEIDX ((LONG)0xFFFFFFF4)
#define     OBJID_NATIVEOM      ((LONG)0xFFFFFFF0)

/**
 *This class implements IMAccessible interface, which inherits from IAccessible2, and
 *in turn inherits from IAccessible. So its methods include the methods defined only in
 *IAccessible, plus the methods defined only in IAccessible2, plus the methods defined
 *only in IMAccessible.
 */
class ATL_NO_VTABLE CMAccessible :
            public CComObjectRoot,
            public CComCoClass<CMAccessible, &CLSID_MAccessible>,
            public IDispatchImpl<IMAccessible, &IID_IMAccessible, &LIBID_UACCCOMLib>,
            public IServiceProvider,
            public IAccessibleApplication
{
    typedef ::std::map<const GUID, CComPtr<IUnknown>, ltComp> XGUIDToComObjHash;

public:
    CMAccessible();
    virtual ~CMAccessible();

    DECLARE_NO_REGISTRY()

    DECLARE_GET_CONTROLLING_UNKNOWN()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CMAccessible)
    COM_INTERFACE_ENTRY(IMAccessible)
    COM_INTERFACE_ENTRY(IAccessible)
    COM_INTERFACE_ENTRY(IAccessible2)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IAccessibleApplication)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    // IMAccessible
    STDMETHOD(put_accValue)(VARIANT varChild,BSTR szValue);
    STDMETHOD(put_accName)(VARIANT varChild,BSTR szName);
    STDMETHOD(accDoDefaultAction)(VARIANT varChild);
    STDMETHOD(accHitTest)(long xLeft,long yTop,VARIANT *pvarChild);
    STDMETHOD(accNavigate)(long navDir,VARIANT varStart,VARIANT *pvarEndUpAt);
    STDMETHOD(accLocation)(long *pxLeft,long *pyTop,long *pcxWidth,long *pcyHeight,VARIANT varChild);
    STDMETHOD(accSelect)(long flagsSelect,VARIANT varChild);
    STDMETHOD(get_accDefaultAction)( VARIANT varChild,BSTR *pszDefaultAction);
    STDMETHOD(get_accSelection)(VARIANT *pvarChildren);
    STDMETHOD(get_accFocus)(VARIANT *pvarChild);
    STDMETHOD(get_accKeyboardShortcut)( VARIANT varChild,BSTR *pszKeyboardShortcut);
    STDMETHOD(get_accHelpTopic)(BSTR *pszHelpFile,VARIANT varChild,long *pidTopic);
    STDMETHOD(get_accHelp)(VARIANT varChild,BSTR *pszHelp);
    STDMETHOD(get_accState)(VARIANT varChild,VARIANT *pvarState);
    STDMETHOD(get_accRole)(VARIANT varChild,VARIANT *pvarRole);
    STDMETHOD(get_accDescription)(VARIANT varChild,BSTR *pszDescription);
    STDMETHOD(get_accValue)( VARIANT varChild,BSTR *pszValue);
    STDMETHOD(get_accName)(VARIANT varChild,BSTR *pszName);
    STDMETHOD(get_accChild)(VARIANT varChild,IDispatch **ppdispChild);
    STDMETHOD(get_accChildCount)(long *pcountChildren);
    STDMETHOD(get_accParent)( IDispatch **ppdispParent);

    // methods which are defined only in the IAccessible2
    // These methods only declare here, and their implementation bodies are empty now.
    STDMETHOD(get_nRelations)( long __RPC_FAR *nRelations) ;
    STDMETHOD(get_relation)( long relationIndex, IAccessibleRelation __RPC_FAR *__RPC_FAR *relation) ;
    STDMETHOD(get_relations)( long maxRelations, IAccessibleRelation __RPC_FAR *__RPC_FAR *relation, long __RPC_FAR *nRelations) ;
    STDMETHOD(role)(long __RPC_FAR *role);
    STDMETHOD(get_nActions)(long __RPC_FAR *nActions);
    STDMETHOD(scrollTo)(enum IA2ScrollType scrollType);
    STDMETHOD(scrollToPoint)(enum IA2CoordinateType coordinateType, long x, long y);
    STDMETHOD(get_groupPosition)(long __RPC_FAR *groupLevel,long __RPC_FAR *similarItemsInGroup,long __RPC_FAR *positionInGroup);
    STDMETHOD(get_states)( AccessibleStates __RPC_FAR *states );
    STDMETHOD(get_extendedRole)( BSTR __RPC_FAR *extendedRole );
    STDMETHOD(get_localizedExtendedRole)( BSTR __RPC_FAR *localizedExtendedRole );
    STDMETHOD(get_nExtendedStates)( long __RPC_FAR *nExtendedStates);
    STDMETHOD(get_extendedStates)( long maxExtendedStates, BSTR __RPC_FAR *__RPC_FAR *extendedStates, long __RPC_FAR *nExtendedStates);
    STDMETHOD(get_localizedExtendedStates)(long maxLocalizedExtendedStates,BSTR __RPC_FAR *__RPC_FAR *localizedExtendedStates,long __RPC_FAR *nLocalizedExtendedStates);
    STDMETHOD(get_uniqueID)(long __RPC_FAR *uniqueID);
    STDMETHOD(get_windowHandle)(HWND __RPC_FAR *windowHandle);
    STDMETHOD(get_indexInParent)( long __RPC_FAR *accParentIndex );
    STDMETHOD(get_locale)( IA2Locale __RPC_FAR *locale );
    STDMETHOD(get_attributes)(/*[out]*/ BSTR *pAttr);

    //IServiceProvider.
    STDMETHOD(QueryService)(REFGUID guidService, REFIID riid, void** ppvObject);

    //IAccessibleApplication
    STDMETHOD(get_appName)(BSTR __RPC_FAR *name);
    STDMETHOD(get_appVersion)(BSTR __RPC_FAR *version);
    STDMETHOD(get_toolkitName)(BSTR __RPC_FAR *name);
    STDMETHOD(get_toolkitVersion)(BSTR __RPC_FAR *version);

    // methods which are defined only in IMAccessible
    // These methods are provided for UNO management system.
    // The UNO management system use these methods to put Accessibility
    // information to COM.
    STDMETHOD(Put_XAccName)(const OLECHAR __RPC_FAR *pszName);
    STDMETHOD(Put_XAccRole)(unsigned short pRole);
    STDMETHOD(DecreaseState)(DWORD pXSate);
    STDMETHOD(IncreaseState)(DWORD pXSate);
    STDMETHOD(SetState)(DWORD pXSate);
    STDMETHOD(Put_XAccDescription)(const OLECHAR __RPC_FAR *pszDescription);
    STDMETHOD(Put_XAccValue)(const OLECHAR __RPC_FAR *pszAccValue);
    STDMETHOD(Put_XAccLocation)(const Location sLocation);
    STDMETHOD(Put_XAccFocus)(long dChildID);
    STDMETHOD(Put_XAccParent)(IMAccessible __RPC_FAR *pIParent);
    STDMETHOD(Put_XAccWindowHandle)(HWND hwnd);
    STDMETHOD(Put_XAccChildID)(long dChildID);
    STDMETHOD(Put_XAccAgent)(hyper pAgent);
    STDMETHOD(NotifyDestroy)(BOOL isDestroy);
    STDMETHOD(Put_ActionDescription)( const OLECHAR* szAction);
    STDMETHOD(SetDefaultAction)(hyper pAction);
    STDMETHOD(GetUNOInterface)(hyper*);
    STDMETHOD(SetXAccessible)(hyper);

private:
    OLECHAR* m_pszName;
    OLECHAR* m_pszValue;
    OLECHAR* m_pszActionDescription;
    unsigned short m_iRole;
    DWORD   m_dState;
    OLECHAR* m_pszDescription;
    IMAccessible* m_pIParent;
    Location m_sLocation;

    // identify a COM object/Acc object uniquely
    long m_dChildID;
    // specify the focus child ID in object self and its direct children

    long m_dFocusChildID;
    // parent window handle,will be used in the future application, its value comes from UNO

    HWND m_hwnd;

    // the COM class which implements IEnumVARIANT interface,currently only used in
    // the implementation of get_accSelection
    CEnumVariant* m_pEnumVar;

    // specify if the XAccessible is invalid
    BOOL m_isDestroy;

    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
    // initially m_xAction and m_xContext are the same object
    // but they may be different once AccObject::UpdateAction() is called?
    css::uno::Reference<css::accessibility::XAccessibleAction>  m_xAction;
    css::uno::Reference<css::accessibility::XAccessibleContext> m_xContext;

private:

    // the helper methods in order to implement the above public methods
    IMAccessible* GetChildInterface(long dChildIndex);//notice here the parameter is child index,not child id
    IMAccessible* GetNavigateChildForDM(VARIANT varCur,short flags);//for descendant manage
    HRESULT GetFirstChild(VARIANT varStart,VARIANT* pvarEndUpAt);//for accNavigate implementation
    HRESULT GetLastChild(VARIANT varStart,VARIANT* pvarEndUpAt);//for accNavigate implementation
    HRESULT GetNextSibling(VARIANT varStart,VARIANT* pvarEndUpAt);//for accNavigate implementation
    HRESULT GetPreSibling(VARIANT varStart,VARIANT* pvarEndUpAt);//for accNavigate implementation
    BOOL IsDescendantManage();//identify whether the current COM belongs to manage_descendant roles

    // the following private methods are used to implement accSelect method
    HRESULT SelectChild(css::accessibility::XAccessible* pItem);
    HRESULT DeSelectChild(css::accessibility::XAccessible* pItem);
    HRESULT SelectMutipleChidren(css::accessibility::XAccessible** pItem,
                int size);
    HRESULT DeSelectMutipleChildren(css::accessibility::XAccessible** pItem,
                int size);
    css::accessibility::XAccessibleContext* GetContextByXAcc(
            css::accessibility::XAccessible* pXAcc);
    css::uno::Reference<css::accessibility::XAccessibleSelection> GetSelection();
    // end accSelect implementation methods
    BOOL GetXInterfaceFromXAccessible(css::accessibility::XAccessible*,
            css::uno::XInterface**, int);
    HRESULT WINAPI SmartQI(void* pv, REFIID iid, void** ppvObject);

public:
    STDMETHOD(Get_XAccChildID)(/*[out,retval]*/ long* childID);
    // AccObjectManagerAgent is a management object in UNO, here keep its pointer for
    // the implementation of accNavigate when descendant manage happens for List,Tree, or Table
    // AccObjectManagerAgent and the following UNO objects XAccessble,XAccessibleSelection,
    // XAccessibleAction are all used to operate UNO accessiblility information directly when
    // implement some specific MSAA methods,such as accSelection,accNavigate
    static AccObjectManagerAgent* g_pAgent;

    static BOOL get_IAccessibleFromXAccessible(
            css::accessibility::XAccessible * pXAcc, IAccessible** ppIA);
    BOOL m_bRequiresSave;
    XGUIDToComObjHash m_containedObjects;

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return ((CMAccessible*)pv)->SmartQI(pv,iid,ppvObject);
    }

    static void get_OLECHARFromAny(css::uno::Any& pAny, OLECHAR* pChar);

    static void get_OLECHAR4Numbering(const css::uno::Any& pAny,
            short numberingLevel, const OUString& numberingPrefix,
            OLECHAR* pChar);

    // Helper function for data conversion.
    static void ConvertAnyToVariant(const css::uno::Any &rAnyVal,
            VARIANT *pvData);
};



#endif // INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_MACCESSIBLE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
