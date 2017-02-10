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
#ifndef INCLUDED_EXTENSIONS_SOURCE_OLE_UNOCONVERSIONUTILITIES_HXX
#define INCLUDED_EXTENSIONS_SOURCE_OLE_UNOCONVERSIONUTILITIES_HXX

#include <memory>
#include "com/sun/star/script/XInvocationAdapterFactory.hpp"
#include "com/sun/star/script/XInvocationAdapterFactory2.hpp"
#include "com/sun/star/script/XTypeConverter.hpp"
#include "com/sun/star/script/FailReason.hpp"
#include "com/sun/star/bridge/oleautomation/Date.hpp"
#include "com/sun/star/bridge/oleautomation/Currency.hpp"
#include "com/sun/star/bridge/oleautomation/SCode.hpp"
#include "com/sun/star/bridge/oleautomation/Decimal.hpp"
#include "typelib/typedescription.hxx"
#include <o3tl/any.hxx>
#include "ole2uno.hxx"
#include <cppuhelper/weakref.hxx>

#include "unotypewrapper.hxx"
#include <unordered_map>

// for some reason DECIMAL_NEG (wtypes.h) which contains BYTE is not resolved.
typedef unsigned char   BYTE;
// classes for wrapping uno objects
#define INTERFACE_OLE_WRAPPER_IMPL      1
#define UNO_OBJECT_WRAPPER_REMOTE_OPT   2

#define INVOCATION_SERVICE reinterpret_cast<const sal_Unicode*>(L"com.sun.star.script.Invocation")


// classes for wrapping ole objects
#define IUNKNOWN_WRAPPER_IMPL           1

#define INTERFACE_ADAPTER_FACTORY  reinterpret_cast<const sal_Unicode*>(L"com.sun.star.script.InvocationAdapterFactory")
// COM or JScript objects implementing UNO interfaces have to implement this property
#define SUPPORTED_INTERFACES_PROP L"_implementedInterfaces"
// Second property without leading underscore for use in VB
#define SUPPORTED_INTERFACES_PROP2 L"Bridge_ImplementedInterfaces"

using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::bridge::oleautomation;
namespace ole_adapter
{
extern std::unordered_map<sal_uIntPtr, sal_uIntPtr> AdapterToWrapperMap;
extern std::unordered_map<sal_uIntPtr, sal_uIntPtr> WrapperToAdapterMap;
typedef std::unordered_map<sal_uIntPtr, sal_uIntPtr>::iterator IT_Wrap;
typedef std::unordered_map<sal_uIntPtr, sal_uIntPtr>::iterator CIT_Wrap;
//Maps IUnknown pointers to a weak reference of the respective wrapper class (e.g.
// IUnknownWrapperImpl. It is the responsibility of the wrapper to remove the entry when
// it is being destroyed.
// Used to ensure that an Automation object is always mapped to the same UNO objects.
extern std::unordered_map<sal_uIntPtr, WeakReference<XInterface> > ComPtrToWrapperMap;
typedef std::unordered_map<sal_uIntPtr, WeakReference<XInterface> >::iterator IT_Com;
typedef std::unordered_map<sal_uIntPtr, WeakReference<XInterface> >::const_iterator CIT_Com;

// Maps XInterface pointers to a weak reference of its wrapper class (i.e.
// InterfaceOleWrapper_Impl). It is the responsibility of the wrapper to remove the entry when
// it is being destroyed. It is used to ensure the identity of objects. That is, an UNO interface
// is mapped to IDispatch which is kept alive in the COM environment. If the same
// UNO interface is mapped again to COM then the IDispach of the first mapped instance
// must be returned.
extern std::unordered_map<sal_uIntPtr, WeakReference<XInterface> > UnoObjToWrapperMap;
typedef std::unordered_map<sal_uIntPtr, WeakReference<XInterface> >::iterator IT_Uno;
typedef std::unordered_map<sal_uIntPtr, WeakReference<XInterface> >::const_iterator CIT_Uno;

// createUnoObjectWrapper gets a wrapper instance by calling createUnoWrapperInstance
    // and initializes it via XInitialization. The wrapper object is required to implement
    // XBridgeSupplier so that it can convert itself to IDispatch.
    // class T: Deriving class ( must implement XInterface )
/** All methods are allowed to throw at least a BridgeRuntimeError.
 */
template< class >
class UnoConversionUtilities
{
public:
    explicit UnoConversionUtilities( const Reference<XMultiServiceFactory> & smgr):
        m_nUnoWrapperClass( INTERFACE_OLE_WRAPPER_IMPL),
        m_nComWrapperClass( IUNKNOWN_WRAPPER_IMPL),
        m_smgr( smgr)
    {}

    UnoConversionUtilities( const Reference<XMultiServiceFactory> & xFactory, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass )
        : m_nUnoWrapperClass(unoWrapperClass),
          m_nComWrapperClass(comWrapperClass), m_smgr(xFactory)
    {}

    virtual ~UnoConversionUtilities() {}
    /** converts only into oleautomation types, that is there is no VT_I1, VT_UI2, VT_UI4
        a sal_Unicode character is converted into a BSTR.
        @exception com.sun.star.lang.IllegalArgumentException
        If the any was inappropriate for conversion.
        @exception com.sun.star.script.CannotConvertException
        The any contains a type class for which no conversion is provided.
    */
    void anyToVariant(VARIANT* pVariant, const Any& rAny);
    void anyToVariant(VARIANT* pVariant, const Any& rAny, VARTYPE type);

    /** @exception com.sun.star.lang.IllegalArgumentException
        If rSeq does not contain a sequence then the exception is thrown.
    */
    SAFEARRAY*  createUnoSequenceWrapper(const Any& rSeq);
    /** @exception com.sun.star.lang.IllegalArgumentException
        If rSeq does not contain a sequence or elemtype has no proper value
        then the exception is thrown.
    */
    SAFEARRAY*  createUnoSequenceWrapper(const Any& rSeq, VARTYPE elemtype);
    /**
       @exception com.sun.star.lang.IllegalArgumentException
       If rObj does not contain a struct or interface
     */
    void createUnoObjectWrapper(const Any & rObj, VARIANT * pVar);
    /** @exception CannotConvertException
        Thrown if the VARIANT contains a type that cannot be coerced in the expected Any.
        ArgumentIndex is 0.
        @IllegalArgumentException
        Thrown if the VARIANT is inappropriate for conversion. ArgumentPosition is -1,
     */
    void variantToAny(const VARIANT* pVariant, Any& rAny, bool bReduceValueRange = true);
    /** This method converts variants arguments in calls from COM -> UNO. Only then
        the expected UNO type is known.
        @exception CannotConvertException
        Thrown if the VARIANT contains a type that cannot be coerced in the expected Any.
        ArgumentIndex is 0.
        @IllegalArgumentException
        Thrown if the VARIANT is inappropriate for conversion. ArgumentPosition is -1,
     */
    void variantToAny( const VARIANTARG* pArg, Any& rAny, const Type& ptype, bool bReduceValueRange = true);

    /**
       @exception IllegalArgumentException
       -if pVar does not contain VT_UNKNOWN or VT_DISPATCH or
       pVar is used for a particular UNO type which is not supported by pVar
     */
    Any createOleObjectWrapper(VARIANT* pVar, const Type& aType= Type());

    /*
      Return true means var contained a ValueObject, and it was successfully converted.
      The result is in any. It an error occurred a BridgeRuntimeError will be thrown.
     */
    bool convertValueObject( const VARIANTARG *var, Any& any);
    void dispatchExObject2Sequence( const VARIANTARG* pvar, Any& anySeq, const Type& type);

    Sequence<Any> createOleArrayWrapperOfDim(SAFEARRAY* pArray, unsigned int dimCount, unsigned int actDim, long* index,
                                             VARTYPE type, const Type& unotype);
    Sequence<Any> createOleArrayWrapper(SAFEARRAY* pArray, VARTYPE type, const Type& unotype= Type());


    VARTYPE mapTypeClassToVartype( TypeClass type);
    Reference< XSingleServiceFactory > getInvocationFactory(const Any& anyObject);


    virtual Reference< XInterface > createUnoWrapperInstance()=0;
    virtual Reference< XInterface > createComWrapperInstance()=0;

    static bool isJScriptArray(const VARIANT* pvar);

    Sequence<Type> getImplementedInterfaces(IUnknown* pUnk);

protected:
    Reference<XInterface> createAdapter(const Sequence<Type>& types, const Reference<XInterface>& receiver);

    // helper function for Sequence conversion
    void getElementCountAndTypeOfSequence( const Any& rSeq, sal_Int32 dim, Sequence< sal_Int32 >& seqElementCounts, TypeDescription& typeDesc);
    // helper function for Sequence conversion
    static bool incrementMultidimensionalIndex(sal_Int32 dimensions, const sal_Int32 * parDimensionLength,
                                    sal_Int32 * parMultidimensionalIndex);
    // helper function for Sequence conversion
    static size_t getOleElementSize( VARTYPE type);

    static Type getElementTypeOfSequence( const Type& seqType);

    //Provides a typeconverter
    Reference<XTypeConverter> getTypeConverter();

    // This member determines what class is used to convert a UNO object
    // or struct to a COM object. It is passed along to the o2u_anyToVariant
    // function in the createBridge function implementation
    sal_uInt8 m_nUnoWrapperClass;
    sal_uInt8 m_nComWrapperClass;

