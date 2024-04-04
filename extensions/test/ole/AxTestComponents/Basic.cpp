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

#include "stdafx.h"
#include "Basic.h"



// CBasic
CBasic::CBasic():   m_cPrpByte(0),m_nPrpShort(0),m_lPrpLong(0),m_fPrpFloat(0), m_dPrpDouble(0),m_PrpArray(0),
m_safearray(NULL), m_bool(VARIANT_FALSE),
m_arByte(0), m_arShort(0), m_arLong(0), m_arString(0), m_arVariant(0), m_arFloat(0),
m_arDouble(0), m_arObject(0), m_arByteDim2(0), m_date(0.), m_scode(0)

{
    memset(&m_cy, 0, sizeof(CY));
    memset(&m_decimal, 0, sizeof(DECIMAL));
}

CBasic::~CBasic()
{
    SafeArrayDestroy(m_safearray);
    SafeArrayDestroy(m_arByte);
    SafeArrayDestroy(m_arShort);
    SafeArrayDestroy(m_arLong);
    SafeArrayDestroy(m_arString);
    SafeArrayDestroy(m_arVariant);
    SafeArrayDestroy(m_arFloat);
    SafeArrayDestroy(m_arDouble);
    SafeArrayDestroy(m_arObject);
    SafeArrayDestroy(m_arByteDim2);

}
STDMETHODIMP CBasic::inBool(VARIANT_BOOL val)
{
    m_bool = val;
    return S_OK;
}
STDMETHODIMP CBasic::inByte(unsigned char val)
{
    m_byte = val;
    return S_OK;
}

STDMETHODIMP CBasic::inShort(short val)
{
    m_short = val;
    return S_OK;
}

STDMETHODIMP CBasic::inLong(long val)
{
    m_long = val;
    return S_OK;
}

STDMETHODIMP CBasic::inString(BSTR val)
{
    m_bstr = val;
    return S_OK;
}

STDMETHODIMP CBasic::inFloat(float val)
{
    m_float = val;
    return S_OK;
}

STDMETHODIMP CBasic::inDouble(double val)
{
    m_double = val;

    CComVariant varDest;
    CComVariant varSource(val);
    HRESULT hr = VariantChangeType(&varDest, &varSource, 0, VT_BSTR);
    return S_OK;
}

STDMETHODIMP CBasic::inVariant(VARIANT val)
{
    m_var1 = val;
    return S_OK;
}

STDMETHODIMP CBasic::inArray(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_safearray)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, &m_safearray)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::inObject(IDispatch *val)
{
    m_obj = val;
    return S_OK;
}

STDMETHODIMP CBasic::inoutBool(VARIANT_BOOL* val)
{
    std::swap(*val, m_bool);
    return S_OK;
}


STDMETHODIMP CBasic::inoutByte(unsigned char* val)
{
    std::swap(*val, m_byte);
    return S_OK;
}

STDMETHODIMP CBasic::inoutShort(short *val)
{
    std::swap(*val, m_short);
    return S_OK;
}

STDMETHODIMP CBasic::inoutLong(long *val)
{
    std::swap(*val, m_long);
    return S_OK;
}

STDMETHODIMP CBasic::inoutString(BSTR *val)
{
    CComBSTR aStr = *val;
    HRESULT hr = S_OK;
    if (FAILED( hr = m_bstr.CopyTo(val)))
        return hr;
    m_bstr = aStr;
    return S_OK;
}

STDMETHODIMP CBasic::inoutFloat(float *val)
{
    std::swap(*val, m_float);
    return S_OK;
}

STDMETHODIMP CBasic::inoutDouble(double *val)
{
    std::swap(*val, m_double);
    return S_OK;
}

STDMETHODIMP CBasic::inoutVariant(VARIANT *val)
{
    CComVariant aVar = *val;
    HRESULT hr = S_OK;
    if (FAILED(hr = VariantCopy(val, &m_var1)))
        return hr;
    m_var1 = aVar;
    return S_OK;
}

