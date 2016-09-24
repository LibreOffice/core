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
// Simple.cpp : Implementation of CSimple
#include "stdafx.h"
#include "XCallback_Impl.h"
#include "Simple.h"


// CSimple


STDMETHODIMP CSimple::func(BSTR message)
{
    USES_CONVERSION;
    MessageBox( NULL, W2T( message), _T("XCallback_Impl.Simple"), MB_OK);
    return S_OK;
}


STDMETHODIMP CSimple::func2(BSTR message)
{
    USES_CONVERSION;
    MessageBox( NULL, W2T( message), _T("XCallback_Impl.Simple"), MB_OK);
    return S_OK;
}

STDMETHODIMP CSimple::func3(BSTR message)
{
    USES_CONVERSION;
    MessageBox( NULL, W2T( message), _T("XCallback_Impl.Simple"), MB_OK);
    return S_OK;
}


STDMETHODIMP CSimple::get__implementedInterfaces(LPSAFEARRAY *pVal)
{
    HRESULT hr= S_OK;
    SAFEARRAY *pArr=    SafeArrayCreateVector( VT_BSTR, 0, 3);
    if( pArr)
    {   long index=0;
        BSTR name1= SysAllocString(L"oletest.XSimple");
        BSTR name2= SysAllocString(L"oletest.XSimple2");
        BSTR name3= SysAllocString(L"oletest.XSimple3");

        hr= SafeArrayPutElement( pArr, & index, name1);
        index++;
        hr|= SafeArrayPutElement( pArr, &index, name2);
        index++;
        hr|= SafeArrayPutElement( pArr, &index, name3);
        *pVal= pArr;

    }
    *pVal= pArr;
    return hr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
