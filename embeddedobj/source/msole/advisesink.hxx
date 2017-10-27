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

#ifndef INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_ADVISESINK_HXX
#define INCLUDED_EMBEDDEDOBJ_SOURCE_MSOLE_ADVISESINK_HXX

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include "platform.h"

class OleComponent;
class OleWrapperAdviseSink : public IAdviseSink
{
protected:
    osl::Mutex m_aMutex;
    oslInterlockedCount m_nRefCount;
    OleComponent* m_pOleComp;

public:
    OleWrapperAdviseSink( OleComponent* pOleComp );
    OleWrapperAdviseSink();
    virtual ~OleWrapperAdviseSink();

    void disconnectOleComponent();
    STDMETHODIMP QueryInterface(REFIID, void**) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    STDMETHODIMP_(void)  OnDataChange(LPFORMATETC, LPSTGMEDIUM) override;
    STDMETHODIMP_(void)  OnViewChange(DWORD, LONG) override;
    STDMETHODIMP_(void)  OnRename(LPMONIKER) override;
    STDMETHODIMP_(void)  OnSave() override;
    STDMETHODIMP_(void)  OnClose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
