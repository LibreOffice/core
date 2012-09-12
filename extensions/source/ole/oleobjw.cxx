/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "ole2uno.hxx"
#include "rtl/ustrbuf.hxx"


#include "osl/diagnose.h"
#include "osl/doublecheckedlocking.h"
#include "osl/thread.h"

#include "boost/scoped_array.hpp"
#include <com/sun/star/script/FailReason.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/FinishEngineEvent.hpp>
#include <com/sun/star/script/InterruptReason.hpp>
#include <com/sun/star/script/XEngineListener.hpp>
#include <com/sun/star/script/XDebugging.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/ContextInformation.hpp>
#include <com/sun/star/script/FinishReason.hpp>
#include <com/sun/star/script/XEngine.hpp>
#include <com/sun/star/script/InterruptEngineEvent.hpp>
#include <com/sun/star/script/XLibraryAccess.hpp>
#include <com/sun/star/bridge/ModelDependent.hpp>

#include "com/sun/star/bridge/oleautomation/NamedArgument.hpp"
#include "com/sun/star/bridge/oleautomation/PropertyPutArgument.hpp"

#include <typelib/typedescription.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>
#include <rtl/ustring.hxx>

#include "jscriptclasses.hxx"

#include "oleobjw.hxx"
#include "unoobjw.hxx"
#include <stdio.h>
using namespace std;
using namespace boost;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::script;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::oleautomation;
using namespace com::sun::star::bridge::ModelDependent;
using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringBuffer;

#define JSCRIPT_ID_PROPERTY L"_environment"
#define JSCRIPT_ID          L"jscript"
namespace ole_adapter
{


// key: XInterface pointer created by Invocation Adapter Factory
// value: XInterface pointer to the wrapper class.
// Entries to the map are made within
// Any createOleObjectWrapper(IUnknown* pUnknown, const Type& aType);
// Entries are being deleted if the wrapper class's destructor has been
// called.
// Before UNO object is wrapped to COM object this map is checked
// to see if the UNO object is already a wrapper.
boost::unordered_map<sal_uInt32, sal_uInt32> AdapterToWrapperMap;
// key: XInterface of the wrapper object.
// value: XInterface of the Interface created by the Invocation Adapter Factory.
// A COM wrapper is responsible for removing the corresponding entry
// in AdapterToWrappperMap if it is being destroyed. Because the wrapper does not
// know about its adapted interface it uses WrapperToAdapterMap to get the
// adapted interface which is then used to locate the entry in AdapterToWrapperMap.
boost::unordered_map<sal_uInt32,sal_uInt32> WrapperToAdapterMap;

boost::unordered_map<sal_uInt32, WeakReference<XInterface> > ComPtrToWrapperMap;
/*****************************************************************************

    class implementation IUnknownWrapper_Impl

*****************************************************************************/

IUnknownWrapper_Impl::IUnknownWrapper_Impl( Reference<XMultiServiceFactory>& xFactory,
                                           sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
    UnoConversionUtilities<IUnknownWrapper_Impl>( xFactory, unoWrapperClass, comWrapperClass),
    m_pxIdlClass( NULL), m_eJScript( JScriptUndefined),
    m_bComTlbIndexInit(false),  m_bHasDfltMethod(false), m_bHasDfltProperty(false)
{
}


IUnknownWrapper_Impl::~IUnknownWrapper_Impl()
{
    o2u_attachCurrentThread();
    MutexGuard guard(getBridgeMutex());
    XInterface * xIntRoot = (OWeakObject *)this;
#if OSL_DEBUG_LEVEL > 0
    acquire(); // make sure we don't delete us twice because of Reference
    OSL_ASSERT( Reference<XInterface>( static_cast<XWeak*>(this), UNO_QUERY).get() == xIntRoot );
#endif

    // remove entries in global maps
    typedef boost::unordered_map<sal_uInt32, sal_uInt32>::iterator _IT;
    _IT it= WrapperToAdapterMap.find( (sal_uInt32) xIntRoot);
    if( it != WrapperToAdapterMap.end())
    {
        sal_uInt32 adapter= it->second;

        AdapterToWrapperMap.erase( adapter);
        WrapperToAdapterMap.erase( it);
    }

     IT_Com it_c= ComPtrToWrapperMap.find( (sal_uInt32) m_spUnknown.p);
    if(it_c != ComPtrToWrapperMap.end())
        ComPtrToWrapperMap.erase(it_c);

#if OSL_DEBUG_LEVEL > 0
    fprintf(stderr,"[automation bridge] ComPtrToWrapperMap  contains: %i \n",
            ComPtrToWrapperMap.size());
#endif
}

Any IUnknownWrapper_Impl::queryInterface(const Type& t)
    throw (RuntimeException)
{
    if (t == getCppuType(static_cast<Reference<XDefaultMethod>*>( 0)) && !m_bHasDfltMethod )
        return Any();
    if (t == getCppuType(static_cast<Reference<XDefaultProperty>*>( 0)) && !m_bHasDfltProperty )
        return Any();
    if ( ( t == getCppuType(static_cast<Reference<XInvocation>*>( 0)) || t == getCppuType(static_cast<Reference<XAutomationInvocation>*>( 0)) ) && !m_spDispatch)
        return Any();
    // XDirectInvocation seems to be an oracle replacement for XAutomationInvocation, however it is flawed esecially wrt. assumptions about whether to invoke a
    // Put or Get property, the implementation code has no business guessing that, it's up to the caller to decide that. Worse XDirectInvocation duplicates lots of code.
    // XAutomationInvocation provides seperate calls for put& get
    // properties. Note: Currently the basic runtime doesn't call put properties directly, it should... after all the basic runtime should know whether it is calling a put or get property.
    // For the moment for ease of merging we will let the XDirectInvoke and XAuthomationInvocation interfaces stay side by side ( and for the momemnt at least I would prefer the basic
    // runtime to call XAutomationInvocation instead of XDirectInvoke
    return WeakImplHelper7<XBridgeSupplier2,
        XInitialization, XAutomationObject, XDefaultProperty, XDefaultMethod, XDirectInvocation, XAutomationInvocation >::queryInterface(t);
}

Reference<XIntrospectionAccess> SAL_CALL IUnknownWrapper_Impl::getIntrospection(void)
    throw (RuntimeException )
{
    Reference<XIntrospectionAccess> ret;

    return ret;
}

Any SAL_CALL IUnknownWrapper_Impl::invokeGetProperty( const OUString& aPropertyName, const Sequence< Any >& aParams, Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
{
    Any aResult;
    try
    {
        o2u_attachCurrentThread();
        ITypeInfo * pInfo = getTypeInfo();
        FuncDesc aDescGet(pInfo);
        FuncDesc aDescPut(pInfo);
        VarDesc aVarDesc(pInfo);
        getPropDesc(aPropertyName, & aDescGet, & aDescPut, & aVarDesc);
        if ( !aDescGet )
        {
            OUString msg(OUSTR("[automation bridge]Property \"") + aPropertyName +
                OUSTR("\" is not supported"));
            throw UnknownPropertyException(msg, Reference<XInterface>());
        }
        aResult = invokeWithDispIdComTlb( aDescGet, aPropertyName, aParams, aOutParamIndex, aOutParam );
    }
    catch ( const Exception& e )
    {
       throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
               "IUnknownWrapper_Impl::invokeGetProperty ! Message : \n") +
                e.Message, Reference<XInterface>());
    }
    return aResult;
}

Any SAL_CALL IUnknownWrapper_Impl::invokePutProperty( const OUString& aPropertyName, const Sequence< Any >& aParams, Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
{
    Any aResult;
    try
    {
        o2u_attachCurrentThread();
        ITypeInfo * pInfo = getTypeInfo();
        FuncDesc aDescGet(pInfo);
        FuncDesc aDescPut(pInfo);
        VarDesc aVarDesc(pInfo);
        getPropDesc(aPropertyName, & aDescGet, & aDescPut, & aVarDesc);
        if ( !aDescPut )
        {
            OUString msg(OUSTR("[automation bridge]Property \"") + aPropertyName +
                OUSTR("\" is not supported"));
            throw UnknownPropertyException(msg, Reference<XInterface>());
        }
        aResult = invokeWithDispIdComTlb( aDescPut, aPropertyName, aParams, aOutParamIndex, aOutParam );
    }
    catch ( const Exception& e )
    {
       throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
               "IUnknownWrapper_Impl::invokePutProperty ! Message : \n") +
                e.Message, Reference<XInterface>());
    }
    return aResult;
}


Any SAL_CALL IUnknownWrapper_Impl::invoke( const OUString& aFunctionName,
             const Sequence< Any >& aParams, Sequence< sal_Int16 >& aOutParamIndex,
             Sequence< Any >& aOutParam )
    throw(IllegalArgumentException, CannotConvertException, InvocationTargetException,
          RuntimeException)
{
    if ( ! m_spDispatch )
    {
        throw RuntimeException(
            OUSTR("[automation bridge] The object does not have an IDispatch interface"),
            Reference<XInterface>());
    }

    Any ret;

    try
    {
        o2u_attachCurrentThread();

        TypeDescription methodDesc;
        getMethodInfo(aFunctionName, methodDesc);
        if( methodDesc.is())
        {
            ret = invokeWithDispIdUnoTlb(aFunctionName,
                                         aParams,
                                         aOutParamIndex,
                                         aOutParam);
        }
        else
        {
            ret= invokeWithDispIdComTlb( aFunctionName,
                                         aParams,
                                         aOutParamIndex,
                                         aOutParam);
        }
    }
    catch (const IllegalArgumentException &)
    {
        throw;
    }
    catch (const CannotConvertException &)
    {
        throw;
    }
    catch (const BridgeRuntimeError & e)
    {
         throw RuntimeException(e.message, Reference<XInterface>());
    }
    catch (const Exception & e)
    {
        throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
                                     "IUnknownWrapper_Impl::invoke ! Message : \n") +
                               e.Message, Reference<XInterface>());

    }
    catch(...)
    {
        throw RuntimeException(
            OUSTR("[automation bridge] unexpected exception in "
                  "IUnknownWrapper_Impl::Invoke !"), Reference<XInterface>());
    }
    return ret;
}

