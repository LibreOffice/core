/*************************************************************************
 *
 *  $RCSfile: Callback.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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
// Callback.h : Declaration of the CCallback

#ifndef __CALLBACK_H_
#define __CALLBACK_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CCallback
class ATL_NO_VTABLE CCallback :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CCallback, &CLSID_Callback>,
    public IDispatchImpl<ICallback, &IID_ICallback, &LIBID_XCALLBACK_IMPLLib>
{
public:
    CCallback()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CALLBACK)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCallback)
    COM_INTERFACE_ENTRY(ICallback)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ICallback
public:
    STDMETHOD(inSeqByte)(/*[in]*/ LPSAFEARRAY val);
    STDMETHOD(outSeqByte)(/*[out]*/ LPSAFEARRAY* outVal);
    STDMETHOD(inValues)(/*[in]*/short aChar, /*[in]*/ long aLong, /*[in]*/ BSTR aString);
    STDMETHOD(inoutLong)(/*[in,out]*/ long* inoutVal);
    STDMETHOD(inoutShort)(/*[in,out]*/ short* inoutVal);
    STDMETHOD(inoutByte)(/*[in,out]*/ unsigned char* inoutVal);
    STDMETHOD(inoutDouble)(/*[in,out]*/ double* inoutVal);
    STDMETHOD(inoutFloat)(/*[in,out]*/ float* inoutVal);
    STDMETHOD(inoutString)(/*[in,out]*/ BSTR *inoutVal);
    STDMETHOD(inoutChar)(/*[in,out]*/ short* inoutVal);
    STDMETHOD(inoutBool)(/*[in,out]*/ VARIANT_BOOL * inoutVal);
    STDMETHOD(inoutAny)(/*[in,out]*/ VARIANT* inoutVal);
    STDMETHOD(inoutSeqAny)(/*[in,out]*/ LPSAFEARRAY* pArray);
    STDMETHOD(inoutEnum)(/*[in,out]*/ long * inoutVal);
    STDMETHOD(inoutStruct)(/*[in,out]*/ IDispatch** inoutVal);
    STDMETHOD(inoutInterface)(/*[in,out]*/ IDispatch** ppdisp);
    STDMETHOD(inoutValuesAll)(
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *aXSimple,
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *aStruct,
            /* [out][in] */ long __RPC_FAR *aEnum,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *aSeq,
            /* [out][in] */ VARIANT __RPC_FAR *aAny,
            /* [out][in] */ VARIANT_BOOL __RPC_FAR *aBool,
            /* [out][in] */ short __RPC_FAR *aChar,
            /* [out][in] */ BSTR __RPC_FAR *aString,
            /* [out][in] */ float __RPC_FAR *aFloat,
            /* [out][in] */ double __RPC_FAR *aDouble,
            /* [out][in] */ unsigned char __RPC_FAR *aByte,
            /* [out][in] */ short __RPC_FAR *aShort,
            /* [out][in] */ long __RPC_FAR *aLong);

    STDMETHOD(outByte)( unsigned char* outByte);
    STDMETHOD(outLong)(/*[out]*/ long* outLong);
    STDMETHOD(outShort)(/*[out]*/ short *outShort);
    STDMETHOD(outDouble)(/*[out]*/ double* outDouble);
    STDMETHOD(outFloat)(/*[out]*/ float* outFloat);
    STDMETHOD(outString)(/*[out]*/ BSTR * outString);
    STDMETHOD(outChar)(short* outChar);
    STDMETHOD(outBool)(VARIANT_BOOL* outBool);
    STDMETHOD(outAny)(VARIANT* outAny);
    STDMETHOD(outSeqAny)(/*[out]*/LPSAFEARRAY* outSeq);
    STDMETHOD(outEnum)(/*[out]*/ long* outEnum);
    STDMETHOD(outStruct)(/*[out]*/ IDispatch** outStruct);
            virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE outValuesAll(
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdisp,
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppSimpleStruct,
            /* [out] */ long __RPC_FAR *aSimpleEnum,
            /* [out] */ LPSAFEARRAY* outSeq,
            /* [out] */ VARIANT __RPC_FAR *varAny,
            /* [out] */ VARIANT_BOOL __RPC_FAR *aBool,
            /* [out] */ short __RPC_FAR *aChar,
            /* [out] */ BSTR __RPC_FAR *aString,
            /* [out] */ float __RPC_FAR *aFloat,
            /* [out] */ double __RPC_FAR *aDouble,
            /* [out] */ unsigned char __RPC_FAR *aByte,
            /* [out] */ short __RPC_FAR *aShort,
            /* [out] */ long __RPC_FAR *aLong);
//              );
//
//  STDMETHOD(outValuesAll)(
//          /*[out]*/ IDispatch** ppdisp,
//          /*[out]*/ IUnknown** ppSimpleStruct,
//          /*[out]*/ long* aSimpleEnum,
//          /*[out]*/ VARIANT* ArrayAny,
//          /*[out]*/ VARIANT* varAny,
//          /*[out]*/ VARIANT_BOOL * aBool,
//          /*[out]*/ unsigned short* aChar,
//          /*[out]*/ BSTR* aString, /*[out]*/ float* aFloat,
//          /*[out]*/ double* aDouble,
//          /*[out]*/ signed char* aByte, /*[out]*/ short* aShort, /*[out]*/long* aLong, /*[out]*/ unsigned short* aUShort, /*[out]*/ unsigned long* aULong);
    STDMETHOD(outValuesMixed)(/*[in]*/ long val, /*[out]*/ long* pval, /*[in]*/ BSTR string);
    STDMETHOD(outInterface)(/*[out]*/ IDispatch** ppdisp);
    STDMETHOD(returnInterface)(/*[out, retval]*/ IDispatch** ppdisp);
    STDMETHOD(func1)();
};

#endif //__CALLBACK_H_
