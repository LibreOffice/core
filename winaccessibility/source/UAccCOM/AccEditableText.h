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

#ifndef INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCEDITABLETEXT_H
#define INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCEDITABLETEXT_H

#include "resource.h"
#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include "UNOXWrapper.h"

/**
 * CAccEditableText implements IAccessibleEditableText interface.
 */
class CAccEditableText :
            public CComObjectRoot,
            public CComCoClass<CAccEditableText,&CLSID_AccEditableText>,
            public IAccessibleEditableText,
            public CUNOXWrapper
{
public:
    CAccEditableText()
    {

    }
    virtual ~CAccEditableText()
    {

    }

    BEGIN_COM_MAP(CAccEditableText)
    COM_INTERFACE_ENTRY(IAccessibleEditableText)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return ((CAccEditableText*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_NO_REGISTRY()

public:
    // IAccessibleEditableText

    // Copies a range of text to the clipboard.
    STDMETHOD(copyText)(long startOffset, long endOffset);

    // Deletes a range of text.
    STDMETHOD(deleteText)(long startOffset, long endOffset);

    // Inserts text at a specified offset.
    STDMETHOD(insertText)(long offset, BSTR * text);

    // Cuts a range of text to the clipboard.
    STDMETHOD(cutText)(long startOffset, long endOffset);

    // Pastes text from clipboard at specified offset.
    STDMETHOD(pasteText)(long offset);

    // Replaces range of text with new text.
    STDMETHOD(replaceText)(long startOffset, long endOffset, BSTR * text);


    // Sets attributes of range of text.
    STDMETHOD(setAttributes)(long startOffset, long endOffset, BSTR * attributes);

    // Override of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface);

private:

    css::uno::Reference<css::accessibility::XAccessibleEditableText> pRXEdtTxt;

    void get_AnyFromOLECHAR(const ::rtl::OUString &ouName, const ::rtl::OUString &ouValue, css::uno::Any &rAny);

    inline css::accessibility::XAccessibleEditableText* GetXInterface()
    {
        return pRXEdtTxt.get();
    }
};

#endif // INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCEDITABLETEXT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