void SAL_CALL IUnknownWrapper_Impl::setValue( const OUString& aPropertyName,
                 const Any& aValue )
    throw(UnknownPropertyException, CannotConvertException, InvocationTargetException,
          RuntimeException)
{
    if ( ! m_spDispatch )
    {
        throw RuntimeException(
            OUSTR("[automation bridge] The object does not have an IDispatch interface"),
            Reference<XInterface>());
    }
    try
    {
        o2u_attachCurrentThread();

        ITypeInfo * pInfo = getTypeInfo();
        FuncDesc aDescGet(pInfo);
        FuncDesc aDescPut(pInfo);
        VarDesc aVarDesc(pInfo);
        getPropDesc(aPropertyName, & aDescGet, & aDescPut, & aVarDesc);
        //check if there is such a property at all or if it is read only
        if ( ! aDescPut && ! aDescGet && ! aVarDesc)
        {
            OUString msg(OUSTR("[automation bridge]Property \"") + aPropertyName +
                         OUSTR("\" is not supported"));
            throw UnknownPropertyException(msg, Reference<XInterface>());
        }

        if ( (! aDescPut && aDescGet) || aVarDesc
             && aVarDesc->wVarFlags == VARFLAG_FREADONLY )
        {
            //read-only
            OUString msg(OUSTR("[automation bridge] Property ") + aPropertyName +
                         OUSTR(" is read-only"));
            OString sMsg = OUStringToOString(msg, osl_getThreadTextEncoding());
            OSL_FAIL(sMsg.getStr());
            // ignore silently
            return;
        }

        HRESULT hr= S_OK;
        DISPPARAMS dispparams;
        CComVariant varArg;
        CComVariant varRefArg;
        CComVariant varResult;
        ExcepInfo excepinfo;
        unsigned int uArgErr;

        // converting UNO value to OLE variant
        DISPID dispidPut= DISPID_PROPERTYPUT;
        dispparams.rgdispidNamedArgs = &dispidPut;
        dispparams.cArgs = 1;
        dispparams.cNamedArgs = 1;
        dispparams.rgvarg = & varArg;

        OSL_ASSERT(aDescPut || aVarDesc);

        VARTYPE vt = 0;
        DISPID dispid = 0;
        INVOKEKIND invkind = INVOKE_PROPERTYPUT;
        //determine the expected type, dispid, invoke kind (DISPATCH_PROPERTYPUT,
        //DISPATCH_PROPERTYPUTREF)
        if (aDescPut)
        {
            vt = getElementTypeDesc(& aDescPut->lprgelemdescParam[0].tdesc);
            dispid = aDescPut->memid;
            invkind = aDescPut->invkind;
        }
        else
        {
            vt = getElementTypeDesc( & aVarDesc->elemdescVar.tdesc);
            dispid = aVarDesc->memid;
            if (vt == VT_UNKNOWN || vt == VT_DISPATCH ||
                (vt & VT_ARRAY) || (vt & VT_BYREF))
            {
                invkind = INVOKE_PROPERTYPUTREF;
            }
        }

        // convert the uno argument
        if (vt & VT_BYREF)
        {
            anyToVariant( & varRefArg, aValue, ::sal::static_int_cast< VARTYPE, int >( vt ^ VT_BYREF ) );
            varArg.vt = vt;
            if( (vt & VT_TYPEMASK) == VT_VARIANT)
                varArg.byref = & varRefArg;
            else if ((vt & VT_TYPEMASK) == VT_DECIMAL)
                varArg.byref = & varRefArg.decVal;
            else
                varArg.byref = & varRefArg.byref;
        }
        else
        {
            anyToVariant(& varArg, aValue, vt);
        }
        // call to IDispatch
        hr = m_spDispatch->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, ::sal::static_int_cast< WORD, INVOKEKIND >( invkind ),
                                 &dispparams, & varResult, & excepinfo, &uArgErr);

        // lookup error code
        switch (hr)
        {
        case S_OK:
            break;
        case DISP_E_BADPARAMCOUNT:
            throw RuntimeException();
            break;
        case DISP_E_BADVARTYPE:
            throw RuntimeException();
            break;
        case DISP_E_EXCEPTION:
            throw InvocationTargetException();
            break;
        case DISP_E_MEMBERNOTFOUND:
            throw UnknownPropertyException();
            break;
        case DISP_E_NONAMEDARGS:
            throw RuntimeException();
            break;
        case DISP_E_OVERFLOW:
            throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")), static_cast<XInterface*>(
                                             static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::OUT_OF_RANGE, uArgErr);
            break;
        case DISP_E_PARAMNOTFOUND:
            throw IllegalArgumentException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")), static_cast<XInterface*>(
                                            static_cast<XWeak*>(this)), ::sal::static_int_cast< sal_Int16, unsigned int >( uArgErr )) ;
            break;
        case DISP_E_TYPEMISMATCH:
            throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")), static_cast<XInterface*>(
                                             static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::UNKNOWN, ::sal::static_int_cast< sal_Int16, unsigned int >( uArgErr ));
            break;
        case DISP_E_UNKNOWNINTERFACE:
            throw RuntimeException();
            break;
        case DISP_E_UNKNOWNLCID:
            throw RuntimeException();
            break;
        case DISP_E_PARAMNOTOPTIONAL:
            throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")),static_cast<XInterface*>(
                                             static_cast<XWeak*>(this)) , TypeClass_UNKNOWN, FailReason::NO_DEFAULT_AVAILABLE, uArgErr);
            break;
        default:
            throw  RuntimeException();
            break;
        }
    }
    catch (const CannotConvertException &)
    {
        throw;
    }
    catch (const UnknownPropertyException &)
    {
        throw;
    }
    catch (const BridgeRuntimeError& e)
    {
        throw RuntimeException(
            e.message, Reference<XInterface>());
    }
    catch (const Exception & e)
    {
        throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
                                     "IUnknownWrapper_Impl::setValue ! Message : \n") +
                               e.Message, Reference<XInterface>());

    }
    catch (...)
    {
        throw RuntimeException(
            OUSTR("[automation bridge] unexpected exception in "
            "IUnknownWrapper_Impl::setValue !"), Reference<XInterface>());
    }
}

Any SAL_CALL IUnknownWrapper_Impl::getValue( const OUString& aPropertyName )
        throw(UnknownPropertyException, RuntimeException)
{
    if ( ! m_spDispatch )
    {
        throw RuntimeException(
            OUSTR("[automation bridge] The object does not have an IDispatch interface"),
            Reference<XInterface>());
    }
    Any ret;
    try
    {
        o2u_attachCurrentThread();
        ITypeInfo * pInfo = getTypeInfo();
        // I was going to implement an XServiceInfo interface to allow the type
        // of the automation object to be exposed.. but it seems
        // from looking at comments in the code that it is possible for
        // this object to actually wrap an UNO object ( I guess if automation is
        // used from MSO to create Openoffice objects ) Therefore, those objects
        // will more than likely already have their own XServiceInfo interface.
        // Instead here I chose a name that should be illegal both in COM and
        // UNO ( from an IDL point of view ) therefore I think this is a safe
        // hack
        if ( aPropertyName == "$GetTypeName" )
        {
            if ( pInfo && m_sTypeName.getLength() == 0 )
            {
                 m_sTypeName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IDispatch") );
                CComBSTR sName;

                if ( SUCCEEDED( pInfo->GetDocumentation( -1, &sName, NULL, NULL, NULL  ) ) )
                {
                    rtl::OUString sTmp( reinterpret_cast<const sal_Unicode*>(LPCOLESTR(sName)));
                    if ( sTmp.indexOf('_')  == 0 )
                       sTmp = sTmp.copy(1);
                    // do we own the memory for pTypeLib, msdn doco is vague
                    // I'll assume we do
                    CComPtr< ITypeLib > pTypeLib;
                    unsigned int index;
                    if ( SUCCEEDED(  pInfo->GetContainingTypeLib(  &pTypeLib.p, &index )) )
                    {
                        if ( SUCCEEDED( pTypeLib->GetDocumentation( -1, &sName, NULL, NULL, NULL  ) ) )
                        {
                            rtl::OUString sLibName( reinterpret_cast<const sal_Unicode*>(LPCOLESTR(sName)));
                            m_sTypeName = sLibName.concat( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(".") ) ).concat( sTmp );

                        }
                    }
                }

            }
            ret <<= m_sTypeName;
            return ret;
        }
        FuncDesc aDescGet(pInfo);
        FuncDesc aDescPut(pInfo);
        VarDesc aVarDesc(pInfo);
        getPropDesc(aPropertyName, & aDescGet, & aDescPut, & aVarDesc);
        if ( ! aDescGet && ! aDescPut && ! aVarDesc)
        {
            //property not found
            OUString msg(OUSTR("[automation bridge]Property \"") + aPropertyName +
                         OUSTR("\" is not supported"));
            throw UnknownPropertyException(msg, Reference<XInterface>());
        }
        // write-only should not be possible
        OSL_ASSERT(  aDescGet  || ! aDescPut);

        HRESULT hr;
        DISPPARAMS dispparams = {0, 0, 0, 0};
        CComVariant varResult;
        ExcepInfo excepinfo;
        unsigned int uArgErr;
        DISPID dispid;
        if (aDescGet)
            dispid = aDescGet->memid;
        else if (aVarDesc)
            dispid = aVarDesc->memid;
        else
            dispid = aDescPut->memid;

        hr = m_spDispatch->Invoke(dispid,
                                 IID_NULL,
                                 LOCALE_USER_DEFAULT,
                                 DISPATCH_PROPERTYGET,
                                 &dispparams,
                                 &varResult,
                                 &excepinfo,
                                 &uArgErr);

        // converting return value and out parameter back to UNO
        if (hr == S_OK)
        {
            // If the com object implements uno interfaces then we have
            // to convert the attribute into the expected type.
            TypeDescription attrInfo;
            getAttributeInfo(aPropertyName, attrInfo);
            if( attrInfo.is() )
                variantToAny( &varResult, ret, Type( attrInfo.get()->pWeakRef));
            else
                variantToAny(&varResult, ret);
        }

        // lookup error code
        switch (hr)
        {
        case S_OK:
            break;
        case DISP_E_BADPARAMCOUNT:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_BADVARTYPE:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_EXCEPTION:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_MEMBERNOTFOUND:
            throw UnknownPropertyException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_NONAMEDARGS:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_OVERFLOW:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_PARAMNOTFOUND:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_TYPEMISMATCH:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_UNKNOWNINTERFACE:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_UNKNOWNLCID:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        case DISP_E_PARAMNOTOPTIONAL:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        default:
            throw RuntimeException(OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription)),
                                   Reference<XInterface>());
            break;
        }
    }
    catch ( const UnknownPropertyException& )
    {
        throw;
    }
    catch (const BridgeRuntimeError& e)
    {
        throw RuntimeException(
            e.message, Reference<XInterface>());
    }
    catch (const Exception & e)
    {
        throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
                                     "IUnknownWrapper_Impl::getValue ! Message : \n") +
                               e.Message, Reference<XInterface>());
    }
    catch (...)
    {
        throw RuntimeException(
            OUSTR("[automation bridge] unexpected exception in "
            "IUnknownWrapper_Impl::getValue !"), Reference<XInterface>());
    }
    return ret;
}

sal_Bool SAL_CALL IUnknownWrapper_Impl::hasMethod( const OUString& aName )
        throw(RuntimeException)
{
    if ( ! m_spDispatch )
    {
        throw RuntimeException(
            OUSTR("[automation bridge] The object does not have an IDispatch interface"),
            Reference<XInterface>());
    }
    sal_Bool ret = sal_False;

    try
    {
        o2u_attachCurrentThread();
        ITypeInfo* pInfo = getTypeInfo();
        FuncDesc aDesc(pInfo);
        getFuncDesc(aName, & aDesc);
        // Automation properties can have arguments. Those are treated as methods and
        //are called through XInvocation::invoke.
        if ( ! aDesc)
        {
            FuncDesc aDescGet(pInfo);
            FuncDesc aDescPut(pInfo);
            VarDesc aVarDesc(pInfo);
            getPropDesc( aName, & aDescGet, & aDescPut, & aVarDesc);
            if (aDescGet  && aDescGet->cParams > 0
                || aDescPut && aDescPut->cParams > 0)
                ret = sal_True;
        }
        else
            ret = sal_True;
    }
    catch (const BridgeRuntimeError& e)
    {
        throw RuntimeException(e.message, Reference<XInterface>());
    }
    catch (const Exception & e)
    {
        throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
                                     "IUnknownWrapper_Impl::hasMethod ! Message : \n") +
                               e.Message, Reference<XInterface>());
    }
    catch (...)
    {
        throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
            "IUnknownWrapper_Impl::hasMethod !"), Reference<XInterface>());
    }
    return ret;
}

