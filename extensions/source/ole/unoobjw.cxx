/*************************************************************************
 *
 *  $RCSfile: unoobjw.cxx,v $
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

#include <tools/presys.h>
#include <olectl.h>
#include <vector>
#include <list>
#include <hash_map>
#include "stdafx.h"
#include "comifaces.hxx"
#include <tools/postsys.h>


#include <vos/diagnose.hxx>
#include <vos/mutex.hxx>
#include <vos/refernce.hxx>
#include <tools/debug.hxx>

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _COM_SUN_STAR_BEANS_METHODCONCEPT_HPP_
#include <com/sun/star/beans/MethodConcept.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCONCEPT_HPP_
#include <com/sun/star/beans/PropertyConcept.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FAILREASON_HPP_
#include <com/sun/star/script/FailReason.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_PARAMINFO_HPP_
#include <com/sun/star/reflection/ParamInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLARRAY_H_
#include <com/sun/star/reflection/XIdlArray.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_FIELDACCESSMODE_HPP_
#include <com/sun/star/reflection/FieldAccessMode.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLCLASS_HPP_
#include <com/sun/star/reflection/XIdlClass.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLFIELD_HPP_
#include <com/sun/star/reflection/XIdlField.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_PARAMMODE_HPP_
#include <com/sun/star/reflection/ParamMode.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_METHODMODE_HPP_
#include <com/sun/star/reflection/MethodMode.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLMEMBER_HPP_
#include <com/sun/star/reflection/XIdlMember.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLREFLECTION_HPP_
#include <com/sun/star/reflection/XIdlReflection.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLMETHOD_HPP_
#include <com/sun/star/reflection/XIdlMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XEXACTNAME_HPP_
#include <com/sun/star/beans/XExactName.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHER_HPP_
#include <com/sun/star/script/XEventAttacher.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENT_HPP_
#include <com/sun/star/script/ScriptEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTLISTENER_HPP_
#include <com/sun/star/script/XScriptListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XALLLISTENERADAPTERSERVICE_HPP_
#include <com/sun/star/script/XAllListenerAdapterService.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XALLLISTENER_HPP_
#include <com/sun/star/script/XAllListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_ALLEVENTOBJECT_HPP_
#include <com/sun/star/script/AllEventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif
#include <com/sun/star/beans/XMaterialHolder.hpp>


#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <uno/typedescription.h>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _COM_SUN_STAR_UNO_GENFUNC_H_
#include <com/sun/star/uno/genfunc.h>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include "comifaces.hxx"
#include "jscriptclasses.hxx"
#include "oleobjw.hxx"
#include "unoobjw.hxx"
#include "servprov.hxx"

using namespace vos;
using namespace std;
using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::script;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge::ModelDependent;

#define INVOCATION_SERVICE L"com.sun.star.script.Invocation"
#define JSCRIPT_VALUE_FUNC L"_GetValueObject"
#define DISPID_JSCRIPT_VALUE_FUNC -1000l



#if _MSC_VER < 1200
extern "C" const GUID IID_IDispatchEx;
#endif

namespace ole_adapter
{
static sal_Bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource);
static sal_Bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource);
//static const VARTYPE getVarType( const Any& val);
//static sal_Bool convertValueObject( const VARIANTARG *var, Any& any, sal_Bool& bHandled);
static HRESULT mapCannotConvertException( CannotConvertException e, unsigned int * puArgErr);



/*****************************************************************************

    static objects

*****************************************************************************/

OMutex globalWrapperMutex;
//static WrapperMap globalWrapperMap;

