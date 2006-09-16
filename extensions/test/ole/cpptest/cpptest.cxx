/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cpptest.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:32:57 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
// cpptest.cpp : Defines the entry point for the console application.
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <comdef.h>
#include <tchar.h>
#include<atlbase.h>
CComModule _Module;
#include<atlcom.h>
#include<atlimpl.cpp>

//CComModule _Module;
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
    HRESULT hr;
    CComPtr<IUnknown> spUnkMgr;


    if( FAILED(hr= spUnkMgr.CoCreateInstance(L"com.sun.star.ServiceManager")))
        return hr;

    IDispatchPtr starManager;
    //    var starManager=new ActiveXObject("com.sun.star.ServiceManager");
    hr= starManager.CreateInstance(_T("com.sun.star.ServiceManager"));
    //    var starDesktop=starManager.createInstance("com.sun.star.frame.Desktop");
    _variant_t varP1(L"com.sun.star.frame.Desktop");
    _variant_t varRet;
    CComDispatchDriver dispMgr(starManager);
    hr= dispMgr.Invoke1(L"createInstance", &varP1, &varRet);
    CComDispatchDriver dispDesk(varRet.pdispVal);
    varP1.Clear();
    varRet.Clear();
    //    var bOK=new Boolean(true);

    //    var noArgs=new Array();
    //    var oDoc=starDesktop.loadComponentFromURL("private:factory/swriter", "Test", 40, noArgs);
    IDispatchPtr oDoc;
    SAFEARRAY* ar= SafeArrayCreateVector(VT_DISPATCH, 0, 0);
    _variant_t args[4];
    args[3]= _variant_t(L"private:factory/swriter");
    args[2]= _variant_t(L"Test");
    args[1]= _variant_t((long) 40);
    args[0].vt= VT_ARRAY | VT_DISPATCH;;
    args[0].parray= ar;
    hr= dispDesk.InvokeN(L"loadComponentFromURL", args, 4, &varRet);
    CComDispatchDriver dispDoc(varRet.pdispVal);
    varRet.Clear();

    //var oFieldMaster = oDoc.createInstance("com.sun.star.text.FieldMaster.Database");
    varP1= _variant_t(L"com.sun.star.text.FieldMaster.Database");
    hr= dispDoc.Invoke1(L"createInstance", &varP1, &varRet);
    CComDispatchDriver dispFieldMaster(varRet.pdispVal);
    varP1.Clear();
    varRet.Clear();

    //var oObj = oDoc.createInstance("com.sun.star.text.TextField.Database");
    varP1= _variant_t(L"com.sun.star.text.TextField.Database");
    hr= dispDoc.Invoke1(L"createInstance", &varP1, &varRet);
    CComDispatchDriver dispField(varRet.pdispVal);
    varP1.Clear();
    varRet.Clear();

    //oObj.attachTextFieldMaster(oFieldMaster);
    varP1= _variant_t(dispFieldMaster);
    hr= dispField.Invoke1(L"attachTextFieldMaster", &varP1);


    return S_OK;

}
