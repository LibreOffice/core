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

#pragma once

#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/script/InvocationInfo.hpp>
#include <salhelper/simplereferenceobject.hxx>
#include <cppuhelper/implbase.hxx>

#include "comifaces.hxx"
#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"

#define JSCRIPT_VALUE_FUNC  L"_GetValueObject"
#define GET_STRUCT_FUNC     L"_GetStruct"
#define BRIDGE_VALUE_FUNC L"Bridge_GetValueObject"
#define BRIDGE_GET_STRUCT_FUNC L"Bridge_GetStruct"
#define BRIDGE_CREATE_TYPE_FUNC L"Bridge_CreateType"

#define DISPID_JSCRIPT_VALUE_FUNC   -10l
#define DISPID_GET_STRUCT_FUNC      -102
#define DISPID_CREATE_TYPE_FUNC     -103

using namespace std;
using namespace cppu;
using namespace com::sun::star::bridge;
using namespace com::sun::star::script;

struct MemberInfo
{
    MemberInfo() : flags(0), name() {}
    MemberInfo(WORD f, const OUString& n) : flags(f), name(n) {}

    WORD    flags;
    OUString name;
};

typedef std::unordered_map
<
    OUString,
    DISPID
> NameToIdMap;

typedef std::unordered_map
<
    OUString,
    bool
> BadNameMap;

typedef std::unordered_map
<
    DISPID,
    MemberInfo
> IdToMemberInfoMap;

// An InterfaceOleWrapper object can wrap either a UNO struct or a UNO
// interface as a COM IDispatchEx and IUnoObjectWrapper.

class InterfaceOleWrapper : public WeakImplHelper<XBridgeSupplier2, XInitialization>,
                            public IDispatchEx,
                            public IProvideClassInfo,
                            public IConnectionPointContainer,
                            public UnoConversionUtilities<InterfaceOleWrapper>,
                            public IUnoObjectWrapper
{
public:
    InterfaceOleWrapper(Reference<XMultiServiceFactory> const & xFactory, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass);
    ~InterfaceOleWrapper() override;

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    // IDispatch
    STDMETHOD( GetTypeInfoCount )( UINT * pctinfo ) override;
    STDMETHOD( GetTypeInfo )( UINT itinfo, LCID lcid, ITypeInfo ** pptinfo ) override;
    STDMETHOD( GetIDsOfNames )( REFIID riid, LPOLESTR * rgszNames, UINT cNames,
                                LCID lcid, DISPID * rgdispid ) override;
    STDMETHOD( Invoke )( DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         UINT * puArgErr ) override;

    // IDispatchEx
    virtual HRESULT STDMETHODCALLTYPE GetDispID(
        /* [in] */ BSTR bstrName,
        /* [in] */ DWORD grfdex,
        /* [out] */ DISPID __RPC_FAR *pid) override;

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE InvokeEx(
        /* [in] */ DISPID id,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [in] */ DISPPARAMS __RPC_FAR *pdp,
        /* [out] */ VARIANT __RPC_FAR *pvarRes,
        /* [out] */ EXCEPINFO __RPC_FAR *pei,
        /* [unique][in] */ IServiceProvider __RPC_FAR *pspCaller) override;

    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName(
        /* [in] */ BSTR bstr,
        /* [in] */ DWORD grfdex) override;

    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(
        /* [in] */ DISPID id) override;

    virtual HRESULT STDMETHODCALLTYPE GetMemberProperties(
        /* [in] */ DISPID id,
        /* [in] */ DWORD grfdexFetch,
        /* [out] */ DWORD __RPC_FAR *pgrfdex) override;

    virtual HRESULT STDMETHODCALLTYPE GetMemberName(
        /* [in] */ DISPID id,
        /* [out] */ BSTR __RPC_FAR *pbstrName) override;

    virtual HRESULT STDMETHODCALLTYPE GetNextDispID(
        /* [in] */ DWORD grfdex,
        /* [in] */ DISPID id,
        /* [out] */ DISPID __RPC_FAR *pid) override;

    virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent(
        /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) override;

    // IProvideClassInfo
    virtual HRESULT STDMETHODCALLTYPE GetClassInfo(
        /* [out] */ ITypeInfo **ppTI) override;

    // IConnectionPointContainer
    virtual HRESULT STDMETHODCALLTYPE EnumConnectionPoints(
        /* [out] */ IEnumConnectionPoints **ppEnum) override;
    virtual HRESULT STDMETHODCALLTYPE FindConnectionPoint(
        /* [in] */ REFIID riid,
        /* [out] */ IConnectionPoint **ppCP) override;

    // XBridgeSupplier2
    virtual Any SAL_CALL createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& ProcessId,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // IUnoObjectWrapper
    STDMETHOD( getWrapperXInterface)( Reference<XInterface>* pXInt) override;
    STDMETHOD( getOriginalUnoObject)( Reference<XInterface>* pXInt) override;
    STDMETHOD( getOriginalUnoStruct)( Any * pStruct) override;

    // UnoConversionUtility
    virtual Reference< XInterface > createUnoWrapperInstance() override;
    virtual Reference< XInterface > createComWrapperInstance() override;

    const OUString& getImplementationName() const
    {
        return m_sImplementationName;
    }

protected:
    virtual HRESULT doInvoke( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString & name, Sequence<Any>& params);

    virtual HRESULT doGetProperty( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                        EXCEPINFO * pexcepinfo, OUString & name );

    virtual HRESULT doSetProperty( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                        EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString & name, Sequence<Any> const & params);

    virtual HRESULT InvokeGeneral( DISPID dispidMember, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * puArgErr, bool& bHandled);

    void convertDispparamsArgs( DISPID id, unsigned short wFlags, DISPPARAMS* pdispparams,
                            Sequence<Any>& rSeq);

    bool getInvocationInfoForCall(DISPID id, InvocationInfo& info);

    Reference<XInvocation>                  m_xInvocation;
    Reference<XExactName>                   m_xExactName;
    Reference<XInterface>                   m_xOrigin;
    NameToIdMap                     m_nameToDispIdMap;
    vector<MemberInfo>              m_MemberInfos;
    // This member is used to determine the default value
    // denoted by DISPID_VALUE (0). For proper results in JavaScript
    // we have to return the default value when we write an object
    // as out parameter. That is, we get a JScript Array as parameter
    // and put a wrapped object on index null. The array object tries
    // to detect the default value. The wrapped object must then return
    // its own IDispatch* otherwise we cannot access it within the script.
    // see InterfaceOleWrapper::Invoke
    VARTYPE                         m_defaultValueType;

    // The name of the implementation. Can be empty if unknown.
    OUString                        m_sImplementationName;
};

