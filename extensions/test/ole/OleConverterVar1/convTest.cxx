/*************************************************************************
 *
 *  $RCSfile: convTest.cxx,v $
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
#include <oletest/XTestSequence.hpp>
#include <rtl/process.h>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/servicefactory.hxx>
#include <rtl/string.h>
#pragma hdrstop


CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

#include "smartarray.h"
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace oletest;
using namespace cppu;
using namespace rtl;
HRESULT doTest();
HRESULT InitializeParameter();
void printResultVariantArray( VARIANT & var);
void printVariant( VARIANT & var);




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
char _c[]={ 1,2,3,4,5};
short _short[]={0xffff, 1, 11 ,111, 1111 };
unsigned short _ushort[]={0xffff, 1, 11 ,111, 1111 };
long _long[]= { 0xffffffff, 11, 111 ,1111, 1111 };
unsigned long _ulong[]= { 0xffffffff, 11, 111 ,1111, 1111 };
float _float[]= { 12345., 1234.5, 123.45, 12.345, 1.2345};
double _double[]= {12345, 1234.5, 123.45, 12.345, 1.2345};

CComVariant _variant[]= {L"variant 1", L"variant2", L"variant3"};
wchar_t _wchar[]= {L'1', L'2', L'3', L'A', L' '};
BSTR _bstr[]={L"Ich", L"bin", L"ein", L"Hamburger", L"Jung"};
SmartArray<char>            arByte( _c, 5, VT_I1);
SmartArray< short>          arShort( _short, 5, VT_I2);
//SmartArray< unsigned short> arUShort( _ushort, 5, VT_UI2);
SmartArray< long>           arLong( _long, 5, VT_I4);
//SmartArray< unsigned long>    arULong( _ulong, 5, VT_UI4);
//SmartArray< float>            arFloat( _float, 5, VT_R4 );
SmartArray< double>         arDouble( _double, 5, VT_R8 );
//SmartArray< unsigned short> arWChar( _wchar, 5, VT_UI2 );
SmartArray< wchar_t* >      arString( _bstr, 5, VT_BSTR);
SmartArray< VARIANT >        arVariant( _variant, 3, VT_VARIANT);


HRESULT doTest()
{
    HRESULT hr;
    USES_CONVERSION;
    CComPtr<IUnknown> spUnkMgr;

    Reference< XMultiServiceFactory > mgr= createRegistryServiceFactory( OUString(L"applicat.rdb"));
    Reference< XInterface > xIntSupplier= mgr->createInstance(OUString(L"com.sun.star.bridge.OleBridgeSupplierVar1"));
    Reference< XBridgeSupplier2 > xSuppl( xIntSupplier, UNO_QUERY);
    Reference <XInterface> xOletest= mgr->createInstance( OUString(L"oletest.OleTest"));
    Any any;
    any <<= xOletest;
    sal_uInt8 arId[16];
    rtl_getGlobalProcessId( arId);
    Any target= xSuppl->createBridge( any, Sequence<sal_Int8>( (sal_Int8*)arId, 16), UNO, OLE);
    CComDispatchDriver oletest;
    if (target.getValueTypeClass() == getCppuType((sal_uInt32*) 0).getTypeClass())
    {
        VARIANT* pVariant = *(VARIANT**)target.getValue();

        oletest= pVariant->pdispVal;

        VariantClear(pVariant);
        CoTaskMemFree(pVariant);
    }

    CComVariant varRet;
    CComVariant varParam1;
    CComVariant varParam2;
    CComVariant varParam3;
    CComVariant varParam4;

    long value= 100;
    varParam1.vt= VT_I1 | VT_BYREF;
    varParam1.plVal= &value;

    // Testing the caching of DISPIDs and the process of aquiring member information
    // on demand in IDispatch::Invoke
    // Step through the corresponding IDispatch implementation of the ole bridge
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodByte"), &varParam1, &varRet);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodByte"), &varParam1, &varRet);
    // Name ok but different case
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"Testinout_methodByte"), &varParam1, &varRet);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"Testinout_methodByte"), &varParam1, &varRet);
    // not existing member
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"Testinout"), &varParam1, &varRet);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"Testinout"), &varParam1, &varRet);

    // Property
    varParam1.vt= VT_ARRAY | VT_I1;
    varParam1.parray= (SAFEARRAY*)arByte;
    hr= oletest.PutPropertyByName( static_cast<LPCOLESTR>(L"AttrByte"), &varParam1);
    hr= oletest.PutPropertyByName( static_cast<LPCOLESTR>(L"AttrByte"), &varParam1);
    // Name ok but different case
    hr= oletest.PutPropertyByName( static_cast<LPCOLESTR>(L"attrByte"), &varParam1);
    hr= oletest.PutPropertyByName( static_cast<LPCOLESTR>(L"attrByte"), &varParam1);
    // not existing member
    hr= oletest.PutPropertyByName( static_cast<LPCOLESTR>(L"attr"), &varParam1);
    hr= oletest.PutPropertyByName( static_cast<LPCOLESTR>(L"attr"), &varParam1);

    // PropertyGet
    hr= oletest.GetPropertyByName( static_cast<LPCOLESTR>(L"AttrByte"), &varRet);

    hr= oletest.GetPropertyByName( static_cast<LPCOLESTR>(L"attrByte"), &varRet);
    hr= oletest.GetPropertyByName( static_cast<LPCOLESTR>(L"attrByte"), &varRet);
    //not existing member
    hr= oletest.GetPropertyByName( static_cast<LPCOLESTR>(L"attrBy"), &varRet);
    hr= oletest.GetPropertyByName( static_cast<LPCOLESTR>(L"attrBy"), &varRet);

    DISPID dispid;
    LPOLESTR method= L"methodByte";
    hr = oletest.p->GetIDsOfNames(IID_NULL, &method, 1, LOCALE_USER_DEFAULT, &dispid);


    CComVariant arg[1];
    arg[0].vt= VT_ARRAY | VT_I1;
    arg[0].parray= (SAFEARRAY*)arByte;
    DISPPARAMS params={ arg,0,1,0};

    hr = oletest.p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_METHOD | DISPATCH_PROPERTYPUT, &params, &varRet, NULL, NULL);

    hr = oletest.p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_METHOD | DISPATCH_PROPERTYPUT, &params, &varRet, NULL, NULL);

    // different case
    LPOLESTR method2= L"MEthodByte";
    hr = oletest.p->GetIDsOfNames(IID_NULL, &method2, 1, LOCALE_USER_DEFAULT, &dispid);

    hr = oletest.p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_METHOD | DISPATCH_PROPERTYPUT, &params, &varRet, NULL, NULL);

    hr = oletest.p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_METHOD | DISPATCH_PROPERTYPUT, &params, &varRet, NULL, NULL);

    LPOLESTR attrib= L"AttrByte";
    hr = oletest.p->GetIDsOfNames(IID_NULL, &attrib, 1, LOCALE_USER_DEFAULT, &dispid);

    hr = oletest.p->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_METHOD | DISPATCH_PROPERTYPUTREF, &params, &varRet, NULL, NULL);

    hr = oletest.p->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT,
        DISPATCH_METHOD | DISPATCH_PROPERTYGET, &params, &varRet, NULL, NULL);



    CComVariant varByteArray;
    varByteArray.vt= VT_ARRAY | VT_I1;
    varByteArray.parray= (SAFEARRAY*)arByte;
    CComVariant varShortArray;
    varShortArray.vt= VT_ARRAY | VT_I2;
    varShortArray.parray= (SAFEARRAY*)arShort;
    CComVariant varLongArray;
    varLongArray.vt= VT_ARRAY | VT_I4;
    varLongArray.parray= (SAFEARRAY*)arLong;
    CComVariant varDoubleArray;
    varDoubleArray.vt= VT_ARRAY | VT_R8;
    varDoubleArray.parray= (SAFEARRAY*)arDouble;
    CComVariant varStringArray;
    varStringArray.vt= VT_ARRAY | VT_BSTR;
    varStringArray.parray= (SAFEARRAY*)arString;
    CComVariant varArray;
    varArray.vt= VT_ARRAY | VT_VARIANT;
    varArray.parray= (SAFEARRAY*)arVariant;

    FONTDESC fd={ sizeof( fd), L"ARIAL", 10, FW_NORMAL, 0, 0, 0, 0};


    CComPtr< IUnknown > unk1;
    CComPtr< IUnknown > unk2;
    CComPtr< IUnknown > unk3;

    IUnknown* _unknown[3];
    hr= OleCreateFontIndirect( &fd, __uuidof( IUnknown), (void**)&unk1.p);
    hr= OleCreateFontIndirect( &fd, __uuidof( IUnknown), (void**)&unk2.p);
    hr= OleCreateFontIndirect( &fd, __uuidof( IUnknown), (void**)&unk3.p);
    _unknown[0]= unk1;
    _unknown[1]= unk2;
    _unknown[2]= unk3;
    SmartArray<IUnknown*> arUnknown( _unknown, 3, VT_UNKNOWN);

    CComVariant varUnkArray;
    varUnkArray.vt= VT_ARRAY | VT_UNKNOWN;
    varUnkArray.parray= (SAFEARRAY*)arUnknown;

    // preparing out parameter;
    char byteOut;
    CComVariant varOutByte;     //###
    varOutByte.vt= VT_BYREF | VT_UI1;
    V_I1REF(&varOutByte)= &byteOut;
    short shortOut;
    CComVariant varOutShort;    //###
    varOutShort.vt= VT_BYREF | VT_I2;
    V_I2REF( &varOutShort)= &shortOut;
    long longOut;
    CComVariant varOutLong;     //###
    varOutLong.vt= VT_BYREF | VT_I4;
    V_I4REF( &varOutLong)= &longOut;
    double doubleOut;
    CComVariant varOutDouble;   //###
    varOutDouble.vt= VT_BYREF | VT_R8;
    V_R8REF( &varOutDouble)= &doubleOut;
    BSTR bstrOut= NULL;
    CComVariant varOutString;   //###
    varOutString.vt= VT_BYREF | VT_BSTR;
    V_BSTRREF(&varOutString)= &bstrOut;
    CComVariant variantOut;
    CComVariant varOutAny;      //###
    varOutAny.vt= VT_BYREF | VT_VARIANT;
    V_VARIANTREF(&varOutAny)= &variantOut;

    CComPtr<IDispatch> dispOut;
    CComVariant varOutXInterface; //###
    varOutXInterface.vt= VT_BYREF |VT_DISPATCH;
    V_DISPATCHREF(&varOutXInterface)= &dispOut.p;

    // In Parameter ( all of type Sequence ###########################################################
    OutputDebugString( _T("In parameter of type Sequence ###########################################\n"
        "The functions return the Sequence parameter \n\n"));

    OutputDebugStringA("methodByte | Params: \n");
    printVariant( varByteArray);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"methodByte"), &varByteArray, &varRet);
    OutputDebugStringA("methodByte  | return value \n");
    printVariant( varRet);

    OutputDebugStringA("methodShort | Params: \n");
    printVariant( varShortArray);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"methodShort"), &varShortArray, &varRet);
    OutputDebugStringA("methodShort  | return value \n");
    printVariant( varRet);

    OutputDebugStringA("methodLong | Params: \n");
    printVariant( varLongArray);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"methodLong"), &varLongArray, &varRet);
    OutputDebugStringA("methodLong  | return value \n");
    printVariant( varRet);

    OutputDebugStringA("methodDouble | Params: \n");
    printVariant( varDoubleArray);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"methodDouble"), &varDoubleArray, &varRet);
    OutputDebugStringA("methodDouble  | return value \n");
    printVariant( varRet);

    OutputDebugStringA("methodString | Params: \n");
    printVariant( varStringArray);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"methodString"), &varStringArray, &varRet);
    OutputDebugStringA("methodString  | return value \n");
    printVariant( varRet);

    OutputDebugStringA("methodAny | Params: \n");
    printVariant( varArray);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"methodAny"), &varArray, &varRet);
    OutputDebugStringA("methodAny  | return value \n");
    printVariant( varRet);

    OutputDebugStringA("methodXInterface | Params: \n");
    printVariant( varUnkArray);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"methodXInterface"), &varUnkArray, &varRet);
    OutputDebugStringA("methodAny  | return value \n");
    printVariant( varRet);

    // Out Parameter ###########################################################################
    OutputDebugString( _T("Out parameter ###########################################\n\n"));

    OutputDebugString(_T("testout_methodByte  \n"));
    hr= oletest.InvokeN(static_cast<LPCOLESTR>(L"testout_methodByte"), &varOutByte, 1, &varRet);
    OutputDebugString(_T("testout_methodByte  | out value: \n"));
    printVariant( varOutByte);

    OutputDebugString(_T("testout_methodShort \n"));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testout_methodShort"), &varOutShort, &varRet);
    OutputDebugString(_T("testout_methodShort  | out value: \n"));
    printVariant( varOutShort);

    OutputDebugString(_T("testout_methodLong \n"));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testout_methodLong"), &varOutLong, &varRet);
    OutputDebugString(_T("testout_methodLong  | out value: \n"));
    printVariant( varOutLong);

    OutputDebugString(_T("testout_methodDouble \n"));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testout_methodDouble"), &varOutDouble, &varRet);
    OutputDebugString(_T("testout_methodDouble  | out value: \n"));
    printVariant( varOutDouble);

    OutputDebugString(_T("testout_methodString \n"));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testout_methodString"), &varOutString, &varRet);
    OutputDebugString(_T("testout_methodString  | out value: \n"));
    printVariant( varOutString);

    OutputDebugString(_T("testout_methodAny \n"));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testout_methodAny"), &varOutAny, &varRet);
    OutputDebugString(_T("methodAny  | out value: \n"));
    printVariant( varOutAny);

    OutputDebugString(_T("testout_methodXInterface \n"));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testout_methodXInterface"), &varOutXInterface, &varRet);
    OutputDebugString(_T("methodAny  | out value: \n"));
    printVariant( varOutXInterface);
    CComDispatchDriver outDisp( *varOutXInterface.ppdispVal);
    CComVariant varAttr3;
    outDisp.GetPropertyByName(L"AttrAny2", &varAttr3);
    ATLTRACE("property OleTest.AttrAny2: %s", W2T(varAttr3.bstrVal));

    OutputDebugString(_T("testout_methodMulParams1 ( 2 out Parameter) \n"));
    long longOut2=0;
    CComVariant _params[2];
    longOut=0;
    _params[0]= varOutLong;
    _params[1].vt= VT_BYREF | VT_I4;
    V_I4REF(& _params[1])= &longOut2;
    hr= oletest.InvokeN( static_cast<LPCOLESTR>(L"testout_methodMulParams1"), (VARIANT*)&_params, 2);
    OutputDebugString(_T("testout_methodMulParams1  | out values: \n"));
    printVariant( _params[1]);
    printVariant( _params[0]);

    OutputDebugString(_T("testout_methodMulParams2 ( 3 out Parameter) \n"));
    CComVariant _params2[3];
    _params2[2]= varOutLong;
    _params2[1].vt= VT_BYREF | VT_I4;
    V_I4REF(& _params2[1])= &longOut2;
    _params2[0]= varOutString;
    hr= oletest.InvokeN( static_cast<LPCOLESTR>( L"testout_methodMulParams2"), (VARIANT*)&_params2, 3);
    OutputDebugString(_T("testout_methodMulParams2  | out values: \n"));
    printVariant( _params2[2]);
    printVariant( _params2[1]);
    printVariant( _params2[0]);

    OutputDebugString(_T("testout_methodMulParams3 ( 1 in and 1 out Parameter) \n"));
    CComVariant _params3[2];
    _params3[1]= CComBSTR(L" In string");
    _params3[0]= varOutString;
    hr= oletest.InvokeN( static_cast<LPCOLESTR>( L"testout_methodMulParams3"), (VARIANT*)&_params3, 2);
    OutputDebugString(_T("testout_methodMulParams3  | out values: \n"));
    printVariant( _params3[1]);
    printVariant( _params3[0]);

    //In Out Parameter ###########################################################################
    OutputDebugString( _T("In Out parameter ###########################################\n\n"));

    *V_I1REF(&varOutByte)= 5;
    ATLTRACE(_T("testinout_methodByte | in value: %d \n"), *V_I1REF(&varOutByte));
    hr= oletest.InvokeN(static_cast<LPCOLESTR>(L"testinout_methodByte"), &varOutByte, 1, &varRet);
    OutputDebugString(_T("testinout_methodByte  | out value: \n"));
    printVariant( varOutByte);

    OutputDebugString(_T("testinout_methodShort | in value= 1000 \n"));
    *V_UI2REF(&varOutShort)= 1000;
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodShort"), &varOutShort, &varRet);
    OutputDebugString(_T("testinout_methodShort  | out value: \n"));
    printVariant( varOutShort);

    OutputDebugString(_T("testinout_methodLong | in value= 10000 \n"));
    *V_UI4REF(&varOutLong)= 10000;
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodLong"), &varOutLong, &varRet);
    OutputDebugString(_T("testinout_methodLong  | out value: \n"));
    printVariant( varOutLong);

    *V_R8REF(&varOutDouble)= 3.14;
    ATLTRACE(_T("testinou_methodDouble in value: %f \n"),*V_R8REF(&varOutDouble));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodDouble"), &varOutDouble, &varRet);
    OutputDebugString(_T("testinout_methodDouble  | out value: \n"));
    printVariant( varOutDouble);

    SysFreeString( *V_BSTRREF(&varOutString));
    *V_BSTRREF(&varOutString)= SysAllocString( L"this is a in string");
    ATLTRACE(_T("testinout_methodString | value: %s \n"), W2T(*V_BSTRREF(&varOutString)));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodString"), &varOutString, &varRet);
    OutputDebugString(_T("testinout_methodString  | out value: \n"));
    printVariant( varOutString);

    CComVariant var1(CComBSTR(L" this is a string in a VARIANT"));
    CComVariant outVar1;
    outVar1.vt= VT_BYREF | VT_VARIANT;
    outVar1.pvarVal= &var1;
    ATLTRACE(_T("testinout_methodAny | parameter: %s\n"), W2T(var1.bstrVal));
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodAny"), &varOutAny, &varRet);
    OutputDebugString(_T("testinout_methodAny  | out value: \n"));
    printVariant( varOutAny);

    CComPtr< IUnknown > objectIn = unk1;
    CComVariant varOutIFace;
    varOutIFace.vt= VT_BYREF | VT_UNKNOWN;
    varOutIFace.ppunkVal= &objectIn.p;
    (*varOutIFace.ppunkVal)->AddRef();
    OutputDebugString(_T("testinout_methodXInterface | in value: \n"));
    printVariant(varOutIFace);
    hr= oletest.Invoke1(static_cast<LPCOLESTR>(L"testinout_methodXInterface"), &varOutIFace, &varRet);
    OutputDebugString(_T("testinout_methodXInterface  | out value: \n"));
    printVariant( varOutIFace);

    // Properties ######################################################################
    OutputDebugString( _T(" Properties ###########################################\n\n"));

    OutputDebugString(_T("set property \"AttrByte\" | value"));
    CComVariant propArByte;
    propArByte.vt= VT_ARRAY | VT_I1;
      varParam1.parray= (SAFEARRAY*)arByte;
    printVariant( varParam1);
    hr= oletest.PutPropertyByName( static_cast<LPCOLESTR>(L"AttrByte"), &varParam1);
    OutputDebugString(_T("get property \"AttrByte\" | value:"));
    varRet.Clear();
    hr= oletest.GetPropertyByName( static_cast<LPCOLESTR>(L"AttrByte"), &varRet);
    printVariant( varRet);



    return S_OK;


}


void printVariant( VARIANT & _var)
{
    HRESULT hr;
    USES_CONVERSION;
    CComVariant var;
    hr= VariantCopyInd( &var, &_var);
    if( var.vt & VT_ARRAY)
    {
        VARTYPE type= var.vt ^ VT_ARRAY;
        SAFEARRAY * sarray= var.parray;
        long lbound;
        long ubound;
        hr= SafeArrayGetLBound( sarray, 1, &lbound);
        hr= SafeArrayGetUBound( sarray, 1, &ubound);
        long count= ubound - lbound + 1;
        char charValue;
        BYTE byteValue;
        short shortValue;
        long longValue;
        double doubleValue;
        IUnknown* unkValue;
        BSTR bstrValue;
        OutputDebugString( _T("# Array \n"));
        for( long i= 0; i < count; i++)
        {
//          CComVariant variantValue;
            TCHAR *buf[256];
            wsprintf( (TCHAR*)buf, _T("%d : "), i);
            OutputDebugString( (TCHAR*)buf);
            VARIANT varTemp;
            VariantInit( &varTemp);
            VARIANT variantValue;
            VariantInit( &variantValue);
            switch( type)
            {
            case VT_UI1:
                hr= SafeArrayGetElement( sarray, &i, &byteValue);
                varTemp.vt= VT_UI1;
                V_UI1( &varTemp)= byteValue;
                printVariant( varTemp);
                break;
            case VT_I1:
                hr= SafeArrayGetElement( sarray, &i, &charValue);
                varTemp.vt= VT_I1;
                V_I1( &varTemp)= charValue;
                printVariant( varTemp);
                break;
            case VT_I2:
                hr= SafeArrayGetElement( sarray, &i, &shortValue);
                varTemp.vt= VT_I2;
                V_I2( &varTemp)= shortValue;
                printVariant( varTemp);
                break;

            case VT_UI2:
            case VT_I4:
                hr= SafeArrayGetElement( sarray, &i, &longValue);
                varTemp.vt= VT_I4;
                V_I4( &varTemp)= longValue;
                printVariant( varTemp);
                break;
            case VT_R8:
                hr= SafeArrayGetElement( sarray, &i, &doubleValue);
                varTemp.vt= VT_R8;
                V_R8( &varTemp)= doubleValue;
                printVariant( varTemp);
                break;
            case VT_BSTR:
                hr= SafeArrayGetElement( sarray, &i, &bstrValue);
                varTemp.vt= VT_BSTR;
                varTemp.bstrVal= bstrValue;
                printVariant( varTemp);
                break;
            case VT_VARIANT:
                hr= SafeArrayGetElement( sarray, &i, &varTemp);
                printVariant( varTemp);
                break;

            case VT_UNKNOWN:
                hr= SafeArrayGetElement( sarray, &i, &unkValue);
                varTemp.vt= VT_UNKNOWN;
                varTemp.punkVal= unkValue;
                printVariant( varTemp);
                break;
            }

            VariantClear( &varTemp);
            VariantClear( &variantValue);
        }

    }
    else
    {
        TCHAR buf[256];
        switch (var.vt)
        {
        case VT_I1: wsprintf( (TCHAR*)buf, _T(" VT_I1: %d \n"), V_I1( &var) );
            break;
        case VT_UI1: wsprintf( (TCHAR*)buf, _T(" VT_UI1: %d \n"), V_I1( &var) );
            break;

        case VT_I2: wsprintf( (TCHAR*)buf, _T(" VT_I2: %d \n"), V_I2( &var) );
            break;
        case VT_I4: wsprintf( (TCHAR*)buf, _T(" VT_I4: %d \n"), V_I4( &var) );
            break;
        case VT_R8:
            {

//              int     decimal,   sign;
//              char    *buffer;
//              int     precision = 14;
//              double  source = 3.1415926535;

//              buffer = _ecvt( V_R8(&var), precision, &decimal, &sign );
                sprintf( (TCHAR*)buf, _T(" VT_R8: %f \n"),V_R8( &var) );
            break;
            }
        case VT_UNKNOWN:
            // The object implement IFont
            {
                CComDispatchDriver disp( var.punkVal);
                CComVariant ret;
                hr= disp.GetPropertyByName( static_cast<LPCOLESTR>(L"Name"), &ret);
                wsprintf( (TCHAR*)buf, _T(" VT_UNKNOWN: property \"Name\": %s \n"), W2T(ret.bstrVal));
                break;
            }
        case VT_DISPATCH:
            // The object implement IFont
            {
                CComDispatchDriver disp( var.punkVal);
                CComVariant ret;
                if( SUCCEEDED( hr= disp.GetPropertyByName( static_cast<LPCOLESTR>(L"Name"), &ret)))
                    wsprintf( (TCHAR*)buf, _T(" VT_DISPATCH: property \"Name\": %s \n"), W2T(ret.bstrVal));
                else
                    wsprintf( (TCHAR*)buf, _T(" VT_DISPATCH \n"));

                break;
            }


        case VT_BSTR:
            {
                TCHAR* str= W2T( var.bstrVal);
                wsprintf( (TCHAR*)buf, _T(" VT_BSTR: %s \n"), str);
            }
            break;
        default:
            wsprintf( (TCHAR*)buf, _T("\n"));

        }

        OutputDebugString( (TCHAR*) buf);
    }

        return;

}

