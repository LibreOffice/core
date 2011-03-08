/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// EvtListener.h : Deklaration von CEvtListener

#ifndef __EVTLISTENER_H_
#define __EVTLISTENER_H_

#include "resource.h"       // Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CEvtListener
class ATL_NO_VTABLE CEvtListener :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CEvtListener, &CLSID_EvtListener>,
    public IDispatchImpl<IEvtListener, &IID_IEvtListener, &LIBID_EVENTLISTENERLib>
{
public:
    CEvtListener()
    {
    }
    ~CEvtListener();

DECLARE_REGISTRY_RESOURCEID(IDR_EVTLISTENER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEvtListener)
    COM_INTERFACE_ENTRY(IEvtListener)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IEvtListener
public:
    STDMETHOD(disposing)(IDispatch* source);
};

#endif //__EVTLISTENER_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
