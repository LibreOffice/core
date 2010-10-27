/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <windows.h>
#include "smartpointer.hxx"

namespace inprocserv {

class OleWrapperAdviseSink : public IAdviseSink
{
protected:
    ULONG m_nRefCount;

    ComSmart< IAdviseSink > m_pListener;
    DWORD m_nListenerID;

    FORMATETC* m_pFormatEtc;
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
    OleWrapperAdviseSink( const ComSmart< IAdviseSink >& pListener );

    // an AdviseSink for IDataObject interface
    OleWrapperAdviseSink( const ComSmart< IAdviseSink >& pListener, FORMATETC* pFormatEtc, DWORD nDataRegFlag );

    // an AdviseSink for IViewObject interface
    OleWrapperAdviseSink( const ComSmart< IAdviseSink >& pListener, DWORD nAspect, DWORD nViewRegFlag );

    virtual ~OleWrapperAdviseSink();

    void SetRegID( DWORD nRegID ) { m_nRegID = nRegID; }
    DWORD GetRegID() { return m_nRegID; }

    BOOL IsOleAdvise() { return m_bObjectAdvise; }
    DWORD GetDataAdviseFlag() { return m_nDataRegFlag; }
    DWORD GetViewAdviseFlag() { return m_nViewRegFlag; }

    FORMATETC* GetFormatEtc() { return m_pFormatEtc; }
    DWORD GetAspect() { return m_nAspect; }
    ComSmart< IAdviseSink >& GetOrigAdvise() { return m_pListener; }
    void DisconnectOrigAdvise() { m_pListener = NULL; }

    void SetClosed() { m_bClosed = TRUE; }
    void UnsetClosed() { m_bClosed = FALSE; }
    BOOL IsClosed() { return m_bClosed; }

    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP_(void)  OnDataChange(LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP_(void)  OnViewChange(DWORD, LONG);
    STDMETHODIMP_(void)  OnRename(LPMONIKER);
    STDMETHODIMP_(void)  OnSave(void);
    STDMETHODIMP_(void)  OnClose(void);
};

}; // namespace advisesink

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
