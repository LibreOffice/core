/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Simple.cpp,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:55:22 $
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

