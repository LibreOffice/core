/*************************************************************************
 *
 *  $RCSfile: oleobjw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:40 $
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

#include "ole2uno.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <osl/diagnose.h>

#ifndef _COM_SUN_STAR_SCRIPT_FAILREASON_HPP_
#include <com/sun/star/script/FailReason.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMATERIALHOLDER_HPP_
#include <com/sun/star/beans/XMaterialHolder.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FINISHENGINEEVENT_HPP_
#include <com/sun/star/script/FinishEngineEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_INTERRUPTREASON_HPP_
#include <com/sun/star/script/InterruptReason.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XENGINELISTENER_HPP_
#include <com/sun/star/script/XEngineListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XDEBUGGING_HPP_
#include <com/sun/star/script/XDebugging.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_CONTEXTINFORMATION_HPP_
#include <com/sun/star/script/ContextInformation.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FINISHREASON_HPP_
#include <com/sun/star/script/FinishReason.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XENGINE_HPP_
#include <com/sun/star/script/XEngine.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_INTERRUPTENGINEEVENT_HPP_
#include <com/sun/star/script/InterruptEngineEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYACCESS_HPP_
#include <com/sun/star/script/XLibraryAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XBRIDGESUPPLIER_HPP_
#include <com/sun/star/bridge/XBridgeSupplier.hpp>
#endif
//#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
//#include <com/sun/star/lang/XUnoTunnel.hpp>
//#endif

#ifndef _COM_SUN_STAR_BRIDGE_MODELDEPENDENT_HPP_
#include <com/sun/star/bridge/ModelDependent.hpp>
#endif

#include <typelib/typedescription.hxx>
#include <rtl/uuid.h>
#include <rtl/memory.h>
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//#include <tools/presys.h>

#include "jscriptclasses.hxx"
//#include <tools/postsys.h>

#include "oleobjw.hxx"
#include "unoobjw.hxx"

using namespace std;
using namespace osl;
using namespace rtl;
using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;

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
hash_map<sal_uInt32,sal_uInt32> AdapterToWrapperMap;
// key: XInterface of the wrapper object.
// value: XInterface of the Interface created by the Invocation Adapter Factory.
// A COM wrapper is responsible for removing the corresponding entry
// in AdapterToWrappperMap if it is being destroyed. Because the wrapper does not
// know about its adapted interface it uses WrapperToAdapterMap to get the
// adapted interface which is then used to locate the entry in AdapterToWrapperMap.
hash_map<sal_uInt32,sal_uInt32> WrapperToAdapterMap;


/*****************************************************************************

    class implementation IUnknownWrapper_Impl

*****************************************************************************/

