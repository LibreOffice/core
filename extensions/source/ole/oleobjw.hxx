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

#ifndef INCLUDED_EXTENSIONS_SOURCE_OLE_OLEOBJW_HXX
#define INCLUDED_EXTENSIONS_SOURCE_OLE_OLEOBJW_HXX

#include "ole2uno.hxx"
#include "wincrap.hxx"

#include <unordered_map>
#include <vector>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
#include <com/sun/star/script//XAutomationInvocation.hpp>
#include <rtl/ustring.hxx>

#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/script/XDirectInvocation.hpp>

#include <typelib/typedescription.hxx>
#include "unoconversionutilities.hxx"
#include "windata.hxx"
using namespace cppu;
using namespace std;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::oleautomation;

namespace ole_adapter
{


typedef std::unordered_map<OUString, pair<DISPID, unsigned short>, OUStringHash> DispIdMap;

typedef std::unordered_multimap<OUString, unsigned int, OUStringHash> TLBFuncIndexMap;

// This class wraps an IDispatch and maps XInvocation calls to IDispatch calls on the wrapped object.
// If m_TypeDescription is set then this class represents an UNO interface implemented in a COM component.
// The interface is not a real interface in terms of an abstract class but is realized through IDispatch.
class IUnknownWrapper_Impl : public WeakImplHelper< XBridgeSupplier2, XInitialization, XAutomationObject, XDefaultProperty, XDefaultMethod, XDirectInvocation, XAutomationInvocation >,

                             public UnoConversionUtilities<IUnknownWrapper_Impl>

{
public:
    IUnknownWrapper_Impl(Reference<XMultiServiceFactory> &xFactory,
                         sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass);

    ~IUnknownWrapper_Impl();

    //XInterface
    Any SAL_CALL queryInterface(const Type& t)
        throw (RuntimeException);

    // XInvokation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(  )
        throw(RuntimeException);
    virtual Any SAL_CALL invoke( const OUString& aFunctionName,
                                 const Sequence< Any >& aParams,
                                 Sequence< sal_Int16 >& aOutParamIndex,
                                 Sequence< Any >& aOutParam )
        throw(IllegalArgumentException, CannotConvertException,
              InvocationTargetException, RuntimeException);
    virtual void SAL_CALL setValue( const OUString& aPropertyName,
                                    const Any& aValue )
        throw(UnknownPropertyException, CannotConvertException,
              InvocationTargetException, RuntimeException);
    virtual Any SAL_CALL getValue( const OUString& aPropertyName )
        throw(UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName )
        throw(RuntimeException);
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName )
        throw(RuntimeException);

    // XBridgeSupplier2
    // This interface is implemented to provide a safe way to obtain the original
    // IUnknown or IDispatch within the function anyToVariant. The function asks
    // every UNO object for its XBridgeSupplier2 and if it is available uses it to convert
    // the object with its own supplier.
    virtual Any SAL_CALL createBridge( const Any& modelDepObject,
                                       const Sequence< sal_Int8 >& aProcessId,
                                       sal_Int16 sourceModelType,
                                       sal_Int16 destModelType )
        throw(IllegalArgumentException, RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException);

