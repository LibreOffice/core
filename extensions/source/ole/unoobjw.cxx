/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ole2uno.hxx"

#include <stdio.h>
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>

#include <osl/diagnose.h>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
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
#include <cppuhelper/implbase1.hxx>

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
boost::unordered_map<sal_uInt32, WeakReference<XInterface> > UnoObjToWrapperMap;
static sal_Bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource);
static sal_Bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource);
static HRESULT mapCannotConvertException(const CannotConvertException &e, unsigned int * puArgErr);

/* Does not throw any exceptions.
   Param pInfo can be NULL.
 */
static void writeExcepinfo(EXCEPINFO * pInfo, const OUString& message)
{
    if (pInfo != NULL)
    {
        pInfo->wCode = UNO_2_OLE_EXCEPTIONCODE;
        pInfo->bstrSource = SysAllocString(L"[automation bridge] ");
        pInfo->bstrDescription = SysAllocString(reinterpret_cast<LPCOLESTR>(message.getStr()));
    }
}

InterfaceOleWrapper_Impl::InterfaceOleWrapper_Impl( Reference<XMultiServiceFactory>& xFactory,
                                                    sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
        m_defaultValueType( 0),
        UnoConversionUtilities<InterfaceOleWrapper_Impl>( xFactory, unoWrapperClass, comWrapperClass)
{
}

InterfaceOleWrapper_Impl::~InterfaceOleWrapper_Impl()
{
    MutexGuard guard(getBridgeMutex());
    
    IT_Uno it= UnoObjToWrapperMap.find( (sal_uInt32) m_xOrigin.get());
    if(it != UnoObjToWrapperMap.end())
        UnoObjToWrapperMap.erase(it);
#if OSL_DEBUG_LEVEL > 0
    fprintf(stderr,"[automation bridge] UnoObjToWrapperMap  contains: %i \n",
            UnoObjToWrapperMap.size());
#endif
}

STDMETHODIMP InterfaceOleWrapper_Impl::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    HRESULT ret= S_OK;

    if( !ppv)
        return E_POINTER;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = (IUnknown*) (IDispatch*) this;
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        AddRef();
        *ppv = (IDispatch*) this;
    }
    else if( IsEqualIID( riid, __uuidof( IUnoObjectWrapper)))
    {
        AddRef();
        *ppv= (IUnoObjectWrapper*) this;
    }
    else
        ret= E_NOINTERFACE;
    return ret;
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper_Impl::AddRef()
{
    acquire();
    
    
    return m_refCount;
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper_Impl::Release()
{
    ULONG n= m_refCount;
    release();
    return n - 1;
}


STDMETHODIMP InterfaceOleWrapper_Impl::getWrapperXInterface( Reference<XInterface>* pXInt)
{
    *pXInt= Reference<XInterface>( static_cast<XWeak*>( this), UNO_QUERY);
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
        OSL_ASSERT(0);
    }
    catch(const Exception&)
    {
        OSL_ASSERT(0);
    }
    catch(...)
    {
        OSL_ASSERT(0);
    }

    return ret;
}






































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
         { 
            pParams[countArgs - (i + 1)] = anyParam;
             continue;
         }

        
        
        
        
        
        CComVariant varParam;

        
        
        if( pdispparams->rgvarg[i].vt == VT_DISPATCH )
        {
            if( info.eMemberType == MemberType_METHOD && info.aParamModes[ countArgs - i -1 ]  == ParamMode_INOUT)
            {
                
                
                
                
                IDispatch* pdisp= pdispparams->rgvarg[i].pdispVal;

                OLECHAR* sindex= L"0";
                DISPID id;
                DISPPARAMS noParams= {0,0,0,0};
                if(SUCCEEDED( hr= pdisp->GetIDsOfNames( IID_NULL, &sindex, 1, LOCALE_USER_DEFAULT, &id)))
                    hr= pdisp->Invoke( id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                       & noParams, & varParam, NULL, NULL);
                if( FAILED( hr))
                {
                    throw BridgeRuntimeError(
                        "[automation bridge] Could not determine "
                        "if the object has a member \"0\". Error: " +
                        OUString::number(hr));
                }
            }
        }

        if( varParam.vt == VT_EMPTY) 
                 varParam= pdispparams->rgvarg[i];

        if(info.eMemberType == MemberType_METHOD)
            variantToAny( & varParam, anyParam,
                           info.aParamTypes[ countArgs - i - 1]);
        else if(info.eMemberType == MemberType_PROPERTY)
            variantToAny( & varParam, anyParam, info.aType);
        else
            OSL_ASSERT(0);

        pParams[countArgs - (i + 1)]= anyParam;
    }
}