/* The array contains VARIANT according to IDL.
    If the VARIANTs contain strings then we append "out" to each string.
*/
STDMETHODIMP CBasic::inoutArray(LPSAFEARRAY *val)
{
    SAFEARRAY* aAr = NULL;
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayCopy(*val, &aAr)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_safearray, val)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(aAr, & m_safearray)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::inoutObject(IDispatch **val)
{
    CComPtr<IDispatch> disp = *val;
    if (*val)
        (*val)->Release();
    *val = m_obj;
    if (*val)
        (*val)->AddRef();
    m_obj = disp;
    return S_OK;
}


STDMETHODIMP CBasic::outBool(VARIANT_BOOL* val)
{
    *val = m_bool;
    return S_OK;
}

STDMETHODIMP CBasic::outByte(unsigned char *val)
{
    *val= m_byte;
    return S_OK;
}

STDMETHODIMP CBasic::outShort(short *val)
{
    *val= m_short;
    return S_OK;
}

STDMETHODIMP CBasic::outLong(long *val)
{
    *val= m_long;
    return S_OK;
}

STDMETHODIMP CBasic::outString(BSTR *val)
{
    *val= SysAllocString(m_bstr);
    return S_OK;
}

STDMETHODIMP CBasic::outFloat(float *val)
{
    *val= m_float;
    return S_OK;
}

STDMETHODIMP CBasic::outDouble(double *val)
{
    *val= m_double;
    return S_OK;
}

STDMETHODIMP CBasic::outVariant(VARIANT *val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = VariantCopy(val, &m_var1)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::outArray(LPSAFEARRAY *val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayCopy(m_safearray, val)))
        return false;
    return S_OK;
}

STDMETHODIMP CBasic::outObject(IDispatch* *val)
{
    *val = m_obj;
    if (m_obj)
        (*val)->AddRef();

    return S_OK;
}


STDMETHODIMP CBasic::get_prpBool(VARIANT_BOOL* pVal)
{
    if (!pVal) return E_POINTER;
    *pVal = m_bool;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpBool(VARIANT_BOOL val)
{
    m_bool = val;
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
    HRESULT hr = S_OK;
    if (FAILED(hr = VariantCopy( pVal, &m_PropVariant)))
        return hr;
    return hr;
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
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayCopy( m_PrpArray, pVal)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::put_prpArray(LPSAFEARRAY newVal)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy( m_PrpArray)))
        return hr;
    if (FAILED(hr = SafeArrayCopy( newVal, &m_PrpArray)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::get_prpObject(IDispatch **pVal)
{
    if( !pVal)
        return E_POINTER;
    *pVal= m_PrpObject;
    if( *pVal != NULL)
        (*pVal)->AddRef();
    return S_OK;
}

STDMETHODIMP CBasic::put_prpObject(IDispatch *newVal)
{
    m_PrpObject= newVal;
    return S_OK;
}

STDMETHODIMP CBasic::mixed1(
            /* [out][in] */ unsigned char *aChar,
            /* [out][in] */ float *aFloat,
            /* [out][in] */ VARIANT *aVar)

{
    HRESULT hr= S_OK;
    inoutByte(aChar);
    inoutFloat(aFloat);
    inoutVariant(aVar);
    return hr;
}




// VT_UI1

STDMETHODIMP CBasic::inSequenceLong(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arLong)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arLong)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::inSequenceByte( LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arByte)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arByte)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::inSequenceShort(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arShort)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arShort)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::inSequenceString(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arString)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arString)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::inSequenceFloat(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arFloat)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arFloat)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::inSequenceDouble(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arDouble)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arDouble)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::inSequenceObject(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arObject)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arObject)))
        return hr;
    return hr;
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
            sprintf( tmp, "%ld \n", *(long*)&data);
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
    hr = SafeArrayCopy(m_arByte, val);
    return hr;
}

STDMETHODIMP CBasic::outSequenceShort(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;
    hr = SafeArrayCopy(m_arShort, val);
    return hr;
}

STDMETHODIMP CBasic::outSequenceLong(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;
    hr = SafeArrayCopy(m_arLong, val);
    return hr;
}

STDMETHODIMP CBasic::outSequenceString(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;
    hr = SafeArrayCopy(m_arString, val);
    return hr;
}

