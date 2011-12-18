/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// SOActionsApproval.cpp : Implementation of CHelpApp and DLL registration.

#include "stdafx2.h"

#include "so_activex.h"
#include "SOActionsApproval.h"
#include <sal/macros.h>

/////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP SOActionsApproval::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] =
    {
        &IID_ISOActionsApproval,
    };

    for (int i=0;i<SAL_N_ELEMENTS(arr);i++)
    {
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
        if (InlineIsEqualGUID(*arr[i],riid))
#else
        if (::ATL::InlineIsEqualGUID(*arr[i],riid))
#endif
            return S_OK;
    }
    return S_FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
