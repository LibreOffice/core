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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/errcode.hxx>
#include <svl/hint.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>

#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/script/NativeObjectWrapper.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/InvocationAdapterFactory.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/script/XDirectInvocation.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XIdlArray.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/bridge/oleautomation/NamedArgument.hpp>
#include <com/sun/star/bridge/oleautomation/Date.hpp>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <com/sun/star/bridge/oleautomation/Currency.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
#include <com/sun/star/script/XAutomationInvocation.hpp>
#include <basic/codecompletecache.hxx>

#include <rtlproto.hxx>

#include <basic/sbstar.hxx>
#include <basic/sbuno.hxx>
#include <basic/sberrors.hxx>
#include <sbunoobj.hxx>
#include "sbjsmod.hxx"
#include <basic/basmgr.hxx>
#include <sbintern.hxx>
#include <runtime.hxx>

#include <math.h>
#include <memory>
#include <unordered_map>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>

using com::sun::star::uno::Reference;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::container;
using namespace com::sun::star::bridge;
using namespace cppu;


// Identifiers for creating the strings for dbg_Properties
static char const ID_DBG_SUPPORTEDINTERFACES[] = "Dbg_SupportedInterfaces";
static char const ID_DBG_PROPERTIES[] = "Dbg_Properties";
static char const ID_DBG_METHODS[] = "Dbg_Methods";

static char const aSeqLevelStr[] = "[]";

// Gets the default property for an uno object. Note: There is some
// redirection built in. The property name specifies the name
// of the default property.

bool SbUnoObject::getDefaultPropName( SbUnoObject* pUnoObj, OUString& sDfltProp )
{
    bool bResult = false;
    Reference< XDefaultProperty> xDefaultProp( pUnoObj->maTmpUnoObj, UNO_QUERY );
    if ( xDefaultProp.is() )
    {
        sDfltProp = xDefaultProp->getDefaultPropertyName();
        if ( !sDfltProp.isEmpty() )
            bResult = true;
    }
    return bResult;
}

SbxVariable* getDefaultProp( SbxVariable* pRef )
{
    SbxVariable* pDefaultProp = nullptr;
    if ( pRef->GetType() == SbxOBJECT )
    {
        SbxObject* pObj = dynamic_cast<SbxObject*>(pRef);
        if (!pObj)
        {
            SbxBase* pObjVarObj = pRef->GetObject();
            pObj = dynamic_cast<SbxObject*>( pObjVarObj );
        }
        if (SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>(pObj))
        {
            pDefaultProp = pUnoObj->GetDfltProperty();
        }
    }
    return pDefaultProp;
}

void SetSbUnoObjectDfltPropName( SbxObject* pObj )
{
    SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>( pObj );
    if ( pUnoObj )
    {
        OUString sDfltPropName;

        if ( SbUnoObject::getDefaultPropName( pUnoObj, sDfltPropName ) )
        {
            pUnoObj->SetDfltProperty( sDfltPropName );
        }
    }
}

// save CoreReflection statically
Reference< XIdlReflection > getCoreReflection_Impl()
{
    return css::reflection::theCoreReflection::get(
        comphelper::getProcessComponentContext());
}

// save CoreReflection statically
Reference< XHierarchicalNameAccess > const & getCoreReflection_HierarchicalNameAccess_Impl()
{
    static Reference< XHierarchicalNameAccess > xCoreReflection_HierarchicalNameAccess;

    if( !xCoreReflection_HierarchicalNameAccess.is() )
    {
        Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
        if( xCoreReflection.is() )
        {
            xCoreReflection_HierarchicalNameAccess =
                Reference< XHierarchicalNameAccess >( xCoreReflection, UNO_QUERY );
        }
    }
    return xCoreReflection_HierarchicalNameAccess;
}

// Hold TypeProvider statically
Reference< XHierarchicalNameAccess > const & getTypeProvider_Impl()
{
    static Reference< XHierarchicalNameAccess > xAccess;

    // Do we have already CoreReflection; if not obtain it
    if( !xAccess.is() )
    {
        Reference< XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        if( xContext.is() )
        {
            xContext->getValueByName(
                "/singletons/com.sun.star.reflection.theTypeDescriptionManager" )
                    >>= xAccess;
            OSL_ENSURE( xAccess.is(), "### TypeDescriptionManager singleton not accessible!?" );
        }
        if( !xAccess.is() )
        {
            throw DeploymentException(
                    "/singletons/com.sun.star.reflection.theTypeDescriptionManager singleton not accessible" );
        }
    }
    return xAccess;
}

// Hold TypeConverter statically
Reference< XTypeConverter > const & getTypeConverter_Impl()
{
    static Reference< XTypeConverter > xTypeConverter;

    // Do we have already CoreReflection; if not obtain it
    if( !xTypeConverter.is() )
    {
        Reference< XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        if( xContext.is() )
        {
            xTypeConverter = Converter::create(xContext);
        }
        if( !xTypeConverter.is() )
        {
            throw DeploymentException(
                "com.sun.star.script.Converter service not accessible" );
        }
    }
    return xTypeConverter;
}


// #111851 factory function to create an OLE object
SbUnoObject* createOLEObject_Impl( const OUString& aType )
{
    static Reference< XMultiServiceFactory > xOLEFactory;
    static bool bNeedsInit = true;

    if( bNeedsInit )
    {
        bNeedsInit = false;

        Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
            xOLEFactory.set(
                xSMgr->createInstanceWithContext( "com.sun.star.bridge.OleObjectFactory", xContext ),
                UNO_QUERY );
        }
    }

    SbUnoObject* pUnoObj = nullptr;
    if( xOLEFactory.is() )
    {
        // some type names available in VBA can not be directly used in COM
        OUString aOLEType = aType;
        if ( aOLEType == "SAXXMLReader30" )
        {
            aOLEType = "Msxml2.SAXXMLReader.3.0";
        }
        Reference< XInterface > xOLEObject = xOLEFactory->createInstance( aOLEType );
        if( xOLEObject.is() )
        {
            pUnoObj = new SbUnoObject( aType, Any(xOLEObject) );
            OUString sDfltPropName;

            if ( SbUnoObject::getDefaultPropName( pUnoObj, sDfltPropName ) )
                pUnoObj->SetDfltProperty( sDfltPropName );
        }
    }
    return pUnoObj;
}


namespace
{
    void lcl_indent( OUStringBuffer& _inout_rBuffer, sal_Int32 _nLevel )
    {
        while ( _nLevel-- > 0 )
        {
            _inout_rBuffer.append( "  " );
        }
    }
}

void implAppendExceptionMsg( OUStringBuffer& _inout_rBuffer, const Exception& _e, const OUString& _rExceptionType, sal_Int32 _nLevel )
{
    _inout_rBuffer.append( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.append( "Type: " );

    if ( _rExceptionType.isEmpty() )
        _inout_rBuffer.append( "Unknown" );
    else
        _inout_rBuffer.append( _rExceptionType );

    _inout_rBuffer.append( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.append( "Message: " );
    _inout_rBuffer.append( _e.Message );

}

// construct an error message for the exception
OUString implGetExceptionMsg( const Exception& e, const OUString& aExceptionType_ )
{
    OUStringBuffer aMessageBuf;
    implAppendExceptionMsg( aMessageBuf, e, aExceptionType_, 0 );
    return aMessageBuf.makeStringAndClear();
}

OUString implGetExceptionMsg( const Any& _rCaughtException )
{
    auto e = o3tl::tryAccess<Exception>(_rCaughtException);
    OSL_PRECOND( e, "implGetExceptionMsg: illegal argument!" );
    if ( !e )
    {
        return OUString();
    }
    return implGetExceptionMsg( *e, _rCaughtException.getValueTypeName() );
}

Any convertAny( const Any& rVal, const Type& aDestType )
{
    Any aConvertedVal;
    Reference< XTypeConverter > xConverter = getTypeConverter_Impl();
    try
    {
        aConvertedVal = xConverter->convertTo( rVal, aDestType );
    }
    catch( const IllegalArgumentException& )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( ::cppu::getCaughtException() ) );
        return aConvertedVal;
    }
    catch( const CannotConvertException& e2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                          implGetExceptionMsg( e2, "com.sun.star.lang.IllegalArgumentException" ) );
        return aConvertedVal;
    }
    return aConvertedVal;
}


// #105565 Special Object to wrap a strongly typed Uno Any


// TODO: source out later
Reference<XIdlClass> TypeToIdlClass( const Type& rType )
{
    return getCoreReflection_Impl()->forName(rType.getTypeName());
}

// Exception type unknown
template< class EXCEPTION >
OUString implGetExceptionMsg( const EXCEPTION& e )
{
    return implGetExceptionMsg( e, cppu::UnoType<decltype(e)>::get().getTypeName() );
}

void implHandleBasicErrorException( BasicErrorException& e )
{
    SbError nError = StarBASIC::GetSfxFromVBError( (sal_uInt16)e.ErrorCode );
    StarBASIC::Error( nError, e.ErrorMessageArgument );
}

void implHandleWrappedTargetException( const Any& _rWrappedTargetException )
{
    Any aExamine( _rWrappedTargetException );

    // completely strip the first InvocationTargetException, its error message isn't of any
    // interest to the user, it just says something like "invoking the UNO method went wrong.".
    InvocationTargetException aInvocationError;
    if ( aExamine >>= aInvocationError )
        aExamine = aInvocationError.TargetException;

    BasicErrorException aBasicError;

    SbError nError( ERRCODE_BASIC_EXCEPTION );
    OUStringBuffer aMessageBuf;

    // strip any other WrappedTargetException instances, but this time preserve the error messages.
    WrappedTargetException aWrapped;
    sal_Int32 nLevel = 0;
    while ( aExamine >>= aWrapped )
    {
        // special handling for BasicErrorException errors
        if ( aWrapped.TargetException >>= aBasicError )
        {
            nError = StarBASIC::GetSfxFromVBError( (sal_uInt16)aBasicError.ErrorCode );
            aMessageBuf.append( aBasicError.ErrorMessageArgument );
            aExamine.clear();
            break;
        }

        // append this round's message
        implAppendExceptionMsg( aMessageBuf, aWrapped, aExamine.getValueTypeName(), nLevel );
        if ( aWrapped.TargetException.getValueTypeClass() == TypeClass_EXCEPTION )
            // there is a next chain element
            aMessageBuf.append( "\nTargetException:" );

        // next round
        aExamine = aWrapped.TargetException;
        ++nLevel;
    }

    if ( auto e = o3tl::tryAccess<Exception>(aExamine) )
    {
        // the last element in the chain is still an exception, but no WrappedTargetException
        implAppendExceptionMsg( aMessageBuf, *e, aExamine.getValueTypeName(), nLevel );
    }

    StarBASIC::Error( nError, aMessageBuf.makeStringAndClear() );
}

static void implHandleAnyException( const Any& _rCaughtException )
{
    BasicErrorException aBasicError;
    WrappedTargetException aWrappedError;

    if ( _rCaughtException >>= aBasicError )
    {
        implHandleBasicErrorException( aBasicError );
    }
    else if ( _rCaughtException >>= aWrappedError )
    {
        implHandleWrappedTargetException( _rCaughtException );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( _rCaughtException ) );
    }
}

// NativeObjectWrapper handling
struct ObjectItem
{
    SbxObjectRef    m_xNativeObj;

    explicit ObjectItem( SbxObject* pNativeObj )
        : m_xNativeObj( pNativeObj )
    {}
};

typedef std::vector< ObjectItem > NativeObjectWrapperVector;
class GaNativeObjectWrapperVector : public rtl::Static<NativeObjectWrapperVector, GaNativeObjectWrapperVector> {};

void clearNativeObjectWrapperVector()
{
    GaNativeObjectWrapperVector::get().clear();
}

static sal_uInt32 lcl_registerNativeObjectWrapper( SbxObject* pNativeObj )
{
    NativeObjectWrapperVector &rNativeObjectWrapperVector = GaNativeObjectWrapperVector::get();
    sal_uInt32 nIndex = rNativeObjectWrapperVector.size();
    rNativeObjectWrapperVector.push_back( ObjectItem( pNativeObj ) );
    return nIndex;
}

static SbxObject* lcl_getNativeObject( sal_uInt32 nIndex )
{
    SbxObjectRef xRetObj;
    NativeObjectWrapperVector &rNativeObjectWrapperVector = GaNativeObjectWrapperVector::get();
    if( nIndex < rNativeObjectWrapperVector.size() )
    {
        ObjectItem& rItem = rNativeObjectWrapperVector[ nIndex ];
        xRetObj = rItem.m_xNativeObj;
    }
    return xRetObj.get();
}

// convert from Uno to Sbx
SbxDataType unoToSbxType( TypeClass eType )
{
    SbxDataType eRetType = SbxVOID;

    switch( eType )
    {
        case TypeClass_INTERFACE:
        case TypeClass_TYPE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:       eRetType = SbxOBJECT;   break;

        case TypeClass_ENUM:            eRetType = SbxLONG;     break;
        case TypeClass_SEQUENCE:
            eRetType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            break;


        case TypeClass_ANY:             eRetType = SbxVARIANT;  break;
        case TypeClass_BOOLEAN:         eRetType = SbxBOOL;     break;
        case TypeClass_CHAR:            eRetType = SbxCHAR;     break;
        case TypeClass_STRING:          eRetType = SbxSTRING;   break;
        case TypeClass_FLOAT:           eRetType = SbxSINGLE;   break;
        case TypeClass_DOUBLE:          eRetType = SbxDOUBLE;   break;
        case TypeClass_BYTE:            eRetType = SbxINTEGER;  break;
        case TypeClass_SHORT:           eRetType = SbxINTEGER;  break;
        case TypeClass_LONG:            eRetType = SbxLONG;     break;
        case TypeClass_HYPER:           eRetType = SbxSALINT64; break;
        case TypeClass_UNSIGNED_SHORT:  eRetType = SbxUSHORT;   break;
        case TypeClass_UNSIGNED_LONG:   eRetType = SbxULONG;    break;
        case TypeClass_UNSIGNED_HYPER:  eRetType = SbxSALUINT64;break;
        default: break;
    }
    return eRetType;
}

SbxDataType unoToSbxType( const Reference< XIdlClass >& xIdlClass )
{
    SbxDataType eRetType = SbxVOID;
    if( xIdlClass.is() )
    {
        TypeClass eType = xIdlClass->getTypeClass();
        eRetType = unoToSbxType( eType );
    }
    return eRetType;
}

static void implSequenceToMultiDimArray( SbxDimArray*& pArray, Sequence< sal_Int32 >& indices, Sequence< sal_Int32 >& sizes, const Any& aValue, sal_Int32& dimension, bool bIsZeroIndex, Type* pType )
{
    const Type& aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();

    sal_Int32 dimCopy = dimension;

    if ( eTypeClass == TypeClass_SEQUENCE )
    {
        Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
        Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
        typelib_TypeDescription * pTD = nullptr;
        aType.getDescription( &pTD );
        Type aElementType( reinterpret_cast<typelib_IndirectTypeDescription *>(pTD)->pType );
        ::typelib_typedescription_release( pTD );

        sal_Int32 nLen = xIdlArray->getLen( aValue );
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            Any aElementAny = xIdlArray->get( aValue, (sal_uInt32)index );
            // This detects the dimension were currently processing
            if ( dimCopy == dimension )
            {
                ++dimCopy;
                if ( sizes.getLength() < dimCopy )
                {
                    sizes.realloc( sizes.getLength() + 1 );
                    sizes[ sizes.getLength() - 1 ] = nLen;
                    indices.realloc( indices.getLength() + 1 );
                }
            }

            if ( bIsZeroIndex )
                indices[ dimCopy - 1 ] = index;
            else
                indices[ dimCopy - 1] = index + 1;

            implSequenceToMultiDimArray( pArray, indices, sizes, aElementAny, dimCopy, bIsZeroIndex, &aElementType );
        }

    }
    else
    {
        if ( indices.getLength() < 1 )
        {
            // Should never ever get here ( indices.getLength()
            // should equal number of dimensions in the array )
            // And that should at least be 1 !
            // #QUESTION is there a better error?
            StarBASIC::Error( ERRCODE_BASIC_INVALID_OBJECT );
            return;
        }

        SbxDataType eSbxElementType = unoToSbxType( pType ? pType->getTypeClass() : aValue.getValueTypeClass() );
        if ( !pArray )
        {
            pArray = new SbxDimArray( eSbxElementType );
            sal_Int32 nIndexLen = indices.getLength();

            // Dimension the array
            for ( sal_Int32 index = 0; index < nIndexLen; ++index )
            {
                if ( bIsZeroIndex )
                    pArray->unoAddDim32( 0, sizes[ index ] - 1);
                else
                    pArray->unoAddDim32( 1, sizes[ index ] );

            }
        }

        if ( pArray )
        {
            auto xVar = tools::make_ref<SbxVariable>( eSbxElementType );
            unoToSbxValue( xVar.get(), aValue );

            sal_Int32* pIndices = indices.getArray();
            pArray->Put32(  xVar.get(), pIndices );

        }
    }
}