sal_Bool SAL_CALL IUnknownWrapper_Impl::hasProperty( const OUString& aName )
        throw(RuntimeException)
{
    if ( ! m_spDispatch )
    {
        throw RuntimeException(OUSTR("[automation bridge] The object does not have an "
            "IDispatch interface"), Reference<XInterface>());
    }
    sal_Bool ret = sal_False;
    try
    {
        o2u_attachCurrentThread();

        ITypeInfo * pInfo = getTypeInfo();
        FuncDesc aDescGet(pInfo);
        FuncDesc aDescPut(pInfo);
        VarDesc aVarDesc(pInfo);
        getPropDesc(aName, & aDescGet, & aDescPut, & aVarDesc);

    // we should probably just check the funckind
        // basic has been modified to handle properties ( 'get' ) props at
    // least with parameters
    // additionally you can call invoke(Get|Set)Property on the bridge
        // you can determine if a property has parameter is hasMethod
    // returns true for the name
        if (aVarDesc
            || aDescPut
            || aDescGet )
        {
            ret = sal_True;
        }
    }
    catch (const BridgeRuntimeError& e)
    {
        throw RuntimeException(e.message, Reference<XInterface>());
    }
    catch (const Exception & e)
    {
        throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
                                     "IUnknownWrapper_Impl::hasProperty ! Message : \n") +
                               e.Message, Reference<XInterface>());

    }
    catch (...)
    {
        throw RuntimeException(OUSTR("[automation bridge] unexpected exception in "
            "IUnknownWrapper_Impl::hasProperty !"), Reference<XInterface>());
    }
    return ret;
}

Any SAL_CALL IUnknownWrapper_Impl::createBridge( const Any& modelDepObject,
                const Sequence< sal_Int8 >& /*aProcessId*/, sal_Int16 sourceModelType,
                 sal_Int16 destModelType )
    throw( IllegalArgumentException, RuntimeException)
{
    Any ret;
    o2u_attachCurrentThread();

    if (
        (sourceModelType == UNO) &&
        (destModelType == OLE) &&
        (modelDepObject.getValueTypeClass() == TypeClass_INTERFACE)
       )
    {
        Reference<XInterface> xInt( *(XInterface**) modelDepObject.getValue());
        Reference<XInterface> xSelf( (OWeakObject*)this);

        if (xInt == xSelf)
        {
            VARIANT* pVariant = (VARIANT*) CoTaskMemAlloc(sizeof(VARIANT));

            VariantInit(pVariant);
            if (m_bOriginalDispatch == sal_True)
            {
                pVariant->vt = VT_DISPATCH;
                pVariant->pdispVal = m_spDispatch;
                pVariant->pdispVal->AddRef();
            }
            else
            {
                pVariant->vt = VT_UNKNOWN;
                pVariant->punkVal = m_spUnknown;
                pVariant->punkVal->AddRef();
            }

            ret.setValue((void*)&pVariant, getCppuType( (sal_uInt32*) 0));
        }
    }

    return ret;
}
/** @internal
    @exception IllegalArgumentException
    @exception CannotConvertException
    @exception InvocationTargetException
    @RuntimeException
*/
Any  IUnknownWrapper_Impl::invokeWithDispIdUnoTlb(const OUString& sFunctionName,
                                                  const Sequence< Any >& Params,
                                                  Sequence< sal_Int16 >& OutParamIndex,
                                                  Sequence< Any >& OutParam)
{
    Any ret;
    HRESULT hr= S_OK;

    sal_Int32 parameterCount= Params.getLength();
    sal_Int32 outParameterCount= 0;
    typelib_InterfaceMethodTypeDescription* pMethod= NULL;
    TypeDescription methodDesc;
    getMethodInfo(sFunctionName, methodDesc);

    // We need to know whether the IDispatch is from a JScript object.
    // Then out and in/out parameters have to be treated differently than
    // with common COM objects.
    sal_Bool bJScriptObject= isJScriptObject();
    scoped_array<CComVariant> sarParams;
    scoped_array<CComVariant> sarParamsRef;
    CComVariant *pVarParams= NULL;
    CComVariant *pVarParamsRef= NULL;
    sal_Bool bConvRet= sal_True;

    if( methodDesc.is())
    {
        pMethod = (typelib_InterfaceMethodTypeDescription* )methodDesc.get();
        parameterCount = pMethod->nParams;
        // Create the Array for the array being passed in DISPPARAMS
        // the array also contains the outparameter (but not the values)
        if( pMethod->nParams > 0)
        {
            sarParams.reset(new CComVariant[ parameterCount]);
            pVarParams = sarParams.get();
        }

        // Create the Array for the out an in/out parameter. These values
        // are referenced by the VT_BYREF VARIANTs in DISPPARAMS.
        // We need to find out the number of out and in/out parameter.
        for( sal_Int32 i=0; i < parameterCount; i++)
        {
            if( pMethod->pParams[i].bOut)
                outParameterCount++;
        }

        if( !bJScriptObject)
        {
            sarParamsRef.reset(new CComVariant[outParameterCount]);
            pVarParamsRef = sarParamsRef.get();
            // build up the parameters for IDispatch::Invoke
            sal_Int32 outParamIndex=0;
            int i = 0;
            try
            {
                for( i= 0; i < parameterCount; i++)
                {
                    // In parameter
                    if( pMethod->pParams[i].bIn == sal_True && ! pMethod->pParams[i].bOut)
                    {
                        anyToVariant( &pVarParams[parameterCount - i -1], Params.getConstArray()[i]);
                    }
                    // Out parameter + in/out parameter
                    else if( pMethod->pParams[i].bOut == sal_True)
                    {
                        CComVariant var;
                        if(pMethod->pParams[i].bIn)
                        {
                            anyToVariant( & var,Params[i]);
                            pVarParamsRef[outParamIndex] = var;
                        }

                        switch( pMethod->pParams[i].pTypeRef->eTypeClass)
                        {
                        case TypeClass_INTERFACE:
                        case TypeClass_STRUCT:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt= VT_DISPATCH;
                                pVarParamsRef[ outParamIndex].pdispVal= 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_DISPATCH | VT_BYREF;
                            pVarParams[parameterCount - i -1].ppdispVal= &pVarParamsRef[outParamIndex].pdispVal;
                            break;
                        case TypeClass_ENUM:
                        case TypeClass_LONG:
                        case TypeClass_UNSIGNED_LONG:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_I4;
                                pVarParamsRef[ outParamIndex].lVal = 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_I4 | VT_BYREF;
                            pVarParams[parameterCount - i -1].plVal= &pVarParamsRef[outParamIndex].lVal;
                            break;
                        case TypeClass_SEQUENCE:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_ARRAY| VT_VARIANT;
                                pVarParamsRef[ outParamIndex].parray= NULL;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_ARRAY| VT_BYREF | VT_VARIANT;
                            pVarParams[parameterCount - i -1].pparray= &pVarParamsRef[outParamIndex].parray;
                            break;
                        case TypeClass_ANY:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_EMPTY;
                                pVarParamsRef[ outParamIndex].lVal = 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_VARIANT | VT_BYREF;
                            pVarParams[parameterCount - i -1].pvarVal = &pVarParamsRef[outParamIndex];
                            break;
                        case TypeClass_BOOLEAN:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_BOOL;
                                pVarParamsRef[ outParamIndex].boolVal = 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_BOOL| VT_BYREF;
                            pVarParams[parameterCount - i -1].pboolVal =
                                & pVarParamsRef[outParamIndex].boolVal;
                            break;

                        case TypeClass_STRING:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_BSTR;
                                pVarParamsRef[ outParamIndex].bstrVal= 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_BSTR| VT_BYREF;
                            pVarParams[parameterCount - i -1].pbstrVal=
                                & pVarParamsRef[outParamIndex].bstrVal;
                            break;

                        case TypeClass_FLOAT:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_R4;
                                pVarParamsRef[ outParamIndex].fltVal= 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_R4| VT_BYREF;
                            pVarParams[parameterCount - i -1].pfltVal =
                                & pVarParamsRef[outParamIndex].fltVal;
                            break;
                        case TypeClass_DOUBLE:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_R8;
                                pVarParamsRef[ outParamIndex].dblVal= 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_R8| VT_BYREF;
                            pVarParams[parameterCount - i -1].pdblVal=
                                & pVarParamsRef[outParamIndex].dblVal;
                            break;
                        case TypeClass_BYTE:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_UI1;
                                pVarParamsRef[ outParamIndex].bVal= 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_UI1| VT_BYREF;
                            pVarParams[parameterCount - i -1].pbVal=
                                & pVarParamsRef[outParamIndex].bVal;
                            break;
                        case TypeClass_CHAR:
                        case TypeClass_SHORT:
                        case TypeClass_UNSIGNED_SHORT:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_I2;
                                pVarParamsRef[ outParamIndex].iVal = 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_I2| VT_BYREF;
                            pVarParams[parameterCount - i -1].piVal=
                                & pVarParamsRef[outParamIndex].iVal;
                            break;

                        default:
                            if( ! pMethod->pParams[i].bIn)
                            {
                                pVarParamsRef[ outParamIndex].vt = VT_EMPTY;
                                pVarParamsRef[ outParamIndex].lVal = 0;
                            }
                            pVarParams[parameterCount - i -1].vt = VT_VARIANT | VT_BYREF;
                            pVarParams[parameterCount - i -1].pvarVal =
                                & pVarParamsRef[outParamIndex];
                        }
                        outParamIndex++;
                    } // end else if
                } // end for
            }
            catch (IllegalArgumentException & e)
            {
                e.ArgumentPosition = ::sal::static_int_cast< sal_Int16, int >( i );
                throw;
            }
            catch (CannotConvertException & e)
            {
                e.ArgumentIndex = i;
                throw;
            }
        }
        else // it is an JScriptObject
        {
            int i = 0;
            try
            {
                for( ; i< parameterCount; i++)
                {
                    // In parameter
                    if( pMethod->pParams[i].bIn == sal_True && ! pMethod->pParams[i].bOut)
                    {
                        anyToVariant( &pVarParams[parameterCount - i -1], Params.getConstArray()[i]);
                    }
                    // Out parameter + in/out parameter
                    else if( pMethod->pParams[i].bOut == sal_True)
                    {
                        CComObject<JScriptOutParam>* pParamObject;
                        if( SUCCEEDED( CComObject<JScriptOutParam>::CreateInstance( &pParamObject)))
                        {
                            CComPtr<IUnknown> pUnk(pParamObject->GetUnknown());
#ifdef __MINGW32__
                            CComQIPtr<IDispatch, &__uuidof(IDispatch)> pDisp( pUnk);
#else
                            CComQIPtr<IDispatch> pDisp( pUnk);
#endif

                            pVarParams[ parameterCount - i -1].vt= VT_DISPATCH;
                            pVarParams[ parameterCount - i -1].pdispVal= pDisp;
                            pVarParams[ parameterCount - i -1].pdispVal->AddRef();
                            // if the param is in/out then put the parameter on index 0
                            if( pMethod->pParams[i].bIn == sal_True ) // in / out
                            {
                                CComVariant varParam;
                                anyToVariant( &varParam, Params.getConstArray()[i]);
                                CComDispatchDriver dispDriver( pDisp);
                                if(FAILED( dispDriver.PutPropertyByName( L"0", &varParam)))
                                    throw BridgeRuntimeError(
                                        OUSTR("[automation bridge]IUnknownWrapper_Impl::"
                                              "invokeWithDispIdUnoTlb\n"
                                              "Could not set property \"0\" for the in/out "
                                              "param!"));

                            }
                        }
                        else
                        {
                            throw BridgeRuntimeError(
                                OUSTR("[automation bridge]IUnknownWrapper_Impl::"
                                      "invokeWithDispIdUnoTlb\n"
                                      "Could not create out parameter at index: ") +
                                OUString::valueOf((sal_Int32) i));
                        }

                    }
                }
            }
            catch (IllegalArgumentException & e)
            {
                e.ArgumentPosition = ::sal::static_int_cast< sal_Int16, int >( i );
                throw;
            }
            catch (CannotConvertException & e)
            {
                e.ArgumentIndex = i;
                throw;
            }
        }
    }
    // No type description Available, that is we have to deal with a COM component,
    // that does not implements UNO interfaces ( IDispatch based)
    else
    {
        //We should not run into this block, because invokeWithDispIdComTlb should
        //have been called instead.
        OSL_ASSERT(0);
    }


    CComVariant     varResult;
    ExcepInfo       excepinfo;
    unsigned int    uArgErr;
    DISPPARAMS dispparams= { pVarParams, NULL, parameterCount, 0};
    // Get the DISPID
    FuncDesc aDesc(getTypeInfo());
    getFuncDesc(sFunctionName, & aDesc);
    // invoking OLE method
    hr = m_spDispatch->Invoke(aDesc->memid,
                             IID_NULL,
                             LOCALE_USER_DEFAULT,
                             DISPATCH_METHOD,
                             &dispparams,
                             &varResult,
                             &excepinfo,
                             &uArgErr);

    // converting return value and out parameter back to UNO
    if (hr == S_OK)
    {
        if( outParameterCount && pMethod)
        {
            OutParamIndex.realloc( outParameterCount);
            OutParam.realloc( outParameterCount);
            sal_Int32 outIndex=0;
            int i = 0;
            try
            {
                for( ; i < parameterCount; i++)
                {
                    if( pMethod->pParams[i].bOut )
                    {
                        OutParamIndex[outIndex]= (sal_Int16) i;
                        Any outAny;
                        if( !bJScriptObject)
                        {
                            variantToAny( &pVarParamsRef[outIndex], outAny,
                                        Type(pMethod->pParams[i].pTypeRef), sal_False);
                            OutParam[outIndex++]= outAny;
                        }
                        else //JScriptObject
                        {
                            if( pVarParams[i].vt == VT_DISPATCH)
                            {
                                CComDispatchDriver pDisp( pVarParams[i].pdispVal);
                                if( pDisp)
                                {
                                    CComVariant varOut;
                                    if( SUCCEEDED( pDisp.GetPropertyByName( L"0", &varOut)))
                                    {
                                        variantToAny( &varOut, outAny,
                                                    Type(pMethod->pParams[parameterCount - 1 - i].pTypeRef), sal_False);
                                        OutParam[outParameterCount - 1 - outIndex++]= outAny;
                                    }
                                    else
                                        bConvRet= sal_False;
                                }
                                else
                                    bConvRet= sal_False;
                            }
                            else
                                bConvRet= sal_False;
                        }
                    }
                    if( !bConvRet) break;
                }
            }
            catch(IllegalArgumentException & e)
            {
                e.ArgumentPosition = ::sal::static_int_cast< sal_Int16, int >( i );
                throw;
            }
            catch(CannotConvertException & e)
            {
                e.ArgumentIndex = i;
                throw;
            }
        }
        // return value, no type information available
        if ( bConvRet)
        {
            try
            {
                if( pMethod )
                    variantToAny(&varResult, ret, Type( pMethod->pReturnTypeRef), sal_False);
                else
                    variantToAny(&varResult, ret, sal_False);
            }
            catch (IllegalArgumentException & e)
            {
                e.Message =
                    OUSTR("[automation bridge]IUnknownWrapper_Impl::invokeWithDispIdUnoTlb\n"
                    "Could not convert return value! \n Message: \n") + e.Message;
                throw;
            }
            catch (CannotConvertException & e)
            {
                e.Message =
                    OUSTR("[automation bridge]IUnknownWrapper_Impl::invokeWithDispIdUnoTlb\n"
                    "Could not convert return value! \n Message: \n") + e.Message;
                throw;
            }
        }
    }

    if( !bConvRet) // conversion of return or out parameter failed
        throw CannotConvertException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Call to COM object failed. Conversion of return or out value failed")),
                                      Reference<XInterface>( static_cast<XWeak*>(this), UNO_QUERY   ), TypeClass_UNKNOWN,
                                      FailReason::UNKNOWN, 0);// lookup error code
    // conversion of return or out parameter failed
    switch (hr)
    {
    case S_OK:
        break;
    case DISP_E_BADPARAMCOUNT:
        throw IllegalArgumentException();
        break;
    case DISP_E_BADVARTYPE:
        throw RuntimeException();
        break;
    case DISP_E_EXCEPTION:
        throw InvocationTargetException();
        break;
    case DISP_E_MEMBERNOTFOUND:
        throw IllegalArgumentException();
        break;
    case DISP_E_NONAMEDARGS:
        throw IllegalArgumentException();
        break;
    case DISP_E_OVERFLOW:
        throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")), static_cast<XInterface*>(
                                         static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::OUT_OF_RANGE, uArgErr);
        break;
    case DISP_E_PARAMNOTFOUND:
        throw IllegalArgumentException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")), static_cast<XInterface*>(
                                           static_cast<XWeak*>(this)), ::sal::static_int_cast< sal_Int16, unsigned int >( uArgErr ));
        break;
    case DISP_E_TYPEMISMATCH:
        throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")),static_cast<XInterface*>(
                                         static_cast<XWeak*>(this)) , TypeClass_UNKNOWN, FailReason::UNKNOWN, uArgErr);
        break;
    case DISP_E_UNKNOWNINTERFACE:
        throw RuntimeException() ;
        break;
    case DISP_E_UNKNOWNLCID:
        throw RuntimeException() ;
        break;
    case DISP_E_PARAMNOTOPTIONAL:
        throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("call to OLE object failed")), static_cast<XInterface*>(
                                         static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::NO_DEFAULT_AVAILABLE, uArgErr);
                break;
    default:
        throw RuntimeException();
        break;
    }

    return ret;
}



