/*************************************************************************
 *
 *  $RCSfile: Basic.h,v $
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
// Basic.h : Declaration of the CBasic

#ifndef __BASIC_H_
#define __BASIC_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CBasic
class ATL_NO_VTABLE CBasic :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CBasic, &CLSID_Basic>,
    public IDispatchImpl<IBasic, &IID_IBasic, &LIBID_AXTESTCOMPONENTSLib>
{
public:
    CBasic()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_BASIC)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CBasic)
    COM_INTERFACE_ENTRY(IBasic)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IBasic
public:
    STDMETHOD(inSequenceLong)(LPSAFEARRAY  ar);
    STDMETHOD(mixed1)(
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
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *inoutDisp)  ;

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

    STDMETHOD(outObject)(/*[out]*/ IDispatch* *val);
    STDMETHOD(outArray)(/*[out]*/ LPSAFEARRAY * val);
    STDMETHOD(outVariant)(/*[out]*/ VARIANT* val);
    STDMETHOD(outDouble)(/*[out]*/ double* val);
    STDMETHOD(outFloat)(/*[out]*/ float* val);
    STDMETHOD(outString)(/*[out]*/ BSTR* val);
    STDMETHOD(outLong)(/*[out]*/ long* val);
    STDMETHOD(outShort)(/*[out]*/ short* val);
    STDMETHOD(outByte)(/*[out]*/ unsigned char* val);

    STDMETHOD(inoutObject)(/*[in,out]*/ IDispatch* *val);
    STDMETHOD(inoutArray)(/*[in,out]*/ LPSAFEARRAY * val);
    STDMETHOD(inoutVariant)(/*[in,out]*/ VARIANT * val);
    STDMETHOD(inoutDouble)(/*[in,out]*/ double * val);
    STDMETHOD(inoutFloat)(/*[in,out]*/ float * val);
    STDMETHOD(inoutString)(/*[in, out]*/ BSTR* val);
    STDMETHOD(inoutLong)(/*[in,out]*/ long * val);
    STDMETHOD(inoutShort)(/*[in,out]*/ short* val);
    STDMETHOD(inoutByte)(/*[in,out]*/ unsigned char* val);

    STDMETHOD(inObject)(/*[in]*/ IDispatch* val);
    STDMETHOD(inArray)(/*[in]*/ LPSAFEARRAY val);
    STDMETHOD(inVariant)(/*[in]*/ VARIANT val);
    STDMETHOD(inDouble)(/*[in]*/ double val);
    STDMETHOD(inFloat)(/*[in]*/ float val);
    STDMETHOD(inString)(/*[in]*/ BSTR val);
    STDMETHOD(inLong)(/*[in]*/ long val);
    STDMETHOD(inShort)(/*[in]*/ short val);
    STDMETHOD(inByte)(/*[in]*/ unsigned char val);


    CComBSTR m_bstrPrpString;
};

#endif //__BASIC_H_