    // XDefaultProperty
    virtual OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return m_sDefaultMember; }

    // XDefaultMethod
    virtual OUString SAL_CALL getDefaultMethodName(  ) throw (css::uno::RuntimeException) { return m_sDefaultMember; }

    virtual css::uno::Any SAL_CALL invokeGetProperty( const OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL invokePutProperty( const OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);

    // XDirectInvocation
    virtual css::uno::Any SAL_CALL directInvoke( const OUString& aName, const css::uno::Sequence< css::uno::Any >& aParams ) throw (css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasMember( const OUString& aName ) throw (css::uno::RuntimeException);


    Any  invokeWithDispIdComTlb(FuncDesc& aFuncDesc,
                            const OUString& sFuncName,
                            const Sequence< Any >& Params,
                            Sequence< sal_Int16 >& OutParamIndex,
                            Sequence< Any >& OutParam);


protected:

    virtual Any invokeWithDispIdUnoTlb(const OUString& sFunctionName,
                                       const Sequence< Any >& Params,
                                       Sequence<sal_Int16 >& OutParamIndex,
                                       Sequence< Any >& OutParam);
    // Is used for OleObjectFactory service
    virtual Any invokeWithDispIdComTlb(const OUString& sFuncName,
                                       const Sequence< Any >& Params,
                                       Sequence< sal_Int16 >& OutParamIndex,
                                       Sequence< Any >& OutParam);

    // UnoConversionUtilities -------------------------------------------------------------------------------
    virtual Reference<XInterface> createUnoWrapperInstance();
    virtual Reference<XInterface> createComWrapperInstance();

    /**Obtains a FUNCDESC structure for a function.
       Fills the FUNCDESC structure if ITypeInfo provides information for
       the function of name sFuncName or pFuncDesc will not be filled in.
       May throw a BridgeRuntimeError.
     */
    void getFuncDesc(const OUString & sFuncName, FUNCDESC ** pFuncDesc);
    /**Obtains a FUNCDESC structures or a VARDESC structure
       for a property. pFuncDescPut may also contain
       a structure for a "propertyputref" operation. If pFuncDesc contains a
       "put ref" or "put" FUNCDESC depends on what was found first in the type
       description.
       Fills the FUNCDESC structure if ITypeInfo provides information for
       the respective property functions or the structures will not be filled in.
       May throw a BridgeRuntimeError.
     */
    void getPropDesc(const OUString & sFuncName, FUNCDESC ** pFuncDescGet,
                     FUNCDESC** pFuncDescPut, VARDESC ** pVarDesc);
    // These functions are for the case if an object of this class wraps an IDispatch
    // object that implements UNO interfaces. In that case the member m_seqTypes
    // is set through XInitialization::initialize.
    void getMethodInfo(const OUString& sName, TypeDescription& methodDescription);
    // After return attributInfo contains typelib_InterfaceAttributeTypeDescription::pAttributeTypeRef
    void getAttributeInfo(const OUString& sName, TypeDescription& attributeInfo);
    // used by get MethodInfo
    TypeDescription  getInterfaceMemberDescOfCurrentCall(const OUString& sName);
    /** Returns always a valid ITypeInfo interface or throws a BridgeRuntimeError.
        The returned interface does not need to be AddRef'ed as long as it is locally
        used. The interface is kept in the instance of this class.
     */
    ITypeInfo*  getTypeInfo();

    /** Returns the DISPID for a function or property name. If true is returned then
        id contains a valid DISPID.
    */

    bool getDispid(const OUString& sFuncName, DISPID * id);

    VARTYPE getUserDefinedElementType( ITypeInfo* pTypeInfo, const DWORD nHrefType );

    /** Gets the element type in a VARIANT like style. E.g. if desc->lptdesc contains
        a VT_PTR than it is replaced by VT_BYREF and VT_SAFEARRAY is replaced by VT_ARRAY
        If the TYPEDESC describes an SAFEARRAY then varType is a combination of VT_ARRAY
        and the element type.
        The argument desc must be obtained from FUNCDESC::lprgelemdescParam[i].tdesc where
        FUNCDESC was obtained from the ITypeInfo belonging to wrapped IDispatch.
    */
    VARTYPE getElementTypeDesc( const TYPEDESC *desc);
    /** Iterates over all functions and put the names and indices into the map
        m_mapComFunc of type TLBFuncIndexMap.
        Call the function every time before accessing the map.
        Throws a BridgeRuntimeError on failure.
    */
    void  buildComTlbIndex();

    /** Returns a FUNCDESC structure which contains type information about the
        current XInvocation::invoke call. The FUNCDESC either describes a method,
        a property put or a property get operation.
        It uses the types  com.sun.star.bridge.oleautomation.PropertyPutArgument
        which can be
        contained in the sequence of in-arguments of invoke to determine if the call is
        a property put or property get operation.
        If no adequate FUNCDESC was found, an IllegalArgumentException is thrown.
        Therefore it is safe to assume that the returned FUNCDESC* is not NULL.

        @exception IllegalArgumentException
        Thrown if no adequate FUNCDESC could be found.
    */
    void getFuncDescForInvoke(const OUString & sFuncName,
                              const Sequence<Any> & seqArgs, FUNCDESC** pFuncDesc);

    // Finds out whether the wrapped IDispatch is an JScript Object. This is
    // done by
    // asking for the property "_environment". If it has the value "JScript"
    // (case insensitive) then the IDispatch is considered a JScript object.
    sal_Bool isJScriptObject();


    // If UNO interfaces are implemented in JScript objects, VB or C++ COM objects
    // and those are passed as parameter to a UNO interface function, then
    // the IDispatch* are wrapped by objects of this class. Assuming that the functions
    // implemented by the IDispatch object returns another UNO interface then
    // it has to be wrapped to this type. But this is only possible if an object of this
    // wrapper class knows what type it is represting. The member m_TypeDescription holds this
    // information.
    // m_TypeDescription is only useful when an object wraps an IDispatch object that implements
    // an UNO interface. The value is set during a call to XInitialization::initialize.
    Sequence<Type> m_seqTypes;
    CComPtr<IUnknown> m_spUnknown;
    CComPtr<IDispatch> m_spDispatch;
        OUString m_sTypeName; // is "" ( not initialised ), "IDispatch" ( we have no idea ) or "SomeLibrary.SomeTypeName" if we managed to get a type
    /** This value is set during XInitialization::initialize. It indicates that the COM interface
    was transported as VT_DISPATCH in a VARIANT rather then a VT_UNKNOWN
    */
    sal_Bool  m_bOriginalDispatch;
    DispIdMap           m_dispIdMap;
    Reference<XIdlClass>*       m_pxIdlClass;


    // used by isJScriptObject
    enum JScriptDetermination{ JScriptUndefined=0, NoJScript, IsJScript};
    JScriptDetermination m_eJScript;
    // The map is filled by buildComTlbIndex
    // It maps Uno Function names to an index which is used in ITypeInfo::GetFuncDesc
    TLBFuncIndexMap m_mapComFunc;
    // used for synchroizing the computation of the content for m_mapComFunc
    bool m_bComTlbIndexInit;
    // Keeps the ITypeInfo obtained from IDispatch::GetTypeInfo
    CComPtr< ITypeInfo > m_spTypeInfo;
    OUString m_sDefaultMember;
    bool m_bHasDfltMethod;
    bool m_bHasDfltProperty;
};

} // end namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