STDMETHODIMP CBasic::outSequenceFloat(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;
    hr = SafeArrayCopy(m_arFloat, val);
    return hr;
}

STDMETHODIMP CBasic::outSequenceDouble(LPSAFEARRAY* val)
{
    HRESULT hr= S_OK;
    hr = SafeArrayCopy(m_arDouble, val);
    return hr;
}

STDMETHODIMP CBasic::outSequenceObject(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    hr = SafeArrayCopy(m_arObject, val);
    return S_OK;
}

STDMETHODIMP CBasic::inoutSequenceByte(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    SAFEARRAY *arTemp = NULL;
    if (FAILED(hr = SafeArrayCopy(*val, &arTemp)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_arByte, val)))
        return hr;
    m_arByte = arTemp;
    return hr;
}

STDMETHODIMP CBasic::inoutSequenceShort(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    SAFEARRAY *arTemp = NULL;
    if (FAILED(hr = SafeArrayCopy(*val, &arTemp)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_arShort, val)))
        return hr;
    m_arShort = arTemp;
    return hr;
}

STDMETHODIMP CBasic::inoutSequenceLong(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    SAFEARRAY *arTemp = NULL;
    if (FAILED(hr = SafeArrayCopy(*val, &arTemp)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_arLong, val)))
        return hr;
    m_arLong = arTemp;
    return hr;
}

STDMETHODIMP CBasic::inoutSequenceString(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    SAFEARRAY *arTemp = NULL;
    if (FAILED(hr = SafeArrayCopy(*val, &arTemp)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_arString, val)))
        return hr;
    m_arString = arTemp;
    return hr;
}

STDMETHODIMP CBasic::inoutSequenceFloat(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    SAFEARRAY *arTemp = NULL;
    if (FAILED(hr = SafeArrayCopy(*val, &arTemp)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_arFloat, val)))
        return hr;
    m_arFloat = arTemp;
    return hr;
}

STDMETHODIMP CBasic::inoutSequenceDouble(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    SAFEARRAY *arTemp = NULL;
    if (FAILED(hr = SafeArrayCopy(*val, &arTemp)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_arDouble, val)))
        return hr;
    m_arDouble = arTemp;
    return hr;
}

STDMETHODIMP CBasic::inoutSequenceObject(LPSAFEARRAY* val)
{
    HRESULT hr = S_OK;
    SAFEARRAY *arTemp = NULL;
    if (FAILED(hr = SafeArrayCopy(*val, &arTemp)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(m_arObject, val)))
        return hr;
    m_arObject = arTemp;
    return hr;
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
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arByteDim2)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arByteDim2)))
        return hr;
    return hr;
}
// 3-dimensionales array
STDMETHODIMP CBasic::inMulDimArrayByte2(LPSAFEARRAY val)
{

    // TODO: Add your implementation code here
    //printMulArray( val, VT_UI1);
    return S_OK;
}

// supports 2 and 3 dimensional SAFEARRAY with elements of long or VARIANT
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
                    sprintf( tmpBuf, "(%ld,%ld): %ld\n", index[1], index[0], longVal);
                    break;
                case VT_UI1:
                    hr= SafeArrayGetElement( val, index, &longVal);
                    sprintf( tmpBuf, "(%ld,%ld): %d\n", index[1], index[0], (unsigned char)longVal);
                    break;
                case VT_VARIANT:
                    hr= SafeArrayGetElement( val, index, &var );
                    sprintf( tmpBuf, "(%ld,%ld):  %d (vartype %d)\n",  index[1], index[0], var.byref, var.vt);
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
        long length3= ubound3 - lbound3 +1;
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
                        sprintf( tmpBuf, "(%ld,%ld,%ld): %ld\n", index[2], index[1], index[0], longVal);
                        break;
                    case VT_UI1:
                        hr= SafeArrayGetElement( val, index, &longVal);
                        sprintf( tmpBuf, "(%ld,%ld,%ld): %d\n", index[2], index[1], index[0], (unsigned char)longVal);
                        break;

                    case VT_VARIANT:
                        hr= SafeArrayGetElement( val, index, &var );
                        sprintf( tmpBuf, "(%ld,%ld,%ld):  %d (vartype %d)\n", index[2],  index[1], index[0], var.byref, var.vt);
                        break;
                    }
                    strcat( buff,tmpBuf);
                }
            }

        }

    }

    MessageBox( NULL, A2T( buff), _T("AxTestControl.Basic"), MB_OK);


}