// --------------------------
// XInitialization
void SAL_CALL IUnknownWrapper_Impl::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    // 1.parameter is IUnknown
    // 2.parameter is a boolean which indicates if the the COM pointer was a IUnknown or IDispatch
    // 3.parameter is a Sequence<Type>
    o2u_attachCurrentThread();
    OSL_ASSERT(aArguments.getLength() == 3);

    m_spUnknown= *(IUnknown**) aArguments[0].getValue();
#ifdef __MINGW32__
    m_spUnknown->QueryInterface(IID_IDispatch, reinterpret_cast<LPVOID*>( & m_spDispatch.p));
#else
    m_spUnknown.QueryInterface( & m_spDispatch.p);
#endif

    aArguments[1] >>= m_bOriginalDispatch;
    aArguments[2] >>= m_seqTypes;

    ITypeInfo* pType = NULL;
    try
    {
        // a COM object implementation that has no TypeInfo is still a legal COM object;
        // such objects can at least be transported through UNO using the bridge
        // so we should allow to create wrappers for them as well
        pType = getTypeInfo();
    }
    catch( const BridgeRuntimeError& )
    {}
    catch( const Exception& )
    {}

    if ( pType )
    {
        try
        {
            // Get Default member
            CComBSTR defaultMemberName;
            if ( SUCCEEDED( pType->GetDocumentation(0, &defaultMemberName, 0, 0, 0 ) ) )
            {
                OUString usName(reinterpret_cast<const sal_Unicode*>(LPCOLESTR(defaultMemberName)));
                FuncDesc aDescGet(pType);
                FuncDesc aDescPut(pType);
                VarDesc aVarDesc(pType);
                // see if this is a property first ( more likely to be a property then a method )
                getPropDesc( usName, & aDescGet, & aDescPut, & aVarDesc);

                if ( !aDescGet && !aDescPut )
                {
                    getFuncDesc( usName, &aDescGet );
                    if ( !aDescGet )
                        throw BridgeRuntimeError( OUSTR("[automation bridge]IUnknownWrapper_Impl::initialize() Failed to get Function or Property desc. for " ) + usName );
                }
                // now for some funny heuristics to make basic understand what to do
                // a single aDescGet ( that doesn't take any params ) would be
                // a read only ( defaultmember ) property e.g. this object
                // should implement XDefaultProperty
                // a single aDescGet ( that *does* ) take params is basically a
                // default method e.g. implement XDefaultMethod

                // a DescPut ( I guess we only really support a default param with '1' param ) as a setValue ( but I guess we can leave it through, the object will fail if we don't get it right anyway )
                if ( aDescPut || ( aDescGet && aDescGet->cParams == 0 ) )
                    m_bHasDfltProperty = true;
                if ( aDescGet->cParams > 0 )
                    m_bHasDfltMethod = true;
                if ( m_bHasDfltProperty || m_bHasDfltMethod )
                    m_sDefaultMember = usName;
            }
        }
        catch ( const BridgeRuntimeError & e )
        {
            throw RuntimeException( e.message, Reference<XInterface>() );
        }
        catch( const Exception& e )
        {
            throw RuntimeException(
                    OUSTR("[automation bridge] unexpected exception in IUnknownWrapper_Impl::initialiase() error message: \n") + e.Message,
                    Reference<XInterface>() );
        }
    }
}