void unoToSbxValue( SbxVariable* pVar, const Any& aValue )
{
    const Type& aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();
    switch( eTypeClass )
    {
        case TypeClass_TYPE:
        {
            // Map Type to IdlClass
            Type aType_;
            aValue >>= aType_;
            Reference<XIdlClass> xClass = TypeToIdlClass( aType_ );
            Any aClassAny;
            aClassAny <<= xClass;

            // instantiate SbUnoObject
            OUString aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aClassAny );
            SbxObjectRef xWrapper = static_cast<SbxObject*>(pSbUnoObject);

            // If the object is invalid deliver null
            if( !pSbUnoObject->getUnoAny().hasValue() )
            {
                pVar->PutObject( nullptr );
            }
            else
            {
                pVar->PutObject( xWrapper.get() );
            }
        }
        break;
        // Interfaces and Structs must be wrapped in a SbUnoObject
        case TypeClass_INTERFACE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        {
            if( eTypeClass == TypeClass_STRUCT )
            {
                ArrayWrapper aWrap;
                NativeObjectWrapper aNativeObjectWrapper;
                if ( (aValue >>= aWrap) )
                {
                    SbxDimArray* pArray = nullptr;
                    Sequence< sal_Int32 > indices;
                    Sequence< sal_Int32 > sizes;
                    sal_Int32 dimension = 0;
                    implSequenceToMultiDimArray( pArray, indices, sizes, aWrap.Array, dimension, aWrap.IsZeroIndex, nullptr );
                    if ( pArray )
                    {
                        SbxDimArrayRef xArray = pArray;
                        SbxFlagBits nFlags = pVar->GetFlags();
                        pVar->ResetFlag( SbxFlagBits::Fixed );
                        pVar->PutObject( xArray.get() );
                        pVar->SetFlags( nFlags );
                    }
                    else
                        pVar->PutEmpty();
                    break;
                }
                else if ( (aValue >>= aNativeObjectWrapper) )
                {
                    sal_uInt32 nIndex = 0;
                    if( (aNativeObjectWrapper.ObjectId >>= nIndex) )
                    {
                        SbxObject* pObj = lcl_getNativeObject( nIndex );
                        pVar->PutObject( pObj );
                    }
                    else
                        pVar->PutEmpty();
                    break;
                }
                else
                {
                    SbiInstance* pInst = GetSbData()->pInst;
                    if( pInst && pInst->IsCompatibility() )
                    {
                        oleautomation::Date aDate;
                        if( (aValue >>= aDate) )
                        {
                            pVar->PutDate( aDate.Value );
                            break;
                        }
                        else
                        {
                            oleautomation::Decimal aDecimal;
                            if( (aValue >>= aDecimal) )
                            {
                                pVar->PutDecimal( aDecimal );
                                break;
                            }
                            else
                            {
                                oleautomation::Currency aCurrency;
                                if( (aValue >>= aCurrency) )
                                {
                                    pVar->PutCurrency( aCurrency.Value );
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            // instantiate a SbUnoObject
            OUString aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aValue );
            //If this is called externally e.g. from the scripting
            //framework then there is no 'active' runtime the default property will not be set up
            //only a vba object will have XDefaultProp set anyway so... this
            //test seems a bit of overkill
            //if ( SbiRuntime::isVBAEnabled() )
            {
                OUString sDfltPropName;

                if ( SbUnoObject::getDefaultPropName( pSbUnoObject, sDfltPropName ) )
                {
                    pSbUnoObject->SetDfltProperty( sDfltPropName );
                }
            }
            SbxObjectRef xWrapper = static_cast<SbxObject*>(pSbUnoObject);

            // If the object is invalid deliver null
            if( !pSbUnoObject->getUnoAny().hasValue() )
            {
                pVar->PutObject( nullptr );
            }
            else
            {
                pVar->PutObject( xWrapper.get() );
            }
        }
        break;


        case TypeClass_ENUM:
        {
            sal_Int32 nEnum = 0;
            enum2int( nEnum, aValue );
            pVar->PutLong( nEnum );
        }
            break;

        case TypeClass_SEQUENCE:
        {
            Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
            Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
            sal_Int32 i, nLen = xIdlArray->getLen( aValue );

            typelib_TypeDescription * pTD = nullptr;
            aType.getDescription( &pTD );
            OSL_ASSERT( pTD && pTD->eTypeClass == typelib_TypeClass_SEQUENCE );
            Type aElementType( reinterpret_cast<typelib_IndirectTypeDescription *>(pTD)->pType );
            ::typelib_typedescription_release( pTD );

            // build an Array in Basic
            SbxDimArrayRef xArray;
            SbxDataType eSbxElementType = unoToSbxType( aElementType.getTypeClass() );
            xArray = new SbxDimArray( eSbxElementType );
            if( nLen > 0 )
            {
                xArray->unoAddDim32( 0, nLen - 1 );

                // register the elements as variables
                for( i = 0 ; i < nLen ; i++ )
                {
                    // convert elements
                    Any aElementAny = xIdlArray->get( aValue, (sal_uInt32)i );
                    auto xVar = tools::make_ref<SbxVariable>( eSbxElementType );
                    unoToSbxValue( xVar.get(), aElementAny );

                    // put into the Array
                    xArray->Put32( xVar.get(), &i );
                }
            }
            else
            {
                xArray->unoAddDim( 0, -1 );
            }

            // return the Array
            SbxFlagBits nFlags = pVar->GetFlags();
            pVar->ResetFlag( SbxFlagBits::Fixed );
            pVar->PutObject( xArray.get() );
            pVar->SetFlags( nFlags );

        }
        break;


        case TypeClass_BOOLEAN:         pVar->PutBool( *o3tl::forceAccess<bool>(aValue) ); break;
        case TypeClass_CHAR:
        {
            pVar->PutChar( *o3tl::forceAccess<sal_Unicode>(aValue) );
            break;
        }
        case TypeClass_STRING:          { OUString val; aValue >>= val; pVar->PutString( val ); }  break;
        case TypeClass_FLOAT:           { float val = 0; aValue >>= val; pVar->PutSingle( val ); } break;
        case TypeClass_DOUBLE:          { double val = 0; aValue >>= val; pVar->PutDouble( val ); } break;
        case TypeClass_BYTE:            { sal_Int8 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        case TypeClass_SHORT:           { sal_Int16 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        case TypeClass_LONG:            { sal_Int32 val = 0; aValue >>= val; pVar->PutLong( val ); } break;
        case TypeClass_HYPER:           { sal_Int64 val = 0; aValue >>= val; pVar->PutInt64( val ); } break;
        case TypeClass_UNSIGNED_SHORT:  { sal_uInt16 val = 0; aValue >>= val; pVar->PutUShort( val ); } break;
        case TypeClass_UNSIGNED_LONG:   { sal_uInt32 val = 0; aValue >>= val; pVar->PutULong( val ); } break;
        case TypeClass_UNSIGNED_HYPER:  { sal_uInt64 val = 0; aValue >>= val; pVar->PutUInt64( val ); } break;
        default:                        pVar->PutEmpty();                       break;
    }
}

// Deliver the reflection for Sbx types
Type getUnoTypeForSbxBaseType( SbxDataType eType )
{
    Type aRetType = cppu::UnoType<void>::get();
    switch( eType )
    {
        case SbxNULL:       aRetType = cppu::UnoType<XInterface>::get(); break;
        case SbxINTEGER:    aRetType = cppu::UnoType<sal_Int16>::get(); break;
        case SbxLONG:       aRetType = cppu::UnoType<sal_Int32>::get(); break;
        case SbxSINGLE:     aRetType = cppu::UnoType<float>::get(); break;
        case SbxDOUBLE:     aRetType = cppu::UnoType<double>::get(); break;
        case SbxCURRENCY:   aRetType = cppu::UnoType<oleautomation::Currency>::get(); break;
        case SbxDECIMAL:    aRetType = cppu::UnoType<oleautomation::Decimal>::get(); break;
        case SbxDATE:       {
                            SbiInstance* pInst = GetSbData()->pInst;
                            if( pInst && pInst->IsCompatibility() )
                                aRetType = cppu::UnoType<double>::get();
                            else
                                aRetType = cppu::UnoType<oleautomation::Date>::get();
                            }
                            break;
        case SbxSTRING:     aRetType = cppu::UnoType<OUString>::get(); break;
        case SbxBOOL:       aRetType = cppu::UnoType<sal_Bool>::get(); break;
        case SbxVARIANT:    aRetType = cppu::UnoType<Any>::get(); break;
        case SbxCHAR:       aRetType = cppu::UnoType<cppu::UnoCharType>::get(); break;
        case SbxBYTE:       aRetType = cppu::UnoType<sal_Int8>::get(); break;
        case SbxUSHORT:     aRetType = cppu::UnoType<cppu::UnoUnsignedShortType>::get(); break;
        case SbxULONG:      aRetType = ::cppu::UnoType<sal_uInt32>::get(); break;
        // map machine-dependent ones to long for consistency
        case SbxINT:        aRetType = ::cppu::UnoType<sal_Int32>::get(); break;
        case SbxUINT:       aRetType = ::cppu::UnoType<sal_uInt32>::get(); break;
        default: break;
    }
    return aRetType;
}

// Converting of Sbx to Uno without a know target class for TypeClass_ANY
Type getUnoTypeForSbxValue( const SbxValue* pVal )
{
    Type aRetType = cppu::UnoType<void>::get();
    if( !pVal )
        return aRetType;

    // convert SbxType to Uno
    SbxDataType eBaseType = pVal->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = pVal->GetObject();
        if( !xObj.is() )
        {
            aRetType = cppu::UnoType<XInterface>::get();
            return aRetType;
        }

        if( auto pArray = dynamic_cast<SbxDimArray*>( xObj.get() ) )
        {
            short nDims = pArray->GetDims();
            Type aElementType = getUnoTypeForSbxBaseType( (SbxDataType)(pArray->GetType() & 0xfff) );
            TypeClass eElementTypeClass = aElementType.getTypeClass();

            // Normal case: One dimensional array
            sal_Int32 nLower, nUpper;
            if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
            {
                if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
                {
                    // If all elements of the arrays are from the same type, take
                    // this one - otherwise the whole will be considered as Any-Sequence
                    bool bNeedsInit = true;

                    sal_Int32 nSize = nUpper - nLower + 1;
                    sal_Int32 aIdx[1];
                    aIdx[0] = nLower;
                    for (sal_Int32 i = 0; i < nSize; ++i, ++aIdx[0])
                    {
                        SbxVariableRef xVar = pArray->Get32(aIdx);
                        Type aType = getUnoTypeForSbxValue( xVar.get() );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                // if only first element is void: different types  -> []any
                                // if all elements are void: []void is not allowed -> []any
                                aElementType = cppu::UnoType<Any>::get();
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = false;
                        }
                        else if( aElementType != aType )
                        {
                            // different types -> AnySequence
                            aElementType = cppu::UnoType<Any>::get();
                            break;
                        }
                    }
                }

                OUString aSeqTypeName = aSeqLevelStr + aElementType.getTypeName();
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
            }
            // #i33795 Map also multi dimensional arrays to corresponding sequences
            else if( nDims > 1 )
            {
                if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
                {
                    // For this check the array's dim structure does not matter
                    sal_uInt32 nFlatArraySize = pArray->Count32();

                    bool bNeedsInit = true;
                    for( sal_uInt32 i = 0 ; i < nFlatArraySize ; i++ )
                    {
                        SbxVariableRef xVar = pArray->SbxArray::Get32( i );
                        Type aType = getUnoTypeForSbxValue( xVar.get() );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                // if only first element is void: different types  -> []any
                                // if all elements are void: []void is not allowed -> []any
                                aElementType = cppu::UnoType<Any>::get();
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = false;
                        }
                        else if( aElementType != aType )
                        {
                            // different types -> AnySequence
                            aElementType = cppu::UnoType<Any>::get();
                            break;
                        }
                    }
                }

                OUStringBuffer aSeqTypeName;
                for( short iDim = 0 ; iDim < nDims ; iDim++ )
                {
                    aSeqTypeName.append(aSeqLevelStr);
                }
                aSeqTypeName.append(aElementType.getTypeName());
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName.makeStringAndClear() );
            }
        }
        // No array, but ...
        else if( auto obj = dynamic_cast<SbUnoObject*>( xObj.get() ) )
        {
            aRetType = obj->getUnoAny().getValueType();
        }
        // SbUnoAnyObject?
        else if( auto any = dynamic_cast<SbUnoAnyObject*>( xObj.get() ) )
        {
            aRetType = any->getValue().getValueType();
        }
        // Otherwise it is a No-Uno-Basic-Object -> default==deliver void
    }
    // No object, convert basic type
    else
    {
        aRetType = getUnoTypeForSbxBaseType( eBaseType );
    }
    return aRetType;
}

// converting of Sbx to Uno without known target class for TypeClass_ANY
Any sbxToUnoValueImpl( const SbxValue* pVar, bool bBlockConversionToSmallestType = false )
{
    SbxDataType eBaseType = pVar->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = pVar->GetObject();
        if( xObj.is() )
        {
            if( auto obj = dynamic_cast<SbUnoAnyObject*>( xObj.get() ) )
                return obj->getValue();
            if( auto pClassModuleObj = dynamic_cast<SbClassModuleObject*>( xObj.get() ) )
            {
                Any aRetAny;
                SbModule* pClassModule = pClassModuleObj->getClassModule();
                if( pClassModule->createCOMWrapperForIface( aRetAny, pClassModuleObj ) )
                    return aRetAny;
            }
            if( nullptr == dynamic_cast<const SbUnoObject*>( xObj.get() ) )
            {
                // Create NativeObjectWrapper to identify object in case of callbacks
                SbxObject* pObj = dynamic_cast<SbxObject*>( pVar->GetObject() );
                if( pObj != nullptr )
                {
                    NativeObjectWrapper aNativeObjectWrapper;
                    sal_uInt32 nIndex = lcl_registerNativeObjectWrapper( pObj );
                    aNativeObjectWrapper.ObjectId <<= nIndex;
                    Any aRetAny;
                    aRetAny <<= aNativeObjectWrapper;
                    return aRetAny;
                }
            }
        }
    }

    Type aType = getUnoTypeForSbxValue( pVar );
    TypeClass eType = aType.getTypeClass();

    if( !bBlockConversionToSmallestType )
    {
        // #79615 Choose "smallest" represention for int values
        // because up cast is allowed, downcast not
        switch( eType )
        {
            case TypeClass_FLOAT:
            case TypeClass_DOUBLE:
            {
                double d = pVar->GetDouble();
                if( rtl::math::approxEqual(d, floor( d )) )
                {
                    if( d >= -128 && d <= 127 )
                        aType = ::cppu::UnoType<sal_Int8>::get();
                    else if( d >= SbxMININT && d <= SbxMAXINT )
                        aType = ::cppu::UnoType<sal_Int16>::get();
                    else if( d >= -SbxMAXLNG && d <= SbxMAXLNG )
                        aType = ::cppu::UnoType<sal_Int32>::get();
                }
                break;
            }
            case TypeClass_SHORT:
            {
                sal_Int16 n = pVar->GetInteger();
                if( n >= -128 && n <= 127 )
                    aType = ::cppu::UnoType<sal_Int8>::get();
                break;
            }
            case TypeClass_LONG:
            {
                sal_Int32 n = pVar->GetLong();
                if( n >= -128 && n <= 127 )
                    aType = ::cppu::UnoType<sal_Int8>::get();
                else if( n >= SbxMININT && n <= SbxMAXINT )
                    aType = ::cppu::UnoType<sal_Int16>::get();
                break;
            }
            case TypeClass_UNSIGNED_SHORT:
            {
                sal_uInt16 n = pVar->GetUShort();
                if( n <= 255 )
                    aType = cppu::UnoType<sal_uInt8>::get();
                break;
            }
            case TypeClass_UNSIGNED_LONG:
            {
                sal_uInt32 n = pVar->GetLong();
                if( n <= 255 )
                    aType = cppu::UnoType<sal_uInt8>::get();
                else if( n <= SbxMAXUINT )
                    aType = cppu::UnoType<cppu::UnoUnsignedShortType>::get();
                break;
            }
            // TODO: need to add hyper types ?
            default: break;
        }
    }

    return sbxToUnoValue( pVar, aType );
}


// Helper function for StepREDIMP
static Any implRekMultiDimArrayToSequence( SbxDimArray* pArray,
    const Type& aElemType, short nMaxDimIndex, short nActualDim,
    sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
    sal_Int32 nSeqLevel = nMaxDimIndex - nActualDim + 1;
    OUStringBuffer aSeqTypeName;
    sal_Int32 i;
    for( i = 0 ; i < nSeqLevel ; i++ )
    {
        aSeqTypeName.append(aSeqLevelStr);
    }
    aSeqTypeName.append(aElemType.getTypeName());
    Type aSeqType( TypeClass_SEQUENCE, aSeqTypeName.makeStringAndClear() );

    // Create Sequence instance
    Any aRetVal;
    Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aSeqType );
    xIdlTargetClass->createObject( aRetVal );

    // Alloc sequence according to array bounds
    sal_Int32 nUpper = pUpperBounds[nActualDim];
    sal_Int32 nLower = pLowerBounds[nActualDim];
    sal_Int32 nSeqSize = nUpper - nLower + 1;
    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
    xArray->realloc( aRetVal, nSeqSize );

    sal_Int32& ri = pActualIndices[nActualDim];

    for( ri = nLower,i = 0 ; ri <= nUpper ; ri++,i++ )
    {
        Any aElementVal;

        if( nActualDim < nMaxDimIndex )
        {
            aElementVal = implRekMultiDimArrayToSequence( pArray, aElemType,
                nMaxDimIndex, nActualDim + 1, pActualIndices, pLowerBounds, pUpperBounds );
        }
        else
        {
            SbxVariable* pSource = pArray->Get32( pActualIndices );
            aElementVal = sbxToUnoValue( pSource, aElemType );
        }

        try
        {
            // transfer to the sequence
            xArray->set( aRetVal, i, aElementVal );
        }
        catch( const IllegalArgumentException& )
        {
            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                implGetExceptionMsg( ::cppu::getCaughtException() ) );
        }
        catch (const IndexOutOfBoundsException&)
        {
            StarBASIC::Error( ERRCODE_BASIC_OUT_OF_RANGE );
        }
    }
    return aRetVal;
}

// Map old interface
Any sbxToUnoValue( const SbxValue* pVar )
{
    return sbxToUnoValueImpl( pVar );
}

// function to find a global identifier in
// the UnoScope and to wrap it for Sbx
static bool implGetTypeByName( const OUString& rName, Type& rRetType )
{
    bool bSuccess = false;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            rRetType = Type( xTypeDesc->getTypeClass(), xTypeDesc->getName() );
            bSuccess = true;
        }
    }
    return bSuccess;
}