    // The servicemanager is either a local smgr or remote when the service
    // com.sun.star.bridge.OleBridgeSupplierVar1 is used. This service can be
    // created by createInstanceWithArguments where one can supply a service
    // manager that is to be used.
    // Local service manager as supplied by the loader when the creator function
    // of the service is being called.
    Reference<XMultiServiceFactory> m_smgr;
    // An explicitly supplied service manager when the service
    // com.sun.star.bridge.OleBridgeSupplierVar1 is used. That can be a remote
    // manager.
    Reference<XMultiServiceFactory> m_smgrRemote;
    Reference<XSingleServiceFactory> m_xInvocationFactoryLocal;
    Reference<XSingleServiceFactory> m_xInvocationFactoryRemote;

private:
    // Holds the type converter which is used for sequence conversion etc.
    // Use the getTypeConverter function to obtain the interface.
    Reference<XTypeConverter> m_typeConverter;


};

// ask the object for XBridgeSupplier2 and on success bridges
// the uno object to IUnknown or IDispatch.
// return  true the UNO object supports
template < class T >
bool convertSelfToCom( T& unoInterface, VARIANT * pVar)
{
    bool ret = false;
    Reference< XInterface > xInt( unoInterface, UNO_QUERY);
    if( xInt.is())
    {
        Reference< XBridgeSupplier2 > xSupplier( xInt, UNO_QUERY);
        if( xSupplier.is())
        {
            sal_Int8 arId[16];
            rtl_getGlobalProcessId( reinterpret_cast<sal_uInt8*>(arId));
            Sequence<sal_Int8> seqId( arId, 16);
            Any anySource;
            anySource <<= xInt;
            Any anyDisp = xSupplier->createBridge(anySource, seqId, UNO, OLE);

            // due to global-process-id check this must be in-process pointer
            if (auto v = o3tl::tryAccess<sal_uIntPtr>(anyDisp))
            {
                VARIANT* pvariant= reinterpret_cast<VARIANT*>(*v);
                HRESULT hr;
                if (FAILED(hr = VariantCopy(pVar, pvariant)))
                    throw BridgeRuntimeError(
                        "[automation bridge] convertSelfToCom\n"
                        "VariantCopy failed! Error: " +
                        OUString::number(hr));
                VariantClear( pvariant);
                CoTaskMemFree( pvariant);
                ret = true;
            }
        }
    }
    return ret;
}


// Gets the invocation factory depending on the Type in the Any.
// The factory can be created by a local or remote multi service factory.
// In case there is a remote multi service factory available there are
// some services or types for which the local factory is used. The exceptions
// are:  all structs.
// Param anyObject - contains the object ( interface, struct) for what we need an invocation object.

template<class T>
Reference< XSingleServiceFactory > UnoConversionUtilities<T>::getInvocationFactory(const Any& anyObject)
{
    Reference< XSingleServiceFactory > retVal;
    MutexGuard guard( getBridgeMutex());
    if( anyObject.getValueTypeClass() != TypeClass_STRUCT &&
        m_smgrRemote.is() )
    {
        if(  ! m_xInvocationFactoryRemote.is() )
            m_xInvocationFactoryRemote.set(m_smgrRemote->createInstance( INVOCATION_SERVICE), UNO_QUERY);
        retVal= m_xInvocationFactoryRemote;
    }
    else
    {
        if( ! m_xInvocationFactoryLocal.is() )
            m_xInvocationFactoryLocal.set(m_smgr->createInstance(INVOCATION_SERVICE ), UNO_QUERY);
        retVal= m_xInvocationFactoryLocal;
    }
    return retVal;
}

template<class T>
void UnoConversionUtilities<T>::variantToAny( const VARIANTARG* pArg, Any& rAny, const Type& ptype, bool bReduceValueRange /* = sal_True */)
{
    try
    {
        HRESULT hr;
        bool bFail = false;
        bool bCannotConvert = false;
        CComVariant var;

        // There is no need to support indirect values, since they're not supported by UNO
        if( FAILED(hr= VariantCopyInd( &var, pArg))) // remove VT_BYREF
            throw BridgeRuntimeError(
                "[automation bridge] UnoConversionUtilities<T>::variantToAny \n"
                "VariantCopyInd failed for reason : " + OUString::number(hr));
        bool bHandled = convertValueObject( & var, rAny);
        if( bHandled)
            OSL_ENSURE(  rAny.getValueType() == ptype, "type in Value Object must match the type parameter");

        if( ! bHandled)
        {
            // convert into a variant type that is the equivalent to the type
            // the sequence expects. Thus variantToAny produces the correct type
            // E.g. An Array object contains VT_I4 and the sequence expects shorts
            // than the vartype must be changed. The reason is, you can't specify the
            // type in JavaScript and the script engine determines the type being used.
            switch( ptype.getTypeClass())
            {
            case TypeClass_CHAR: // could be: new Array( 12, 'w', "w")
                if( var.vt == VT_BSTR)
                {
                    if(SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_BSTR)))
                        rAny.setValue( V_BSTR( &var), ptype);
                    else if (hr == DISP_E_TYPEMISMATCH)
                        bCannotConvert = true;
                    else
                        bFail = true;
                }
                else
                {
                    if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_I2)))
                        rAny.setValue(& var.iVal, ptype);
                    else if (hr == DISP_E_TYPEMISMATCH)
                        bCannotConvert = true;
                    else
                        bFail = true;
                }
                break;
            case TypeClass_INTERFACE: // could also be an IUnknown
            case TypeClass_STRUCT:
            {
                rAny = createOleObjectWrapper( & var, ptype);
                break;
            }
            case TypeClass_ENUM:
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_I4)))
                    rAny.setValue(& var.lVal, ptype);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_SEQUENCE:
                // There are different ways of receiving a sequence:
                // 1: JScript, VARTYPE: VT_DISPATCH
                // 2. VBScript simple arraysVT_VARIANT|VT_BYREF the referenced VARIANT contains
                //      a VT_ARRAY|  <type>
                // 3. VBSrcript multi dimensional arrays: VT_ARRAY|VT_BYREF
                if( pArg->vt == VT_DISPATCH)
                {
                    dispatchExObject2Sequence( pArg, rAny, ptype);
                }
                else
                {
                    if ((var.vt & VT_ARRAY) != 0)
                    {
                        VARTYPE oleType = ::sal::static_int_cast< VARTYPE, int >( var.vt ^ VT_ARRAY );
                        Sequence<Any> unoSeq = createOleArrayWrapper( var.parray, oleType, ptype);
                        Reference<XTypeConverter> conv = getTypeConverter();
                        if (conv.is())
                        {
                            try
                            {
                                Any anySeq = makeAny(unoSeq);
                                Any convAny = conv->convertTo(anySeq, ptype);
                                rAny = convAny;
                            }
                            catch (const IllegalArgumentException& e)
                            {
                                throw BridgeRuntimeError(
                                    "[automation bridge]com.sun.star.lang.IllegalArgumentException "
                                    "in UnoConversionUtilities<T>::variantToAny! Message: " +
                                    e.Message);
                            }
                            catch (const CannotConvertException& e)
                            {
                                throw BridgeRuntimeError(
                                    "[automation bridge]com.sun.star.script.CannotConvertException "
                                    "in UnoConversionUtilities<T>::variantToAny! Message: " +
                                    e.Message);
                            }
                        }
                    }
                }
                break;
            case TypeClass_VOID:
                rAny.setValue(nullptr,Type());
                break;
            case TypeClass_ANY:     //  Any
                // There could be a JScript Array that needs special handling
                // If an Any is expected and this Any must contain a Sequence
                // then we cannot figure out what element type is required.
                // Therefore we convert to Sequence< Any >
                if( pArg->vt == VT_DISPATCH &&  isJScriptArray( pArg))
                {
                    dispatchExObject2Sequence( pArg, rAny,
                                               cppu::UnoType<Sequence<Any>>::get());
                }
                else if (pArg->vt == VT_DECIMAL)
                {
                    //Decimal maps to hyper in calls from COM -> UNO
                    // It does not matter if we create a sal_uInt64 or sal_Int64,
                    // because the UNO object is called through invocation which
                    //will do a type conversion if necessary
                    if (var.decVal.sign == 0)
                    {
                        // positive value
                        variantToAny( & var, rAny, cppu::UnoType<sal_uInt64>::get(),
                                      bReduceValueRange);
                    }
                    else
                    {
                        //negative value
                        variantToAny( & var, rAny, cppu::UnoType<sal_Int64>::get(),
                                      bReduceValueRange);
                    }
                }
                else
                {
                    variantToAny( & var, rAny);
                }
                break;
            case TypeClass_BOOLEAN:         // VARIANT could be VARIANT_BOOL or other
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_BOOL)))
                    variantToAny( & var, rAny);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_STRING:      // UString
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_BSTR)))
                    variantToAny( & var, rAny);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_FLOAT:       // float
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_R4)))
                    variantToAny( & var, rAny);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_DOUBLE:      // double
            if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_R8)))
                variantToAny(& var, rAny);
            else if (hr == DISP_E_TYPEMISMATCH)
                bCannotConvert = true;
            else
                bFail = true;
            break;
            case TypeClass_BYTE:            // BYTE
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_I1)))
                    variantToAny( & var, rAny);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_SHORT:       // INT16
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_I2)))
                    variantToAny( & var, rAny);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_LONG:
                if(SUCCEEDED(hr = VariantChangeType(& var, &var, 0, VT_I4)))
                    variantToAny( & var, rAny, bReduceValueRange);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_HYPER:
                if(SUCCEEDED(hr = VariantChangeType(& var, &var, 0, VT_DECIMAL)))
                {
                    if (var.decVal.Lo64 > SAL_CONST_UINT64(0x8000000000000000)
                        || var.decVal.Hi32 > 0
                        || var.decVal.scale > 0)
                    {
                        bFail = true;
                        break;
                    }
                    sal_Int64 value = var.decVal.Lo64;
                    if (var.decVal.sign == DECIMAL_NEG)
                        value |=  SAL_CONST_UINT64(0x8000000000000000);
                    rAny <<= value;
                }
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_UNSIGNED_SHORT:  // UINT16
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_UI2)))
                    variantToAny( & var, rAny);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_UNSIGNED_LONG:
                if(SUCCEEDED(hr = VariantChangeType( & var, &var, 0, VT_UI4)))
                    variantToAny( & var, rAny, bReduceValueRange);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_UNSIGNED_HYPER:
                if(SUCCEEDED(hr = VariantChangeType(& var, &var, 0, VT_DECIMAL)))
                {
                    if (var.decVal.Hi32 > 0 || var.decVal.scale > 0)
                    {
                        bFail = true;
                        break;
                    }
                    rAny <<= var.decVal.Lo64;
                }
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            case TypeClass_TYPE:
                if(SUCCEEDED(hr = VariantChangeType(& var, &var, 0, VT_UNKNOWN)))
                    variantToAny( & var, rAny);
                else if (hr == DISP_E_TYPEMISMATCH)
                    bCannotConvert = true;
                else
                    bFail = true;
                break;
            default:
                bCannotConvert = true;
                break;
            }
        }
        if (bCannotConvert)
            throw CannotConvertException(
                "[automation bridge]UnoConversionUtilities<T>::variantToAny \n"
                "Cannot convert the value of vartype :\"" +
                OUString::number((sal_Int32) var.vt) +
                "\"  to the expected UNO type of type class: " +
                OUString::number((sal_Int32) ptype.getTypeClass()),
                nullptr, TypeClass_UNKNOWN, FailReason::TYPE_NOT_SUPPORTED,0);

        if (bFail)
            throw IllegalArgumentException(
                "[automation bridge]UnoConversionUtilities<T>:variantToAny\n"
                "The provided VARIANT of type\" " + OUString::number((sal_Int32) var.vt) +
                "\" is unappropriate for conversion!", Reference<XInterface>(), -1);
    }
    catch (const CannotConvertException &)
    {
        throw;
    }
    catch (const IllegalArgumentException &)
    {
        throw;
    }
    catch (const BridgeRuntimeError &)
    {
         throw;
    }
    catch (const Exception & e)
    {
        throw BridgeRuntimeError("[automation bridge] unexpected exception in "
                                 "UnoConversionUtilities<T>::variantToAny ! Message : \n" +
                               e.Message);
    }
    catch(...)
    {
        throw BridgeRuntimeError(
                  "[automation bridge] unexpected exception in "
                  "UnoConversionUtilities<T>::variantToAny !");
    }
}

