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
// Basic.h : Declaration of the CBasic

#ifndef __BASIC_H_
#define __BASIC_H_

#include "resource.h"
#import  "AxTestComponents.tlb" no_namespace no_implementation raw_interfaces_only named_guids


// CBasic
class ATL_NO_VTABLE CBasic :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CBasic, &CLSID_Basic>,
    public IDispatchImpl<IBasic, &IID_IBasic, &LIBID_AXTESTCOMPONENTSLib>
    {
public:
    CBasic();
    ~CBasic();

    DECLARE_REGISTRY_RESOURCEID(IDR_BASIC)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CBasic)
        COM_INTERFACE_ENTRY(IBasic)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    // IBasic
public:
    STDMETHOD(outMore)(/*[out]*/long* val1, /*[out]*/long* val2);
    STDMETHOD(inMulDimArrayByte2)(LPSAFEARRAY val);
    STDMETHOD(inMulDimArrayByte)(LPSAFEARRAY val);
    STDMETHOD(inMulDimArrayVariant2)(LPSAFEARRAY val);
    STDMETHOD(inMulDimArrayLong2)(LPSAFEARRAY val);
    STDMETHOD(inMulDimArrayVariant)(LPSAFEARRAY val);
    STDMETHOD(inMulDimArrayLong)( LPSAFEARRAY val);
    STDMETHOD(inoutSequenceObject)(LPSAFEARRAY* val);
    STDMETHOD(inoutSequenceDouble)(LPSAFEARRAY * val);
    STDMETHOD(inoutSequenceFloat)(LPSAFEARRAY * val);
    STDMETHOD(inoutSequenceString)(LPSAFEARRAY* val);
    STDMETHOD(inoutSequenceLong)(LPSAFEARRAY * val);
    STDMETHOD(inoutSequenceShort)(LPSAFEARRAY * val);
    STDMETHOD(inoutSequenceByte)(LPSAFEARRAY * val);
    STDMETHOD(outSequenceObject)(/*[out]*/ LPSAFEARRAY* val);
    STDMETHOD(outSequenceDouble)(/*[out]*/ LPSAFEARRAY* val);
    STDMETHOD(outSequenceFloat)(/*[out]*/ LPSAFEARRAY* val);
    STDMETHOD(outSequenceString)(/*[out]*/ LPSAFEARRAY* val);
    STDMETHOD(outSequenceLong)(/*[out]*/ LPSAFEARRAY* val);
    STDMETHOD(outSequenceShort)(/*[out]*/ LPSAFEARRAY* val);
    STDMETHOD(outSequenceByte)(/*[out]*/ LPSAFEARRAY* val);
    STDMETHOD(inSequenceObject)(LPSAFEARRAY ar);
    STDMETHOD(inSequenceDouble)(LPSAFEARRAY ar);
    STDMETHOD(inSequenceFloat)(LPSAFEARRAY ar);
    STDMETHOD(inSequenceString)(LPSAFEARRAY ar);
    STDMETHOD(inSequenceShort)(LPSAFEARRAY ar);
    STDMETHOD(inSequenceByte)(LPSAFEARRAY ar);
    STDMETHOD(inSequenceLong)(LPSAFEARRAY  ar);
    STDMETHOD(mixed1)(
        /* [out][in] */ unsigned char *aChar,
        /* [out][in] */ float *aFloat,
        /* [out][in] */ VARIANT *aVar);
        STDMETHOD(get_prpObject)(/*[out, retval]*/ IDispatch* *pVal);
    STDMETHOD(put_prpObject)(/*[in]*/ IDispatch* newVal);
    STDMETHOD(get_prpArray)(/*[out, retval]*/ LPSAFEARRAY *pVal);
    STDMETHOD(put_prpArray)(/*[in]*/ LPSAFEARRAY newVal);
    STDMETHOD(get_prpVariant)(/*[out, retval]*/ VARIANT *pVal);
    STDMETHOD(put_prpVariant)(/*[in]*/ VARIANT newVal);
    STDMETHOD(get_prpDouble)(/*[out, retval]*/ double *pVal);
    STDMETHOD(put_prpDouble)(/*[in]*/ double newVal);
    STDMETHOD(get_prpFloat)(/*[out, retval]*/ float *pVal);
    STDMETHOD(put_prpFloat)(/*[in]*/ float newVal);
    STDMETHOD(get_prpString)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_prpString)(/*[in]*/ BSTR newVal);
    STDMETHOD(get_prpLong)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_prpLong)(/*[in]*/ long newVal);
    STDMETHOD(get_prpShort)(/*[out, retval]*/ short *pVal);
    STDMETHOD(put_prpShort)(/*[in]*/ short newVal);
    STDMETHOD(get_prpByte)(/*[out, retval]*/ unsigned char *pVal);
    STDMETHOD(put_prpByte)(/*[in]*/ unsigned char newVal);
    STDMETHOD(get_prpBool)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_prpBool)(/*[in]*/ VARIANT_BOOL newVal);

    STDMETHOD(outObject)(/*[out]*/ IDispatch* *val);
    STDMETHOD(outArray)(/*[out]*/ LPSAFEARRAY * val);
    STDMETHOD(outVariant)(/*[out]*/ VARIANT* val);
    STDMETHOD(outDouble)(/*[out]*/ double* val);
    STDMETHOD(outFloat)(/*[out]*/ float* val);
    STDMETHOD(outString)(/*[out]*/ BSTR* val);
    STDMETHOD(outLong)(/*[out]*/ long* val);
    STDMETHOD(outShort)(/*[out]*/ short* val);
    STDMETHOD(outByte)(/*[out]*/ unsigned char* val);
    STDMETHOD(outBool)(/*[out]*/ VARIANT_BOOL* val);

    STDMETHOD(inoutObject)(/*[in,out]*/ IDispatch* *val);
    STDMETHOD(inoutArray)(/*[in,out]*/ LPSAFEARRAY * val);
    STDMETHOD(inoutVariant)(/*[in,out]*/ VARIANT * val);
    STDMETHOD(inoutDouble)(/*[in,out]*/ double * val);
    STDMETHOD(inoutFloat)(/*[in,out]*/ float * val);
    STDMETHOD(inoutString)(/*[in, out]*/ BSTR* val);
    STDMETHOD(inoutLong)(/*[in,out]*/ long * val);
    STDMETHOD(inoutShort)(/*[in,out]*/ short* val);
    STDMETHOD(inoutByte)(/*[in,out]*/ unsigned char* val);
    STDMETHOD(inoutBool)(/*[in,out]*/ VARIANT_BOOL* val);


    STDMETHOD(inObject)(/*[in]*/ IDispatch* val);
    STDMETHOD(inArray)(/*[in]*/ LPSAFEARRAY val);
    STDMETHOD(inVariant)(/*[in]*/ VARIANT val);
    STDMETHOD(inDouble)(/*[in]*/ double val);
    STDMETHOD(inFloat)(/*[in]*/ float val);
    STDMETHOD(inString)(/*[in]*/ BSTR val);
    STDMETHOD(inLong)(/*[in]*/ long val);
    STDMETHOD(inShort)(/*[in]*/ short val);
    STDMETHOD(inByte)(/*[in]*/ unsigned char val);
    STDMETHOD(inBool)(/*[in]*/ VARIANT_BOOL val);


    STDMETHOD(optional1)(/*[in]*/ long val1, /*[in, optional]*/ VARIANT* val2);
    STDMETHOD(optional2)(/*[out]*/ long* val1,/*[out, optional]*/ VARIANT* val2);
    STDMETHOD(optional3)(/*[in, optional]*/ VARIANT* val1,/*[in, optional]*/ VARIANT* val2);
    STDMETHOD(optional4)(/*[in, out, optional]*/ VARIANT* val1,/*[in, out, optional]*/ VARIANT* val2);
    STDMETHOD(optional5)(/*[out, optional]*/ VARIANT* val1,/*[out, optional]*/ VARIANT* val2);


    STDMETHOD(defaultvalue1)(/*[in, defaultvalue(10)]*/ long val1,
        /*[in, defaultvalue(3.14)]*/ double* val2,
        /*[in, defaultvalue(100)]*/ VARIANT* val4);

        STDMETHOD(defaultvalue2)(/*[in, out, defaultvalue(10)]*/ long* val1,
        /*[in, out, defaultvalue(3.14)]*/ double* val2,
        /*[in, out, defaultvalue(100)]*/ VARIANT* val4);

        STDMETHOD(varargfunc1)(/*[in]*/ long val1,/*[in]*/ LPSAFEARRAY val2);

    STDMETHOD(varargfunc2)(/*[out]*/ long* val1, /*[out]*/ SAFEARRAY ** val2);


    // members for property implementations
    unsigned char m_cPrpByte;
    short m_nPrpShort;
    long m_lPrpLong;
    float m_fPrpFloat;
    double m_dPrpDouble;
    CComPtr<IDispatch> m_PrpObject;
    CComPtr<IUnknown> m_prpUnknown;

    CComBSTR m_bstrPrpString;
    CComVariant m_PropVariant;
    LPSAFEARRAY m_PrpArray;