// converting of Sbx to Uno with known target class
Any sbxToUnoValue( const SbxValue* pVar, const Type& rType, Property* pUnoProperty )
{
    Any aRetVal;

    // #94560 No conversion of empty/void for MAYBE_VOID properties
    if( pUnoProperty && pUnoProperty->Attributes & PropertyAttribute::MAYBEVOID )
    {
        if( pVar->IsEmpty() )
            return aRetVal;
    }

    SbxDataType eBaseType = pVar->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = pVar->GetObject();
        if ( auto obj = dynamic_cast<SbUnoAnyObject*>( xObj.get() ) )
        {
            return obj->getValue();
        }
    }

    TypeClass eType = rType.getTypeClass();
    switch( eType )
    {
        case TypeClass_INTERFACE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        {
            Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );

            // null reference?
            if( pVar->IsNull() && eType == TypeClass_INTERFACE )
            {
                Reference< XInterface > xRef;
                OUString aClassName = xIdlTargetClass->getName();
                Type aClassType( xIdlTargetClass->getTypeClass(), aClassName.getStr() );
                aRetVal.setValue( &xRef, aClassType );
            }
            else
            {
                // #112368 Special conversion for Decimal, Currency and Date
                if( eType == TypeClass_STRUCT )
                {
                    SbiInstance* pInst = GetSbData()->pInst;
                    if( pInst && pInst->IsCompatibility() )
                    {
                        if( rType == cppu::UnoType<oleautomation::Decimal>::get())
                        {
                            oleautomation::Decimal aDecimal;
                            pVar->fillAutomationDecimal( aDecimal );
                            aRetVal <<= aDecimal;
                            break;
                        }
                        else if( rType == cppu::UnoType<oleautomation::Currency>::get())
                        {
                            // assumes per previous code that ole Currency is Int64
                            aRetVal <<= (sal_Int64)( pVar->GetInt64() );
                            break;
                        }
                        else if( rType == cppu::UnoType<oleautomation::Date>::get())
                        {
                            oleautomation::Date aDate;
                            aDate.Value = pVar->GetDate();
                            aRetVal <<= aDate;
                            break;
                        }
                    }
                }

                SbxBaseRef pObj = pVar->GetObject();
                if( auto obj = dynamic_cast<SbUnoObject*>( pObj.get() ) )
                {
                    aRetVal = obj->getUnoAny();
                }
                else if( auto structRef = dynamic_cast<SbUnoStructRefObject*>( pObj.get() ) )
                {
                    aRetVal = structRef->getUnoAny();
                }
                else
                {
                    // null object -> null XInterface
                    Reference<XInterface> xInt;
                    aRetVal <<= xInt;
                }
            }
        }
        break;

        case TypeClass_TYPE:
        {
            if( eBaseType == SbxOBJECT )
            {
                // XIdlClass?
                Reference< XIdlClass > xIdlClass;

                SbxBaseRef pObj = pVar->GetObject();
                if( auto obj = dynamic_cast<SbUnoObject*>( pObj.get() ) )
                {
                    Any aUnoAny = obj->getUnoAny();
                    aUnoAny >>= xIdlClass;
                }

                if( xIdlClass.is() )
                {
                    OUString aClassName = xIdlClass->getName();
                    Type aType( xIdlClass->getTypeClass(), aClassName.getStr() );
                    aRetVal <<= aType;
                }
            }
            else if( eBaseType == SbxSTRING )
            {
                OUString aTypeName = pVar->GetOUString();
                Type aType;
                bool bSuccess = implGetTypeByName( aTypeName, aType );
                if( bSuccess )
                {
                    aRetVal <<= aType;
                }
            }
        }
        break;


        case TypeClass_ENUM:
        {
            aRetVal = int2enum( pVar->GetLong(), rType );
        }
        break;

        case TypeClass_SEQUENCE:
        {
            SbxBaseRef xObj = pVar->GetObject();
            if( auto pArray = dynamic_cast<SbxDimArray*>( xObj.get() ) )
            {
                short nDims = pArray->GetDims();

                // Normal case: One dimensional array
                sal_Int32 nLower, nUpper;
                if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
                {
                    sal_Int32 nSeqSize = nUpper - nLower + 1;

                    // create the instance of the required sequence
                    Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );
                    xIdlTargetClass->createObject( aRetVal );
                    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
                    xArray->realloc( aRetVal, nSeqSize );

                    // Element-Type
                    OUString aClassName = xIdlTargetClass->getName();
                    typelib_TypeDescription * pSeqTD = nullptr;
                    typelib_typedescription_getByName( &pSeqTD, aClassName.pData );
                    OSL_ASSERT( pSeqTD );
                    Type aElemType( reinterpret_cast<typelib_IndirectTypeDescription *>(pSeqTD)->pType );

                    // convert all array member and register them
                    sal_Int32 aIdx[1];
                    aIdx[0] = nLower;
                    for (sal_Int32 i = 0 ; i < nSeqSize; ++i, ++aIdx[0])
                    {
                        SbxVariableRef xVar = pArray->Get32(aIdx);

                        // Convert the value of Sbx to Uno
                        Any aAnyValue = sbxToUnoValue( xVar.get(), aElemType );

                        try
                        {
                            // insert in the sequence
                            xArray->set( aRetVal, i, aAnyValue );
                        }
                        catch( const IllegalArgumentException& )
                        {
                            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                                implGetExceptionMsg( ::cppu::getCaughtException() ) );
                        }
                        catch (const IndexOutOfBoundsException&)
                        {
                            StarBASIC::Error( ERRCODE_BASIC_OUT_OF_RANGE );
                        }
                    }
                }
                // #i33795 Map also multi dimensional arrays to corresponding sequences
                else if( nDims > 1 )
                {
                    // Element-Type
                    typelib_TypeDescription * pSeqTD = nullptr;
                    Type aCurType( rType );
                    sal_Int32 nSeqLevel = 0;
                    Type aElemType;
                    do
                    {
                        OUString aTypeName = aCurType.getTypeName();
                        typelib_typedescription_getByName( &pSeqTD, aTypeName.pData );
                        OSL_ASSERT( pSeqTD );
                        if( pSeqTD->eTypeClass == typelib_TypeClass_SEQUENCE )
                        {
                            aCurType = Type( reinterpret_cast<typelib_IndirectTypeDescription *>(pSeqTD)->pType );
                            nSeqLevel++;
                        }
                        else
                        {
                            aElemType = aCurType;
                            break;
                        }
                    }
                    while( true );

                    if( nSeqLevel == nDims )
                    {
                        std::unique_ptr<sal_Int32[]> pLowerBounds(new sal_Int32[nDims]);
                        std::unique_ptr<sal_Int32[]> pUpperBounds(new sal_Int32[nDims]);
                        std::unique_ptr<sal_Int32[]> pActualIndices(new sal_Int32[nDims]);
                        for( short i = 1 ; i <= nDims ; i++ )
                        {
                            sal_Int32 lBound, uBound;
                            pArray->GetDim32( i, lBound, uBound );

                            short j = i - 1;
                            pActualIndices[j] = pLowerBounds[j] = lBound;
                            pUpperBounds[j] = uBound;
                        }

                        aRetVal = implRekMultiDimArrayToSequence( pArray, aElemType,
                            nDims - 1, 0, pActualIndices.get(), pLowerBounds.get(), pUpperBounds.get() );
                    }
                }
            }
        }
        break;


        // for Any use the class independent converting routine
        case TypeClass_ANY:
        {
            aRetVal = sbxToUnoValueImpl( pVar );
        }
        break;

        case TypeClass_BOOLEAN:
        {
            aRetVal <<= pVar->GetBool();
            break;
        }
        case TypeClass_CHAR:
        {
            aRetVal <<= pVar->GetChar();
            break;
        }
        case TypeClass_STRING:          aRetVal <<= pVar->GetOUString(); break;
        case TypeClass_FLOAT:           aRetVal <<= pVar->GetSingle(); break;
        case TypeClass_DOUBLE:          aRetVal <<= pVar->GetDouble(); break;

        case TypeClass_BYTE:
        {
            sal_Int16 nVal = pVar->GetInteger();
            bool bOverflow = false;
            if( nVal < -128 )
            {
                bOverflow = true;
                nVal = -128;
            }
            else if( nVal > 127 )
            {
                bOverflow = true;
                nVal = 127;
            }
            if( bOverflow )
                   StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );

            sal_Int8 nByteVal = (sal_Int8)nVal;
            aRetVal <<= nByteVal;
            break;
        }
        case TypeClass_SHORT:           aRetVal <<= (sal_Int16)( pVar->GetInteger() );  break;
        case TypeClass_LONG:            aRetVal <<= (sal_Int32)( pVar->GetLong() );     break;
        case TypeClass_HYPER:           aRetVal <<= (sal_Int64)( pVar->GetInt64() );    break;
        case TypeClass_UNSIGNED_SHORT:  aRetVal <<= (sal_uInt16)( pVar->GetUShort() );  break;
        case TypeClass_UNSIGNED_LONG:   aRetVal <<= (sal_uInt32)( pVar->GetULong() );   break;
        case TypeClass_UNSIGNED_HYPER:  aRetVal <<= (sal_uInt64)( pVar->GetUInt64() );  break;
        default: break;
    }

    return aRetVal;
}

void processAutomationParams( SbxArray* pParams, Sequence< Any >& args, bool bOLEAutomation, sal_uInt32 nParamCount )
{
    AutomationNamedArgsSbxArray* pArgNamesArray = nullptr;
    if( bOLEAutomation )
        pArgNamesArray = dynamic_cast<AutomationNamedArgsSbxArray*>( pParams );

    args.realloc( nParamCount );
    Any* pAnyArgs = args.getArray();
    bool bBlockConversionToSmallestType = GetSbData()->pInst->IsCompatibility();
    sal_uInt32 i = 0;
    if( pArgNamesArray )
    {
        Sequence< OUString >& rNameSeq = pArgNamesArray->getNames();
        OUString* pNames = rNameSeq.getArray();
        Any aValAny;
        for( i = 0 ; i < nParamCount ; i++ )
        {
            sal_uInt16 iSbx = (sal_uInt16)(i+1);

            aValAny = sbxToUnoValueImpl( pParams->Get( iSbx ),
            bBlockConversionToSmallestType );

            OUString aParamName = pNames[iSbx];
            if( !aParamName.isEmpty() )
            {
                oleautomation::NamedArgument aNamedArgument;
                aNamedArgument.Name = aParamName;
                aNamedArgument.Value = aValAny;
                pAnyArgs[i] <<= aNamedArgument;
            }
            else
            {
                pAnyArgs[i] = aValAny;
            }
        }
    }
    else
    {
        for( i = 0 ; i < nParamCount ; i++ )
        {
            pAnyArgs[i] = sbxToUnoValueImpl( pParams->Get( (sal_uInt16)(i+1) ),
            bBlockConversionToSmallestType );
        }
    }

}
enum INVOKETYPE
{
   GetProp = 0,
   SetProp,
   Func
};
Any invokeAutomationMethod( const OUString& Name, Sequence< Any >& args, SbxArray* pParams, sal_uInt32 nParamCount, Reference< XInvocation >& rxInvocation, INVOKETYPE invokeType )
{
    Sequence< sal_Int16 > OutParamIndex;
    Sequence< Any > OutParam;

    Any aRetAny;
    switch( invokeType )
    {
        case Func:
            aRetAny = rxInvocation->invoke( Name, args, OutParamIndex, OutParam );
            break;
        case GetProp:
            {
                Reference< XAutomationInvocation > xAutoInv( rxInvocation, UNO_QUERY );
                aRetAny = xAutoInv->invokeGetProperty( Name, args, OutParamIndex, OutParam );
                break;
            }
        case SetProp:
            {
                Reference< XAutomationInvocation > xAutoInv( rxInvocation, UNO_QUERY_THROW );
                aRetAny = xAutoInv->invokePutProperty( Name, args, OutParamIndex, OutParam );
                break;
            }
        default:
            break; // should introduce an error here

    }
    const sal_Int16* pIndices = OutParamIndex.getConstArray();
    sal_uInt32 nLen = OutParamIndex.getLength();
    if( nLen )
    {
        const Any* pNewValues = OutParam.getConstArray();
        for( sal_uInt32 j = 0 ; j < nLen ; j++ )
        {
            sal_Int16 iTarget = pIndices[ j ];
            if( iTarget >= (sal_Int16)nParamCount )
                break;
            unoToSbxValue( pParams->Get( (sal_uInt16)(j+1) ), pNewValues[ j ] );
        }
    }
    return aRetAny;
}

// Debugging help method to readout the imlemented interfaces of an object
OUString Impl_GetInterfaceInfo( const Reference< XInterface >& x, const Reference< XIdlClass >& xClass, sal_uInt16 nRekLevel )
{
    Type aIfaceType = cppu::UnoType<XInterface>::get();
    static Reference< XIdlClass > xIfaceClass = TypeToIdlClass( aIfaceType );

    OUStringBuffer aRetStr;
    for( sal_uInt16 i = 0 ; i < nRekLevel ; i++ )
        aRetStr.append( "    " );
    aRetStr.append( xClass->getName() );
    OUString aClassName = xClass->getName();
    Type aClassType( xClass->getTypeClass(), aClassName.getStr() );

    // checking if the interface is really supported
    if( !x->queryInterface( aClassType ).hasValue() )
    {
        aRetStr.append( " (ERROR: Not really supported!)\n" );
    }
    // Are there super interfaces?
    else
    {
        aRetStr.append( "\n" );

        // get the super interfaces
        Sequence< Reference< XIdlClass > > aSuperClassSeq = xClass->getSuperclasses();
        const Reference< XIdlClass >* pClasses = aSuperClassSeq.getConstArray();
        sal_uInt32 nSuperIfaceCount = aSuperClassSeq.getLength();
        for( sal_uInt32 j = 0 ; j < nSuperIfaceCount ; j++ )
        {
            const Reference< XIdlClass >& rxIfaceClass = pClasses[j];
            if( !rxIfaceClass->equals( xIfaceClass ) )
                aRetStr.append( Impl_GetInterfaceInfo( x, rxIfaceClass, nRekLevel + 1 ) );
        }
    }
    return aRetStr.makeStringAndClear();
}

OUString getDbgObjectNameImpl(SbUnoObject& rUnoObj)
{
    OUString aName = rUnoObj.GetClassName();
    if( aName.isEmpty() )
    {
        Any aToInspectObj = rUnoObj.getUnoAny();
        Reference< XInterface > xObj(aToInspectObj, css::uno::UNO_QUERY);
        if( xObj.is() )
        {
            Reference< XServiceInfo > xServiceInfo( xObj, UNO_QUERY );
            if( xServiceInfo.is() )
                aName = xServiceInfo->getImplementationName();
        }
    }
    return aName;
}

OUString getDbgObjectName(SbUnoObject& rUnoObj)
{
    OUString aName = getDbgObjectNameImpl(rUnoObj);
    if( aName.isEmpty() )
        aName += "Unknown";

    OUStringBuffer aRet;
    if( aName.getLength() > 20 )
    {
        aRet.append( "\n" );
    }
    aRet.append( "\"" );
    aRet.append( aName );
    aRet.append( "\":" );
    return aRet.makeStringAndClear();
}

OUString getBasicObjectTypeName( SbxObject* pObj )
{
    if (pObj)
    {
        if (SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>(pObj))
        {
            return getDbgObjectNameImpl(*pUnoObj);
        }
        else if (SbUnoStructRefObject* pUnoStructObj = dynamic_cast<SbUnoStructRefObject*>(pObj))
        {
            return pUnoStructObj->GetClassName();
        }
    }
    return OUString();
}

bool checkUnoObjectType(SbUnoObject& rUnoObj, const OUString& rClass)
{
    Any aToInspectObj = rUnoObj.getUnoAny();

    // Return true for XInvocation based objects as interface type names don't count then
    Reference< XInvocation > xInvocation( aToInspectObj, UNO_QUERY );
    if( xInvocation.is() )
    {
        return true;
    }
    bool bResult = false;
    Reference< XTypeProvider > xTypeProvider( aToInspectObj, UNO_QUERY );
    if( xTypeProvider.is() )
    {
        /*  Although interfaces in the ooo.vba namespace obey the IDL rules and
            have a leading 'X', in Basic we want to be able to do something
            like 'Dim wb As Workbooks' or 'Dim lb As MSForms.Label'. Here we
            add a leading 'X' to the class name and a leading dot to the entire
            type name. This results e.g. in '.XWorkbooks' or '.MSForms.XLabel'
            which matches the interface names 'ooo.vba.excel.XWorkbooks' or
            'ooo.vba.msforms.XLabel'.
         */
        OUString aClassName;
        if ( SbiRuntime::isVBAEnabled() )
        {
            aClassName = ".";
            sal_Int32 nClassNameDot = rClass.lastIndexOf( '.' );
            if( nClassNameDot >= 0 )
            {
                aClassName += rClass.copy( 0, nClassNameDot + 1 ) + "X" + rClass.copy( nClassNameDot + 1 );
            }
            else
            {
                aClassName += "X" + rClass;
            }
        }
        else // assume extended type declaration support for basic ( can't get here
             // otherwise.
            aClassName = rClass;

        Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
        const Type* pTypeArray = aTypeSeq.getConstArray();
        sal_uInt32 nIfaceCount = aTypeSeq.getLength();
        for( sal_uInt32 j = 0 ; j < nIfaceCount ; j++ )
        {
            const Type& rType = pTypeArray[j];

            Reference<XIdlClass> xClass = TypeToIdlClass( rType );
            if( !xClass.is() )
            {
                OSL_FAIL("failed to get XIdlClass for type");
                break;
            }
            OUString aInterfaceName = xClass->getName();
            if ( aInterfaceName == "com.sun.star.bridge.oleautomation.XAutomationObject" )
            {
                // there is a hack in the extensions/source/ole/oleobj.cxx  to return the typename of the automation object, lets check if it
                // matches
                Reference< XInvocation > xInv( aToInspectObj, UNO_QUERY );
                if ( xInv.is() )
                {
                    OUString sTypeName;
                    xInv->getValue( "$GetTypeName" ) >>= sTypeName;
                    if ( sTypeName.isEmpty() || sTypeName == "IDispatch" )
                    {
                        // can't check type, leave it pass
                        bResult = true;
                    }
                    else
                    {
                        bResult = sTypeName.equals( rClass );
                    }
                }
                break; // finished checking automation object
            }

            // match interface name with passed class name
            if ( (aClassName.getLength() <= aInterfaceName.getLength()) &&
                    aInterfaceName.endsWithIgnoreAsciiCase( aClassName ) )
            {
                bResult = true;
                break;
            }
        }
    }
    return bResult;
}

// Debugging help method to readout the imlemented interfaces of an object
OUString Impl_GetSupportedInterfaces(SbUnoObject& rUnoObj)
{
    Any aToInspectObj = rUnoObj.getUnoAny();

    // allow only TypeClass interface
    OUStringBuffer aRet;
    auto x = o3tl::tryAccess<Reference<XInterface>>(aToInspectObj);
    if( !x )
    {
        aRet.append( ID_DBG_SUPPORTEDINTERFACES );
        aRet.append( " not available.\n(TypeClass is not TypeClass_INTERFACE)\n" );
    }
    else
    {
        Reference< XTypeProvider > xTypeProvider( *x, UNO_QUERY );

        aRet.append( "Supported interfaces by object " );
        aRet.append(getDbgObjectName(rUnoObj));
        aRet.append( "\n" );
        if( xTypeProvider.is() )
        {
            // get the interfaces of the implementation
            Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
            const Type* pTypeArray = aTypeSeq.getConstArray();
            sal_uInt32 nIfaceCount = aTypeSeq.getLength();
            for( sal_uInt32 j = 0 ; j < nIfaceCount ; j++ )
            {
                const Type& rType = pTypeArray[j];

                Reference<XIdlClass> xClass = TypeToIdlClass( rType );
                if( xClass.is() )
                {
                    aRet.append( Impl_GetInterfaceInfo( *x, xClass, 1 ) );
                }
                else
                {
                    typelib_TypeDescription * pTD = nullptr;
                    rType.getDescription( &pTD );

                    aRet.append( "*** ERROR: No IdlClass for type \"" );
                    aRet.append( pTD->pTypeName );
                    aRet.append( "\"\n*** Please check type library\n" );
                }
            }
        }
    }
    return aRet.makeStringAndClear();
}