IUnknownWrapper_Impl::IUnknownWrapper_Impl( Reference<XMultiServiceFactory>& xFactory,
                                           sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
    UnoConversionUtilities<IUnknownWrapper_Impl>( xFactory, unoWrapperClass, comWrapperClass),
    m_pxIdlClass( NULL), m_pDispatch(NULL), m_eJScript( JScriptUndefined)
{
}


IUnknownWrapper_Impl::~IUnknownWrapper_Impl()
{
    OGuard guard( globalWrapperMutex);
    acquire(); // make sure we don't delete us twice
    Reference<XInterface> xInt( static_cast<XWeak*>(this), UNO_QUERY);

    // remove entries in global maps
    typedef hash_map<sal_uInt32, sal_uInt32>::iterator _IT;
    _IT it= WrapperToAdapterMap.find( (sal_uInt32) xInt.get());
    if( it != WrapperToAdapterMap.end())
    {
        sal_uInt32 adapter= it->second;

        AdapterToWrapperMap.erase( adapter);
        WrapperToAdapterMap.erase( it);
    }

    o2u_attachCurrentThread();

    m_pUnknown->Release();
    if (m_pDispatch)
    {
        m_pDispatch->Release();
    }

}



Reference<XIntrospectionAccess> SAL_CALL IUnknownWrapper_Impl::getIntrospection(void)
    throw (RuntimeException )
{
    Reference<XIntrospectionAccess> ret;

    return ret;
}

DispIdMap::iterator IUnknownWrapper_Impl::getDispIdEntry(const OUString& name)
{

    DispIdMap::iterator iter = m_dispIdMap.find(name);

    if (iter == m_dispIdMap.end())
    {
        unsigned int    flags = 0;
        HRESULT         result;
        DISPID          dispId;
        OLECHAR*        oleNames[1];

        oleNames[0] = (OLECHAR*)name.getStr();

        result = m_pDispatch->GetIDsOfNames(IID_NULL,
                                            oleNames,
                                            1,
                                            LOCALE_SYSTEM_DEFAULT,
                                            &dispId);

        if (result == NOERROR)
        {
            flags |= DISPATCH_METHOD;
            flags |= DISPATCH_PROPERTYPUT;
            flags |= DISPATCH_PROPERTYPUTREF;

            DISPPARAMS      dispparams;
            VARIANT         varResult;
            EXCEPINFO       excepinfo;
            unsigned int    uArgErr;

            VariantInit(&varResult);

            // converting UNO parameters to OLE variants
            dispparams.rgdispidNamedArgs = NULL;
            dispparams.cArgs = 0;
            dispparams.cNamedArgs = 0;
            dispparams.rgvarg = NULL;

            // try to invoke PROPERTY_GET to evaluate if this name denotes a property
            result = m_pDispatch->Invoke(dispId,
                                         IID_NULL,
                                         LOCALE_SYSTEM_DEFAULT,
                                         DISPATCH_PROPERTYGET,
                                         &dispparams,
                                         &varResult,
                                         &excepinfo,
                                         &uArgErr);

            if (result == S_OK)
            {
                flags |= DISPATCH_PROPERTYGET;
            }

            // freeing allocated OLE parameters
            VariantClear(&varResult);

            if (flags != 0)
            {
                iter = ((DispIdMap&)m_dispIdMap).insert(
                    DispIdMap::value_type(name, pair<DISPID, unsigned short>(dispId, flags))).first;
            }
        }
    }

    return iter;
}


Any SAL_CALL IUnknownWrapper_Impl::invoke( const OUString& aFunctionName,
             const Sequence< Any >& aParams, Sequence< sal_Int16 >& aOutParamIndex,
             Sequence< Any >& aOutParam )
    throw(IllegalArgumentException, CannotConvertException, InvocationTargetException,
          RuntimeException)
{
    setCurrentInvokeCall( aFunctionName);

    Any ret;

    DispIdMap::iterator iter = getDispIdEntry(aFunctionName);

    if ((iter != m_dispIdMap.end()) && (((*iter).second.second & DISPATCH_METHOD) != 0))
    {
        TypeDescription methodDesc;
        getMethodInfo( methodDesc );
        if( methodDesc.is())
            ret = invokeWithDispIdUnoTlb((*iter).second.first,
                               aParams,
                               aOutParamIndex,
                               aOutParam);
        else
            ret= invokeWithDispIdComTlb( (*iter).second.first, //DISPID
                                         aParams,
                                         aOutParamIndex,
                                         aOutParam);

    }
    else
        throw IllegalArgumentException();

    return ret;
}

void SAL_CALL IUnknownWrapper_Impl::setValue( const OUString& aPropertyName,
                 const Any& aValue )
    throw(UnknownPropertyException, CannotConvertException, InvocationTargetException,
          RuntimeException)
{

    DispIdMap::iterator iter = getDispIdEntry(aPropertyName);

    if ((iter != m_dispIdMap.end()) && (((*iter).second.second & DISPATCH_PROPERTYPUT) != 0))
    {
        setValueWithDispId((*iter).second.first, aValue);
    }
    else
        throw UnknownPropertyException();
}

Any SAL_CALL IUnknownWrapper_Impl::getValue( const OUString& aPropertyName )
        throw(UnknownPropertyException, RuntimeException)
{
    setCurrentGetCall( aPropertyName);

    Any ret;

    DispIdMap::iterator iter = getDispIdEntry(aPropertyName);

    if (iter != m_dispIdMap.end() && (((*iter).second.second & DISPATCH_PROPERTYGET) != 0))
    {
        ret = getValueWithDispId((*iter).second.first);
    }
    else
        throw UnknownPropertyException();

    return ret;
}

sal_Bool SAL_CALL IUnknownWrapper_Impl::hasMethod( const OUString& aName )
        throw(RuntimeException)
{
    sal_Bool ret = FALSE;

    o2u_attachCurrentThread();

    DispIdMap::iterator iter = ((IUnknownWrapper_Impl*)this)->getDispIdEntry(aName);

    ret = (
            (iter != ((IUnknownWrapper_Impl*)this)->m_dispIdMap.end()) &&
            (((*iter).second.second & DISPATCH_METHOD) != 0)
          );

    return ret;
}

sal_Bool SAL_CALL IUnknownWrapper_Impl::hasProperty( const OUString& aName )
        throw(RuntimeException)
{
    sal_Bool ret = FALSE;

    o2u_attachCurrentThread();

    DispIdMap::iterator iter = ((IUnknownWrapper_Impl*)this)->getDispIdEntry(aName);

    ret = (
            (iter != ((IUnknownWrapper_Impl*)this)->m_dispIdMap.end()) &&
            (((*iter).second.second & DISPATCH_PROPERTYGET) != 0)
          );

    return ret;
}

Any SAL_CALL IUnknownWrapper_Impl::createBridge( const Any& modelDepObject,
                const Sequence< sal_Int8 >& aProcessId, sal_Int16 sourceModelType,
                 sal_Int16 destModelType )
    throw( IllegalArgumentException, RuntimeException)
{
    Any ret;

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

            if (m_pDispatch != NULL)
            {
                V_VT(pVariant) = VT_DISPATCH;
                V_DISPATCH(pVariant) = m_pDispatch;
                m_pDispatch->AddRef();
            }
            else
            {
                V_VT(pVariant) = VT_UNKNOWN;
                V_UNKNOWN(pVariant) = m_pUnknown;
                m_pUnknown->AddRef();
            }

            ret.setValue((void*)&pVariant, getCppuType( (sal_uInt32*) 0));
        }
    }

    return ret;
}