STDMETHODIMP CBasic::outMore(long* val1, long* val2)
{
    // TODO: Add your implementation code here
    *val1= 111;
    *val2= 112;
    return S_OK;
}
// If an optional parameter was not provided then the respective member will
// not be set
STDMETHODIMP CBasic::optional1(/*[in]*/ long val1, /*[in, optional]*/ VARIANT* val2)
{
    m_long = val1;
    if (val2->vt != VT_ERROR)
        m_var1 = *val2;
    return S_OK;
}

STDMETHODIMP CBasic::optional2(/*[out]*/ long* val1,/*[out, optional]*/ VARIANT* val2)
{
    HRESULT hr = S_OK;
    *val1 = m_long;

    if (val2->vt != VT_ERROR)
        hr = VariantCopy(val2, & m_var1);
    return hr;
}

STDMETHODIMP CBasic::optional3(/*[in, optional]*/ VARIANT* val1,/*[in, optional]*/ VARIANT* val2)
{
    //if (val1->vt != VT_ERROR)
        m_var1 = *val1;

    //if (val2->vt != VT_ERROR)
        m_var2 = *val2;
    return S_OK;
}

STDMETHODIMP CBasic::optional4(/*[in, out, optional]*/ VARIANT* val1,
                               /*[in, out, optional]*/ VARIANT* val2)
{
    HRESULT hr = S_OK;
    //return the previously set in values
    if (val1->vt != VT_ERROR)
    {
        CComVariant var1(*val1);
        if (FAILED(hr = VariantCopy(val1, & m_var1)))
            return hr;
        m_var1 = var1;
    }
    if (val2->vt != VT_ERROR)
    {
        CComVariant var2(*val2);
        if (FAILED(hr = VariantCopy(val2, & m_var2)))
            return hr;
        m_var2 = var2;
    }
    return hr;
}

STDMETHODIMP CBasic::optional5(/*[out, optional]*/ VARIANT* val1,
                               /*[out, optional]*/ VARIANT* val2)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = VariantCopy(val1, &m_var1)))
        return hr;
    if (FAILED(hr = VariantCopy(val2, &m_var2)))
        return hr;
    return hr;
}

STDMETHODIMP CBasic::defaultvalue1(/*[in, defaultvalue(10)]*/ long val1,
                                   /*[in, defaultvalue(3.14)]*/ double* val2,
                                //   /*[in, defaultvalue(10)]*/ VARIANT val3,
                                   /*[in, defaultvalue(100)]*/ VARIANT* val4)
{
    m_long = val1;
    m_double = *val2;
//  m_var1 = val3;
    m_var2 = *val4;
    return S_OK;
}
STDMETHODIMP CBasic::defaultvalue2(/*[in, out, defaultvalue(10)]*/ long* val1,
                                   /*[in, out, defaultvalue(3.14)]*/ double* val2,
                                //   /*[in, out, defaultvalue(10)]*/ VARIANT* val3,
                                   /*[in, out, defaultvalue(100)]*/ VARIANT* val4)
{
    HRESULT hr = S_OK;
    long aLong = *val1;
    double aDouble = *val2;
//  CComVariant var1(*val3);
    CComVariant var2(*val4);
    *val1 = m_long;
    *val2 = m_double;
    //if (FAILED(hr = VariantCopy(val3, &m_var1)))
    //  return hr;
    if (FAILED(hr = VariantCopy(val4, &m_var2)))
        return hr;
    m_long = aLong;
    m_double = aDouble;
//  m_var1 = var1;
    m_var2 = var2;
    return hr;
}
/* val2 contains the variable argument list. If no such arguments are supplied
    then the safearray is invalid. SafeArrayCopy then returns E_INVALIDARG
*/
STDMETHODIMP CBasic::varargfunc1(/*[in]*/ long val1,/*[in]*/ LPSAFEARRAY val2)
{
    m_long = val1;

    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_safearray)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val2, & m_safearray)))
    {
        if (hr != E_INVALIDARG)
            return hr;
    }
    return S_OK;
}