protected:
    VARIANT_BOOL m_bool;
    unsigned char  m_byte;
    short m_short;
    long m_long;
    long m_long2;
    float m_float;
    double m_double;
    CComVariant m_var1;
    CComVariant m_var2;
    CComVariant m_var3;
    CComVariant m_var4;
    CComBSTR m_bstr;
    CY m_cy;
    DATE m_date;
    DECIMAL m_decimal;
    SCODE m_scode;
    SAFEARRAY * m_safearray;
    CComPtr<IDispatch> m_obj;
    CComPtr<IUnknown> m_unknown;

    SAFEARRAY * m_arByte;
    SAFEARRAY * m_arShort;
    SAFEARRAY * m_arLong;
    SAFEARRAY * m_arString;
    SAFEARRAY * m_arVariant;
    SAFEARRAY * m_arFloat;
    SAFEARRAY * m_arDouble;
    SAFEARRAY * m_arObject;
    SAFEARRAY * m_arByteDim2;

    static void printArray(LPSAFEARRAY val, BSTR message, VARTYPE type);
    static void printMulArray(LPSAFEARRAY val, VARTYPE type);


public:
    STDMETHOD(inSequenceByteDim2)(LPSAFEARRAY val);
    STDMETHOD(inCurrency)(CY val);
    STDMETHOD(outCurrency)(CY* val);
    STDMETHOD(inoutCurrency)(CY* val);
    STDMETHOD(inDate)(DATE val);
    STDMETHOD(outDate)(DATE* val);
    STDMETHOD(inoutDate)(DATE* val);
    STDMETHOD(get_prpCurrency)(CY* pVal);
    STDMETHOD(put_prpCurrency)(CY newVal);
    STDMETHOD(get_prpDate)(DATE* pVal);
    STDMETHOD(put_prpDate)(DATE newVal);
    STDMETHOD(inDecimal)(DECIMAL val);
    STDMETHOD(outDecimal)(DECIMAL* val);
    STDMETHOD(inoutDecimal)(DECIMAL* val);
    STDMETHOD(get_prpDecimal)(DECIMAL* pVal);
    STDMETHOD(put_prpDecimal)(DECIMAL newVal);
    STDMETHOD(inSCode)(SCODE val);
    STDMETHOD(outScode)(SCODE* val);
    STDMETHOD(inoutSCode)(SCODE* val);
    STDMETHOD(get_prpSCode)(SCODE* pVal);
    STDMETHOD(put_prpSCode)(SCODE newVal);
    STDMETHOD(inrefLong)(LONG* val);
    STDMETHOD(inrefVariant)(VARIANT* val);
    STDMETHOD(inrefDecimal)(DECIMAL* val);
    STDMETHOD(get_prpRefLong)(long* pVal);
    STDMETHOD(putref_prpRefLong)(long* newVal);
    STDMETHOD(get_prprefVariant)(VARIANT* pVal);
    STDMETHOD(putref_prprefVariant)(VARIANT* newVal);
    STDMETHOD(get_prprefDecimal)(DECIMAL* pVal);
    STDMETHOD(putref_prprefDecimal)(DECIMAL* newVal);
    STDMETHOD(optional6)(VARIANT* val1, VARIANT* val2, VARIANT* val3, VARIANT* val4);
    STDMETHOD(optional7)(VARIANT* val1, VARIANT* val2, VARIANT* val3, VARIANT* val4);

    STDMETHOD(get_prpMultiArg1)(VARIANT* val1, VARIANT* val2, VARIANT* pVal);
    //STDMETHOD(get_prpMultiArg1)(VARIANT* val1, VARIANT* pVal);
    STDMETHOD(put_prpMultiArg1)(VARIANT* val1, VARIANT* val2, VARIANT* newVal);
    //STDMETHOD(put_prpMultiArg1)(VARIANT* val1, VARIANT* newVal);
    STDMETHOD(get_prpMultiArg2)(VARIANT val1, VARIANT* pVal);
    STDMETHOD(put_prpMultiArg2)(VARIANT val1, VARIANT newVal);
    STDMETHOD(prpMultiArg2GetValues)(VARIANT* val1, VARIANT* valProperty);
    STDMETHOD(get_prpMultiArg3)(LONG* val1, LONG* pVal);
    STDMETHOD(put_prpMultiArg3)(LONG* val1, LONG newVal);

    // IFoo Methods
public:
    STDMETHOD(inUnknown)(IUnknown* val);
    STDMETHOD(outUnknown)(IUnknown** val);
    STDMETHOD(inoutUnknown)(IUnknown** val);
    STDMETHOD(get_prpUnknown)(IUnknown** pVal);
    STDMETHOD(put_prpUnknown)(IUnknown* newVal);
};

#endif //__BASIC_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
