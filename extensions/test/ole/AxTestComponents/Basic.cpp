/*************************************************************************
 *
 *  $RCSfile: Basic.cpp,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2000-10-12 13:18:22 $
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
    *val= 3.14f;
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
    if( !pVal)
        return E_POINTER;
    *pVal= m_cPrpByte;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpByte(unsigned char newVal)
{
    m_cPrpByte= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpShort(short *pVal)
{
    if( !pVal)
        return E_POINTER;
    *pVal= m_nPrpShort;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpShort(short newVal)
{
    m_nPrpShort= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpLong(long *pVal)
{
    if( !pVal)
        return E_POINTER;
    *pVal= m_lPrpLong;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpLong(long newVal)
{
    m_lPrpLong= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpString(BSTR *pVal)
{
    if( !pVal)
        return E_POINTER;
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
    if( !pVal)
        return E_POINTER;
    *pVal= m_fPrpFloat;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpFloat(float newVal)
{
    m_fPrpFloat= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpDouble(double *pVal)
{
    if( !pVal)
        return E_POINTER;
    *pVal= m_dPrpDouble;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpDouble(double newVal)
{
    m_dPrpDouble= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpVariant(VARIANT *pVal)
{
    if( !pVal)
        return E_POINTER;
    VariantCopy( pVal, &m_PropVariant);
    return S_OK;
}

STDMETHODIMP CBasic::put_prpVariant(VARIANT newVal)
{
    m_PropVariant= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpArray(LPSAFEARRAY *pVal)
{
    if( !pVal)
        return E_POINTER;
    SafeArrayCopy( m_PrpArray, pVal);
    return S_OK;
}

STDMETHODIMP CBasic::put_prpArray(LPSAFEARRAY newVal)
{
    SafeArrayDestroy( m_PrpArray);
    SafeArrayCopy( newVal, &m_PrpArray);

    return S_OK;
}

STDMETHODIMP CBasic::get_prpObject(IDispatch **pVal)
{
    if( !pVal)
        return E_POINTER;
    *pVal= m_PrpObject;
    (*pVal)->AddRef();
    return S_OK;
}

STDMETHODIMP CBasic::put_prpObject(IDispatch *newVal)
{
    m_PrpObject= newVal;
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
    printArray( val, CComBSTR(L"inSequenceLong\n"), VT_I4);
    return S_OK;
}

STDMETHODIMP CBasic::inSequenceByte( LPSAFEARRAY val)
{
    printArray( val, CComBSTR(L"inSequenceByte\n"), VT_UI1);
    return S_OK;
}

STDMETHODIMP CBasic::inSequenceShort(LPSAFEARRAY val)
{
    printArray( val, CComBSTR(L"inSequenceShort\n"), VT_I2);
    return S_OK;
}

STDMETHODIMP CBasic::inSequenceString(LPSAFEARRAY val)
{
    printArray( val, CComBSTR(L"inSequenceString\n"), VT_BSTR);
    return S_OK;

}

STDMETHODIMP CBasic::inSequenceFloat(LPSAFEARRAY val)
{
    printArray( val, CComBSTR(L"inSequenceFloat\n"), VT_R4);
    return S_OK;
}

STDMETHODIMP CBasic::inSequenceDouble(LPSAFEARRAY val)
{
    printArray( val, CComBSTR(L"inSequenceDouble\n"), VT_R8);
    return S_OK;
}

STDMETHODIMP CBasic::inSequenceObject(LPSAFEARRAY val)
{
    printArray( val, CComBSTR(L"inSequenceObject\n"), VT_DISPATCH);
    return S_OK;
}

void CBasic::printArray( LPSAFEARRAY val, BSTR message, VARTYPE type)
{

    HRESULT hr= S_OK;
    USES_CONVERSION;
    long lbound=0;
    long ubound= 0;
    hr= SafeArrayGetLBound( val, 1, &lbound);
    hr= SafeArrayGetUBound( val, 1, &ubound);
    long length= ubound - lbound +1;

    CComVariant varElement;
    char buf[1024];
    sprintf( buf,"%s", W2A(message));

    for( long i= 0; i < length ; i++)
    {
        char tmp[1024];
        long data=0;
        CComVariant var;
        switch( type)
        {
        case VT_UI1:
        case VT_I2:
        case VT_I4:
        case VT_ERROR:
            hr= SafeArrayGetElement( val, &i, (void*)&data);
            sprintf( tmp, "%d \n", *(long*)&data);
            break;
        case VT_BSTR:
            hr= SafeArrayGetElement( val, &i, (void*)&data);
            sprintf( tmp, "%S \n", (BSTR)data);
            break;
        case VT_VARIANT:
            hr= SafeArrayGetElement( val, &i, &var);
            sprintf( tmp, "%x \n", var.byref);
            break;
        case VT_R4:
            hr= SafeArrayGetElement( val, &i, (void*)&data);
            sprintf( tmp, "%f \n", *(float*) &data);
            break;
        case VT_R8: ;
            hr= SafeArrayGetElement( val, &i, (void*)&data);
            sprintf( tmp, "%f \n", *(double*) &data);
            break;
        case VT_DISPATCH:
            // we assume the objects are instances of this component and have the
            // property prpString set.
            hr= SafeArrayGetElement( val, &i, (void*)&data);
            IDispatch* pdisp= ( IDispatch*) data;
            CComDispatchDriver driver( pdisp);
            CComVariant var;
            if( pdisp)
            {
                driver.GetPropertyByName(L"prpString", &var);
                sprintf( tmp, "%x : %S \n", *(long*)&data, var.bstrVal);
            }
            else
                sprintf( tmp, "%x\n", *(long*)&data);
        }

        strcat( buf, tmp);
    }
    MessageBox( NULL, _T(A2T(buf)), _T("AxTestComponents.Basic"), MB_OK);

}
// V_ERROR OLECHAR VARIANT VT_UI1

STDMETHODIMP CBasic::outSequenceByte(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;

    SAFEARRAY* ar=  SafeArrayCreateVector( VT_UI1, 0, 3);
    for( long i=0; i< 3; i++)
    {
        unsigned char charVal= i +1;
        hr= SafeArrayPutElement( ar, &i, &charVal);
    }

    *val= ar;
    return hr;
}

STDMETHODIMP CBasic::outSequenceShort(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;

    SAFEARRAY* ar=  SafeArrayCreateVector( VT_I2, 0, 3);
    for( long i=0; i< 3; i++)
    {
        short shortVal= i +1;
        hr= SafeArrayPutElement( ar, &i, &shortVal);
    }

    *val= ar;
    return hr;
}

STDMETHODIMP CBasic::outSequenceLong(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;

    SAFEARRAY* ar=  SafeArrayCreateVector( VT_I4, 0, 3);
    for( long i=0; i< 3; i++)
    {
        long longVal= i +1;
        hr= SafeArrayPutElement( ar, &i, &longVal);
    }

    *val= ar;
    return hr;
}

STDMETHODIMP CBasic::outSequenceString(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;

    SAFEARRAY* ar=  SafeArrayCreateVector( VT_BSTR, 0, 3);
    BSTR strings[3];
    strings[0]= SysAllocString(L"string 1");
    strings[1]= SysAllocString(L"string 2");
    strings[2]= SysAllocString(L"string 3");
    for( long i=0; i< 3; i++)
    {
        hr= SafeArrayPutElement( ar, &i, strings[i]);
    }

    *val= ar;
    return hr;
}

STDMETHODIMP CBasic::outSequenceFloat(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;

    SAFEARRAY* ar=  SafeArrayCreateVector( VT_R4, 0, 3);
    float arfloats[]= { 3.14f, 31.4f, 314.f};
    for( long i=0; i< 3; i++)
    {
        hr= SafeArrayPutElement( ar, &i, &arfloats[i]);
    }

    *val= ar;
    return hr;
}

STDMETHODIMP CBasic::outSequenceDouble(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;

    SAFEARRAY* ar=  SafeArrayCreateVector( VT_R8, 0, 3);
    double arDouble[]= { 3.14f, 31.4f, 314.f};
    for( long i=0; i< 3; i++)
    {
        hr= SafeArrayPutElement( ar, &i, &arDouble[i]);
    }
    *val= ar;
    return hr;
}

STDMETHODIMP CBasic::outSequenceObject(LPSAFEARRAY* val)
{
    CComPtr<IDispatch> ob1;
    CComPtr<IDispatch> ob2;
    CComPtr<IDispatch> ob3;

    ob1.CoCreateInstance(L"AxTestComponents.Basic");
    ob2.CoCreateInstance(L"AxTestComponents.Basic");
    ob3.CoCreateInstance(L"AxTestComponents.Basic");

    CComDispatchDriver disp( ob1);
    CComVariant param;
    param= L"this property prpString (1)";
    disp.PutPropertyByName(L"prpString", &param);

    disp= ob2;
    param= L"this property prpString (2)";
    disp.PutPropertyByName(L"prpString", &param);

    disp= ob3;
    param= L"this property prpString (3)";
    disp.PutPropertyByName(L"prpString", &param);

    SAFEARRAY* ar=  SafeArrayCreateVector( VT_DISPATCH, 0, 3);
    long i= 0;
    SafeArrayPutElement( ar, &i, ob1.p);
    i++;
    SafeArrayPutElement( ar, &i, ob2.p);
    i++;
    SafeArrayPutElement( ar, &i, ob3.p);

    *val= ar;
    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceByte(LPSAFEARRAY* val)
{
    inSequenceByte( *val);
    SafeArrayDestroy( *val);
    outSequenceByte( val);
    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceShort(LPSAFEARRAY* val)
{
    inSequenceShort( *val);
    SafeArrayDestroy( *val);
    outSequenceShort( val);

    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceLong(LPSAFEARRAY* val)
{
    inSequenceLong( *val);
    SafeArrayDestroy( *val);
    outSequenceLong( val);

    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceString(LPSAFEARRAY* val)
{
    inSequenceString( *val);
    SafeArrayDestroy( *val);
    outSequenceString( val);

    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceFloat(LPSAFEARRAY* val)
{
    inSequenceFloat( *val);
    SafeArrayDestroy( *val);
    outSequenceFloat( val);

    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceDouble(LPSAFEARRAY* val)
{
    inSequenceDouble( *val);
    SafeArrayDestroy( *val);
    outSequenceDouble( val);

    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceObject(LPSAFEARRAY* val)
{
    inSequenceObject( *val);
    SafeArrayDestroy( *val);
    outSequenceObject( val);

    return S_OK;
}

// 2-dimensional Array
STDMETHODIMP CBasic::inMulDimArrayLong(LPSAFEARRAY val)
{
    printMulArray( val, VT_I4);
    return S_OK;
}
// 2-dimensional Array
STDMETHODIMP CBasic::inMulDimArrayVariant(LPSAFEARRAY val)
{
    printMulArray( val, VT_VARIANT);
    return S_OK;
}
// 3-dimensional Array
STDMETHODIMP CBasic::inMulDimArrayLong2(LPSAFEARRAY val)
{
    printMulArray( val, VT_I4);
    return S_OK;
}
// 3-dimensional Array
STDMETHODIMP CBasic::inMulDimArrayVariant2(LPSAFEARRAY val)
{
    return S_OK;
}


STDMETHODIMP CBasic::inMulDimArrayByte(LPSAFEARRAY val)
{
    // TODO: Add your implementation code here

    return S_OK;
}
// 3-dimensionales array
STDMETHODIMP CBasic::inMulDimArrayByte2(LPSAFEARRAY val)
{
    // TODO: Add your implementation code here
    printMulArray( val, VT_UI1);
    return S_OK;
}

// supports 2 and 3 dimensionals SAFEARRAY with elements of long or VARIANT
void CBasic::printMulArray( SAFEARRAY* val, VARTYPE type)
{
    HRESULT hr= S_OK;
    UINT dims= SafeArrayGetDim( val);
    long lbound1;
    long ubound1;
    long lbound2;
    long ubound2;
    long lbound3;
    long ubound3;
    long length1;
    long length2;
    long length3;

    char buff[4096];
    buff[0]=0;

    if( dims == 2)
    {
        hr= SafeArrayGetLBound( val, 1, &lbound1);
        hr= SafeArrayGetUBound( val, 1, &ubound1);
        length1= ubound1 - lbound1 +1;

        hr= SafeArrayGetLBound( val, 2, &lbound2);
        hr= SafeArrayGetUBound( val, 2, &ubound2);
        length2= ubound2 - lbound2 + 1;
        char tmpBuf[1024];
        tmpBuf[0]=0;
        long index[2];
        for( long i= 0; i< length2; i++)
        {
            for( long j= 0; j<length1; j++)
            {
                index[0]= j;
                index[1]= i;
                long longVal;
                CComVariant var;
                switch( type)
                {
                case VT_I4:
                    hr= SafeArrayGetElement( val, index, &longVal);
                    sprintf( tmpBuf, "(%d,%d): %d\n", index[1], index[0], longVal);
                    break;
                case VT_UI1:
                    hr= SafeArrayGetElement( val, index, &longVal);
                    sprintf( tmpBuf, "(%d,%d): %d\n", index[1], index[0], (unsigned char)longVal);
                    break;
                case VT_VARIANT:
                    hr= SafeArrayGetElement( val, index, &var );
                    sprintf( tmpBuf, "(%d,%d):  %d (vartype %d)\n",  index[1], index[0], var.byref, var.vt);
                    break;
                }
                strcat( buff,tmpBuf);
            }

        }


    }
    else if( dims == 3 )
    {
        hr= SafeArrayGetLBound( val, 1, &lbound1);
        hr= SafeArrayGetUBound( val, 1, &ubound1);
        length1= ubound1 - lbound1 +1;

        hr= SafeArrayGetLBound( val, 2, &lbound2);
        hr= SafeArrayGetUBound( val, 2, &ubound2);
        length2= ubound2 - lbound2 + 1;

        hr= SafeArrayGetLBound( val, 3, &lbound3);
        hr= SafeArrayGetUBound( val, 3, &ubound3);
        length3= ubound3 - lbound3 +1;
        char tmpBuf[1024];
        tmpBuf[0]=0;
        long index[3];
        for( long i= 0; i< length3; i++)
        {
            for( long j= 0; j<length2; j++)
            {
                for( long k= 0; k<length1; k++)
                {
                    index[0]= k;
                    index[1]= j;
                    index[2]= i;
                    long longVal;
                    CComVariant var;
                    switch( type)
                    {
                    case VT_I4:
                        hr= SafeArrayGetElement( val, index, &longVal);
                        sprintf( tmpBuf, "(%d,%d,%d): %d\n", index[2], index[1], index[0], longVal);
                        break;
                    case VT_UI1:
                        hr= SafeArrayGetElement( val, index, &longVal);
                        sprintf( tmpBuf, "(%d,%d,%d): %d\n", index[2], index[1], index[0], (unsigned char)longVal);
                        break;

                    case VT_VARIANT:
                        hr= SafeArrayGetElement( val, index, &var );
                        sprintf( tmpBuf, "(%d,%d,%d):  %d (vartype %d)\n", index[2],  index[1], index[0], var.byref, var.vt);
                        break;
                    }
                    strcat( buff,tmpBuf);
                }
            }

        }

    }

    MessageBox( NULL, A2T( buff), _T("AxTestControl.Basic"), MB_OK);


}