STDMETHODIMP CBasic::varargfunc2(/*[out]*/ long* val1, /*[out]*/ SAFEARRAY ** val2)
{
    *val1 = m_long;
    HRESULT hr = SafeArrayCopy(m_safearray, val2);
    return hr;
}

STDMETHODIMP CBasic::inSequenceByteDim2(LPSAFEARRAY val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = SafeArrayDestroy(m_arByteDim2)))
        return hr;
    if (FAILED(hr = SafeArrayCopy(val, & m_arByteDim2)))
        return hr;
    return hr;
}


STDMETHODIMP CBasic::inCurrency(CY val)
{
    m_cy = val;
    return S_OK;
}

STDMETHODIMP CBasic::outCurrency(CY* val)
{
    *val = m_cy;
    return S_OK;
}

STDMETHODIMP CBasic::inoutCurrency(CY* val)
{
    std::swap(*val, m_cy);
    return S_OK;
}

STDMETHODIMP CBasic::inDate(DATE val)
{
    m_date = val;
    return S_OK;
}

STDMETHODIMP CBasic::outDate(DATE* val)
{
    *val = m_date;
    return S_OK;
}

STDMETHODIMP CBasic::inoutDate(DATE* val)
{
    std::swap(*val, m_date);
    return S_OK;
}

