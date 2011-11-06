/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