/*****************************************************************************

    UnoObjectWrapperRemoteOpt = Uno Object Wrapper Remote Optimized

    This is the UNO wrapper used in the service com.sun.star.bridge.OleBridgeSupplierVar1.
    Key features:
    DISPIDs are passed out blindly. That is in GetIDsOfNames is no name checking carried out.
    Only if Invoke fails the name is being checked. Moreover Invoke tries to figure out
    if a call is made to a property or method if the flags are DISPATCH_METHOD | DISPATCH_PROPERTYPUT.
    If something has been found out about a property or member than it is saved
    in a MemberInfo structure hold by an IdToMemberInfoMap stl map.

*****************************************************************************/
class UnoObjectWrapperRemoteOpt: public InterfaceOleWrapper
{
public:
    UnoObjectWrapperRemoteOpt( Reference<XMultiServiceFactory> const & aFactory, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass);
    ~UnoObjectWrapperRemoteOpt() override;

    STDMETHOD( GetIDsOfNames )( REFIID riid, LPOLESTR * rgszNames, UINT cNames,
                                LCID lcid, DISPID * rgdispid ) override;
    STDMETHOD( Invoke )( DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         UINT * puArgErr ) override;

    // UnoConversionUtility
    // If UNO interfaces are converted in methods of this class then
    // they are always wrapped with instances of this class
    virtual Reference< XInterface > createUnoWrapperInstance() override;

protected:

    static HRESULT methodInvoke( DISPID dispidMember, DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, Sequence<Any> const & params);
    // In GetIDsOfNames are blindly passed out, that is without verifying
    // the name. If two names are passed in during different calls to
    // GetIDsOfNames and the names differ only in their cases then different
    // id's are passed out ( e.g. "doSomethingMethod" or "dosomethingmethod").
    // In Invoke the DISPID is remapped to the name passed to GetIDsOfNames
    // and the name is used as parameter for XInvocation::invoke. If invoke
    // fails because of a wrong name, then m_xExactName ( XExactName) is used
    // to verify the name. The correct name is then inserted to m_MemberInfos
    // ( vector<MemberInfo> ). During the next call to Invoke the right name
    // is used.         .


    BadNameMap m_badNameMap;

    IdToMemberInfoMap m_idToMemberInfoMap;

    DISPID m_currentId;


};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