// The function only converts Sequences to SAFEARRAYS with elements of the type
// specified by the parameter type. Everything else is forwarded to
// anyToVariant(VARIANT* pVariant, const Any& rAny)
// Param type must not be VT_BYREF
template<class T>
void UnoConversionUtilities<T>::anyToVariant(VARIANT* pVariant, const Any& rAny, VARTYPE type)
{
    try
    {
        HRESULT hr= S_OK;

        OSL_ASSERT( (type & VT_BYREF) == 0);
        if (type & VT_ARRAY)
        {
            type ^= VT_ARRAY;
            SAFEARRAY* ar= createUnoSequenceWrapper( rAny, type);
            if( ar)
            {
                VariantClear( pVariant);
                pVariant->vt= ::sal::static_int_cast< VARTYPE, int >( VT_ARRAY | type );
                pVariant->byref= ar;
            }
        }
        else if(type == VT_VARIANT)
        {
            anyToVariant(pVariant, rAny);
        }
        else
        {
            CComVariant var;
            anyToVariant( &var, rAny);
            if(FAILED(hr = VariantChangeType(&var, &var, 0, type)))
            {
                if (hr == DISP_E_TYPEMISMATCH)
                    throw CannotConvertException(
                        "[automation bridge]UnoConversionUtilities<T>::anyToVariant \n"
                        "Cannot convert the value of type :\"" +
                        rAny.getValueTypeName() +
                        "\"  to the expected Automation type of VARTYPE: " +
                        OUString::number((sal_Int32)type),
                        nullptr, TypeClass_UNKNOWN, FailReason::TYPE_NOT_SUPPORTED,0);

                throw BridgeRuntimeError(
                    "[automation bridge]UnoConversionUtilities<T>::anyToVariant \n"
                    "Conversion of any with " +
                    rAny.getValueType().getTypeName() +
                    " to VARIANT with type: " + OUString::number((sal_Int32) type) +
                    " failed! Error code: " + OUString::number(hr));

            }
            if(FAILED(hr = VariantCopy(pVariant, &var)))
            {
                throw BridgeRuntimeError(
                          "[automation bridge]UnoConversionUtilities<T>::anyToVariant \n"
                          "VariantCopy failed for reason: " + OUString::number(hr));
            }
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
    catch (const BridgeRuntimeError&)
    {
        throw;
    }
    catch(const Exception & e)
    {
        throw BridgeRuntimeError(
                  "[automation bridge]UnoConversionUtilities<T>::anyToVariant \n"
                  "Unexpected exception occurred. Message: " + e.Message);
    }
    catch(...)
    {
        throw BridgeRuntimeError(
                  "[automation bridge]UnoConversionUtilities<T>::anyToVariant \n"
                  "Unexpected exception occurred.");
    }
}

template<class T>
void UnoConversionUtilities<T>::anyToVariant(VARIANT* pVariant, const Any& rAny)
{
    try
    {
        bool bIllegal = false;
        switch (rAny.getValueTypeClass())
        {
        case TypeClass_INTERFACE:
        {
            Reference<XInterface> xInt;
            if (rAny >>= xInt)
            {
                createUnoObjectWrapper(rAny, pVariant);
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_STRUCT:
        {
            if (rAny.getValueType() == cppu::UnoType<Date>::get() )
            {
                Date d;
                if (rAny >>= d)
                {
                    pVariant->vt = VT_DATE;
                    pVariant->date = d.Value;
                }
                else
                {
                    bIllegal = true;
                }
            }
            else if(rAny.getValueType() == cppu::UnoType<Decimal>::get())
            {
                Decimal d;
                if (rAny >>= d)
                {
                    pVariant->vt = VT_DECIMAL;
                    pVariant->decVal.scale = d.Scale;
                    pVariant->decVal.sign = d.Sign;
                    pVariant->decVal.Lo32 = d.LowValue;
                    pVariant->decVal.Mid32 = d.MiddleValue;
                    pVariant->decVal.Hi32 = d.HighValue;
                }
                else
                {
                    bIllegal = true;
                }
            }
            else if (rAny.getValueType() == cppu::UnoType<Currency>::get())
            {
                Currency c;
                if (rAny >>= c)
                {
                    pVariant->vt = VT_CY;
                    pVariant->cyVal.int64 = c.Value;
                }
                else
                {
                    bIllegal = true;
                }
            }
            else if(rAny.getValueType() == cppu::UnoType<SCode>::get())
            {
                SCode s;
                if (rAny >>= s)
                {
                    pVariant->vt = VT_ERROR;
                    pVariant->scode = s.Value;
                }
                else
                {
                    bIllegal = true;
                }
            }
            else
            {
                createUnoObjectWrapper(rAny, pVariant);
            }
            break;
        }
        case TypeClass_SEQUENCE:        // sequence ??? SafeArray descriptor
        {
            SAFEARRAY* pArray = createUnoSequenceWrapper(rAny);
            if (pArray)
            {
                V_VT(pVariant) = VT_ARRAY | VT_VARIANT;
                V_ARRAY(pVariant) = pArray;
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_VOID:
        {
            HRESULT hr = S_OK;
            if (FAILED(hr = VariantClear(pVariant)))
            {
                throw BridgeRuntimeError(
                        "[automation bridge]UnoConversionUtilities<T>::anyToVariant\n"
                        "VariantClear failed with error:" + OUString::number(hr));
            }
            break;
        }
        case TypeClass_BOOLEAN:
        {
            bool value;
            if (rAny >>= value)
            {
                pVariant->vt = VT_BOOL;
                pVariant->boolVal = value ? VARIANT_TRUE: VARIANT_FALSE;
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_CHAR:
        {
            // Because VT_UI2 does not conform to oleautomation we convert into VT_I2 instead
            sal_uInt16 value = *o3tl::forceAccess<sal_Unicode>(rAny);
            pVariant->vt = VT_I2;
            pVariant->iVal = value;
            break;
        }
        case TypeClass_STRING:
        {
            OUString value;
            if (rAny >>= value)
            {
                pVariant->vt = VT_BSTR;
                pVariant->bstrVal = SysAllocString(reinterpret_cast<LPCOLESTR>(value.getStr()));
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_FLOAT:
        {
            float value;
            if (rAny >>= value)
            {
                pVariant->vt = VT_R4;
                pVariant->fltVal = value;
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_DOUBLE:
        {
            double value;
            if (rAny >>= value)
            {
                pVariant->vt = VT_R8;
                pVariant->dblVal = value;
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_BYTE:
        {
            // ole automation does not know a signed char but only unsigned char
            sal_Int8 value;
            if (rAny >>= value)
            {
                pVariant->vt = VT_UI1;
                pVariant->bVal = value;
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_SHORT:       // INT16
        case TypeClass_UNSIGNED_SHORT:  // UINT16
        {
            sal_Int16 value;
            if (rAny >>= value)
            {
                pVariant->vt = VT_I2;
                pVariant->iVal = value;
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_ENUM:
        {
            sal_Int32 value = *static_cast<sal_Int32 const *>(rAny.getValue());
            pVariant->vt = VT_I4;
            pVariant->lVal= value;
            break;
        }
        case TypeClass_LONG:
        case TypeClass_UNSIGNED_LONG:
        {
            sal_Int32 value;
            if (rAny >>= value)
            {
                pVariant->vt = VT_I4;
                pVariant->lVal= value;
            }
            else
            {
                bIllegal = true;
            }
            break;
        }
        case TypeClass_HYPER:
        {

            pVariant->vt = VT_DECIMAL;
            pVariant->decVal.scale = 0;
            pVariant->decVal.sign = 0;
            pVariant->decVal.Hi32 = 0;

            sal_Int64 value;
            rAny >>= value;

            if (value & SAL_CONST_UINT64(0x8000000000000000))
                pVariant->decVal.sign = DECIMAL_NEG;

            pVariant->decVal.Lo64 = value;
            break;
        }
        case TypeClass_UNSIGNED_HYPER:
        {
            pVariant->vt = VT_DECIMAL;
            pVariant->decVal.scale = 0;
            pVariant->decVal.sign = 0;
            pVariant->decVal.Hi32 = 0;

            sal_uInt64 value;
            rAny >>= value;
            pVariant->decVal.Lo64 = value;
            break;
        }
        case TypeClass_TYPE:
        {
            Type type;
            rAny >>= type;
            CComVariant var;
            if (!createUnoTypeWrapper(type.getTypeName(), & var))
                throw BridgeRuntimeError(
                          "[automation bridge] UnoConversionUtilities<T>::anyToVariant \n"
                          "Error during conversion of UNO type to Automation object!");

            if (FAILED(VariantCopy(pVariant, &var)))
                throw BridgeRuntimeError(
                          "[automation bridge] UnoConversionUtilities<T>::anyToVariant \n"
                          "Unexpected error!");
            break;
        }
        default:
            //TypeClass_SERVICE:
            //TypeClass_EXCEPTION:
            //When a InvocationTargetException is thrown when calling XInvocation::invoke
            //on a UNO object, then the target exception is directly used to create a
            //EXEPINFO structure
            //TypeClass_TYPEDEF
            //TypeClass_ANY:
            //TypeClass_UNKNOWN:
            //TypeClass_MODULE:
            throw CannotConvertException(
                      "[automation bridge]UnoConversionUtilities<T>::anyToVariant\n"
                      "There is no conversion for this UNO type to a Automation type."
                      "The destination type class is the type class of the UNO "
                      "argument which was to be converted.",
                Reference<XInterface>(), rAny.getValueTypeClass(),
                FailReason::TYPE_NOT_SUPPORTED, 0);

            break;
        }
        if (bIllegal)
        {
            throw IllegalArgumentException(
                      "[automation bridge]UnoConversionUtilities<T>::anyToVariant\n"
                      "The provided any of type\" " + rAny.getValueType().getTypeName() +
                "\" is unappropriate for conversion!", Reference<XInterface>(), -1);

        }
    }
    catch (const CannotConvertException &)
    {
        throw;
    }
    catch (const IllegalArgumentException &)
    {
        throw;
    }
    catch(const BridgeRuntimeError&)
    {
        throw;
    }
    catch(const Exception & e)
    {
        throw BridgeRuntimeError(
                  "[automation bridge]UnoConversionUtilities<T>::anyToVariant \n"
                  "Unexpected exception occurred. Message: " + e.Message);
    }
    catch(...)
    {
        throw BridgeRuntimeError(
                  "[automation bridge]UnoConversionUtilities<T>::anyToVariant \n"
                  "Unexpected exception occurred. " );
    }
}

// Creates an SAFEARRAY of the specified element and if necessary
// creates a SAFEARRAY with multiple dimensions.
// Used by sal_Bool anyToVariant(VARIANT* pVariant, const Any& rAny, VARTYPE type);
template<class T>
SAFEARRAY*  UnoConversionUtilities<T>::createUnoSequenceWrapper(const Any& rSeq, VARTYPE elemtype)
{
    if (rSeq.getValueTypeClass() != TypeClass_SEQUENCE)
        throw IllegalArgumentException(
                  "[automation bridge]UnoConversionUtilities<T>::createUnoSequenceWrapper \n"
                  "The any does not contain a sequence!", nullptr, 0);
    if (elemtype == VT_NULL  ||  elemtype == VT_EMPTY)
        throw IllegalArgumentException(
                  "[automation bridge]UnoConversionUtilities<T>::createUnoSequenceWrapper \n"
                  "No element type supplied!",nullptr, -1);
    SAFEARRAY*  pArray= nullptr;
    // Get the dimensions. This is done by examining the type name string
    // The count of brackets determines the dimensions.
    OUString sTypeName= rSeq.getValueType().getTypeName();
    sal_Int32 dims=0;
    for(sal_Int32 lastIndex=0;(lastIndex= sTypeName.indexOf( L'[', lastIndex)) != -1; lastIndex++,dims++);

    //get the maximum number of elements per dimensions and the typedescription of the elements
    Sequence<sal_Int32> seqElementCounts( dims);
    TypeDescription elementTypeDesc;
    getElementCountAndTypeOfSequence( rSeq, 1, seqElementCounts, elementTypeDesc );

    if( elementTypeDesc.is() )
    {
        // set up the SAFEARRAY
        std::unique_ptr<SAFEARRAYBOUND[]> sarSafeArrayBound(new SAFEARRAYBOUND[dims]);
        SAFEARRAYBOUND* prgsabound= sarSafeArrayBound.get();
        for( sal_Int32 i=0; i < dims; i++)
        {
            //prgsabound[0] is the right most dimension
             prgsabound[dims - i - 1].lLbound = 0;
            prgsabound[dims - i - 1].cElements = seqElementCounts[i];
        }

        typelib_TypeDescription* rawTypeDesc= elementTypeDesc.get();
        sal_Int32 elementSize= rawTypeDesc->nSize;
        size_t oleElementSize= getOleElementSize( elemtype);
        // SafeArrayCreate clears the memory for the data itself.
        pArray = SafeArrayCreate(elemtype, dims, prgsabound);

        // convert the Sequence's elements and populate the SAFEARRAY
        if( pArray)
        {
            // Iterate over every Sequence that contains the actual elements
            void* pSAData;
            if( SUCCEEDED( SafeArrayAccessData( pArray, &pSAData)))
            {
                const sal_Int32* parElementCount= seqElementCounts.getConstArray();
                uno_Sequence * pMultiSeq= *static_cast<uno_Sequence* const*>(rSeq.getValue());
                sal_Int32 dimsSeq= dims - 1;

                // arDimSeqIndices contains the current index of a block of data.
                // E.g. Sequence<Sequence<sal_Int32>> , the index would refer to Sequence<sal_Int32>
                // In this case arDimSeqIndices would have the size 1. That is the elements are not counted
                // but the Sequences that contain those elements.
                // The indices ar 0 based
                std::unique_ptr<sal_Int32[]> sarDimsSeqIndices;
                sal_Int32* arDimsSeqIndices= nullptr;
                if( dimsSeq > 0)
                {
                    sarDimsSeqIndices.reset(new sal_Int32[dimsSeq]);
                    arDimsSeqIndices = sarDimsSeqIndices.get();
                    memset( arDimsSeqIndices, 0,  sizeof( sal_Int32 ) * dimsSeq);
                }

                char* psaCurrentData= static_cast<char*>(pSAData);

                do
                {
                    // Get the Sequence at the current index , see arDimsSeqIndices
                    uno_Sequence * pCurrentSeq= pMultiSeq;
                    sal_Int32 curDim=1; // 1 based
                    bool skipSeq= false;
                    while( curDim <= dimsSeq )
                    {
                        // get the Sequence at the index if valid
                        if( pCurrentSeq->nElements > arDimsSeqIndices[ curDim - 1] ) // don't point to Nirvana
                        {
                            // size of Sequence is 4
                            sal_Int32 offset= arDimsSeqIndices[ curDim - 1] * 4;
                            pCurrentSeq= *reinterpret_cast<uno_Sequence**>(&pCurrentSeq->elements[ offset]);
                            curDim++;
                        }
                        else
                        {
                            // There is no Sequence at this index, so skip this index
                            skipSeq= true;
                            break;
                        }
                    }

                    if( skipSeq)
                        continue;

                    // Calculate the current position within the datablock of the SAFEARRAY
                    // for the next Sequence.
                    sal_Int32 memOffset= 0;
                    sal_Int32 dimWeight= parElementCount[ dims - 1]; // size of the rightmost dimension
                    for(sal_Int32 idims=0; idims < dimsSeq; idims++ )
                    {
                        memOffset+= arDimsSeqIndices[dimsSeq - 1 - idims] * dimWeight;
                        // now determine the weight of the dimension to the left of the current.
                        if( dims - 2 - idims >=0)
                            dimWeight*= parElementCount[dims - 2 - idims];
                    }
                    psaCurrentData= static_cast<char*>(pSAData) + memOffset * oleElementSize;
                    // convert the Sequence and put the elements into the Safearray
                    for( sal_Int32 i= 0; i < pCurrentSeq->nElements; i++)
                    {
                        Any unoElement( pCurrentSeq->elements + i * elementSize, rawTypeDesc );
                        // The any is being converted into an VARIANT which value is then copied
                        // to the SAFEARRAY's data block. When copying one has to follow the rules for
                        // copying certain types, as are VT_DISPATCH, VT_UNKNOWN, VT_VARIANT, VT_BSTR.
                        // To increase performance, we just do a memcpy of VARIANT::byref. This is possible
                        // because anyToVariant has already followed the copying rules. To make this
                        // work there must not be a VariantClear.
                        // One Exception is VARIANT because I don't know how VariantCopy works.

                        VARIANT var;
                        VariantInit( &var);
                        anyToVariant( &var, unoElement);
                        if( elemtype == VT_VARIANT )
                        {
                            VariantCopy( reinterpret_cast<VARIANT*>(psaCurrentData), &var);
                            VariantClear( &var);
                        }
                        else
                            memcpy( psaCurrentData, &var.byref, oleElementSize);

                        psaCurrentData+= oleElementSize;
                    }
                }
                while( incrementMultidimensionalIndex( dimsSeq, parElementCount, arDimsSeqIndices));

                SafeArrayUnaccessData( pArray);
            }
        }
    }
    return pArray;
}

// Increments a multi dimensional index.
// Returns true as long as the index has been successfully incremented, false otherwise.
// False is also returned if an overflow of the most significant dimension occurs. E.g.
// assume an array with the dimensions (2,2), then the lowest index is (0,0) and the highest
// index is (1,1). If the function is being called with the index (1,1) then the overflow would
// occur, with the result (0,0) and a sal_False as return value.
// Param dimensions - number of dimensions
// Param parDimensionsLength - The array contains the size of each dimension, that is the
//                              size of the array equals the parameter dimensions.
//                              The rightmost dimensions is the least significant one
//                              ( parDimensionsLengths[ dimensions -1 ] ).
// Param parMultiDimensionalIndex - The array contains the index. Each dimension index is
//                                  0 based.
template<class T>
bool UnoConversionUtilities<T>::incrementMultidimensionalIndex(sal_Int32 dimensions,
                                                                   const sal_Int32 * parDimensionLengths,
                                                                   sal_Int32 * parMultidimensionalIndex)
{
    if( dimensions < 1)
        return false;

    bool ret= true;
    bool carry= true; // to get into the while loop

    sal_Int32 currentDimension= dimensions; //most significant is 1
    while( carry)
    {
        parMultidimensionalIndex[ currentDimension - 1]++;
        // if carryover, set index to 0 and handle carry on a level above
        if( parMultidimensionalIndex[ currentDimension - 1] > (parDimensionLengths[ currentDimension - 1] - 1))
            parMultidimensionalIndex[ currentDimension - 1]= 0;
        else
            carry= false;

        currentDimension --;
        // if dimensions drops below 1 and carry is set than then all indices are 0 again
        // this is signalled by returning sal_False
        if( currentDimension < 1 && carry)
        {
            carry= false;
            ret= false;
        }
    }
    return ret;
}

// Determines the size of a certain OLE type. The function takes
// only those types into account which are oleautomation types and
// can have a value ( unless VT_NULL, VT_EMPTY, VT_ARRAY, VT_BYREF).
// Currently used in createUnoSequenceWrapper to calculate addresses
// for data within a SAFEARRAY.
template<class T>
size_t UnoConversionUtilities<T>::getOleElementSize( VARTYPE type)
{
    size_t size;
    switch( type)
    {
    case VT_BOOL: size= sizeof( VARIANT_BOOL);break;
    case VT_UI1: size= sizeof( unsigned char);break;
    case VT_R8: size= sizeof( double);break;
    case VT_R4: size= sizeof( float);break;
    case VT_I2: size= sizeof( short);break;
    case VT_I4: size= sizeof( long);break;
    case VT_BSTR: size= sizeof( BSTR); break;
    case VT_ERROR: size= sizeof( SCODE); break;
    case VT_DISPATCH:
    case VT_UNKNOWN: size= sizeof( IUnknown*); break;
    case VT_VARIANT: size= sizeof( VARIANT);break;
    default: size= 0;
    }
    return size;
}

//If a Sequence is being converted into a SAFEARRAY then we possibly have
// to create a SAFEARRAY with multiple dimensions. This is the case when a
// Sequence contains Sequences ( Sequence< Sequence < XXX > > ). The leftmost
// Sequence in the declaration is assumed to represent dimension 1. Because
// all Sequence elements of a Sequence can have different length, we have to
// determine the maximum length which is then the length of the respective
// dimension.
// getElementCountAndTypeOfSequence determines the length of each dimension and calls itself recursively
// in the process.
// param rSeq - an Any that has to contain a Sequence
// param dim - the dimension for which the number of elements is being determined,
//              must be one.
// param seqElementCounts - contains the maximum number of elements for each
//                          dimension. Index 0 contains the number of dimension one.
//                          After return the Sequence contains the maximum number of
//                          elements for each dimension.
//                          The length of the Sequence must equal the number of dimensions.
// param typeClass - TypeClass of the element type that is no Sequence, e.g.
//                          Sequence< Sequence <Sequence <sal_Int32> > > - type is sal_Int32)
template<class T>
void  UnoConversionUtilities<T>::getElementCountAndTypeOfSequence( const Any& rSeq, sal_Int32 dim,
                                             Sequence< sal_Int32 >& seqElementCounts, TypeDescription& typeDesc)
{
    sal_Int32 dimCount= (*static_cast<uno_Sequence* const *>(rSeq.getValue()))->nElements;
    if( dimCount > seqElementCounts[ dim-1])
        seqElementCounts[ dim-1]= dimCount;

    // we need the element type to construct the any that is
    // passed into getElementCountAndTypeOfSequence again
    typelib_TypeDescription* pSeqDesc= nullptr;
    rSeq.getValueTypeDescription( &pSeqDesc);
    typelib_TypeDescriptionReference* pElementDescRef= reinterpret_cast<typelib_IndirectTypeDescription*>(pSeqDesc)->pType;

    // if the elements are Sequences than do recursion
    if( dim < seqElementCounts.getLength() )
    {
        uno_Sequence* pSeq = *static_cast<uno_Sequence* const*>(rSeq.getValue());
        uno_Sequence** arSequences= reinterpret_cast<uno_Sequence**>(pSeq->elements);
        for( sal_Int32 i=0; i < dimCount; i++)
        {
            uno_Sequence* arElement=  arSequences[ i];
            getElementCountAndTypeOfSequence( Any( &arElement, pElementDescRef), dim + 1 , seqElementCounts, typeDesc);
        }
    }
    else
    {
        // determine the element type ( e.g. Sequence< Sequence <Sequence <sal_Int32> > > - type is sal_Int32)
        typeDesc= pElementDescRef;
    }
    typelib_typedescription_release( pSeqDesc);
}


template<class T>
SAFEARRAY*  UnoConversionUtilities<T>::createUnoSequenceWrapper(const Any& rSeq)
{
    SAFEARRAY* pArray = nullptr;
    sal_uInt32 n = 0;

    if( rSeq.getValueTypeClass() != TypeClass_SEQUENCE )
        throw IllegalArgumentException(
                  "[automation bridge]UnoConversionUtilities<T>::createUnoSequenceWrapper\n"
                  "The UNO argument is not a sequence", nullptr, -1);

    uno_Sequence * punoSeq= *static_cast<uno_Sequence* const *>(rSeq.getValue());

    typelib_TypeDescriptionReference* pSeqTypeRef= rSeq.getValueTypeRef();
    typelib_TypeDescription* pSeqType= nullptr;
    TYPELIB_DANGER_GET( &pSeqType, pSeqTypeRef);
    typelib_IndirectTypeDescription * pSeqIndDec=   reinterpret_cast<typelib_IndirectTypeDescription*>(pSeqType);


    typelib_TypeDescriptionReference * pSeqElementTypeRef= pSeqIndDec->pType;
    TYPELIB_DANGER_RELEASE( pSeqType);

    typelib_TypeDescription* pSeqElementDesc= nullptr;
    TYPELIB_DANGER_GET( &pSeqElementDesc, pSeqElementTypeRef);
    sal_Int32 nElementSize= pSeqElementDesc->nSize;
    n= punoSeq->nElements;

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = n;
    VARIANT oleElement;
    long safeI[1];

    pArray = SafeArrayCreate(VT_VARIANT, 1, rgsabound);

    Any unoElement;
    char * pSeqData= punoSeq->elements;

    for (sal_uInt32 i = 0; i < n; i++)
    {
        unoElement.setValue( pSeqData + i * nElementSize, pSeqElementDesc);
        VariantInit(&oleElement);

        anyToVariant(&oleElement, unoElement);

        safeI[0] = i;
        SafeArrayPutElement(pArray, safeI, &oleElement);

        VariantClear(&oleElement);
    }
    TYPELIB_DANGER_RELEASE( pSeqElementDesc);

    return pArray;
}

/* The argument rObj can contain
- UNO struct
- UNO interface
- UNO interface created by this bridge (adapter factory)
- UNO interface created by this bridge ( COM Wrapper)

pVar must be initialized.
*/
template<class T>
void UnoConversionUtilities<T>::createUnoObjectWrapper(const Any & rObj, VARIANT * pVar)
{
    MutexGuard guard(getBridgeMutex());

    Reference<XInterface> xInt;

    TypeClass tc = rObj.getValueTypeClass();
    if (tc != TypeClass_INTERFACE && tc != TypeClass_STRUCT)
        throw IllegalArgumentException(
                  "[automation bridge]UnoConversionUtilities<T>::createUnoObjectWrapper \n"
                  "Cannot create an Automation interface for a UNO type which is not "
                  "a struct or interface!", nullptr, -1);

    if (rObj.getValueTypeClass() == TypeClass_INTERFACE)
    {
        if (! (rObj >>= xInt))
            throw IllegalArgumentException(
                  "[automation bridge] UnoConversionUtilities<T>::createUnoObjectWrapper\n "
                  "Could not create wrapper object for UNO object!", nullptr, -1);
        //If XInterface is NULL, which is a valid value, then simply return NULL.
        if ( ! xInt.is())
        {
            pVar->vt = VT_UNKNOWN;
            pVar->punkVal = nullptr;
            return;
        }
        //make sure we have the main XInterface which is used with a map
        xInt.set(xInt, UNO_QUERY);
        //If there is already a wrapper for the UNO object then use it

        Reference<XInterface> xIntWrapper;
        // Does a UNO wrapper exist already ?
        IT_Uno it_uno = UnoObjToWrapperMap.find( reinterpret_cast<sal_uIntPtr>(xInt.get()));
        if(it_uno != UnoObjToWrapperMap.end())
        {
            xIntWrapper =  it_uno->second;
            if (xIntWrapper.is())
            {
                convertSelfToCom(xIntWrapper, pVar);
                return;
            }
        }
        // Is the object a COM wrapper ( either XInvocation, or Adapter object)
        // or does it supply an IDispatch by its own ?
        else
        {
            Reference<XInterface> xIntComWrapper = xInt;
            typedef std::unordered_map<sal_uIntPtr,sal_uIntPtr>::iterator IT;
            // Adapter? then get the COM wrapper to which the adapter delegates its calls
            IT it= AdapterToWrapperMap.find( reinterpret_cast<sal_uIntPtr>(xInt.get()));
            if( it != AdapterToWrapperMap.end() )
                xIntComWrapper= reinterpret_cast<XInterface*>(it->second);

            if (convertSelfToCom(xIntComWrapper, pVar))
                return;
        }
    }
    // If we have no UNO wrapper nor the IDispatch yet then we have to create
    // a wrapper. For that we need an XInvocation from the UNO object.

    // get an XInvocation or create one using the invocation service
    Reference<XInvocation> xInv(xInt, UNO_QUERY);
    if ( ! xInv.is())
    {
        Reference<XSingleServiceFactory> xInvFactory= getInvocationFactory(rObj);
        if (xInvFactory.is())
        {
            Sequence<Any> params(1);
            params.getArray()[0] = rObj;
            Reference<XInterface> xInt2 = xInvFactory->createInstanceWithArguments(params);
            xInv.set(xInt2, UNO_QUERY);
        }
    }

    if (xInv.is())
    {
        Reference<XInterface> xNewWrapper = createUnoWrapperInstance();
        Reference<XInitialization> xInitWrapper(xNewWrapper, UNO_QUERY);
        if (xInitWrapper.is())
        {
            VARTYPE vartype= getVarType( rObj);

            if (xInt.is())
            {
                Any params[3];
                params[0] <<= xInv;
                params[1] <<= xInt;
                params[2] <<= vartype;
                xInitWrapper->initialize( Sequence<Any>(params, 3));
            }
            else
            {
                Any params[2];
                params[0] <<= xInv;
                params[1] <<= vartype;
                xInitWrapper->initialize( Sequence<Any>(params, 2));
            }

            // put the newly created object into a map. If the same object will
            // be mapped again and there is already a wrapper then the old wrapper
            // will be used.
            if(xInt.is()) // only interfaces
                UnoObjToWrapperMap[reinterpret_cast<sal_uIntPtr>(xInt.get())]= xNewWrapper;
            convertSelfToCom(xNewWrapper, pVar);
            return;
        }
    }
}

template<class T>
void UnoConversionUtilities<T>::variantToAny( const VARIANT* pVariant, Any& rAny,
                                                  bool bReduceValueRange /* = sal_True */)
{
    HRESULT hr = S_OK;
    try
    {
        CComVariant var;

        // There is no need to support indirect values, since they're not supported by UNO
        if( FAILED(hr= VariantCopyInd( &var, pVariant))) // remove VT_BYREF
            throw BridgeRuntimeError(
                      "[automation bridge] UnoConversionUtilities<T>::variantToAny \n"
                      "VariantCopyInd failed for reason : " + OUString::number(hr));

        if ( ! convertValueObject( & var, rAny))
        {
            if ((var.vt & VT_ARRAY) > 0)
            {
                VARTYPE oleTypeFlags = ::sal::static_int_cast< VARTYPE, int >( var.vt ^ VT_ARRAY );

                Sequence<Any> unoSeq = createOleArrayWrapper(var.parray, oleTypeFlags);
                rAny.setValue( &unoSeq, cppu::UnoType<decltype(unoSeq)>::get());
            }
            else
            {
                switch (var.vt)
                {
                case VT_EMPTY:
                    rAny.setValue(nullptr, Type());
                    break;
                case VT_NULL:
                    rAny.setValue(nullptr, Type());
                    break;
                case VT_I2:
                    rAny.setValue( & var.iVal, cppu::UnoType<sal_Int16>::get());
                    break;
                case VT_I4:
                    rAny.setValue( & var.lVal, cppu::UnoType<sal_Int32>::get());
                    // necessary for use in JavaScript ( see "reduceRange")
                    if( bReduceValueRange)
                        reduceRange(rAny);
                    break;
                case VT_R4:
                    rAny.setValue( & var.fltVal, cppu::UnoType<float>::get());
                    break;
                case VT_R8:
                    rAny.setValue(& var.dblVal, cppu::UnoType<double>::get());
                    break;
                case VT_CY:
                {
                    Currency cy(var.cyVal.int64);
                    rAny <<= cy;
                    break;
                }
                case VT_DATE:
                {
                    Date d(var.date);
                    rAny <<= d;
                    break;
                }
                case VT_BSTR:
                {
                    OUString b(reinterpret_cast<const sal_Unicode*>(var.bstrVal));
                    rAny.setValue( &b, cppu::UnoType<decltype(b)>::get());
                    break;
                }
                case VT_UNKNOWN:
                case VT_DISPATCH:
                {
                    //check if it is a UNO type
                    CComQIPtr<IUnoTypeWrapper> spType(static_cast<IUnknown*>(var.byref));
                    if (spType)
                    {
                        CComBSTR sName;
                        if (FAILED(spType->get_Name(&sName)))
                            throw BridgeRuntimeError(
                                    "[automation bridge]UnoConversionUtilities<T>::variantToAny \n"
                                    "Failed to get the type name from a UnoTypeWrapper!");
                        Type type;
                        if (!getType(sName, type))
                        {
                            throw CannotConvertException(
                                      "[automation bridge]UnoConversionUtilities<T>::variantToAny \n"
                                      "A UNO type with the name: " + OUString(reinterpret_cast<const sal_Unicode*>(LPCOLESTR(sName))) +
                                "does not exist!",
                                nullptr, TypeClass_UNKNOWN, FailReason::TYPE_NOT_SUPPORTED,0);
                        }
                        rAny <<= type;
                    }
                    else
                    {
                        rAny = createOleObjectWrapper( & var);
                    }
                    break;
                }
                case VT_ERROR:
                {
                    SCode scode(var.scode);
                    rAny <<= scode;
                    break;
                }
                case VT_BOOL:
                {
                    rAny <<= (var.boolVal == VARIANT_TRUE);
                    break;
                }
                case VT_I1:
                    rAny.setValue( & var.cVal, cppu::UnoType<sal_Int8>::get());
                    break;
                case VT_UI1: // there is no unsigned char in UNO
                    rAny <<= sal_Int8(var.bVal);
                    break;
                case VT_UI2:
                    rAny.setValue( & var.uiVal, cppu::UnoType<cppu::UnoUnsignedShortType>::get() );
                    break;
                case VT_UI4:
                    rAny.setValue( & var.ulVal, cppu::UnoType<sal_uInt32>::get());
                    break;
                case VT_INT:
                    rAny.setValue( & var.intVal, cppu::UnoType<sal_Int32>::get());
                    break;
                case VT_UINT:
                    rAny.setValue( & var.uintVal, cppu::UnoType<sal_uInt32>::get());
                    break;
                case VT_VOID:
                    rAny.setValue( nullptr, Type());
                    break;
                case VT_DECIMAL:
                {
                    Decimal dec;
                    dec.Scale = var.decVal.scale;
                    dec.Sign = var.decVal.sign;
                    dec.LowValue = var.decVal.Lo32;
                    dec.MiddleValue = var.decVal.Mid32;
                    dec.HighValue = var.decVal.Hi32;
                    rAny <<= dec;
                    break;
                }

                default:
                    break;
                }
            }
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
    catch (const BridgeRuntimeError &)
    {
         throw;
    }
    catch (const Exception & e)
    {
        throw BridgeRuntimeError("[automation bridge] unexpected exception in "
                                 "UnoConversionUtilities<T>::variantToAny ! Message : \n" +
                               e.Message);
    }
    catch(...)
    {
        throw BridgeRuntimeError(
                  "[automation bridge] unexpected exception in "
                  "UnoConversionUtilities<T>::variantToAny !");
    }

}
// The function converts an IUnknown* into an UNO interface or struct. The
// IUnknown pointer can constitute different kind of objects:
// 1. a wrapper of an UNO struct (the wrapper was created by this bridge)
// 2. a wrapper of an UNO interface (created by this bridge)
// 3. a dispatch object that implements UNO interfaces
// 4. a dispatch object.

// If the parameter "aType" has a value then the COM object ( pUnknown) is supposed to
// implement the interface described by "aType". Moreover it ( pUnknown) can implement
// several other
// UNO interfaces in which case it has to support the SUPPORTED_INTERFACES_PROP (see
// #define) property. That property contains all names of interfaces.
// "pUnknown" is wrapped by a COM wrapper object that implements XInvocation, e.g.
// IUnknownWrapper_Impl. Additionally an object of type "aType" is created by help
// of the INTERFACE_ADAPTER_FACTORY (see #define) service. The implementation of
// "aType" calls on the COM wrapper's XInvocation::invoke. If the COM object supports
// more than one UNO interfaces, as can be determined by the property
// SUPPORTED_INTERFACES_PROP, then the INTERFACE_ADAPTER_FACTORY creates an object that
// implements all these interfaces.
// This is only done if "pUnknown" is not already a UNO wrapper,
// that is it is actually NOT an UNO object that was converted to a COM object. If it is an
// UNO wrapper than the original UNO object is being extracted, queried for "aType" (if
// it is no struct) and returned.
template<class T>
Any UnoConversionUtilities<T>::createOleObjectWrapper(VARIANT* pVar, const Type& aType)
{
    //To allow passing "Nothing" in VS 2008 we need to accept VT_EMPTY
    if (pVar->vt != VT_UNKNOWN && pVar->vt != VT_DISPATCH && pVar->vt != VT_EMPTY)
        throw IllegalArgumentException(
                  "[automation bridge]UnoConversionUtilities<T>::createOleObjectWrapper \n"
                  "The VARIANT does not contain an object type! ", nullptr, -1);

    MutexGuard guard( getBridgeMutex());

    CComPtr<IUnknown>  spUnknown;
    CComPtr<IDispatch> spDispatch;

    if (pVar->vt == VT_UNKNOWN)
    {
        spUnknown = pVar->punkVal;
        if (spUnknown)
            spUnknown.QueryInterface( & spDispatch.p);
    }
    else if (pVar->vt == VT_DISPATCH && pVar->pdispVal != nullptr)
    {
        CComPtr<IDispatch> spDispatch2(pVar->pdispVal);
        if (spDispatch2)
            spDispatch2.QueryInterface( & spUnknown.p);
    }

    static Type VOID_TYPE= Type();
    Any ret;
    //If no Type is provided and pVar contains IUnknown then we return a XInterface.
    //If pVar contains an IDispatch then we return a XInvocation.
    Type desiredType = aType;

    if (aType == VOID_TYPE)
    {
        switch (pVar->vt)
        {
        case VT_EMPTY:
        case VT_UNKNOWN:
            desiredType = cppu::UnoType<XInterface>::get();
            break;
        case VT_DISPATCH:
            desiredType = cppu::UnoType<XInvocation>::get();
            break;
        default:
            desiredType = aType;
        }
    }

    // COM pointer are NULL, no wrapper required
     if (spUnknown == nullptr)
    {
        Reference<XInterface> xInt;
        if( aType.getTypeClass() == TypeClass_INTERFACE)
            ret.setValue( &xInt, aType);
        else if( aType.getTypeClass() == TypeClass_STRUCT)
            ret.setValue( nullptr, aType);
        else
            ret <<= xInt;
        return ret;
    }


    // Check if "spUnknown" is a UNO wrapper, that is an UNO object that has been
    // passed to COM. Then it supports IUnoObjectWrapper
    // and we extract the original UNO object.
    CComQIPtr<IUnoObjectWrapper> spUno( spUnknown);
    if( spUno)
    {   // it is a wrapper
         Reference<XInterface> xInt;
        if( SUCCEEDED( spUno->getOriginalUnoObject( &xInt)))
        {
            ret <<= xInt;
        }
        else
        {
            Any any;
            if( SUCCEEDED( spUno->getOriginalUnoStruct(&any)))
                ret= any;
        }
        return ret;
    }

    // "spUnknown" is a real COM object.
    // Before we create a new wrapper object we check if there is an existing wrapper
    // There can be two kinds of wrappers, those who wrap dispatch - UNO objects, and those who
    // wrap ordinary dispatch objects. The dispatch-UNO objects usually are adapted to represent
    // particular UNO interfaces.
    Reference<XInterface> xIntWrapper;
    CIT_Com cit_currWrapper= ComPtrToWrapperMap.find( reinterpret_cast<sal_uIntPtr>(spUnknown.p));
    if(cit_currWrapper != ComPtrToWrapperMap.end())
            xIntWrapper = cit_currWrapper->second;
    if (xIntWrapper.is())
    {
        //Try to find an adapter for the wrapper
        //find the proper Adapter. The pointer in the WrapperToAdapterMap are valid as long as
        //we get a pointer to the wrapper from ComPtrToWrapperMap, because the Adapter hold references
        //to the wrapper.
        CIT_Wrap it = WrapperToAdapterMap.find(reinterpret_cast<sal_uIntPtr>(xIntWrapper.get()));
        if (it == WrapperToAdapterMap.end())
        {
            // No adapter available.
            //The COM component could be a UNO object. Then we need to provide
            // a proxy  that implements all interfaces
            Sequence<Type> seqTypes= getImplementedInterfaces(spUnknown);
            Reference<XInterface> xIntAdapter;
            if (seqTypes.getLength() > 0)
            {
                //It is a COM UNO object
                xIntAdapter = createAdapter(seqTypes, xIntWrapper);
            }
            else
            {
                // Some ordinary COM object
                xIntAdapter = xIntWrapper;
            }
            // return the wrapper directly, return XInterface or XInvocation
            ret = xIntWrapper->queryInterface(desiredType);
            if ( ! ret.hasValue())
                throw IllegalArgumentException(
                          "[automation bridge]UnoConversionUtilities<T>::createOleObjectWrapper \n"
                          "The COM object is not suitable for the UNO type: " +
                    desiredType.getTypeName(), nullptr, -1);
        }
        else
        {
            //There is an adapter available
            Reference<XInterface> xIntAdapter(reinterpret_cast<XInterface*>(it->second));
            ret = xIntAdapter->queryInterface( desiredType);
            if ( ! ret.hasValue())
                throw IllegalArgumentException(
                          "[automation bridge]UnoConversionUtilities<T>::createOleObjectWrapper \n"
                          "The COM object is not suitable for the UNO type: " +
                    desiredType.getTypeName(), nullptr, -1);
        }

        return ret;
    }
    // No existing wrapper. Therefore create a new proxy.
    // If the object implements UNO interfaces then get the types.
    Sequence<Type> seqTypes = getImplementedInterfaces(spUnknown);
    if (seqTypes.getLength() == 0 &&
        aType != VOID_TYPE && aType != cppu::UnoType<XInvocation>::get())
    {
        seqTypes = Sequence<Type>( & aType, 1);
    }

    //There is no existing wrapper, therefore we create one for the real COM object
    Reference<XInterface> xIntNewProxy= createComWrapperInstance();
    if ( ! xIntNewProxy.is())
        throw BridgeRuntimeError(
                  "[automation bridge]UnoConversionUtilities<T>::createOleObjectWrapper \n"
                  "Could not create proxy object for COM object!");

    // initialize the COM wrapper
    Reference<XInitialization> xInit( xIntNewProxy, UNO_QUERY);
    OSL_ASSERT( xInit.is());

    Any  params[3];
    params[0] <<= reinterpret_cast<sal_uIntPtr>(spUnknown.p);
    params[1] <<= (pVar->vt == VT_DISPATCH);
    params[2] <<= seqTypes;

    xInit->initialize( Sequence<Any>( params, 3));
    ComPtrToWrapperMap[reinterpret_cast<sal_uInt64>(spUnknown.p)] = xIntNewProxy;

    // we have a wrapper object
    //The wrapper implements already XInvocation and XInterface. If
    //param aType is void then the object is supposed to have XInvocation.
     if (aType == cppu::UnoType<XInvocation>::get()||
         (aType == VOID_TYPE && seqTypes.getLength() == 0 ))
     {
         ret = xIntNewProxy->queryInterface(desiredType);
     }
     else
     {
         Reference<XInterface> xIntAdapter =
             createAdapter(seqTypes, xIntNewProxy);
         ret = xIntAdapter->queryInterface(desiredType);
     }
    return ret;
}
template<class T>
Reference<XInterface> UnoConversionUtilities<T>::createAdapter(const Sequence<Type>& seqTypes,
                                    const Reference<XInterface>& receiver)
{
    Reference< XInterface> xIntAdapterFac;
    xIntAdapterFac= m_smgr->createInstance(INTERFACE_ADAPTER_FACTORY);
    // We create an adapter object that does not only implement the required type but also
    // all types that the COM object pretends to implement. An COM object must therefore
    // support the property "_implementedInterfaces".
    Reference<XInterface> xIntAdapted;
    Reference<XInvocation> xInv(receiver, UNO_QUERY);
    Reference<XInvocationAdapterFactory2> xAdapterFac( xIntAdapterFac, UNO_QUERY);
    if( xAdapterFac.is())
        xIntAdapted= xAdapterFac->createAdapter( xInv, seqTypes);

    if( xIntAdapted.is())
    {
        // Put the pointer to the wrapper object and the interface pointer of the adapted interface
        // in a global map. Thus we can determine in a call to createUnoObjectWrapper whether the UNO
        // object is a wrapped COM object. In that case we extract the original COM object rather than
        // creating a wrapper around the UNO object.
        typedef std::unordered_map<sal_uInt64,sal_uInt64>::value_type VALUE;
        AdapterToWrapperMap.insert( VALUE( reinterpret_cast<sal_uInt64>(xIntAdapted.get()), reinterpret_cast<sal_uInt64>(receiver.get())));
        WrapperToAdapterMap.insert( VALUE( reinterpret_cast<sal_uInt64>(receiver.get()), reinterpret_cast<sal_uInt64>(xIntAdapted.get())));
    }
    else
    {
        throw BridgeRuntimeError(
                  "[automation bridge]UnoConversionUtilities<T>::createOleObjectWrapper \n"
                  "Could not create a proxy for COM object! Creation of adapter failed.");
    }
    return xIntAdapted;
}
// "convertValueObject" converts a JScriptValue object contained in "var" into
// an any. The type contained in the any is stipulated by a "type value" thas
// was set within the JScript script on the value object ( see JScriptValue).
template<class T>
bool UnoConversionUtilities<T>::convertValueObject( const VARIANTARG *var, Any& any)
{
    bool ret = false;
    try
    {
        bool bFail = false;
        HRESULT hr= S_OK;
        CComVariant varDisp;

        if(SUCCEEDED(hr = varDisp.ChangeType( VT_DISPATCH, var)))
        {
            CComPtr <IJScriptValueObject> spValue;
            VARIANT_BOOL varBool;
            CComBSTR bstrType;
            CComVariant varValue;
            CComPtr<IDispatch> spDisp( varDisp.pdispVal);
            if(spDisp)
            {
                if(SUCCEEDED( spDisp->QueryInterface( __uuidof( IJScriptValueObject),
                                                       reinterpret_cast<void**> (&spValue))))
                {
                    ret = true; // is a ValueObject
                    //If it is an out - param then it does not need to be converted. In/out and
                    // in params does so.
                    if (SUCCEEDED(hr= spValue->IsOutParam( &varBool)))
                    {
                        // if varBool == true then no conversion needed because out param
                        if (varBool == VARIANT_FALSE)
                        {
                            if(SUCCEEDED(hr = spValue->GetValue( & bstrType, & varValue)))
                            {
                                Type type;
                                if (getType(bstrType, type))
                                    variantToAny( & varValue, any, type);
                                else
                                    bFail = true;
                            }
                            else
                                bFail = true;
                        }
                    }
                    else
                        bFail = true;
                }
            }
        }
        else if( hr != DISP_E_TYPEMISMATCH && hr != E_NOINTERFACE)
            bFail = true;

        if (bFail)
            throw BridgeRuntimeError(
                "[automation bridge] Conversion of ValueObject failed ");
    }
    catch (const BridgeRuntimeError &)
    {
         throw;
    }
    catch (const Exception & e)
    {
        throw BridgeRuntimeError("[automation bridge] unexpected exception in "
                                 "UnoConversionUtilities<T>::convertValueObject ! Message : \n" +
                               e.Message);
    }
    catch(...)
    {
        throw BridgeRuntimeError(
                  "[automation bridge] unexpected exception in "
                  "UnoConversionUtilities<T>::convertValueObject !");
    }
    return ret;
}

template<class T>
void UnoConversionUtilities<T>::dispatchExObject2Sequence( const VARIANTARG* pvar, Any& anySeq, const Type& type)
{
    try
    {
        bool bFail = false;
        if( pvar->vt != VT_DISPATCH)
            throw BridgeRuntimeError("[automation bridge] UnoConversionUtilities<T>::dispatchExObject2Sequence \n"
                                     "Conversion of dispatch object to Sequence failed!");
        IDispatchEx* pdispEx;
        HRESULT hr;
        if( FAILED( hr= pvar->pdispVal->QueryInterface( IID_IDispatchEx,
                                                        reinterpret_cast<void**>( &pdispEx))))
            throw BridgeRuntimeError("[automation bridge] UnoConversionUtilities<T>::dispatchExObject2Sequence \n"
                                     "Conversion of dispatch object to Sequence failed!");

        DISPID dispid;
        DISPPARAMS param= {nullptr,nullptr,0,0};
        CComVariant result;

        OLECHAR const * sLength= L"length";

        // Get the length of the array. Can also be obtained throu GetNextDispID. The
        // method only returns DISPIDs of the array data. Their names are like "0", "1" etc.
        if( FAILED( hr= pdispEx->GetIDsOfNames(IID_NULL, const_cast<OLECHAR **>(&sLength), 1, LOCALE_USER_DEFAULT, &dispid)))
            throw BridgeRuntimeError("[automation bridge] UnoConversionUtilities<T>::dispatchExObject2Sequence \n"
                                     "Conversion of dispatch object to Sequence failed!");
        if( FAILED( hr= pdispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                          &param, &result, nullptr, nullptr)))
            throw BridgeRuntimeError("[automation bridge] UnoConversionUtilities<T>::dispatchExObject2Sequence \n"
                                     "Conversion of dispatch object to Sequence failed!");
        if( FAILED( VariantChangeType( &result, &result, 0, VT_I4)))
            throw BridgeRuntimeError("[automation bridge] UnoConversionUtilities<T>::dispatchExObject2Sequence \n"
                                     "Conversion of dispatch object to Sequence failed!");
        long length= result.lVal;

        result.Clear();

        // get a few basic facts about the sequence, and reallocate:
        // create the Sequences
        // get the size of the elements
        typelib_TypeDescription *pDesc= nullptr;
        type.getDescription( &pDesc);

        typelib_IndirectTypeDescription *pSeqDesc= reinterpret_cast<typelib_IndirectTypeDescription*>(pDesc);
        typelib_TypeDescriptionReference *pSeqElemDescRef= pSeqDesc->pType; // type of the Sequence' elements
        Type elemType( pSeqElemDescRef);
        _typelib_TypeDescription* pSeqElemDesc=nullptr;
        TYPELIB_DANGER_GET( &pSeqElemDesc, pSeqElemDescRef);
            sal_uInt32 nelementSize= pSeqElemDesc->nSize;
        TYPELIB_DANGER_RELEASE( pSeqElemDesc);

            uno_Sequence *p_uno_Seq;
        uno_sequence_construct( &p_uno_Seq, pDesc, nullptr, length, cpp_acquire);

        typelib_TypeClass typeElement= pSeqDesc->pType->eTypeClass;
        char *pArray= p_uno_Seq->elements;

        // Get All properties in the object, convert their values to the expected type and
        // put them into the passed in sequence
        for( sal_Int32 i= 0; i< length; i++)
        {
            OUString ousIndex=OUString::number( i);
            OLECHAR* sindex = const_cast<sal_Unicode *>(ousIndex.getStr());

            if( FAILED( hr= pdispEx->GetIDsOfNames(IID_NULL, &sindex , 1, LOCALE_USER_DEFAULT, &dispid)))
            {
                throw BridgeRuntimeError("[automation bridge] UnoConversionUtilities<T>::dispatchExObject2Sequence \n"
                                         "Conversion of dispatch object to Sequence failed!");
            }
            if( FAILED( hr= pdispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                              &param, &result, nullptr, nullptr)))
            {
                throw BridgeRuntimeError("[automation bridge] UnoConversionUtilities<T>::dispatchExObject2Sequence \n"
                                         "Conversion of dispatch object to Sequence failed!");
            }

            // If the result is VT_DISPATCH than the Sequence's element type could be Sequence
            // Look that up in the CoreReflection to make clear.
            // That requires a recursiv conversion
            Any any;
            // Destination address within the out-Sequence "anySeq" where to copy the next converted element
            void* pDest= pArray + (i * nelementSize);

            if( result.vt & VT_DISPATCH && typeElement == typelib_TypeClass_SEQUENCE)
            {
                variantToAny( &result, any, elemType, false);
                // copy the converted VARIANT, that is a Sequence to the Sequence
                uno_Sequence * p_unoSeq= *static_cast<uno_Sequence* const *>(any.getValue());
                // just copy the pointer of the uno_Sequence
                // nelementSize should be 4 !!!!
                memcpy( pDest, &p_unoSeq, nelementSize);
                osl_atomic_increment( &p_unoSeq->nRefCount);
            }
            else // Element type is no Sequence -> do one conversion
            {
                variantToAny( &result, any, elemType, false);
                if( typeElement == typelib_TypeClass_ANY)
                {
                    // copy the converted VARIANT to the Sequence
                    uno_type_assignData( pDest, pSeqElemDescRef , &any, pSeqElemDescRef,cpp_queryInterface,
                                         cpp_acquire, cpp_release);
                }
                else
                {
                    // type after conversion must be the element type of the sequence
                    OSL_ENSURE(any.getValueTypeClass() == css::uno::TypeClass(typeElement), "wrong conversion");
                    uno_type_assignData( pDest, pSeqElemDescRef,const_cast<void*>( any.getValue()), any.getValueTypeRef(),
                                         cpp_queryInterface, cpp_acquire, cpp_release);
                }
            }
        } // else
        result.Clear();
        anySeq.setValue( &p_uno_Seq, pDesc);
        uno_destructData( &p_uno_Seq, pDesc, cpp_release);
        typelib_typedescription_release( pDesc);

        if (bFail)
            throw BridgeRuntimeError(
                "[automation bridge] Conversion of ValueObject failed ");
    }
    catch (const BridgeRuntimeError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        throw BridgeRuntimeError("[automation bridge] unexpected exception in "
                                 "UnoConversionUtilities<T>::convertValueObject ! Message : \n" +
                                 e.Message);
    }
    catch(...)
    {
        throw BridgeRuntimeError(
                  "[automation bridge] unexpected exception in "
                  "UnoConversionUtilities<T>::convertValueObject !");
    }
}

/* The argument unotype is the type that is expected by the currently called UNO function.
   For example: []long, [][]long. If the function calls itself recursively then the element type
   is passed on. For example a two dimensional SAFEARRAY of type VT_I4 is to be converted. Then
   unotype has to be either void or [][]long. When the function calls itself recursivly then
   it passes the element type which is []long.
*/
template<class T>
Sequence<Any> UnoConversionUtilities<T>::createOleArrayWrapperOfDim(SAFEARRAY* pArray,
              unsigned int dimCount, unsigned int actDim, long* index, VARTYPE type, const Type& unotype)
{
    HRESULT hr= S_OK;
    long lBound;
    long uBound;
    long nCountElements;

    SafeArrayGetLBound(pArray, actDim, &lBound);
    SafeArrayGetUBound(pArray, actDim, &uBound);
    nCountElements= uBound - lBound +1;

    Sequence<Any>   anySeq(nCountElements);
    Any*            pUnoArray = anySeq.getArray();

    for (index[actDim - 1] = lBound; index[actDim - 1] <= uBound; index[actDim - 1]++)
    {
        if (actDim > 1 )
        {
            Sequence<Any> element = createOleArrayWrapperOfDim(pArray, dimCount,
                actDim - 1, index, type, getElementTypeOfSequence(unotype));

            pUnoArray[index[actDim - 1] - lBound].setValue(&element, cppu::UnoType<decltype(element)>::get());
        }
        else
        {
            VARIANT variant;

            VariantInit(&variant);

            V_VT(&variant) = type;

            switch (type)
            {
                case VT_I2:
                    SafeArrayGetElement(pArray, index, &V_I2(&variant));
                    break;
                case VT_I4:
                    SafeArrayGetElement(pArray, index, &V_I4(&variant));
                    break;
                case VT_R4:
                    SafeArrayGetElement(pArray, index, &V_R4(&variant));
                    break;
                case VT_R8:
                    SafeArrayGetElement(pArray, index, &V_R8(&variant));
                    break;
                case VT_CY:
                    SafeArrayGetElement(pArray, index, &V_CY(&variant));
                    break;
                case VT_DATE:
                    SafeArrayGetElement(pArray, index, &V_DATE(&variant));
                    break;
                case VT_BSTR:
                    hr= SafeArrayGetElement(pArray, index, &V_BSTR(&variant));
                    break;
                case VT_DISPATCH:
                    SafeArrayGetElement(pArray, index, &V_DISPATCH(&variant));
                    break;
                case VT_ERROR:
                    SafeArrayGetElement(pArray, index, &V_ERROR(&variant));
                    break;
                case VT_BOOL:
                    SafeArrayGetElement(pArray, index, &V_BOOL(&variant));
                    break;
                case VT_VARIANT:
                    SafeArrayGetElement(pArray, index, &variant);
                    break;
                case VT_UNKNOWN:
                    SafeArrayGetElement(pArray, index, &V_UNKNOWN(&variant));
                    break;
                case VT_I1:
                    SafeArrayGetElement(pArray, index, &V_I1(&variant));
                    break;
                case VT_UI1:
                    SafeArrayGetElement(pArray, index, &V_UI1(&variant));
                    break;
                case VT_UI2:
                    SafeArrayGetElement(pArray, index, &V_UI2(&variant));
                    break;
                case VT_UI4:
                    SafeArrayGetElement(pArray, index, &V_UI4(&variant));
                    break;
                default:
                    break;
            }

            if( unotype.getTypeClass() == TypeClass_VOID)
                // the function was called without specifying the destination type
                variantToAny(&variant, pUnoArray[index[actDim - 1] - lBound], false);
            else
                variantToAny(&variant, pUnoArray[index[actDim - 1] - lBound],
                    getElementTypeOfSequence(unotype), false);

            VariantClear(&variant);
        }
    }
    return anySeq;
}

template<class T>
Type UnoConversionUtilities<T>::getElementTypeOfSequence( const Type& seqType)
{
    Type retValue;
    if( seqType.getTypeClass() != TypeClass_VOID)
    {
        OSL_ASSERT( seqType.getTypeClass() == TypeClass_SEQUENCE);
        typelib_TypeDescription* pDescSeq= nullptr;
        seqType.getDescription(& pDescSeq);
        retValue = Type(reinterpret_cast<typelib_IndirectTypeDescription *>(pDescSeq)->pType);
        typelib_typedescription_release(pDescSeq);
    }
    return retValue;
}
template<class T>
Sequence<Any> UnoConversionUtilities<T>::createOleArrayWrapper(SAFEARRAY* pArray, VARTYPE type, const Type& unoType)
{
    sal_uInt32 dim = SafeArrayGetDim(pArray);

    Sequence<Any> ret;

    if (dim > 0)
    {
        std::unique_ptr<long[]> sarIndex(new long[dim]);
        long * index =  sarIndex.get();

        for (unsigned int i = 0; i < dim; i++)
        {
            index[i] = 0;
        }

        ret = createOleArrayWrapperOfDim(pArray, dim, dim, index, type, unoType);
    }

    return ret;
}

// If an VARIANT has the type VT_DISPATCH it can either be an JScript Array
// or some other object. This function finds out if it is such an array or
// not. Currently there's no way to make sure it's an array
// so we assume that when the object has a property "0" then it is an Array.
// An JScript has property like "0", "1", "2" etc. which represent the
// value at the corresponding index of the array
template<class T>
bool UnoConversionUtilities<T>::isJScriptArray(const VARIANT* rvar)
{
    OSL_ENSURE( rvar->vt == VT_DISPATCH, "param is not a VT_DISPATCH");
    HRESULT hr;
    OLECHAR const * sindex= L"0";
    DISPID id;
    if ( rvar->vt == VT_DISPATCH && rvar->pdispVal )
    {
        hr= rvar->pdispVal->GetIDsOfNames(
            IID_NULL, const_cast<OLECHAR **>(&sindex), 1, LOCALE_USER_DEFAULT,
            &id);

        if( SUCCEEDED ( hr) )
            return true;
    }

    return false;
}

template<class T>
VARTYPE UnoConversionUtilities<T>::mapTypeClassToVartype( TypeClass type)
{
    VARTYPE ret;
    switch( type)
    {
    case TypeClass_INTERFACE: ret= VT_DISPATCH;
        break;
    case TypeClass_STRUCT: ret= VT_DISPATCH;
        break;
    case TypeClass_ENUM: ret= VT_I4;
        break;
    case TypeClass_SEQUENCE: ret= VT_ARRAY;
        break;
    case TypeClass_ANY: ret= VT_VARIANT;
        break;
    case TypeClass_BOOLEAN: ret= VT_BOOL;
        break;
    case TypeClass_CHAR: ret= VT_I2;
        break;
    case TypeClass_STRING: ret= VT_BSTR;
        break;
    case TypeClass_FLOAT: ret= VT_R4;
        break;
    case TypeClass_DOUBLE: ret= VT_R8;
        break;
    case TypeClass_BYTE: ret= VT_UI1;
        break;
    case TypeClass_SHORT: ret= VT_I2;
        break;
    case TypeClass_LONG: ret= VT_I4;
        break;
    case TypeClass_UNSIGNED_SHORT: ret= VT_UI2;
         break;
    case TypeClass_UNSIGNED_LONG: ret= VT_UI4;
        break;
    default:
        ret= VT_EMPTY;
    }
    return ret;
}

template<class T>
Sequence<Type> UnoConversionUtilities<T>::getImplementedInterfaces(IUnknown* pUnk)
{
    Sequence<Type> seqTypes;
    CComDispatchDriver disp( pUnk);
    if( disp)
    {
        CComVariant var;
        HRESULT hr= S_OK;
        // There are two different property names possible.
        if( FAILED( hr= disp.GetPropertyByName( SUPPORTED_INTERFACES_PROP, &var)))
        {
            hr= disp.GetPropertyByName( SUPPORTED_INTERFACES_PROP2, &var);
        }
        if (SUCCEEDED( hr))
        {
            // we exspect an array( SafeArray or IDispatch) of Strings.
            Any anyNames;
            variantToAny( &var, anyNames, cppu::UnoType<Sequence<Any>>::get());
            Sequence<Any> seqAny;
            if( anyNames >>= seqAny)
            {
                seqTypes.realloc( seqAny.getLength());
                for( sal_Int32 i=0; i < seqAny.getLength(); i++)
                {
                    OUString typeName;
                    seqAny[i] >>= typeName;
                    seqTypes[i]= Type( TypeClass_INTERFACE, typeName);
                }
            }
        }
    }
    return seqTypes;
}
template<class T>
Reference<XTypeConverter> UnoConversionUtilities<T>::getTypeConverter()
{
    if ( ! m_typeConverter.is())
    {
        MutexGuard guard(getBridgeMutex());
        if ( ! m_typeConverter.is())
        {
            Reference<XInterface> xIntConverter =
                m_smgr->createInstance("com.sun.star.script.Converter");
            if (xIntConverter.is())
                m_typeConverter.set(xIntConverter, UNO_QUERY);
        }
    }
    return m_typeConverter;
}

// This function tries to the change the type of a value (contained in the Any)
// to the smallest possible that can hold the value. This is actually done only
// for types of VT_I4 (see o2u_variantToAny). The reason is the following:
// JavaScript passes integer values always as VT_I4. If there is a parameter or
// property of type any then the bridge converts the any's content according
// to "o2u_variantToAny". Because the VARTYPE is VT_I4 the value would be converted
// to TypeClass_LONG. Say the method XPropertySet::setPropertyValue( string name, any value)
// would be called on an object and the property actually is of TypeClass_SHORT.
// After conversion of the VARIANT parameter the Any would contain type
// TypeClass_LONG. Because the corereflection does not cast from long to short
// the "setPropertValue" would fail as the value has not the right type.

// The corereflection does convert small integer types to bigger types.
// Therefore we can reduce the type if possible and avoid the above mentioned
// problem.

// The function is not used when elements are to be converted for Sequences.

inline void reduceRange( Any& any)
{
    OSL_ASSERT( any.getValueTypeClass() == TypeClass_LONG);

    sal_Int32 value= *o3tl::doAccess<sal_Int32>(any);
    if( value <= 0x7f &&  value >= -0x80)
    {// -128 bis 127
        sal_Int8 charVal= static_cast<sal_Int8>( value);
        any.setValue( &charVal, cppu::UnoType<sal_Int8>::get());
    }
    else if( value <= 0x7fff && value >= -0x8000)
    {// -32768 bis 32767
        sal_Int16 shortVal= static_cast<sal_Int16>( value);
        any.setValue( &shortVal, cppu::UnoType<sal_Int16>::get());
    }
}

} // end namespace
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
