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

#ifndef INCLUDED_SHELL_INC_INTERNAL_COLUMNINFO_HXX
#define INCLUDED_SHELL_INC_INTERNAL_COLUMNINFO_HXX

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4917)
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif


class CColumnInfo : public IColumnProvider
{
public:
    CColumnInfo(long RefCnt = 1);
    virtual ~CColumnInfo();


    // IUnknown methods


    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);


    // IColumnProvider


    virtual HRESULT STDMETHODCALLTYPE Initialize(LPCSHCOLUMNINIT psci);

    virtual HRESULT STDMETHODCALLTYPE GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO *psci);

    virtual HRESULT STDMETHODCALLTYPE GetItemData(
        LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData);

private:
    bool IsOOFileExtension(wchar_t* Extension) const;

private:
    long    m_RefCnt;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
