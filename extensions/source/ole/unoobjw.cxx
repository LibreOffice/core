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

#include "ole2uno.hxx"

#include <stdio.h>
#include <list>
#include <unordered_map>
#include <vector>

#include <osl/diagnose.h>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/lang/NoSuchMethodException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/FailReason.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/ParamInfo.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>

#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/script/MemberType.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <osl/interlck.h>
#include <com/sun/star/uno/genfunc.h>
#include <comphelper/processfactory.hxx>

#include "comifaces.hxx"
#include "jscriptclasses.hxx"
#include "unotypewrapper.hxx"
#include "oleobjw.hxx"
#include "unoobjw.hxx"
#include "servprov.hxx"

using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::script;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge::ModelDependent;
using namespace com::sun::star::reflection;

#ifndef _MSC_VER
extern "C" const GUID IID_IDispatchEx;
#endif

namespace ole_adapter
{
std::unordered_map<sal_uIntPtr, WeakReference<XInterface> > UnoObjToWrapperMap;
static bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource);
static bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource);
static HRESULT mapCannotConvertException(const CannotConvertException &e, unsigned int * puArgErr);

/* Does not throw any exceptions.
   Param pInfo can be NULL.
 */
static void writeExcepinfo(EXCEPINFO * pInfo, const OUString& message)
{
    if (pInfo != nullptr)
    {
        pInfo->wCode = UNO_2_OLE_EXCEPTIONCODE;
        pInfo->bstrSource = SysAllocString(L"[automation bridge] ");
        pInfo->bstrDescription = SysAllocString(reinterpret_cast<LPCOLESTR>(message.getStr()));
    }
}