// Debugging help method SbxDataType -> String
OUString Dbg_SbxDataType2String( SbxDataType eType )
{
    OUStringBuffer aRet;
    switch( +eType )
    {
        case SbxEMPTY:      aRet.append("SbxEMPTY"); break;
        case SbxNULL:       aRet.append("SbxNULL"); break;
        case SbxINTEGER:    aRet.append("SbxINTEGER"); break;
        case SbxLONG:       aRet.append("SbxLONG"); break;
        case SbxSINGLE:     aRet.append("SbxSINGLE"); break;
        case SbxDOUBLE:     aRet.append("SbxDOUBLE"); break;
        case SbxCURRENCY:   aRet.append("SbxCURRENCY"); break;
        case SbxDECIMAL:    aRet.append("SbxDECIMAL"); break;
        case SbxDATE:       aRet.append("SbxDATE"); break;
        case SbxSTRING:     aRet.append("SbxSTRING"); break;
        case SbxOBJECT:     aRet.append("SbxOBJECT"); break;
        case SbxERROR:      aRet.append("SbxERROR"); break;
        case SbxBOOL:       aRet.append("SbxBOOL"); break;
        case SbxVARIANT:    aRet.append("SbxVARIANT"); break;
        case SbxDATAOBJECT: aRet.append("SbxDATAOBJECT"); break;
        case SbxCHAR:       aRet.append("SbxCHAR"); break;
        case SbxBYTE:       aRet.append("SbxBYTE"); break;
        case SbxUSHORT:     aRet.append("SbxUSHORT"); break;
        case SbxULONG:      aRet.append("SbxULONG"); break;
        case SbxSALINT64:   aRet.append("SbxINT64"); break;
        case SbxSALUINT64:  aRet.append("SbxUINT64"); break;
        case SbxINT:        aRet.append("SbxINT"); break;
        case SbxUINT:       aRet.append("SbxUINT"); break;
        case SbxVOID:       aRet.append("SbxVOID"); break;
        case SbxHRESULT:    aRet.append("SbxHRESULT"); break;
        case SbxPOINTER:    aRet.append("SbxPOINTER"); break;
        case SbxDIMARRAY:   aRet.append("SbxDIMARRAY"); break;
        case SbxCARRAY:     aRet.append("SbxCARRAY"); break;
        case SbxUSERDEF:    aRet.append("SbxUSERDEF"); break;
        case SbxLPSTR:      aRet.append("SbxLPSTR"); break;
        case SbxLPWSTR:     aRet.append("SbxLPWSTR"); break;
        case SbxCoreSTRING: aRet.append("SbxCoreSTRING"); break;
        case SbxOBJECT | SbxARRAY: aRet.append("SbxARRAY"); break;
        default: aRet.append("Unknown Sbx-Type!");break;
    }
    return aRet.makeStringAndClear();
}

// Debugging help method to display the properties of a SbUnoObjects
OUString Impl_DumpProperties(SbUnoObject& rUnoObj)
{
    OUStringBuffer aRet;
    aRet.append("Properties of object ");
    aRet.append(getDbgObjectName(rUnoObj));

    // analyse the Uno-Infos to recognise the arrays
    Reference< XIntrospectionAccess > xAccess = rUnoObj.getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = rUnoObj.getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.append( "\nUnknown, no introspection available\n" );
        return aRet.makeStringAndClear();
    }

    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    sal_uInt32 nUnoPropCount = props.getLength();
    const Property* pUnoProps = props.getConstArray();

    SbxArray* pProps = rUnoObj.GetProperties();
    sal_uInt16 nPropCount = pProps->Count();
    sal_uInt16 nPropsPerLine = 1 + nPropCount / 30;
    for( sal_uInt16 i = 0; i < nPropCount; i++ )
    {
        SbxVariable* pVar = pProps->Get( i );
        if( pVar )
        {
            OUStringBuffer aPropStr;
            if( (i % nPropsPerLine) == 0 )
                aPropStr.append( "\n" );

            // output the type and name
            // Is it in Uno a sequence?
            SbxDataType eType = pVar->GetFullType();

            bool bMaybeVoid = false;
            if( i < nUnoPropCount )
            {
                const Property& rProp = pUnoProps[ i ];

                // For MAYBEVOID freshly convert the type from Uno,
                // so not just SbxEMPTY is returned.
                if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
                {
                    eType = unoToSbxType( rProp.Type.getTypeClass() );
                    bMaybeVoid = true;
                }
                if( eType == SbxOBJECT )
                {
                    Type aType = rProp.Type;
                    if( aType.getTypeClass() == TypeClass_SEQUENCE )
                        eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
                }
            }
            aPropStr.append( Dbg_SbxDataType2String( eType ) );
            if( bMaybeVoid )
                aPropStr.append( "/void" );
            aPropStr.append( " " );
            aPropStr.append( pVar->GetName() );

            if( i == nPropCount - 1 )
                aPropStr.append( "\n" );
            else
                aPropStr.append( "; " );

            aRet.append( aPropStr.makeStringAndClear() );
        }
    }
    return aRet.makeStringAndClear();
}

// Debugging help method to display the methods of an SbUnoObjects
OUString Impl_DumpMethods(SbUnoObject& rUnoObj)
{
    OUStringBuffer aRet;
    aRet.append("Methods of object ");
    aRet.append(getDbgObjectName(rUnoObj));

    // XIntrospectionAccess, so that the types of the parameter could be outputted
    Reference< XIntrospectionAccess > xAccess = rUnoObj.getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = rUnoObj.getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.append( "\nUnknown, no introspection available\n" );
        return aRet.makeStringAndClear();
    }
    Sequence< Reference< XIdlMethod > > methods = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    const Reference< XIdlMethod >* pUnoMethods = methods.getConstArray();

    SbxArray* pMethods = rUnoObj.GetMethods();
    sal_uInt16 nMethodCount = pMethods->Count();
    if( !nMethodCount )
    {
        aRet.append( "\nNo methods found\n" );
        return aRet.makeStringAndClear();
    }
    sal_uInt16 nPropsPerLine = 1 + nMethodCount / 30;
    for( sal_uInt16 i = 0; i < nMethodCount; i++ )
    {
        SbxVariable* pVar = pMethods->Get( i );
        if( pVar )
        {
            if( (i % nPropsPerLine) == 0 )
                aRet.append( "\n" );

            // address the method
            const Reference< XIdlMethod >& rxMethod = pUnoMethods[i];

            // Is it in Uno a sequence?
            SbxDataType eType = pVar->GetFullType();
            if( eType == SbxOBJECT )
            {
                Reference< XIdlClass > xClass = rxMethod->getReturnType();
                if( xClass.is() && xClass->getTypeClass() == TypeClass_SEQUENCE )
                    eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            }
            // output the name and the type
            aRet.append( Dbg_SbxDataType2String( eType ) );
            aRet.append( " " );
            aRet.append ( pVar->GetName() );
            aRet.append( " ( " );

            // the get-method mustn't have a parameter
            Sequence< Reference< XIdlClass > > aParamsSeq = rxMethod->getParameterTypes();
            sal_uInt32 nParamCount = aParamsSeq.getLength();
            const Reference< XIdlClass >* pParams = aParamsSeq.getConstArray();

            if( nParamCount > 0 )
            {
                for( sal_uInt32 j = 0; j < nParamCount; j++ )
                {
                    aRet.append ( Dbg_SbxDataType2String( unoToSbxType( pParams[ j ] ) ) );
                    if( j < nParamCount - 1 )
                        aRet.append( ", " );
                }
            }
            else
                aRet.append( "void" );

            aRet.append( " ) " );

            if( i == nMethodCount - 1 )
                aRet.append( "\n" );
            else
                aRet.append( "; " );
        }
    }
    return aRet.makeStringAndClear();
}


// Implementation SbUnoObject
void SbUnoObject::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( bNeedIntrospection )
        doIntrospection();

    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoProperty* pProp = dynamic_cast<SbUnoProperty*>( pVar );
        SbUnoMethod* pMeth = dynamic_cast<SbUnoMethod*>( pVar );
        if( pProp )
        {
            bool bInvocation = pProp->isInvocationBased();
            if( pHint->GetId() == SfxHintId::BasicDataWanted )
            {
                // Test-Properties
                sal_Int32 nId = pProp->nId;
                if( nId < 0 )
                {
                    // Id == -1: Display implemented interfaces according the ClassProvider
                    if( nId == -1 )     // Property ID_DBG_SUPPORTEDINTERFACES"
                    {
                        OUString aRetStr = Impl_GetSupportedInterfaces(*this);
                        pVar->PutString( aRetStr );
                    }
                    // Id == -2: output properties
                    else if( nId == -2 )        // Property ID_DBG_PROPERTIES
                    {
                        // now all properties must be created
                        implCreateAll();
                        OUString aRetStr = Impl_DumpProperties(*this);
                        pVar->PutString( aRetStr );
                    }
                    // Id == -3: output the methods
                    else if( nId == -3 )        // Property ID_DBG_METHODS
                    {
                        // now all properties must be created
                        implCreateAll();
                        OUString aRetStr = Impl_DumpMethods(*this);
                        pVar->PutString( aRetStr );
                    }
                    return;
                }

                if( !bInvocation && mxUnoAccess.is() )
                {
                    try
                    {
                        if ( maStructInfo.get()  )
                        {
                            StructRefInfo aMember = maStructInfo->getStructMember( pProp->GetName() );
                            if ( aMember.isEmpty() )
                            {
                                 StarBASIC::Error( ERRCODE_BASIC_PROPERTY_NOT_FOUND );
                            }
                            else
                            {
                                if ( pProp->isUnoStruct() )
                                {
                                    SbUnoStructRefObject* pSbUnoObject = new SbUnoStructRefObject( pProp->GetName(), aMember );
                                    SbxObjectRef xWrapper = static_cast<SbxObject*>(pSbUnoObject);
                                    pVar->PutObject( xWrapper.get() );
                                }
                                else
                                {
                                    Any aRetAny = aMember.getValue();
                                    // take over the value from Uno to Sbx
                                    unoToSbxValue( pVar, aRetAny );
                                }
                                return;
                            }
                        }
                        // get the value
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( cppu::UnoType<XPropertySet>::get()), UNO_QUERY );
                        Any aRetAny = xPropSet->getPropertyValue( pProp->GetName() );
                        // The use of getPropertyValue (instead of using the index) is
                        // suboptimal, but the refactoring to XInvocation is already pending
                        // Otherwise it is possible to use FastPropertySet

                        // take over the value from Uno to Sbx
                        unoToSbxValue( pVar, aRetAny );
                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
                else if( bInvocation && mxInvocation.is() )
                {
                    try
                    {
                        sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
                        bool bCanBeConsideredAMethod = mxInvocation->hasMethod( pProp->GetName() );
                        Any aRetAny;
                        if ( bCanBeConsideredAMethod && nParamCount )
                        {
                            // Automation properties have methods, so.. we need to invoke this through
                            // XInvocation
                            Sequence<Any> args;
                            processAutomationParams( pParams, args, true, nParamCount );
                            aRetAny = invokeAutomationMethod( pProp->GetName(), args, pParams, nParamCount, mxInvocation, GetProp );
                        }
                        else
                            aRetAny = mxInvocation->getValue( pProp->GetName() );
                        // take over the value from Uno to Sbx
                        unoToSbxValue( pVar, aRetAny );
                        if( pParams && bCanBeConsideredAMethod )
                            pVar->SetParameters( nullptr );

                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
            }
            else if( pHint->GetId() == SfxHintId::BasicDataChanged )
            {
                if( !bInvocation && mxUnoAccess.is() )
                {
                    if( pProp->aUnoProp.Attributes & PropertyAttribute::READONLY )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_PROP_READONLY );
                        return;
                    }
                    if (  maStructInfo.get()  )
                    {
                        StructRefInfo aMember = maStructInfo->getStructMember( pProp->GetName() );
                        if ( aMember.isEmpty() )
                        {
                            StarBASIC::Error( ERRCODE_BASIC_PROPERTY_NOT_FOUND );
                        }
                        else
                        {
                            Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                            aMember.setValue( aAnyValue );
                        }
                        return;
                   }
                    // take over the value from Uno to Sbx
                    Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                    try
                    {
                        // set the value
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( cppu::UnoType<XPropertySet>::get()), UNO_QUERY );
                        xPropSet->setPropertyValue( pProp->GetName(), aAnyValue );
                        // The use of getPropertyValue (instead of using the index) is
                        // suboptimal, but the refactoring to XInvocation is already pending
                        // Otherwise it is possible to use FastPropertySet
                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
                else if( bInvocation && mxInvocation.is() )
                {
                    // take over the value from Uno to Sbx
                    Any aAnyValue = sbxToUnoValueImpl( pVar );
                    try
                    {
                        // set the value
                        mxInvocation->setValue( pProp->GetName(), aAnyValue );
                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
            }
        }
        else if( pMeth )
        {
            bool bInvocation = pMeth->isInvocationBased();
            if( pHint->GetId() == SfxHintId::BasicDataWanted )
            {
                // number of Parameter -1 because of Param0 == this
                sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
                Sequence<Any> args;
                bool bOutParams = false;
                sal_uInt32 i;

                if( !bInvocation && mxUnoAccess.is() )
                {
                    // get info
                    const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
                    const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
                    sal_uInt32 nUnoParamCount = rInfoSeq.getLength();
                    sal_uInt32 nAllocParamCount = nParamCount;

                    // ignore surplus parameter; alternative: throw an error
                    if( nParamCount > nUnoParamCount )
                    {
                        nParamCount = nUnoParamCount;
                        nAllocParamCount = nParamCount;
                    }
                    else if( nParamCount < nUnoParamCount )
                    {
                        SbiInstance* pInst = GetSbData()->pInst;
                        if( pInst && pInst->IsCompatibility() )
                        {
                            // Check types
                            bool bError = false;
                            for( i = nParamCount ; i < nUnoParamCount ; i++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[i];
                                const Reference< XIdlClass >& rxClass = rInfo.aType;
                                if( rxClass->getTypeClass() != TypeClass_ANY )
                                {
                                    bError = true;
                                    StarBASIC::Error( ERRCODE_BASIC_NOT_OPTIONAL );
                                }
                            }
                            if( !bError )
                                nAllocParamCount = nUnoParamCount;
                        }
                    }

                    if( nAllocParamCount > 0 )
                    {
                        args.realloc( nAllocParamCount );
                        Any* pAnyArgs = args.getArray();
                        for( i = 0 ; i < nParamCount ; i++ )
                        {
                            const ParamInfo& rInfo = pParamInfos[i];
                            const Reference< XIdlClass >& rxClass = rInfo.aType;

                            css::uno::Type aType( rxClass->getTypeClass(), rxClass->getName() );

                            // ATTENTION: Don't forget for Sbx-Parameter the offset!
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( (sal_uInt16)(i+1) ), aType );

                            // If it is not certain check whether the out-parameter are available.
                            if( !bOutParams )
                            {
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    bOutParams = true;
                            }
                        }
                    }
                }
                else if( bInvocation && pParams && mxInvocation.is() )
                {
                    bool bOLEAutomation = true;
                    processAutomationParams( pParams, args, bOLEAutomation, nParamCount );
                }

                // call the method
                GetSbData()->bBlockCompilerError = true;  // #106433 Block compiler errors for API calls
                try
                {
                    if( !bInvocation && mxUnoAccess.is() )
                    {
                        Any aRetAny = pMeth->m_xUnoMethod->invoke( getUnoAny(), args );

                        // take over the value from Uno to Sbx
                        unoToSbxValue( pVar, aRetAny );

                        // Did we to copy back the Out-Parameter?
                        if( bOutParams )
                        {
                            const Any* pAnyArgs = args.getConstArray();

                            // get info
                            const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
                            const ParamInfo* pParamInfos = rInfoSeq.getConstArray();

                            sal_uInt32 j;
                            for( j = 0 ; j < nParamCount ; j++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[j];
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    unoToSbxValue( pParams->Get( (sal_uInt16)(j+1) ), pAnyArgs[ j ] );
                            }
                        }
                    }
                    else if( bInvocation && mxInvocation.is() )
                    {
                        Any aRetAny = invokeAutomationMethod( pMeth->GetName(), args, pParams, nParamCount, mxInvocation, Func );
                        unoToSbxValue( pVar, aRetAny );
                        }

                    // remove parameter here, because this was not done anymore in unoToSbxValue()
                    // for arrays
                    if( pParams )
                        pVar->SetParameters( nullptr );
                }
                catch( const Exception& )
                {
                    implHandleAnyException( ::cppu::getCaughtException() );
                }
                GetSbData()->bBlockCompilerError = false;  // #106433 Unblock compiler errors
            }
        }
        else
            SbxObject::Notify( rBC, rHint );
    }
}


SbUnoObject::SbUnoObject( const OUString& aName_, const Any& aUnoObj_ )
    : SbxObject( aName_ )
    , bNeedIntrospection( true )
    , bNativeCOMObject( false )
{
    static Reference< XIntrospection > xIntrospection;

    // beat out again the default properties of Sbx
    Remove( "Name", SbxClassType::DontCare );
    Remove( "Parent", SbxClassType::DontCare );

    // check the type of the objects
    TypeClass eType = aUnoObj_.getValueType().getTypeClass();
    Reference< XInterface > x;
    if( eType == TypeClass_INTERFACE )
    {
        // get the interface from the Any
        aUnoObj_ >>= x;
        if( !x.is() )
            return;
    }

    Reference< XTypeProvider > xTypeProvider;
    // Did the object have an invocation itself?
    mxInvocation.set( x, UNO_QUERY );

    xTypeProvider.set( x, UNO_QUERY );

    if( mxInvocation.is() )
    {

        // get the ExactName
        mxExactNameInvocation.set( mxInvocation, UNO_QUERY );

        // The remainder refers only to the introspection
        if( !xTypeProvider.is() )
        {
            bNeedIntrospection = false;
            return;
        }

        // Ignore introspection based members for COM objects to avoid
        // hiding of equally named COM symbols, e.g. XInvocation::getValue
        Reference< oleautomation::XAutomationObject > xAutomationObject( aUnoObj_, UNO_QUERY );
        if( xAutomationObject.is() )
            bNativeCOMObject = true;
    }

    maTmpUnoObj = aUnoObj_;


    //*** Define the name ***
    bool bFatalError = true;

    // Is it an interface or a struct?
    bool bSetClassName = false;
    OUString aClassName_;
    if( eType == TypeClass_STRUCT || eType == TypeClass_EXCEPTION )
    {
        // Struct is Ok
        bFatalError = false;

        // insert the real name of the class
        if( aName_.isEmpty() )
        {
            aClassName_ = aUnoObj_.getValueType().getTypeName();
            bSetClassName = true;
        }
        StructRefInfo aThisStruct( maTmpUnoObj, maTmpUnoObj.getValueType(), 0 );
        maStructInfo.reset( new SbUnoStructRefObject( GetName(), aThisStruct ) );
    }
    else if( eType == TypeClass_INTERFACE )
    {
        // Interface works always through the type in the Any
        bFatalError = false;
    }
    if( bSetClassName )
        SetClassName( aClassName_ );

    // Neither interface nor Struct -> FatalError
    if( bFatalError )
    {
        StarBASIC::FatalError( ERRCODE_BASIC_EXCEPTION );
        return;
    }

    // pass the introspection primal on demand
}

SbUnoObject::~SbUnoObject()
{
}


// pass the introspection on Demand
void SbUnoObject::doIntrospection()
{
    if( !bNeedIntrospection )
        return;

    Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();

    if (!xContext.is())
        return;


    // get the introspection service
    Reference<XIntrospection> xIntrospection;

    try
    {
        xIntrospection = theIntrospection::get(xContext);
    }
    catch ( const css::uno::DeploymentException& )
    {
    }

    if (!xIntrospection.is())
        return;

    bNeedIntrospection = false;

    // pass the introspection
    try
    {
        mxUnoAccess = xIntrospection->inspect( maTmpUnoObj );
    }
    catch( const RuntimeException& e )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
    }

    if( !mxUnoAccess.is() )
    {
        // #51475 mark to indicate an invalid object (no mxMaterialHolder)
        return;
    }

    // get MaterialHolder from access
    mxMaterialHolder.set( mxUnoAccess, UNO_QUERY );

    // get ExactName from access
    mxExactName.set( mxUnoAccess, UNO_QUERY );
}


