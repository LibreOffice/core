/*************************************************************************
 *
 *  $RCSfile: oleobjw.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2000-10-12 12:58:27 $
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

#ifndef __OLEOBJW_HXX
#define __OLEOBJW_HXX
#include "ole2uno.hxx"

#include <tools/presys.h>
#include "stdafx.h"
#include <vector>
#include <hash_map>
#include <tools/postsys.h>

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#include <com/sun/star/lang/XInitialization.hpp>
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#include <typelib/typedescription.hxx>
#include "unoconversionutilities.hxx"

using namespace cppu;
using namespace rtl;
using namespace std;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;

namespace ole_adapter
{



typedef hash_map<OUString, pair<DISPID, unsigned short>, hashOUString_Impl, equalOUString_Impl> DispIdMap;

typedef hash_map<OUString, unsigned int, hashOUString_Impl, equalOUString_Impl> TLBFuncIndexMap;

// This class wraps an IDispatch and maps XInvocation calls to IDispatch calls on the wrapped object.
// If m_TypeDescription is set then this class represents an UNO interface implemented in a COM component.
// The interface is not a real interface in terms of an abstract class but is realized through IDispatch.
class IUnknownWrapper_Impl : public WeakImplHelper3<XInvocation, XBridgeSupplier2, XInitialization>,
                             public UnoConversionUtilities<IUnknownWrapper_Impl>

{
public:
    IUnknownWrapper_Impl(Reference<XMultiServiceFactory> &xFactory, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass);

    ~IUnknownWrapper_Impl();


    // XInvokation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(  ) throw(RuntimeException);
    virtual Any SAL_CALL invoke( const OUString& aFunctionName, const Sequence< Any >& aParams, Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam ) throw(IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException);
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException);
    virtual Any SAL_CALL getValue( const OUString& aPropertyName ) throw(UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) throw(RuntimeException);

    // XBridgeSupplier2
    // This interface is implemented to provide a safe way to obtain the original
    // IUnknown or IDispatch within the function anyToVariant. The function asks
    // every UNO object for its XBridgeSupplier and if it is available uses it to convert
    // the object with its own supplier.
    virtual Any SAL_CALL createBridge( const Any& modelDepObject, const Sequence< sal_Int8 >& aProcessId, sal_Int16 sourceModelType, sal_Int16 destModelType ) throw(IllegalArgumentException, RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // ------------------------------------------------------------------------------------------------------------------
    virtual Any invokeWithDispIdUnoTlb(DISPID dispID, const Sequence< Any >& Params, Sequence<sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam) throw (IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException);
    // Is used for OleObjectFactory service
    virtual Any invokeWithDispIdComTlb(DISPID dispID,
                          const Sequence< Any >& Params,
                          Sequence< sal_Int16 >& OutParamIndex,
                          Sequence< Any >& OutParam)
              throw ( IllegalArgumentException, CannotConvertException,
                      InvocationTargetException, RuntimeException);

    virtual void setValueWithDispId(DISPID dispID, const Any& Value) throw ( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException);
    virtual Any getValueWithDispId(DISPID dispID) throw (UnknownPropertyException, RuntimeException );


    // UnoConversionUtilities -------------------------------------------------------------------------------
    virtual Reference<XInterface> createUnoWrapperInstance();
    virtual Reference<XInterface> createComWrapperInstance();
protected:

    // These functions are for the case if an object of this class wraps an IDispatch
    // object that implements UNO interfaces. In that case the member m_seqTypes
    // is set through XInitialization::initialize.
    void getMethodInfo( TypeDescription& methodDescription);
    // used by get MethodInfo
    TypeDescription  getInterfaceMemberDescOfCurrentCall( );
    ITypeInfo*  getTypeInfo();


    // Gets information about the parameter types ( VARTYPE) and mode ( in, out, in/out)
    sal_Bool getParameterInfo();
    // analyses the type information provided by the COM component and saves the idizes of the
    // out and in/out parameter in the member m_outIndizes
//  sal_Bool prepareOutParams( VARIANT* params, sal_uInt32 count );
    // Builds up the complete vartype from a TYPEDESC, e.g VT_BSTR, VT_BYREF | VT_I4, VT_BYREF|VT_ARRAY|VT_I1
    sal_Bool getElementTypeDesc( TYPEDESC *desc, VARTYPE& varType );
    // Iterates over all functions and put the names and indices into a map (TLBFuncIndexMap)
    sal_Bool buildComTlbIndex();
    // Determines whether the map m_mapComFunc has already been set up
    sal_Bool isComTlbIndex();

    // The information is necessary for getReturnType and getOutParameterType because they obtain
    // the information for the current call on Invocation.
    void setCurrentInvokeCall( const OUString& name){ m_usCurrentGet=L""; m_usCurrentInvoke= name;}
    void setCurrentGetCall( const OUString& name){ m_usCurrentInvoke= L""; m_usCurrentGet= name;}

    // Finds out wheter the wrapped IDispatch is an JScript Object. This is is done by
    // asking for the property "_environment". If it has the value "JScript" ( case insensitive)
    // then the IDispatch is considered a JScript object.
    sal_Bool isJScriptObject();
    // -------------------------------------------------------------------------------

    DispIdMap::iterator getDispIdEntry(const OUString& name);
    // If UNO interfaces are being implemented in JScript objects, VB or C++ COM objects
    // and those are passed as parameter to a UNO interface function, then
    // the IDispatch* are wrapped by objects of this class. Assuming that the functions
    // implemented by the IDispatch object returns another UNO interface then
    // it has to be wrapped to this type. But this is only possible if an object of this
    // wrapper class knows what type it is represting. The member m_TypeDescription holds this
    // information.
    // m_TypeDescription is only useful when an object wrapps an IDispatch object that implements
    // an UNO interface. The value is set during a call to XInitialization::initialize.
    Sequence<Type> m_seqTypes;
    IUnknown*           m_pUnknown;
    IDispatch*          m_pDispatch;
    DispIdMap           m_dispIdMap;
    Reference<XIdlClass>*       m_pxIdlClass;
    Mutex               m_mutex;

    // see prepare OutParams
    Sequence< sal_Int16 > m_seqOutIndizes;
    // The name of the function being executed ( invoke)
    OUString    m_usCurrentInvoke;
    // The name of the property being retrieved ( getValue)
    OUString    m_usCurrentGet;
    // used by isJScriptObject
    enum JScriptDetermination{ JScriptUndefined=0, NoJScript, IsJScript};
    JScriptDetermination m_eJScript;
    // The map is filled by buildComTlbIndex and
    TLBFuncIndexMap m_mapComFunc;
    // Keeps the ITypeInfo obtained from IDispatch::GetTypeInfo
    CComPtr< ITypeInfo > m_spTypeInfo;
    // keeps the modes of parameters of the current function call
    // These values correspond to Windows PARAMFLAGS constants ( PARAMFLAG_FIN, PARAMFLAG_FOUT)
    // The Sequence contains only the in and out flag
    // The length of the sequence reprensentsf the number of parameters.
    Sequence< sal_Int32> m_seqCurrentParamTypes;
    // contains the VARTYPES of the parameters of the current call
    Sequence<sal_uInt16> m_seqCurrentVartypes;

};

} // end namespace
#endif