// --------------------------
// XDirectInvocation
uno::Any SAL_CALL IUnknownWrapper_Impl::directInvoke( const ::rtl::OUString& aName, const uno::Sequence< uno::Any >& aParams )
    throw (lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    Any aResult;

    if ( !m_spDispatch )
    {
        throw RuntimeException(
            OUSTR("[automation bridge] The object does not have an IDispatch interface"),
            Reference<XInterface>());
    }

    o2u_attachCurrentThread();
    DISPID dispid;
    if ( !getDispid( aName, &dispid ) )
        throw IllegalArgumentException(
            OUSTR( "[automation bridge] The object does not have a function or property " )
            + aName, Reference<XInterface>(), 0);

    CComVariant     varResult;
    ExcepInfo       excepinfo;
    unsigned int    uArgErr = 0;
    INVOKEKIND pInvkinds[2];
    pInvkinds[0] = INVOKE_FUNC;
    pInvkinds[1] = aParams.getLength() ? INVOKE_PROPERTYPUT : INVOKE_PROPERTYGET;
    HRESULT hInvRes = E_FAIL;

    // try Invoke first, if it does not work, try put/get property
    for ( sal_Int32 nStep = 0; FAILED( hInvRes ) && nStep < 2; nStep++ )
    {
        DISPPARAMS      dispparams = {NULL, NULL, 0, 0};

        DISPID idPropertyPut = DISPID_PROPERTYPUT;
        scoped_array<DISPID> arDispidNamedArgs;
        scoped_array<CComVariant> ptrArgs;
        scoped_array<CComVariant> ptrRefArgs; // referenced arguments
        CComVariant * arArgs = NULL;
        CComVariant * arRefArgs = NULL;

        dispparams.cArgs = aParams.getLength();

        // Determine the number of named arguments
        for ( sal_Int32 nInd = 0; nInd < aParams.getLength(); nInd++ )
            if ( aParams[nInd].getValueType() == getCppuType((NamedArgument*) 0) )
                dispparams.cNamedArgs ++;

        // fill the named arguments
        if ( dispparams.cNamedArgs > 0
          && !( dispparams.cNamedArgs == 1 && pInvkinds[nStep] == INVOKE_PROPERTYPUT ) )
        {
            int nSizeAr = dispparams.cNamedArgs + 1;
            if ( pInvkinds[nStep] == INVOKE_PROPERTYPUT )
                nSizeAr = dispparams.cNamedArgs;

            scoped_array<OLECHAR*> saNames(new OLECHAR*[nSizeAr]);
            OLECHAR ** pNames = saNames.get();
            pNames[0] = const_cast<OLECHAR*>(reinterpret_cast<LPCOLESTR>(aName.getStr()));

            int cNamedArg = 0;
            for ( size_t nInd = 0; nInd < dispparams.cArgs; nInd++ )
            {
                if ( aParams[nInd].getValueType() == getCppuType((NamedArgument*) 0))
                {
                    const NamedArgument& arg = *(NamedArgument const*)aParams[nInd].getValue();

                    //We put the parameter names in reverse order into the array,
                    //so we can use the DISPID array for DISPPARAMS::rgdispidNamedArgs
                    //The first name in the array is the method name
                    pNames[nSizeAr - 1 - cNamedArg++] = const_cast<OLECHAR*>(reinterpret_cast<LPCOLESTR>(arg.Name.getStr()));
                }
            }

            arDispidNamedArgs.reset( new DISPID[nSizeAr] );
            HRESULT hr = getTypeInfo()->GetIDsOfNames( pNames, nSizeAr, arDispidNamedArgs.get() );
            if ( hr == E_NOTIMPL )
                hr = m_spDispatch->GetIDsOfNames(IID_NULL, pNames, nSizeAr, LOCALE_USER_DEFAULT, arDispidNamedArgs.get() );

            if ( SUCCEEDED( hr ) )
            {
                if ( pInvkinds[nStep] == DISPATCH_PROPERTYPUT )
                {
                    DISPID*  arIDs = arDispidNamedArgs.get();
                    arIDs[0] = DISPID_PROPERTYPUT;
                    dispparams.rgdispidNamedArgs = arIDs;
                }
                else
                {
                    DISPID*  arIDs = arDispidNamedArgs.get();
                    dispparams.rgdispidNamedArgs = & arIDs[1];
                }
            }
            else if (hr == DISP_E_UNKNOWNNAME)
            {
                 throw IllegalArgumentException(
                     OUSTR("[automation bridge]One of the named arguments is wrong!"),
                     Reference<XInterface>(), 0);
            }
            else
            {
                throw InvocationTargetException(
                    OUSTR("[automation bridge] ITypeInfo::GetIDsOfNames returned error ")
                    + OUString::valueOf((sal_Int32) hr, 16), Reference<XInterface>(), Any());
            }
        }

        //Convert arguments
        ptrArgs.reset(new CComVariant[dispparams.cArgs]);
        ptrRefArgs.reset(new CComVariant[dispparams.cArgs]);
        arArgs = ptrArgs.get();
        arRefArgs = ptrRefArgs.get();

        sal_Int32 nInd = 0;
        try
        {
            sal_Int32 revIndex = 0;
            for ( nInd = 0; nInd < sal_Int32(dispparams.cArgs); nInd++)
            {
                revIndex = dispparams.cArgs - nInd - 1;
                arRefArgs[revIndex].byref = 0;
                Any  anyArg;
                if ( nInd < aParams.getLength() )
                    anyArg = aParams.getConstArray()[nInd];

                // Property Put arguments
                if ( anyArg.getValueType() == getCppuType((PropertyPutArgument*)0) )
                {
                    PropertyPutArgument arg;
                    anyArg >>= arg;
                    anyArg <<= arg.Value;
                }
                // named argument
                if (anyArg.getValueType() == getCppuType((NamedArgument*) 0))
                {
                    NamedArgument aNamedArgument;
                    anyArg >>= aNamedArgument;
                    anyArg <<= aNamedArgument.Value;
                }

                if ( nInd < aParams.getLength() && anyArg.getValueTypeClass() != TypeClass_VOID )
                {
                    anyToVariant( &arArgs[revIndex], anyArg, VT_VARIANT );
                }
                else
                {
                    arArgs[revIndex].vt = VT_ERROR;
                    arArgs[revIndex].scode = DISP_E_PARAMNOTFOUND;
                }
            }
        }
        catch (IllegalArgumentException & e)
        {
            e.ArgumentPosition = ::sal::static_int_cast< sal_Int16, sal_Int32 >( nInd );
            throw;
        }
        catch (CannotConvertException & e)
        {
            e.ArgumentIndex = nInd;
            throw;
        }

        dispparams.rgvarg = arArgs;
        // invoking OLE method
        DWORD localeId = LOCALE_USER_DEFAULT;
        hInvRes = m_spDispatch->Invoke( dispid,
                                        IID_NULL,
                                        localeId,
                                        ::sal::static_int_cast< WORD, INVOKEKIND >( pInvkinds[nStep] ),
                                        &dispparams,
                                        &varResult,
                                        &excepinfo,
                                        &uArgErr);
    }

    // converting return value and out parameter back to UNO
    if ( SUCCEEDED( hInvRes ) )
        variantToAny( &varResult, aResult, sal_False );
    else
    {
        // map error codes to exceptions
        OUString message;
        switch ( hInvRes )
        {
            case S_OK:
                break;
            case DISP_E_BADPARAMCOUNT:
                throw IllegalArgumentException(OUSTR("[automation bridge] Wrong "
                      "number of arguments. Object returned DISP_E_BADPARAMCOUNT."),
                      0, 0);
                break;
            case DISP_E_BADVARTYPE:
                throw RuntimeException(OUSTR("[automation bridge] One or more "
                      "arguments have the wrong type. Object returned "
                      "DISP_E_BADVARTYPE."), 0);
                break;
            case DISP_E_EXCEPTION:
                    message = OUSTR("[automation bridge]: ");
                    message += OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription),
                        ::SysStringLen(excepinfo.bstrDescription));
                    throw InvocationTargetException(message, Reference<XInterface>(), Any());
                    break;
            case DISP_E_MEMBERNOTFOUND:
                message = OUSTR("[automation bridge]: A function with the name \"")
                    + aName + OUSTR("\" is not supported. Object returned "
                    "DISP_E_MEMBERNOTFOUND.");
                throw IllegalArgumentException(message, 0, 0);
                break;
            case DISP_E_NONAMEDARGS:
                throw IllegalArgumentException(OUSTR("[automation bridge] Object "
                      "returned DISP_E_NONAMEDARGS"),0, ::sal::static_int_cast< sal_Int16, unsigned int >( uArgErr ));
                break;
            case DISP_E_OVERFLOW:
                throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[automation bridge] Call failed.")),
                                             static_cast<XInterface*>(
                    static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::OUT_OF_RANGE, uArgErr);
                break;
            case DISP_E_PARAMNOTFOUND:
                throw IllegalArgumentException(OUSTR("[automation bridge]Call failed."
                                                     "Object returned DISP_E_PARAMNOTFOUND."),
                                               0, ::sal::static_int_cast< sal_Int16, unsigned int >( uArgErr ));
                break;
            case DISP_E_TYPEMISMATCH:
                throw CannotConvertException(OUSTR("[automation bridge] Call  failed. "
                                             "Object returned DISP_E_TYPEMISMATCH"),
                    static_cast<XInterface*>(
                    static_cast<XWeak*>(this)) , TypeClass_UNKNOWN, FailReason::UNKNOWN, uArgErr);
                break;
            case DISP_E_UNKNOWNINTERFACE:
                throw RuntimeException(OUSTR("[automation bridge] Call failed. "
                                           "Object returned DISP_E_UNKNOWNINTERFACE."),0);
                break;
            case DISP_E_UNKNOWNLCID:
                throw RuntimeException(OUSTR("[automation bridge] Call failed. "
                                           "Object returned DISP_E_UNKNOWNLCID."),0);
                break;
            case DISP_E_PARAMNOTOPTIONAL:
                throw CannotConvertException(OUSTR("[automation bridge] Call failed."
                      "Object returned DISP_E_PARAMNOTOPTIONAL"),
                            static_cast<XInterface*>(static_cast<XWeak*>(this)),
                                  TypeClass_UNKNOWN, FailReason::NO_DEFAULT_AVAILABLE, uArgErr);
                break;
            default:
                throw RuntimeException();
                break;
        }
    }

    return aResult;
}

::sal_Bool SAL_CALL IUnknownWrapper_Impl::hasMember( const ::rtl::OUString& aName )
    throw (uno::RuntimeException)
{
    if ( ! m_spDispatch )
    {
        throw RuntimeException(
            OUSTR("[automation bridge] The object does not have an IDispatch interface"),
            Reference<XInterface>());
    }

    o2u_attachCurrentThread();
    DISPID dispid;
    return getDispid( aName, &dispid );
}


// UnoConversionUtilities --------------------------------------------------------------------------------
Reference< XInterface > IUnknownWrapper_Impl::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                                m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else if( m_nUnoWrapperClass == UNO_OBJECT_WRAPPER_REMOTE_OPT)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else
        return Reference<XInterface>();
}
Reference<XInterface> IUnknownWrapper_Impl::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}



void IUnknownWrapper_Impl::getMethodInfo(const OUString& sName, TypeDescription& methodInfo)
{
    TypeDescription desc= getInterfaceMemberDescOfCurrentCall(sName);
    if( desc.is())
    {
        typelib_TypeDescription* pMember= desc.get();
        if( pMember->eTypeClass == TypeClass_INTERFACE_METHOD )
            methodInfo= pMember;
    }
}

void IUnknownWrapper_Impl::getAttributeInfo(const OUString& sName, TypeDescription& attributeInfo)
{
    TypeDescription desc= getInterfaceMemberDescOfCurrentCall(sName);
    if( desc.is())
    {
        typelib_TypeDescription* pMember= desc.get();
        if( pMember->eTypeClass == TypeClass_INTERFACE_ATTRIBUTE )
        {
            attributeInfo= ((typelib_InterfaceAttributeTypeDescription*)pMember)->pAttributeTypeRef;
        }
    }
}
TypeDescription IUnknownWrapper_Impl::getInterfaceMemberDescOfCurrentCall(const OUString& sName)
{
    TypeDescription ret;

    for( sal_Int32 i=0; i < m_seqTypes.getLength(); i++)
    {
        TypeDescription _curDesc( m_seqTypes[i]);
        _curDesc.makeComplete();
        typelib_InterfaceTypeDescription * pInterface= (typelib_InterfaceTypeDescription*) _curDesc.get();
        if( pInterface)
        {
            typelib_InterfaceMemberTypeDescription* pMember= NULL;
            //find the member description of the current call
            for( int i=0; i < pInterface->nAllMembers; i++)
            {
                typelib_TypeDescriptionReference* pTypeRefMember = pInterface->ppAllMembers[i];
                typelib_TypeDescription* pDescMember= NULL;
                TYPELIB_DANGER_GET( &pDescMember, pTypeRefMember)

                typelib_InterfaceMemberTypeDescription* pInterfaceMember=
                    (typelib_InterfaceMemberTypeDescription*) pDescMember;
                if( OUString( pInterfaceMember->pMemberName) == sName)
                {
                    pMember= pInterfaceMember;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pDescMember)
            }

            if( pMember)
            {
                ret= (typelib_TypeDescription*)pMember;
                TYPELIB_DANGER_RELEASE( (typelib_TypeDescription*)pMember);
            }
        }
        if( ret.is())
            break;
    }
    return ret;
}

sal_Bool IUnknownWrapper_Impl::isJScriptObject()
{
    if(  m_eJScript == JScriptUndefined)
    {
        CComDispatchDriver disp( m_spDispatch);
        if( disp)
        {
            CComVariant result;
            if( SUCCEEDED(  disp.GetPropertyByName( JSCRIPT_ID_PROPERTY, &result)))
            {
                if(result.vt == VT_BSTR)
                {
                    CComBSTR name( result.bstrVal);
                    name.ToLower();
                    if( name == CComBSTR(JSCRIPT_ID))
                        m_eJScript= IsJScript;
                }
            }
        }
        if( m_eJScript == JScriptUndefined)
            m_eJScript= NoJScript;
    }

    return m_eJScript == NoJScript ? sal_False : sal_True;
}



/** @internal
    The function ultimately calls IDispatch::Invoke on the wrapped COM object.
    The COM object does not implement UNO Interfaces ( via IDispatch). This
    is the case when the OleObjectFactory service has been used to create a
    component.
    @exception IllegalArgumentException
    @exception CannotConvertException
    @InvocationTargetException
    @RuntimeException
    @BridgeRuntimeError
*/
Any  IUnknownWrapper_Impl::invokeWithDispIdComTlb(const OUString& sFuncName,
                                                  const Sequence< Any >& Params,
                                                  Sequence< sal_Int16 >& OutParamIndex,
                                                  Sequence< Any >& OutParam)
{
    // Get type info for the call. It can be a method call or property put or
    // property get operation.
    FuncDesc aFuncDesc(getTypeInfo());
    getFuncDescForInvoke(sFuncName, Params, & aFuncDesc);
    return invokeWithDispIdComTlb( aFuncDesc, sFuncName, Params, OutParamIndex, OutParam );
}