// Start of a list of all SbUnoMethod-Instances
static SbUnoMethod* pFirst = nullptr;

void clearUnoMethodsForBasic( StarBASIC* pBasic )
{
    SbUnoMethod* pMeth = pFirst;
    while( pMeth )
    {
        SbxObject* pObject = dynamic_cast< SbxObject* >( pMeth->GetParent() );
        if ( pObject )
        {
            StarBASIC* pModBasic = dynamic_cast< StarBASIC* >( pObject->GetParent() );
            if ( pModBasic == pBasic )
            {
                // for now the solution is to remove the method from the list and to clear it,
                // but in case the element should be correctly transferred to another StarBASIC,
                // we should either set module parent to NULL without clearing it, or even
                // set the new StarBASIC as the parent of the module
                // pObject->SetParent( NULL );

                if( pMeth == pFirst )
                    pFirst = pMeth->pNext;
                else if( pMeth->pPrev )
                    pMeth->pPrev->pNext = pMeth->pNext;
                if( pMeth->pNext )
                    pMeth->pNext->pPrev = pMeth->pPrev;

                pMeth->pPrev = nullptr;
                pMeth->pNext = nullptr;

                pMeth->SbxValue::Clear();
                pObject->SbxValue::Clear();

                // start from the beginning after object clearing, the cycle will end since the method is removed each time
                pMeth = pFirst;
            }
            else
                pMeth = pMeth->pNext;
        }
        else
            pMeth = pMeth->pNext;
    }
}

void clearUnoMethods()
{
    SbUnoMethod* pMeth = pFirst;
    while( pMeth )
    {
        pMeth->SbxValue::Clear();
        pMeth = pMeth->pNext;
    }
}


SbUnoMethod::SbUnoMethod
(
    const OUString& aName_,
    SbxDataType eSbxType,
    Reference< XIdlMethod > const & xUnoMethod_,
    bool bInvocation
)
    : SbxMethod( aName_, eSbxType )
    , mbInvocation( bInvocation )
{
    m_xUnoMethod = xUnoMethod_;
    pParamInfoSeq = nullptr;

    // enregister the method in a list
    pNext = pFirst;
    pPrev = nullptr;
    pFirst = this;
    if( pNext )
        pNext->pPrev = this;
}

SbUnoMethod::~SbUnoMethod()
{
    delete pParamInfoSeq;

    if( this == pFirst )
        pFirst = pNext;
    else if( pPrev )
        pPrev->pNext = pNext;
    if( pNext )
        pNext->pPrev = pPrev;
}

SbxInfo* SbUnoMethod::GetInfo()
{
    if( !pInfo.is() && m_xUnoMethod.is() )
    {
        SbiInstance* pInst = GetSbData()->pInst;
        if( pInst && pInst->IsCompatibility() )
        {
            pInfo = new SbxInfo();

            const Sequence<ParamInfo>& rInfoSeq = getParamInfos();
            const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
            sal_uInt32 nParamCount = rInfoSeq.getLength();

            for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
            {
                const ParamInfo& rInfo = pParamInfos[i];
                OUString aParamName = rInfo.aName;

                SbxDataType t = SbxVARIANT;
                SbxFlagBits nFlags_ = SbxFlagBits::Read;
                pInfo->AddParam( aParamName, t, nFlags_ );
            }
        }
    }
    return pInfo.get();
}

const Sequence<ParamInfo>& SbUnoMethod::getParamInfos()
{
    if (!pParamInfoSeq)
    {
        Sequence<ParamInfo> aTmp;
        if (m_xUnoMethod.is())
            aTmp = m_xUnoMethod->getParameterInfos();
        pParamInfoSeq = new Sequence<ParamInfo>(aTmp);
    }
    return *pParamInfoSeq;
}

SbUnoProperty::SbUnoProperty
(
    const OUString& aName_,
    SbxDataType eSbxType,
    SbxDataType eRealSbxType,
    const Property& aUnoProp_,
    sal_Int32 nId_,
    bool bInvocation,
    bool bUnoStruct
)
    : SbxProperty( aName_, eSbxType )
    , aUnoProp( aUnoProp_ )
    , nId( nId_ )
    , mbInvocation( bInvocation )
    , mRealType( eRealSbxType )
    , mbUnoStruct( bUnoStruct )
{
    // as needed establish an dummy array so that SbiRuntime::CheckArray() works
    static SbxArrayRef xDummyArray = new SbxArray( SbxVARIANT );
    if( eSbxType & SbxARRAY )
        PutObject( xDummyArray.get() );
}

SbUnoProperty::~SbUnoProperty()
{}


SbxVariable* SbUnoObject::Find( const OUString& rName, SbxClassType t )
{
    static Reference< XIdlMethod > xDummyMethod;
    static Property aDummyProp;

    SbxVariable* pRes = SbxObject::Find( rName, t );

    if( bNeedIntrospection )
        doIntrospection();

    // New 1999-03-04: Create properties on demand. Therefore search now via
    // IntrospectionAccess if a property or a method of the required name exist
    if( !pRes )
    {
        OUString aUName( rName );
        if( mxUnoAccess.is() && !bNativeCOMObject )
        {
            if( mxExactName.is() )
            {
                OUString aUExactName = mxExactName->getExactName( aUName );
                if( !aUExactName.isEmpty() )
                {
                    aUName = aUExactName;
                }
            }
            if( mxUnoAccess->hasProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS ) )
            {
                const Property& rProp = mxUnoAccess->
                    getProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS );

                // If the property could be void the type had to be set to Variant
                SbxDataType eSbxType;
                if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
                    eSbxType = SbxVARIANT;
                else
                    eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

                SbxDataType eRealSbxType = ( ( rProp.Attributes & PropertyAttribute::MAYBEVOID ) ? unoToSbxType( rProp.Type.getTypeClass() ) : eSbxType );
                // create the property and superimpose it
                auto pProp = tools::make_ref<SbUnoProperty>( rProp.Name, eSbxType, eRealSbxType, rProp, 0, false, ( rProp.Type.getTypeClass() ==  css::uno::TypeClass_STRUCT  ) );
                QuickInsert( pProp.get() );
                pRes = pProp.get();
            }
            else if( mxUnoAccess->hasMethod( aUName,
                MethodConcept::ALL - MethodConcept::DANGEROUS ) )
            {
                // address the method
                const Reference< XIdlMethod >& rxMethod = mxUnoAccess->
                    getMethod( aUName, MethodConcept::ALL - MethodConcept::DANGEROUS );

                // create SbUnoMethod and superimpose it
                auto xMethRef = tools::make_ref<SbUnoMethod>( rxMethod->getName(),
                    unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
                QuickInsert( xMethRef.get() );
                pRes = xMethRef.get();
            }

            // If nothing was found check via XNameAccess
            if( !pRes )
            {
                try
                {
                    Reference< XNameAccess > xNameAccess( mxUnoAccess->queryAdapter( cppu::UnoType<XPropertySet>::get()), UNO_QUERY );

                    if( xNameAccess.is() && xNameAccess->hasByName( rName ) )
                    {
                        Any aAny = xNameAccess->getByName( rName );

                        // ATTENTION: Because of XNameAccess, the variable generated here
                        // may not be included as a fixed property in the object and therefore
                        // won't be stored anywhere.
                        // If this leads to problems, it has to be created
                        // synthetically or a class SbUnoNameAccessProperty,
                        // which checks the existence on access and which
                        // is disposed if the name is not found anymore.
                        pRes = new SbxVariable( SbxVARIANT );
                        unoToSbxValue( pRes, aAny );
                    }
                }
                catch( const NoSuchElementException& e )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
                }
                catch( const Exception& )
                {
                    // Establish so that the exception error will not be overwritten
                    if( !pRes )
                        pRes = new SbxVariable( SbxVARIANT );

                    implHandleAnyException( ::cppu::getCaughtException() );
                }
            }
        }
        if( !pRes && mxInvocation.is() )
        {
            if( mxExactNameInvocation.is() )
            {
                OUString aUExactName = mxExactNameInvocation->getExactName( aUName );
                if( !aUExactName.isEmpty() )
                {
                    aUName = aUExactName;
                }
            }

            try
            {
                if( mxInvocation->hasProperty( aUName ) )
                {
                    // create a property and superimpose it
                    auto xVarRef = tools::make_ref<SbUnoProperty>( aUName, SbxVARIANT, SbxVARIANT, aDummyProp, 0, true, false );
                    QuickInsert( xVarRef.get() );
                    pRes = xVarRef.get();
                }
                else if( mxInvocation->hasMethod( aUName ) )
                {
                    // create SbUnoMethode and superimpose it
                    auto xMethRef = tools::make_ref<SbUnoMethod>( aUName, SbxVARIANT, xDummyMethod, true );
                    QuickInsert( xMethRef.get() );
                    pRes = xMethRef.get();
                }
                else
                {
                    Reference< XDirectInvocation > xDirectInvoke( mxInvocation, UNO_QUERY );
                    if ( xDirectInvoke.is() && xDirectInvoke->hasMember( aUName ) )
                    {
                        auto xMethRef = tools::make_ref<SbUnoMethod>( aUName, SbxVARIANT, xDummyMethod, true );
                        QuickInsert( xMethRef.get() );
                        pRes = xMethRef.get();
                    }

                }
            }
            catch( const RuntimeException& e )
            {
                // Establish so that the exception error will not be overwritten
                if( !pRes )
                    pRes = new SbxVariable( SbxVARIANT );

                StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
            }
        }
    }

    // At the very end checking if the Dbg_-Properties are meant

    if( !pRes )
    {
        if( rName.equalsIgnoreAsciiCase(ID_DBG_SUPPORTEDINTERFACES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_PROPERTIES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_METHODS) )
        {
            // Create
            implCreateDbgProperties();

            // Now they have to be found regular
            pRes = SbxObject::Find( rName, SbxClassType::DontCare );
        }
    }
    return pRes;
}


// help method to create the dbg_-Properties
void SbUnoObject::implCreateDbgProperties()
{
    Property aProp;

    // Id == -1: display the implemented interfaces corresponding the ClassProvider
    auto xVarRef = tools::make_ref<SbUnoProperty>( OUString(ID_DBG_SUPPORTEDINTERFACES), SbxSTRING, SbxSTRING, aProp, -1, false, false );
    QuickInsert( xVarRef.get() );

    // Id == -2: output the properties
    xVarRef = tools::make_ref<SbUnoProperty>( OUString(ID_DBG_PROPERTIES), SbxSTRING, SbxSTRING, aProp, -2, false, false );
    QuickInsert( xVarRef.get() );

    // Id == -3: output the Methods
    xVarRef = tools::make_ref<SbUnoProperty>( OUString(ID_DBG_METHODS), SbxSTRING, SbxSTRING, aProp, -3, false, false );
    QuickInsert( xVarRef.get() );
}

void SbUnoObject::implCreateAll()
{
    // throw away all existing methods and properties
    pMethods   = tools::make_ref<SbxArray>();
    pProps     = tools::make_ref<SbxArray>();

    if( bNeedIntrospection ) doIntrospection();

    // get introspection
    Reference< XIntrospectionAccess > xAccess = mxUnoAccess;
    if( !xAccess.is() || bNativeCOMObject )
    {
        if( mxInvocation.is() )
            xAccess = mxInvocation->getIntrospection();
        else if( bNativeCOMObject )
            return;
    }
    if( !xAccess.is() )
        return;

    // Establish properties
    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    sal_uInt32 nPropCount = props.getLength();
    const Property* pProps_ = props.getConstArray();

    sal_uInt32 i;
    for( i = 0 ; i < nPropCount ; i++ )
    {
        const Property& rProp = pProps_[ i ];

        // If the property could be void the type had to be set to Variant
        SbxDataType eSbxType;
        if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
            eSbxType = SbxVARIANT;
        else
            eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

        SbxDataType eRealSbxType = ( ( rProp.Attributes & PropertyAttribute::MAYBEVOID ) ? unoToSbxType( rProp.Type.getTypeClass() ) : eSbxType );
        // Create property and superimpose it
        auto xVarRef = tools::make_ref<SbUnoProperty>( rProp.Name, eSbxType, eRealSbxType, rProp, i, false, ( rProp.Type.getTypeClass() == css::uno::TypeClass_STRUCT   ) );
        QuickInsert( xVarRef.get() );
    }

    // Create Dbg_-Properties
    implCreateDbgProperties();

    // Create methods
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    sal_uInt32 nMethCount = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods_ = aMethodSeq.getConstArray();
    for( i = 0 ; i < nMethCount ; i++ )
    {
        // address method
        const Reference< XIdlMethod >& rxMethod = pMethods_[i];

        // Create SbUnoMethod and superimpose it
        auto xMethRef = tools::make_ref<SbUnoMethod>
            ( rxMethod->getName(), unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
        QuickInsert( xMethRef.get() );
    }
}


// output the value
Any SbUnoObject::getUnoAny()
{
    Any aRetAny;
    if( bNeedIntrospection ) doIntrospection();
    if ( maStructInfo.get() )
       aRetAny = maTmpUnoObj;
    else if( mxMaterialHolder.is() )
        aRetAny = mxMaterialHolder->getMaterial();
    else if( mxInvocation.is() )
        aRetAny <<= mxInvocation;
    return aRetAny;
}

// help method to create an Uno-Struct per CoreReflection
SbUnoObject* Impl_CreateUnoStruct( const OUString& aClassName )
{
    // get CoreReflection
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return nullptr;

    // search for the class
    Reference< XIdlClass > xClass;
    Reference< XHierarchicalNameAccess > xHarryName =
        getCoreReflection_HierarchicalNameAccess_Impl();
    if( xHarryName.is() && xHarryName->hasByHierarchicalName( aClassName ) )
        xClass = xCoreReflection->forName( aClassName );
    if( !xClass.is() )
        return nullptr;

    // Is it really a struct?
    TypeClass eType = xClass->getTypeClass();
    if ( ( eType != TypeClass_STRUCT ) && ( eType != TypeClass_EXCEPTION ) )
        return nullptr;

    // create an instance
    Any aNewAny;
    xClass->createObject( aNewAny );
    // make a SbUnoObject out of it
    SbUnoObject* pUnoObj = new SbUnoObject( aClassName, aNewAny );
    return pUnoObj;
}


// Factory-Class to create Uno-Structs per DIM AS NEW
SbxBase* SbUnoFactory::Create( sal_uInt16, sal_uInt32 )
{
    // Via SbxId nothing works in Uno
    return nullptr;
}

SbxObject* SbUnoFactory::CreateObject( const OUString& rClassName )
{
    return Impl_CreateUnoStruct( rClassName );
}


// Provisional interface for the UNO-Connection
// Deliver a SbxObject, that wrap an Uno-Interface
SbxObjectRef GetSbUnoObject( const OUString& aName, const Any& aUnoObj_ )
{
    return new SbUnoObject( aName, aUnoObj_ );
}

// Force creation of all properties for debugging
void createAllObjectProperties( SbxObject* pObj )
{
    if( !pObj )
        return;

    SbUnoObject* pUnoObj = dynamic_cast<SbUnoObject*>( pObj );
    SbUnoStructRefObject* pUnoStructObj = dynamic_cast<SbUnoStructRefObject*>( pObj );
    if( pUnoObj )
    {
        pUnoObj->createAllProperties();
    }
    else if ( pUnoStructObj )
    {
        pUnoStructObj->createAllProperties();
    }
}


void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 1 parameter minimum
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    OUString aClassName = rPar.Get(1)->GetOUString();

    // try to create Struct with the same name
    SbUnoObjectRef xUnoObj = Impl_CreateUnoStruct( aClassName );
    if( !xUnoObj.is() )
    {
        return;
    }
    // return the object
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( xUnoObj.get() );
}

void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 1 Parameter minimum
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    OUString aServiceName = rPar.Get(1)->GetOUString();

    // search for the service and instantiate it
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Reference< XInterface > xInterface;
    try
    {
        xInterface = xFactory->createInstance( aServiceName );
    }
    catch( const Exception& )
    {
        implHandleAnyException( ::cppu::getCaughtException() );
    }

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        // Create a SbUnoObject out of it and return it
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, Any(xInterface) );
        if( xUnoObj->getUnoAny().hasValue() )
        {
            // return the object
            refVar->PutObject( xUnoObj.get() );
        }
        else
        {
            refVar->PutObject( nullptr );
        }
    }
    else
    {
        refVar->PutObject( nullptr );
    }
}

void RTL_Impl_CreateUnoServiceWithArguments( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 2 parameter minimum
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    OUString aServiceName = rPar.Get(1)->GetOUString();
    Any aArgAsAny = sbxToUnoValue( rPar.Get(2),
                cppu::UnoType<Sequence<Any>>::get() );
    Sequence< Any > aArgs;
    aArgAsAny >>= aArgs;

    // search for the service and instantiate it
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Reference< XInterface > xInterface;
    try
    {
        xInterface = xFactory->createInstanceWithArguments( aServiceName, aArgs );
    }
    catch( const Exception& )
    {
        implHandleAnyException( ::cppu::getCaughtException() );
    }

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        // Create a SbUnoObject out of it and return it
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, Any(xInterface) );
        if( xUnoObj->getUnoAny().hasValue() )
        {
            // return the object
            refVar->PutObject( xUnoObj.get() );
        }
        else
        {
            refVar->PutObject( nullptr );
        }
    }
    else
    {
        refVar->PutObject( nullptr );
    }
}

