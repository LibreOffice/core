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

#ifndef __OLEOBJW_HXX
#define __OLEOBJW_HXX
#include "ole2uno.hxx"

#ifdef _MSC_VER
#pragma warning (push,1)
#pragma warning (disable:4548)
#endif

#include <tools/presys.h>
#define _WIN32_WINNT 0x0403

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif
#include <atlbase.h>
#include <vector>
#include <boost/unordered_map.hpp>
#include <tools/postsys.h>

#ifdef _MSC_VER
#pragma warning (pop)
#endif
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase6.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
#include <com/sun/star/script//XAutomationInvocation.hpp>
#include <rtl/ustring.hxx>

#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>

#include <typelib/typedescription.hxx>
#include "unoconversionutilities.hxx"
#include "windata.hxx"
using namespace cppu;
using namespace std;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::oleautomation;

using ::rtl::OUString;
namespace ole_adapter
{



typedef boost::unordered_map<OUString, pair<DISPID, unsigned short>, hashOUString_Impl, equalOUString_Impl> DispIdMap;

typedef boost::unordered_multimap<OUString, unsigned int, hashOUString_Impl, equalOUString_Impl> TLBFuncIndexMap;

// This class wraps an IDispatch and maps XInvocation calls to IDispatch calls on the wrapped object.
// If m_TypeDescription is set then this class represents an UNO interface implemented in a COM component.
// The interface is not a real interface in terms of an abstract class but is realized through IDispatch.
class IUnknownWrapper_Impl : public WeakImplHelper6<XAutomationInvocation, XBridgeSupplier2, XInitialization, XAutomationObject, XDefaultProperty, XDefaultMethod>,

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
    virtual ::rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (::com::sun::star::uno::RuntimeException) { return m_sDefaultMember; }
protected:
    virtual ::rtl::OUString SAL_CALL getDefaultMethodName(  ) throw (::com::sun::star::uno::RuntimeException) { return m_sDefaultMember; }

    virtual ::com::sun::star::uno::Any SAL_CALL invokeGetProperty( const ::rtl::OUString& aFunctionName, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams, ::com::sun::star::uno::Sequence< ::sal_Int16 >& aOutParamIndex, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::script::CannotConvertException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL invokePutProperty( const ::rtl::OUString& aFunctionName, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams, ::com::sun::star::uno::Sequence< ::sal_Int16 >& aOutParamIndex, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::script::CannotConvertException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException);

    // ----------------------------------------------------------------------------
    virtual Any invokeWithDispIdUnoTlb(const OUString& sFunctionName,
                                       const Sequence< Any >& Params,
                                       Sequence<sal_Int16 >& OutParamIndex,
                                       Sequence< Any >& OutParam);
    // Is used for OleObjectFactory service
    virtual Any invokeWithDispIdComTlb(const OUString& sFuncName,
                                       const Sequence< Any >& Params,
                                       Sequence< sal_Int16 >& OutParamIndex,
                                       Sequence< Any >& OutParam);

  Any  IUnknownWrapper_Impl::invokeWithDispIdComTlb(FuncDesc& aFuncDesc,
                            const OUString& sFuncName,
                            const Sequence< Any >& Params,
                            Sequence< sal_Int16 >& OutParamIndex,
                            Sequence< Any >& OutParam);

//    virtual void setValueWithDispId(DISPID dispID, const Any& Value);

//    virtual Any getValueWithDispId(const OUString& sName, DISPID dispID);


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
    /** Returns alway a valid ITypeInfo interface or throws a BridgeRuntimeError.
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

    // Finds out wheter the wrapped IDispatch is an JScript Object. This is is
    // done by
    // asking for the property "_environment". If it has the value "JScript"
    // (case insensitive) then the IDispatch is considered a JScript object.
    sal_Bool isJScriptObject();
    // -------------------------------------------------------------------------------

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
        rtl::OUString m_sTypeName; // is "" ( not initialised ), "IDispatch" ( we have no idea ) or "SomeLibrary.SomeTypeName" if we managed to get a type
    /** This value is set dureing XInitialization::initialize. It indicates that the COM interface
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
    rtl::OUString m_sDefaultMember;
    bool m_bHasDfltMethod;
    bool m_bHasDfltProperty;
};

} // end namespace
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