Any  IUnknownWrapper_Impl::invokeWithDispIdComTlb(FuncDesc& aFuncDesc,
                                                  const OUString& sFuncName,
                                                  const Sequence< Any >& Params,
                                                  Sequence< sal_Int16 >& OutParamIndex,
                                                  Sequence< Any >& OutParam)
{
    Any ret;
    HRESULT result;

    DISPPARAMS      dispparams = {NULL, NULL, 0, 0};
    CComVariant     varResult;
    ExcepInfo       excepinfo;
    unsigned int    uArgErr;
    sal_Int32       i = 0;
    sal_Int32 nUnoArgs = Params.getLength();
    DISPID idPropertyPut = DISPID_PROPERTYPUT;
    scoped_array<DISPID> arDispidNamedArgs;
    scoped_array<CComVariant> ptrArgs;
    scoped_array<CComVariant> ptrRefArgs; // referenced arguments
    CComVariant * arArgs = NULL;
    CComVariant * arRefArgs = NULL;
    sal_Int32 revIndex = 0;

    //Set the array of DISPIDs for named args if it is a property put operation.
    //If there are other named arguments another array is set later on.
    if (aFuncDesc->invkind == INVOKE_PROPERTYPUT
        || aFuncDesc->invkind == INVOKE_PROPERTYPUTREF)
        dispparams.rgdispidNamedArgs = & idPropertyPut;

    //Determine the number of named arguments
    for (int iParam = 0; iParam < nUnoArgs; iParam ++)
    {
        const Any & curArg = Params[iParam];
        if (curArg.getValueType() == getCppuType((NamedArgument*) 0))
            dispparams.cNamedArgs ++;
    }
    //In a property put operation a property value is a named argument (DISPID_PROPERTYPUT).
    //Therefore the number of named arguments is increased by one.
    //Although named, the argument is not named in a actual language, such as  Basic,
    //therefore it is never a com.sun.star.bridge.oleautomation.NamedArgument
    if (aFuncDesc->invkind == DISPATCH_PROPERTYPUT
        || aFuncDesc->invkind == DISPATCH_PROPERTYPUTREF)
        dispparams.cNamedArgs ++;

    //Determine the number of all arguments and named arguments
    if (aFuncDesc->cParamsOpt == -1)
    {
        //Attribute vararg is set on this method. "Unlimited" number of args
        //supported. There can be no optional or defaultvalue on any of the arguments.
        dispparams.cArgs = nUnoArgs;
    }
    else
    {
        //If there are namesd arguments, then the dispparams.cArgs
        //is the number of supplied args, otherwise it is the expected number.
        if (dispparams.cNamedArgs)
            dispparams.cArgs = nUnoArgs;
        else
            dispparams.cArgs = aFuncDesc->cParams;
    }

    //check if there are not to many arguments supplied
    if (::sal::static_int_cast< sal_uInt32, int >( nUnoArgs ) > dispparams.cArgs)
    {
        OUStringBuffer buf(256);
        buf.appendAscii("[automation bridge] There are too many arguments for this method");
        throw IllegalArgumentException( buf.makeStringAndClear(),
            Reference<XInterface>(), (sal_Int16) dispparams.cArgs);
    }

    //Set up the array of DISPIDs (DISPPARAMS::rgdispidNamedArgs)
    //for the named arguments.
    //If there is only one named arg and if it is because of a property put
    //operation, then we need not set up the DISPID array.
    if (dispparams.cNamedArgs > 0 &&
        ! (dispparams.cNamedArgs == 1 &&
           (aFuncDesc->invkind == INVOKE_PROPERTYPUT ||
            aFuncDesc->invkind == INVOKE_PROPERTYPUTREF)))
    {
        //set up an array containing the member and parameter names
        //which is then used in ITypeInfo::GetIDsOfNames
        //First determine the size of the array of names which is passed to
        //ITypeInfo::GetIDsOfNames. It must hold the method names + the named
        //args.
        int nSizeAr = dispparams.cNamedArgs + 1;
        if (aFuncDesc->invkind == INVOKE_PROPERTYPUT
            || aFuncDesc->invkind == INVOKE_PROPERTYPUTREF)
        {
            nSizeAr = dispparams.cNamedArgs; //counts the DISID_PROPERTYPUT
        }

        scoped_array<OLECHAR*> saNames(new OLECHAR*[nSizeAr]);
        OLECHAR ** arNames = saNames.get();
        arNames[0] = const_cast<OLECHAR*>(reinterpret_cast<LPCOLESTR>(sFuncName.getStr()));

        int cNamedArg = 0;
        for (size_t iParams = 0; iParams < dispparams.cArgs; iParams ++)
        {
            const Any &  curArg = Params[iParams];
            if (curArg.getValueType() == getCppuType((NamedArgument*) 0))
            {
                const NamedArgument& arg = *(NamedArgument const*) curArg.getValue();
                //We put the parameter names in reverse order into the array,
                //so we can use the DISPID array for DISPPARAMS::rgdispidNamedArgs
                //The first name in the array is the method name
                arNames[nSizeAr - 1 - cNamedArg++] = const_cast<OLECHAR*>(reinterpret_cast<LPCOLESTR>(arg.Name.getStr()));
            }
        }

        //Prepare the array of DISPIDs for ITypeInfo::GetIDsOfNames
        //it must be big enough to contain the DISPIDs of the member + parameters
        arDispidNamedArgs.reset(new DISPID[nSizeAr]);
        HRESULT hr = getTypeInfo()->GetIDsOfNames(arNames, nSizeAr,
                                                  arDispidNamedArgs.get());
        if ( hr == E_NOTIMPL )
            hr = m_spDispatch->GetIDsOfNames(IID_NULL, arNames, nSizeAr, LOCALE_USER_DEFAULT, arDispidNamedArgs.get() );

        if (hr == S_OK)
        {
            // In a "property put" operation, the property value is a named param with the
            //special DISPID DISPID_PROPERTYPUT
            if (aFuncDesc->invkind == DISPATCH_PROPERTYPUT
                || aFuncDesc->invkind == DISPATCH_PROPERTYPUTREF)
            {
                //Element at index 0 in the DISPID array must be DISPID_PROPERTYPUT
                //The first item in the array arDispidNamedArgs is the DISPID for
                //the method. We replace it with DISPID_PROPERTYPUT.
                DISPID*  arIDs = arDispidNamedArgs.get();
                arIDs[0] = DISPID_PROPERTYPUT;
                dispparams.rgdispidNamedArgs = arIDs;
            }
            else
            {
                //The first item in the array arDispidNamedArgs is the DISPID for
                //the method. It must be removed
                DISPID*  arIDs = arDispidNamedArgs.get();
                dispparams.rgdispidNamedArgs = & arIDs[1];
            }
        }
        else if (hr == DISP_E_UNKNOWNNAME)
        {
             throw IllegalArgumentException(
                 OUSTR("[automation bridge]One of the named arguments is wrong!"),
                 Reference<XInterface>(), 0);
        }
        else
        {
            throw InvocationTargetException(
                OUSTR("[automation bridge] ITypeInfo::GetIDsOfNames returned error ")
                + OUString::valueOf((sal_Int32) hr, 16), Reference<XInterface>(), Any());
        }
    }

    //Convert arguments
    ptrArgs.reset(new CComVariant[dispparams.cArgs]);
    ptrRefArgs.reset(new CComVariant[dispparams.cArgs]);
    arArgs = ptrArgs.get();
    arRefArgs = ptrRefArgs.get();
    try
    {
        for (i = 0; i < (sal_Int32) dispparams.cArgs; i++)
        {
            revIndex= dispparams.cArgs - i -1;
            arRefArgs[revIndex].byref=0;
            Any  anyArg;
            if ( i < nUnoArgs)
                anyArg= Params.getConstArray()[i];

            unsigned short paramFlags = PARAMFLAG_FOPT | PARAMFLAG_FIN;
            VARTYPE varType = VT_VARIANT;
            if (aFuncDesc->cParamsOpt != -1 || aFuncDesc->cParams != (i + 1))
            {
                paramFlags = aFuncDesc->lprgelemdescParam[i].paramdesc.wParamFlags;
                varType = getElementTypeDesc(&aFuncDesc->lprgelemdescParam[i].tdesc);
            }

            // Make sure that there is a UNO parameter for every
            // expected parameter. If there is no UNO parameter where the
            // called function expects one, then it must be optional. Otherwise
            // its a UNO programming error.
            if (i  >= nUnoArgs && !(paramFlags & PARAMFLAG_FOPT))
            {
                OUStringBuffer buf(256);
                buf.appendAscii("ole automation bridge: The called function expects an argument at"
                                "position: "); //a different number of arguments")),
                buf.append(OUString::valueOf((sal_Int32) i));
                buf.appendAscii(" (index starting at 0).");
                throw IllegalArgumentException( buf.makeStringAndClear(),
                                                Reference<XInterface>(), (sal_Int16) i);
            }

            // Property Put arguments
            if (anyArg.getValueType() == getCppuType((PropertyPutArgument*)0))
            {
                PropertyPutArgument arg;
                anyArg >>= arg;
                anyArg <<= arg.Value;
            }
            // named argument
            if (anyArg.getValueType() == getCppuType((NamedArgument*) 0))
            {
                NamedArgument aNamedArgument;
                anyArg >>= aNamedArgument;
                anyArg <<= aNamedArgument.Value;
            }
            // out param
            if (paramFlags & PARAMFLAG_FOUT &&
                ! (paramFlags & PARAMFLAG_FIN)  )
            {
                VARTYPE type = ::sal::static_int_cast< VARTYPE, int >( varType ^ VT_BYREF );
                if (i < nUnoArgs)
                {
                    arRefArgs[revIndex].vt= type;
                }
                else
                {
                    //optional arg
                    arRefArgs[revIndex].vt = VT_ERROR;
                    arRefArgs[revIndex].scode = DISP_E_PARAMNOTFOUND;
                }
                if( type == VT_VARIANT )
                {
                    arArgs[revIndex].vt= VT_VARIANT | VT_BYREF;
                    arArgs[revIndex].byref= &arRefArgs[revIndex];
                }
                else
                {
                    arArgs[revIndex].vt= varType;
                    if (type == VT_DECIMAL)
                        arArgs[revIndex].byref= & arRefArgs[revIndex].decVal;
                    else
                        arArgs[revIndex].byref= & arRefArgs[revIndex].byref;
                }
            }
            // in/out  + in byref params
            else if (varType & VT_BYREF)
            {
                VARTYPE type = ::sal::static_int_cast< VARTYPE, int >( varType ^ VT_BYREF );
                CComVariant var;

                if (i < nUnoArgs && anyArg.getValueTypeClass() != TypeClass_VOID)
                {
                    anyToVariant( & arRefArgs[revIndex], anyArg, type);
                }
                else if (paramFlags & PARAMFLAG_FHASDEFAULT)
                {
                    //optional arg with default
                    VariantCopy( & arRefArgs[revIndex],
                                & aFuncDesc->lprgelemdescParam[i].paramdesc.
                                pparamdescex->varDefaultValue);
                }
                else
                {
                    //optional arg
                    //e.g: call func(x) in basic : func() ' no arg supplied
                    OSL_ASSERT(paramFlags & PARAMFLAG_FOPT);
                    arRefArgs[revIndex].vt = VT_ERROR;
                    arRefArgs[revIndex].scode = DISP_E_PARAMNOTFOUND;
                }

                // Set the converted arguments in the array which will be
                // DISPPARAMS::rgvarg
                // byref arg VT_XXX |VT_BYREF
                arArgs[revIndex].vt = varType;
                if (revIndex == 0 && aFuncDesc->invkind == INVOKE_PROPERTYPUT)
                {
                    arArgs[revIndex] = arRefArgs[revIndex];
                }
                else if (type == VT_DECIMAL)
                {
                    arArgs[revIndex].byref= & arRefArgs[revIndex].decVal;
                }
                else if (type == VT_VARIANT)
                {
                    if ( ! (paramFlags & PARAMFLAG_FOUT))
                        arArgs[revIndex] = arRefArgs[revIndex];
                    else
                        arArgs[revIndex].byref = & arRefArgs[revIndex];
                }
                else
                {
                    arArgs[revIndex].byref = & arRefArgs[revIndex].byref;
                    arArgs[revIndex].vt = ::sal::static_int_cast< VARTYPE, int >( arRefArgs[revIndex].vt | VT_BYREF );
                }

            }
            // in parameter no VT_BYREF except for array, interfaces
            else
            {   // void any stands for optional param
                if (i < nUnoArgs && anyArg.getValueTypeClass() != TypeClass_VOID)
                {
                    anyToVariant( & arArgs[revIndex], anyArg, varType);
                }
                //optional arg but no void any supplied
                //Basic:  obj.func() ' first parameter left out because it is optional
                else if (paramFlags & PARAMFLAG_FHASDEFAULT)
                {
                    //optional arg with defaulteithter as direct arg : VT_XXX or
                    VariantCopy( & arArgs[revIndex],
                        & aFuncDesc->lprgelemdescParam[i].paramdesc.
                            pparamdescex->varDefaultValue);
                }
                else if (paramFlags & PARAMFLAG_FOPT)
                {
                    arArgs[revIndex].vt = VT_ERROR;
                    arArgs[revIndex].scode = DISP_E_PARAMNOTFOUND;
                }
                else
                {
                    arArgs[revIndex].vt = VT_EMPTY;
                    arArgs[revIndex].lVal = 0;
                }
            }
        }
    }
    catch (IllegalArgumentException & e)
    {
        e.ArgumentPosition = ::sal::static_int_cast< sal_Int16, sal_Int32 >( i );
        throw;
    }
    catch (CannotConvertException & e)
    {
        e.ArgumentIndex = i;
        throw;
    }
    dispparams.rgvarg= arArgs;
    // invoking OLE method
    DWORD localeId = LOCALE_USER_DEFAULT;
    result = m_spDispatch->Invoke(aFuncDesc->memid,
                                 IID_NULL,
                                 localeId,
                                 ::sal::static_int_cast< WORD, INVOKEKIND >( aFuncDesc->invkind ),
                                 &dispparams,
                                 &varResult,
                                 &excepinfo,
                                 &uArgErr);

    // converting return value and out parameter back to UNO
    if (result == S_OK)
    {

        // allocate space for the out param Sequence and indices Sequence
        int outParamsCount= 0; // includes in/out parameter
        for (int i = 0; i < aFuncDesc->cParams; i++)
        {
            if (aFuncDesc->lprgelemdescParam[i].paramdesc.wParamFlags &
                PARAMFLAG_FOUT)
                outParamsCount++;
        }

        OutParamIndex.realloc(outParamsCount);
        OutParam.realloc(outParamsCount);
        // Convert out params
        if (outParamsCount)
        {
            int outParamIndex=0;
            for (int paramIndex = 0; paramIndex < nUnoArgs; paramIndex ++)
            {
                //Determine the index within the method sinature
                int realParamIndex = paramIndex;
                int revParamIndex = dispparams.cArgs - paramIndex - 1;
                if (Params[paramIndex].getValueType()
                    == getCppuType((NamedArgument*) 0))
                {
                    //dispparams.rgdispidNamedArgs contains the mapping from index
                    //of named args list to index of parameter list
                    realParamIndex = dispparams.rgdispidNamedArgs[revParamIndex];
                }

                // no named arg, always come before named args
                if (! (aFuncDesc->lprgelemdescParam[realParamIndex].paramdesc.wParamFlags
                       & PARAMFLAG_FOUT))
                    continue;
                Any outAny;
                // variantToAny is called with the "reduce range" parameter set to sal_False.
                // That causes VT_I4 values not to be converted down to a "lower" type. That
                // feature exist for JScript only because it only uses VT_I4 for integer types.
                try
                {
                    variantToAny( & arRefArgs[revParamIndex], outAny, sal_False );
                }
                catch (IllegalArgumentException & e)
                {
                    e.ArgumentPosition = (sal_Int16)paramIndex;
                    throw;
                }
                catch (CannotConvertException & e)
                {
                    e.ArgumentIndex = paramIndex;
                    throw;
                }
                OutParam[outParamIndex] = outAny;
                OutParamIndex[outParamIndex] = ::sal::static_int_cast< sal_Int16, int >( paramIndex );
                outParamIndex++;
            }
            OutParam.realloc(outParamIndex);
            OutParamIndex.realloc(outParamIndex);
        }
        // Return value
        variantToAny(&varResult, ret, sal_False);
    }

    // map error codes to exceptions
    OUString message;
    switch (result)
    {
        case S_OK:
            break;
        case DISP_E_BADPARAMCOUNT:
            throw IllegalArgumentException(OUSTR("[automation bridge] Wrong "
                  "number of arguments. Object returned DISP_E_BADPARAMCOUNT."),
                  0, 0);
            break;
        case DISP_E_BADVARTYPE:
            throw RuntimeException(OUSTR("[automation bridge] One or more "
                  "arguments have the wrong type. Object returned "
                  "DISP_E_BADVARTYPE."), 0);
            break;
        case DISP_E_EXCEPTION:
                message = OUSTR("[automation bridge]: ");
                message += OUString(reinterpret_cast<const sal_Unicode*>(excepinfo.bstrDescription),
                                    ::SysStringLen(excepinfo.bstrDescription));

                throw InvocationTargetException(message, Reference<XInterface>(), Any());
            break;
        case DISP_E_MEMBERNOTFOUND:
            message = OUSTR("[automation bridge]: A function with the name \"")
                + sFuncName + OUSTR("\" is not supported. Object returned "
                "DISP_E_MEMBERNOTFOUND.");
            throw IllegalArgumentException(message, 0, 0);
            break;
        case DISP_E_NONAMEDARGS:
            throw IllegalArgumentException(OUSTR("[automation bridge] Object "
                  "returned DISP_E_NONAMEDARGS"),0, ::sal::static_int_cast< sal_Int16, unsigned int >( uArgErr ));
            break;
        case DISP_E_OVERFLOW:
            throw CannotConvertException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[automation bridge] Call failed.")),
                                         static_cast<XInterface*>(
                static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::OUT_OF_RANGE, uArgErr);
            break;
        case DISP_E_PARAMNOTFOUND:
            throw IllegalArgumentException(OUSTR("[automation bridge]Call failed."
                                                 "Object returned DISP_E_PARAMNOTFOUND."),
                                           0, ::sal::static_int_cast< sal_Int16, unsigned int >( uArgErr ));
            break;
        case DISP_E_TYPEMISMATCH:
            throw CannotConvertException(OUSTR("[automation bridge] Call  failed. "
                                         "Object returned DISP_E_TYPEMISMATCH"),
                static_cast<XInterface*>(
                static_cast<XWeak*>(this)) , TypeClass_UNKNOWN, FailReason::UNKNOWN, uArgErr);
            break;
        case DISP_E_UNKNOWNINTERFACE:
            throw RuntimeException(OUSTR("[automation bridge] Call failed. "
                                       "Object returned DISP_E_UNKNOWNINTERFACE."),0);
            break;
        case DISP_E_UNKNOWNLCID:
            throw RuntimeException(OUSTR("[automation bridge] Call failed. "
                                       "Object returned DISP_E_UNKNOWNLCID."),0);
            break;
        case DISP_E_PARAMNOTOPTIONAL:
            throw CannotConvertException(OUSTR("[automation bridge] Call failed."
                  "Object returned DISP_E_PARAMNOTOPTIONAL"),
                        static_cast<XInterface*>(static_cast<XWeak*>(this)),
                              TypeClass_UNKNOWN, FailReason::NO_DEFAULT_AVAILABLE, uArgErr);
            break;
        default:
            throw RuntimeException();
            break;
    }

    return ret;
}

