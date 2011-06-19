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
// Simple.cpp : Implementation of CSimple
#include "stdafx.h"
#include "XCallback_Impl.h"
#include "Simple.h"

/////////////////////////////////////////////////////////////////////////////
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
        hr= SafeArrayPutElement( pArr, &index, name2);
        index++;
        hr= SafeArrayPutElement( pArr, &index, name3);
        *pVal= pArr;

    }
    *pVal= pArr;
    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