void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    SbxVariableRef refVar = rPar.Get(0);

    // get the global service manager
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );

    // Create a SbUnoObject out of it and return it
    SbUnoObjectRef xUnoObj = new SbUnoObject( "ProcessServiceManager", Any(xFactory) );
    refVar->PutObject( xUnoObj.get() );
}

void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 2 parameter minimum
    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // variable for the return value
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( false );

    // get the Uno-Object
    SbxBaseRef pObj = rPar.Get( 1 )->GetObject();
    auto obj = dynamic_cast<SbUnoObject*>( pObj.get() );
    if( obj == nullptr )
    {
        return;
    }
    Any aAny = obj->getUnoAny();
    auto x = o3tl::tryAccess<Reference<XInterface>>(aAny);
    if( !x )
    {
        return;
    }

    // get CoreReflection
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
    {
        return;
    }
    for( sal_uInt16 i = 2 ; i < nParCount ; i++ )
    {
        // get the name of the interface of the struct
        OUString aIfaceName = rPar.Get( i )->GetOUString();

        // search for the class
        Reference< XIdlClass > xClass = xCoreReflection->forName( aIfaceName );
        if( !xClass.is() )
        {
            return;
        }
        // check if the interface will be supported
        OUString aClassName = xClass->getName();
        Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
        if( !(*x)->queryInterface( aClassType ).hasValue() )
        {
            return;
        }
    }

    // Every thing works; then return TRUE
    refVar->PutBool( true );
}

void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 1 parameter minimum
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // variable for the return value
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( false );

    // get the Uno-Object
    SbxVariableRef xParam = rPar.Get( 1 );
    if( !xParam->IsObject() )
    {
        return;
    }
    SbxBaseRef pObj = rPar.Get( 1 )->GetObject();
    auto obj = dynamic_cast<SbUnoObject*>( pObj.get() );
    if( obj == nullptr )
    {
        return;
    }
    Any aAny = obj->getUnoAny();
    TypeClass eType = aAny.getValueType().getTypeClass();
    if( eType == TypeClass_STRUCT )
    {
        refVar->PutBool( true );
    }
}


void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // variable for the return value
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( false );

    // get the Uno-Objects
    SbxVariableRef xParam1 = rPar.Get( 1 );
    if( !xParam1->IsObject() )
    {
        return;
    }
    SbxBaseRef pObj1 = xParam1->GetObject();
    auto obj1 = dynamic_cast<SbUnoObject*>( pObj1.get() );
    if( obj1 == nullptr )
    {
        return;
    }
    Any aAny1 = obj1->getUnoAny();
    TypeClass eType1 = aAny1.getValueType().getTypeClass();
    if( eType1 != TypeClass_INTERFACE )
    {
        return;
    }
    Reference< XInterface > x1;
    aAny1 >>= x1;

    SbxVariableRef xParam2 = rPar.Get( 2 );
    if( !xParam2->IsObject() )
    {
        return;
    }
    SbxBaseRef pObj2 = xParam2->GetObject();
    auto obj2 = dynamic_cast<SbUnoObject*>( pObj2.get() );
    if( obj2 == nullptr )
    {
        return;
    }
    Any aAny2 = obj2->getUnoAny();
    TypeClass eType2 = aAny2.getValueType().getTypeClass();
    if( eType2 != TypeClass_INTERFACE )
    {
        return;
    }
    Reference< XInterface > x2;
    aAny2 >>= x2;

    if( x1 == x2 )
    {
        refVar->PutBool( true );
    }
}


// helper wrapper function to interact with TypeProvider and
// XTypeDescriptionEnumerationAccess.
// if it fails for whatever reason
// returned Reference<> be null e.g. .is() will be false

Reference< XTypeDescriptionEnumeration > getTypeDescriptorEnumeration( const OUString& sSearchRoot,
                                                                       const Sequence< TypeClass >& types,
                                                                       TypeDescriptionSearchDepth depth )
{
    Reference< XTypeDescriptionEnumeration > xEnum;
    Reference< XTypeDescriptionEnumerationAccess> xTypeEnumAccess( getTypeProvider_Impl(), UNO_QUERY );
    if ( xTypeEnumAccess.is() )
    {
        try
        {
            xEnum = xTypeEnumAccess->createTypeDescriptionEnumeration(
                sSearchRoot, types, depth );
        }
        catch(const NoSuchTypeNameException& /*nstne*/ ) {}
        catch(const InvalidTypeNameException& /*nstne*/ ) {}
    }
    return xEnum;
}

typedef std::unordered_map< OUString, Any, OUStringHash > VBAConstantsHash;

VBAConstantHelper&
VBAConstantHelper::instance()
{
    static VBAConstantHelper aHelper;
    return aHelper;
}

void VBAConstantHelper::init()
{
    if ( !isInited )
    {
        Sequence< TypeClass > types(1);
        types[ 0 ] = TypeClass_CONSTANTS;
        Reference< XTypeDescriptionEnumeration > xEnum = getTypeDescriptorEnumeration( "ooo.vba", types, TypeDescriptionSearchDepth_INFINITE  );

        if ( !xEnum.is())
        {
            return; //NULL;
        }
        while ( xEnum->hasMoreElements() )
        {
            Reference< XConstantsTypeDescription > xConstants( xEnum->nextElement(), UNO_QUERY );
            if ( xConstants.is() )
            {
                // store constant group name
                OUString sFullName = xConstants->getName();
                sal_Int32 indexLastDot = sFullName.lastIndexOf('.');
                OUString sLeafName( sFullName );
                if ( indexLastDot > -1 )
                {
                    sLeafName = sFullName.copy( indexLastDot + 1);
                }
                aConstCache.push_back( sLeafName ); // assume constant group names are unique
                Sequence< Reference< XConstantTypeDescription > > aConsts = xConstants->getConstants();
                for (sal_Int32 i = 0; i != aConsts.getLength(); ++i)
                {
                    // store constant member name
                    sFullName = aConsts[i]->getName();
                    indexLastDot = sFullName.lastIndexOf('.');
                    sLeafName = sFullName;
                    if ( indexLastDot > -1 )
                    {
                        sLeafName = sFullName.copy( indexLastDot + 1);
                    }
                    aConstHash[ sLeafName.toAsciiLowerCase() ] = aConsts[i]->getConstantValue();
                }
            }
        }
        isInited = true;
    }
}

bool
VBAConstantHelper::isVBAConstantType( const OUString& rName )
{
    init();
    bool bConstant = false;
    VBAConstantsVector::const_iterator it = aConstCache.begin();

    for( ; it != aConstCache.end(); ++it )
    {
        if( rName.equalsIgnoreAsciiCase( *it ) )
        {
            bConstant = true;
            break;
        }
    }
    return bConstant;
}

SbxVariable*
VBAConstantHelper::getVBAConstant( const OUString& rName )
{
    SbxVariable* pConst = nullptr;
    init();

    VBAConstantsHash::const_iterator it = aConstHash.find( rName.toAsciiLowerCase() );

    if ( it != aConstHash.end() )
    {
        pConst = new SbxVariable( SbxVARIANT );
        pConst->SetName( rName );
        unoToSbxValue( pConst, it->second );
    }

    return pConst;
}

// Function to search for a global identifier in the
// UnoScope and to wrap it for Sbx
SbUnoClass* findUnoClass( const OUString& rName )
{
    // #105550 Check if module exists
    SbUnoClass* pUnoClass = nullptr;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_MODULE || eTypeClass == TypeClass_CONSTANTS )
            {
                pUnoClass = new SbUnoClass( rName );
            }
        }
    }
    return pUnoClass;
}

SbxVariable* SbUnoClass::Find( const OUString& rName, SbxClassType )
{
    SbxVariable* pRes = SbxObject::Find( rName, SbxClassType::Variable );

    // If nothing were located the submodule isn't known yet
    if( !pRes )
    {
        // If it is already a class, ask for the field
        if( m_xClass.is() )
        {
            // Is it a field(?)
            Reference< XIdlField > xField = m_xClass->getField( rName );
            Reference< XIdlClass > xClass;
            if( xField.is() )
            {
                try
                {
                    Any aAny;
                    aAny = xField->get( aAny );

                    // Convert to Sbx
                    pRes = new SbxVariable( SbxVARIANT );
                    pRes->SetName( rName );
                    unoToSbxValue( pRes, aAny );
                }
                catch( const Exception& )
                {
                    implHandleAnyException( ::cppu::getCaughtException() );
                }
            }
        }
        else
        {
            // expand fully qualified name
            OUString aNewName = GetName()
                              + "."
                              + rName;

            // get CoreReflection
            Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
            if( xCoreReflection.is() )
            {
                // Is it a constant?
                Reference< XHierarchicalNameAccess > xHarryName( xCoreReflection, UNO_QUERY );
                if( xHarryName.is() )
                {
                    try
                    {
                        Any aValue = xHarryName->getByHierarchicalName( aNewName );
                        TypeClass eType = aValue.getValueType().getTypeClass();

                        // Interface located? Then it is a class
                        if( eType == TypeClass_INTERFACE )
                        {
                            Reference< XIdlClass > xClass( aValue, UNO_QUERY );
                            if( xClass.is() )
                            {
                                pRes = new SbxVariable( SbxVARIANT );
                                SbxObjectRef xWrapper = static_cast<SbxObject*>(new SbUnoClass( aNewName, xClass ));
                                pRes->PutObject( xWrapper.get() );
                            }
                        }
                        else
                        {
                            pRes = new SbxVariable( SbxVARIANT );
                            unoToSbxValue( pRes, aValue );
                        }
                    }
                    catch( const NoSuchElementException& )
                    {
                    }
                }

                // Otherwise take it again as class
                if( !pRes )
                {
                    SbUnoClass* pNewClass = findUnoClass( aNewName );
                    if( pNewClass )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = static_cast<SbxObject*>(pNewClass);
                        pRes->PutObject( xWrapper.get() );
                    }
                }

                // An UNO service?
                if( !pRes )
                {
                    SbUnoService* pUnoService = findUnoService( aNewName );
                    if( pUnoService )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = static_cast<SbxObject*>(pUnoService);
                        pRes->PutObject( xWrapper.get() );
                    }
                }

                // An UNO singleton?
                if( !pRes )
                {
                    SbUnoSingleton* pUnoSingleton = findUnoSingleton( aNewName );
                    if( pUnoSingleton )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = static_cast<SbxObject*>(pUnoSingleton);
                        pRes->PutObject( xWrapper.get() );
                    }
                }
            }
        }

        if( pRes )
        {
            pRes->SetName( rName );

            // Insert variable, so that it could be found later
            QuickInsert( pRes );

            // Take us out as listener at once,
            // the values are all constant
            if( pRes->IsBroadcaster() )
                EndListening( pRes->GetBroadcaster(), true );
        }
    }
    return pRes;
}


SbUnoService* findUnoService( const OUString& rName )
{
    SbUnoService* pSbUnoService = nullptr;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_SERVICE )
            {
                Reference< XServiceTypeDescription2 > xServiceTypeDesc( xTypeDesc, UNO_QUERY );
                if( xServiceTypeDesc.is() )
                    pSbUnoService = new SbUnoService( rName, xServiceTypeDesc );
            }
        }
    }
    return pSbUnoService;
}

SbxVariable* SbUnoService::Find( const OUString& rName, SbxClassType )
{
    SbxVariable* pRes = SbxObject::Find( rName, SbxClassType::Method );

    if( !pRes )
    {
        // If it is already a class ask for a field
        if( m_bNeedsInit && m_xServiceTypeDesc.is() )
        {
            m_bNeedsInit = false;

            Sequence< Reference< XServiceConstructorDescription > > aSCDSeq = m_xServiceTypeDesc->getConstructors();
            const Reference< XServiceConstructorDescription >* pCtorSeq = aSCDSeq.getConstArray();
            int nCtorCount = aSCDSeq.getLength();
            for( int i = 0 ; i < nCtorCount ; ++i )
            {
                Reference< XServiceConstructorDescription > xCtor = pCtorSeq[i];

                OUString aName( xCtor->getName() );
                if( aName.isEmpty() )
                {
                    if( xCtor->isDefaultConstructor() )
                    {
                        aName = "create";
                    }
                }

                if( !aName.isEmpty() )
                {
                    // Create and insert SbUnoServiceCtor
                    SbxVariableRef xSbCtorRef = new SbUnoServiceCtor( aName, xCtor );
                    QuickInsert( xSbCtorRef.get() );
                }
            }
            pRes = SbxObject::Find( rName, SbxClassType::Method );
        }
    }

    return pRes;
}

void SbUnoService::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoServiceCtor* pUnoCtor = dynamic_cast<SbUnoServiceCtor*>( pVar );
        if( pUnoCtor && pHint->GetId() == SfxHintId::BasicDataWanted )
        {
            // Parameter count -1 because of Param0 == this
            sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
            Sequence<Any> args;

            Reference< XServiceConstructorDescription > xCtor = pUnoCtor->getServiceCtorDesc();
            Sequence< Reference< XParameter > > aParameterSeq = xCtor->getParameters();
            const Reference< XParameter >* pParameterSeq = aParameterSeq.getConstArray();
            sal_uInt32 nUnoParamCount = aParameterSeq.getLength();

            // Default: Ignore not needed parameters
            bool bParameterError = false;

            // Is the last parameter a rest parameter?
            bool bRestParameterMode = false;
            if( nUnoParamCount > 0 )
            {
                Reference< XParameter > xLastParam = pParameterSeq[ nUnoParamCount - 1 ];
                if( xLastParam.is() )
                {
                    if( xLastParam->isRestParameter() )
                        bRestParameterMode = true;
                }
            }

            // Too many parameters with context as first parameter?
            sal_uInt16 nSbxParameterOffset = 1;
            sal_uInt16 nParameterOffsetByContext = 0;
            Reference < XComponentContext > xFirstParamContext;
            if( nParamCount > nUnoParamCount )
            {
                // Check if first parameter is a context and use it
                // then in createInstanceWithArgumentsAndContext
                Any aArg0 = sbxToUnoValue( pParams->Get( nSbxParameterOffset ) );
                if( (aArg0 >>= xFirstParamContext) && xFirstParamContext.is() )
                    nParameterOffsetByContext = 1;
            }

            sal_uInt32 nEffectiveParamCount = nParamCount - nParameterOffsetByContext;
            sal_uInt32 nAllocParamCount = nEffectiveParamCount;
            if( nEffectiveParamCount > nUnoParamCount )
            {
                if( !bRestParameterMode )
                {
                    nEffectiveParamCount = nUnoParamCount;
                    nAllocParamCount = nUnoParamCount;
                }
            }
            // Not enough parameters?
            else if( nUnoParamCount > nEffectiveParamCount )
            {
                // RestParameterMode only helps if one (the last) parameter is missing
                int nDiff = nUnoParamCount - nEffectiveParamCount;
                if( !bRestParameterMode || nDiff > 1 )
                {
                    bParameterError = true;
                    StarBASIC::Error( ERRCODE_BASIC_NOT_OPTIONAL );
                }
            }

            if( !bParameterError )
            {
                bool bOutParams = false;
                if( nAllocParamCount > 0 )
                {
                    args.realloc( nAllocParamCount );
                    Any* pAnyArgs = args.getArray();
                    for( sal_uInt32 i = 0 ; i < nEffectiveParamCount ; i++ )
                    {
                        sal_uInt16 iSbx = (sal_uInt16)(i + nSbxParameterOffset + nParameterOffsetByContext);

                        // bRestParameterMode allows nEffectiveParamCount > nUnoParamCount
                        Reference< XParameter > xParam;
                        if( i < nUnoParamCount )
                        {
                            xParam = pParameterSeq[i];
                            if( !xParam.is() )
                                continue;

                            Reference< XTypeDescription > xParamTypeDesc = xParam->getType();
                            if( !xParamTypeDesc.is() )
                                continue;
                            css::uno::Type aType( xParamTypeDesc->getTypeClass(), xParamTypeDesc->getName() );

                            // sbx parameter needs offset 1
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ), aType );

                            // Check for out parameter if not already done
                            if( !bOutParams )
                            {
                                if( xParam->isOut() )
                                    bOutParams = true;
                            }
                        }
                        else
                        {
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ) );
                        }
                    }
                }

                // "Call" ctor using createInstanceWithArgumentsAndContext
                Reference < XComponentContext > xContext(
                    xFirstParamContext.is()
                    ? xFirstParamContext
                    : comphelper::getProcessComponentContext() );
                Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );

                Any aRetAny;
                OUString aServiceName = GetName();
                Reference < XInterface > xRet;
                try
                {
                    xRet = xServiceMgr->createInstanceWithArgumentsAndContext( aServiceName, args, xContext );
                }
                catch( const Exception& )
                {
                    implHandleAnyException( ::cppu::getCaughtException() );
                }
                aRetAny <<= xRet;
                unoToSbxValue( pVar, aRetAny );

                // Copy back out parameters?
                if( bOutParams )
                {
                    const Any* pAnyArgs = args.getConstArray();

                    for( sal_uInt32 j = 0 ; j < nUnoParamCount ; j++ )
                    {
                        Reference< XParameter > xParam = pParameterSeq[j];
                        if( !xParam.is() )
                            continue;

                        if( xParam->isOut() )
                            unoToSbxValue( pParams->Get( (sal_uInt16)(j+1) ), pAnyArgs[ j ] );
                    }
                }
            }
        }
        else
            SbxObject::Notify( rBC, rHint );
    }
}


static SbUnoServiceCtor* pFirstCtor = nullptr;

void clearUnoServiceCtors()
{
    SbUnoServiceCtor* pCtor = pFirstCtor;
    if( pCtor )
        pCtor->SbxValue::Clear();
}

SbUnoServiceCtor::SbUnoServiceCtor( const OUString& aName_, Reference< XServiceConstructorDescription > const & xServiceCtorDesc )
    : SbxMethod( aName_, SbxOBJECT )
    , m_xServiceCtorDesc( xServiceCtorDesc )
{
}

SbUnoServiceCtor::~SbUnoServiceCtor()
{
}

SbxInfo* SbUnoServiceCtor::GetInfo()
{
    SbxInfo* pRet = nullptr;

    return pRet;
}


SbUnoSingleton* findUnoSingleton( const OUString& rName )
{
    SbUnoSingleton* pSbUnoSingleton = nullptr;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
        Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_SINGLETON )
            {
                Reference< XSingletonTypeDescription > xSingletonTypeDesc( xTypeDesc, UNO_QUERY );
                if( xSingletonTypeDesc.is() )
                    pSbUnoSingleton = new SbUnoSingleton( rName );
            }
        }
    }
    return pSbUnoSingleton;
}

