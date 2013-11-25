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

#if !defined(AFX_ACCHYPERLINK_H__59DA79A5_A895_43DB_9495_2B2049CF5C65__INCLUDED_)
#define AFX_ACCHYPERLINK_H__59DA79A5_A895_43DB_9495_2B2049CF5C65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols


#include <com/sun/star/accessibility/XAccessibleHyperlink.hpp>
#include "AccActionBase.h"
#include "UNOXWrapper.h"

/**
 * CAccHyperLink implements IAccessibleHyperlink interface.
 */
class ATL_NO_VTABLE CAccHyperLink :
            public CComObjectRoot,
            public CComCoClass<CAccHyperLink,&CLSID_AccHyperLink>,
            public IAccessibleHyperlink,
            public CAccActionBase
{
public:
    CAccHyperLink()
    {
            }
    ~CAccHyperLink()
    {
            }

    BEGIN_COM_MAP(CAccHyperLink)
    COM_INTERFACE_ENTRY(IAccessibleAction)
    COM_INTERFACE_ENTRY(IAccessibleHyperlink)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    END_COM_MAP()

    DECLARE_NO_REGISTRY()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD)
    {
        return ((CAccHyperLink*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    // IAccessibleHyperlink
public:
    // IAccessibleAction

    // Returns the number of action.
    STDMETHOD(nActions)(/*[out,retval]*/long* nActions);

    // Performs specified action on the object.
    STDMETHOD(doAction)(/* [in] */ long actionIndex);

    // get the action name
    STDMETHOD(get_name)( long actionIndex, BSTR __RPC_FAR *name);

    // get the localized action name
    STDMETHOD(get_localizedName)( long actionIndex, BSTR __RPC_FAR *localizedName);

    // Gets description of specified action.
    STDMETHOD(get_description)(long actionIndex,BSTR __RPC_FAR *description);

    // Returns key binding object (if any) associated with specified action
    // key binding is string.
    // e.g. "alt+d" (like IAccessible::get_accKeyboardShortcut).
    STDMETHOD(get_keyBinding)(
        /* [in] */ long actionIndex,
        /* [in] */ long nMaxBinding,
        /* [length_is][length_is][size_is][size_is][out] */ BSTR __RPC_FAR *__RPC_FAR *keyBinding,
        /* [retval][out] */ long __RPC_FAR *nBinding);

    // IAccessibleHyperlink

    // get an object, e.g. BSTR or image object, that is overloaded with link behavior
    STDMETHOD(get_anchor)(/* [in] */ long index,
                                     /* [retval][out] */ VARIANT __RPC_FAR *anchor);

    // get an object representing the target of the link, usually a BSTR of the URI
    STDMETHOD(get_anchorTarget)(/* [in] */ long index,
                                           /* [retval][out] */ VARIANT __RPC_FAR *anchorTarget);

    // Returns the index at which the textual representation of the
    // hyperlink (group) starts.
    STDMETHOD(get_startIndex)(/* [retval][out] */ long __RPC_FAR *index);

    // Returns the index at which the textual rerpesentation of the
    // hyperlink (group) ends.
    STDMETHOD(get_endIndex)(/* [retval][out] */ long __RPC_FAR *index);

    // Returns whether the document referenced by this links is still valid.
    STDMETHOD(get_valid)(/* [retval][out] */ boolean __RPC_FAR *valid);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XSubInterface)(hyper pXSubInterface);

private:

    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessibleHyperlink> pRXLink;

    inline com::sun::star::accessibility::XAccessibleHyperlink* GetXInterface()
    {
        return pRXLink.get();
    }

};

#endif // !defined(AFX_ACCHYPERLINK_H__59DA79A5_A895_43DB_9495_2B2049CF5C65__INCLUDED_)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
