/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// EvtListener.cpp : Implementierung von CEvtListener
#include "stdafx.h"
#include "EventListener.h"
#include "EvtListener.h"

/////////////////////////////////////////////////////////////////////////////
// CEvtListener

STDMETHODIMP CEvtListener::disposing( IDispatch* source)
{
    ::MessageBox(NULL,_T("XEventListener::disposing"),
        _T("EventListener.EvtListener component"), MB_OK);
    return S_OK;
}

CEvtListener::~CEvtListener()
{

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
