/*************************************************************************
 *
 *  $RCSfile: Basic.cpp,v $
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
#include "stdafx.h"
#include "AxTestComponents.h"
#include "Basic.h"

/////////////////////////////////////////////////////////////////////////////
// CBasic


STDMETHODIMP CBasic::inByte(unsigned char val)
{
    USES_CONVERSION;
    char buf[256];
    sprintf( buf, "inByte: value= %d", val);
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);
    return S_OK;
}

STDMETHODIMP CBasic::inShort(short val)
{
    USES_CONVERSION;
    char buf[256];
    sprintf( buf, "inByte: value= %d", val);
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);

    return S_OK;
}

STDMETHODIMP CBasic::inLong(long val)
{
    USES_CONVERSION;
    char buf[256];
    sprintf( buf, "inLong: value= %d", val);
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);
    return S_OK;
}

STDMETHODIMP CBasic::inString(BSTR val)
{
    USES_CONVERSION;
    char buf[256];
    sprintf( buf, "inString: value= %S", val);
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);

    return S_OK;
}

STDMETHODIMP CBasic::inFloat(float val)
{
    USES_CONVERSION;
    char buf[256];
    sprintf( buf, "inFloat: value= %f", val);
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);
    return S_OK;
}

STDMETHODIMP CBasic::inDouble(double val)
{
    USES_CONVERSION;
    char buf[256];
    sprintf( buf, "inDouble: value= %g", val);
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);
    return S_OK;
}

STDMETHODIMP CBasic::inVariant(VARIANT val)
{
    USES_CONVERSION;
    CComVariant var;
    VariantCopyInd( &var, &val);
    if( var.vt == VT_BSTR)
    {
        char buf[256];
        sprintf( buf, "inVariant: value= %S", var.bstrVal);
        MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);
    }

    return S_OK;
}

STDMETHODIMP CBasic::inArray(LPSAFEARRAY val)
{
    HRESULT hr= S_OK;
    long lbound= 0;

    long ubound= 0;
    hr= SafeArrayGetLBound( val, 1, &lbound);
    hr= SafeArrayGetUBound( val, 1, &ubound);
    long length= ubound - lbound +1;

    CComVariant varElement;
    char buf[4096];
    ZeroMemory( buf, 10);
    for( long i= 0; i < length ; i++)
    {
        varElement.Clear();
        hr= SafeArrayGetElement( val, &i, &varElement);

        if( varElement.vt == VT_BSTR)
        {   char bufTmp[256];
            sprintf( bufTmp, " %d  string: = %S \n", i, varElement.bstrVal);
            strcat( buf, bufTmp);
        }
    }
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);
    return S_OK;
}

STDMETHODIMP CBasic::inObject(IDispatch *val)
{
    // We expect the object to be an AxTestComponent.Basic object
    // with the property prpString set.
    HRESULT hr= S_OK;
    USES_CONVERSION;
    char buf[256];
    CComDispatchDriver disp( val);
    CComBSTR bstr;
    if( disp)
    {
        CComVariant var;
        hr= disp.GetPropertyByName(L"prpString", &var);
        if( var.vt== VT_BSTR)
            bstr= var.bstrVal;
    }
    sprintf( buf, "inObject: value= %S", bstr.m_str);
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);

    return S_OK;
}



STDMETHODIMP CBasic::inoutByte(unsigned char* val)
{
    *val+= 1;
    return S_OK;
}

STDMETHODIMP CBasic::inoutShort(short *val)
{
    *val+= 1;

    return S_OK;
}

STDMETHODIMP CBasic::inoutLong(long *val)
{
    *val+= 1;

    return S_OK;
}

STDMETHODIMP CBasic::inoutString(BSTR *val)
{
    MessageBoxW( NULL, *val, L"AxTestComponents.Basic", MB_OK);
    return S_OK;
}

STDMETHODIMP CBasic::inoutFloat(float *val)
{
    *val+= 1;

    return S_OK;
}

STDMETHODIMP CBasic::inoutDouble(double *val)
{
    *val+= 1;

    return S_OK;
}

STDMETHODIMP CBasic::inoutVariant(VARIANT *val)
{
    if( val->vt & VT_BSTR)
    {
        MessageBoxW( NULL, val->bstrVal, L"AxTestComponents.Basic", MB_OK);
        SysFreeString( val->bstrVal);
        val->bstrVal= SysAllocString(L" a string from AxTestComponents.Basic");
    }
    return S_OK;
}

STDMETHODIMP CBasic::inoutArray(LPSAFEARRAY *val)
{
    inArray(*val);
    SafeArrayDestroy(*val);
    outArray( val);
    return S_OK;
}

STDMETHODIMP CBasic::inoutObject(IDispatch **val)
{
    inObject( *val);
    outObject( val);
    return S_OK;
}



STDMETHODIMP CBasic::outByte(unsigned char *val)
{
    *val= 111;
    return S_OK;
}

STDMETHODIMP CBasic::outShort(short *val)
{
    *val= 1111;
    return S_OK;
}

STDMETHODIMP CBasic::outLong(long *val)
{
    *val= 111111;
    return S_OK;
}

STDMETHODIMP CBasic::outString(BSTR *val)
{
    *val= SysAllocString(L" a string as out value");
    return S_OK;
}

STDMETHODIMP CBasic::outFloat(float *val)
{
    *val= 3.14;
    return S_OK;
}

STDMETHODIMP CBasic::outDouble(double *val)
{
    *val= 3.145;
    return S_OK;
}

STDMETHODIMP CBasic::outVariant(VARIANT *val)
{
    val->vt = VT_BSTR;
    val->bstrVal= SysAllocString(L"This is a string in a VARIANT");
    return S_OK;
}

STDMETHODIMP CBasic::outArray(LPSAFEARRAY *val)
{
    HRESULT hr= S_OK;
    SAFEARRAY* ar= SafeArrayCreateVector( VT_VARIANT, 0, 3);
    CComVariant arStrings[3]; //BSTR arStrings[3];
    arStrings[0].bstrVal= SysAllocString(L" out string 0");
    arStrings[0].vt= VT_BSTR;
    arStrings[1].bstrVal= SysAllocString( L"out string 1");
    arStrings[1].vt= VT_BSTR;
    arStrings[2].bstrVal= SysAllocString( L" outstring 2");
    arStrings[2].vt= VT_BSTR;
    for( long i= 0; i < 3; i++)
    {
        SafeArrayPutElement( ar, &i, (void*) &arStrings[i]);
    }
    *val= ar;
    return S_OK;
}

STDMETHODIMP CBasic::outObject(IDispatch* *val)
{
    HRESULT hr= S_OK;
    CComPtr< IUnknown > spUnk;
    hr= spUnk.CoCreateInstance(L"AxTestComponents.Basic");
    CComDispatchDriver disp( spUnk);
    CComVariant varVal( L"This is the property prp String");
    hr= disp.PutPropertyByName( L"prpString", &varVal);
    *val= disp;
    (*val)->AddRef();
    return S_OK;
}




STDMETHODIMP CBasic::get_prpByte(unsigned char *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpByte(unsigned char newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::get_prpShort(short *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpShort(short newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::get_prpLong(long *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpLong(long newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::get_prpString(BSTR *pVal)
{
    m_bstrPrpString.CopyTo( pVal );
    return S_OK;
}

STDMETHODIMP CBasic::put_prpString(BSTR newVal)
{
    m_bstrPrpString= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpFloat(float *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpFloat(float newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::get_prpDouble(double *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpDouble(double newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::get_prpVariant(VARIANT *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpVariant(VARIANT newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::get_prpArray(LPSAFEARRAY *pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpArray(LPSAFEARRAY newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::get_prpObject(IDispatch **pVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::put_prpObject(IDispatch *newVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CBasic::mixed1(
            /* [in] */ unsigned char aChar,
            /* [out] */ unsigned char __RPC_FAR *outChar,
            /* [out][in] */ unsigned char __RPC_FAR *inoutChar,
            /* [in] */ VARIANT var,
            /* [out] */ VARIANT __RPC_FAR *outVar,
            /* [out][in] */ VARIANT __RPC_FAR *inoutVar,
            /* [in] */ SAFEARRAY __RPC_FAR * ar,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *outAr,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *inoutAr,
            /* [in] */ IDispatch __RPC_FAR *disp,
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *outDisp,
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *inoutDisp)
{
    HRESULT hr= S_OK;
    inByte( aChar);
    outByte( outChar);
    inoutByte( inoutChar);
    inVariant( var);
    outVariant( outVar);
    inoutVariant( inoutVar);
    inArray( ar);
    outArray( outAr);
    inoutArray( inoutAr);
    inObject( disp);
    outObject( outDisp);
    inoutObject( inoutDisp);
    return hr;
}




// VT_UI1

STDMETHODIMP CBasic::inSequenceLong(LPSAFEARRAY val)
{
    HRESULT hr= S_OK;
    long lbound=0;
    long ubound= 0;
    hr= SafeArrayGetLBound( val, 1, &lbound);
    hr= SafeArrayGetUBound( val, 1, &ubound);
    long length= ubound - lbound +1;

    CComVariant varElement;
    char buf[4096];
    ZeroMemory( buf, 10);
    for( long i= 0; i < length ; i++)
    {
        varElement.Clear();
        hr= SafeArrayGetElement( val, &i, &varElement);

        if( varElement.vt == VT_BSTR)
        {   char bufTmp[256];
            sprintf( bufTmp, " %d  string: = %d \n", i, varElement.lVal);
            strcat( buf, bufTmp);
        }
    }
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);

    return S_OK;
}