SbUnoSingleton::SbUnoSingleton( const OUString& aName_ )
        : SbxObject( aName_ )
{
    SbxVariableRef xGetMethodRef = new SbxMethod( "get", SbxOBJECT );
    QuickInsert( xGetMethodRef.get() );
}

void SbUnoSingleton::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
        sal_uInt32 nAllowedParamCount = 1;

        Reference < XComponentContext > xContextToUse;
        if( nParamCount > 0 )
        {
            // Check if first parameter is a context and use it then
            Reference < XComponentContext > xFirstParamContext;
            Any aArg1 = sbxToUnoValue( pParams->Get( 1 ) );
            if( (aArg1 >>= xFirstParamContext) && xFirstParamContext.is() )
                xContextToUse = xFirstParamContext;
        }

        if( !xContextToUse.is() )
        {
            xContextToUse = comphelper::getProcessComponentContext();
            --nAllowedParamCount;
        }

        if( nParamCount > nAllowedParamCount )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
            return;
        }

        Any aRetAny;
        if( xContextToUse.is() )
        {
            OUString aSingletonName = "/singletons/"
                                    + GetName();
            Reference < XInterface > xRet;
            xContextToUse->getValueByName( aSingletonName ) >>= xRet;
            aRetAny <<= xRet;
        }
        unoToSbxValue( pVar, aRetAny );
    }
    else
    {
        SbxObject::Notify( rBC, rHint );
    }
}


// Implementation of an EventAttacher-drawn AllListener, which
// solely transmits several events to an general AllListener
class BasicAllListener_Impl : public WeakImplHelper< XAllListener >
{
    void firing_impl(const AllEventObject& Event, Any* pRet);

public:
    SbxObjectRef    xSbxObj;
    OUString        aPrefixName;

    explicit BasicAllListener_Impl( const OUString& aPrefixName );

    // Methods of XAllListener
    virtual void SAL_CALL firing(const AllEventObject& Event) override;
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) override;

    // Methods of XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) override;
};


BasicAllListener_Impl::BasicAllListener_Impl(const OUString& aPrefixName_)
    : aPrefixName( aPrefixName_ )
{
}

void BasicAllListener_Impl::firing_impl( const AllEventObject& Event, Any* pRet )
{
    SolarMutexGuard guard;

    if( xSbxObj.is() )
    {
        OUString aMethodName = aPrefixName;
        aMethodName = aMethodName + Event.MethodName;

        SbxVariable * pP = xSbxObj.get();
        while( pP->GetParent() )
        {
            pP = pP->GetParent();
            StarBASIC * pLib = dynamic_cast<StarBASIC*>( pP );
            if( pLib )
            {
                // Create in a Basic Array
                SbxArrayRef xSbxArray = new SbxArray( SbxVARIANT );
                const Any * pArgs = Event.Arguments.getConstArray();
                sal_Int32 nCount = Event.Arguments.getLength();
                for( sal_Int32 i = 0; i < nCount; i++ )
                {
                    // Convert elements
                    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                    unoToSbxValue( xVar.get(), pArgs[i] );
                    xSbxArray->Put( xVar.get(), sal::static_int_cast< sal_uInt16 >(i+1) );
                }

                pLib->Call( aMethodName, xSbxArray.get() );

                // get the return value from the Param-Array, if requested
                if( pRet )
                {
                    SbxVariable* pVar = xSbxArray->Get( 0 );
                    if( pVar )
                    {
                        // #95792 Avoid a second call
                        SbxFlagBits nFlags = pVar->GetFlags();
                        pVar->SetFlag( SbxFlagBits::NoBroadcast );
                        *pRet = sbxToUnoValueImpl( pVar );
                        pVar->SetFlags( nFlags );
                    }
                }
                break;
            }
        }
    }
}


// Methods of Listener
void BasicAllListener_Impl::firing( const AllEventObject& Event )
{
    firing_impl( Event, nullptr );
}

Any BasicAllListener_Impl::approveFiring( const AllEventObject& Event )
{
    Any aRetAny;
    firing_impl( Event, &aRetAny );
    return aRetAny;
}


// Methods of XEventListener
void BasicAllListener_Impl ::disposing(const EventObject& )
{
    SolarMutexGuard guard;

    xSbxObj.clear();
}


//  class InvocationToAllListenerMapper
//  helper class to map XInvocation to XAllListener (also in project eventattacher!)

class InvocationToAllListenerMapper : public WeakImplHelper< XInvocation >
{
public:
    InvocationToAllListenerMapper( const Reference< XIdlClass >& ListenerType,
        const Reference< XAllListener >& AllListener, const Any& Helper );

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection() override;
    virtual Any SAL_CALL invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam) override;
    virtual void SAL_CALL setValue(const OUString& PropertyName, const Any& Value) override;
    virtual Any SAL_CALL getValue(const OUString& PropertyName) override;
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name) override;
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name) override;

private:
    Reference< XAllListener >    m_xAllListener;
    Reference< XIdlClass >       m_xListenerType;
    Any                          m_Helper;
};


// Function to replace AllListenerAdapterService::createAllListerAdapter
Reference< XInterface > createAllListenerAdapter
(
    const Reference< XInvocationAdapterFactory2 >& xInvocationAdapterFactory,
    const Reference< XIdlClass >& xListenerType,
    const Reference< XAllListener >& xListener,
    const Any& Helper
)
{
    Reference< XInterface > xAdapter;
    if( xInvocationAdapterFactory.is() && xListenerType.is() && xListener.is() )
    {
        Reference< XInvocation > xInvocationToAllListenerMapper =
            static_cast<XInvocation*>(new InvocationToAllListenerMapper( xListenerType, xListener, Helper ));
        Type aListenerType( xListenerType->getTypeClass(), xListenerType->getName() );
        Sequence<Type> arg2(1);
        arg2[0] = aListenerType;
        xAdapter = xInvocationAdapterFactory->createAdapter( xInvocationToAllListenerMapper, arg2 );
    }
    return xAdapter;
}


// InvocationToAllListenerMapper
InvocationToAllListenerMapper::InvocationToAllListenerMapper
    ( const Reference< XIdlClass >& ListenerType, const Reference< XAllListener >& AllListener, const Any& Helper )
        : m_xAllListener( AllListener )
        , m_xListenerType( ListenerType )
        , m_Helper( Helper )
{
}


Reference< XIntrospectionAccess > SAL_CALL InvocationToAllListenerMapper::getIntrospection()
{
    return Reference< XIntrospectionAccess >();
}


Any SAL_CALL InvocationToAllListenerMapper::invoke(const OUString& FunctionName, const Sequence< Any >& Params,
    Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
{
    (void)OutParamIndex;
    (void)OutParam     ;

    Any aRet;

    // Check if to firing or approveFiring has to be called
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( FunctionName );
    bool bApproveFiring = false;
    if( !xMethod.is() )
        return aRet;
    Reference< XIdlClass > xReturnType = xMethod->getReturnType();
    Sequence< Reference< XIdlClass > > aExceptionSeq = xMethod->getExceptionTypes();
    if( ( xReturnType.is() && xReturnType->getTypeClass() != TypeClass_VOID ) ||
        aExceptionSeq.getLength() > 0 )
    {
        bApproveFiring = true;
    }
    else
    {
        Sequence< ParamInfo > aParamSeq = xMethod->getParameterInfos();
        sal_uInt32 nParamCount = aParamSeq.getLength();
        if( nParamCount > 1 )
        {
            const ParamInfo* pInfo = aParamSeq.getConstArray();
            for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
            {
                if( pInfo[ i ].aMode != ParamMode_IN )
                {
                    bApproveFiring = true;
                    break;
                }
            }
        }
    }

    AllEventObject aAllEvent;
    aAllEvent.Source = static_cast<OWeakObject*>(this);
    aAllEvent.Helper = m_Helper;
    aAllEvent.ListenerType = Type(m_xListenerType->getTypeClass(), m_xListenerType->getName() );
    aAllEvent.MethodName = FunctionName;
    aAllEvent.Arguments = Params;
    if( bApproveFiring )
        aRet = m_xAllListener->approveFiring( aAllEvent );
    else
        m_xAllListener->firing( aAllEvent );
    return aRet;
}


void SAL_CALL InvocationToAllListenerMapper::setValue(const OUString& PropertyName, const Any& Value)
{
    (void)PropertyName;
    (void)Value;
}


Any SAL_CALL InvocationToAllListenerMapper::getValue(const OUString& PropertyName)
{
    (void)PropertyName;

    return Any();
}


sal_Bool SAL_CALL InvocationToAllListenerMapper::hasMethod(const OUString& Name)
{
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( Name );
    return xMethod.is();
}


sal_Bool SAL_CALL InvocationToAllListenerMapper::hasProperty(const OUString& Name)
{
    Reference< XIdlField > xField = m_xListenerType->getField( Name );
    return xField.is();
}


// create Uno-Service
// 1. Parameter == Prefix-Name of the macro
// 2. Parameter == fully qualified name of the listener
void SbRtl_CreateUnoListener( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
//RTLFUNC(CreateUnoListener)
{
    (void)bWrite;

    // We need 2 parameters
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    OUString aPrefixName = rPar.Get(1)->GetOUString();
    OUString aListenerClassName = rPar.Get(2)->GetOUString();

    // get the CoreReflection
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return;

    // get the AllListenerAdapterService
    Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // search the class
    Reference< XIdlClass > xClass = xCoreReflection->forName( aListenerClassName );
    if( !xClass.is() )
        return;

    // From 1999-11-30: get the InvocationAdapterFactory
    Reference< XInvocationAdapterFactory2 > xInvocationAdapterFactory =
         InvocationAdapterFactory::create( xContext );

    BasicAllListener_Impl * p;
    Reference< XAllListener > xAllLst = p = new BasicAllListener_Impl( aPrefixName );
    Any aTmp;
    Reference< XInterface > xLst = createAllListenerAdapter( xInvocationAdapterFactory, xClass, xAllLst, aTmp );
    if( !xLst.is() )
        return;

    OUString aClassName = xClass->getName();
    Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
    aTmp = xLst->queryInterface( aClassType );
    if( !aTmp.hasValue() )
        return;

    SbUnoObject* pUnoObj = new SbUnoObject( aListenerClassName, aTmp );
    p->xSbxObj = pUnoObj;
    p->xSbxObj->SetParent( pBasic );

    // #100326 Register listener object to set Parent NULL in Dtor
    SbxArrayRef xBasicUnoListeners = pBasic->getUnoListeners();
    xBasicUnoListeners->Insert( pUnoObj, xBasicUnoListeners->Count() );

    // return the object
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( p->xSbxObj.get() );
}


// Represents the DefaultContext property of the ProcessServiceManager
// in the Basic runtime system.
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    SbxVariableRef refVar = rPar.Get(0);

    Any aContextAny( comphelper::getProcessComponentContext() );

    SbUnoObjectRef xUnoObj = new SbUnoObject( "DefaultContext", aContextAny );
    refVar->PutObject( xUnoObj.get() );
}


// Creates a Basic wrapper object for a strongly typed Uno value
// 1. parameter: Uno type as full qualified type name, e.g. "byte[]"
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // 2 parameters needed
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    OUString aTypeName = rPar.Get(1)->GetOUString();
    SbxVariable* pVal = rPar.Get(2);

    if( aTypeName == "type" )
    {
        SbxDataType eBaseType = pVal->SbxValue::GetType();
        OUString aValTypeName;
        if( eBaseType == SbxSTRING )
        {
            aValTypeName = pVal->GetOUString();
        }
        else if( eBaseType == SbxOBJECT )
        {
            // XIdlClass?
            Reference< XIdlClass > xIdlClass;

            SbxBaseRef pObj = pVal->GetObject();
            if( auto obj = dynamic_cast<SbUnoObject*>( pObj.get() ) )
            {
                Any aUnoAny = obj->getUnoAny();
                aUnoAny >>= xIdlClass;
            }

            if( xIdlClass.is() )
            {
                aValTypeName = xIdlClass->getName();
            }
        }
        Type aType;
        bool bSuccess = implGetTypeByName( aValTypeName, aType );
        if( bSuccess )
        {
            Any aTypeAny( aType );
            SbxVariableRef refVar = rPar.Get(0);
            SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aTypeAny );
            refVar->PutObject( xUnoAnyObject.get() );
        }
        return;
    }

    // Check the type
    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    Any aRet;
    try
    {
        aRet = xTypeAccess->getByHierarchicalName( aTypeName );
    }
    catch( const NoSuchElementException& e1 )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e1, "com.sun.star.container.NoSuchElementException" ) );
        return;
    }
    Reference< XTypeDescription > xTypeDesc;
    aRet >>= xTypeDesc;
    TypeClass eTypeClass = xTypeDesc->getTypeClass();
    Type aDestType( eTypeClass, aTypeName );


    // Preconvert value
    Any aVal = sbxToUnoValueImpl( pVal );
    Any aConvertedVal = convertAny( aVal, aDestType );

    SbxVariableRef refVar = rPar.Get(0);
    SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aConvertedVal );
    refVar->PutObject( xUnoAnyObject.get() );
}


class ModuleInvocationProxy : public WeakImplHelper< XInvocation, XComponent >
{
    ::osl::Mutex        m_aMutex;
    OUString            m_aPrefix;
    SbxObjectRef        m_xScopeObj;
    bool                m_bProxyIsClassModuleObject;

    ::comphelper::OInterfaceContainerHelper2 m_aListeners;

public:
    ModuleInvocationProxy( const OUString& aPrefix, SbxObjectRef xScopeObj );

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection() override;
    virtual void SAL_CALL setValue( const OUString& rProperty, const Any& rValue ) override;
    virtual Any SAL_CALL getValue( const OUString& rProperty ) override;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& rName ) override;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& rProp ) override;

    virtual Any SAL_CALL invoke( const OUString& rFunction,
                                 const Sequence< Any >& rParams,
                                 Sequence< sal_Int16 >& rOutParamIndex,
                                 Sequence< Any >& rOutParam ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) override;
};

ModuleInvocationProxy::ModuleInvocationProxy( const OUString& aPrefix, SbxObjectRef xScopeObj )
    : m_aMutex()
    , m_aPrefix( aPrefix + "_" )
    , m_xScopeObj( xScopeObj )
    , m_aListeners( m_aMutex )
{
    m_bProxyIsClassModuleObject = xScopeObj.is() && nullptr != dynamic_cast<const SbClassModuleObject*>( xScopeObj.get() );
}

Reference< XIntrospectionAccess > SAL_CALL ModuleInvocationProxy::getIntrospection()
{
    return Reference< XIntrospectionAccess >();
}

void SAL_CALL ModuleInvocationProxy::setValue(const OUString& rProperty, const Any& rValue)
{
    if( !m_bProxyIsClassModuleObject )
        throw UnknownPropertyException();

    SolarMutexGuard guard;

    OUString aPropertyFunctionName = "Property Set "
                                   + m_aPrefix
                                   + rProperty;

    SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxClassType::Method );
    SbMethod* pMeth = dynamic_cast<SbMethod*>( p );
    if( pMeth == nullptr )
    {
        // TODO: Check vba behavior concernig missing function
        //StarBASIC::Error( ERRCODE_BASIC_NO_METHOD, aFunctionName );
        throw UnknownPropertyException();
    }

    // Setup parameter
    SbxArrayRef xArray = new SbxArray;
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    unoToSbxValue( xVar.get(), rValue );
    xArray->Put( xVar.get(), 1 );

    // Call property method
    SbxVariableRef xValue = new SbxVariable;
    pMeth->SetParameters( xArray.get() );
    pMeth->Call( xValue.get() );
    pMeth->SetParameters( nullptr );

    // TODO: OutParameter?


}

Any SAL_CALL ModuleInvocationProxy::getValue(const OUString& rProperty)
{
    if( !m_bProxyIsClassModuleObject )
    {
        throw UnknownPropertyException();
    }
    SolarMutexGuard guard;

    OUString aPropertyFunctionName = "Property Get "
                                   + m_aPrefix
                                   + rProperty;

    SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxClassType::Method );
    SbMethod* pMeth = dynamic_cast<SbMethod*>( p );
    if( pMeth == nullptr )
    {
        // TODO: Check vba behavior concernig missing function
        //StarBASIC::Error( ERRCODE_BASIC_NO_METHOD, aFunctionName );
        throw UnknownPropertyException();
    }

    // Call method
    SbxVariableRef xValue = new SbxVariable;
    pMeth->Call( xValue.get() );
    Any aRet = sbxToUnoValue( xValue.get() );
    return aRet;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasMethod( const OUString& )
{
    return false;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasProperty( const OUString& )
{
    return false;
}

Any SAL_CALL ModuleInvocationProxy::invoke( const OUString& rFunction,
                                            const Sequence< Any >& rParams,
                                            Sequence< sal_Int16 >&,
                                            Sequence< Any >& )
{
    SolarMutexGuard guard;

    Any aRet;
    SbxObjectRef xScopeObj = m_xScopeObj;
    if( !xScopeObj.is() )
    {
        return aRet;
    }
    OUString aFunctionName = m_aPrefix
                           + rFunction;

    bool bSetRescheduleBack = false;
    bool bOldReschedule = true;
    SbiInstance* pInst = GetSbData()->pInst;
    if( pInst && pInst->IsCompatibility() )
    {
        bOldReschedule = pInst->IsReschedule();
        if ( bOldReschedule )
        {
            pInst->EnableReschedule( false );
            bSetRescheduleBack = true;
        }
    }

    SbxVariable* p = xScopeObj->Find( aFunctionName, SbxClassType::Method );
    SbMethod* pMeth = dynamic_cast<SbMethod*>( p );
    if( pMeth == nullptr )
    {
        // TODO: Check vba behavior concernig missing function
        //StarBASIC::Error( ERRCODE_BASIC_NO_METHOD, aFunctionName );
        return aRet;
    }

    // Setup parameters
    SbxArrayRef xArray;
    sal_Int32 nParamCount = rParams.getLength();
    if( nParamCount )
    {
        xArray = new SbxArray;
        const Any *pArgs = rParams.getConstArray();
        for( sal_Int32 i = 0 ; i < nParamCount ; i++ )
        {
            SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
            unoToSbxValue( xVar.get(), pArgs[i] );
            xArray->Put( xVar.get(), sal::static_int_cast< sal_uInt16 >(i+1) );
        }
    }

    // Call method
    SbxVariableRef xValue = new SbxVariable;
    if( xArray.is() )
        pMeth->SetParameters( xArray.get() );
    pMeth->Call( xValue.get() );
    aRet = sbxToUnoValue( xValue.get() );
    pMeth->SetParameters( nullptr );

    if( bSetRescheduleBack )
        pInst->EnableReschedule( bOldReschedule );

    // TODO: OutParameter?

    return aRet;
}

void SAL_CALL ModuleInvocationProxy::dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    EventObject aEvent( static_cast<XComponent*>(this) );
    m_aListeners.disposeAndClear( aEvent );

    m_xScopeObj = nullptr;
}