/*****************************************************************************

    class implementation: InterfaceOleWrapper_Impl

*****************************************************************************/
InterfaceOleWrapper_Impl::InterfaceOleWrapper_Impl( Reference<XMultiServiceFactory>& xFactory,
                                                    sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
        m_defaultValueType( 0),
        UnoConversionUtilities<InterfaceOleWrapper_Impl>( xFactory, unoWrapperClass, comWrapperClass)
{
}

InterfaceOleWrapper_Impl::~InterfaceOleWrapper_Impl()
{
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
//    else if (IsEqualIID(riid, IID_IDispatchEx))
//  {
//      AddRef();
//      *ppv = (IDispatchEx*) this;
//      return NOERROR;
//  }
//    else if (IsEqualIID(riid, IID_IConnectionPointContainer))
//  {
//      AddRef();
//      *ppv = (IConnectionPointContainer*) this;
//      return NOERROR;
//  }
//
    return ret;
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper_Impl::AddRef()
{
    return m_refCount.acquire();
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper_Impl::Release()
{
    ULONG n;

    {
        OGuard guard(globalWrapperMutex);

        n = m_refCount.release();
    }

    if (n == 0)
    {
        delete this;
    }

    return n;
}

// IUnoObjectWrapper --------------------------------------------------------
STDMETHODIMP InterfaceOleWrapper_Impl::getWrapperXInterface( Reference<XInterface>* pXInt)
{
    *pXInt= Reference<XInterface>( static_cast<XWeak*>( this), UNO_QUERY);
    return pXInt->is() ? S_OK : E_FAIL;
}
STDMETHODIMP InterfaceOleWrapper_Impl::getOriginalUnoObject( Reference<XInterface>* pXInt)
{
    *pXInt= m_xOrigin;
    return S_OK;
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

STDMETHODIMP InterfaceOleWrapper_Impl::GetTypeInfoCount( unsigned int * pctinfo )
{
    return E_NOTIMPL ;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo ** pptinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetIDsOfNames(REFIID riid,
                                                     OLECHAR ** rgszNames,
                                                     unsigned int cNames,
                                                     LCID lcid,
                                                     DISPID * rgdispid )
{
    if( ! rgdispid)
        return E_POINTER;
    HRESULT ret = E_UNEXPECTED;
    // ----------------------------------------
    if( ! wcscmp( *rgszNames, JSCRIPT_VALUE_FUNC))
    {
        *rgdispid= DISPID_JSCRIPT_VALUE_FUNC;
        return S_OK;
    }
    // ----------------------------------------
    if (m_xInvocation.is() && (cNames > 0))
    {
        OUString name(rgszNames[0]);
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
            ret = ResultFromScode(DISP_E_UNKNOWNNAME);
        }
        else
        {
            *rgdispid = (*iter).second;
            ret = ResultFromScode(S_OK);
        }
    }

    return ret;
}

// "convertVARIANTARGS" converts VARIANTS to their respecting Any counterparts
// As parameter it uses some of the parameters passed to IDispatch::Invoke. These
// ar "id", "wFlags" and "pdispparams". The function handles special JavaScript
// cases where a VARIANT of type VT_DISPATCH is ambiguous and can represent
// an object, array ( JavaScript Array object), out ( Array) and in/out ( Array)
// parameter. Because all those VT_DISPATCH objects need a different conversion
// we have to find out what the object is supposed to be. The function does this
// by either using type information or by help of specialized JavaScript object.

// A. Type Information
// -----------------------------------------------------------------------------
// With the help of type information the kind of parameter can be exactly determined
// and an appropriate conversion can be choosen. A problem arises if a method expects
// an Any. Then the type info does not tell what the type of the value, that is kept
// by the any, should be. In this situation the decision wheter the param is a
// sequence or an object is made upon the fact if the object has a property "0"
// ( see function "isJScriptArray"). Since this is unsafe it is recommended to use
// the JScript value objects within a JScript script on such an occasion.

// B. JavaScript Value Object ( class JScriptValue )
// -----------------------------------------------------------------------------
// A JScriptValue object is a COM object in that it implements IDispatch and the
// IJScriptValue object interface. Such objects are provided by all UNO wrapper
// objects used within a JScript script. To obtain an instance one has to call
// "_GetValueObject()" on an UNO wrapper object (class InterfaceOleWrapper_Impl).
// A value object is appropriately initialized within the script and passed as
// parameter to an UNO object method or property. The convertVARIANTARGS function
// can easily find out that a param is such an object by queriing for the
// IJScriptValue interface. By this interface one the type and kind ( out, in/out)
// can be determined and the right conversion can be applied.
// Using ValueObjects we spare us the effort of aquiring and examining type information
// in order to figure out what the an IDispatch parameter is meant for.

// Normal JScript object parameter can be mixed with JScriptValue object. If an
// VARIANT contains an VT_DISPATCH that is no JScriptValue than the type information
// is used to find out about the reqired type.

// out Parameter in Visual Basic have the type VT_BYREF | VT_VARIANT and if the bridge
// is used from C++ than the type is usually VT_BYREF |VT_xxx. Those parameter are converted
// and we don't go to the trouble of aquiring type information.
sal_Bool InterfaceOleWrapper_Impl::convertVARIANTARGS(  DISPID id, unsigned short wFlags, DISPPARAMS* pdispparams, Sequence<Any>& rSeq)
{
    HRESULT hr;
    if( pdispparams->cArgs == 0)
        return TRUE;
    sal_Bool convOk = TRUE;

    // iterate over all parameter and check for special JScript value objects that originated in this bridge.
    // Those can be converted straight away. The index in the sequence "seqConvertedParams" matches the index in the
    // DISPPARAMS.rgvarg array. If the value is true then the corresponding VARIANT has been converted
    // and the any has been written to the Sequence that holds  the converted values.
    Sequence< sal_Bool > seqConvertedParams( pdispparams->cArgs);
    rSeq.realloc( pdispparams->cArgs);
    Any*    pParams = rSeq.getArray();

    sal_Bool allDispHandled= sal_True;
    Any anyParam;
    sal_Bool bHandled= sal_False;

    for( int i= 0; i < pdispparams->cArgs; i++)
    {
        if( sal_False == convertValueObject( &pdispparams->rgvarg[i], anyParam, bHandled) )
        {
            convOk= sal_False;
            break;
        }
        else if( bHandled)
        {// a param is a JScriptValue and could be converted
            pParams[pdispparams->cArgs - i - 1]= anyParam;
            seqConvertedParams[i]= sal_True;
            continue;
        }// also take care of VBasic ( VT_BYREF)
        else if( (pdispparams->rgvarg[i].vt & VT_DISPATCH )== VT_DISPATCH )
            allDispHandled= sal_False;
    }

    if( ! convOk)
        return sal_False;
    // ----------------------

    // If there is a VT_DISPATCH  and it is no JScriptValue object then
    // we need to know if it represents a Sequence, out or in/out parameter
    // or an actual object. Therefore we obtain type information
    try{
        Sequence<ParamMode> seqParamMode;
        sal_Bool bTypesAvailable= sal_False;
        Sequence< Type > seqParamTypes;
        sal_Bool bIsMethod= sal_False;

        //only aquire type information if at least one of the parameter is IDispatch
        // and has not been convert yet because it is no JScriptValue object
        if( ! allDispHandled)
        {
            if( !m_xInvocation.is() )return FALSE;
            Reference<XIntrospectionAccess> xIntroAccess= m_xInvocation->getIntrospection();
            if( !xIntroAccess.is()) return FALSE;

            // We need the name of the property or method to get its type description
            // The name can be identified through the param "id"
            // that is kept als value in the map m_nameToDispIdMap.
            // Proplem: the Windows JScript engine sometimes changes small letters to capital
            // letters as happens in xidlclass_obj.createObject( var) // in JScript.
            // IDispatch::GetIdsOfNames is then called with "CreateObject" !!!
            // m_nameToDispIdMap can contain several names for one DISPID but only one is
            // the exact one. If there's no m_xExactName and therfore no exact name then
            // there's only one entry in the map.
            typedef NameToIdMap::const_iterator cit;
            OUString sMemberName;
            if( m_xExactName.is())
            {
                for(cit ci1= m_nameToDispIdMap.begin(); ci1 != m_nameToDispIdMap.end(); ci1++)
                {
                    if( (*ci1).second == id) // iterator is a pair< OUString, DISPID>
                    {
                        sMemberName= (*ci1).first;
                        sMemberName= m_xExactName->getExactName( sMemberName);
                        break;
                    }
                }
            }
            else // the first match is certainly the only
            {
                for(cit ci1= m_nameToDispIdMap.begin(); ci1 != m_nameToDispIdMap.end(); ci1++)
                {
                    if( (*ci1).second == id) // iterator is a pair< OUString, DISPID>
                    {
                        sMemberName= (*ci1).first;
                        break;
                    }
                }
            }

            if( sMemberName == L"")
                return FALSE;
            // Get the type description of the uno method or property
            // Problem: In some cases DISPATCH_METHOD AND!!! DISPATCH_PROPERTYPUT or
            // DISPATCH_PROPERTYGET can be set. That might
            // be due to VBScript or JScript that can't differ what's what or it doesnt matter
            // For that reaseon we can't use the wFlags param in Invoke
            // The automation wrapper (this class (InterfaceOleWrapper_Impl) doesn't provide
            // any type information currently.
            try{
                // throws // NoSuchMethodException
                Reference< XIdlMethod > spTypeMethod= xIntroAccess->getMethod( sMemberName, MethodConcept::ALL);
                // ----------------
                Sequence< ParamInfo > seqInfos= spTypeMethod->getParameterInfos();
                Sequence< Reference< XIdlClass > > seqParamClasses= spTypeMethod->getParameterTypes();
                int count= min( seqParamClasses.getLength(), seqConvertedParams.getLength());

                seqParamTypes.realloc( count);
                seqParamMode.realloc( count);
                // --------------------
                for( int i= 0; i < count ; i++)
                {
                    seqParamTypes[i]= Type( seqParamClasses[count - 1 - i]->getTypeClass(),
                        seqParamClasses[count - 1 - i]->getName());

                    seqParamMode[i]= seqInfos[ count - 1 -i].aMode;
                }
                bTypesAvailable= sal_True;
                bIsMethod= sal_True;
            }catch(...)  {}

            if( ! bTypesAvailable)
            {
                try{
                    // throws NoSuchElementException
                    Property aProperty= xIntroAccess->getProperty( sMemberName, PropertyConcept::ATTRIBUTES);
                    seqParamTypes.realloc(1);
                    seqParamTypes[0]= aProperty.Type;
                    bTypesAvailable= sal_True;
                }catch(...) {}
            }
            if( ! bTypesAvailable)
                return FALSE;
        }

        // used within the for loop. If the param is a IDispatch and the reqired parameter
        // is an in/out parameter, then this uno wrapper is likely used in JScript. Then we
        // extract Array[0] and put the value into varParam. At the end of the loop varParam
        // is converted if it contains a value otherwise the VARIANT from
        // DISPPARAMS is converted.
        CComVariant varParam;

        // In this loop all params are converted except for those which are already converted
        // by convertValueObject ( see above ). If a parameter is converted or is can be infered
        // from the Sequence seqConvertedParams. Its values relate to the VARIANT params in DISPPARAMS
        // at the same index.
        for (int i = 0; convOk && (i < pdispparams->cArgs); i++)
        {
            if( seqConvertedParams[i] == sal_True)
                continue;
            sal_Bool bDoConversion= TRUE;
            varParam.Clear();
            // Check for dispatch
            // If the param is one and is not refereced ( VT_BYREF) then it could be an object
            // from JScript like an Array used for Squences and out-params
            if( pdispparams->rgvarg[i].vt == VT_DISPATCH )
            {
                if( bIsMethod && seqParamMode[i] == ParamMode_INOUT)
                {
                    // An INOUT-param in JSCript must be VT_DISPATCH since JScript Array objects are used
                    // for OUT-params. Index ( property) "0" contains the actual IN-param.
                    // It can also be an JScript Array or an other object (Interface).
                    // First get the IN-param at index "0"
                    IDispatch* pdisp= pdispparams->rgvarg[i].pdispVal;

                    OLECHAR* sindex= L"0";
                    DISPID id;
                    DISPPARAMS noParams= {0,0,0,0};
                    if(SUCCEEDED( hr= pdisp->GetIDsOfNames( IID_NULL, &sindex, 1, LOCALE_USER_DEFAULT, &id)))
                        hr= pdisp->Invoke( id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                            &noParams, &varParam, NULL, NULL);
                    if( FAILED( hr))
                    {
                        varParam.Clear();
                        convOk= FALSE; break;
                    }
                }
                else if( bIsMethod && seqParamMode[i] == ParamMode_OUT)
                {
                    // No conversion necessary. The VT_DISPATCH represents
                    // an Array in which the outparam is written on index 0
                    bDoConversion= FALSE;
                }
            }
            if( bDoConversion)
            {
                if( varParam.vt == VT_EMPTY) // then it was no in/out parameter
                    varParam= pdispparams->rgvarg[i];

                Any theParam;
                if( bTypesAvailable)
                    convOk= variantToAny2( & varParam,theParam,
                            seqParamTypes[i]);
                else
                    convOk = variantToAny( & varParam, theParam);

                if( convOk)
                    pParams[pdispparams->cArgs - (i + 1)]= theParam;
            }

        }// end for / iterating over all parameters


    }catch(NoSuchMethodException e)
    {
        e;
        convOk= FALSE;
    }catch(RuntimeException e)
    {
        e;
        convOk= FALSE;
    }catch( NoSuchElementException e)
    {
        e;
        convOk= FALSE;
    }
    catch(...)
    {
        convOk= FALSE;
    }
    return convOk;
}

// XBridgeSupplier2 ---------------------------------------------------
// only bridges itself ( this instance of InterfaceOleWrapper_Impl)from UNO to IDispatch
// If sourceModelType is UNO than any UNO interface implemented by InterfaceOleWrapper_Impl
// can bridged to IDispatch ( if destModelType == OLE). The IDispatch is
// implemented by this class.
Any SAL_CALL InterfaceOleWrapper_Impl::createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& ProcessId,
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
            if( xInt == Reference<XInterface>( static_cast<XWeak*>( this)), UNO_QUERY)
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

// XInterface ------------------------------------------------------
void SAL_CALL InterfaceOleWrapper_Impl::acquire(  ) throw()
{
    AddRef();
}
void SAL_CALL InterfaceOleWrapper_Impl::release(  ) throw()
{
    Release();
}


// XInitialization --------------------------------------------------
void SAL_CALL InterfaceOleWrapper_Impl::initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
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

    m_xExactName= Reference< XExactName> ( m_xOrigin, UNO_QUERY);
    if( ! m_xExactName.is())
    {
        m_xExactName= Reference<XExactName>( m_xInvocation, UNO_QUERY);
    }

}

Reference< XInterface > InterfaceOleWrapper_Impl::createUnoWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper_Impl(
                            m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

Reference<XInterface> InterfaceOleWrapper_Impl::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper_Impl(
                            m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}



// "getType" is used in convertValueObject to map the string denoting the type
// to an actual Type object.
Type getType( const BSTR type)
{
    Type retType;
    typelib_TypeDescription * pDesc= NULL;
    OUString str( type);
    typelib_typedescription_getByName( &pDesc, str.pData );
    if( pDesc)
    {
        retType= Type( pDesc->pWeakRef );
        typelib_typedescription_release( pDesc);
    }
    return retType;
}


static sal_Bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource)
{
    sal_Bool ret = sal_False;
    HRESULT hr;

    if( !(pDest->vt & VT_BYREF)  && (pDest->vt == VT_DISPATCH)  )
    {
        CComPtr <IJScriptValueObject> spValue;

        // special Handling for a JScriptValue object
        if( SUCCEEDED( pDest->pdispVal->QueryInterface( __uuidof( IJScriptValueObject),
            reinterpret_cast<void**> (&spValue))))

        {
            VARIANT_BOOL varBool= VARIANT_FALSE;
            if( SUCCEEDED( hr= spValue->IsOutParam( &varBool) )
                && varBool == VARIANT_TRUE  ||
                SUCCEEDED(hr= spValue->IsInOutParam( &varBool) )
                && varBool == VARIANT_TRUE )
            {
                if( SUCCEEDED( spValue->Set( CComVariant(), *pSource)))
                    ret= sal_True;
            }
        }
        else//  // no VT_BYREF but VT_DISPATCH -> JScript out param
        {
            // prepare DISPPARAMS for PROPERTYPUT operation
            DISPID namedArgs = DISPID_PROPERTYPUT;
            DISPPARAMS dispparamsPut;
            dispparamsPut.rgdispidNamedArgs= &namedArgs;
            dispparamsPut.cArgs = 1;
            dispparamsPut.cNamedArgs = 1;

            CComBSTR bstrNullIndex= SysAllocString( L"0");
            if(  bstrNullIndex)
            {
                CComPtr<IDispatchEx> pdispEx;
                if( SUCCEEDED( hr= pDest->pdispVal->QueryInterface( IID_IDispatchEx, (void**)&pdispEx)))
                {
                    DISPID dispid;
                    if( SUCCEEDED(hr= pdispEx->GetDispID( bstrNullIndex, fdexNameEnsure, &dispid)))
                    {
                        dispparamsPut.rgvarg= pSource;
                        if( SUCCEEDED( hr= pdispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
                                                &dispparamsPut, NULL, NULL, NULL)))
                            ret= sal_True;
                    } // if( bstrIndex)
                }
            }
        }
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
static sal_Bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource)
{
    HRESULT hr;
    sal_Bool ret = FALSE;
    // Out parameter must be VT_BYREF
    if ((V_VT(pDest) & VT_BYREF) != 0 )
    {
        VARTYPE oleTypeFlags = V_VT(pSource);

        // if caller accept VARIANT as out parameter, any value must be converted
        if (V_VT(pDest) == (VT_VARIANT | VT_BYREF))
        {
            if (oleTypeFlags == (VT_VARIANT | VT_BYREF))
            {
                if (V_VARIANTREF(pSource) != NULL)
                    VariantCopy(V_VARIANTREF(pDest), V_VARIANTREF(pSource));
                ret = sal_True;
            }
            else
            {
                VariantCopy(V_VARIANTREF(pDest), pSource);
                ret = sal_True;
            }
        }
        else
        {
            // variantarg and variant must have same type
            if ((V_VT(pDest) & oleTypeFlags) == oleTypeFlags)
            {
                if ((oleTypeFlags & VT_ARRAY) != 0)
                {
                    // copy safearray
                    // if VT_ARRAY | VT_BYREF then VariantCopy doesn't destroy the array
                    if (*V_ARRAYREF(pDest) != NULL)
                        SafeArrayDestroy(*V_ARRAYREF(pDest));

                    if (SafeArrayCopy(V_ARRAY(pSource), V_ARRAYREF(pDest)) == NOERROR)
                        ret = sal_True;
                }
                else
                {
                    // copy base type
                    switch (V_VT(pSource))
                    {
                        case VT_I2:
                            *V_I2REF(pDest) = V_I2(pSource);
                            ret = sal_True;
                            break;
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
                        default:
                            break;
                    }
                }
            }
        }
    }
    return ret;
}



STDMETHODIMP InterfaceOleWrapper_Impl::Invoke(DISPID dispidMember,
                                              REFIID riid,
                                              LCID lcid,
                                              unsigned short wFlags,
                                               DISPPARAMS * pdispparams,
                                              VARIANT * pvarResult,
                                              EXCEPINFO * pexcepinfo,
                                               unsigned int * puArgErr )
{
    HRESULT ret = S_OK;

    sal_Bool bHandled= sal_False;
    ret= InvokeGeneral( dispidMember,  wFlags, pdispparams, pvarResult,  pexcepinfo,
                   puArgErr, bHandled);
    if( bHandled)
        return ret;

    if ((dispidMember > 0) && (dispidMember <= m_MemberInfos.size()) && m_xInvocation.is())
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
                    if( convertVARIANTARGS(dispidMember, wFlags, pdispparams , params ))
                    {
                        ret= doInvoke(  pdispparams, pvarResult,
                          pexcepinfo, puArgErr, d.name, params );
                    }
                    else
                        ret = DISP_E_BADVARTYPE;
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
                    if (convertVARIANTARGS(dispidMember, wFlags, pdispparams, params ) && (params.getLength() > 0))
                    {
                        ret= doSetProperty( pdispparams, pvarResult, pexcepinfo, puArgErr, d.name, params);
                    }
                    else
                        ret = DISP_E_BADVARTYPE;
                }
            }
        }
    }
    else
        ret = DISP_E_MEMBERNOTFOUND;

    return ret;
}

