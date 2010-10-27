/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "stdafx2.h"

HRESULT ExecuteFunc( IDispatch* idispUnoObject,
                     OLECHAR* sFuncName,
                     CComVariant* params,
                     unsigned int count,
                     CComVariant* pResult );

HRESULT GetIDispByFunc( IDispatch* idispUnoObject,
                          OLECHAR* sFuncName,
                          CComVariant* params,
                          unsigned int count,
                          CComPtr<IDispatch>& pdispResult );

HRESULT PutPropertiesToIDisp( IDispatch* pdispObject,
                              OLECHAR** sMemberNames,
                              CComVariant* pVariant,
                              unsigned int count );

HRESULT GetPropertiesFromIDisp( IDispatch* pdispObject,
                                OLECHAR** sMemberNames,
                                CComVariant* pVariant,
                                unsigned int count );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