Any  IUnknownWrapper_Impl::invokeWithDispIdUnoTlb(DISPID dispID,
                          const Sequence< Any >& Params,
                          Sequence< sal_Int16 >& OutParamIndex,
                          Sequence< Any >& OutParam)
              throw ( IllegalArgumentException, CannotConvertException,
                      InvocationTargetException, RuntimeException)
{
    Any ret;
    HRESULT hr= S_OK;
    o2u_attachCurrentThread();


    sal_Int32 parameterCount= Params.getLength();
    sal_Int32 outParameterCount= 0;
    typelib_InterfaceMethodTypeDescription* pMethod= NULL;
    TypeDescription methodDesc;
    getMethodInfo( methodDesc);

    // We need to know whether the IDispatch is from a JScript object.
    // Then out and in/out parameters have to be treated differently than
    // with common COM objects.
    sal_Bool bJScriptObject= isJScriptObject();
//  CComDispatchDriver disp( m_pDispatch);

    CComVariant *pVarParams= NULL;
    CComVariant *pVarParamsRef= NULL;
    sal_Bool bConvOk= sal_True;
    sal_Bool bConvRet= sal_True;

    if( methodDesc.is())
    {
        pMethod=    ( typelib_InterfaceMethodTypeDescription* )methodDesc.get();
        parameterCount= pMethod->nParams;
        // Create the Array for the array being passed in DISPPARAMS
        // the array also contains the outparameter (but not the values)
        if( pMethod->nParams > 0)
            pVarParams= new CComVariant[ parameterCount];
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
            pVarParamsRef= new CComVariant[ outParameterCount];
            // build up the parameters for IDispatch::Invoke
            sal_Int32 inParamIndex=0;
            sal_Int32 outParamIndex=0;

            for( i= 0; i < parameterCount; i++)
            {

                // In parameter
                if( pMethod->pParams[i].bIn == sal_True && ! pMethod->pParams[i].bOut)
                {
                    bConvOk= anyToVariant( &pVarParams[parameterCount - i -1], Params[inParamIndex++]);
                }
                // Out parameter + in/out parameter
                else if( pMethod->pParams[i].bOut == sal_True)
                {
                    CComVariant var;
                    switch( pMethod->pParams[i].pTypeRef->eTypeClass)
                    {
                    case TypeClass_INTERFACE:
                    case TypeClass_STRUCT:
                        if( pMethod->pParams[i].bIn &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
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
                        if( pMethod->pParams[i].bIn  &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_I4;
                            pVarParamsRef[ outParamIndex].lVal= 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_I4 | VT_BYREF;
                        pVarParams[parameterCount - i -1].plVal= &pVarParamsRef[outParamIndex].lVal;
                        break;
                    case TypeClass_SEQUENCE:
                        if( pMethod->pParams[i].bIn &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_ARRAY| VT_VARIANT;
                            pVarParamsRef[ outParamIndex].parray= NULL;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_ARRAY| VT_BYREF | VT_VARIANT;
                        pVarParams[parameterCount - i -1].pparray= &pVarParamsRef[outParamIndex].parray;
                        break;
                    case TypeClass_ANY:
                        if( pMethod->pParams[i].bIn &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_EMPTY;
                            pVarParamsRef[ outParamIndex].lVal = 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_VARIANT | VT_BYREF;
                        pVarParams[parameterCount - i -1].pvarVal= &pVarParamsRef[outParamIndex];
                        break;
                    case TypeClass_BOOLEAN:
                        if( pMethod->pParams[i].bIn &&  (bConvOk= anyToVariant( &var,Params[i])) )
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_BOOL;
                            pVarParamsRef[ outParamIndex].boolVal = 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_BOOL| VT_BYREF;
                        pVarParams[parameterCount - i -1].pboolVal= &pVarParamsRef[outParamIndex].boolVal;
                        break;

                    case TypeClass_STRING:
                        if( pMethod->pParams[i].bIn &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_BSTR;
                            pVarParamsRef[ outParamIndex].bstrVal= 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_BSTR| VT_BYREF;
                        pVarParams[parameterCount - i -1].pbstrVal= &pVarParamsRef[outParamIndex].bstrVal;
                        break;

                    case TypeClass_FLOAT:
                        if( pMethod->pParams[i].bIn &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_R4;
                            pVarParamsRef[ outParamIndex].fltVal= 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_R4| VT_BYREF;
                        pVarParams[parameterCount - i -1].pfltVal= &pVarParamsRef[outParamIndex].fltVal;
                        break;
                    case TypeClass_DOUBLE:
                        if( pMethod->pParams[i].bIn&&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_R8;
                            pVarParamsRef[ outParamIndex].dblVal= 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_R8| VT_BYREF;
                        pVarParams[parameterCount - i -1].pdblVal= &pVarParamsRef[outParamIndex].dblVal;
                        break;
                    case TypeClass_BYTE:
                        if( pMethod->pParams[i].bIn &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_UI1;
                            pVarParamsRef[ outParamIndex].bVal= 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_UI1| VT_BYREF;
                        pVarParams[parameterCount - i -1].pbVal= &pVarParamsRef[outParamIndex].bVal;
                        break;
                    case TypeClass_CHAR:
                    case TypeClass_SHORT:
                    case TypeClass_UNSIGNED_SHORT:
                        if( pMethod->pParams[i].bIn&&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_I2;
                            pVarParamsRef[ outParamIndex].iVal = 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_I2| VT_BYREF;
                        pVarParams[parameterCount - i -1].piVal= &pVarParamsRef[outParamIndex].iVal;
                        break;

                    default:
                        if( pMethod->pParams[i].bIn  &&  (bConvOk= anyToVariant( &var,Params[i])))
                            pVarParamsRef[ outParamIndex]= var;
                        else
                        {
                            pVarParamsRef[ outParamIndex].vt = VT_EMPTY;
                            pVarParamsRef[ outParamIndex].lVal = 0;
                        }
                        pVarParams[parameterCount - i -1].vt = VT_VARIANT | VT_BYREF;
                        pVarParams[parameterCount - i -1].pvarVal= &pVarParamsRef[outParamIndex];
                    }
                    outParamIndex++;
                } // end else if
                if( ! bConvOk) break;
            } // end for
        }
        else // it is an JScriptObject
        {
            sal_Int32 inParamIndex= 0;
            for( sal_Int32 i= 0; i< parameterCount; i++)
            {
                // In parameter
                if( pMethod->pParams[i].bIn == sal_True && ! pMethod->pParams[i].bOut)
                {
                    bConvOk= anyToVariant( &pVarParams[parameterCount - i -1], Params[inParamIndex++]);
                }
                // Out parameter + in/out parameter
                else if( pMethod->pParams[i].bOut == sal_True)
                {
                    CComObject<JScriptOutParam>* pParamObject;
                    if( SUCCEEDED( CComObject<JScriptOutParam>::CreateInstance( &pParamObject)))
                    {
                        CComPtr<IUnknown> pUnk(pParamObject->GetUnknown());
                        CComQIPtr<IDispatch> pDisp( pUnk);

                        pVarParams[ parameterCount - i -1].vt= VT_DISPATCH;
                        pVarParams[ parameterCount - i -1].pdispVal= pDisp;
                        pVarParams[ parameterCount - i -1].pdispVal->AddRef();
                        // if the param is in/out then put the parameter on index 0
                        if( pMethod->pParams[i].bIn == sal_True ) // in / out
                        {
                            CComVariant varParam;
                            bConvOk= anyToVariant( &varParam, Params[ inParamIndex++]);
                            if( bConvOk)
                            {
                                CComDispatchDriver dispDriver( pDisp);
                                if( pDisp)
                                    if( FAILED( dispDriver.PutPropertyByName( L"0", &varParam)))
                                        bConvOk= sal_False;
                            }
                        }
                    }
                }
                if( ! bConvOk) break;
            }
        }
    }
    // No type description Available, that is we have to deal with a COM component,
    // that does not implements UNO interfaces ( IDispatch based)
    else
    {
        if( parameterCount)
        {
            pVarParams= new CComVariant[parameterCount];
            CComVariant var;
            for( sal_Int32 i= 0; i < parameterCount; i++)
            {
                var.Clear();
                if( bConvOk= anyToVariant( &var, Params[i]))
                    pVarParams[ parameterCount - 1 -i]= var;
            }
        }
    }

    if( bConvOk)
    {
        CComVariant     varResult;
        EXCEPINFO       excepinfo;
        unsigned int    uArgErr;
        DISPPARAMS dispparams= { pVarParams, NULL, parameterCount, 0};
        // invoking OLE method
        hr = m_pDispatch->Invoke(dispID,
                                     IID_NULL,
                                     LOCALE_SYSTEM_DEFAULT,
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
                for( sal_Int32 i= 0; i < parameterCount; i++)
                {
                    if( pMethod->pParams[i].bOut )
                    {
                        OutParamIndex[outIndex]= i;
                        Any outAny;
                        if( !bJScriptObject)
                        {
                            if( bConvRet= variantToAny2( &pVarParamsRef[outIndex], outAny,
                                Type(pMethod->pParams[i].pTypeRef), sal_False))
                                OutParam[outIndex++]= outAny;

                        }
                        else //JScriptObject
                        {
                            if( pVarParams[i].vt= VT_DISPATCH)
                            {
                                CComDispatchDriver pDisp( pVarParams[i].pdispVal);
                                if( pDisp)
                                {
                                    CComVariant varOut;
                                    if( SUCCEEDED( pDisp.GetPropertyByName( L"0", &varOut)))
                                    {
                                        if( bConvRet= variantToAny2( &varOut, outAny,
                                            Type(pMethod->pParams[parameterCount - 1 - i].pTypeRef), sal_False))
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
            // return value, no type information available
            if ( bConvRet)
                if( pMethod )
                    bConvRet= variantToAny2(&varResult, ret, Type( pMethod->pReturnTypeRef), sal_False);
                else
                    bConvRet= variantToAny(&varResult, ret, sal_False);
        }

        if( pVarParams)
            delete[] pVarParams;
        // The destructor of CComVariant calls VariantClear which takes null pointers into account
        // and does not try to destroy them.
        if( pVarParamsRef)
            delete[] pVarParamsRef;

        if( !bConvRet) // conversion of return or out parameter failed
            throw CannotConvertException( L"Call to COM object failed. Conversion of return or out value failed",
                Reference<XInterface>( static_cast<XWeak*>(this), UNO_QUERY ), TypeClass_UNKNOWN,
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
                throw CannotConvertException(L"call to OLE object failed", static_cast<XInterface*>(
                    static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::OUT_OF_RANGE, uArgErr);
                break;
            case DISP_E_PARAMNOTFOUND:
                throw IllegalArgumentException(L"call to OLE object failed", static_cast<XInterface*>(
                    static_cast<XWeak*>(this)), uArgErr);
                break;
            case DISP_E_TYPEMISMATCH:
                throw CannotConvertException(L"call to OLE object failed",static_cast<XInterface*>(
                    static_cast<XWeak*>(this)) , TypeClass_UNKNOWN, FailReason::UNKNOWN, uArgErr);
                break;
            case DISP_E_UNKNOWNINTERFACE:
                throw RuntimeException() ;
                break;
            case DISP_E_UNKNOWNLCID:
                throw RuntimeException() ;
                break;
            case DISP_E_PARAMNOTOPTIONAL:
                throw CannotConvertException(L"call to OLE object failed", static_cast<XInterface*>(
                    static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::NO_DEFAULT_AVAILABLE, uArgErr);
                break;
            default:
                throw RuntimeException();
                break;
        }
    }
    if( !bConvOk) // conversion of parameters failed
    {
        if( pVarParams)
            delete [] pVarParams;
        if( pVarParamsRef)
            delete [] pVarParamsRef;
        throw CannotConvertException( L"Call to COM object failed. Conversion of in or in/out parameters failed",
            Reference<XInterface>( static_cast<XWeak*>(this), UNO_QUERY ), TypeClass_UNKNOWN,
            FailReason::UNKNOWN, 0);
    }

    return ret;
}

void IUnknownWrapper_Impl::setValueWithDispId(DISPID dispID,
                                              const Any& Value)
                                              throw (UnknownPropertyException,
                                                         CannotConvertException,
                                                       InvocationTargetException,
                                                       RuntimeException)
{
    HRESULT         hr= S_OK;
    DISPPARAMS      dispparams;
    VARIANT         varResult;
    EXCEPINFO       excepinfo;
    unsigned int    uArgErr;

    o2u_attachCurrentThread();

    VariantInit(&varResult);

    // converting UNO value to OLE variant
    DISPID dispidPut= DISPID_PROPERTYPUT;
    dispparams.rgdispidNamedArgs = &dispidPut;
    dispparams.cArgs = 1;
    dispparams.cNamedArgs = 1;

    dispparams.rgvarg = (VARIANTARG*)malloc(sizeof(VARIANTARG));

    anyToVariant(&(dispparams.rgvarg[0]), Value);

    // set OLE property
    hr = m_pDispatch->Invoke(dispID,
                                 IID_NULL,
                                 LOCALE_SYSTEM_DEFAULT,
                                 DISPATCH_PROPERTYPUT,
                                 &dispparams,
                                 &varResult,
                                 &excepinfo,
                                 &uArgErr);

    // freeing allocated OLE parameters
    VariantClear(&(dispparams.rgvarg[0]));
    free(dispparams.rgvarg);
    VariantClear(&varResult);

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
            throw CannotConvertException(L"call to OLE object failed", static_cast<XInterface*>(
                static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::OUT_OF_RANGE, uArgErr);
            break;
        case DISP_E_PARAMNOTFOUND:
            throw IllegalArgumentException(L"call to OLE object failed", static_cast<XInterface*>(
                static_cast<XWeak*>(this)), uArgErr) ;
            break;
        case DISP_E_TYPEMISMATCH:
            throw CannotConvertException(L"call to OLE object failed", static_cast<XInterface*>(
                static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::UNKNOWN, uArgErr);
            break;
        case DISP_E_UNKNOWNINTERFACE:
            throw RuntimeException();
            break;
        case DISP_E_UNKNOWNLCID:
            throw RuntimeException();
            break;
        case DISP_E_PARAMNOTOPTIONAL:
            throw CannotConvertException(L"call to OLE object failed",static_cast<XInterface*>(
                static_cast<XWeak*>(this)) , TypeClass_UNKNOWN, FailReason::NO_DEFAULT_AVAILABLE, uArgErr);
            break;
        default:
            throw  RuntimeException();
            break;
    }
}

Any  IUnknownWrapper_Impl::getValueWithDispId(DISPID dispID)
                        throw (UnknownPropertyException,RuntimeException)
{
    Any ret;
    HRESULT hr;
    DISPPARAMS      dispparams;
    VARIANT         varResult;
    EXCEPINFO       excepinfo;
    unsigned int    uArgErr;

    o2u_attachCurrentThread();

    VariantInit(&varResult);

    // converting UNO parameters to OLE variants
    dispparams.rgdispidNamedArgs = NULL;
    dispparams.cArgs = 0;
    dispparams.cNamedArgs = 0;
    dispparams.rgvarg = NULL;

    // invoking OLE method
    hr = m_pDispatch->Invoke(dispID,
                                 IID_NULL,
                                 LOCALE_SYSTEM_DEFAULT,
                                 DISPATCH_PROPERTYGET,
                                 &dispparams,
                                 &varResult,
                                 &excepinfo,
                                 &uArgErr);

    // converting return value and out parameter back to UNO
    if (hr == S_OK)
    {
        variantToAny(&varResult, ret);
    }

    // freeing allocated OLE parameters
    VariantClear(&varResult);

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
            throw RuntimeException();
            break;
        case DISP_E_MEMBERNOTFOUND:
            throw UnknownPropertyException();
            break;
        case DISP_E_NONAMEDARGS:
            throw RuntimeException();
            break;
        case DISP_E_OVERFLOW:
            throw RuntimeException();
            break;
        case DISP_E_PARAMNOTFOUND:
            throw RuntimeException();
            break;
        case DISP_E_TYPEMISMATCH:
            throw RuntimeException();
            break;
        case DISP_E_UNKNOWNINTERFACE:
            throw RuntimeException();
            break;
        case DISP_E_UNKNOWNLCID:
            throw RuntimeException();
            break;
        case DISP_E_PARAMNOTOPTIONAL:
            throw RuntimeException();
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
    // 2.parameter is a Sequence<Type> ( not mandatory)
    HRESULT result;
    o2u_attachCurrentThread();

    if( aArguments.getLength() == 1)
    {
        m_pUnknown= *(IUnknown**) aArguments[0].getValue();
        result = m_pUnknown->QueryInterface(IID_IDispatch, (void**) &m_pDispatch);
        m_pUnknown->AddRef();
    }
    else if( aArguments.getLength() == 2)
    {
        m_pUnknown= *(IUnknown**) aArguments[0].getValue();
        result = m_pUnknown->QueryInterface(IID_IDispatch, (void**) &m_pDispatch);
        m_pUnknown->AddRef();

        aArguments[1] >>= m_seqTypes;
    }

}

// UnoConversionUtilities --------------------------------------------------------------------------------
Reference< XInterface > IUnknownWrapper_Impl::createUnoWrapperInstance()
{
    if( m_nUnoWrapperClass == INTERFACE_OLE_WRAPPER_IMPL)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                                m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else if( m_nUnoWrapperClass == UNO_OBJECT_WRAPPER_REMOTE_OPT)
    {
        Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
        return Reference<XInterface>( xWeak, UNO_QUERY);
    }
    else
        return Reference<XInterface>();
}
Reference<XInterface> IUnknownWrapper_Impl::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}



void IUnknownWrapper_Impl::getMethodInfo( TypeDescription& methodInfo)
{
    TypeDescription desc= getInterfaceMemberDescOfCurrentCall();
    if( desc.is())
    {
        typelib_TypeDescription* pMember= desc.get();
        if( pMember->eTypeClass == TypeClass_INTERFACE_METHOD )
            methodInfo= pMember;
    }
}

TypeDescription IUnknownWrapper_Impl::getInterfaceMemberDescOfCurrentCall( )
{
    TypeDescription ret;
    OUString usCurrentCall;
    if( m_usCurrentInvoke.getLength())
    {
        usCurrentCall= m_usCurrentInvoke;
    }
    else
    {
        usCurrentCall= m_usCurrentGet;
    }
    OSL_ENSURE( usCurrentCall.getLength() != 0, "");

    for( sal_Int32 i=0; i < m_seqTypes.getLength(); i++)
    {
        TypeDescription _curDesc( m_seqTypes[i]);
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
                if( OUString( pInterfaceMember->pMemberName) == usCurrentCall)
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
        CComDispatchDriver disp( m_pDispatch);
        if( disp)
        {
            CComVariant result;
            if( SUCCEEDED(  disp.GetPropertyByName( JSCRIPT_ID_PROPERTY, &result)))
            {
                if(result.vt == VT_BSTR)
                {
                    CComBSTR name( result.bstrVal);
                    name.ToLower();
                    if( name == JSCRIPT_ID)
                        m_eJScript= IsJScript;
                }
            }
        }
        if( m_eJScript == JScriptUndefined)
            m_eJScript= NoJScript;
    }

    return m_eJScript == NoJScript ? sal_False : sal_True;
}

// check for all changed parameter and create UNO out parameters
void IUnknownWrapper_Impl::processOutParameters(VARIANT* pOrgParams,
                                 VARIANT* pResultParams,
                                 sal_uInt32 n,
                                 Sequence< sal_Int16 >& OutParamIndex,
                                 Sequence< Any >& OutParam)
{
    for (sal_uInt32 i = 0; i < n; i++)
    {
        VARIANT* pOrgElement = &(pOrgParams[n - (i + 1)]);
         VARIANT* pResultElement = &(pResultParams[n - (i + 1)]);
        Any aOutParam;

        // check for changes in element
        if (memcmp(pOrgElement, pResultElement, sizeof(VARIANT)) != 0)
        {
            // out parameters are only valid if the new type matches the original type
            if (V_VT(pOrgElement) == V_VT(pResultElement))
            {
                variantToAny(pResultElement, aOutParam);
            }
        }

//      if (aOutParam.getReflection() != Void_getReflection())
        if( aOutParam.getValueTypeClass() != TypeClass_VOID)
        {
            sal_uInt32 outParamCount = OutParam.getLength() + 1;

            OutParam.realloc(outParamCount);
            OutParamIndex.realloc(outParamCount);

            OutParam.getArray()[outParamCount - 1] = aOutParam;
            OutParamIndex.getArray()[outParamCount - 1] = i;
        }
    }
}


// It would be desirable if every COM component could handle parameters of VT_BYREF be it
// in or out parameter. But that is not the case. When calling an ATL component that
// has a dual interface then note this:
// - in parameter can be VT_BYREF
// - in parameter of VARIANT: one can pass the exact type
// - out parameter: one must pass the exact type
// - out parameter of VARIANT: the referenced VARIANT must be VT_EMPTY
// - inout - the same as out
Any  IUnknownWrapper_Impl::invokeWithDispIdComTlb(DISPID dispID,
                          const Sequence< Any >& Params,
                          Sequence< sal_Int16 >& OutParamIndex,
                          Sequence< Any >& OutParam)
              throw ( IllegalArgumentException, CannotConvertException,
                      InvocationTargetException, RuntimeException)
{
    Any ret;
    HRESULT result;

    DISPPARAMS      dispparams;
    CComVariant     varResult;
    EXCEPINFO       excepinfo;
    unsigned int    uArgErr;
    sal_uInt32          i;

    o2u_attachCurrentThread();

    // converting UNO parameters to OLE variants
    dispparams.rgdispidNamedArgs = NULL;
    dispparams.cArgs = Params.getLength();
    dispparams.cNamedArgs = 0;

    CComVariant* arArgs = NULL;
    CComVariant* arRefArgs= NULL; // referenced arguments

    if (dispparams.cArgs == 0)
    {
        dispparams.rgvarg = NULL;
    }
    else
    {
        arArgs = new CComVariant[dispparams.cArgs];
        arRefArgs= new CComVariant[dispparams.cArgs];

        // converting orginal parameters
        // if the object uses ATL with a dual interface, then VT_VARIANT| VT_BYREF does not suffice
//      prepareOutParams( orgArgs, dispparams.cArgs);
        getParameterInfo();

        for (i = 0; i < dispparams.cArgs; i++)
        {
            sal_Int32 revIndex= dispparams.cArgs - i -1;
            arRefArgs[revIndex].byref=0;
            // out param
            if (m_seqCurrentParamTypes[i] & PARAMFLAG_FOUT &&
                ! (m_seqCurrentParamTypes[i] & PARAMFLAG_FIN)  )
            {
                VARTYPE type= m_seqCurrentVartypes[i] ^ VT_BYREF;
                if( type == VT_VARIANT )
                {
                    arArgs[revIndex].vt= VT_VARIANT | VT_BYREF;
                    arArgs[revIndex].byref= &arRefArgs[revIndex];
                }
                else
                {
                    arRefArgs[revIndex].vt= type;
                    arArgs[revIndex].vt= m_seqCurrentVartypes[i];
                    arArgs[revIndex].byref= &arRefArgs[revIndex].byref;
                }
            }
            // in/out param
            else if( m_seqCurrentParamTypes[i] & PARAMFLAG_FOUT&&
                     m_seqCurrentParamTypes[i] & PARAMFLAG_FIN)
            {
                VARTYPE type= m_seqCurrentVartypes[i] ^ VT_BYREF;
                CComVariant var;
                anyToVariant( &arRefArgs[revIndex],
                         Params.getConstArray()[i]);

                if( type == VT_VARIANT )
                {
                    arArgs[revIndex].vt= VT_VARIANT | VT_BYREF;
                    arArgs[revIndex].byref= &arRefArgs[revIndex];
                }
                else
                {
                    arArgs[revIndex].vt= arRefArgs[revIndex].vt | VT_BYREF;
                    arArgs[revIndex].byref= &arRefArgs[revIndex].byref;
                }
            }
            // in/param
            else if(  m_seqCurrentParamTypes[i] & PARAMFLAG_FIN &&
                      ! (m_seqCurrentParamTypes[i] & PARAMFLAG_FOUT))
            {
                if( m_seqCurrentVartypes[i] & VT_BYREF)
                    anyToVariant( &arRefArgs[revIndex],
                        Params.getConstArray()[i]);
                else
                    anyToVariant( &arArgs[revIndex],
                        Params.getConstArray()[i]);
            }

        }
    }
    dispparams.rgvarg= arArgs;
    // invoking OLE method
    result = m_pDispatch->Invoke(dispID,
                                 IID_NULL,
                                 LOCALE_SYSTEM_DEFAULT,
                                 DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                                 &dispparams,
                                 &varResult,
                                 &excepinfo,
                                 &uArgErr);

    // converting return value and out parameter back to UNO
    if (result == S_OK)
    {
        if ( m_seqCurrentParamTypes.getLength())
        {
            // allocate space for the out param Sequence and indices Sequence
            sal_Int32 outParamsCount= 0; // includes in/out parameter
            for( sal_Int32 iparams=0; iparams < m_seqCurrentParamTypes.getLength(); iparams++)
            {
                if( m_seqCurrentParamTypes[iparams] & PARAMFLAG_FOUT)
                    outParamsCount++;
            }

            OutParamIndex.realloc( outParamsCount);
            OutParam.realloc( outParamsCount);

            sal_Int32 outParamIndex=0;
            for( sal_Int32 i=0; i < m_seqCurrentParamTypes.getLength(); i ++)
            {
                if( m_seqCurrentParamTypes[i] & PARAMFLAG_FOUT)
                {
                    OutParamIndex[outParamIndex]= i;
                    variantToAny( &arRefArgs[dispparams.cArgs - i -1], OutParam[outParamIndex] );
                    outParamIndex++;
                }
            }
        }
        // Return value
        variantToAny(&varResult, ret);
    }

    // lookup error code
    switch (result)
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
            throw CannotConvertException(L"call to OLE object failed", static_cast<XInterface*>(
                static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::OUT_OF_RANGE, uArgErr);
            break;
        case DISP_E_PARAMNOTFOUND:
            throw IllegalArgumentException(L"call to OLE object failed", static_cast<XInterface*>(
                static_cast<XWeak*>(this)), uArgErr);
            break;
        case DISP_E_TYPEMISMATCH:
            throw CannotConvertException(L"call to OLE object failed",static_cast<XInterface*>(
                static_cast<XWeak*>(this)) , TypeClass_UNKNOWN, FailReason::UNKNOWN, uArgErr);
            break;
        case DISP_E_UNKNOWNINTERFACE:
            throw RuntimeException() ;
            break;
        case DISP_E_UNKNOWNLCID:
            throw RuntimeException() ;
            break;
        case DISP_E_PARAMNOTOPTIONAL:
            throw CannotConvertException(L"call to OLE object failed", static_cast<XInterface*>(
                static_cast<XWeak*>(this)), TypeClass_UNKNOWN, FailReason::NO_DEFAULT_AVAILABLE, uArgErr);
            break;
        default:
            throw RuntimeException();
            break;
    }

    return ret;
}
// parameter is an array that contains the referenced parameters
// params is an array of VARIANTs that contains values which are referenced by
// the VARIANTS in DISPPARAMS::rgvarg
//sal_Bool IUnknownWrapper_Impl::prepareOutParams( VARIANT* params, sal_uInt32 count )
//{
//  sal_Bool ret= sal_True;
//  ITypeInfo* pType= getTypeInfo();
//  if(! pType )
//      return sal_False;
//  // build the map of function names and their tlb index
//  if( sal_False == isComTlbIndex())
//      buildComTlbIndex();
//
//  typedef TLBFuncIndexMap::const_iterator cit;
//  cit itIndex= m_mapComFunc.find( m_usCurrentInvoke );
//  if( itIndex != m_mapComFunc.end())
//  {
//      FUNCDESC* funcDesc= NULL;
//      if( SUCCEEDED( pType->GetFuncDesc( itIndex->second, &funcDesc)))
//      {
//          m_seqCurrentParamTypes.realloc( funcDesc->cParams);
//          // Examine each param
//          for( sal_Int16 iparams= 0; iparams < funcDesc->cParams; iparams++)
//          {
//              sal_Int32 flags= funcDesc->lprgelemdescParam->paramdesc.wParamFlags;
//
//              // out parameter
//              if(  flags & PARAMFLAG_FOUT &&
//                  ! (flags & PARAMFLAG_FIN))
//              {
//                  m_seqCurrentParamTypes[ iparams]= (sal_Int32) OUT_PARAM;
//                  VariantClear( &params[iparams] );
//                  params[ iparams].byref=0;
//                  // Get the type
//                  VARTYPE paramType;
//                  if( getElementTypeDesc( & funcDesc->lprgelemdescParam[iparams].tdesc, paramType))
//                  {
//                      // dispparams: VT_VARIANT |VT_BYREF , referenced Value: VT_EMPTY
//                      if( !( paramType & VT_ARRAY) &&
//                          (paramType & VT_TYPEMASK) == VT_VARIANT)
//                          params[iparams].vt= VT_EMPTY;
//                      else
//                          params[iparams].vt= (paramType ^ VT_BYREF);
//                  }
//                  else
//                      ret= sal_False;
//              }
//              // in/out parameter
//              else if(flags &  PARAMFLAG_FIN && flags & PARAMFLAG_FOUT )
//                  m_seqCurrentParamTypes[ iparams]= (sal_Int32) INOUT_PARAM;
//              // in parameter
//              else if(flags &  PARAMFLAG_FIN && ! (flags & PARAMFLAG_FOUT ))
//                  m_seqCurrentParamTypes[ iparams]= (sal_Int32) IN_PARAM;
//          }
//          pType->ReleaseFuncDesc( funcDesc);
//      }
//      else
//          ret= sal_False;
//  }
//  else
//      ret= sal_False;
//
//  return ret;
//}

sal_Bool IUnknownWrapper_Impl::getParameterInfo()
{
    sal_Bool ret= sal_True;
    ITypeInfo* pType= getTypeInfo();
    if(! pType )
        return sal_False;
    // build the map of function names and their tlb index
    if( sal_False == isComTlbIndex())
        buildComTlbIndex();

    typedef TLBFuncIndexMap::const_iterator cit;
    cit itIndex= m_mapComFunc.find( m_usCurrentInvoke );
    if( itIndex != m_mapComFunc.end())
    {
        FUNCDESC* funcDesc= NULL;
        if( SUCCEEDED( pType->GetFuncDesc( itIndex->second, &funcDesc)))
        {
            m_seqCurrentParamTypes.realloc( funcDesc->cParams);
            m_seqCurrentVartypes.realloc( funcDesc->cParams);
            // Examine each param
            for( sal_Int16 iparams= 0; iparams < funcDesc->cParams; iparams++)
            {
                sal_Int32 flags= funcDesc->lprgelemdescParam[iparams].paramdesc.wParamFlags;
                m_seqCurrentParamTypes[iparams]= flags & ( PARAMFLAG_FIN | PARAMFLAG_FOUT);

                if( ! getElementTypeDesc( & funcDesc->lprgelemdescParam[iparams].tdesc,
                                    m_seqCurrentVartypes[iparams]))
                                    ret= sal_False;

                if( ! ret) break;
            }
            pType->ReleaseFuncDesc( funcDesc);
        }
        else
            ret= sal_False;
    }
    else
        ret= sal_False;

    return ret;
}

sal_Bool IUnknownWrapper_Impl::getElementTypeDesc( TYPEDESC *desc, VARTYPE& varType )
{
    sal_Bool ret= sal_True;
    VARTYPE _type;

    if( desc->vt == VT_PTR)
    {
        if( getElementTypeDesc( desc->lptdesc, _type))
            varType= _type | VT_BYREF;
        else
            ret= sal_False;
    }
    else if( desc->vt == VT_SAFEARRAY )
    {
        if( getElementTypeDesc( desc->lptdesc, _type))
            varType= _type | VT_ARRAY;
        else
            sal_False;
    }
    else
    {
        varType= desc->vt;
    }
    return ret;
}

sal_Bool IUnknownWrapper_Impl::buildComTlbIndex()
{
    sal_Bool ret= sal_True;

    ITypeInfo* pType= getTypeInfo();
    if( pType)
    {
        TYPEATTR* typeAttr= NULL;
        if( SUCCEEDED( pType->GetTypeAttr( &typeAttr)))
        {
            for( long i= 0; i < typeAttr->cFuncs; i++)
            {
                FUNCDESC* funcDesc= NULL;
                if( SUCCEEDED( pType->GetFuncDesc( i, &funcDesc)))
                {
                    BSTR memberName= NULL;
                    unsigned int pcNames=0;
                    if( SUCCEEDED(pType->GetNames( funcDesc->memid, & memberName, 1, &pcNames)))
                    {
                        m_mapComFunc.insert( TLBFuncIndexMap::value_type( OUString( memberName), i));
                        SysFreeString( memberName);
                    }
                    else
                        ret= sal_False;
                    pType->ReleaseFuncDesc( funcDesc);
                }
                else
                    ret=sal_False;
            }
            pType->ReleaseTypeAttr( typeAttr);
        }
        else
            ret= sal_False;
    }
    else
        ret= sal_False;
    return ret;
}

inline sal_Bool IUnknownWrapper_Impl::isComTlbIndex()
{
    if( m_mapComFunc.size())
        return sal_True;
    return sal_False;
}

ITypeInfo* IUnknownWrapper_Impl::getTypeInfo()
{
    HRESULT hr= S_OK;
    if( !m_pDispatch)
        return NULL;

    if( !m_spTypeInfo )
    {
        LCID localId= GetUserDefaultLCID();
        CComPtr< ITypeInfo > spType;
        if( SUCCEEDED( m_pDispatch->GetTypeInfo( 0, localId, &spType.p)))
            m_spTypeInfo= spType;
    }
    return m_spTypeInfo;
}
} // end namespace