HRESULT InterfaceOleWrapper_Impl::doInvoke( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString& name, Sequence<Any>& params)
{

    HRESULT ret= S_OK;
    Sequence<INT16>     outIndex;
    Sequence<Any>   outParams;
    Any                 returnValue;

    if (pdispparams->cNamedArgs > 0)
        return DISP_E_NONAMEDARGS;

    // invoke method and take care of exceptions
    try
    {
        returnValue = m_xInvocation->invoke(name,
                                            params,
                                            outIndex,
                                            outParams);

        // try to write back out parameter
        if (outIndex.getLength() > 0)
        {
             const INT16* pOutIndex = outIndex.getConstArray();
            const Any* pOutParams = outParams.getConstArray();

            for (UINT32 i = 0; i < outIndex.getLength(); i++)
            {
                VARIANT variant;

                VariantInit(&variant);

                // Currently a Sequence is converted to an SafeArray of VARIANTs.
                anyToVariant( &variant, pOutParams[i]);

                // out parameter need special handling if they are VT_DISPATCH
                // and used in JScript
                int outindex= pOutIndex[i];
                writeBackOutParameter2(&(pdispparams->rgvarg[pdispparams->cArgs - 1 - outindex]),
                                      &variant );

                VariantClear(&variant);
            }
        }

        // write back return value
        if ((pvarResult != NULL) && ! anyToVariant(pvarResult, returnValue))
        {
            ret = DISP_E_BADVARTYPE;
        }
//      else
//      {
//          ret = NOERROR;
//      }
    }
    catch(IllegalArgumentException e)
    {
        e;
        ret = ResultFromScode(DISP_E_TYPEMISMATCH);
    }
    catch(CannotConvertException e)
    {
        ret= mapCannotConvertException( e, puArgErr);
    }
    catch(InvocationTargetException e)
    {
        if (pexcepinfo != NULL)
        {
            Any org = e.TargetException;

            pexcepinfo->wCode = UNO_2_OLE_EXCEPTIONCODE;
            pexcepinfo->bstrSource = SysAllocString(L"any ONE component");
            pexcepinfo->bstrDescription = SysAllocString(
                  org.getValueType().getTypeName());
        }
        ret = ResultFromScode(DISP_E_EXCEPTION);
    }
    catch( NoSuchMethodException e)
    {
        ret= DISP_E_MEMBERNOTFOUND;
    }
    catch( ... )
    {
        ret= DISP_E_EXCEPTION;
    }
    return ret;
}