InterfaceOleWrapper_Impl::InterfaceOleWrapper_Impl( Reference<XMultiServiceFactory>& xFactory,
                                                    sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
        UnoConversionUtilities<InterfaceOleWrapper_Impl>( xFactory, unoWrapperClass, comWrapperClass),
        m_defaultValueType( 0)
{
}

InterfaceOleWrapper_Impl::~InterfaceOleWrapper_Impl()
{
    MutexGuard guard(getBridgeMutex());
    // remove entries in global map
    IT_Uno it= UnoObjToWrapperMap.find( reinterpret_cast<sal_uIntPtr>(m_xOrigin.get()));
    if(it != UnoObjToWrapperMap.end())
        UnoObjToWrapperMap.erase(it);
}

STDMETHODIMP InterfaceOleWrapper_Impl::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    HRESULT ret= S_OK;

    if( !ppv)
        return E_POINTER;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = static_cast<IUnknown*>(static_cast<IDispatch*>(this));
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        AddRef();
        *ppv = static_cast<IDispatch*>(this);
    }
    else if( IsEqualIID( riid, __uuidof( IUnoObjectWrapper)))
    {
        AddRef();
        *ppv= static_cast<IUnoObjectWrapper*>(this);
    }
    else
        ret= E_NOINTERFACE;
    return ret;
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper_Impl::AddRef()
{
    acquire();
    // does not need to guard because one should not rely on the return value of
    // AddRef anyway
    return m_refCount;
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper_Impl::Release()
{
    ULONG n= m_refCount;
    release();
    return n - 1;
}

// IUnoObjectWrapper --------------------------------------------------------
STDMETHODIMP InterfaceOleWrapper_Impl::getWrapperXInterface( Reference<XInterface>* pXInt)
{
    pXInt->set( static_cast<XWeak*>( this), UNO_QUERY);
    return pXInt->is() ? S_OK : E_FAIL;
}
STDMETHODIMP InterfaceOleWrapper_Impl::getOriginalUnoObject( Reference<XInterface>* pXInt)
{
    *pXInt= m_xOrigin;
    return m_xOrigin.is() ? S_OK : E_FAIL;
}
STDMETHODIMP  InterfaceOleWrapper_Impl::getOriginalUnoStruct( Any * pStruct)
{
    HRESULT ret= E_FAIL;
    if( !m_xOrigin.is())
    {
        Reference<XMaterialHolder> xMatHolder( m_xInvocation, UNO_QUERY);
        if( xMatHolder.is())
        {
            Any any = xMatHolder->getMaterial();
            if( any.getValueTypeClass() == TypeClass_STRUCT)
            {
                *pStruct= any;
                ret= S_OK;
            }
        }
    }
    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetTypeInfoCount( unsigned int * /*pctinfo*/ )
{
    return E_NOTIMPL ;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetTypeInfo(unsigned int /*itinfo*/, LCID /*lcid*/, ITypeInfo ** /*pptinfo*/)
{
    return E_NOTIMPL;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetIDsOfNames(REFIID /*riid*/,
                                                     OLECHAR ** rgszNames,
                                                     unsigned int cNames,
                                                     LCID /*lcid*/,
                                                     DISPID * rgdispid )
{
    HRESULT ret = DISP_E_UNKNOWNNAME;
    try
    {
        MutexGuard guard( getBridgeMutex());
        if( ! rgdispid)
            return E_POINTER;

        if( ! _wcsicmp( *rgszNames, JSCRIPT_VALUE_FUNC) ||
            ! _wcsicmp( *rgszNames, BRIDGE_VALUE_FUNC))
        {
            *rgdispid= DISPID_JSCRIPT_VALUE_FUNC;
            return S_OK;
        }
        else if( ! _wcsicmp( *rgszNames, GET_STRUCT_FUNC) ||
                 ! _wcsicmp( *rgszNames, BRIDGE_GET_STRUCT_FUNC))
        {
            *rgdispid= DISPID_GET_STRUCT_FUNC;
            return S_OK;
        }
        else if( ! _wcsicmp( *rgszNames, BRIDGE_CREATE_TYPE_FUNC))
        {
            *rgdispid= DISPID_CREATE_TYPE_FUNC;
            return S_OK;
        }

        if (m_xInvocation.is() && (cNames > 0))
        {
            OUString name(reinterpret_cast<const sal_Unicode*>(rgszNames[0]));
            NameToIdMap::iterator iter = m_nameToDispIdMap.find(name);

            if (iter == m_nameToDispIdMap.end())
            {
                OUString exactName;

                if (m_xExactName.is())
                {
                    exactName = m_xExactName->getExactName(name);
                }
                else
                {
                    exactName = name;
                }

                MemberInfo d(0, exactName);

                if (m_xInvocation->hasProperty(exactName))
                {
                    d.flags |= DISPATCH_PROPERTYGET;
                    d.flags |= DISPATCH_PROPERTYPUT;
                    d.flags |= DISPATCH_PROPERTYPUTREF;
                }

                if (m_xInvocation->hasMethod(exactName))
                {
                    d.flags |= DISPATCH_METHOD;
                }

                if (d.flags != 0)
                {
                    m_MemberInfos.push_back(d);
                    iter = m_nameToDispIdMap.insert(NameToIdMap::value_type(exactName, (DISPID)m_MemberInfos.size())).first;

                    if (exactName != name)
                    {
                        iter = m_nameToDispIdMap.insert(NameToIdMap::value_type(name, (DISPID)m_MemberInfos.size())).first;
                    }
                }
            }

            if (iter == m_nameToDispIdMap.end())
            {
                ret = DISP_E_UNKNOWNNAME;
            }
            else
            {
                *rgdispid = (*iter).second;
                ret = S_OK;
            }
        }
    }
    catch(const BridgeRuntimeError&)
    {
        OSL_ASSERT(false);
    }
    catch(const Exception&)
    {
        OSL_ASSERT(false);
    }
    catch(...)
    {
        OSL_ASSERT(false);
    }

    return ret;
}

// "convertDispparamsArgs" converts VARIANTS to their respecting Any counterparts
// The parameters "id", "wFlags" and "pdispparams" equal those as used in
// IDispatch::Invoke. The function handles special JavaScript
// cases where a VARIANT of type VT_DISPATCH is ambiguous and could represent
// an object, array ( JavaScript Array object), out parameter and in/out ( JavaScript Array object)
//  parameter (JavaScript Array object)
// Because all those VT_DISPATCH objects need a different conversion
// we have to find out what the object is supposed to be. The function does this
// by either using type information or by help of a specialized ValueObject object.

// A. Type Information

// With the help of type information the kind of parameter can be exactly determined
// and an appropriate conversion can be chosen. A problem arises if a method expects
// an Any. Then the type info does not tell what the type of the value, that is kept
// by the any, should be. In this situation the decision whether the param is a
// sequence or an object is made upon the fact if the object has a property "0"
// ( see function "isJScriptArray"). Since this is unsafe it is recommended to use
// the JScript value objects within a JScript script on such an occasion.

// B. JavaScript Value Object ( class JScriptValue )

// A JScriptValue (ValueObject) object is a COM object in that it implements IDispatch and the
// IJScriptValue object interface. Such objects are provided by all UNO wrapper
// objects used within a JScript script. To obtain an instance one has to call
// "_GetValueObject() or Bridge_GetValueObject()" on an UNO wrapper object (class InterfaceOleWrapper_Impl).
// A value object is appropriately initialized within the script and passed as
// parameter to an UNO object method or property. The convertDispparamsArgs function
// can easily find out that a param is such an object by querying for the
// IJScriptValue interface. By this interface one the type and kind ( out, in/out)
// can be determined and the right conversion can be applied.
// Using ValueObjects we spare us the effort of acquiring and examining type information
// in order to figure out what the an IDispatch parameter is meant for.

// Normal JScript object parameter can be mixed with JScriptValue object. If an
// VARIANT contains an VT_DISPATCH that is no JScriptValue than the type information
// is used to find out about the required type.
void InterfaceOleWrapper_Impl::convertDispparamsArgs(DISPID id,
    unsigned short /*wFlags*/, DISPPARAMS* pdispparams, Sequence<Any>& rSeq)
{
    HRESULT hr= S_OK;
    sal_Int32 countArgs= pdispparams->cArgs;
    if( countArgs == 0)
        return;

    rSeq.realloc( countArgs);
    Any*    pParams = rSeq.getArray();

    Any anyParam;

    //Get type information for the current call
    InvocationInfo info;
    if( ! getInvocationInfoForCall( id, info))
        throw BridgeRuntimeError(
                  "[automation bridge]InterfaceOleWrapper_Impl::convertDispparamsArgs \n"
                  "Could not obtain type information for current call.");

    for (int i = 0; i < countArgs; i++)
    {
        if (info.eMemberType == MemberType_METHOD &&
            info.aParamModes[ countArgs - i -1 ]  == ParamMode_OUT)
            continue;

         if(convertValueObject( & pdispparams->rgvarg[i], anyParam))
         { //a param is a ValueObject and could be converted
            pParams[countArgs - (i + 1)] = anyParam;
             continue;
         }

        // If the param is an out, in/out parameter in
        // JScript (Array object, with value at index 0) then we
        // extract Array[0] and put the value into varParam. At the end of the loop varParam
        // is converted if it contains a value otherwise the VARIANT from
        // DISPPARAMS is converted.
        CComVariant varParam;

        // Check for JScript out and in/out paramsobjects (VT_DISPATCH).
        // To find them out we use typeinformation of the function being called.
        if( pdispparams->rgvarg[i].vt == VT_DISPATCH )
        {
            if( info.eMemberType == MemberType_METHOD && info.aParamModes[ countArgs - i -1 ]  == ParamMode_INOUT)
            {
                // INOUT-param
                // Index ( property) "0" contains the actual IN-param. The object is a JScript
                // Array object.
                // Get the IN-param at index "0"
                IDispatch* pdisp= pdispparams->rgvarg[i].pdispVal;

                OLECHAR const * sindex= L"0";
                DISPID id2;
                DISPPARAMS noParams= {nullptr,nullptr,0,0};
                if(SUCCEEDED( hr= pdisp->GetIDsOfNames( IID_NULL, const_cast<OLECHAR **>(&sindex), 1, LOCALE_USER_DEFAULT, &id2)))
                    hr= pdisp->Invoke( id2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                       & noParams, & varParam, nullptr, nullptr);
                if( FAILED( hr))
                {
                    throw BridgeRuntimeError(
                        "[automation bridge] Could not determine "
                        "if the object has a member \"0\". Error: " +
                        OUString::number(hr));
                }
            }
        }

        if( varParam.vt == VT_EMPTY) // then it was no in/out parameter
                 varParam= pdispparams->rgvarg[i];

        if(info.eMemberType == MemberType_METHOD)
            variantToAny( & varParam, anyParam,
                           info.aParamTypes[ countArgs - i - 1]);
        else if(info.eMemberType == MemberType_PROPERTY)
            variantToAny( & varParam, anyParam, info.aType);
        else
            OSL_ASSERT(false);

        pParams[countArgs - (i + 1)]= anyParam;
    }// end for / iterating over all parameters
}

bool  InterfaceOleWrapper_Impl::getInvocationInfoForCall( DISPID id, InvocationInfo& info)
{
    bool bTypesAvailable= false;

    if( !m_xInvocation.is() )return false;
    Reference<XInvocation2> inv2( m_xInvocation, UNO_QUERY);
    if( inv2.is())
    {
        // We need the name of the property or method to get its type information.
        // The name can be identified through the param "id"
        // that is kept as value in the map m_nameToDispIdMap.
        // Proplem: the Windows JScript engine sometimes changes small letters to capital
        // letters as happens in xidlclass_obj.createObject( var) // in JScript.
        // IDispatch::GetIdsOfNames is then called with "CreateObject" !!!
        // m_nameToDispIdMap can contain several names for one DISPID but only one is
        // the exact one. If there's no m_xExactName and therefore no exact name then
        // there's only one entry in the map.
        typedef NameToIdMap::const_iterator cit;
        OUString sMemberName;

        for(cit ci1= m_nameToDispIdMap.begin(); ci1 != m_nameToDispIdMap.end(); ++ci1)
        {
            if( (*ci1).second == id) // iterator is a pair< OUString, DISPID>
            {
                sMemberName= (*ci1).first;
                break;
            }
        }
        // Get information for the current call ( property or method).
        // There could be similar names which only differ in the cases
        // of letters. First we assume that the name which was passed into
        // GetIDsOfNames is correct. If we won't get information with that
        // name then we have the invocation service use the XExactName interface.
        bool validInfo= true;
        InvocationInfo invInfo;
        try{
            invInfo= inv2->getInfoForName( sMemberName, false);
        }
        catch(const IllegalArgumentException&)
        {
            validInfo= false;
        }

        if( ! validInfo)
        {
            invInfo= inv2->getInfoForName( sMemberName, true);
        }
        if( invInfo.aName.pData)
        {
            bTypesAvailable= true;
            info= invInfo;
        }
    }
    return bTypesAvailable;
}

// XBridgeSupplier2 ---------------------------------------------------
// only bridges itself ( this instance of InterfaceOleWrapper_Impl)from UNO to IDispatch
// If sourceModelType is UNO than any UNO interface implemented by InterfaceOleWrapper_Impl
// can bridged to IDispatch ( if destModelType == OLE). The IDispatch is
// implemented by this class.
Any SAL_CALL InterfaceOleWrapper_Impl::createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& /*ProcessId*/,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType)
{

    Any retAny;
    if( sourceModelType == UNO && destModelType == OLE &&
        modelDepObject.getValueTypeClass() == TypeClass_INTERFACE )
    {
        Reference<XInterface> xInt;
        if( modelDepObject >>= xInt )
        {
            if( xInt == Reference<XInterface>( static_cast<XWeak*>( this), UNO_QUERY))
            {
                VARIANT *pVar= static_cast<VARIANT*>(CoTaskMemAlloc( sizeof( VARIANT)));
                if( pVar)
                {
                    pVar->vt= VT_DISPATCH;
                    pVar->pdispVal= static_cast<IDispatch*>( this);
                    AddRef();

                    retAny<<= reinterpret_cast< sal_uIntPtr >( pVar);
                }
            }
        }
    }

    return retAny;
}

// XInitialization --------------------------------------------------
void SAL_CALL InterfaceOleWrapper_Impl::initialize( const Sequence< Any >& aArguments )
{
    switch( aArguments.getLength() )
    {
    case 2: // the object wraps an UNO struct
        aArguments[0] >>= m_xInvocation;
        aArguments[1] >>= m_defaultValueType;
        break;
    case 3: // the object wraps an UNO interface
        aArguments[0] >>= m_xInvocation;
        aArguments[1] >>= m_xOrigin;
        aArguments[2] >>= m_defaultValueType;
        break;
    }

    m_xExactName.set( m_xInvocation, UNO_QUERY);
}

Reference< XInterface > InterfaceOleWrapper_Impl::createUnoWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

Reference<XInterface> InterfaceOleWrapper_Impl::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

// "getType" is used in convertValueObject to map the string denoting the type
// to an actual Type object.
bool getType( const BSTR name, Type & type)
{
    bool ret = false;
    typelib_TypeDescription * pDesc= nullptr;
    OUString str( reinterpret_cast<const sal_Unicode*>(name));
    typelib_typedescription_getByName( &pDesc, str.pData );
    if( pDesc)
    {
        type = Type( pDesc->pWeakRef );
        typelib_typedescription_release( pDesc);
        ret = true;
    }
    return ret;
}

static bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource)
{
    bool ret = false;
    HRESULT hr;

    // Handle JScriptValue objects and JScript out params ( Array object )
    CComVariant varDest( *pDest);

    if( SUCCEEDED( varDest.ChangeType(VT_DISPATCH)))
    {
        CComPtr<IDispatch> spDispDest(varDest.pdispVal);

        // special Handling for a JScriptValue object
#ifdef __MINGW32__
        CComQIPtr<IJScriptValueObject, &__uuidof(IJScriptValueObject)> spValueDest(spDispDest);
#else
        CComQIPtr<IJScriptValueObject> spValueDest(spDispDest);
#endif
        if (spValueDest)
        {
            VARIANT_BOOL varBool= VARIANT_FALSE;
            if ((SUCCEEDED(hr = spValueDest->IsOutParam(&varBool))
                 && varBool == VARIANT_TRUE)
                || (SUCCEEDED(hr = spValueDest->IsInOutParam(&varBool))
                    && varBool == VARIANT_TRUE))
            {
                if( SUCCEEDED( spValueDest->Set( CComVariant(), *pSource)))
                    ret= true;
            }
        }
        else if (pDest->vt == VT_DISPATCH)// VT_DISPATCH -> JScript out param
        {
            // We use IDispatchEx because its GetDispID function causes the creation
            // of a property if it does not exist already. This is convenient for
            // out parameters in JScript. Then the user must not specify propery "0"
            // explicitly
#ifdef __MINGW32__
            CComQIPtr<IDispatchEx, &__uuidof(IDispatchEx)> spDispEx( spDispDest);
#else
            CComQIPtr<IDispatchEx> spDispEx( spDispDest);
#endif
            if( spDispEx)
            {
                CComBSTR nullProp(L"0");
                DISPID dwDispID;
                if( SUCCEEDED( spDispEx->GetDispID( nullProp, fdexNameEnsure, &dwDispID)))
                {
                    DISPPARAMS dispparams = {nullptr, nullptr, 1, 1};
                    dispparams.rgvarg = pSource;
                    DISPID dispidPut = DISPID_PROPERTYPUT;
                    dispparams.rgdispidNamedArgs = &dispidPut;

                    if (pSource->vt == VT_UNKNOWN || pSource->vt == VT_DISPATCH ||
                        (pSource->vt & VT_ARRAY) || (pSource->vt & VT_BYREF))
                        hr = spDispEx->InvokeEx(dwDispID, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF,
                                                &dispparams, nullptr, nullptr, nullptr);
                    else
                        hr= spDispEx->InvokeEx(dwDispID, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
                                               &dispparams, nullptr, nullptr, nullptr);
                    if( SUCCEEDED(hr))
                        ret= true;
                }
            }
        }
        else
            ret= writeBackOutParameter( pDest, pSource);
    }
    else // The param can't be a JScript out-parameter ( an Array object), it could be a VBScript
    {   // param. The function checks itself for correct VBScript params
        ret= writeBackOutParameter( pDest, pSource);
    }
    return ret;
}

// VisualBasic Script passes arguments as VT_VARIANT | VT_BYREF be it in or out parameter.
// Thus we are in charge of freeing an eventual value contained by the inner VARIANT
// Please note: VariantCopy doesn't free a VT_BYREF value
// The out parameters are expected to have always a valid type
static bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource)
{
    HRESULT hr;
    bool ret = false;
    // Out parameter must be VT_BYREF
    if ((V_VT(pDest) & VT_BYREF) != 0 )
    {
        VARTYPE oleTypeFlags = V_VT(pSource);

        // if caller accept VARIANT as out parameter, any value must be converted
        if (V_VT(pDest) == (VT_VARIANT | VT_BYREF))
        {
            // When the user provides a VARIANT rather then a concrete type
            // we just copy the source to the out, in/out parameter
            // VT_DISPATCH, VT_UNKNOWN, VT_ARRAY, VT_BSTR in the VARIANT that
            // is contained in pDest are released by VariantCopy
            VariantCopy(V_VARIANTREF(pDest), pSource);
            ret = true;
        }
        else
        {
            // variantarg and variant must have same type
              if ((V_VT(pDest) & oleTypeFlags) == oleTypeFlags)
            {
                if ((oleTypeFlags & VT_ARRAY) != 0)
                {
                    // In / Out Param
                    if( *V_ARRAYREF(pDest) != nullptr)
                        hr= SafeArrayCopyData( V_ARRAY(pSource), *V_ARRAYREF(pDest));
                    else
                        // Out Param
                        hr= SafeArrayCopy(V_ARRAY(pSource), V_ARRAYREF(pDest));
                    if( SUCCEEDED( hr))
                        ret = true;
                }
                else
                {
                    // copy base type
                    switch (V_VT(pSource))
                    {
                    case VT_I2:
                    {
                        *V_I2REF(pDest) = V_I2(pSource);
                        ret = true;
                        break;
                    }
                    case VT_I4:
                        *V_I4REF(pDest) = V_I4(pSource);
                        ret = true;
                        break;
                    case VT_R4:
                        *V_R4REF(pDest) = V_R4(pSource);
                        ret = true;
                        break;
                    case VT_R8:
                        *V_R8REF(pDest) = V_R8(pSource);
                        ret = true;
                        break;
                    case VT_CY:
                        *V_CYREF(pDest) = V_CY(pSource);
                        ret = true;
                        break;
                    case VT_DATE:
                        *V_DATEREF(pDest) = V_DATE(pSource);
                        ret = true;
                        break;
                    case VT_BSTR:
                        SysFreeString( *pDest->pbstrVal);

                        *V_BSTRREF(pDest) = SysAllocString(V_BSTR(pSource));
                        ret = true;
                        break;
                    case VT_DISPATCH:
                        if (*V_DISPATCHREF(pDest) != nullptr)
                            (*V_DISPATCHREF(pDest))->Release();

                        *V_DISPATCHREF(pDest) = V_DISPATCH(pSource);

                        if (*V_DISPATCHREF(pDest) != nullptr)
                            (*V_DISPATCHREF(pDest))->AddRef();

                        ret = true;
                        break;
                    case VT_ERROR:
                        *V_ERRORREF(pDest) = V_ERROR(pSource);
                        ret = true;
                        break;
                    case VT_BOOL:
                        *V_BOOLREF(pDest) = V_BOOL(pSource);
                        ret = true;
                        break;
                    case VT_UNKNOWN:
                        if (*V_UNKNOWNREF(pDest) != nullptr)
                            (*V_UNKNOWNREF(pDest))->Release();

                        *V_UNKNOWNREF(pDest) = V_UNKNOWN(pSource);

                        if (*V_UNKNOWNREF(pDest) != nullptr)
                            (*V_UNKNOWNREF(pDest))->AddRef();

                        ret = true;
                        break;
                    case VT_I1:
                        *V_I1REF(pDest) = V_I1(pSource);
                        ret = true;
                        break;
                    case VT_UI1:
                        *V_UI1REF(pDest) = V_UI1(pSource);
                        ret = true;
                        break;
                    case VT_UI2:
                        *V_UI2REF(pDest) = V_UI2(pSource);
                        ret = true;
                        break;
                    case VT_UI4:
                        *V_UI4REF(pDest) = V_UI4(pSource);
                        ret = true;
                        break;
                    case VT_INT:
                        *V_INTREF(pDest) = V_INT(pSource);
                        ret = true;
                        break;
                    case VT_UINT:
                        *V_UINTREF(pDest) = V_UINT(pSource);
                        ret = true;
                        break;
                    case VT_DECIMAL:
                        memcpy(pDest->pdecVal, pSource, sizeof(DECIMAL));
                        ret = true;
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                // Handling of special cases
                // Destination and source types are different
                if( pDest->vt == (VT_BSTR | VT_BYREF)
                    && pSource->vt == VT_I2)
                {
                    // When the user provides a String as out our in/out parameter
                    // and the type is char (TypeClass_CHAR) then we convert to a BSTR
                    // instead of VT_I2 as is done otherwise
                    OLECHAR buff[]= {0,0};
                    buff[0]= pSource->iVal;

                    SysFreeString( *pDest->pbstrVal);
                    *pDest->pbstrVal= SysAllocString( buff);
                    ret = true;
                }
            }
        }
    }
    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::Invoke(DISPID dispidMember,
                                              REFIID /*riid*/,
                                              LCID /*lcid*/,
                                              unsigned short wFlags,
                                               DISPPARAMS * pdispparams,
                                              VARIANT * pvarResult,
                                              EXCEPINFO * pexcepinfo,
                                               unsigned int * puArgErr )
{
    HRESULT ret = S_OK;

    try
    {
        bool bHandled= false;
        ret= InvokeGeneral( dispidMember,  wFlags, pdispparams, pvarResult,  pexcepinfo,
                            puArgErr, bHandled);
        if( bHandled)
            return ret;

        if ((dispidMember > 0) && ((size_t)dispidMember <= m_MemberInfos.size()) && m_xInvocation.is())
        {
            MemberInfo d = m_MemberInfos[dispidMember - 1];
            DWORD flags = wFlags & d.flags;

            if (flags != 0)
            {
                if ((flags & DISPATCH_METHOD) != 0)
                {
                    if (pdispparams->cNamedArgs > 0)
                        ret = DISP_E_NONAMEDARGS;
                    else
                    {
                        Sequence<Any> params;

                        convertDispparamsArgs(dispidMember, wFlags, pdispparams , params );

                        ret= doInvoke(pdispparams, pvarResult,
                                      pexcepinfo, puArgErr, d.name, params);
                    }
                }
                else if ((flags & DISPATCH_PROPERTYGET) != 0)
                {
                    ret=  doGetProperty( pdispparams, pvarResult,
                                         pexcepinfo, d.name);
                }
                else if ((flags & DISPATCH_PROPERTYPUT) != 0 || (flags & DISPATCH_PROPERTYPUTREF) != 0)
                {
                    if (pdispparams->cArgs != 1)
                        ret = DISP_E_BADPARAMCOUNT;
                    else
                    {
                        Sequence<Any> params;
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        if(params.getLength() > 0)
                            ret= doSetProperty( pdispparams, pvarResult, pexcepinfo, puArgErr, d.name, params);
                        else
                            ret = DISP_E_BADVARTYPE;
                    }
                }
            }
            else
                ret= DISP_E_MEMBERNOTFOUND;
        }
        else
            ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const BridgeRuntimeError& e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception& e)
    {
        OUString message= "InterfaceOleWrapper_Impl::Invoke : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch(...)
    {
        OUString message= "InterfaceOleWrapper_Impl::Invoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
         ret = DISP_E_EXCEPTION;
    }

    return ret;
}

HRESULT InterfaceOleWrapper_Impl::doInvoke( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString& name, Sequence<Any>& params)
{


    HRESULT ret= S_OK;
    try
    {
        Sequence<sal_Int16>     outIndex;
        Sequence<Any>   outParams;
        Any                 returnValue;

        if (pdispparams->cNamedArgs > 0)
            return DISP_E_NONAMEDARGS;

        // invoke method and take care of exceptions
        returnValue = m_xInvocation->invoke(name,
                                            params,
                                            outIndex,
                                            outParams);

        // try to write back out parameter
        if (outIndex.getLength() > 0)
        {
            const sal_Int16* pOutIndex = outIndex.getConstArray();
            const Any* pOutParams = outParams.getConstArray();

            for (sal_Int32 i = 0; i < outIndex.getLength(); i++)
            {
                CComVariant variant;
                // Currently a Sequence is converted to an SafeArray of VARIANTs.
                anyToVariant( &variant, pOutParams[i]);

                // out parameter need special handling if they are VT_DISPATCH
                // and used in JScript
                int outindex= pOutIndex[i];
                writeBackOutParameter2(&(pdispparams->rgvarg[pdispparams->cArgs - 1 - outindex]),
                                       &variant );
            }
        }

        // write back return value
        if (pvarResult != nullptr)
            anyToVariant(pvarResult, returnValue);
    }
    catch(const IllegalArgumentException & e) //XInvocation::invoke
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = DISP_E_TYPEMISMATCH;
    }
    catch(const CannotConvertException & e) //XInvocation::invoke
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = mapCannotConvertException( e, puArgErr);
    }
    catch(const InvocationTargetException &  e) //XInvocation::invoke
    {
        const Any& org = e.TargetException;
        Exception excTarget;
        org >>= excTarget;
        OUString message=
            org.getValueType().getTypeName() + ": " + excTarget.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const NoSuchMethodException & e) //XInvocation::invoke
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const BridgeRuntimeError & e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception & e)
    {
        OUString message= "InterfaceOleWrapper_Impl::doInvoke : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch( ... )
     {
        OUString message= "InterfaceOleWrapper_Impl::doInvoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
         ret = DISP_E_EXCEPTION;
     }
    return ret;
}

HRESULT InterfaceOleWrapper_Impl::doGetProperty( DISPPARAMS * /*pdispparams*/, VARIANT * pvarResult,
                                                EXCEPINFO * pexcepinfo, OUString& name)
{
    HRESULT ret= S_OK;

    try
    {
        Any returnValue = m_xInvocation->getValue( name);
        // write back return value
        if (pvarResult)
            anyToVariant(pvarResult, returnValue);
    }
    catch(const UnknownPropertyException& e) //XInvocation::getValue
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const BridgeRuntimeError& e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception& e)
    {
        OUString message= "InterfaceOleWrapper_Impl::doGetProperty : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
    }
    catch( ... )
    {
        OUString message= "InterfaceOleWrapper_Impl::doInvoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
         ret = DISP_E_EXCEPTION;
    }
    return  ret;
}

HRESULT InterfaceOleWrapper_Impl::doSetProperty( DISPPARAMS * /*pdispparams*/, VARIANT * /*pvarResult*/,
                                        EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString& name, Sequence<Any> const & params)
{
    HRESULT ret= S_OK;

    try
    {
        m_xInvocation->setValue( name, params.getConstArray()[0]);
    }
    catch(const UnknownPropertyException &)
    {
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const CannotConvertException &e)
    {
        ret= mapCannotConvertException( e, puArgErr);
    }
    catch(const InvocationTargetException &e)
    {
        if (pexcepinfo != nullptr)
        {
            Any org = e.TargetException;

            pexcepinfo->wCode = UNO_2_OLE_EXCEPTIONCODE;
            pexcepinfo->bstrSource = SysAllocString(L"any ONE component");
            pexcepinfo->bstrDescription = SysAllocString(
                reinterpret_cast<LPCOLESTR>(org.getValueType().getTypeName().getStr()));
        }
        ret = DISP_E_EXCEPTION;
    }
    catch( ... )
    {
        ret= DISP_E_EXCEPTION;
    }
    return ret;
}

HRESULT InterfaceOleWrapper_Impl::InvokeGeneral( DISPID dispidMember, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * /*puArgErr*/, bool& bHandled)
{
    HRESULT ret= S_OK;
    try
    {
// DISPID_VALUE | The DEFAULT Value is required in JScript when the situation
// is that we put an object into an Array object ( out parameter). We have to return
// IDispatch otherwise the object cannot be accessed from the Script.
        if( dispidMember == DISPID_VALUE && wFlags == DISPATCH_PROPERTYGET
            && m_defaultValueType != VT_EMPTY && pvarResult != nullptr)
        {
            bHandled= true;
            if( m_defaultValueType == VT_DISPATCH)
            {
                pvarResult->vt= VT_DISPATCH;
                pvarResult->pdispVal= static_cast<IDispatch*>( this);
                AddRef();
                ret= S_OK;
            }
        }

        // function: _GetValueObject
        else if( dispidMember == DISPID_JSCRIPT_VALUE_FUNC)
        {
            bHandled= true;
            if( !pvarResult)
                ret= E_POINTER;
            CComObject< JScriptValue>* pValue;
            if( SUCCEEDED( CComObject<JScriptValue>::CreateInstance( &pValue)))
            {
                pValue->AddRef();
                pvarResult->vt= VT_DISPATCH;
#ifdef __MINGW32__
                pvarResult->pdispVal= CComQIPtr<IDispatch, &__uuidof(IDispatch)>(pValue->GetUnknown());
#else
                pvarResult->pdispVal= CComQIPtr<IDispatch>(pValue->GetUnknown());
#endif
                ret= S_OK;
            }
            else
                ret= DISP_E_EXCEPTION;
        }
        else if( dispidMember == DISPID_GET_STRUCT_FUNC)
        {
            bHandled= true;
            bool bStruct= false;


            Reference<XIdlReflection> xRefl = theCoreReflection::get(comphelper::getComponentContext(m_smgr));
            // the first parameter is in DISPPARAMS rgvargs contains the name of the struct.
            CComVariant arg;
            if( pdispparams->cArgs == 1 && SUCCEEDED( arg.ChangeType( VT_BSTR, &pdispparams->rgvarg[0])) )
            {
                Reference<XIdlClass> classStruct= xRefl->forName( reinterpret_cast<const sal_Unicode*>(arg.bstrVal));
                if( classStruct.is())
                {
                    Any anyStruct;
                    classStruct->createObject( anyStruct);
                    CComVariant var;
                    anyToVariant( &var, anyStruct );

                    if( var.vt == VT_DISPATCH)
                    {
                        VariantCopy( pvarResult, & var);
                        bStruct= true;
                    }
                }
            }
            ret= bStruct ? S_OK : DISP_E_EXCEPTION;
        }
        else if (dispidMember == DISPID_CREATE_TYPE_FUNC)
        {
            bHandled= true;
            if( !pvarResult)
                ret= E_POINTER;
            // the first parameter is in DISPPARAMS rgvargs contains the name of the struct.
            CComVariant arg;
            if( pdispparams->cArgs != 1)
                return DISP_E_BADPARAMCOUNT;
            if (FAILED( arg.ChangeType( VT_BSTR, &pdispparams->rgvarg[0])))
                return DISP_E_BADVARTYPE;

            //check if the provided name represents a valid type
            Type type;
            if (!getType(arg.bstrVal, type))
            {
                writeExcepinfo(pexcepinfo,OUString(
                                   "[automation bridge] A UNO type with the name " +
                                   OUString(reinterpret_cast<const sal_Unicode*>(arg.bstrVal)) + " does not exist!"));
                return DISP_E_EXCEPTION;
            }

            if (!createUnoTypeWrapper(arg.bstrVal, pvarResult))
            {
                writeExcepinfo(pexcepinfo, "[automation bridge] InterfaceOleWrapper_Impl::InvokeGeneral\n"
                                           "Could not initialize UnoTypeWrapper object!");
                return DISP_E_EXCEPTION;
            }
        }
    }
    catch(const BridgeRuntimeError & e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception & e)
    {
        OUString message= "InterfaceOleWrapper_Impl::InvokeGeneral : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch( ... )
     {
        OUString message= "InterfaceOleWrapper_Impl::InvokeGeneral : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
         ret = DISP_E_EXCEPTION;
     }
    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetDispID(BSTR /*bstrName*/, DWORD /*grfdex*/, DISPID __RPC_FAR* /*pid*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::InvokeEx(
    /* [in] */ DISPID /*id*/,
    /* [in] */ LCID /*lcid*/,
    /* [in] */ WORD /*wFlags*/,
    /* [in] */ DISPPARAMS __RPC_FAR* /*pdp*/,
    /* [out] */ VARIANT __RPC_FAR* /*pvarRes*/,
    /* [out] */ EXCEPINFO __RPC_FAR* /*pei*/,
    /* [unique][in] */ IServiceProvider __RPC_FAR* /*pspCaller*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::DeleteMemberByName(
    /* [in] */ BSTR /*bstr*/,
    /* [in] */ DWORD /*grfdex*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::DeleteMemberByDispID(DISPID /*id*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetMemberProperties(
    /* [in] */ DISPID /*id*/,
    /* [in] */ DWORD /*grfdexFetch*/,
    /* [out] */ DWORD __RPC_FAR* /*pgrfdex*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetMemberName(
    /* [in] */ DISPID /*id*/,
    /* [out] */ BSTR __RPC_FAR* /*pbstrName*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetNextDispID(
    /* [in] */ DWORD /*grfdex*/,
    /* [in] */ DISPID /*id*/,
    /* [out] */ DISPID __RPC_FAR* /*pid*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetNameSpaceParent(
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR* /*ppunk*/)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

// UnoObjectWrapperRemoteOpt ---------------------------------------------------

UnoObjectWrapperRemoteOpt::UnoObjectWrapperRemoteOpt( Reference<XMultiServiceFactory>& aFactory,
                                                     sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
InterfaceOleWrapper_Impl( aFactory, unoWrapperClass, comWrapperClass),
m_currentId(1)

{
}
UnoObjectWrapperRemoteOpt::~UnoObjectWrapperRemoteOpt()
{
}

// UnoConversionUtilities
Reference< XInterface > UnoObjectWrapperRemoteOpt::createUnoWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                                 m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

STDMETHODIMP  UnoObjectWrapperRemoteOpt::GetIDsOfNames ( REFIID /*riid*/, OLECHAR ** rgszNames, unsigned int cNames,
                                LCID /*lcid*/, DISPID * rgdispid )
{
    MutexGuard guard( getBridgeMutex());

    if( ! rgdispid)
        return E_POINTER;
    HRESULT ret = E_UNEXPECTED;

    // _GetValueObject
    if( ! wcscmp( *rgszNames, JSCRIPT_VALUE_FUNC))
    {
        *rgdispid= DISPID_JSCRIPT_VALUE_FUNC;
        return S_OK;
    }
    else if( ! wcscmp( *rgszNames, GET_STRUCT_FUNC))
    {
        *rgdispid= DISPID_GET_STRUCT_FUNC;
        return S_OK;
    }

    if (m_xInvocation.is() && (cNames > 0))
    {
        OUString name(reinterpret_cast<const sal_Unicode*>(rgszNames[0]));
        // has this name been determined as "bad"
        BadNameMap::iterator badIter= m_badNameMap.find( name);
        if( badIter == m_badNameMap.end() )
        {
            // name has not been bad before( member exists
            typedef NameToIdMap::iterator ITnames;
            pair< ITnames, bool > pair_id= m_nameToDispIdMap.insert( NameToIdMap::value_type(name, m_currentId++));
            // new ID inserted ?
            if( pair_id.second )
            {// yes, now create MemberInfo and ad to IdToMemberInfoMap
                MemberInfo d(0, name);
                m_idToMemberInfoMap.insert( IdToMemberInfoMap::value_type( m_currentId - 1, d));
            }

            *rgdispid = pair_id.first->second;
            ret = S_OK;
        }
        else
            ret= DISP_E_UNKNOWNNAME;
    }
    return ret;
}

STDMETHODIMP  UnoObjectWrapperRemoteOpt::Invoke ( DISPID dispidMember, REFIID /*riid*/, LCID /*lcid*/, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * puArgErr )
{
    HRESULT ret = S_OK;
    try
    {
        bool bHandled= false;
        ret= InvokeGeneral( dispidMember,  wFlags, pdispparams, pvarResult,  pexcepinfo,
                            puArgErr, bHandled);
        if( bHandled)
            return ret;

        if ( dispidMember > 0  && m_xInvocation.is())
        {

            IdToMemberInfoMap::iterator it_MemberInfo= m_idToMemberInfoMap.find( dispidMember);
            if( it_MemberInfo != m_idToMemberInfoMap.end() )
            {
                MemberInfo& info= it_MemberInfo->second;

                Sequence<Any> params; // holds converted any s
                if( ! info.flags )
                { // DISPID called for the first time
                    if( wFlags == DISPATCH_METHOD )
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );

                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                                  pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;
                    }
                    else if( wFlags == DISPATCH_PROPERTYPUT || wFlags == DISPATCH_PROPERTYPUTREF)
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        if( FAILED( ret= doSetProperty( pdispparams, pvarResult,
                                                        pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYGET;
                    }
                    else if( wFlags == DISPATCH_PROPERTYGET)
                    {
                        if( FAILED( ret= doGetProperty( pdispparams, pvarResult,
                                                        pexcepinfo, info.name))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doGetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, exactName)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT;
                    }
                    else if( wFlags & DISPATCH_METHOD &&
                             (wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF))
                    {

                        OUString exactName;
                        // convert params for DISPATCH_METHOD or DISPATCH_PROPERTYPUT
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        // try first as method
                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                                  pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;

                        // try as property
                        if( FAILED( ret) && pdispparams->cArgs == 1)
                        {
                            if( FAILED( ret= doSetProperty( pdispparams, pvarResult,
                                                            pexcepinfo, puArgErr, info.name, params))
                                && ret == DISP_E_MEMBERNOTFOUND)
                            {
                                // try to get the exact name
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                            if( SUCCEEDED( ret ) )
                                info.flags= DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYGET;
                        }
                    }
                    else if( wFlags & DISPATCH_METHOD && wFlags & DISPATCH_PROPERTYGET)
                    {
                        OUString exactName;
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );

                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                                  pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;

                        // try as property
                        if( FAILED( ret) && pdispparams->cArgs == 1)
                        {
                            if( FAILED( ret= doGetProperty( pdispparams, pvarResult,
                                                            pexcepinfo, info.name))
                                && ret == DISP_E_MEMBERNOTFOUND)
                            {
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                            if( SUCCEEDED( ret ) )
                                info.flags= DISPATCH_PROPERTYGET;
                        }
                    }

                    // update information about this member
                    if( ret == DISP_E_MEMBERNOTFOUND)
                    {
                        // Remember the name as not existing
                        // and remove the MemberInfo
                        m_badNameMap[info.name]= false;
                        m_idToMemberInfoMap.erase( it_MemberInfo);
                    }
                } // if( ! info.flags )
                else // IdToMemberInfoMap contains a MemberInfo
                {
                    if( wFlags & DISPATCH_METHOD && info.flags == DISPATCH_METHOD)
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        ret= doInvoke( pdispparams, pvarResult,
                                       pexcepinfo, puArgErr, info.name, params);
                    }
                    else if( (wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF )  &&
                             info.flags & DISPATCH_PROPERTYPUT)
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        ret= doSetProperty( pdispparams, pvarResult,
                                            pexcepinfo, puArgErr, info.name, params);
                    }
                    else if( (wFlags & DISPATCH_PROPERTYGET) && ( info.flags & DISPATCH_PROPERTYGET))
                    {
                        ret= doGetProperty( pdispparams, pvarResult,
                                            pexcepinfo, info.name);
                    }
                    else
                    {
                        ret= DISP_E_MEMBERNOTFOUND;
                    }
                }
            }//     if( it_MemberInfo != m_idToMemberInfoMap.end() )
            else
                ret= DISP_E_MEMBERNOTFOUND;
        }
    }
    catch(const BridgeRuntimeError& e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception& e)
    {
        OUString message= "UnoObjectWrapperRemoteOpt::Invoke : \n" +
            e.Message;
            writeExcepinfo(pexcepinfo, message);
            ret = DISP_E_EXCEPTION;
    }
    catch(...)
    {
        OUString message= "UnoObjectWrapperRemoteOpt::Invoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }

    return ret;
}

HRESULT UnoObjectWrapperRemoteOpt::methodInvoke( DISPID /*dispidMember*/, DISPPARAMS * /*pdispparams*/, VARIANT * /*pvarResult*/,
                              EXCEPINFO * /*pexcepinfo*/, unsigned int * /*puArgErr*/, Sequence<Any> const &)
{
    return S_OK;
}

// The returned HRESULT is only appropriate for IDispatch::Invoke
static HRESULT mapCannotConvertException(const CannotConvertException &e, unsigned int * puArgErr)
{
    HRESULT ret;
    bool bWriteIndex= true;

    switch ( e.Reason)
    {
        case FailReason::OUT_OF_RANGE:
            ret = DISP_E_OVERFLOW;
            break;
        case FailReason::IS_NOT_NUMBER:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::IS_NOT_ENUM:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::IS_NOT_BOOL:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::NO_SUCH_INTERFACE:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::SOURCE_IS_NO_DERIVED_TYPE:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::TYPE_NOT_SUPPORTED:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::INVALID:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::NO_DEFAULT_AVAILABLE:
            ret = DISP_E_BADPARAMCOUNT;
            break;
        case FailReason::UNKNOWN:
            ret = E_UNEXPECTED;
            break;
        default:
            ret = E_UNEXPECTED;
            bWriteIndex= false;
            break;
    }

    if( bWriteIndex &&  puArgErr != nullptr)
        *puArgErr = e.ArgumentIndex;
    return ret;
}

// The function maps the TypeClass of the any to VARTYPE: If
// the Any contains STRUCT or INTERFACE then the return value
// is VT_DISPATCH. The function is used from o2u_createUnoObjectWrapper
// and the result is put into the constructor of the uno - wrapper
// object. If a client asks the object for DISPID_VALUE and this
// function returned VT_DISPATCH then the IDispatch of the same
// object is being returned.
// See InterfaceOleWrapper_Impl::Invoke, InterfaceOleWrapper_Impl::m_defaultValueType
VARTYPE getVarType( const Any& value)
{
    VARTYPE ret= VT_EMPTY;

    switch ( value.getValueTypeClass())
    {
    case TypeClass_STRUCT: ret= VT_DISPATCH; break;
    case TypeClass_INTERFACE: ret= VT_DISPATCH; break;
    default: break;
    }
    return ret;
}

} // end namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