void IUnknownWrapper_Impl::getFuncDescForInvoke(const OUString & sFuncName,
                                                const Sequence<Any> & seqArgs,
                                                FUNCDESC** pFuncDesc)
{
    int nUnoArgs = seqArgs.getLength();
    const Any * arArgs = seqArgs.getConstArray();
    ITypeInfo* pInfo = getTypeInfo();

    //If the last of the positional arguments is a PropertyPutArgument
    //then obtain the type info for the property put operation.

    //The property value is always the last argument, in a positional argument list
    //or in a list of named arguments. A PropertyPutArgument is actually a named argument
    //hence it must not be put in an extra NamedArgument structure
    if (nUnoArgs > 0 &&
        arArgs[nUnoArgs - 1].getValueType() == getCppuType((PropertyPutArgument*) 0))
    {
        // DISPATCH_PROPERTYPUT
        FuncDesc aDescGet(pInfo);
        FuncDesc aDescPut(pInfo);
        VarDesc aVarDesc(pInfo);
        getPropDesc(sFuncName, & aDescGet, & aDescPut, & aVarDesc);
        if ( ! aDescPut)
        {
            throw IllegalArgumentException(
                OUSTR("[automation bridge] The object does not have a writeable property: ")
                + sFuncName, Reference<XInterface>(), 0);
        }
        *pFuncDesc = aDescPut.Detach();
    }
    else
    {   // DISPATCH_METHOD
        FuncDesc aFuncDesc(pInfo);
        getFuncDesc(sFuncName, & aFuncDesc);
        if ( ! aFuncDesc)
        {
            // Fallback: DISPATCH_PROPERTYGET can mostly be called as
            // DISPATCH_METHOD
            ITypeInfo * pInfo = getTypeInfo();
            FuncDesc aDescPut(pInfo);
            VarDesc aVarDesc(pInfo);
            getPropDesc(sFuncName, & aFuncDesc, & aDescPut, & aVarDesc);
            if ( ! aFuncDesc )
            {
                throw IllegalArgumentException(
                    OUSTR("[automation bridge] The object does not have a function"
                          "or readable property \"")
                    + sFuncName, Reference<XInterface>(), 0);
            }
        }
        *pFuncDesc = aFuncDesc.Detach();
    }
}
bool IUnknownWrapper_Impl::getDispid(const OUString& sFuncName, DISPID * id)
{
    OSL_ASSERT(m_spDispatch);
    LPOLESTR lpsz = const_cast<LPOLESTR> (reinterpret_cast<LPCOLESTR>(sFuncName.getStr()));
    HRESULT hr = m_spDispatch->GetIDsOfNames(IID_NULL, &lpsz, 1, LOCALE_USER_DEFAULT, id);
    return hr == S_OK ? true : false;
}
void IUnknownWrapper_Impl::getFuncDesc(const OUString & sFuncName, FUNCDESC ** pFuncDesc)

