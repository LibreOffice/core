/*************************************************************************
 *
 *  $RCSfile: testcppuno.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jl $ $Date: 2000-10-24 09:34:19 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <windows.h>
#include <comdef.h>
#include <tchar.h>
#include <atlbase.h>
extern CComModule _Module;
#include<atlcom.h>
#include<atlimpl.cpp>

#include <com/sun/star/bridge/ModelDependent.hpp>
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <oletest/XCallback.hpp>
#include <rtl/process.h>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/servicefactory.hxx>
#include <rtl/string.h>
#pragma hdrstop


CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;
using namespace cppu;
using namespace rtl;
HRESULT doTest();
bool incrementMultidimensionalIndex(sal_Int32 dimensions,
                                                                   const sal_Int32 * parDimensionLengths,
                                                                   sal_Int32 * parMultidimensionalIndex);



int __cdecl _tmain( int argc, _TCHAR * argv[] )
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
    long i,j;
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
        for(  i= 0; i < 5;i++)
            hr= SafeArrayPutElement( par, &i, &arbyte[i]);

        result.Clear();
        param1.vt= VT_ARRAY| VT_UI1;
        param1.byref= par;
        disp.Invoke1(L"methodByte", &param1, &result);
        SafeArrayDestroy( par);


        // two dimensional array
        SAFEARRAYBOUND bounds[2];
        bounds[0].cElements= 3;
        bounds[0].lLbound= 0;
        bounds[1].cElements= 2;
        bounds[1].lLbound= 0;
        par= SafeArrayCreate( VT_I4, 2, bounds );


        long index2[2];
        memset( index2, 0, 2 * sizeof( long) );
        long dimLengths[]={3,2};

        long data;
        do
        {
            data= index2[1] * 3 + index2[0] +1;
            SafeArrayPutElement( par, index2, &data);
        }while( incrementMultidimensionalIndex( 2, dimLengths, index2) );

        result.Clear();
        param1.vt= VT_ARRAY | VT_I4;
        param1.byref= par;
        disp.Invoke1(L"methodSequence", &param1, &result);

        SAFEARRAY* arRet= result.parray;

        for( i= 0; i < 2 ; i++)
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
bool incrementMultidimensionalIndex(sal_Int32 dimensions,
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