/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: advisesink.hxx,v $
 *
 *  $Revision: 1.1.8.2 $
 *
 *  last change: $Author: mav $ $Date: 2008/10/30 11:59:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