{
    OSL_ASSERT( * pFuncDesc == 0);
    buildComTlbIndex();
    typedef TLBFuncIndexMap::const_iterator cit;
        typedef TLBFuncIndexMap::iterator it;
    //We assume there is only one entry with the function name. A property
    //would have two entries.
    cit itIndex= m_mapComFunc.find(sFuncName);
    if (itIndex == m_mapComFunc.end())
    {
        //try case insensive with IDispatch::GetIDsOfNames
        DISPID id;
        if (getDispid(sFuncName, &id))
        {
            CComBSTR memberName;
            unsigned int pcNames=0;
            // get the case sensitive name
            if( SUCCEEDED(getTypeInfo()->GetNames( id, & memberName, 1, &pcNames)))
            {
                //get the associated index and add an entry to the map
                //with the name sFuncName which differs in the casing of the letters to
                //the actual name as obtained from ITypeInfo
                OUString sRealName(reinterpret_cast<const sal_Unicode*>(LPCOLESTR(memberName)));
                cit itOrg  = m_mapComFunc.find(sRealName);
                OSL_ASSERT(itOrg != m_mapComFunc.end());
                // maybe this is a property, if so we need
                // to store either both id's ( put/get ) or
                // just the get. Storing both is more consistent
                pair<cit, cit> pItems = m_mapComFunc.equal_range( sRealName );
                for ( ;pItems.first != pItems.second; ++pItems.first )
                    m_mapComFunc.insert( TLBFuncIndexMap::value_type ( make_pair(sFuncName, pItems.first->second ) ));
                itIndex =
                    m_mapComFunc.find( sFuncName );
            }
        }
    }

#if OSL_DEBUG_LEVEL >= 1
    // There must only be one entry if sFuncName represents a function or two
    // if it is a property
    pair<cit, cit> p = m_mapComFunc.equal_range(sFuncName.toAsciiLowerCase());
    int numEntries = 0;
    for ( ;p.first != p.second; p.first ++, numEntries ++);
    OSL_ASSERT( ! (numEntries > 3) );
#endif
    if( itIndex != m_mapComFunc.end())
    {
        ITypeInfo* pType= getTypeInfo();
        FUNCDESC * pDesc = NULL;
        if (SUCCEEDED(pType->GetFuncDesc(itIndex->second, & pDesc)))
        {
            if (pDesc->invkind == INVOKE_FUNC)
            {
                (*pFuncDesc) = pDesc;
            }
            else
            {
                pType->ReleaseFuncDesc(pDesc);
            }
        }
        else
        {
            throw BridgeRuntimeError(OUSTR("[automation bridge] Could not get "
                                           "FUNCDESC for ") + sFuncName);
        }
    }
   //else no entry found for sFuncName, pFuncDesc will not be filled in
}

void IUnknownWrapper_Impl::getPropDesc(const OUString & sFuncName, FUNCDESC ** pFuncDescGet,
                                       FUNCDESC** pFuncDescPut, VARDESC** pVarDesc)
{
    OSL_ASSERT( * pFuncDescGet == 0 && * pFuncDescPut == 0);
    buildComTlbIndex();
    typedef TLBFuncIndexMap::const_iterator cit;
    pair<cit, cit> p = m_mapComFunc.equal_range(sFuncName);
    if (p.first == m_mapComFunc.end())
    {
        //try case insensive with IDispatch::GetIDsOfNames
        DISPID id;
        if (getDispid(sFuncName, &id))
        {
            CComBSTR memberName;
            unsigned int pcNames=0;
            // get the case sensitive name
            if( SUCCEEDED(getTypeInfo()->GetNames( id, & memberName, 1, &pcNames)))
            {
                //As opposed to getFuncDesc, we do not add the value because we would
                // need to find the get and set description for the property. This would
                //mean to iterate over all FUNCDESCs again.
                p = m_mapComFunc.equal_range(OUString(reinterpret_cast<const sal_Unicode*>(LPCOLESTR(memberName))));
            }
        }
    }

    for ( int i = 0 ;p.first != p.second; p.first ++, i ++)
    {
        // There are a maximum of two entries, property put and property get
        OSL_ASSERT( ! (i > 2) );
        ITypeInfo* pType= getTypeInfo();
        FUNCDESC * pFuncDesc = NULL;
        if (SUCCEEDED( pType->GetFuncDesc(p.first->second, & pFuncDesc)))
        {
            if (pFuncDesc->invkind == INVOKE_PROPERTYGET)
            {
                (*pFuncDescGet) = pFuncDesc;
            }
            else if (pFuncDesc->invkind == INVOKE_PROPERTYPUT ||
                     pFuncDesc->invkind == INVOKE_PROPERTYPUTREF)
            {
                //a property can have 3 entries, put, put ref, get
                // If INVOKE_PROPERTYPUTREF or INVOKE_PROPERTYPUT is used
                //depends on what is found first.
                if ( * pFuncDescPut)
                {
                    //we already have found one
                    pType->ReleaseFuncDesc(pFuncDesc);
                }
                else
                {
                    (*pFuncDescPut) = pFuncDesc;
                }
            }
            else
            {
                pType->ReleaseFuncDesc(pFuncDesc);
            }
        }
        //ITypeInfo::GetFuncDesc may even provide a funcdesc for a VARDESC
        // with invkind = INVOKE_FUNC. Since this function should only return
        //a value for a real property (XInvokation::hasMethod, ..::hasProperty
        //we need to make sure that sFuncName represents a real property.
        VARDESC * pVD = NULL;
        if (SUCCEEDED(pType->GetVarDesc(p.first->second, & pVD)))
            (*pVarDesc) = pVD;
    }
   //else no entry for sFuncName, pFuncDesc will not be filled in
}

VARTYPE IUnknownWrapper_Impl::getUserDefinedElementType( ITypeInfo* pTypeInfo, const DWORD nHrefType )
{
    VARTYPE _type( VT_NULL );
    if ( pTypeInfo )
    {
        CComPtr<ITypeInfo> spRefInfo;
        pTypeInfo->GetRefTypeInfo( nHrefType, &spRefInfo.p );
        if ( spRefInfo )
        {
            TypeAttr attr( spRefInfo );
            spRefInfo->GetTypeAttr( &attr );
            if ( attr->typekind == TKIND_ENUM )
            {
                // We use the type of the first enum value.
                if ( attr->cVars == 0 )
                {
                    throw BridgeRuntimeError(OUSTR("[automation bridge] Could not obtain type description"));
                }
                VarDesc var( spRefInfo );
                spRefInfo->GetVarDesc( 0, &var );
                _type = var->lpvarValue->vt;
            }
            else if ( attr->typekind == TKIND_INTERFACE )
            {
                _type = VT_UNKNOWN;
            }
            else if ( attr->typekind == TKIND_DISPATCH )
            {
                _type = VT_DISPATCH;
            }
            else if ( attr->typekind == TKIND_ALIAS )
            {
                // TKIND_ALIAS is a type that is an alias for another type. So get that alias type.
                _type = getUserDefinedElementType( pTypeInfo, attr->tdescAlias.hreftype );
            }
            else
            {
                throw BridgeRuntimeError( OUSTR("[automation bridge] Unhandled user defined type.") );
            }
        }
    }
    return _type;
}

VARTYPE IUnknownWrapper_Impl::getElementTypeDesc(const TYPEDESC *desc)
{
    VARTYPE _type( VT_NULL );

    if (desc->vt == VT_PTR)
    {
        _type = getElementTypeDesc(desc->lptdesc);
        _type |= VT_BYREF;
    }
    else if (desc->vt == VT_SAFEARRAY)
    {
        _type = getElementTypeDesc(desc->lptdesc);
        _type |= VT_ARRAY;
    }
    else if (desc->vt == VT_USERDEFINED)
    {
        ITypeInfo* thisInfo = getTypeInfo(); //kept by this instance
        _type = getUserDefinedElementType( thisInfo, desc->hreftype );
    }
    else
    {
        _type = desc->vt;
    }
    return _type;
}

void IUnknownWrapper_Impl::buildComTlbIndex()
{
    if ( ! m_bComTlbIndexInit)
    {
        MutexGuard guard(getBridgeMutex());
        {
            if ( ! m_bComTlbIndexInit)
            {
                OUString sError;
                ITypeInfo* pType= getTypeInfo();
                TypeAttr typeAttr(pType);
                if( SUCCEEDED( pType->GetTypeAttr( &typeAttr)))
                {
                    for( long i= 0; i < typeAttr->cFuncs; i++)
                    {
                        FuncDesc funcDesc(pType);
                        if( SUCCEEDED( pType->GetFuncDesc( i, &funcDesc)))
                        {
                            CComBSTR memberName;
                            unsigned int pcNames=0;
                            if( SUCCEEDED(pType->GetNames( funcDesc->memid, & memberName, 1, &pcNames)))
                            {
                                OUString usName(reinterpret_cast<const sal_Unicode*>(LPCOLESTR(memberName)));
                                m_mapComFunc.insert( TLBFuncIndexMap::value_type( usName, i));
                            }
                            else
                            {
                                sError = OUSTR("[automation bridge] IUnknownWrapper_Impl::buildComTlbIndex, " \
                                                "ITypeInfo::GetNames failed.");
                            }
                        }
                        else
                            sError = OUSTR("[automation bridge] IUnknownWrapper_Impl::buildComTlbIndex, " \
                                            "ITypeInfo::GetFuncDesc failed.");
                    }

                    //If we create an Object in JScript and a a property then it
                    //has VARDESC instead of FUNCDESC
                    for (long i = 0; i < typeAttr->cVars; i++)
                    {
                        VarDesc varDesc(pType);
                        if (SUCCEEDED(pType->GetVarDesc(i, & varDesc)))
                        {
                            CComBSTR memberName;
                            unsigned int pcNames = 0;
                            if (SUCCEEDED(pType->GetNames(varDesc->memid, & memberName, 1, &pcNames)))
                            {
                                if (varDesc->varkind == VAR_DISPATCH)
                                {
                                    OUString usName(reinterpret_cast<const sal_Unicode*>(LPCOLESTR(memberName)));
                                    m_mapComFunc.insert(TLBFuncIndexMap::value_type(
                                                        usName, i));
                                }
                            }
                            else
                            {
                                sError = OUSTR("[automation bridge] IUnknownWrapper_Impl::buildComTlbIndex, " \
                                                "ITypeInfo::GetNames failed.");
                            }
                        }
                        else
                            sError = OUSTR("[automation bridge] IUnknownWrapper_Impl::buildComTlbIndex, " \
                                           "ITypeInfo::GetVarDesc failed.");

                    }
                }
                else
                    sError = OUSTR("[automation bridge] IUnknownWrapper_Impl::buildComTlbIndex, " \
                                    "ITypeInfo::GetTypeAttr failed.");

                if (sError.getLength())
                {
                    throw BridgeRuntimeError(sError);
                }

                m_bComTlbIndexInit = true;
            }
        }
    }
}

ITypeInfo* IUnknownWrapper_Impl::getTypeInfo()
{
    if( !m_spDispatch)
    {
        throw BridgeRuntimeError(OUSTR("The object has no IDispatch interface!"));
    }

    if( !m_spTypeInfo )
    {
        MutexGuard guard(getBridgeMutex());
        if( ! m_spTypeInfo)
        {
            CComPtr< ITypeInfo > spType;
            if( SUCCEEDED( m_spDispatch->GetTypeInfo( 0, LOCALE_USER_DEFAULT, &spType.p)))

            {
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();

                //If this is a dual interface then TYPEATTR::typekind is usually TKIND_INTERFACE
                //We need to get the type description for TKIND_DISPATCH
                TypeAttr typeAttr(spType.p);
                if( SUCCEEDED(spType->GetTypeAttr( &typeAttr)))
                {
                    if (typeAttr->typekind == TKIND_INTERFACE &&
                            typeAttr->wTypeFlags & TYPEFLAG_FDUAL)
                    {
                        HREFTYPE refDispatch;
                        if (SUCCEEDED(spType->GetRefTypeOfImplType(::sal::static_int_cast< UINT, int >( -1 ), &refDispatch)))
                        {
                            CComPtr<ITypeInfo> spTypeDisp;
                            if (SUCCEEDED(spType->GetRefTypeInfo(refDispatch, & spTypeDisp)))
                                m_spTypeInfo= spTypeDisp;
                        }
                        else
                        {
                            throw BridgeRuntimeError(
                                OUSTR("[automation bridge] Could not obtain type information "
                                "for dispatch interface." ));
                        }
                    }
                    else if (typeAttr->typekind == TKIND_DISPATCH)
                    {
                        m_spTypeInfo= spType;
                    }
                    else
                    {
                        throw BridgeRuntimeError(
                            OUSTR("[automation bridge] Automation object does not "
                            "provide type information."));
                    }
                }
            }
            else
            {
                throw BridgeRuntimeError(OUSTR("[automation bridge]The dispatch object does not "
                                               "support ITypeInfo!"));
            }
        }
    }
    return m_spTypeInfo;
}

} // end namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