sal_Bool  InterfaceOleWrapper_Impl::getInvocationInfoForCall( DISPID id, InvocationInfo& info)
{
    sal_Bool bTypesAvailable= sal_False;

    if( !m_xInvocation.is() )return false;
    Reference<XInvocation2> inv2( m_xInvocation, UNO_QUERY);
    if( inv2.is())
    {
        
        
        
        
        
        
        
        
        
        typedef NameToIdMap::const_iterator cit;
        OUString sMemberName;

        for(cit ci1= m_nameToDispIdMap.begin(); ci1 != m_nameToDispIdMap.end(); ++ci1)
        {
            if( (*ci1).second == id) 
            {
                sMemberName= (*ci1).first;
                break;
            }
        }
        
        
        
        
        
        sal_Bool validInfo= sal_True;
        InvocationInfo invInfo;
        try{
            invInfo= inv2->getInfoForName( sMemberName, sal_False);
        }
        catch(const IllegalArgumentException&)
        {
            validInfo= sal_False;
        }

        if( ! validInfo)
        {
            invInfo= inv2->getInfoForName( sMemberName, sal_True);
        }
        if( invInfo.aName.pData)
        {
            bTypesAvailable= sal_True;
            info= invInfo;
        }
    }
    return bTypesAvailable;
}






Any SAL_CALL InterfaceOleWrapper_Impl::createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& /*ProcessId*/,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType)
            throw (IllegalArgumentException, RuntimeException)
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
                VARIANT *pVar= (VARIANT*)CoTaskMemAlloc( sizeof( VARIANT));
                if( pVar)
                {
                    pVar->vt= VT_DISPATCH;
                    pVar->pdispVal= static_cast<IDispatch*>( this);
                    AddRef();

                    retAny<<= reinterpret_cast< sal_uInt32 >( pVar);
                }
            }
        }
    }

    return retAny;
}


void SAL_CALL InterfaceOleWrapper_Impl::initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
{
    switch( aArguments.getLength() )
    {
    case 2: 
        aArguments[0] >>= m_xInvocation;
        aArguments[1] >>= m_defaultValueType;
        break;
    case 3: 
        aArguments[0] >>= m_xInvocation;
        aArguments[1] >>= m_xOrigin;
        aArguments[2] >>= m_defaultValueType;
        break;
    }

    m_xExactName= Reference<XExactName>( m_xInvocation, UNO_QUERY);
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



bool getType( const BSTR name, Type & type)
{
    Type retType;
    bool ret = false;
    typelib_TypeDescription * pDesc= NULL;
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

static sal_Bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource)
{
    sal_Bool ret = sal_False;
    HRESULT hr;

    
    CComVariant varDest( *pDest);

    if( SUCCEEDED( varDest.ChangeType(VT_DISPATCH)))
    {
        CComPtr<IDispatch> spDispDest(varDest.pdispVal);

        
#ifdef __MINGW32__
        CComQIPtr<IJScriptValueObject, &__uuidof(IJScriptValueObject)> spValueDest(spDispDest);
#else
        CComQIPtr<IJScriptValueObject> spValueDest(spDispDest);
#endif
        if (spValueDest)
        {
            VARIANT_BOOL varBool= VARIANT_FALSE;
            if( SUCCEEDED( hr= spValueDest->IsOutParam( &varBool) )
                && varBool == VARIANT_TRUE  ||
                SUCCEEDED(hr= spValueDest->IsInOutParam( &varBool) )
                && varBool == VARIANT_TRUE )
            {
                if( SUCCEEDED( spValueDest->Set( CComVariant(), *pSource)))
                    ret= sal_True;
            }
        }
        else if (pDest->vt == VT_DISPATCH)
        {
            
            
            
            
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
                    DISPPARAMS dispparams = {NULL, NULL, 1, 1};
                    dispparams.rgvarg = pSource;
                    DISPID dispidPut = DISPID_PROPERTYPUT;
                    dispparams.rgdispidNamedArgs = &dispidPut;

                    if (pSource->vt == VT_UNKNOWN || pSource->vt == VT_DISPATCH ||
                        (pSource->vt & VT_ARRAY) || (pSource->vt & VT_BYREF))
                        hr = spDispEx->InvokeEx(dwDispID, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF,
                                                &dispparams, NULL, NULL, NULL);
                    else
                        hr= spDispEx->InvokeEx(dwDispID, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
                                               &dispparams, NULL, NULL, NULL);
                    if( SUCCEEDED(hr))
                        ret= sal_True;
                }
            }
        }
        else
            ret= writeBackOutParameter( pDest, pSource);
    }
    else 
    {   
        ret= writeBackOutParameter( pDest, pSource);
    }
    return ret;
}





