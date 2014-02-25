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

// SOActionsApproval.cpp : Implementation of CHelpApp and DLL registration.

#include "stdafx2.h"

#include "so_activex.h"
#include "SOActionsApproval.h"
#include <sal/macros.h>




STDMETHODIMP SOActionsApproval::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] =
    {
        &IID_ISOActionsApproval,
    };

    for (int i=0;i<SAL_N_ELEMENTS(arr);i++)
    {
#ifdef _MSC_VER
        if (InlineIsEqualGUID(*arr[i],riid))
#else
        if (::ATL::InlineIsEqualGUID(*arr[i],riid))
#endif
            return S_OK;
    }
    return S_FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
