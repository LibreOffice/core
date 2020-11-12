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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INPROCSERV_ADVISESINK_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INPROCSERV_ADVISESINK_HXX

#include <sal/config.h>

#include <memory>

#if !defined WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objidl.h>
#include "smartpointer.hxx"

namespace inprocserv
{
class OleWrapperAdviseSink : public IAdviseSink
{
protected:
    ULONG m_nRefCount;

    ComSmart<IAdviseSink> m_pListener;
    DWORD m_nListenerID;

    std::unique_ptr<FORMATETC> m_pFormatEtc;
    DWORD m_nAspect;

    DWORD m_nRegID;
    DWORD m_bObjectAdvise;
    DWORD m_nDataRegFlag;
    DWORD m_nViewRegFlag;

    BOOL m_bHandleClosed;
    BOOL m_bClosed;

public:
    // an AdviseSink for own needs, should be created always
    OleWrapperAdviseSink();

    // an AdviseSink for IOleObject interface
    explicit OleWrapperAdviseSink(const ComSmart<IAdviseSink>& pListener);

    // an AdviseSink for IDataObject interface
    OleWrapperAdviseSink(const ComSmart<IAdviseSink>& pListener, FORMATETC* pFormatEtc,
                         DWORD nDataRegFlag);

    // an AdviseSink for IViewObject interface
    OleWrapperAdviseSink(const ComSmart<IAdviseSink>& pListener, DWORD nAspect, DWORD nViewRegFlag);

    virtual ~OleWrapperAdviseSink();

    void SetRegID(DWORD nRegID) { m_nRegID = nRegID; }
    DWORD GetRegID() { return m_nRegID; }

    bool IsOleAdvise() { return m_bObjectAdvise; }
    DWORD GetDataAdviseFlag() { return m_nDataRegFlag; }
    DWORD GetViewAdviseFlag() { return m_nViewRegFlag; }

    FORMATETC* GetFormatEtc() { return m_pFormatEtc.get(); }
    DWORD GetAspect() { return m_nAspect; }
    ComSmart<IAdviseSink>& GetOrigAdvise() { return m_pListener; }
    void DisconnectOrigAdvise() { m_pListener = nullptr; }

    void SetClosed() { m_bClosed = TRUE; }
    void UnsetClosed() { m_bClosed = FALSE; }
    BOOL IsClosed() { return m_bClosed; }

    STDMETHODIMP QueryInterface(REFIID, void**) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    STDMETHODIMP_(void) OnDataChange(FORMATETC*, STGMEDIUM*) override;
    STDMETHODIMP_(void) OnViewChange(DWORD, LONG) override;
    STDMETHODIMP_(void) OnRename(IMoniker*) override;
    STDMETHODIMP_(void) OnSave() override;
    STDMETHODIMP_(void) OnClose() override;
};

}; // namespace advisesink

#endif // INCLUDED_EMBEDSERV_SOURCE_INPROCSERV_ADVISESINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
