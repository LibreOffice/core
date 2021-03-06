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

#pragma once

#include <sal/config.h>

#include <string_view>

#include "Resource.h"
#include <com/sun/star/uno/Reference.hxx>
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
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,SmartQI_)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    static HRESULT WINAPI SmartQI_(void* pv,
                                   REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return static_cast<CAccEditableText*>(pv)->SmartQI(iid,ppvObject);
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
    STDMETHOD(copyText)(long startOffset, long endOffset) override;

    // Deletes a range of text.
    STDMETHOD(deleteText)(long startOffset, long endOffset) override;

    // Inserts text at a specified offset.
    STDMETHOD(insertText)(long offset, BSTR * text) override;

    // Cuts a range of text to the clipboard.
    STDMETHOD(cutText)(long startOffset, long endOffset) override;

    // Pastes text from clipboard at specified offset.
    STDMETHOD(pasteText)(long offset) override;

    // Replaces range of text with new text.
    STDMETHOD(replaceText)(long startOffset, long endOffset, BSTR * text) override;


    // Sets attributes of range of text.
    STDMETHOD(setAttributes)(long startOffset, long endOffset, BSTR * attributes) override;

    // Override of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface) override;

private:

    css::uno::Reference<css::accessibility::XAccessibleEditableText> pRXEdtTxt;

    static void get_AnyFromOLECHAR(std::u16string_view ouName, const OUString &ouValue, css::uno::Any &rAny);

    css::accessibility::XAccessibleEditableText* GetXInterface()
    {
        return pRXEdtTxt.get();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
