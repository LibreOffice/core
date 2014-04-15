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


#pragma warning(disable: 4917)
#include <windows.h>
#include <comdef.h>
#include <tchar.h>
#include <atlbase.h>
#include <atlcom.h>
#include <stdio.h>
#include <com/sun/star/bridge/ModelDependent.hpp>
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <oletest/XCallback.hpp>
#include <rtl/process.h>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/servicefactory.hxx>
#include <rtl/string.h>



using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;
using namespace cppu;

HRESULT doTest();
bool incrementMultidimensionalIndex(
    sal_Int32 dimensions,
    const sal_Int32 * parDimensionLengths,
    sal_Int32 * parMultidimensionalIndex);

int SAL_CALL _tmain( int /*argc*/, _TCHAR * /*argv[]*/ )
{
    HRESULT hr;
    if( FAILED( hr=CoInitialize(NULL)))
    {
        _tprintf(_T("CoInitialize failed \n"));
        return -1;
    }


    if( FAILED(hr=doTest()))
    {
        _com_error err( hr);
        const TCHAR * errMsg= err.ErrorMessage();
        MessageBox( NULL, errMsg, "Test failed", MB_ICONERROR);
    }

    CoUninitialize();
    return 0;
}




HRESULT doTest()
{
    HRESULT hr= S_OK;
    long j = 0;
    SAFEARRAY* par;
    CComDispatchDriver disp;
    CComVariant result;
    CComVariant param1;
    CComPtr<IUnknown> spUnkFactory;
    if( SUCCEEDED( spUnkFactory.CoCreateInstance(L"com.sun.star.ServiceManager")))
    {
        disp= spUnkFactory;
        param1= L"oletest.OleTest";
        disp.Invoke1( L"createInstance", &param1, &result);

        disp= result.pdispVal;

        // disp contains now oletest.OleTest

        // one dimensional array
        par= SafeArrayCreateVector( VT_UI1, 0, 5);
        unsigned char arbyte[]= { 1,2,3,4,5};
        for(long i= 0; i < 5;i++)
            hr= SafeArrayPutElement( par, &i, &arbyte[i]);

        result.Clear();
        param1.vt= VT_ARRAY| VT_UI1;
        param1.byref= par;
        disp.Invoke1(L"methodByte", &param1, &result);
        SafeArrayDestroy( par);


        // two dimensional array
        SAFEARRAYBOUND bounds[2];
        // least significant dimension first, Dimension 1
        bounds[0].cElements= 3;
        bounds[0].lLbound= 0;
        // Dimension 2
        bounds[1].cElements= 2;
        bounds[1].lLbound= 0;
        par= SafeArrayCreate( VT_I4, 2, bounds );

        long uBound1;
        long uBound2;
        hr= SafeArrayGetUBound( par, 1, &uBound1);
        hr= SafeArrayGetUBound( par, 2, &uBound2);

        long index2[2];
        memset( index2, 0, 2 * sizeof( long) );
        long dimLengths[]={3,2};

        long data;
        do
        {
            data= index2[1] * 3 + index2[0] +1;
            hr= SafeArrayPutElement( par, index2, &data);
        }while( incrementMultidimensionalIndex( 2, dimLengths, index2) );

        long* pdata;
        long (*dataL)[2][3];
        hr= SafeArrayAccessData( par, (void**)&pdata);
        dataL= (long(*)[2][3])pdata;

        for (long i= 0; i < 2; i ++)
        {
            for(long j= 0; j < 3; j++)
                data= (*dataL)[i][j];
        }
        hr= SafeArrayUnaccessData(par);

        result.Clear();
        param1.vt= VT_ARRAY | VT_I4;
        param1.byref= par;
        disp.Invoke1(L"methodSequence", &param1, &result);

        SAFEARRAY* arRet= result.parray;

        for(long i= 0; i < 2 ; i++)
        {
            CComVariant varx;
            varx.Clear();
            hr= SafeArrayGetElement( arRet, &i, &varx);
            SAFEARRAY* ari= varx.parray;

            for( j= 0; j < 3; j++)
            {
                CComVariant varj;
                varj.Clear();
                hr= SafeArrayGetElement( ari, &j, &varj);
            }



        }
        SafeArrayDestroy( par);
    }

    return hr;
}

// left index is least significant
bool incrementMultidimensionalIndex(
    sal_Int32 dimensions,
    const sal_Int32 * parDimensionLengths,
    sal_Int32 * parMultidimensionalIndex)
{
    if( dimensions < 1)
        return sal_False;

    bool ret= sal_True;
    bool carry= sal_True; // to get into the while loop

    sal_Int32 currentDimension= 0; //most significant is 1
    while( carry)
    {
        parMultidimensionalIndex[ currentDimension ]++;
        // if carryover, set index to 0 and handle carry on a level above
        if( parMultidimensionalIndex[ currentDimension] > (parDimensionLengths[ currentDimension] - 1))
            parMultidimensionalIndex[ currentDimension]= 0;
        else
            carry= sal_False;

        currentDimension ++;
        // if dimensions drops below 1 and carry is set than then all indices are 0 again
        // this is signalled by returning sal_False
        if( currentDimension > dimensions - 1 && carry)
        {
            carry= sal_False;
            ret= sal_False;
        }
    }
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