HRESULT InterfaceOleWrapper_Impl::doGetProperty( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                                EXCEPINFO * pexcepinfo, OUString& name)
{
    HRESULT ret= S_OK;

    Any value;
    try
    {
        Any returnValue = m_xInvocation->getValue( name);
        // write back return value
        if (!anyToVariant(pvarResult, returnValue))     {
            ret = DISP_E_BADVARTYPE;
        }
    }
    catch(UnknownPropertyException e)
    {
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch( ... )
    {
        ret= DISP_E_EXCEPTION;
    }
    return  ret;
}

HRESULT InterfaceOleWrapper_Impl::doSetProperty( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                        EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString& name, Sequence<Any> params)
{
    HRESULT ret= S_OK;

    try
    {
        m_xInvocation->setValue( name, params.getConstArray()[0]);
    }
    catch(UnknownPropertyException )
    {
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(CannotConvertException e)
    {
        ret= mapCannotConvertException( e, puArgErr);
    }
    catch(InvocationTargetException e)
    {
        if (pexcepinfo != NULL)
        {
            Any org = e.TargetException;

            pexcepinfo->wCode = UNO_2_OLE_EXCEPTIONCODE;
            pexcepinfo->bstrSource = SysAllocString(L"any ONE component");
            pexcepinfo->bstrDescription = SysAllocString(
                org.getValueType().getTypeName());
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
                         unsigned int * puArgErr, sal_Bool& bHandled)
{
    HRESULT ret= S_OK;
// DISPID_VALUE | The DEFAULT Value is required in JScript when the situation
// is that we put an object into an Array object ( out parameter). We have to return
// IDispatch otherwise the object cannot be accessed from the Script.
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
// ---------
    // function: _GetValueObject
    else if( dispidMember == DISPID_JSCRIPT_VALUE_FUNC)
    {
        bHandled= sal_True;
        if( !pvarResult)
            ret= E_POINTER;
        CComObject< JScriptValue>* pValue;
        if( SUCCEEDED( CComObject<JScriptValue>::CreateInstance( &pValue)))
        {
            pvarResult->vt= VT_UNKNOWN;
            pvarResult->punkVal= pValue->GetUnknown();
            pvarResult->punkVal->AddRef();
            ret= S_OK;
        }
        else
            ret= DISP_E_EXCEPTION;
    }
    return S_OK;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetDispID(BSTR bstrName, DWORD grfdex, DISPID __RPC_FAR *pid)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::InvokeEx(
    /* [in] */ DISPID id,
    /* [in] */ LCID lcid,
    /* [in] */ WORD wFlags,
    /* [in] */ DISPPARAMS __RPC_FAR *pdp,
    /* [out] */ VARIANT __RPC_FAR *pvarRes,
    /* [out] */ EXCEPINFO __RPC_FAR *pei,
    /* [unique][in] */ IServiceProvider __RPC_FAR *pspCaller)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}


STDMETHODIMP InterfaceOleWrapper_Impl::DeleteMemberByName(
    /* [in] */ BSTR bstr,
    /* [in] */ DWORD grfdex)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::DeleteMemberByDispID(DISPID id)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetMemberProperties(
    /* [in] */ DISPID id,
    /* [in] */ DWORD grfdexFetch,
    /* [out] */ DWORD __RPC_FAR *pgrfdex)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetMemberName(
    /* [in] */ DISPID id,
    /* [out] */ BSTR __RPC_FAR *pbstrName)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetNextDispID(
    /* [in] */ DWORD grfdex,
    /* [in] */ DISPID id,
    /* [out] */ DISPID __RPC_FAR *pid)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

STDMETHODIMP InterfaceOleWrapper_Impl::GetNameSpaceParent(
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk)
{
    HRESULT ret = ResultFromScode(E_NOTIMPL);

    return ret;
}

//STDMETHODIMP InterfaceOleWrapper_Impl::EnumConnectionPoints(IEnumConnectionPoints __RPC_FAR *__RPC_FAR *ppEnum)
//{
//  HRESULT ret = ResultFromScode(CONNECT_E_NOCONNECTION);
//
//  (*ppEnum) = new EnumConnectionPoints_Impl(&m_rConnectionPoint, 1);
//
//  return ret;
//}
//
//STDMETHODIMP InterfaceOleWrapper_Impl::FindConnectionPoint(
//    /* [in] */ REFIID riid,
//    /* [out] */ IConnectionPoint __RPC_FAR *__RPC_FAR *ppCP)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  if (!m_rConnectionPoint.is())
//  {
//      OGuard guard(globalWrapperMutex);
//
//      if ((!m_rConnectionPoint.is()) && m_xOrigin.is() && m_xEventAttacher.is())
//      {
//          ConnectionPoint_Impl* pConnectionPoint = new ConnectionPoint_Impl(m_xInvocation, m_xOrigin, m_xEventAttacher, this);
//
//          m_rConnectionPoint = pConnectionPoint;
//      }
//  }
//
//  *ppCP = m_rConnectionPoint.get();
//
//  (*ppCP)->AddRef();
//
//  return ret;
//}

/*****************************************************************************

    class implementation: ConnectionPoint_Impl

*****************************************************************************/


//ConnectionPoint_Impl::ConnectionPoint_Impl(const Reference<XInvocation>& xInv,
//                                         const Reference<XInterface>& xOrigin,
//                                         const Reference<XEventAttacher>& xEventAttacher,
//                                         IConnectionPointContainer* pContainer)
//  : m_xInvocation(xInv)
//  , m_xOrigin(xOrigin)
//  , m_xEventAttacher(xEventAttacher)
//  , m_rContainer(pContainer)
//  , m_nextCookie(1)
//{
//}
//
//ConnectionPoint_Impl::~ConnectionPoint_Impl()
//{
//}
//
//STDMETHODIMP ConnectionPoint_Impl::QueryInterface(REFIID riid, LPVOID FAR * ppv)
//{
//    if(IsEqualIID(riid, IID_IUnknown))
//  {
//      AddRef();
//      *ppv = (IUnknown*) (IConnectionPoint*) this;
//      return NOERROR;
//    }
//    else if (IsEqualIID(riid, IID_IConnectionPoint))
//  {
//      AddRef();
//      *ppv = (IConnectionPoint*) this;
//      return NOERROR;
//  }
//
//    *ppv = NULL;
//
//  return ResultFromScode(E_NOINTERFACE);
//}
//
//STDMETHODIMP_(ULONG) ConnectionPoint_Impl::AddRef()
//{
//  return m_refCount.acquire();
//}
//
//STDMETHODIMP_(ULONG) ConnectionPoint_Impl::Release()
//{
//
//  ULONG n = m_refCount.release();
//
//  if (n == 0)
//  {
//      delete this;
//  }
//
//    return n;
//}
//
//STDMETHODIMP ConnectionPoint_Impl::GetConnectionInterface( IID __RPC_FAR *piid)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  *piid = IID_IDispatch;
//
//  return ret;
//}
//
//STDMETHODIMP ConnectionPoint_Impl::GetConnectionPointContainer(IConnectionPointContainer __RPC_FAR *__RPC_FAR *ppCPC)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  *ppCPC = m_rContainer.get();
//
//  (*ppCPC)->AddRef();
//
//  return ret;
//}
//
//void ConnectionPoint_Impl::registerListener()
//{
//  Reference<XIntrospectionAccess> xAccess = m_xInvocation->getIntrospection();
//
//  if (xAccess.is())
//  {
//      Sequence<  Type > listeners = xAccess->getSupportedListeners();
//      Reference<XAllListener> xListener = this;
//
//      for (sal_uInt32 i = 0; i < listeners.getLength(); i++)
//      {
//          GenListenerData data;

    //              data.listenerType = OUString::createFromAscii(
    //                  listeners.getConstArray()[i].getTypeName());
//          data.listenerType = listeners.getConstArray()[i].getTypeName();
//
//          try
//          {
//              data.xGenListener = m_xEventAttacher->attachListener(m_xOrigin,
//                                                                   xListener,
//                                                                   data.helper,
//                                                                   data.listenerType,
//                                                                   data.addListenerParam);
//
//              m_xGenListenerList.push_back(data);
//          }
//          catch(IllegalArgumentException e)
//          {e;
//          }
//          catch(IntrospectionException e)
//          {e;
//          }
//          catch(CannotCreateAdapterException e)
//          {e;
//          }
//          catch(ServiceNotRegisteredException e)
//          {e;
//          }
//      }
//  }
//}
//
//void ConnectionPoint_Impl::unregisterListener()
//{
//  Reference<XIntrospectionAccess> xAccess = m_xInvocation->getIntrospection();
//
//  if (xAccess.is())
//  {
//      GenListenerDataList::iterator iter = m_xGenListenerList.begin();
//
//      while (iter != m_xGenListenerList.end())
//      {
//          try
//          {
//              m_xEventAttacher->removeListener(m_xOrigin,
//                                               (*iter).listenerType,
//                                               (*iter).addListenerParam,
//                                               (*iter).xGenListener);
//          }
//          catch(IllegalArgumentException e)
//          {e;
//          }
//          catch(IntrospectionException e)
//          {e;
//          }
//
//          iter++;
//      }
//  }
//
//  m_xGenListenerList = GenListenerDataList();
//}
//
//STDMETHODIMP ConnectionPoint_Impl::Advise(IUnknown __RPC_FAR *pUnkSink, DWORD __RPC_FAR *pdwCookie)
//{
//  HRESULT ret = ResultFromScode(CONNECT_E_CANNOTCONNECT);
//
//  OGuard guard(globalWrapperMutex);
//
//  IDispatch* pDispatch = NULL;
//
//  if (pUnkSink->QueryInterface(IID_IDispatch, (void**) &pDispatch) == NOERROR)
//  {
//      *pdwCookie = m_nextCookie++;
//
//      if (m_connection.size() == 0)
//      {
//          registerListener();
//      }
//
//      m_connection.insert(ConnectionMap::value_type(*pdwCookie, OleRef<IDispatch>(pDispatch)));
//
//      pDispatch->Release();
//
//      ret = ResultFromScode(S_OK);
//  }
//
//  return ret;
//}
//
//STDMETHODIMP ConnectionPoint_Impl::Unadvise(DWORD dwCookie)
//{
//  HRESULT ret = ResultFromScode(CONNECT_E_NOCONNECTION);
//
//  OGuard guard(globalWrapperMutex);
//
//  ConnectionMap::iterator iter = m_connection.find(dwCookie);
//
//  if (iter != m_connection.end())
//  {
//      m_connection.erase(iter);
//      ret = ResultFromScode(S_OK);
//  }
//
//  if (m_connection.size() == 0)
//  {
//      unregisterListener();
//  }
//
//  return ret;
//}
//
//STDMETHODIMP ConnectionPoint_Impl::EnumConnections(IEnumConnections __RPC_FAR *__RPC_FAR *ppEnum)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  OGuard guard(globalWrapperMutex);
//
//  CONNECTDATA* pConnection = NULL;
//  UINT32 n = m_connection.size();
//
//  if (n > 0)
//  {
//      pConnection = new CONNECTDATA[n];
//
//      ConnectionMap::iterator iter = m_connection.begin();
//      UINT32 i = 0;
//
//      while (iter != m_connection.end())
//      {
//          (*iter).second->QueryInterface(IID_IUnknown, (void**)&(pConnection[i].pUnk));
//          pConnection[i].dwCookie = (*iter).first;
//
//          i++;
//          iter++;
//      }
//
//      *ppEnum = new EnumConnections_Impl(pConnection, n);
//
//      for (i = 0; i < n; i++)
//      {
//          pConnection[i].pUnk->Release();
//      }
//
//      delete[] pConnection;
//  }
//  else
//  {
//      *ppEnum = new EnumConnections_Impl(NULL, 0);
//  }
//
//  (*ppEnum)->AddRef();
//
//  return ret;
//}
//
//
//
//
//void SAL_CALL ConnectionPoint_Impl::disposing(const EventObject& Source) throw( RuntimeException)
//{
//}
//
//void SAL_CALL ConnectionPoint_Impl::firing(const AllEventObject& Event) throw( RuntimeException)
//{
//  IEnumConnections* pEnum = NULL;
//
//  EnumConnections(&pEnum);
//
//  CONNECTDATA connection = {NULL, 0};
//  ULONG fetched = NULL;
//
//  while (pEnum->Next(1, &connection, &fetched) == NOERROR)
//  {
//      IDispatch* pDispatch = NULL;
//
//      if (connection.pUnk->QueryInterface(IID_IDispatch, (void**) &pDispatch) == NOERROR)
//      {
//          // getting dispid of event sink
//          unsigned int    flags = 0;
//          HRESULT         result;
//          DISPID          dispId;
//          OLECHAR*        oleNames[1];
//
//          oleNames[0] = (OLECHAR*)Event.MethodName.getStr();
//
//          result = pDispatch ->GetIDsOfNames(IID_NULL,
//                                              oleNames,
//                                              1,
//                                              LOCALE_SYSTEM_DEFAULT,
//                                              &dispId);
//
//          if (result == NOERROR)
//          {
//              DISPPARAMS      dispparams;
//              VARIANT         varResult;
//              EXCEPINFO       excepinfo;
//              unsigned int    uArgErr;
//
//              VariantInit(&varResult);
//
//              // converting UNO parameters to OLE variants
//              dispparams.rgdispidNamedArgs = NULL;
//              dispparams.cArgs = Event.Arguments.getLength();
//              dispparams.cNamedArgs = 0;
//
//              if (dispparams.cArgs == 0)
//              {
//                  dispparams.rgvarg = NULL;
//              }
//              else
//              {
//                  dispparams.rgvarg = (VARIANTARG*)malloc(sizeof(VARIANTARG) * dispparams.cArgs);
//
//                  for (UINT32 i = 0; i < dispparams.cArgs; i++)
//                  {
//                      o2u_anyToVariant(&(dispparams.rgvarg[i]),
//                                       Event.Arguments.getConstArray()[dispparams.cArgs - (i + 1)]);
//                  }
//              }
//
//              // invoking OLE method
//              result = pDispatch ->Invoke(dispId,
//                                           IID_NULL,
//                                           LOCALE_SYSTEM_DEFAULT,
//                                           DISPATCH_METHOD,
//                                           &dispparams,
//                                           &varResult,
//                                           &excepinfo,
//                                           &uArgErr);
//
//
//              // freeing allocated OLE parameters
//              if (dispparams.cArgs > 0)
//              {
//                  for (UINT32 i = 0; i < dispparams.cArgs; i++)
//                  {
//                      VariantClear(&(dispparams.rgvarg[i]));
//                  }
//                  free(dispparams.rgvarg);
//              }
//
//              VariantClear(&varResult);
//          }
//
//          // getting dispid of event sink
//          oleNames[0] = L"EventRaised";
//
//          result = pDispatch ->GetIDsOfNames(IID_NULL,
//                                              oleNames,
//                                              1,
//                                              LOCALE_SYSTEM_DEFAULT,
//                                              &dispId);
//
//          if (result == NOERROR)
//          {
//              DISPPARAMS      dispparams;
//              VARIANT         varResult;
//              EXCEPINFO       excepinfo;
//              unsigned int    uArgErr;
//
//              VariantInit(&varResult);
//
//              // converting UNO parameters to OLE variants
//              dispparams.rgdispidNamedArgs = NULL;
//              dispparams.cArgs = Event.Arguments.getLength() + 1;
//              dispparams.cNamedArgs = 0;
//
//              if (dispparams.cArgs == 0)
//              {
//                  dispparams.rgvarg = NULL;
//              }
//              else
//              {
//                  dispparams.rgvarg = (VARIANTARG*)malloc(sizeof(VARIANTARG) * dispparams.cArgs);
//
//                  for (UINT32 i = 0; i < (dispparams.cArgs - 1); i++)
//                  {
//                      o2u_anyToVariant(&(dispparams.rgvarg[i]),
//                                       Event.Arguments.getConstArray()[dispparams.cArgs - (i + 2)]);
//                  }
//                  Any methodName( &Event.MethodName, getCppuType( &Event.MethodName));
//
//                  o2u_anyToVariant(&(dispparams.rgvarg[i]), methodName);
//              }
//
//              // invoking OLE method
//              result = pDispatch ->Invoke(dispId,
//                                           IID_NULL,
//                                           LOCALE_SYSTEM_DEFAULT,
//                                           DISPATCH_METHOD,
//                                           &dispparams,
//                                           &varResult,
//                                           &excepinfo,
//                                           &uArgErr);
//
//
//              // freeing allocated OLE parameters
//              if (dispparams.cArgs > 0)
//              {
//                  for (UINT32 i = 0; i < dispparams.cArgs; i++)
//                  {
//                      VariantClear(&(dispparams.rgvarg[i]));
//                  }
//                  free(dispparams.rgvarg);
//              }
//
//              VariantClear(&varResult);
//          }
//
//          pDispatch->Release();
//      }
//
//      connection.pUnk->Release();
//  }
//
//  pEnum->Release();
//}

//Any SAL_CALL ConnectionPoint_Impl::approveFiring(const AllEventObject& Event) throw( InvocationTargetException, RuntimeException)
//{
//  Any ret;
//
//  return ret;
//}

/*****************************************************************************

    class implementation: EnumConnections_Impl

*****************************************************************************/


//EnumConnections_Impl::EnumConnections_Impl(CONNECTDATA* pConnection, ULONG numOfConn)
//{
//  m_index = 0;
//  m_numOfConn = numOfConn;
//
//  if (m_numOfConn)
//  {
//      m_pConnection = new CONNECTDATA[m_numOfConn];
//
//      for (ULONG i = 0; i < m_numOfConn; i++)
//      {
//          m_pConnection[i] = pConnection[i];
//          m_pConnection[i].pUnk->AddRef();
//      }
//  }
//}
//
//EnumConnections_Impl::~EnumConnections_Impl()
//{
//  for (ULONG i = 0; i < m_numOfConn; i++)
//  {
//      m_pConnection[i].pUnk->Release();
//  }
//
//  if (m_numOfConn)
//  {
//      delete[] m_pConnection;
//  }
//}
//
//STDMETHODIMP EnumConnections_Impl::QueryInterface(REFIID riid, LPVOID FAR * ppv)
//{
//    if(IsEqualIID(riid, IID_IUnknown))
//  {
//      AddRef();
//      *ppv = (IUnknown*) (IEnumConnections *) this;
//      return NOERROR;
//    }
//    else if (IsEqualIID(riid, IID_IEnumConnections ))
//  {
//      AddRef();
//      *ppv = (IEnumConnections *) this;
//      return NOERROR;
//  }
//
//    *ppv = NULL;
//
//  return ResultFromScode(E_NOINTERFACE);
//}
//
//STDMETHODIMP_(ULONG) EnumConnections_Impl::AddRef()
//{
//  return m_refCount.acquire();
//}
//
//STDMETHODIMP_(ULONG) EnumConnections_Impl::Release()
//{
//
//  ULONG n = m_refCount.release();
//
//  if (n == 0)
//  {
//      delete this;
//  }
//
//    return n;
//}
//
//STDMETHODIMP EnumConnections_Impl::Next(
//    /* [in] */ ULONG cConnections,
//    /* [out] */ CONNECTDATA __RPC_FAR *rgcd,
//    /* [out] */ ULONG __RPC_FAR *lpcFetched)
//{
//  HRESULT ret = ResultFromScode(S_FALSE);
//
//  *lpcFetched = VOS_MIN(cConnections, m_numOfConn - m_index);
//
//  for (ULONG i = 0; i < (*lpcFetched); i++)
//  {
//      rgcd[i] = m_pConnection[m_index + i];
//      rgcd[i].pUnk->AddRef();
//  }
//
//  m_index += (*lpcFetched);
//
//  if ((*lpcFetched) == cConnections)
//      ret = ResultFromScode(S_OK);
//
//  return ret;
//}
//
//
//STDMETHODIMP EnumConnections_Impl::Skip(
//    /* [in] */ ULONG cConnections)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  m_index = VOS_MIN(m_index + cConnections, m_numOfConn);
//
//  return ret;
//}
//
//STDMETHODIMP EnumConnections_Impl::Reset( void)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  m_index = 0;
//
//  return ret;
//}
//
//STDMETHODIMP EnumConnections_Impl::Clone(
//    /* [out] */ IEnumConnections __RPC_FAR *__RPC_FAR *ppEnum)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  *ppEnum = new EnumConnections_Impl(m_pConnection, m_numOfConn);
//
//  return ret;
//}

/*****************************************************************************

    class implementation: EnumConnectionPoints_Impl

*****************************************************************************/

//EnumConnectionPoints_Impl::EnumConnectionPoints_Impl(OleRef<IConnectionPoint>*    pPoints,
//                                                   ULONG numOfPoints)
//{
//  m_index = 0;
//  m_numOfPoints = numOfPoints;
//
//  if (m_numOfPoints)
//  {
//      m_pPoints = new OleRef<IConnectionPoint>[m_numOfPoints];
//
//      for (ULONG i = 0; i < m_numOfPoints; i++)
//      {
//          m_pPoints[i] = pPoints[i];
//      }
//  }
//}
//
//EnumConnectionPoints_Impl::~EnumConnectionPoints_Impl()
//{
//  if (m_numOfPoints)
//  {
//      delete[] m_pPoints;
//  }
//}
//
//STDMETHODIMP EnumConnectionPoints_Impl::QueryInterface(REFIID riid, LPVOID FAR * ppv)
//{
//    if(IsEqualIID(riid, IID_IUnknown))
//  {
//      AddRef();
//      *ppv = (IUnknown*) (IEnumConnectionPoints*) this;
//      return NOERROR;
//    }
//    else if (IsEqualIID(riid, IID_IEnumConnectionPoints))
//  {
//      AddRef();
//      *ppv = (IEnumConnectionPoints*) this;
//      return NOERROR;
//  }
//
//    *ppv = NULL;
//
//  return ResultFromScode(E_NOINTERFACE);
//}
//
//STDMETHODIMP_(ULONG) EnumConnectionPoints_Impl::AddRef()
//{
//  return m_refCount.acquire();
//}
//
//STDMETHODIMP_(ULONG) EnumConnectionPoints_Impl::Release()
//{
//
//  ULONG n = m_refCount.release();
//
//  if (n == 0)
//  {
//      delete this;
//  }
//
//    return n;
//}
//
//STDMETHODIMP EnumConnectionPoints_Impl::Next(
//    /* [in] */ ULONG cPoints,
//    /* [out] */ IConnectionPoint __RPC_FAR *__RPC_FAR *rgpcn,
//    /* [out] */ ULONG __RPC_FAR *lpcFetched)
//{
//  HRESULT ret = ResultFromScode(S_FALSE);
//
//  *lpcFetched = VOS_MIN(cPoints, m_numOfPoints - m_index);
//
//  for (ULONG i = 0; i < (*lpcFetched); i++)
//  {
//      rgpcn[i] = m_pPoints[m_index + i].get();
//      rgpcn[i]->AddRef();
//  }
//
//  m_index += (*lpcFetched);
//
//  if ((*lpcFetched) == cPoints)
//      ret = ResultFromScode(S_OK);
//
//  return ret;
//}
//
//STDMETHODIMP EnumConnectionPoints_Impl::Skip(
//    /* [in] */ ULONG cPoints)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  m_index = VOS_MIN(m_index + cPoints, m_numOfPoints);
//
//  return ret;
//}
//
//STDMETHODIMP EnumConnectionPoints_Impl::Reset( void)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  m_index = 0;
//
//  return ret;
//}
//
//STDMETHODIMP EnumConnectionPoints_Impl::Clone(
//    /* [out] */ IEnumConnectionPoints __RPC_FAR *__RPC_FAR *ppEnum)
//{
//  HRESULT ret = ResultFromScode(S_OK);
//
//  *ppEnum = new EnumConnectionPoints_Impl(m_pPoints, m_numOfPoints);
//
//  return ret;
//}

/*************************************************************************

    UnoObjectWrapperRemoteOpt

*************************************************************************/
UnoObjectWrapperRemoteOpt::UnoObjectWrapperRemoteOpt( Reference<XMultiServiceFactory>& aFactory,
                                                     sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
                                                     InterfaceOleWrapper_Impl( aFactory, unoWrapperClass, comWrapperClass)

{
}
UnoObjectWrapperRemoteOpt::~UnoObjectWrapperRemoteOpt()
{
}

// UnoConversionUtilities
Reference< XInterface > UnoObjectWrapperRemoteOpt::createUnoWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                                 m_xMultiServiceFactory, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

STDMETHODIMP  UnoObjectWrapperRemoteOpt::GetIDsOfNames ( REFIID riid, OLECHAR ** rgszNames, unsigned int cNames,
                                LCID lcid, DISPID * rgdispid )
{
    static DISPID currentId= 1;

    if( ! rgdispid)
        return E_POINTER;
    HRESULT ret = E_UNEXPECTED;
    // ----------------------------------------
    // _GetValueObject
    if( ! wcscmp( *rgszNames, JSCRIPT_VALUE_FUNC))
    {
        *rgdispid= DISPID_JSCRIPT_VALUE_FUNC;
        return S_OK;
    }
    // ----------------------------------------
    if (m_xInvocation.is() && (cNames > 0))
    {
        OUString name(rgszNames[0]);
        // has this name been determined as "bad"
        BadNameMap::iterator badIter= m_badNameMap.find( name);
        if( badIter == m_badNameMap.end() )
        {
            // name has not been bad before( member exists
            typedef NameToIdMap::iterator ITnames;
            pair< ITnames, bool > pair_id= m_nameToDispIdMap.insert( NameToIdMap::value_type(name, currentId++));
            // new ID inserted ?
            if( pair_id.second )
            {// yes, now create MemberInfo and ad to IdToMemberInfoMap
                MemberInfo d(0, name);
                m_idToMemberInfoMap.insert( IdToMemberInfoMap::value_type( currentId - 1, d));
            }

            *rgdispid = pair_id.first->second;
            ret = S_OK;
        }
        else
            ret= DISP_E_UNKNOWNNAME;
    }
    return ret;
}

STDMETHODIMP  UnoObjectWrapperRemoteOpt::Invoke ( DISPID dispidMember, REFIID riid, LCID lcid, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * puArgErr )
{
    HRESULT ret = S_OK;

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

            Sequence<Any> params; // holds converted any s
            if( ! info.flags )
            { // DISPID called for the first time
                if( wFlags == DISPATCH_METHOD )
                {
                    if( convertVARIANTARGS(dispidMember, wFlags, pdispparams, params ))
                    {
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
                                if( exactName.getLength() != 0)
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                        pexcepinfo, puArgErr, exactName, params)))
                                      info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;

                    } // end if ( convertVARIANTARGS )
                    else // convertVARIANTARGS failed
                        ret= DISP_E_BADVARTYPE;
                } //if( wFlags == DISPATCH_METHOD )

                else if( wFlags == DISPATCH_PROPERTYPUT || wFlags == DISPATCH_PROPERTYPUTREF)
                {
                    if( convertVARIANTARGS(dispidMember, wFlags, pdispparams, params ))
                    {
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
                                if( exactName.getLength() != 0)
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                        pexcepinfo, puArgErr, exactName, params)))
                                      info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYGET;
                    } // end if ( convertVARIANTARGS )
                    else
                        ret= DISP_E_BADVARTYPE;

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
                            if( exactName.getLength() != 0)
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
                    if( convertVARIANTARGS(dispidMember, wFlags, pdispparams, params ))
                    {// try first as method
                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                  pexcepinfo, puArgErr, info.name, params))
                                  && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( exactName.getLength() != 0)
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
                                if( exactName.getLength() != 0)
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                        pexcepinfo, puArgErr, exactName, params)))
                                      info.name= exactName;
                                }
                            }
                            if( SUCCEEDED( ret ) )
                                info.flags= DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYGET;
                        }
                    } // end if ( convertVARIANTARGS )
                    else
                        ret= DISP_E_BADVARTYPE;
                }
                else if( wFlags & DISPATCH_METHOD && wFlags & DISPATCH_PROPERTYGET)
                {
                    OUString exactName;
                    if( convertVARIANTARGS(dispidMember, wFlags, pdispparams, params ))
                    {
                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                  pexcepinfo, puArgErr, info.name, params))
                                  && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( exactName.getLength() != 0)
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                        pexcepinfo, puArgErr, exactName, params)))
                                      info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;

                    } // end if ( convertVARIANTARGS )
                    else
                        ret= DISP_E_BADVARTYPE;

                    // try as property
                    if( FAILED( ret) && pdispparams->cArgs == 1)
                    {
                        if( FAILED( ret= doGetProperty( pdispparams, pvarResult,
                                  pexcepinfo, info.name))
                                  && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            if( exactName.getLength() != 0)
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

                // update ínformation about this member
                if( ret == DISP_E_MEMBERNOTFOUND)
                {
                    // Remember the name as not existing
                    // and remove the MemberInfo
                    m_badNameMap[info.name]= sal_False;
                    m_idToMemberInfoMap.erase( it_MemberInfo);
                }
            } // if( ! info.flags )
            else // IdToMemberInfoMap contains a MemberInfo
            {
                if( wFlags & DISPATCH_METHOD && info.flags == DISPATCH_METHOD)
                {
                    if( convertVARIANTARGS(dispidMember, wFlags, pdispparams, params ))
                    {
                        ret= doInvoke( pdispparams, pvarResult,
                                  pexcepinfo, puArgErr, info.name, params);
                    }
                    else
                        ret= DISP_E_BADVARTYPE;
                }
                else if( (wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF )  &&
                        info.flags & DISPATCH_PROPERTYPUT)
                {
                    if( convertVARIANTARGS(dispidMember, wFlags, pdispparams, params ))
                    {
                        ret= doSetProperty( pdispparams, pvarResult,
                                  pexcepinfo, puArgErr, info.name, params);
                    }
                    else
                        ret= DISP_E_BADVARTYPE;
                }
                else if( (wFlags & DISPATCH_PROPERTYGET) && ( info.flags & DISPATCH_PROPERTYGET))
                {
                    ret= doGetProperty( pdispparams, pvarResult,
                              pexcepinfo, info.name);
                }
            }
        }//     if( it_MemberInfo != m_idToMemberInfoMap.end() )
        else
            ret= DISP_E_MEMBERNOTFOUND;
    }
    return ret;
}

HRESULT UnoObjectWrapperRemoteOpt::methodInvoke( DISPID dispidMember, DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, Sequence<Any> params)
{
    return S_OK;
}


// The returned HRESULT is only appropriate for IDispatch::Invoke
static HRESULT mapCannotConvertException( CannotConvertException e, unsigned int * puArgErr)
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

// The function maps the TypeClass of the any to VARTYPE: If
// the Any contains STRUCT or INTERFACE then the return value
// is VT_DISPATCH. The function is used from o2u_createUnoObjectWrapper
// and the result is put into the constructor of the uno - wrapper
// object. If a client asks the object for DISPID_VALUE and this
// funtion returned VT_DISPATCH then the IDispatch of the same
// object is being returned.
// See InterfaceOleWrapper_Impl::Invoke, InterfaceOleWrapper_Impl::m_defaultValueType
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




} // end namespace
