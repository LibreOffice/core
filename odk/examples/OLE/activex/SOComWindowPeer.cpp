// SOComWindowPeer.cpp : Implementation of CHelpApp and DLL registration.

#include "stdafx2.h"
#include "so_activex.h"
#include "SOComWindowPeer.h"

/////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP SOComWindowPeer::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] =
    {
        &IID_ISOComWindowPeer,
    };

    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

