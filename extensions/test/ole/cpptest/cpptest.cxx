/*************************************************************************
 *
 *  $RCSfile: cpptest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:04:53 $
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