static sal_Bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource)
{
    HRESULT hr;
    sal_Bool ret = FALSE;
    
    if ((V_VT(pDest) & VT_BYREF) != 0 )
    {
        VARTYPE oleTypeFlags = V_VT(pSource);

        
        if (V_VT(pDest) == (VT_VARIANT | VT_BYREF))
        {
            
            
            
            
            VariantCopy(V_VARIANTREF(pDest), pSource);
            ret = sal_True;
        }
        else
        {
            
              if ((V_VT(pDest) & oleTypeFlags) == oleTypeFlags)
            {
                if ((oleTypeFlags & VT_ARRAY) != 0)
                {
                    
                    if( *V_ARRAYREF(pDest) != NULL)
                        hr= SafeArrayCopyData( V_ARRAY(pSource), *V_ARRAYREF(pDest));
                    else
                        
                        hr= SafeArrayCopy(V_ARRAY(pSource), V_ARRAYREF(pDest)) == NOERROR;
                    if( SUCCEEDED( hr))
                        ret = sal_True;
                }
                else
                {
                    
                    switch (V_VT(pSource))
                    {
                    case VT_I2:
                    {
                        *V_I2REF(pDest) = V_I2(pSource);
                        ret = sal_True;
                        break;
                    }
                    case VT_I4:
                        *V_I4REF(pDest) = V_I4(pSource);
                        ret = sal_True;
                        break;
                    case VT_R4:
                        *V_R4REF(pDest) = V_R4(pSource);
                        ret = sal_True;
                        break;
                    case VT_R8:
                        *V_R8REF(pDest) = V_R8(pSource);
                        ret = sal_True;
                        break;
                    case VT_CY:
                        *V_CYREF(pDest) = V_CY(pSource);
                        ret = sal_True;
                        break;
                    case VT_DATE:
                        *V_DATEREF(pDest) = V_DATE(pSource);
                        ret = sal_True;
                        break;
                    case VT_BSTR:
                        SysFreeString( *pDest->pbstrVal);

                        *V_BSTRREF(pDest) = SysAllocString(V_BSTR(pSource));
                        ret = sal_True;
                        break;
                    case VT_DISPATCH:
                        if (*V_DISPATCHREF(pDest) != NULL)
                            (*V_DISPATCHREF(pDest))->Release();

                        *V_DISPATCHREF(pDest) = V_DISPATCH(pSource);

                        if (*V_DISPATCHREF(pDest) != NULL)
                            (*V_DISPATCHREF(pDest))->AddRef();

                        ret = sal_True;
                        break;
                    case VT_ERROR:
                        *V_ERRORREF(pDest) = V_ERROR(pSource);
                        ret = sal_True;
                        break;
                    case VT_BOOL:
                        *V_BOOLREF(pDest) = V_BOOL(pSource);
                        ret = sal_True;
                        break;
                    case VT_UNKNOWN:
                        if (*V_UNKNOWNREF(pDest) != NULL)
                            (*V_UNKNOWNREF(pDest))->Release();

                        *V_UNKNOWNREF(pDest) = V_UNKNOWN(pSource);

                        if (*V_UNKNOWNREF(pDest) != NULL)
                            (*V_UNKNOWNREF(pDest))->AddRef();

                        ret = sal_True;
                        break;
                    case VT_I1:
                        *V_I1REF(pDest) = V_I1(pSource);
                        ret = sal_True;
                        break;
                    case VT_UI1:
                        *V_UI1REF(pDest) = V_UI1(pSource);
                        ret = sal_True;
                        break;
                    case VT_UI2:
                        *V_UI2REF(pDest) = V_UI2(pSource);
                        ret = sal_True;
                        break;
                    case VT_UI4:
                        *V_UI4REF(pDest) = V_UI4(pSource);
                        ret = sal_True;
                        break;
                    case VT_INT:
                        *V_INTREF(pDest) = V_INT(pSource);
                        ret = sal_True;
                        break;
                    case VT_UINT:
                        *V_UINTREF(pDest) = V_UINT(pSource);
                        ret = sal_True;
                        break;
                    case VT_DECIMAL:
                        memcpy(pDest->pdecVal, pSource, sizeof(DECIMAL));
                        ret = sal_True;
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                
                
                if( pDest->vt == (VT_BSTR | VT_BYREF)
                    && pSource->vt == VT_I2)
                {
                    
                    
                    
                    OLECHAR buff[]= {0,0};
                    buff[0]= pSource->iVal;

                    SysFreeString( *pDest->pbstrVal);
                    *pDest->pbstrVal= SysAllocString( buff);
                    ret = sal_True;
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
        sal_Bool bHandled= sal_False;
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
                else if ((flags & DISPATCH_PROPERTYPUT || flags & DISPATCH_PROPERTYPUTREF) != 0)
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

        
        returnValue = m_xInvocation->invoke(name,
                                            params,
                                            outIndex,
                                            outParams);

        
        if (outIndex.getLength() > 0)
        {
            const sal_Int16* pOutIndex = outIndex.getConstArray();
            const Any* pOutParams = outParams.getConstArray();

            for (sal_Int32 i = 0; i < outIndex.getLength(); i++)
            {
                CComVariant variant;
                
                anyToVariant( &variant, pOutParams[i]);

                
                
                int outindex= pOutIndex[i];
                writeBackOutParameter2(&(pdispparams->rgvarg[pdispparams->cArgs - 1 - outindex]),
                                       &variant );
            }
        }

        
        if (pvarResult != NULL)
            anyToVariant(pvarResult, returnValue);
    }
    catch(const IllegalArgumentException & e) 
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = DISP_E_TYPEMISMATCH;
    }
    catch(const CannotConvertException & e) 
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = mapCannotConvertException( e, puArgErr);
    }
    catch(const InvocationTargetException &  e) 
    {
        const Any& org = e.TargetException;
        Exception excTarget;
        org >>= excTarget;
        OUString message=
            org.getValueType().getTypeName() + ": " + excTarget.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const NoSuchMethodException & e) 
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

    Any value;
    try
    {
        Any returnValue = m_xInvocation->getValue( name);
        
        if (pvarResult)
            anyToVariant(pvarResult, returnValue);
    }
    catch(const UnknownPropertyException& e) 
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
                                        EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString& name, Sequence<Any> params)
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
        if (pexcepinfo != NULL)
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
                         unsigned int * /*puArgErr*/, sal_Bool& bHandled)
{
    HRESULT ret= S_OK;
    try
    {



        if( dispidMember == DISPID_VALUE && wFlags == DISPATCH_PROPERTYGET
            && m_defaultValueType != VT_EMPTY && pvarResult != NULL)
        {
            bHandled= sal_True;
            if( m_defaultValueType == VT_DISPATCH)
            {
                pvarResult->vt= VT_DISPATCH;
                pvarResult->pdispVal= static_cast<IDispatch*>( this);
                AddRef();
                ret= S_OK;
            }
        }

        
        else if( dispidMember == DISPID_JSCRIPT_VALUE_FUNC)
        {
            bHandled= sal_True;
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
            bHandled= sal_True;
            sal_Bool bStruct= sal_False;


            Reference<XIdlReflection> xRefl = theCoreReflection::get(comphelper::getComponentContext(m_smgr));
            
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
                        bStruct= sal_True;
                    }
                }
            }
            ret= bStruct == sal_True ? S_OK : DISP_E_EXCEPTION;
        }
        else if (dispidMember == DISPID_CREATE_TYPE_FUNC)
        {
            bHandled= sal_True;
            if( !pvarResult)
                ret= E_POINTER;
            
            CComVariant arg;
            if( pdispparams->cArgs != 1)
                return DISP_E_BADPARAMCOUNT;
            if (FAILED( arg.ChangeType( VT_BSTR, &pdispparams->rgvarg[0])))
                return DISP_E_BADVARTYPE;

            
            Type type;
            if (getType(arg.bstrVal, type) == false)
            {
                writeExcepinfo(pexcepinfo,OUString(
                                   "[automation bridge] A UNO type with the name " +
                                   OUString(reinterpret_cast<const sal_Unicode*>(arg.bstrVal)) + " does not exist!"));
                return DISP_E_EXCEPTION;
            }

            if (createUnoTypeWrapper(arg.bstrVal, pvarResult) == false)
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



UnoObjectWrapperRemoteOpt::UnoObjectWrapperRemoteOpt( Reference<XMultiServiceFactory>& aFactory,
                                                     sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
InterfaceOleWrapper_Impl( aFactory, unoWrapperClass, comWrapperClass),
m_currentId(1)

{
}
UnoObjectWrapperRemoteOpt::~UnoObjectWrapperRemoteOpt()
{
}


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
        
        BadNameMap::iterator badIter= m_badNameMap.find( name);
        if( badIter == m_badNameMap.end() )
        {
            
            typedef NameToIdMap::iterator ITnames;
            pair< ITnames, bool > pair_id= m_nameToDispIdMap.insert( NameToIdMap::value_type(name, m_currentId++));
            
            if( pair_id.second )
            {
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
        sal_Bool bHandled= sal_False;
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

                Sequence<Any> params; 
                if( ! info.flags )
                { 
                    if( wFlags == DISPATCH_METHOD )
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );

                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                
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
                            
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                
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
                            
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                
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
                        
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        
                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                
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

                        
                        if( FAILED( ret) && pdispparams->cArgs == 1)
                        {
                            if( FAILED( ret= doSetProperty( pdispparams, pvarResult,
                                                            pexcepinfo, puArgErr, info.name, params))
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
                            
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                
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

                    
                    if( ret == DISP_E_MEMBERNOTFOUND)
                    {
                        
                        
                        m_badNameMap[info.name]= sal_False;
                        m_idToMemberInfoMap.erase( it_MemberInfo);
                    }
                } 
                else 
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
            }
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
                              EXCEPINFO * /*pexcepinfo*/, unsigned int * /*puArgErr*/, Sequence<Any> params)
{
    return S_OK;
}


static HRESULT mapCannotConvertException(const CannotConvertException &e, unsigned int * puArgErr)
{
    HRESULT ret;
    sal_Bool bWriteIndex= sal_True;

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
            bWriteIndex= sal_False;
            break;
    }

    if( bWriteIndex &&  puArgErr != NULL)
        *puArgErr = e.ArgumentIndex;
    return ret;
}









const VARTYPE getVarType( const Any& value)
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