void SAL_CALL ModuleInvocationProxy::addEventListener( const Reference< XEventListener >& xListener )
{
    m_aListeners.addInterface( xListener );
}

void SAL_CALL ModuleInvocationProxy::removeEventListener( const Reference< XEventListener >& xListener )
{
    m_aListeners.removeInterface( xListener );
}


Reference< XInterface > createComListener( const Any& aControlAny, const OUString& aVBAType,
                                           const OUString& aPrefix, const SbxObjectRef& xScopeObj )
{
    Reference< XInterface > xRet;

    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );

    Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPrefix, xScopeObj );

    Sequence<Any> args( 3 );
    args[0] = aControlAny;
    args[1] <<= aVBAType;
    args[2] <<= xProxy;

    try
    {
        xRet = xServiceMgr->createInstanceWithArgumentsAndContext(
            "com.sun.star.custom.UnoComListener",
            args, xContext );
    }
    catch( const Exception& )
    {
        implHandleAnyException( ::cppu::getCaughtException() );
    }

    return xRet;
}

typedef std::vector< WeakReference< XComponent > >  ComponentRefVector;

struct StarBasicDisposeItem
{
    StarBASIC*              m_pBasic;
    SbxArrayRef             m_pRegisteredVariables;
    ComponentRefVector      m_vComImplementsObjects;

    explicit StarBasicDisposeItem( StarBASIC* pBasic )
        : m_pBasic( pBasic )
    {
        m_pRegisteredVariables = new SbxArray();
    }
};

typedef std::vector< StarBasicDisposeItem* > DisposeItemVector;

static DisposeItemVector GaDisposeItemVector;

static DisposeItemVector::iterator lcl_findItemForBasic( StarBASIC* pBasic )
{
    DisposeItemVector::iterator it;
    for( it = GaDisposeItemVector.begin() ; it != GaDisposeItemVector.end() ; ++it )
    {
        StarBasicDisposeItem* pItem = *it;
        if( pItem->m_pBasic == pBasic )
            return it;
    }
    return GaDisposeItemVector.end();
}

static StarBasicDisposeItem* lcl_getOrCreateItemForBasic( StarBASIC* pBasic )
{
    DisposeItemVector::iterator it = lcl_findItemForBasic( pBasic );
    StarBasicDisposeItem* pItem = (it != GaDisposeItemVector.end()) ? *it : nullptr;
    if( pItem == nullptr )
    {
        pItem = new StarBasicDisposeItem( pBasic );
        GaDisposeItemVector.push_back( pItem );
    }
    return pItem;
}

void registerComponentToBeDisposedForBasic
    ( const Reference< XComponent >& xComponent, StarBASIC* pBasic )
{
    StarBasicDisposeItem* pItem = lcl_getOrCreateItemForBasic( pBasic );
    pItem->m_vComImplementsObjects.push_back( xComponent );
}

void registerComListenerVariableForBasic( SbxVariable* pVar, StarBASIC* pBasic )
{
    StarBasicDisposeItem* pItem = lcl_getOrCreateItemForBasic( pBasic );
    SbxArray* pArray = pItem->m_pRegisteredVariables.get();
    pArray->Put( pVar, pArray->Count() );
}

void disposeComVariablesForBasic( StarBASIC* pBasic )
{
    DisposeItemVector::iterator it = lcl_findItemForBasic( pBasic );
    if( it != GaDisposeItemVector.end() )
    {
        StarBasicDisposeItem* pItem = *it;

        SbxArray* pArray = pItem->m_pRegisteredVariables.get();
        sal_uInt16 nCount = pArray->Count();
        for( sal_uInt16 i = 0 ; i < nCount ; ++i )
        {
            SbxVariable* pVar = pArray->Get( i );
            pVar->ClearComListener();
        }

        ComponentRefVector& rv = pItem->m_vComImplementsObjects;
        ComponentRefVector::iterator itCRV;
        for( itCRV = rv.begin() ; itCRV != rv.end() ; ++itCRV )
        {
            Reference< XComponent > xComponent( (*itCRV).get(), UNO_QUERY );
            if (xComponent.is())
                xComponent->dispose();
        }

        delete pItem;
        GaDisposeItemVector.erase( it );
    }
}


// Handle module implements mechanism for OLE types
bool SbModule::createCOMWrapperForIface( Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject )
{
    // For now: Take first interface that allows to instantiate COM wrapper
    // TODO: Check if support for multiple interfaces is needed

    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );
    Reference< XSingleServiceFactory > xComImplementsFactory
    (
        xServiceMgr->createInstanceWithContext( "com.sun.star.custom.ComImplementsFactory", xContext ),
        UNO_QUERY
    );
    if( !xComImplementsFactory.is() )
        return false;

    bool bSuccess = false;

    SbxArray* pModIfaces = pClassData->mxIfaces.get();
    sal_uInt16 nCount = pModIfaces->Count();
    for( sal_uInt16 i = 0 ; i < nCount ; ++i )
    {
        SbxVariable* pVar = pModIfaces->Get( i );
        OUString aIfaceName = pVar->GetName();

        if( !aIfaceName.isEmpty() )
        {
            OUString aPureIfaceName = aIfaceName;
            sal_Int32 indexLastDot = aIfaceName.lastIndexOf('.');
            if ( indexLastDot > -1 )
            {
                aPureIfaceName = aIfaceName.copy( indexLastDot + 1 );
            }
            Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPureIfaceName, pProxyClassModuleObject );

            Sequence<Any> args( 2 );
            args[0] <<= aIfaceName;
            args[1] <<= xProxy;

            Reference< XInterface > xRet;
            try
            {
                xRet = xComImplementsFactory->createInstanceWithArguments( args );
                bSuccess = true;
            }
            catch( const Exception& )
            {
                implHandleAnyException( ::cppu::getCaughtException() );
            }

            if( bSuccess )
            {
                Reference< XComponent > xComponent( xProxy, UNO_QUERY );
                if( xComponent.is() )
                {
                    StarBASIC* pParentBasic = nullptr;
                    SbxObject* pCurObject = this;
                    do
                    {
                        SbxObject* pObjParent = pCurObject->GetParent();
                        pParentBasic = dynamic_cast<StarBASIC*>( pObjParent  );
                        pCurObject = pObjParent;
                    }
                    while( pParentBasic == nullptr && pCurObject != nullptr );

                    OSL_ASSERT( pParentBasic != nullptr );
                    registerComponentToBeDisposedForBasic( xComponent, pParentBasic );
                }

                o_rRetAny <<= xRet;
                break;
            }
        }
     }

    return bSuccess;
}


// Due to an incorrect behavior IE returns an object instead of a string
// in some scenarios. Calling toString at the object may correct this.
// Helper function used in sbxvalue.cxx
bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal )
{
    bool bSuccess = false;

    if( auto pUnoObj = dynamic_cast<SbUnoObject*>( pObj) )
    {
        // Only for native COM objects
        if( pUnoObj->isNativeCOMObject() )
        {
            SbxVariableRef pMeth = pObj->Find( "toString", SbxClassType::Method );
            if ( pMeth.is() )
            {
                SbxValues aRes;
                pMeth->Get( aRes );
                pVal->Put( aRes );
                bSuccess = true;
            }
        }
    }
    return bSuccess;
}

Any StructRefInfo::getValue()
{
    Any aRet;
    uno_any_destruct(
        &aRet, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    typelib_TypeDescription * pTD = nullptr;
    maType.getDescription(&pTD);
    uno_any_construct(
        &aRet, getInst(), pTD,
                reinterpret_cast< uno_AcquireFunc >(cpp_acquire) );
    typelib_typedescription_release(pTD);
    return aRet;
}

void StructRefInfo::setValue( const Any& rValue )
{
    bool bSuccess = uno_type_assignData( getInst(),
       maType.getTypeLibType(),
       const_cast<void*>(rValue.getValue()),
       rValue.getValueTypeRef(),
       reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
       reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
       reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    OSL_ENSURE(bSuccess,
        "StructRefInfo::setValue: ooops .... the value could not be assigned!");
}

OUString StructRefInfo::getTypeName() const
{
    return maType.getTypeName();
}

void* StructRefInfo::getInst()
{
    return const_cast<char *>(static_cast<char const *>(maAny.getValue()) + mnPos);
}

TypeClass StructRefInfo::getTypeClass() const
{
    return maType.getTypeClass();
}

SbUnoStructRefObject::SbUnoStructRefObject( const OUString& aName_, const StructRefInfo& rMemberInfo ) :  SbxObject( aName_ ), maMemberInfo( rMemberInfo ), mbMemberCacheInit( false )
{
   SetClassName( OUString( maMemberInfo.getTypeName() ) );
}

SbUnoStructRefObject::~SbUnoStructRefObject()
{
    for ( StructFieldInfo::iterator it = maFields.begin(), it_end = maFields.end(); it != it_end; ++it )
        delete it->second;
}

void SbUnoStructRefObject::initMemberCache()
{
    if ( mbMemberCacheInit )
        return;
    sal_Int32 nAll = 0;
    typelib_TypeDescription * pTD = nullptr;
    maMemberInfo.getType().getDescription(&pTD);
    typelib_CompoundTypeDescription * pCompTypeDescr = reinterpret_cast<typelib_CompoundTypeDescription *>(pTD);
    for ( ; pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
        nAll += pCompTypeDescr->nMembers;
    for ( pCompTypeDescr = reinterpret_cast<typelib_CompoundTypeDescription *>(pTD); pCompTypeDescr;
        pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        typelib_TypeDescriptionReference ** ppTypeRefs = pCompTypeDescr->ppTypeRefs;
        rtl_uString ** ppNames                         = pCompTypeDescr->ppMemberNames;
        sal_Int32 * pMemberOffsets                     = pCompTypeDescr->pMemberOffsets;
        for ( sal_Int32 nPos = pCompTypeDescr->nMembers; nPos--; )
        {
            OUString aName( ppNames[nPos] );
            maFields[ aName ] = new StructRefInfo( maMemberInfo.getRootAnyRef(), ppTypeRefs[nPos], maMemberInfo.getPos() + pMemberOffsets[nPos] );
        }
    }
    typelib_typedescription_release(pTD);
    mbMemberCacheInit = true;
}

SbxVariable* SbUnoStructRefObject::Find( const OUString& rName, SbxClassType t )
{
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if ( !pRes )
    {
        if ( !mbMemberCacheInit )
            initMemberCache();
        StructFieldInfo::iterator it = maFields.find( OUString( rName ).toAsciiUpperCase() );
        if ( it != maFields.end() )
        {
            SbxDataType eSbxType;
            eSbxType = unoToSbxType( it->second->getTypeClass() );
            SbxDataType eRealSbxType = eSbxType;
            Property aProp;
            aProp.Name = rName;
            aProp.Type = css::uno::Type( it->second->getTypeClass(), it->second->getTypeName() );
            SbUnoProperty* pProp = new SbUnoProperty( rName, eSbxType, eRealSbxType, aProp, 0, false, ( aProp.Type.getTypeClass() == css::uno::TypeClass_STRUCT) );
            SbxVariableRef xVarRef = pProp;
            QuickInsert( xVarRef.get() );
            pRes = xVarRef.get();
        }
    }

    if( !pRes )
    {
        if( rName.equalsIgnoreAsciiCase(ID_DBG_SUPPORTEDINTERFACES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_PROPERTIES) ||
            rName.equalsIgnoreAsciiCase(ID_DBG_METHODS) )
        {
            // Create
            implCreateDbgProperties();

            // Now they have to be found regular
            pRes = SbxObject::Find( rName, SbxClassType::DontCare );
        }
    }

    return pRes;
}

// help method to create the dbg_-Properties
void SbUnoStructRefObject::implCreateDbgProperties()
{
    Property aProp;

    // Id == -1: display the implemented interfaces corresponding the ClassProvider
    SbxVariableRef xVarRef = new SbUnoProperty( ID_DBG_SUPPORTEDINTERFACES, SbxSTRING, SbxSTRING, aProp, -1, false, false );
    QuickInsert( xVarRef.get() );

    // Id == -2: output the properties
    xVarRef = new SbUnoProperty( ID_DBG_PROPERTIES, SbxSTRING, SbxSTRING, aProp, -2, false, false );
    QuickInsert( xVarRef.get() );

    // Id == -3: output the Methods
    xVarRef = new SbUnoProperty( ID_DBG_METHODS, SbxSTRING, SbxSTRING, aProp, -3, false, false );
    QuickInsert( xVarRef.get() );
}

void SbUnoStructRefObject::implCreateAll()
{
     // throw away all existing methods and properties
    pMethods   = new SbxArray;
    pProps     = new SbxArray;

    if (!mbMemberCacheInit)
        initMemberCache();

    for ( StructFieldInfo::iterator it = maFields.begin(), it_end = maFields.end(); it != it_end; ++it )
    {
        const OUString& rName = it->first;
        SbxDataType eSbxType;
        eSbxType = unoToSbxType( it->second->getTypeClass() );
        SbxDataType eRealSbxType = eSbxType;
        Property aProp;
        aProp.Name = rName;
        aProp.Type = css::uno::Type( it->second->getTypeClass(), it->second->getTypeName() );
        SbUnoProperty* pProp = new SbUnoProperty( rName, eSbxType, eRealSbxType, aProp, 0, false, ( aProp.Type.getTypeClass() == css::uno::TypeClass_STRUCT) );
        SbxVariableRef xVarRef = pProp;
        QuickInsert( xVarRef.get() );
    }

    // Create Dbg_-Properties
    implCreateDbgProperties();
}

 // output the value
Any SbUnoStructRefObject::getUnoAny()
{
    return maMemberInfo.getValue();
}

OUString SbUnoStructRefObject::Impl_DumpProperties()
{
    OUStringBuffer aRet;
    aRet.append("Properties of object ");
    aRet.append( getDbgObjectName() );

    sal_uInt16 nPropCount = pProps->Count();
    sal_uInt16 nPropsPerLine = 1 + nPropCount / 30;
    for( sal_uInt16 i = 0; i < nPropCount; i++ )
    {
        SbxVariable* pVar = pProps->Get( i );
        if( pVar )
        {
            OUStringBuffer aPropStr;
            if( (i % nPropsPerLine) == 0 )
            {
                aPropStr.append( "\n" );
            }
            // output the type and name
            // Is it in Uno a sequence?
            SbxDataType eType = pVar->GetFullType();

            OUString aName( pVar->GetName() );
            StructFieldInfo::iterator it = maFields.find( aName );

            if ( it != maFields.end() )
            {
                const StructRefInfo& rPropInfo = *it->second;

                if( eType == SbxOBJECT )
                {
                    if( rPropInfo.getTypeClass() == TypeClass_SEQUENCE )
                    {
                        eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
                    }
                }
            }
            aPropStr.append( Dbg_SbxDataType2String( eType ) );

            aPropStr.append( " " );
            aPropStr.append( pVar->GetName() );

            if( i == nPropCount - 1 )
            {
                aPropStr.append( "\n" );
            }
            else
            {
                aPropStr.append( "; " );
            }
            aRet.append( aPropStr.makeStringAndClear() );
        }
    }
    return aRet.makeStringAndClear();
}

void SbUnoStructRefObject::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( !mbMemberCacheInit )
        initMemberCache();
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbUnoProperty* pProp = dynamic_cast<SbUnoProperty*>( pVar );
        if( pProp )
        {
             StructFieldInfo::iterator it =  maFields.find(  pProp->GetName() );
            // handle get/set of members of struct
            if( pHint->GetId() == SfxHintId::BasicDataWanted )
            {
                // Test-Properties
                sal_Int32 nId = pProp->nId;
                if( nId < 0 )
                {
                    // Id == -1: Display implemented interfaces according the ClassProvider
                    if( nId == -1 )     // Property ID_DBG_SUPPORTEDINTERFACES"
                    {
                        OUStringBuffer aRet;
                        aRet.append( ID_DBG_SUPPORTEDINTERFACES );
                        aRet.append( " not available.\n(TypeClass is not TypeClass_INTERFACE)\n" );

                        pVar->PutString( aRet.makeStringAndClear() );
                    }
                    // Id == -2: output properties
                    else if( nId == -2 )        // Property ID_DBG_PROPERTIES
                    {
                        // by now all properties must be established
                        implCreateAll();
                        OUString aRetStr = Impl_DumpProperties();
                        pVar->PutString( aRetStr );
                    }
                    // Id == -3: output the methods
                    else if( nId == -3 )        // Property ID_DBG_METHODS
                    {
                        // by now all properties must be established
                        implCreateAll();
                        OUStringBuffer aRet;
                        aRet.append("Methods of object ");
                        aRet.append( getDbgObjectName() );
                        aRet.append( "\nNo methods found\n" );
                        pVar->PutString( aRet.makeStringAndClear() );
                    }
                    return;
                }

                if ( it != maFields.end() )
                {
                    Any aRetAny = it->second->getValue();
                    unoToSbxValue( pVar, aRetAny );
                }
                else
                    StarBASIC::Error( ERRCODE_BASIC_PROPERTY_NOT_FOUND );
            }
            else if( pHint->GetId() == SfxHintId::BasicDataChanged )
            {
                if ( it != maFields.end() )
                {
                    // take over the value from Uno to Sbx
                    Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                    it->second->setValue( aAnyValue );
                }
                else
                    StarBASIC::Error( ERRCODE_BASIC_PROPERTY_NOT_FOUND );
            }
        }
        else
           SbxObject::Notify( rBC, rHint );
    }
}

StructRefInfo SbUnoStructRefObject::getStructMember( const OUString& rMemberName )
{
    if (!mbMemberCacheInit)
    {
        initMemberCache();
    }
    StructFieldInfo::iterator it = maFields.find( rMemberName );

    css::uno::Type aFoundType;
    sal_Int32 nFoundPos = -1;

    if ( it != maFields.end() )
    {
        aFoundType = it->second->getType();
        nFoundPos = it->second->getPos();
    }
    StructRefInfo aRet( maMemberInfo.getRootAnyRef(), aFoundType, nFoundPos );
    return aRet;
}

OUString SbUnoStructRefObject::getDbgObjectName()
{
    OUString aName = GetClassName();
    if( aName.isEmpty() )
    {
        aName += "Unknown";
    }
    OUStringBuffer aRet;
    if( aName.getLength() > 20 )
    {
        aRet.append( "\n" );
    }
    aRet.append( "\"" );
    aRet.append( aName );
    aRet.append( "\":" );
    return aRet.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