STDMETHODIMP CBasic::get_prpCurrency(CY* pVal)
{
    *pVal = m_cy;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpCurrency(CY newVal)
{
    m_cy = newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpDate(DATE* pVal)
{
    *pVal = m_date;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpDate(DATE newVal)
{
    m_date = newVal;
    return S_OK;
}

//VT_I4 DECIMAL_NEG //tagVARIANT DISPATCH_PROPERTYPUT
STDMETHODIMP CBasic::inDecimal(DECIMAL val)
{
    m_decimal = val;
    return S_OK;
}

STDMETHODIMP CBasic::outDecimal(DECIMAL* val)
{
    * val = m_decimal;
    return S_OK;
}

STDMETHODIMP CBasic::inoutDecimal(DECIMAL* val)
{
    std::swap(*val, m_decimal);
    return S_OK;
}

STDMETHODIMP CBasic::get_prpDecimal(DECIMAL* pVal)
{
    * pVal = m_decimal;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpDecimal(DECIMAL newVal)
{
    m_decimal = newVal;
    return S_OK;
}

STDMETHODIMP CBasic::inSCode(SCODE val)
{
    m_scode = val;
    return S_OK;
}

STDMETHODIMP CBasic::outScode(SCODE* val)
{
    * val = m_scode;
    return S_OK;
}

STDMETHODIMP CBasic::inoutSCode(SCODE* val)
{
    std::swap(*val, m_scode);
    return S_OK;
}

STDMETHODIMP CBasic::get_prpSCode(SCODE* pVal)
{
    * pVal = m_scode;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpSCode(SCODE newVal)
{
    m_scode = newVal;
    return S_OK;
}

STDMETHODIMP CBasic::inrefLong(LONG* val)
{
    m_long = * val;
    return S_OK;
}

STDMETHODIMP CBasic::inrefVariant(VARIANT* val)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = VariantCopy( & m_var1, val)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::inrefDecimal(DECIMAL* val)
{
    m_decimal = * val;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpRefLong(long* pVal)
{
    *pVal = m_long;
    return S_OK;
}

STDMETHODIMP CBasic::putref_prpRefLong(long* newVal)
{
    m_long = * newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prprefVariant(VARIANT* pVal)
{
    HRESULT hr = S_OK;
    hr = VariantCopy(pVal, & m_var1);
    return hr;
}

STDMETHODIMP CBasic::putref_prprefVariant(VARIANT* newVal)
{
    m_var1 = * newVal;
    return S_OK;
}

STDMETHODIMP CBasic::get_prprefDecimal(DECIMAL* pVal)
{
    * pVal = m_decimal;
    return S_OK;
}

STDMETHODIMP CBasic::putref_prprefDecimal(DECIMAL* newVal)
{
    m_decimal = *newVal;
    return S_OK;
}


STDMETHODIMP CBasic::optional6(VARIANT* val1, VARIANT* val2, VARIANT* val3, VARIANT* val4)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = m_var1.Copy(val1)))
        return hr;
    if (FAILED(hr = m_var2.Copy(val2)))
        return hr;
    if (FAILED(hr = m_var3.Copy(val3)))
        return hr;
    if (FAILED(hr = m_var4.Copy(val4)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::optional7(VARIANT* val1, VARIANT* val2, VARIANT* val3, VARIANT* val4)
{
    HRESULT hr = S_OK;
    if (FAILED(hr = VariantCopy(val1, & m_var1)))
        return hr;
    if (FAILED(hr = VariantCopy(val2, & m_var2)))
        return hr;
    if (FAILED(hr = VariantCopy(val3, & m_var3)))
        return hr;
    if (FAILED(hr = VariantCopy(val4, & m_var4)))
        return hr;

    return S_OK;
}

STDMETHODIMP CBasic::get_prpMultiArg1(VARIANT* val1, VARIANT* val2, VARIANT* pVal)
{
    HRESULT hr = S_OK;
    CComVariant tmp1(*val1);
    CComVariant tmp2(*val2);

    if (FAILED(hr = VariantCopy(val1, & m_var1)))
        return hr;
    if (FAILED(hr = VariantCopy(val2, & m_var2)))
        return hr;
    m_var1 = tmp1;
    m_var2 = tmp2;
    if  (FAILED(hr = VariantCopy(pVal, & m_var3)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpMultiArg1(VARIANT* val1, VARIANT* val2, VARIANT* newVal)
{
    HRESULT hr = S_OK;
    CComVariant tmp1( * val1);
    CComVariant tmp2( * val2);

    if (FAILED(hr = VariantCopy(val1, & m_var1)))
        return hr;
    if (FAILED(hr = VariantCopy(val2, & m_var2)))
        return hr;
    m_var1 = tmp1;
    m_var2 = tmp2;

    m_var3 = *newVal;
    return S_OK;
}

// tagVARIANT DISPATCH_PROPERTYPUT DISPID_PROPERTYPUT VARIANTARG LOCALE_USER_DEFAULT

STDMETHODIMP CBasic::get_prpMultiArg2(VARIANT val1, VARIANT* pVal)
{
    HRESULT hr = S_OK;
    m_var1 =  val1;

    if (FAILED(hr = VariantCopy(pVal, & m_var2)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpMultiArg2(VARIANT val1, VARIANT newVal)
{
    m_var1 = val1;
    m_var2 = newVal;
    return S_OK;
}

// returns the values set by prpMultiArg2
STDMETHODIMP CBasic::prpMultiArg2GetValues(VARIANT* val1, VARIANT* valProperty)
{
    HRESULT hr = S_OK;
    if (FAILED(VariantCopy(val1, & m_var1)))
        return hr;
    if (FAILED(VariantCopy(valProperty, & m_var2)))
        return hr;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpMultiArg3(LONG* val1, LONG* pVal)
{
    std::swap(*val1, m_long);

    * pVal = m_long2;
    return S_OK;
}

STDMETHODIMP CBasic::put_prpMultiArg3(LONG* val1, LONG newVal)
{
    std::swap(*val1, m_long);

    m_long2 = newVal;
    return S_OK;
}

STDMETHODIMP CBasic::inUnknown(IUnknown* val)
{
    m_unknown = val;

    return S_OK;
}

STDMETHODIMP CBasic::outUnknown(IUnknown** val)
{
    m_unknown.CopyTo(val);
    return S_OK;
}

STDMETHODIMP CBasic::inoutUnknown(IUnknown** val)
{
    CComPtr<IUnknown> tmp = *val;
    m_unknown.CopyTo(val);
    m_unknown = tmp;
    return S_OK;
}

STDMETHODIMP CBasic::get_prpUnknown(IUnknown** pVal)
{
    m_prpUnknown.CopyTo(pVal);
    return S_OK;
}

STDMETHODIMP CBasic::put_prpUnknown(IUnknown* newVal)
{
    m_prpUnknown = newVal;
    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
