/*************************************************************************
 *
 *  $RCSfile: Test.cpp,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../XCallback_Impl/XCallback_Impl.h"
#include "../XCallback_Impl/XCallback_Impl_i.c"

CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

HRESULT doTest();

int main(int argc, char* argv[])
{
    HRESULT hr;
    if( FAILED( hr=CoInitialize(NULL)))
    {
        _tprintf(_T("CoInitialize failed \n"));
        return -1;
    }


    _Module.Init( ObjectMap, GetModuleHandle( NULL));

    if( FAILED(hr=doTest()))
    {
        _com_error err( hr);
        const TCHAR * errMsg= err.ErrorMessage();
        MessageBox( NULL, errMsg, "Test failed", MB_ICONERROR);
    }


    _Module.Term();
    CoUninitialize();


    return 0;
}


HRESULT doTest()
{
    HRESULT hr= S_OK;

    CComPtr<IUnknown> spUnk;
    hr= spUnk.CoCreateInstance(L"com.sun.star.ServiceManager");
    if( FAILED( hr))
        return hr;

    CComDispatchDriver manager( spUnk);
    CComVariant param( L"oletest.OleTest");
    CComVariant retVal;
    hr= manager.Invoke1((LPCOLESTR)L"createInstance", &param, &retVal );

    CComDispatchDriver oletest( retVal.punkVal);

    spUnk.Release();

    hr= spUnk.CoCreateInstance(L"XCallback_Impl.Callback");
    if( FAILED( hr))
        return hr;

    CComQIPtr<IDispatch> paramDisp(spUnk);


    //######################################################################
    //  out parameters
    //######################################################################
    CComVariant param1( paramDisp);
    CComVariant param2(1);
    // oletest calls XCallback::func1
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::returnInterface
    param2= 2;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outInterface
    param2= 3;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outStruct
    param2= 4;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outEnum
    param2= 5;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outSeqAny
    param2= 6;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outAny
    param2= 7;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outBool
    param2= 8;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outChar
    param2= 9;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outString
    param2= 10;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outFloat
    param2= 11;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outDouble
    param2= 12;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outByte
    param2= 13;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outShort
    param2= 14;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outLong
    param2= 15;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outValuesMixed
    param2= 30;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outValuesAll
    param2= 31;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    // XCallback::outSeqByte
    // Does not work currently because Sequences are always converted to
    // SAFEARRAY( VARIANT)
    //  param2= 32;
    //  hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    //######################################################################
    //  in / out parameters
    //######################################################################
    // XCallback::inoutInterface
    param2= 100;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutStruct
    param2= 101;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutEnum
    param2= 102;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutSeqAny
    param2= 103;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutAny
    param2= 104;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutBool
    param2= 105;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutChar
    param2= 106;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutString
    param2= 107;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutFloat
    param2= 108;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutDouble
    param2= 109;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutByte
    param2= 110;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutShort
    param2= 111;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutLong
    param2= 112;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutValuesAll
    param2=120;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    //######################################################################
    //  in  parameters
    //######################################################################
    // XCallback::inValues
    param2= 200;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inSeqByte
    // Does not work currently because Sequences are always converted to
    // SAFEARRAY( VARIANT)
    //  param2= 201;
    //  hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    //######################################################################
    //  The UNO test component OleTest calls on XCallback_Impl.Callback directly
    // that is the COM object has not been past a parameter but rather OleTest
    // creates the COM object itself
    //######################################################################
    // XCallback::outValuesAll
    // does not work currently
    param2= 300;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutValuesAll
    param2= 301;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutValues
    param2= 302;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    // XCallback::inValues
    param2= 303;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    //######################################################################
    // Test a COM object which implements several interfaces.
    //######################################################################

    CComQIPtr<IDispatch> dispSimple;
    hr= dispSimple.CoCreateInstance(L"XCallback_Impl.Simple");
    CComVariant varSimple( dispSimple);
    param2= 0;
    hr= oletest.Invoke2(L"testInterface2", &varSimple, &param2);

    return hr;
}
// VARIANT CComVariant VT_UNKNOWN VT_DISPATCH V_UI1 CComDispatchDriver WINAPI
