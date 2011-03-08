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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/errcode.hxx>
#include <svl/hint.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/extract.hxx>
#include <comphelper/processfactory.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>

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
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XIdlArray.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClassProvider.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/bridge/oleautomation/NamedArgument.hpp>
#include <com/sun/star/bridge/oleautomation/Date.hpp>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <com/sun/star/bridge/oleautomation/Currency.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>
#include <com/sun/star/script/XAutomationInvocation.hpp>

using com::sun::star::uno::Reference;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::container;
using namespace com::sun::star::bridge;
using namespace cppu;


#include<basic/sbstar.hxx>
#include<basic/sbuno.hxx>
#include<basic/sberrors.hxx>
#include<sbunoobj.hxx>
#include"sbjsmod.hxx"
#include<basic/basmgr.hxx>
#include<sbintern.hxx>
#include<runtime.hxx>

#include<math.h>
#include <boost/unordered_map.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>

TYPEINIT1(SbUnoMethod,SbxMethod)
TYPEINIT1(SbUnoProperty,SbxProperty)
TYPEINIT1(SbUnoObject,SbxObject)
TYPEINIT1(SbUnoClass,SbxObject)
TYPEINIT1(SbUnoService,SbxObject)
TYPEINIT1(SbUnoServiceCtor,SbxMethod)
TYPEINIT1(SbUnoSingleton,SbxObject)

typedef WeakImplHelper1< XAllListener > BasicAllListenerHelper;

// Flag to go via invocation
//#define INVOCATION_ONLY


// Identifier fuer die dbg_-Properies als Strings anlegen
static char const ID_DBG_SUPPORTEDINTERFACES[] = "Dbg_SupportedInterfaces";
static char const ID_DBG_PROPERTIES[] = "Dbg_Properties";
static char const ID_DBG_METHODS[] = "Dbg_Methods";

static ::rtl::OUString aSeqLevelStr( RTL_CONSTASCII_USTRINGPARAM("[]") );
static ::rtl::OUString defaultNameSpace( RTL_CONSTASCII_USTRINGPARAM("ooo.vba") );

// Gets the default property for an uno object. Note: There is some
// redirection built in. The property name specifies the name
// of the default property.

bool SbUnoObject::getDefaultPropName( SbUnoObject* pUnoObj, String& sDfltProp )
{
    bool result = false;
    Reference< XDefaultProperty> xDefaultProp( pUnoObj->maTmpUnoObj, UNO_QUERY );
    if ( xDefaultProp.is() )
    {
        sDfltProp = xDefaultProp->getDefaultPropertyName();
        if ( sDfltProp.Len() )
            result = true;
    }
    return result;
}

SbxVariable* getDefaultProp( SbxVariable* pRef )
{
    SbxVariable* pDefaultProp = NULL;
    if ( pRef->GetType() == SbxOBJECT )
    {
          SbxObject* pObj = PTR_CAST(SbxObject,(SbxVariable*) pRef);
        if ( !pObj )
        {
            SbxBase* pObjVarObj = pRef->GetObject();
            pObj = PTR_CAST(SbxObject,pObjVarObj);
        }
        if ( pObj && pObj->ISA(SbUnoObject) )
        {
            SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*)pObj);
            pDefaultProp = pUnoObj->GetDfltProperty();
        }
    }
    return pDefaultProp;
}

void SetSbUnoObjectDfltPropName( SbxObject* pObj )
{
    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*) pObj);
    if ( pUnoObj )
    {
        String sDfltPropName;

        if ( SbUnoObject::getDefaultPropName( pUnoObj, sDfltPropName ) )
        {
            OSL_TRACE("SetSbUnoObjectDfltPropName setting dflt prop for %s", rtl::OUStringToOString( pObj->GetName(), RTL_TEXTENCODING_UTF8 ).getStr() );
            pUnoObj->SetDfltProperty( sDfltPropName );
        }
    }
}

Reference< XComponentContext > getComponentContext_Impl( void )
{
    static Reference< XComponentContext > xContext;

    // Do we have already CoreReflection; if not obtain it
    if( !xContext.is() )
    {
        Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
        Reference< XPropertySet > xProps( xFactory, UNO_QUERY );
        OSL_ASSERT( xProps.is() );
        if (xProps.is())
        {
            xProps->getPropertyValue(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
            OSL_ASSERT( xContext.is() );
        }
    }
    return xContext;
}

// save CoreReflection statically
Reference< XIdlReflection > getCoreReflection_Impl( void )
{
    static Reference< XIdlReflection > xCoreReflection;

    // Do we have already CoreReflection; if not obtain it
    if( !xCoreReflection.is() )
    {
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            xContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection") ) )
                    >>= xCoreReflection;
            OSL_ENSURE( xCoreReflection.is(), "### CoreReflection singleton not accessable!?" );
        }
        if( !xCoreReflection.is() )
        {
            throw DeploymentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection singleton not accessable") ),
                Reference< XInterface >() );
        }
    }
    return xCoreReflection;
}

// save CoreReflection statically
Reference< XHierarchicalNameAccess > getCoreReflection_HierarchicalNameAccess_Impl( void )
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
Reference< XHierarchicalNameAccess > getTypeProvider_Impl( void )
{
    static Reference< XHierarchicalNameAccess > xAccess;

    // Do we have already CoreReflection; if not obtain it
    if( !xAccess.is() )
    {
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            xContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theTypeDescriptionManager") ) )
                    >>= xAccess;
            OSL_ENSURE( xAccess.is(), "### TypeDescriptionManager singleton not accessable!?" );
        }
        if( !xAccess.is() )
        {
            throw DeploymentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM
                    ("/singletons/com.sun.star.reflection.theTypeDescriptionManager singleton not accessable") ),
                Reference< XInterface >() );
        }
    }
    return xAccess;
}

// Hold TypeConverter statically
Reference< XTypeConverter > getTypeConverter_Impl( void )
{
    static Reference< XTypeConverter > xTypeConverter;

    // Do we have already CoreReflection; if not obtain it
    if( !xTypeConverter.is() )
    {
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
            xTypeConverter = Reference<XTypeConverter>(
                xSMgr->createInstanceWithContext(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter")),
                        xContext ), UNO_QUERY );
        }
        if( !xTypeConverter.is() )
        {
            throw DeploymentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM
                    ("com.sun.star.script.Converter service not accessable") ),
                Reference< XInterface >() );
        }
    }
    return xTypeConverter;
}


// #111851 factory function to create an OLE object
SbUnoObject* createOLEObject_Impl( const String& aType )
{
    static Reference< XMultiServiceFactory > xOLEFactory;
    static bool bNeedsInit = true;

    if( bNeedsInit )
    {
        bNeedsInit = false;

        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
            xOLEFactory = Reference<XMultiServiceFactory>(
                xSMgr->createInstanceWithContext(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.OleObjectFactory")),
                        xContext ), UNO_QUERY );
        }
    }

    SbUnoObject* pUnoObj = NULL;
    if( xOLEFactory.is() )
    {
        // some type names available in VBA can not be directly used in COM
        ::rtl::OUString aOLEType = aType;
        if ( aOLEType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SAXXMLReader30" ) ) ) )
            aOLEType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Msxml2.SAXXMLReader.3.0" ) );

        Reference< XInterface > xOLEObject = xOLEFactory->createInstance( aOLEType );
        if( xOLEObject.is() )
        {
            Any aAny;
            aAny <<= xOLEObject;
            pUnoObj = new SbUnoObject( aType, aAny );
        }
    }
    return pUnoObj;
}


namespace
{
    void lcl_indent( ::rtl::OUStringBuffer& _inout_rBuffer, sal_Int32 _nLevel )
    {
        while ( _nLevel-- > 0 )
            _inout_rBuffer.appendAscii( "  " );
    }
}

void implAppendExceptionMsg( ::rtl::OUStringBuffer& _inout_rBuffer, const Exception& _e, const ::rtl::OUString& _rExceptionType, sal_Int32 _nLevel )
{
    _inout_rBuffer.appendAscii( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.appendAscii( "Type: " );

    if ( _rExceptionType.getLength() == 0 )
        _inout_rBuffer.appendAscii( "Unknown" );
    else
        _inout_rBuffer.append( _rExceptionType );

    _inout_rBuffer.appendAscii( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.appendAscii( "Message: " );
    _inout_rBuffer.append( _e.Message );

}

// construct an error message for the exception
::rtl::OUString implGetExceptionMsg( const Exception& e, const ::rtl::OUString& aExceptionType_ )
{
    ::rtl::OUStringBuffer aMessageBuf;
    implAppendExceptionMsg( aMessageBuf, e, aExceptionType_, 0 );
    return aMessageBuf.makeStringAndClear();
}

String implGetExceptionMsg( const Any& _rCaughtException )
{
    OSL_PRECOND( _rCaughtException.getValueTypeClass() == TypeClass_EXCEPTION, "implGetExceptionMsg: illegal argument!" );
    if ( _rCaughtException.getValueTypeClass() != TypeClass_EXCEPTION )
        return String();

    return implGetExceptionMsg( *static_cast< const Exception* >( _rCaughtException.getValue() ), _rCaughtException.getValueTypeName() );
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
    catch( CannotConvertException& e2 )
    {
        String aCannotConvertExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.IllegalArgumentException" ) );
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e2, aCannotConvertExceptionName ) );
        return aConvertedVal;
    }
    return aConvertedVal;
}


// #105565 Special Object to wrap a strongly typed Uno Any
TYPEINIT1(SbUnoAnyObject,SbxObject)


// TODO: source out later
Reference<XIdlClass> TypeToIdlClass( const Type& rType )
{
    // register void as default class
    Reference<XIdlClass> xRetClass;
    typelib_TypeDescription * pTD = 0;
    rType.getDescription( &pTD );

    if( pTD )
    {
        ::rtl::OUString sOWName( pTD->pTypeName );
        Reference< XIdlReflection > xRefl = getCoreReflection_Impl();
        xRetClass = xRefl->forName( sOWName );
    }
    return xRetClass;
}

// Exception type unknown
template< class EXCEPTION >
String implGetExceptionMsg( const EXCEPTION& e )
{
    return implGetExceptionMsg( e, ::getCppuType( &e ).getTypeName() );
}

// Error-Message fuer WrappedTargetExceptions
String implGetWrappedMsg( const WrappedTargetException& e )
{
    String aMsg;
    Any aWrappedAny = e.TargetException;
    Type aExceptionType = aWrappedAny.getValueType();

    // Really an Exception?
    if( aExceptionType.getTypeClass() == TypeClass_EXCEPTION )
    {
        Exception& e_ = *( (Exception*)aWrappedAny.getValue() );
        aMsg = implGetExceptionMsg( e_, String( aExceptionType.getTypeName() ) );
    }
    // Otherwise use WrappedTargetException itself
    else
    {
        aMsg = implGetExceptionMsg( e );
    }

    return aMsg;
}

void implHandleBasicErrorException( BasicErrorException& e )
{
    SbError nError = StarBASIC::GetSfxFromVBError( (USHORT)e.ErrorCode );
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
    ::rtl::OUStringBuffer aMessageBuf;

    // Add for VBA, to get the correct error code and message.
    if ( SbiRuntime::isVBAEnabled() )
    {
        if ( aExamine >>= aBasicError )
        {
            if ( aBasicError.ErrorCode != 0 )
            {
                nError = StarBASIC::GetSfxFromVBError( (USHORT) aBasicError.ErrorCode );
                if ( nError == 0 )
                {
                    nError = (SbError) aBasicError.ErrorCode;
                }
                aMessageBuf.append( aBasicError.ErrorMessageArgument );
                aExamine.clear();
            }
        }

        IndexOutOfBoundsException aIdxOutBndsExp;
        if ( aExamine >>= aIdxOutBndsExp )
        {
            nError = SbERR_OUT_OF_RANGE;
            aExamine.clear();
        }
    }
    // End add

    // strip any other WrappedTargetException instances, but this time preserve the error messages.
    WrappedTargetException aWrapped;
    sal_Int32 nLevel = 0;
    while ( aExamine >>= aWrapped )
    {
        // special handling for BasicErrorException errors
        if ( aWrapped.TargetException >>= aBasicError )
        {
            nError = StarBASIC::GetSfxFromVBError( (USHORT)aBasicError.ErrorCode );
            aMessageBuf.append( aBasicError.ErrorMessageArgument );
            aExamine.clear();
            break;
        }

        // append this round's message
        implAppendExceptionMsg( aMessageBuf, aWrapped, aExamine.getValueTypeName(), nLevel );
        if ( aWrapped.TargetException.getValueTypeClass() == TypeClass_EXCEPTION )
            // there is a next chain element
            aMessageBuf.appendAscii( "\nTargetException:" );

        // next round
        aExamine = aWrapped.TargetException;
        ++nLevel;
    }

    if ( aExamine.getValueTypeClass() == TypeClass_EXCEPTION )
    {
        // the last element in the chain is still an exception, but no WrappedTargetException
        implAppendExceptionMsg( aMessageBuf, *static_cast< const Exception* >( aExamine.getValue() ), aExamine.getValueTypeName(), nLevel );
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

        /* first we leave the following types out
        case TypeClass_SERVICE:         break;
        case TypeClass_CLASS:           break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_ARRAY:           break;
        */
        case TypeClass_ENUM:            eRetType = SbxLONG;     break;
        case TypeClass_SEQUENCE:
            eRetType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
            break;

        /*
        case TypeClass_VOID:            break;
        case TypeClass_UNKNOWN:         break;
        */

        case TypeClass_ANY:             eRetType = SbxVARIANT;  break;
        case TypeClass_BOOLEAN:         eRetType = SbxBOOL;     break;
        case TypeClass_CHAR:            eRetType = SbxCHAR;     break;
        case TypeClass_STRING:          eRetType = SbxSTRING;   break;
        case TypeClass_FLOAT:           eRetType = SbxSINGLE;   break;
        case TypeClass_DOUBLE:          eRetType = SbxDOUBLE;   break;
        //case TypeClass_OCTET:                                 break;
        case TypeClass_BYTE:            eRetType = SbxINTEGER;  break;
        //case TypeClass_INT:               eRetType = SbxINT;  break;
        case TypeClass_SHORT:           eRetType = SbxINTEGER;  break;
        case TypeClass_LONG:            eRetType = SbxLONG;     break;
        case TypeClass_HYPER:           eRetType = SbxSALINT64; break;
        //case TypeClass_UNSIGNED_OCTET:                        break;
        case TypeClass_UNSIGNED_SHORT:  eRetType = SbxUSHORT;   break;
        case TypeClass_UNSIGNED_LONG:   eRetType = SbxULONG;    break;
        case TypeClass_UNSIGNED_HYPER:  eRetType = SbxSALUINT64;break;
        //case TypeClass_UNSIGNED_INT:  eRetType = SbxUINT;     break;
        //case TypeClass_UNSIGNED_BYTE: eRetType = SbxUSHORT;   break;
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

static void implSequenceToMultiDimArray( SbxDimArray*& pArray, Sequence< sal_Int32 >& indices, Sequence< sal_Int32 >& sizes, const Any& aValue, sal_Int32& dimension, sal_Bool bIsZeroIndex, Type* pType = NULL )
{
    Type aType = aValue.getValueType();
    TypeClass eTypeClass = aType.getTypeClass();

    sal_Int32 dimCopy = dimension;

    if ( eTypeClass == TypeClass_SEQUENCE )
    {
        Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
        Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
        typelib_TypeDescription * pTD = 0;
        aType.getDescription( &pTD );
        Type aElementType( ((typelib_IndirectTypeDescription *)pTD)->pType );
        ::typelib_typedescription_release( pTD );

        sal_Int32 nLen = xIdlArray->getLen( aValue );
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            Any aElementAny = xIdlArray->get( aValue, (UINT32)index );
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
            StarBASIC::Error( SbERR_INVALID_OBJECT );
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
            SbxVariableRef xVar = new SbxVariable( eSbxElementType );
            unoToSbxValue( (SbxVariable*)xVar, aValue );

            sal_Int32* pIndices = indices.getArray();
            pArray->Put32(  (SbxVariable*)xVar, pIndices );

        }
    }
}

void unoToSbxValue( SbxVariable* pVar, const Any& aValue )
{
    Type aType = aValue.getValueType();
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
            String aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aClassAny );
            SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

            // If the object is invalid deliver zero
            if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
            {
                pVar->PutObject( NULL );
            }
            else
            {
                pVar->PutObject( xWrapper );
            }
        }
        break;
        // Interfaces and  Structs must be wrapped in a SbUnoObject
        case TypeClass_INTERFACE:
        case TypeClass_STRUCT:
        case TypeClass_EXCEPTION:
        {
            if( eTypeClass == TypeClass_STRUCT )
            {
                ArrayWrapper aWrap;
                if ( (aValue >>= aWrap) )
                {
                    SbxDimArray* pArray = NULL;
                    Sequence< sal_Int32 > indices;
                    Sequence< sal_Int32 > sizes;
                    sal_Int32 dimension = 0;
                    implSequenceToMultiDimArray( pArray, indices, sizes, aWrap.Array, dimension, aWrap.IsZeroIndex );
                    if ( pArray )
                    {
                        SbxDimArrayRef xArray = pArray;
                        USHORT nFlags = pVar->GetFlags();
                        pVar->ResetFlag( SBX_FIXED );
                        pVar->PutObject( (SbxDimArray*)xArray );
                        pVar->SetFlags( nFlags );
                    }
                    else
                        pVar->PutEmpty();
                    break;
                }
                else
                {
                    SbiInstance* pInst = pINST;
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
            String aName;
            SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aValue );
            //If this is called externally e.g. from the scripting
            //framework then there is no 'active' runtime the default property will not be set up
            //only a vba object will have XDefaultProp set anyway so... this
            //test seems a bit of overkill
            //if ( SbiRuntime::isVBAEnabled() )
            {
                String sDfltPropName;

                if ( SbUnoObject::getDefaultPropName( pSbUnoObject, sDfltPropName ) )
                        pSbUnoObject->SetDfltProperty( sDfltPropName );
            }
            SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

            // If the object is invalid deliver zero
            if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
            {
                pVar->PutObject( NULL );
            }
            else
            {
                pVar->PutObject( xWrapper );
            }
        }
        break;

        /* folgende Typen lassen wir erstmal weg
        case TypeClass_SERVICE:         break;
        case TypeClass_CLASS:           break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_ENUM:            break;
        case TypeClass_ARRAY:           break;
        */

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

            typelib_TypeDescription * pTD = 0;
            aType.getDescription( &pTD );
            OSL_ASSERT( pTD && pTD->eTypeClass == typelib_TypeClass_SEQUENCE );
            Type aElementType( ((typelib_IndirectTypeDescription *)pTD)->pType );
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
                    Any aElementAny = xIdlArray->get( aValue, (UINT32)i );
                    SbxVariableRef xVar = new SbxVariable( eSbxElementType );
                    unoToSbxValue( (SbxVariable*)xVar, aElementAny );

                    // put into the Array
                    xArray->Put32( (SbxVariable*)xVar, &i );
                }
            }
            else
            {
                xArray->unoAddDim( 0, -1 );
            }

            // return the Array
            USHORT nFlags = pVar->GetFlags();
            pVar->ResetFlag( SBX_FIXED );
            pVar->PutObject( (SbxDimArray*)xArray );
            pVar->SetFlags( nFlags );

            // The parameter mustn't deleted here
            //pVar->SetParameters( NULL );
        }
        break;

        /*
        case TypeClass_VOID:            break;
        case TypeClass_UNKNOWN:         break;

        case TypeClass_ANY:
        {
            // get Any and convert it
            //Any* pAny = (Any*)aValue.get();
            //if( pAny )
                //unoToSbxValue( pVar, *pAny );
        }
        break;
        */

        case TypeClass_BOOLEAN:         pVar->PutBool( *(sal_Bool*)aValue.getValue() ); break;
        case TypeClass_CHAR:
        {
            pVar->PutChar( *(sal_Unicode*)aValue.getValue() );
            break;
        }
        case TypeClass_STRING:          { ::rtl::OUString val; aValue >>= val; pVar->PutString( String( val ) ); }  break;
        case TypeClass_FLOAT:           { float val = 0; aValue >>= val; pVar->PutSingle( val ); } break;
        case TypeClass_DOUBLE:          { double val = 0; aValue >>= val; pVar->PutDouble( val ); } break;
        //case TypeClass_OCTET:         break;
        case TypeClass_BYTE:            { sal_Int8 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        //case TypeClass_INT:           break;
        case TypeClass_SHORT:           { sal_Int16 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
        case TypeClass_LONG:            { sal_Int32 val = 0; aValue >>= val; pVar->PutLong( val ); } break;
        case TypeClass_HYPER:           { sal_Int64 val = 0; aValue >>= val; pVar->PutInt64( val ); } break;
        //case TypeClass_UNSIGNED_OCTET:break;
        case TypeClass_UNSIGNED_SHORT:  { sal_uInt16 val = 0; aValue >>= val; pVar->PutUShort( val ); } break;
        case TypeClass_UNSIGNED_LONG:   { sal_uInt32 val = 0; aValue >>= val; pVar->PutULong( val ); } break;
        case TypeClass_UNSIGNED_HYPER:  { sal_uInt64 val = 0; aValue >>= val; pVar->PutUInt64( val ); } break;
        //case TypeClass_UNSIGNED_INT:  break;
        //case TypeClass_UNSIGNED_BYTE: break;
        default:                        pVar->PutEmpty();                       break;
    }
}

// Deliver the reflection for Sbx types
Type getUnoTypeForSbxBaseType( SbxDataType eType )
{
    Type aRetType = getCppuVoidType();
    switch( eType )
    {
        //case SbxEMPTY:        eRet = TypeClass_VOID; break;
        case SbxNULL:       aRetType = ::getCppuType( (const Reference< XInterface > *)0 ); break;
        case SbxINTEGER:    aRetType = ::getCppuType( (sal_Int16*)0 ); break;
        case SbxLONG:       aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxSINGLE:     aRetType = ::getCppuType( (float*)0 ); break;
        case SbxDOUBLE:     aRetType = ::getCppuType( (double*)0 ); break;
        case SbxCURRENCY:   aRetType = ::getCppuType( (oleautomation::Currency*)0 ); break;
        case SbxDECIMAL:    aRetType = ::getCppuType( (oleautomation::Decimal*)0 ); break;
        case SbxDATE:       {
                            SbiInstance* pInst = pINST;
                            if( pInst && pInst->IsCompatibility() )
                                aRetType = ::getCppuType( (double*)0 );
                            else
                                aRetType = ::getCppuType( (oleautomation::Date*)0 );
                            }
                            break;
        // case SbxDATE:        aRetType = ::getCppuType( (double*)0 ); break;
        case SbxSTRING:     aRetType = ::getCppuType( (::rtl::OUString*)0 ); break;
        //case SbxOBJECT:   break;
        //case SbxERROR:    break;
        case SbxBOOL:       aRetType = ::getCppuType( (sal_Bool*)0 ); break;
        case SbxVARIANT:    aRetType = ::getCppuType( (Any*)0 ); break;
        //case SbxDATAOBJECT: break;
        case SbxCHAR:       aRetType = ::getCppuType( (sal_Unicode*)0 ); break;
        case SbxBYTE:       aRetType = ::getCppuType( (sal_Int8*)0 ); break;
        case SbxUSHORT:     aRetType = ::getCppuType( (sal_uInt16*)0 ); break;
        case SbxULONG:      aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        // map machine-dependent ones on hyper for secureness
        case SbxINT:        aRetType = ::getCppuType( (sal_Int32*)0 ); break;
        case SbxUINT:       aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
        //case SbxVOID:     break;
        //case SbxHRESULT:  break;
        //case SbxPOINTER:  break;
        //case SbxDIMARRAY: break;
        //case SbxCARRAY:   break;
        //case SbxUSERDEF:  break;
        //case SbxLPSTR:    break;
        //case SbxLPWSTR:   break;
        //case SbxCoreSTRING: break;
        default: break;
    }
    return aRetType;
}

// Converting of Sbx to Uno without a know target class for TypeClass_ANY
Type getUnoTypeForSbxValue( SbxValue* pVal )
{
    Type aRetType = getCppuVoidType();
    if( !pVal )
        return aRetType;

    // convert SbxType to Uno
    SbxDataType eBaseType = pVal->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVal->GetObject();
        if( !xObj )
        {
            // #109936 No error any more
            // StarBASIC::Error( SbERR_INVALID_OBJECT );
            aRetType = getCppuType( static_cast<Reference<XInterface> *>(0) );
            return aRetType;
        }

        if( xObj->ISA(SbxDimArray) )
        {
            SbxBase* pObj = (SbxBase*)xObj;
            SbxDimArray* pArray = (SbxDimArray*)pObj;

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
                    sal_Bool bNeedsInit = sal_True;

                    INT32 nSize = nUpper - nLower + 1;
                    INT32 nIdx = nLower;
                    for( INT32 i = 0 ; i < nSize ; i++,nIdx++ )
                    {
                        SbxVariableRef xVar = pArray->Get32( &nIdx );
                        Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                // #88522
                                // if only first element is void: different types  -> []any
                                // if all elements are void: []void is not allowed -> []any
                                aElementType = getCppuType( (Any*)0 );
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = sal_False;
                        }
                        else if( aElementType != aType )
                        {
                            // different types -> AnySequence
                            aElementType = getCppuType( (Any*)0 );
                            break;
                        }
                    }
                }

                ::rtl::OUString aSeqTypeName( aSeqLevelStr );
                aSeqTypeName += aElementType.getTypeName();
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
            }
            // #i33795 Map also multi dimensional arrays to corresponding sequences
            else if( nDims > 1 )
            {
                if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
                {
                    // For this check the array's dim structure does not matter
                    UINT32 nFlatArraySize = pArray->Count32();

                    sal_Bool bNeedsInit = sal_True;
                    for( UINT32 i = 0 ; i < nFlatArraySize ; i++ )
                    {
                        SbxVariableRef xVar = pArray->SbxArray::Get32( i );
                        Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
                        if( bNeedsInit )
                        {
                            if( aType.getTypeClass() == TypeClass_VOID )
                            {
                                // if only first element is void: different types  -> []any
                                // if all elements are void: []void is not allowed -> []any
                                aElementType = getCppuType( (Any*)0 );
                                break;
                            }
                            aElementType = aType;
                            bNeedsInit = sal_False;
                        }
                        else if( aElementType != aType )
                        {
                            // different types -> AnySequence
                            aElementType = getCppuType( (Any*)0 );
                            break;
                        }
                    }
                }

                ::rtl::OUString aSeqTypeName;
                for( short iDim = 0 ; iDim < nDims ; iDim++ )
                    aSeqTypeName += aSeqLevelStr;
                aSeqTypeName += aElementType.getTypeName();
                aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
            }
        }
        // No array, but ...
        else if( xObj->ISA(SbUnoObject) )
        {
            aRetType = ((SbUnoObject*)(SbxBase*)xObj)->getUnoAny().getValueType();
        }
        // SbUnoAnyObject?
        else if( xObj->ISA(SbUnoAnyObject) )
        {
            aRetType = ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue().getValueType();
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

// Declaration converting of Sbx to Uno with known target class
Any sbxToUnoValue( SbxVariable* pVar, const Type& rType, Property* pUnoProperty = NULL );

// converting of Sbx to Uno without known target class for TypeClass_ANY
Any sbxToUnoValueImpl( SbxVariable* pVar, bool bBlockConversionToSmallestType = false )
{
    SbxDataType eBaseType = pVar->SbxValue::GetType();
    if( eBaseType == SbxOBJECT )
    {
        SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
        if( xObj.Is() )
        {
            if( xObj->ISA(SbUnoAnyObject) )
                return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
            if( xObj->ISA(SbClassModuleObject) )
            {
                Any aRetAny;
                SbClassModuleObject* pClassModuleObj = (SbClassModuleObject*)(SbxBase*)xObj;
                SbModule* pClassModule = pClassModuleObj->getClassModule();
                if( pClassModule->createCOMWrapperForIface( aRetAny, pClassModuleObj ) )
                    return aRetAny;
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
                if( d == floor( d ) )
                {
                    if( d >= -128 && d <= 127 )
                        aType = ::getCppuType( (sal_Int8*)0 );
                    else if( d >= SbxMININT && d <= SbxMAXINT )
                        aType = ::getCppuType( (sal_Int16*)0 );
                    else if( d >= -SbxMAXLNG && d <= SbxMAXLNG )
                        aType = ::getCppuType( (sal_Int32*)0 );
                }
                break;
            }
            case TypeClass_SHORT:
            {
                sal_Int16 n = pVar->GetInteger();
                if( n >= -128 && n <= 127 )
                    aType = ::getCppuType( (sal_Int8*)0 );
                break;
            }
            case TypeClass_LONG:
            {
                sal_Int32 n = pVar->GetLong();
                if( n >= -128 && n <= 127 )
                    aType = ::getCppuType( (sal_Int8*)0 );
                else if( n >= SbxMININT && n <= SbxMAXINT )
                    aType = ::getCppuType( (sal_Int16*)0 );
                break;
            }
            case TypeClass_UNSIGNED_SHORT:
            {
                sal_uInt16 n = pVar->GetUShort();
                if( n <= 255 )
                    aType = ::getCppuType( (sal_uInt8*)0 );
                break;
            }
            case TypeClass_UNSIGNED_LONG:
            {
                sal_uInt32 n = pVar->GetLong();
                if( n <= 255 )
                    aType = ::getCppuType( (sal_uInt8*)0 );
                else if( n <= SbxMAXUINT )
                    aType = ::getCppuType( (sal_uInt16*)0 );
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
    ::rtl::OUString aSeqTypeName;
    sal_Int32 i;
    for( i = 0 ; i < nSeqLevel ; i++ )
        aSeqTypeName += aSeqLevelStr;

    aSeqTypeName += aElemType.getTypeName();
    Type aSeqType( TypeClass_SEQUENCE, aSeqTypeName );

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
        catch (IndexOutOfBoundsException&)
        {
            StarBASIC::Error( SbERR_OUT_OF_RANGE );
        }
    }
    return aRetVal;
}

// Map old interface
Any sbxToUnoValue( SbxVariable* pVar )
{
    return sbxToUnoValueImpl( pVar );
}

// converting of Sbx to Uno with known target class
Any sbxToUnoValue( SbxVariable* pVar, const Type& rType, Property* pUnoProperty )
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
        SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
        if( xObj.Is() && xObj->ISA(SbUnoAnyObject) )
        {
            return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
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

            // zero referenz?
            if( pVar->IsNull() && eType == TypeClass_INTERFACE )
            {
                Reference< XInterface > xRef;
                ::rtl::OUString aClassName = xIdlTargetClass->getName();
                Type aClassType( xIdlTargetClass->getTypeClass(), aClassName.getStr() );
                aRetVal.setValue( &xRef, aClassType );
            }
            else
            {
                // #112368 Special conversion for Decimal, Currency and Date
                if( eType == TypeClass_STRUCT )
                {
                    SbiInstance* pInst = pINST;
                    if( pInst && pInst->IsCompatibility() )
                    {
                        if( rType == ::getCppuType( (oleautomation::Decimal*)0 ) )
                        {
                            oleautomation::Decimal aDecimal;
                            pVar->fillAutomationDecimal( aDecimal );
                            aRetVal <<= aDecimal;
                            break;
                        }
                        else if( rType == ::getCppuType( (oleautomation::Currency*)0 ) )
                        {
                            // assumes per previous code that ole Currency is Int64
                            aRetVal <<= (sal_Int64)( pVar->GetInt64() );
                            break;
                        }
                        else if( rType == ::getCppuType( (oleautomation::Date*)0 ) )
                        {
                            oleautomation::Date aDate;
                            aDate.Value = pVar->GetDate();
                            aRetVal <<= aDate;
                            break;
                        }
                    }
                }

                SbxBaseRef pObj = (SbxBase*)pVar->GetObject();
                if( pObj && pObj->ISA(SbUnoObject) )
                {
                    aRetVal = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
                }
                else
                {
                    // zero object -> zero XInterface
                    Reference<XInterface> xInt;
                    aRetVal <<= xInt;
                }
            }
        }
        break;

        /* we leave out the following types
        case TypeClass_SERVICE:         break;
        case TypeClass_CLASS:           break;
        case TypeClass_TYPEDEF:         break;
        case TypeClass_UNION:           break;
        case TypeClass_ARRAY:           break;
        */

        case TypeClass_ENUM:
        {
            aRetVal = int2enum( pVar->GetLong(), rType );
        }
        break;

        case TypeClass_SEQUENCE:
        {
            SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
            if( xObj && xObj->ISA(SbxDimArray) )
            {
                SbxBase* pObj = (SbxBase*)xObj;
                SbxDimArray* pArray = (SbxDimArray*)pObj;

                short nDims = pArray->GetDims();

                // Normal case: One dimensional array
                sal_Int32 nLower, nUpper;
                if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
                {
                    sal_Int32 nSeqSize = nUpper - nLower + 1;

                    // create the instanz of the required sequence
                    Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );
                    xIdlTargetClass->createObject( aRetVal );
                    Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
                    xArray->realloc( aRetVal, nSeqSize );

                    // Element-Type
                    ::rtl::OUString aClassName = xIdlTargetClass->getName();
                    typelib_TypeDescription * pSeqTD = 0;
                    typelib_typedescription_getByName( &pSeqTD, aClassName.pData );
                    OSL_ASSERT( pSeqTD );
                    Type aElemType( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
                    // Reference< XIdlClass > xElementClass = TypeToIdlClass( aElemType );

                    // convert all array member and register them
                    sal_Int32 nIdx = nLower;
                    for( sal_Int32 i = 0 ; i < nSeqSize ; i++,nIdx++ )
                    {
                        SbxVariableRef xVar = pArray->Get32( &nIdx );

                        // Convert the value of Sbx to Uno
                        Any aAnyValue = sbxToUnoValue( (SbxVariable*)xVar, aElemType );

                        try
                        {
                            // take over to the sequence
                            xArray->set( aRetVal, i, aAnyValue );
                        }
                        catch( const IllegalArgumentException& )
                        {
                            StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                                implGetExceptionMsg( ::cppu::getCaughtException() ) );
                        }
                        catch (IndexOutOfBoundsException&)
                        {
                            StarBASIC::Error( SbERR_OUT_OF_RANGE );
                        }
                    }
                }
                // #i33795 Map also multi dimensional arrays to corresponding sequences
                else if( nDims > 1 )
                {
                    // Element-Type
                    typelib_TypeDescription * pSeqTD = 0;
                    Type aCurType( rType );
                    sal_Int32 nSeqLevel = 0;
                    Type aElemType;
                    do
                    {
                        ::rtl::OUString aTypeName = aCurType.getTypeName();
                        typelib_typedescription_getByName( &pSeqTD, aTypeName.pData );
                        OSL_ASSERT( pSeqTD );
                        if( pSeqTD->eTypeClass == typelib_TypeClass_SEQUENCE )
                        {
                            aCurType = Type( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
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
                        sal_Int32* pLowerBounds = new sal_Int32[nDims];
                        sal_Int32* pUpperBounds = new sal_Int32[nDims];
                        sal_Int32* pActualIndices = new sal_Int32[nDims];
                        for( short i = 1 ; i <= nDims ; i++ )
                        {
                            sal_Int32 lBound, uBound;
                            pArray->GetDim32( i, lBound, uBound );

                            short j = i - 1;
                            pActualIndices[j] = pLowerBounds[j] = lBound;
                            pUpperBounds[j] = uBound;
                        }

                        aRetVal = implRekMultiDimArrayToSequence( pArray, aElemType,
                            nDims - 1, 0, pActualIndices, pLowerBounds, pUpperBounds );

                        delete[] pUpperBounds;
                        delete[] pLowerBounds;
                        delete[] pActualIndices;
                    }
                }
            }
        }
        break;

        /*
        case TypeClass_VOID:            break;
        case TypeClass_UNKNOWN:         break;
        */

        // Use for Any the class indipendent converting routine
        case TypeClass_ANY:
        {
            aRetVal = sbxToUnoValueImpl( pVar );
        }
        break;

        case TypeClass_BOOLEAN:
        {
            sal_Bool b = pVar->GetBool();
            aRetVal.setValue( &b, getBooleanCppuType() );
            break;
        }
        case TypeClass_CHAR:
        {
            sal_Unicode c = pVar->GetChar();
            aRetVal.setValue( &c , getCharCppuType() );
            break;
        }
        case TypeClass_STRING:          aRetVal <<= pVar->GetOUString(); break;
        case TypeClass_FLOAT:           aRetVal <<= pVar->GetSingle(); break;
        case TypeClass_DOUBLE:          aRetVal <<= pVar->GetDouble(); break;
        //case TypeClass_OCTET:         break;

        case TypeClass_BYTE:
        {
            sal_Int16 nVal = pVar->GetInteger();
            sal_Bool bOverflow = sal_False;
            if( nVal < -128 )
            {
                bOverflow = sal_True;
                nVal = -128;
            }
            else if( nVal > 127 )
            {
                bOverflow = sal_True;
                nVal = 127;
            }
            if( bOverflow )
                   StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );

            sal_Int8 nByteVal = (sal_Int8)nVal;
            aRetVal <<= nByteVal;
            break;
        }
        //case TypeClass_INT:           break;
        case TypeClass_SHORT:           aRetVal <<= (sal_Int16)( pVar->GetInteger() );  break;
        case TypeClass_LONG:            aRetVal <<= (sal_Int32)( pVar->GetLong() );     break;
        case TypeClass_HYPER:           aRetVal <<= (sal_Int64)( pVar->GetInt64() );    break;
        //case TypeClass_UNSIGNED_OCTET:break;
        case TypeClass_UNSIGNED_SHORT:  aRetVal <<= (sal_uInt16)( pVar->GetUShort() );  break;
        case TypeClass_UNSIGNED_LONG:   aRetVal <<= (sal_uInt32)( pVar->GetULong() );   break;
        case TypeClass_UNSIGNED_HYPER:  aRetVal <<= (sal_uInt64)( pVar->GetUInt64() );  break;
        //case TypeClass_UNSIGNED_INT:  break;
        //case TypeClass_UNSIGNED_BYTE: break;
        default: break;
    }

    return aRetVal;
}

void processAutomationParams( SbxArray* pParams, Sequence< Any >& args, bool bOLEAutomation, UINT32 nParamCount )
{
    AutomationNamedArgsSbxArray* pArgNamesArray = NULL;
    if( bOLEAutomation )
        pArgNamesArray = PTR_CAST(AutomationNamedArgsSbxArray,pParams);

    args.realloc( nParamCount );
    Any* pAnyArgs = args.getArray();
    bool bBlockConversionToSmallestType = pINST->IsCompatibility();
    UINT32 i = 0;
    if( pArgNamesArray )
    {
        Sequence< ::rtl::OUString >& rNameSeq = pArgNamesArray->getNames();
        ::rtl::OUString* pNames = rNameSeq.getArray();
        Any aValAny;
        for( i = 0 ; i < nParamCount ; i++ )
        {
            USHORT iSbx = (USHORT)(i+1);

            aValAny = sbxToUnoValueImpl( pParams->Get( iSbx ),
            bBlockConversionToSmallestType );

            ::rtl::OUString aParamName = pNames[iSbx];
            if( aParamName.getLength() )
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
            pAnyArgs[i] = sbxToUnoValueImpl( pParams->Get( (USHORT)(i+1) ),
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
Any invokeAutomationMethod( const String& Name, Sequence< Any >& args, SbxArray* pParams, UINT32 nParamCount, Reference< XInvocation >& rxInvocation, INVOKETYPE invokeType = Func )
{
    Sequence< INT16 > OutParamIndex;
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
    const INT16* pIndices = OutParamIndex.getConstArray();
    UINT32 nLen = OutParamIndex.getLength();
    if( nLen )
    {
        const Any* pNewValues = OutParam.getConstArray();
        for( UINT32 j = 0 ; j < nLen ; j++ )
        {
            INT16 iTarget = pIndices[ j ];
            if( iTarget >= (INT16)nParamCount )
                break;
            unoToSbxValue( (SbxVariable*)pParams->Get( (USHORT)(j+1) ), pNewValues[ j ] );
        }
    }
    return aRetAny;
}

// Debugging help method to readout the imlemented interfaces of an object
String Impl_GetInterfaceInfo( const Reference< XInterface >& x, const Reference< XIdlClass >& xClass, USHORT nRekLevel )
{
    Type aIfaceType = ::getCppuType( (const Reference< XInterface > *)0 );
    static Reference< XIdlClass > xIfaceClass = TypeToIdlClass( aIfaceType );

    String aRetStr;
    for( USHORT i = 0 ; i < nRekLevel ; i++ )
        aRetStr.AppendAscii( "    " );
    aRetStr += String( xClass->getName() );
    ::rtl::OUString aClassName = xClass->getName();
    Type aClassType( xClass->getTypeClass(), aClassName.getStr() );

    // checking if the interface is realy supported
    if( !x->queryInterface( aClassType ).hasValue() )
    {
        aRetStr.AppendAscii( " (ERROR: Not really supported!)\n" );
    }
    // Are there super interfaces?
    else
    {
        aRetStr.AppendAscii( "\n" );

        // get the super interfaces
        Sequence< Reference< XIdlClass > > aSuperClassSeq = xClass->getSuperclasses();
        const Reference< XIdlClass >* pClasses = aSuperClassSeq.getConstArray();
        UINT32 nSuperIfaceCount = aSuperClassSeq.getLength();
        for( UINT32 j = 0 ; j < nSuperIfaceCount ; j++ )
        {
            const Reference< XIdlClass >& rxIfaceClass = pClasses[j];
            if( !rxIfaceClass->equals( xIfaceClass ) )
                aRetStr += Impl_GetInterfaceInfo( x, rxIfaceClass, nRekLevel + 1 );
        }
    }
    return aRetStr;
}

String getDbgObjectNameImpl( SbUnoObject* pUnoObj )
{
    String aName;
    if( pUnoObj )
    {
        aName = pUnoObj->GetClassName();
        if( !aName.Len() )
        {
            Any aToInspectObj = pUnoObj->getUnoAny();
            TypeClass eType = aToInspectObj.getValueType().getTypeClass();
            Reference< XInterface > xObj;
            if( eType == TypeClass_INTERFACE )
                xObj = *(Reference< XInterface >*)aToInspectObj.getValue();
            if( xObj.is() )
            {
                Reference< XServiceInfo > xServiceInfo( xObj, UNO_QUERY );
                if( xServiceInfo.is() )
                    aName = xServiceInfo->getImplementationName();
            }
        }
    }
    return aName;
}

String getDbgObjectName( SbUnoObject* pUnoObj )
{
    String aName = getDbgObjectNameImpl( pUnoObj );
    if( !aName.Len() )
        aName.AppendAscii( "Unknown" );

    String aRet;
    if( aName.Len() > 20 )
        aRet.AppendAscii( "\n" );
    aRet.AppendAscii( "\"" );
    aRet += aName;
    aRet.AppendAscii( "\":" );
    return aRet;
}

String getBasicObjectTypeName( SbxObject* pObj )
{
    String aName;
    if( pObj )
    {
        SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
        if( pUnoObj )
            aName = getDbgObjectNameImpl( pUnoObj );
    }
    return aName;
}

bool checkUnoObjectType( SbUnoObject* pUnoObj,
    const String& aClass )
{
    Any aToInspectObj = pUnoObj->getUnoAny();
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE )
        return false;
    const Reference< XInterface > x = *(Reference< XInterface >*)aToInspectObj.getValue();

    // Return true for XInvocation based objects as interface type names don't count then
    Reference< XInvocation > xInvocation( x, UNO_QUERY );
    if( xInvocation.is() )
        return true;

    bool result = false;
    Reference< XTypeProvider > xTypeProvider( x, UNO_QUERY );
    if( xTypeProvider.is() )
    {
        Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
        const Type* pTypeArray = aTypeSeq.getConstArray();
        UINT32 nIfaceCount = aTypeSeq.getLength();
        for( UINT32 j = 0 ; j < nIfaceCount ; j++ )
        {
            const Type& rType = pTypeArray[j];

            Reference<XIdlClass> xClass = TypeToIdlClass( rType );
            if( !xClass.is() )
            {
                OSL_FAIL("failed to get XIdlClass for type");
                break;
            }
            ::rtl::OUString sClassName = xClass->getName();
            if ( sClassName.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.oleautomation.XAutomationObject" ) ) ) )
            {
                // there is a hack in the extensions/source/ole/oleobj.cxx  to return the typename of the automation object, lets check if it
                // matches
                Reference< XInvocation > xInv( aToInspectObj, UNO_QUERY );
                if ( xInv.is() )
                {
                    rtl::OUString sTypeName;
                    xInv->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("$GetTypeName") ) ) >>= sTypeName;
                    if ( sTypeName.getLength() == 0 || sTypeName.equals(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IDispatch") ) ) )
                        // can't check type, leave it pass
                        result = true;
                    else
                        result = sTypeName.equals( aClass );
                }
                break; // finished checking automation object
            }
            OSL_TRACE("Checking if object implements %s",
                OUStringToOString( defaultNameSpace + aClass,
                    RTL_TEXTENCODING_UTF8 ).getStr() );
            // although interfaces in the ooo.vba.vba namespace
            // obey the idl rules and have a leading X, in basic we
            // want to be able to do something like
            // 'dim wrkbooks as WorkBooks'
            // so test assumes the 'X' has been dropped
            sal_Int32 indexLastDot = sClassName.lastIndexOf('.');
            if ( indexLastDot > -1 && sClassName.copy( indexLastDot + 1).equalsIgnoreAsciiCase( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("X") ) + aClass ) )
            {
                result = true;
                break;
            }
        }
    }
    return result;
}

// Debugging help method to readout the imlemented interfaces of an object
String Impl_GetSupportedInterfaces( SbUnoObject* pUnoObj )
{
    Any aToInspectObj = pUnoObj->getUnoAny();

    // allow only TypeClass interface
    TypeClass eType = aToInspectObj.getValueType().getTypeClass();
    String aRet;
    if( eType != TypeClass_INTERFACE )
    {
        aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM(ID_DBG_SUPPORTEDINTERFACES) );
        aRet.AppendAscii( " not available.\n(TypeClass is not TypeClass_INTERFACE)\n" );
    }
    else
    {
        // get the interface from the Any
        const Reference< XInterface > x = *(Reference< XInterface >*)aToInspectObj.getValue();

        // address the XIdlClassProvider-Interface
        Reference< XIdlClassProvider > xClassProvider( x, UNO_QUERY );
        Reference< XTypeProvider > xTypeProvider( x, UNO_QUERY );

        aRet.AssignAscii( "Supported interfaces by object " );
        String aObjName = getDbgObjectName( pUnoObj );
        aRet += aObjName;
        aRet.AppendAscii( "\n" );
        if( xTypeProvider.is() )
        {
            // get the interfaces of the implementation
            Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
            const Type* pTypeArray = aTypeSeq.getConstArray();
            UINT32 nIfaceCount = aTypeSeq.getLength();
            for( UINT32 j = 0 ; j < nIfaceCount ; j++ )
            {
                const Type& rType = pTypeArray[j];

                Reference<XIdlClass> xClass = TypeToIdlClass( rType );
                if( xClass.is() )
                {
                    aRet += Impl_GetInterfaceInfo( x, xClass, 1 );
                }
                else
                {
                    typelib_TypeDescription * pTD = 0;
                    rType.getDescription( &pTD );
                    String TypeName( ::rtl::OUString( pTD->pTypeName ) );

                    aRet.AppendAscii( "*** ERROR: No IdlClass for type \"" );
                    aRet += TypeName;
                    aRet.AppendAscii( "\"\n*** Please check type library\n" );
                }
            }
        }
        else if( xClassProvider.is() )
        {

            OSL_FAIL( "XClassProvider not supported in UNO3" );
        }
    }
    return aRet;
}



// Debugging help method SbxDataType -> String
String Dbg_SbxDataType2String( SbxDataType eType )
{
    String aRet( RTL_CONSTASCII_USTRINGPARAM("Unknown Sbx-Type!") );
    switch( +eType )
    {
        case SbxEMPTY:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxEMPTY") ); break;
        case SbxNULL:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxNULL") ); break;
        case SbxINTEGER:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINTEGER") ); break;
        case SbxLONG:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLONG") ); break;
        case SbxSINGLE:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSINGLE") ); break;
        case SbxDOUBLE:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDOUBLE") ); break;
        case SbxCURRENCY:   aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCURRENCY") ); break;
        case SbxDECIMAL:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDECIMAL") ); break;
        case SbxDATE:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATE") ); break;
        case SbxSTRING:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSTRING") ); break;
        case SbxOBJECT:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxOBJECT") ); break;
        case SbxERROR:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxERROR") ); break;
        case SbxBOOL:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBOOL") ); break;
        case SbxVARIANT:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVARIANT") ); break;
        case SbxDATAOBJECT: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATAOBJECT") ); break;
        case SbxCHAR:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCHAR") ); break;
        case SbxBYTE:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBYTE") ); break;
        case SbxUSHORT:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSHORT") ); break;
        case SbxULONG:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxULONG") ); break;
        case SbxSALINT64:   aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINT64") ); break;
        case SbxSALUINT64:  aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUINT64") ); break;
        case SbxINT:        aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINT") ); break;
        case SbxUINT:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUINT") ); break;
        case SbxVOID:       aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVOID") ); break;
        case SbxHRESULT:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxHRESULT") ); break;
        case SbxPOINTER:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxPOINTER") ); break;
        case SbxDIMARRAY:   aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDIMARRAY") ); break;
        case SbxCARRAY:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCARRAY") ); break;
        case SbxUSERDEF:    aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSERDEF") ); break;
        case SbxLPSTR:      aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPSTR") ); break;
        case SbxLPWSTR:     aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPWSTR") ); break;
        case SbxCoreSTRING: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCoreSTRING" ) ); break;
        case SbxOBJECT | SbxARRAY: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxARRAY") ); break;
        default: break;
    }
    return aRet;
}

// Debugging help method to display the properties of a SbUnoObjects
String Impl_DumpProperties( SbUnoObject* pUnoObj )
{
    String aRet( RTL_CONSTASCII_USTRINGPARAM("Properties of object ") );
    String aObjName = getDbgObjectName( pUnoObj );
    aRet += aObjName;

    // analyse the Uno-Infos to recognise the arrays
    Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = pUnoObj->getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.AppendAscii( "\nUnknown, no introspection available\n" );
        return aRet;
    }

    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    UINT32 nUnoPropCount = props.getLength();
    const Property* pUnoProps = props.getConstArray();

    SbxArray* pProps = pUnoObj->GetProperties();
    USHORT nPropCount = pProps->Count();
    USHORT nPropsPerLine = 1 + nPropCount / 30;
    for( USHORT i = 0; i < nPropCount; i++ )
    {
        SbxVariable* pVar = pProps->Get( i );
        if( pVar )
        {
            String aPropStr;
            if( (i % nPropsPerLine) == 0 )
                aPropStr.AppendAscii( "\n" );

            // output the type and name
            // Is it in Uno a sequence?
            SbxDataType eType = pVar->GetFullType();

            BOOL bMaybeVoid = FALSE;
            if( i < nUnoPropCount )
            {
                const Property& rProp = pUnoProps[ i ];

                // By MAYBEVOID convert the type out of Uno newly,
                // so that not only SbxEMPTY were outputed.
                if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
                {
                    eType = unoToSbxType( rProp.Type.getTypeClass() );
                    bMaybeVoid = TRUE;
                }
                if( eType == SbxOBJECT )
                {
                    Type aType = rProp.Type;
                    if( aType.getTypeClass() == TypeClass_SEQUENCE )
                        eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
                }
            }
            aPropStr += Dbg_SbxDataType2String( eType );
            if( bMaybeVoid )
                aPropStr.AppendAscii( "/void" );
            aPropStr.AppendAscii( " " );
            aPropStr += pVar->GetName();

            if( i == nPropCount - 1 )
                aPropStr.AppendAscii( "\n" );
            else
                aPropStr.AppendAscii( "; " );

            aRet += aPropStr;
        }
    }
    return aRet;
}

// Debugging help method to display the methods of an SbUnoObjects
String Impl_DumpMethods( SbUnoObject* pUnoObj )
{
    String aRet( RTL_CONSTASCII_USTRINGPARAM("Methods of object ") );
    String aObjName = getDbgObjectName( pUnoObj );
    aRet += aObjName;

    // XIntrospectionAccess, so that the types of the parameter could be outputed
    Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
    if( !xAccess.is() )
    {
        Reference< XInvocation > xInvok = pUnoObj->getInvocation();
        if( xInvok.is() )
            xAccess = xInvok->getIntrospection();
    }
    if( !xAccess.is() )
    {
        aRet.AppendAscii( "\nUnknown, no introspection available\n" );
        return aRet;
    }
    Sequence< Reference< XIdlMethod > > methods = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    const Reference< XIdlMethod >* pUnoMethods = methods.getConstArray();

    SbxArray* pMethods = pUnoObj->GetMethods();
    USHORT nMethodCount = pMethods->Count();
    if( !nMethodCount )
    {
        aRet.AppendAscii( "\nNo methods found\n" );
        return aRet;
    }
    USHORT nPropsPerLine = 1 + nMethodCount / 30;
    for( USHORT i = 0; i < nMethodCount; i++ )
    {
        SbxVariable* pVar = pMethods->Get( i );
        if( pVar )
        {
            String aPropStr;
            if( (i % nPropsPerLine) == 0 )
                aPropStr.AppendAscii( "\n" );

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
            aPropStr += Dbg_SbxDataType2String( eType );
            aPropStr.AppendAscii( " " );
            aPropStr += pVar->GetName();
            aPropStr.AppendAscii( " ( " );

            // the get-method mustn't have a parameter
            Sequence< Reference< XIdlClass > > aParamsSeq = rxMethod->getParameterTypes();
            UINT32 nParamCount = aParamsSeq.getLength();
            const Reference< XIdlClass >* pParams = aParamsSeq.getConstArray();

            if( nParamCount > 0 )
            {
                for( USHORT j = 0; j < nParamCount; j++ )
                {
                    String aTypeStr = Dbg_SbxDataType2String( unoToSbxType( pParams[ j ] ) );
                    aPropStr += aTypeStr;

                    if( j < nParamCount - 1 )
                        aPropStr.AppendAscii( ", " );
                }
            }
            else
                aPropStr.AppendAscii( "void" );

            aPropStr.AppendAscii( " ) " );

            if( i == nMethodCount - 1 )
                aPropStr.AppendAscii( "\n" );
            else
                aPropStr.AppendAscii( "; " );

            aRet += aPropStr;
        }
    }
    return aRet;
}

TYPEINIT1(AutomationNamedArgsSbxArray,SbxArray)

// Implementation SbUnoObject
void SbUnoObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    if( bNeedIntrospection )
        doIntrospection();

    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoProperty* pProp = PTR_CAST(SbUnoProperty,pVar);
        SbUnoMethod* pMeth = PTR_CAST(SbUnoMethod,pVar);
        if( pProp )
        {
            bool bInvocation = pProp->isInvocationBased();
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                // Test-Properties
                INT32 nId = pProp->nId;
                if( nId < 0 )
                {
                    // Id == -1: Display implemented interfaces according the ClassProvider
                    if( nId == -1 )     // Property ID_DBG_SUPPORTEDINTERFACES"
                    {
                        String aRetStr = Impl_GetSupportedInterfaces( this );
                        pVar->PutString( aRetStr );
                    }
                    // Id == -2: output properties
                    else if( nId == -2 )        // Property ID_DBG_PROPERTIES
                    {
                        // by now all properties must be established
                        implCreateAll();
                        String aRetStr = Impl_DumpProperties( this );
                        pVar->PutString( aRetStr );
                    }
                    // Id == -3: output the methods
                    else if( nId == -3 )        // Property ID_DBG_METHODS
                    {
                        // y now all properties must be established
                        implCreateAll();
                        String aRetStr = Impl_DumpMethods( this );
                        pVar->PutString( aRetStr );
                    }
                    return;
                }

                if( !bInvocation && mxUnoAccess.is() )
                {
                    try
                    {
                        // get the value
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                        Any aRetAny = xPropSet->getPropertyValue( pProp->GetName() );
                        // The use of getPropertyValue (instead of using the index) is
                        // suboptimal, but the refactoring to XInvocation is already pending
                        // Otherwise it is posible to use FastPropertySet

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
                        UINT32 nParamCount = pParams ? ((UINT32)pParams->Count() - 1) : 0;
                        sal_Bool bCanBeConsideredAMethod = mxInvocation->hasMethod( pProp->GetName() );
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
                            pVar->SetParameters( NULL );

                    }
                    catch( const Exception& )
                    {
                        implHandleAnyException( ::cppu::getCaughtException() );
                    }
                }
            }
            else if( pHint->GetId() == SBX_HINT_DATACHANGED )
            {
                if( !bInvocation && mxUnoAccess.is() )
                {
                    if( pProp->aUnoProp.Attributes & PropertyAttribute::READONLY )
                    {
                        StarBASIC::Error( SbERR_PROP_READONLY );
                        return;
                    }

                    // take over the value from Uno to Sbx
                    Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
                    try
                    {
                        // set the value
                        Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                        xPropSet->setPropertyValue( pProp->GetName(), aAnyValue );
                        // The use of getPropertyValue (instead of using the index) is
                        // suboptimal, but the refactoring to XInvocation is already pending
                        // Otherwise it is posible to use FastPropertySet
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
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                // number of Parameter -1 because of Param0 == this
                UINT32 nParamCount = pParams ? ((UINT32)pParams->Count() - 1) : 0;
                Sequence<Any> args;
                BOOL bOutParams = FALSE;
                UINT32 i;

                if( !bInvocation && mxUnoAccess.is() )
                {
                    // get info
                    const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
                    const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
                    UINT32 nUnoParamCount = rInfoSeq.getLength();
                    UINT32 nAllocParamCount = nParamCount;

                    // ignore surplus parameter; alternative: throw an error
                    if( nParamCount > nUnoParamCount )
                    {
                        nParamCount = nUnoParamCount;
                        nAllocParamCount = nParamCount;
                    }
                    else if( nParamCount < nUnoParamCount )
                    {
                        SbiInstance* pInst = pINST;
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
                                    StarBASIC::Error( SbERR_NOT_OPTIONAL );
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
                            //const XIdlClassRef& rxClass = pUnoParams[i];

                            com::sun::star::uno::Type aType( rxClass->getTypeClass(), rxClass->getName() );

                            // ATTENTION: Don't forget for Sbx-Parameter the offset!
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( (USHORT)(i+1) ), aType );

                            // If it is not certain check whether the out-parameter are available.
                            if( !bOutParams )
                            {
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    bOutParams = TRUE;
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
                GetSbData()->bBlockCompilerError = TRUE;  // #106433 Block compiler errors for API calls
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

                            UINT32 j;
                            for( j = 0 ; j < nParamCount ; j++ )
                            {
                                const ParamInfo& rInfo = pParamInfos[j];
                                ParamMode aParamMode = rInfo.aMode;
                                if( aParamMode != ParamMode_IN )
                                    unoToSbxValue( (SbxVariable*)pParams->Get( (USHORT)(j+1) ), pAnyArgs[ j ] );
                            }
                        }
                    }
                    else if( bInvocation && mxInvocation.is() )
                    {
                        Any aRetAny = invokeAutomationMethod( pMeth->GetName(), args, pParams, nParamCount, mxInvocation );
                        unoToSbxValue( pVar, aRetAny );
                    }

                    // remove parameter here, because this was not done anymore in unoToSbxValue()
                    // for arrays
                    if( pParams )
                        pVar->SetParameters( NULL );
                }
                catch( const Exception& )
                {
                    implHandleAnyException( ::cppu::getCaughtException() );
                }
                GetSbData()->bBlockCompilerError = FALSE;  // #106433 Unblock compiler errors
            }
        }
        else
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}


#ifdef INVOCATION_ONLY
// From USR
Reference< XInvocation > createDynamicInvocationFor( const Any& aAny );
#endif

SbUnoObject::SbUnoObject( const String& aName_, const Any& aUnoObj_ )
    : SbxObject( aName_ )
    , bNeedIntrospection( TRUE )
    , bIgnoreNativeCOMObjectMembers( FALSE )
{
    static Reference< XIntrospection > xIntrospection;

    // beat out again the default properties of Sbx
    Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Name") ), SbxCLASS_DONTCARE );
    Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Parent") ), SbxCLASS_DONTCARE );

    // check the type of the ojekts
    TypeClass eType = aUnoObj_.getValueType().getTypeClass();
    Reference< XInterface > x;
    if( eType == TypeClass_INTERFACE )
    {
        // get the interface from the Any
        x = *(Reference< XInterface >*)aUnoObj_.getValue();
        if( !x.is() )
            return;
    }

    Reference< XTypeProvider > xTypeProvider;
#ifdef INVOCATION_ONLY
    // get the invocation
    mxInvocation = createDynamicInvocationFor( aUnoObj_ );
#else
    // Did the object have an invocation itself?
    mxInvocation = Reference< XInvocation >( x, UNO_QUERY );

    xTypeProvider = Reference< XTypeProvider >( x, UNO_QUERY );
#endif

    if( mxInvocation.is() )
    {
        // #94670: This is WRONG because then the MaterialHolder doesn't refer
        // to the object implementing XInvocation but to the object passed to
        // the invocation service!!!
        // mxMaterialHolder = Reference< XMaterialHolder >::query( mxInvocation );

        // get the ExactName
        mxExactNameInvocation = Reference< XExactName >::query( mxInvocation );

        // The remainder refers only to the introspection
        if( !xTypeProvider.is() )
        {
            bNeedIntrospection = FALSE;
            return;
        }

        // Ignore introspection based members for COM objects to avoid
        // hiding of equally named COM symbols, e.g. XInvocation::getValue
        Reference< oleautomation::XAutomationObject > xAutomationObject( aUnoObj_, UNO_QUERY );
        if( xAutomationObject.is() )
            bIgnoreNativeCOMObjectMembers = TRUE;
    }

    maTmpUnoObj = aUnoObj_;


    //*** Define the name ***
    BOOL bFatalError = TRUE;

    // Is it an interface or a struct?
    BOOL bSetClassName = FALSE;
    String aClassName_;
    if( eType == TypeClass_STRUCT || eType == TypeClass_EXCEPTION )
    {
        // Struct is Ok
        bFatalError = FALSE;

        // insert the real name of the class
        if( aName_.Len() == 0 )
        {
            aClassName_ = String( aUnoObj_.getValueType().getTypeName() );
            bSetClassName = TRUE;
        }
    }
    else if( eType == TypeClass_INTERFACE )
    {
        // Interface works always through the type in the Any
        bFatalError = FALSE;

        // Ask for the XIdlClassProvider-Interface
        Reference< XIdlClassProvider > xClassProvider( x, UNO_QUERY );
        if( xClassProvider.is() )
        {
            // Insert the real name of the class
            if( aName_.Len() == 0 )
            {
                Sequence< Reference< XIdlClass > > szClasses = xClassProvider->getIdlClasses();
                UINT32 nLen = szClasses.getLength();
                if( nLen )
                {
                    const Reference< XIdlClass > xImplClass = szClasses.getConstArray()[ 0 ];
                    if( xImplClass.is() )
                    {
                        aClassName_ = String( xImplClass->getName() );
                        bSetClassName = TRUE;
                    }
                }
            }
        }
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
void SbUnoObject::doIntrospection( void )
{
    static Reference< XIntrospection > xIntrospection;

    if( !bNeedIntrospection )
        return;
    bNeedIntrospection = FALSE;

    if( !xIntrospection.is() )
    {
        // get the introspection service
        Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
        if ( xFactory.is() )
        {
            Reference< XInterface > xI = xFactory->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.Introspection")) );
            if (xI.is())
                xIntrospection = Reference< XIntrospection >::query( xI );
                //xI->queryInterface( ::getCppuType( (const Reference< XIntrospection > *)0 ), xIntrospection );
        }
    }
    if( !xIntrospection.is() )
    {
        StarBASIC::FatalError( ERRCODE_BASIC_EXCEPTION );
        return;
    }

    // pass the introspection
    try
    {
        mxUnoAccess = xIntrospection->inspect( maTmpUnoObj );
    }
    catch( RuntimeException& e )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
    }

    if( !mxUnoAccess.is() )
    {
        // #51475 mark an invalid objekt kennzeichnen (no mxMaterialHolder)
        return;
    }

    // get MaterialHolder from access
    mxMaterialHolder = Reference< XMaterialHolder >::query( mxUnoAccess );

    // get ExactName from access
    mxExactName = Reference< XExactName >::query( mxUnoAccess );
}




// Start of a list of all SbUnoMethod-Instances
static SbUnoMethod* pFirst = NULL;

void clearUnoMethods( void )
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
    const String& aName_,
    SbxDataType eSbxType,
    Reference< XIdlMethod > xUnoMethod_,
    bool bInvocation
)
    : SbxMethod( aName_, eSbxType )
    , mbInvocation( bInvocation )
{
    m_xUnoMethod = xUnoMethod_;
    pParamInfoSeq = NULL;

    // enregister the method in a list
    pNext = pFirst;
    pPrev = NULL;
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
    if( !pInfo && m_xUnoMethod.is() )
    {
        SbiInstance* pInst = pINST;
        if( pInst && pInst->IsCompatibility() )
        {
            pInfo = new SbxInfo();

            const Sequence<ParamInfo>& rInfoSeq = getParamInfos();
            const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
            UINT32 nParamCount = rInfoSeq.getLength();

            for( UINT32 i = 0 ; i < nParamCount ; i++ )
            {
                const ParamInfo& rInfo = pParamInfos[i];
                ::rtl::OUString aParamName = rInfo.aName;

                // const Reference< XIdlClass >& rxClass = rInfo.aType;
                SbxDataType t = SbxVARIANT;
                USHORT nFlags_ = SBX_READ;
                pInfo->AddParam( aParamName, t, nFlags_ );
            }
        }
    }
    return pInfo;
}

const Sequence<ParamInfo>& SbUnoMethod::getParamInfos( void )
{
    if( !pParamInfoSeq && m_xUnoMethod.is() )
    {
        Sequence<ParamInfo> aTmp = m_xUnoMethod->getParameterInfos() ;
        pParamInfoSeq = new Sequence<ParamInfo>( aTmp );
    }
    return *pParamInfoSeq;
}

SbUnoProperty::SbUnoProperty
(
    const String& aName_,
    SbxDataType eSbxType,
    const Property& aUnoProp_,
    INT32 nId_,
    bool bInvocation
)
    : SbxProperty( aName_, eSbxType )
    , aUnoProp( aUnoProp_ )
    , nId( nId_ )
    , mbInvocation( bInvocation )
{
    // as needed establish an dummy array so that SbiRuntime::CheckArray() works
    static SbxArrayRef xDummyArray = new SbxArray( SbxVARIANT );
    if( eSbxType & SbxARRAY )
        PutObject( xDummyArray );
}

SbUnoProperty::~SbUnoProperty()
{}


SbxVariable* SbUnoObject::Find( const String& rName, SbxClassType t )
{
    static Reference< XIdlMethod > xDummyMethod;
    static Property aDummyProp;

    SbxVariable* pRes = SbxObject::Find( rName, t );

    if( bNeedIntrospection )
        doIntrospection();

    // New 1999-03-04: Create properties on demand. Therefore search now perIntrospectionAccess,
    // if a property or a method of the required name exist
    if( !pRes )
    {
        ::rtl::OUString aUName( rName );
        if( mxUnoAccess.is() && !bIgnoreNativeCOMObjectMembers )
        {
            if( mxExactName.is() )
            {
                ::rtl::OUString aUExactName = mxExactName->getExactName( aUName );
                if( aUExactName.getLength() )
                    aUName = aUExactName;
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

                // create the property and superimpose it
                SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, 0, false );
                QuickInsert( (SbxVariable*)xVarRef );
                pRes = xVarRef;
            }
            else if( mxUnoAccess->hasMethod( aUName,
                MethodConcept::ALL - MethodConcept::DANGEROUS ) )
            {
                // address the method
                const Reference< XIdlMethod >& rxMethod = mxUnoAccess->
                    getMethod( aUName, MethodConcept::ALL - MethodConcept::DANGEROUS );

                // create SbUnoMethod and superimpose it
                SbxVariableRef xMethRef = new SbUnoMethod( rxMethod->getName(),
                    unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
                QuickInsert( (SbxVariable*)xMethRef );
                pRes = xMethRef;
            }

            // Elsewise nothing would be found it had to be checked, if NameAccess is existent
            if( !pRes )
            {
                try
                {
                    Reference< XNameAccess > xNameAccess( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
                    ::rtl::OUString aUName2( rName );

                    if( xNameAccess.is() && xNameAccess->hasByName( aUName2 ) )
                    {
                        Any aAny = xNameAccess->getByName( aUName2 );

                        // ATTENTION: Die hier erzeugte Variable darf wegen bei XNameAccess
                        // nicht als feste Property in das Object aufgenommen werden und
                        // wird daher nirgendwo gehalten.
                        // If this leads to problems, it has to be created synthetically or
                        // a class SbUnoNameAccessProperty, whose existence had to be checked
                        // constantly and which were if necessary thrown away
                        // if the name was not found anymore.
                        pRes = new SbxVariable( SbxVARIANT );
                        unoToSbxValue( pRes, aAny );
                    }
                }
                catch( NoSuchElementException& e )
                {
                    StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
                }
                catch( const Exception& )
                {
                    // Establish so that the exeption error will not be overwriten
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
                ::rtl::OUString aUExactName = mxExactNameInvocation->getExactName( aUName );
                if( aUExactName.getLength() )
                    aUName = aUExactName;
            }

            try
            {
                if( mxInvocation->hasProperty( aUName ) )
                {
                    // create a property and superimpose it
                    SbxVariableRef xVarRef = new SbUnoProperty( aUName, SbxVARIANT, aDummyProp, 0, true );
                    QuickInsert( (SbxVariable*)xVarRef );
                    pRes = xVarRef;
                }
                else if( mxInvocation->hasMethod( aUName ) )
                {
                    // create SbUnoMethode and superimpose it
                    SbxVariableRef xMethRef = new SbUnoMethod( aUName, SbxVARIANT, xDummyMethod, true );
                    QuickInsert( (SbxVariable*)xMethRef );
                    pRes = xMethRef;
                }
            }
            catch( RuntimeException& e )
            {
                // Establish so that the exeption error will not be overwriten
                if( !pRes )
                    pRes = new SbxVariable( SbxVARIANT );

                StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
            }
        }
    }

    // At the very end checking if the Dbg_-Properties are meant

    if( !pRes )
    {
        if( rName.EqualsIgnoreCaseAscii( ID_DBG_SUPPORTEDINTERFACES ) ||
            rName.EqualsIgnoreCaseAscii( ID_DBG_PROPERTIES ) ||
            rName.EqualsIgnoreCaseAscii( ID_DBG_METHODS ) )
        {
            // Create
            implCreateDbgProperties();

            // Now they have to be found regular
            pRes = SbxObject::Find( rName, SbxCLASS_DONTCARE );
        }
    }
    return pRes;
}


// help method to create the dbg_-Properties
void SbUnoObject::implCreateDbgProperties( void )
{
    Property aProp;

    // Id == -1: display the implemented interfaces corresponding the ClassProvider
    SbxVariableRef xVarRef = new SbUnoProperty( String(RTL_CONSTASCII_USTRINGPARAM(ID_DBG_SUPPORTEDINTERFACES)), SbxSTRING, aProp, -1, false );
    QuickInsert( (SbxVariable*)xVarRef );

    // Id == -2: output the properties
    xVarRef = new SbUnoProperty( String(RTL_CONSTASCII_USTRINGPARAM(ID_DBG_PROPERTIES)), SbxSTRING, aProp, -2, false );
    QuickInsert( (SbxVariable*)xVarRef );

    // Id == -3: output the Methods
    xVarRef = new SbUnoProperty( String(RTL_CONSTASCII_USTRINGPARAM(ID_DBG_METHODS)), SbxSTRING, aProp, -3, false );
    QuickInsert( (SbxVariable*)xVarRef );
}

void SbUnoObject::implCreateAll( void )
{
    // throw away all existing methods and properties
    pMethods   = new SbxArray;
    pProps     = new SbxArray;

    if( bNeedIntrospection ) doIntrospection();

    // get instrospection
    Reference< XIntrospectionAccess > xAccess = mxUnoAccess;
    if( !xAccess.is() || bIgnoreNativeCOMObjectMembers )
    {
        if( mxInvocation.is() )
            xAccess = mxInvocation->getIntrospection();
        else if( bIgnoreNativeCOMObjectMembers )
            return;
    }
    if( !xAccess.is() )
        return;

    // Establish properties
    Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
    UINT32 nPropCount = props.getLength();
    const Property* pProps_ = props.getConstArray();

    UINT32 i;
    for( i = 0 ; i < nPropCount ; i++ )
    {
        const Property& rProp = pProps_[ i ];

        // If the property could be void the type had to be set to Variant
        SbxDataType eSbxType;
        if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
            eSbxType = SbxVARIANT;
        else
            eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

        // Create property and superimpose it
        SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, i, false );
        QuickInsert( (SbxVariable*)xVarRef );
    }

    // Create Dbg_-Properties
    implCreateDbgProperties();

    // Create methods
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods
        ( MethodConcept::ALL - MethodConcept::DANGEROUS );
    UINT32 nMethCount = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods_ = aMethodSeq.getConstArray();
    for( i = 0 ; i < nMethCount ; i++ )
    {
        // address method
        const Reference< XIdlMethod >& rxMethod = pMethods_[i];

        // Create SbUnoMethod and superimpose it
        SbxVariableRef xMethRef = new SbUnoMethod
            ( rxMethod->getName(), unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
        QuickInsert( (SbxVariable*)xMethRef );
    }
}


// output the value
Any SbUnoObject::getUnoAny( void )
{
    Any aRetAny;
    if( bNeedIntrospection ) doIntrospection();
    if( mxMaterialHolder.is() )
        aRetAny = mxMaterialHolder->getMaterial();
    else if( mxInvocation.is() )
        aRetAny <<= mxInvocation;
    return aRetAny;
}

// help method to create an Uno-Struct per CoreReflection
SbUnoObject* Impl_CreateUnoStruct( const String& aClassName )
{
    // get CoreReflection
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return NULL;

    // search for the class
    Reference< XIdlClass > xClass;
    Reference< XHierarchicalNameAccess > xHarryName =
        getCoreReflection_HierarchicalNameAccess_Impl();
    if( xHarryName.is() && xHarryName->hasByHierarchicalName( aClassName ) )
        xClass = xCoreReflection->forName( aClassName );
    if( !xClass.is() )
        return NULL;

    // Is it realy a struct?
    TypeClass eType = xClass->getTypeClass();
    if ( ( eType != TypeClass_STRUCT ) && ( eType != TypeClass_EXCEPTION ) )
        return NULL;

    // create an instance
    Any aNewAny;
    xClass->createObject( aNewAny );

    // make a SbUnoObject out of it
    SbUnoObject* pUnoObj = new SbUnoObject( aClassName, aNewAny );
    return pUnoObj;
}


// Factory-Class to create Uno-Structs per DIM AS NEW
SbxBase* SbUnoFactory::Create( UINT16, UINT32 )
{
    // Via SbxId nothing works in Uno
    return NULL;
}

SbxObject* SbUnoFactory::CreateObject( const String& rClassName )
{
    return Impl_CreateUnoStruct( rClassName );
}


// Provisional interface for the UNO-Connection
// Deliver a SbxObject, that wrap an Uno-Interface
SbxObjectRef GetSbUnoObject( const String& aName, const Any& aUnoObj_ )
{
    return new SbUnoObject( aName, aUnoObj_ );
}

// Force creation of all properties for debugging
void createAllObjectProperties( SbxObject* pObj )
{
    if( !pObj )
        return;

    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
    if( pUnoObj )
        pUnoObj->createAllProperties();
    else
        pObj->GetAll( SbxCLASS_DONTCARE );
}


void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 1 parameter minimum
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    String aClassName = rPar.Get(1)->GetString();

    // try to create Struct with the same name
    SbUnoObjectRef xUnoObj = Impl_CreateUnoStruct( aClassName );
    if( !xUnoObj )
        return;

    // return the objekt
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( (SbUnoObject*)xUnoObj );
}

void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 1 Parameter minimum
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    String aServiceName = rPar.Get(1)->GetString();

    // search for the service and instatiate it
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Reference< XInterface > xInterface;
    if ( xFactory.is() )
    {
        try
        {
            xInterface = xFactory->createInstance( aServiceName );
        }
        catch( const Exception& )
        {
            implHandleAnyException( ::cppu::getCaughtException() );
        }
    }

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        Any aAny;
        aAny <<= xInterface;

        // Create a SbUnoObject out of it and return it
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            // return the object
            refVar->PutObject( (SbUnoObject*)xUnoObj );
        }
        else
        {
            refVar->PutObject( NULL );
        }
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

void RTL_Impl_CreateUnoServiceWithArguments( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 2 parameter minimum
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    String aServiceName = rPar.Get(1)->GetString();
    Any aArgAsAny = sbxToUnoValue( rPar.Get(2),
                getCppuType( (Sequence<Any>*)0 ) );
    Sequence< Any > aArgs;
    aArgAsAny >>= aArgs;

    // search for the service and instatiate it
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    Reference< XInterface > xInterface;
    if ( xFactory.is() )
    {
        try
        {
            xInterface = xFactory->createInstanceWithArguments( aServiceName, aArgs );
        }
        catch( const Exception& )
        {
            implHandleAnyException( ::cppu::getCaughtException() );
        }
    }

    SbxVariableRef refVar = rPar.Get(0);
    if( xInterface.is() )
    {
        Any aAny;
        aAny <<= xInterface;

        // Create a SbUnoObject out of it and return it
        SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
        if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
        {
            // return the object
            refVar->PutObject( (SbUnoObject*)xUnoObj );
        }
        else
        {
            refVar->PutObject( NULL );
        }
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    SbxVariableRef refVar = rPar.Get(0);

    // get the global service manager
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    if( xFactory.is() )
    {
        Any aAny;
        aAny <<= xFactory;

        // Create a SbUnoObject out of it and return it
        SbUnoObjectRef xUnoObj = new SbUnoObject( String( RTL_CONSTASCII_USTRINGPARAM("ProcessServiceManager") ), aAny );
        refVar->PutObject( (SbUnoObject*)xUnoObj );
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 2 parameter minimum
    USHORT nParCount = rPar.Count();
    if( nParCount < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // variable for the return value
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( FALSE );

    // get the Uno-Object
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
        return;
    Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
    TypeClass eType = aAny.getValueType().getTypeClass();
    if( eType != TypeClass_INTERFACE )
        return;

    // get the interface out of the Any
    Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();

    // get CoreReflection
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return;

    for( USHORT i = 2 ; i < nParCount ; i++ )
    {
        // get the name of the interface of the struct
        String aIfaceName = rPar.Get( i )->GetString();

        // search for the class
        Reference< XIdlClass > xClass = xCoreReflection->forName( aIfaceName );
        if( !xClass.is() )
            return;

        // check if the interface will be supported
        ::rtl::OUString aClassName = xClass->getName();
        Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
        if( !x->queryInterface( aClassType ).hasValue() )
            return;
    }

    // Every thing works; then return TRUE
    refVar->PutBool( TRUE );
}

void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // We need 1 parameter minimum
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // variable for the return value
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( FALSE );

    // get the Uno-Object
    SbxVariableRef xParam = rPar.Get( 1 );
    if( !xParam->IsObject() )
        return;
    SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
    if( !(pObj && pObj->ISA(SbUnoObject)) )
        return;
    Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
    TypeClass eType = aAny.getValueType().getTypeClass();
    if( eType == TypeClass_STRUCT )
        refVar->PutBool( TRUE );
}


void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // variable for the return value
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutBool( FALSE );

    // get the Uno-Objects
    SbxVariableRef xParam1 = rPar.Get( 1 );
    if( !xParam1->IsObject() )
        return;
    SbxBaseRef pObj1 = (SbxBase*)xParam1->GetObject();
    if( !(pObj1 && pObj1->ISA(SbUnoObject)) )
        return;
    Any aAny1 = ((SbUnoObject*)(SbxBase*)pObj1)->getUnoAny();
    TypeClass eType1 = aAny1.getValueType().getTypeClass();
    if( eType1 != TypeClass_INTERFACE )
        return;
    Reference< XInterface > x1;
    aAny1 >>= x1;
    //XInterfaceRef x1 = *(XInterfaceRef*)aAny1.get();

    SbxVariableRef xParam2 = rPar.Get( 2 );
    if( !xParam2->IsObject() )
        return;
    SbxBaseRef pObj2 = (SbxBase*)xParam2->GetObject();
    if( !(pObj2 && pObj2->ISA(SbUnoObject)) )
        return;
    Any aAny2 = ((SbUnoObject*)(SbxBase*)pObj2)->getUnoAny();
    TypeClass eType2 = aAny2.getValueType().getTypeClass();
    if( eType2 != TypeClass_INTERFACE )
        return;
    Reference< XInterface > x2;
    aAny2 >>= x2;
    //XInterfaceRef x2 = *(XInterfaceRef*)aAny2.get();

    if( x1 == x2 )
        refVar->PutBool( TRUE );
}

typedef boost::unordered_map< ::rtl::OUString, std::vector< ::rtl::OUString >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > ModuleHash;


// helper wrapper function to interact with TypeProvider and
// XTypeDescriptionEnumerationAccess.
// if it fails for whatever reason
// returned Reference<> be null e.g. .is() will be false

Reference< XTypeDescriptionEnumeration >
getTypeDescriptorEnumeration( const ::rtl::OUString& sSearchRoot,
    const Sequence< TypeClass >& types, TypeDescriptionSearchDepth depth )
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
        catch( NoSuchTypeNameException& /*nstne*/ ) {}
        catch( InvalidTypeNameException& /*nstne*/ ) {}
    }
    return xEnum;
}

typedef boost::unordered_map< ::rtl::OUString, Any, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > VBAConstantsHash;

VBAConstantHelper&
VBAConstantHelper::instance()
{
    static VBAConstantHelper aHelper;
    return aHelper;
}

void
VBAConstantHelper::init()
{
    if ( !isInited )
    {
        Sequence< TypeClass > types(1);
        types[ 0 ] = TypeClass_CONSTANTS;
        Reference< XTypeDescriptionEnumeration > xEnum = getTypeDescriptorEnumeration( defaultNameSpace, types, TypeDescriptionSearchDepth_INFINITE  );

        if ( !xEnum.is() )
            return; //NULL;

        while ( xEnum->hasMoreElements() )
        {
            Reference< XConstantsTypeDescription > xConstants( xEnum->nextElement(), UNO_QUERY );
            if ( xConstants.is() )
            {
                // store constant group name
                ::rtl::OUString sFullName = xConstants->getName();
                sal_Int32 indexLastDot = sFullName.lastIndexOf('.');
                ::rtl::OUString sLeafName( sFullName );
                if ( indexLastDot > -1 )
                    sLeafName = sFullName.copy( indexLastDot + 1);
                aConstCache.push_back( sLeafName ); // assume constant group names are unique
                Sequence< Reference< XConstantTypeDescription > > aConsts = xConstants->getConstants();
                Reference< XConstantTypeDescription >* pSrc = aConsts.getArray();
                sal_Int32 nLen = aConsts.getLength();
                for ( sal_Int32 index =0;  index<nLen; ++pSrc, ++index )
                {
                    // store constant member name
                    Reference< XConstantTypeDescription >& rXConst =
                        *pSrc;
                    sFullName = rXConst->getName();
                    indexLastDot = sFullName.lastIndexOf('.');
                    sLeafName = sFullName;
                    if ( indexLastDot > -1 )
                        sLeafName = sFullName.copy( indexLastDot + 1);
                    aConstHash[ sLeafName.toAsciiLowerCase() ] = rXConst->getConstantValue();
                }
            }
        }
        isInited = true;
    }
}

bool
VBAConstantHelper::isVBAConstantType( const String& rName )
{
    init();
    bool bConstant = false;
    ::rtl::OUString sKey( rName );
    VBAConstantsVector::const_iterator it = aConstCache.begin();

    for( ; it != aConstCache.end(); ++it )
    {
        if( sKey.equalsIgnoreAsciiCase( *it ) )
        {
            bConstant = true;
            break;
        }
    }
    return bConstant;
}

SbxVariable*
VBAConstantHelper::getVBAConstant( const String& rName )
{
    SbxVariable* pConst = NULL;
    init();

    ::rtl::OUString sKey( rName );

    VBAConstantsHash::const_iterator it = aConstHash.find( sKey.toAsciiLowerCase() );

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
SbUnoClass* findUnoClass( const String& rName )
{
    // #105550 Check if module exists
    SbUnoClass* pUnoClass = NULL;

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
                pUnoClass = new SbUnoClass( rName );
        }
    }
    return pUnoClass;
}

SbxVariable* SbUnoClass::Find( const XubString& rName, SbxClassType t )
{
    (void)t;

    SbxVariable* pRes = SbxObject::Find( rName, SbxCLASS_VARIABLE );

    // If nothing were located the submodule isn't known yet
    if( !pRes )
    {
        // If it is already a class, ask for the field
        if( m_xClass.is() )
        {
            // Is it a field(?)
            ::rtl::OUString aUStr( rName );
            Reference< XIdlField > xField = m_xClass->getField( aUStr );
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
            String aNewName = GetName();
            aNewName.AppendAscii( "." );
            aNewName += rName;

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
                            Reference< XInterface > xIface = *(Reference< XInterface >*)aValue.getValue();
                            Reference< XIdlClass > xClass( xIface, UNO_QUERY );
                            if( xClass.is() )
                            {
                                pRes = new SbxVariable( SbxVARIANT );
                                SbxObjectRef xWrapper = (SbxObject*)new SbUnoClass( aNewName, xClass );
                                pRes->PutObject( xWrapper );
                            }
                        }
                        else
                        {
                            pRes = new SbxVariable( SbxVARIANT );
                            unoToSbxValue( pRes, aValue );
                        }
                    }
                    catch( NoSuchElementException& e1 )
                    {
                        String aMsg = implGetExceptionMsg( e1 );
                    }
                }

                // Otherwise take it again as class
                if( !pRes )
                {
                    SbUnoClass* pNewClass = findUnoClass( aNewName );
                    if( pNewClass )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = (SbxObject*)pNewClass;
                        pRes->PutObject( xWrapper );
                    }
                }

                // An UNO service?
                if( !pRes )
                {
                    SbUnoService* pUnoService = findUnoService( aNewName );
                    if( pUnoService )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = (SbxObject*)pUnoService;
                        pRes->PutObject( xWrapper );
                    }
                }

                // An UNO singleton?
                if( !pRes )
                {
                    SbUnoSingleton* pUnoSingleton = findUnoSingleton( aNewName );
                    if( pUnoSingleton )
                    {
                        pRes = new SbxVariable( SbxVARIANT );
                        SbxObjectRef xWrapper = (SbxObject*)pUnoSingleton;
                        pRes->PutObject( xWrapper );
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
                EndListening( pRes->GetBroadcaster(), TRUE );
        }
    }
    return pRes;
}


SbUnoService* findUnoService( const String& rName )
{
    SbUnoService* pSbUnoService = NULL;

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

SbxVariable* SbUnoService::Find( const String& rName, SbxClassType )
{
    SbxVariable* pRes = SbxObject::Find( rName, SbxCLASS_METHOD );

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

                String aName( xCtor->getName() );
                if( !aName.Len() )
                {
                    if( xCtor->isDefaultConstructor() )
                        aName = String::CreateFromAscii( "create" );
                }

                if( aName.Len() )
                {
                    // Create and insert SbUnoServiceCtor
                    SbxVariableRef xSbCtorRef = new SbUnoServiceCtor( aName, xCtor );
                    QuickInsert( (SbxVariable*)xSbCtorRef );
                }
            }

            pRes = SbxObject::Find( rName, SbxCLASS_METHOD );
        }
    }

    return pRes;
}

void SbUnoService::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        SbUnoServiceCtor* pUnoCtor = PTR_CAST(SbUnoServiceCtor,pVar);
        if( pUnoCtor && pHint->GetId() == SBX_HINT_DATAWANTED )
        {
            // Parameter count -1 because of Param0 == this
            UINT32 nParamCount = pParams ? ((UINT32)pParams->Count() - 1) : 0;
            Sequence<Any> args;
            BOOL bOutParams = FALSE;

            Reference< XServiceConstructorDescription > xCtor = pUnoCtor->getServiceCtorDesc();
            Sequence< Reference< XParameter > > aParameterSeq = xCtor->getParameters();
            const Reference< XParameter >* pParameterSeq = aParameterSeq.getConstArray();
            UINT32 nUnoParamCount = aParameterSeq.getLength();

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
            USHORT nSbxParameterOffset = 1;
            USHORT nParameterOffsetByContext = 0;
            Reference < XComponentContext > xFirstParamContext;
            if( nParamCount > nUnoParamCount )
            {
                // Check if first parameter is a context and use it
                // then in createInstanceWithArgumentsAndContext
                Any aArg0 = sbxToUnoValue( pParams->Get( nSbxParameterOffset ) );
                if( (aArg0 >>= xFirstParamContext) && xFirstParamContext.is() )
                    nParameterOffsetByContext = 1;
            }

            UINT32 nEffectiveParamCount = nParamCount - nParameterOffsetByContext;
            UINT32 nAllocParamCount = nEffectiveParamCount;
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
                    StarBASIC::Error( SbERR_NOT_OPTIONAL );
                }
            }

            if( !bParameterError )
            {
                if( nAllocParamCount > 0 )
                {
                    args.realloc( nAllocParamCount );
                    Any* pAnyArgs = args.getArray();
                    for( UINT32 i = 0 ; i < nEffectiveParamCount ; i++ )
                    {
                        USHORT iSbx = (USHORT)(i + nSbxParameterOffset + nParameterOffsetByContext);

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
                            com::sun::star::uno::Type aType( xParamTypeDesc->getTypeClass(), xParamTypeDesc->getName() );

                            // sbx paramter needs offset 1
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ), aType );

                            // Check for out parameter if not already done
                            if( !bOutParams )
                            {
                                if( xParam->isOut() )
                                    bOutParams = TRUE;
                            }
                        }
                        else
                        {
                            pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ) );
                        }
                    }
                }

                // "Call" ctor using createInstanceWithArgumentsAndContext
                Reference < XComponentContext > xContext;
                if( xFirstParamContext.is() )
                {
                    xContext = xFirstParamContext;
                }
                else
                {
                    Reference < XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
                    xContext.set( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" )) ), UNO_QUERY_THROW );
                }
                Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );

                Any aRetAny;
                if( xServiceMgr.is() )
                {
                    String aServiceName = GetName();
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
                }
                unoToSbxValue( pVar, aRetAny );

                // Copy back out parameters?
                if( bOutParams )
                {
                    const Any* pAnyArgs = args.getConstArray();

                    for( UINT32 j = 0 ; j < nUnoParamCount ; j++ )
                    {
                        Reference< XParameter > xParam = pParameterSeq[j];
                        if( !xParam.is() )
                            continue;

                        if( xParam->isOut() )
                            unoToSbxValue( (SbxVariable*)pParams->Get( (USHORT)(j+1) ), pAnyArgs[ j ] );
                    }
                }
            }
        }
        else
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}



static SbUnoServiceCtor* pFirstCtor = NULL;

void clearUnoServiceCtors( void )
{
    SbUnoServiceCtor* pCtor = pFirstCtor;
    while( pCtor )
    {
        pCtor->SbxValue::Clear();
        pCtor = pCtor->pNext;
    }
}

SbUnoServiceCtor::SbUnoServiceCtor( const String& aName_, Reference< XServiceConstructorDescription > xServiceCtorDesc )
    : SbxMethod( aName_, SbxOBJECT )
    , m_xServiceCtorDesc( xServiceCtorDesc )
{
}

SbUnoServiceCtor::~SbUnoServiceCtor()
{
}

SbxInfo* SbUnoServiceCtor::GetInfo()
{
    SbxInfo* pRet = NULL;

    return pRet;
}


SbUnoSingleton* findUnoSingleton( const String& rName )
{
    SbUnoSingleton* pSbUnoSingleton = NULL;

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
                    pSbUnoSingleton = new SbUnoSingleton( rName, xSingletonTypeDesc );
            }
        }
    }
    return pSbUnoSingleton;
}

SbUnoSingleton::SbUnoSingleton( const String& aName_,
    const Reference< XSingletonTypeDescription >& xSingletonTypeDesc )
        : SbxObject( aName_ )
        , m_xSingletonTypeDesc( xSingletonTypeDesc )
{
    SbxVariableRef xGetMethodRef =
        new SbxMethod( String( RTL_CONSTASCII_USTRINGPARAM( "get" ) ), SbxOBJECT );
    QuickInsert( (SbxVariable*)xGetMethodRef );
}

void SbUnoSingleton::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pParams = pVar->GetParameters();
        UINT32 nParamCount = pParams ? ((UINT32)pParams->Count() - 1) : 0;
        UINT32 nAllowedParamCount = 1;

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
            Reference < XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
            xContextToUse.set( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" )) ), UNO_QUERY_THROW );
            --nAllowedParamCount;
        }

        if( nParamCount > nAllowedParamCount )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return;
        }

        Any aRetAny;
        if( xContextToUse.is() )
        {
            String aSingletonName( RTL_CONSTASCII_USTRINGPARAM("/singletons/") );
            aSingletonName += GetName();
            Reference < XInterface > xRet;
            xContextToUse->getValueByName( aSingletonName ) >>= xRet;
            aRetAny <<= xRet;
        }
        unoToSbxValue( pVar, aRetAny );
    }
    else
        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
}


//========================================================================
//========================================================================
//========================================================================

// Implementation of an EventAttacher-drawn AllListener, which
// solely transmits several events to an general AllListener
class BasicAllListener_Impl : public BasicAllListenerHelper
{
    virtual void firing_impl(const AllEventObject& Event, Any* pRet);

public:
    SbxObjectRef    xSbxObj;
    ::rtl::OUString     aPrefixName;

    BasicAllListener_Impl( const ::rtl::OUString& aPrefixName );
    ~BasicAllListener_Impl();

    // Methods of XInterface
    //virtual BOOL queryInterface( Uik aUik, Reference< XInterface > & rOut );

    // Methods of XAllListener
    virtual void SAL_CALL firing(const AllEventObject& Event) throw ( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw ( RuntimeException );

    // Methods of XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) throw ( RuntimeException );
};


//========================================================================
BasicAllListener_Impl::BasicAllListener_Impl
(
    const ::rtl::OUString   & aPrefixName_
)
    : aPrefixName( aPrefixName_ )
{
}

//========================================================================
BasicAllListener_Impl::~BasicAllListener_Impl()
{
}

//========================================================================

void BasicAllListener_Impl::firing_impl( const AllEventObject& Event, Any* pRet )
{
    SolarMutexGuard guard;

    if( xSbxObj.Is() )
    {
        ::rtl::OUString aMethodName = aPrefixName;
        aMethodName = aMethodName + Event.MethodName;

        SbxVariable * pP = xSbxObj;
        while( pP->GetParent() )
        {
            pP = pP->GetParent();
            StarBASIC * pLib = PTR_CAST(StarBASIC,pP);
            if( pLib )
            {
                // Create in a Basic Array
                SbxArrayRef xSbxArray = new SbxArray( SbxVARIANT );
                const Any * pArgs = Event.Arguments.getConstArray();
                INT32 nCount = Event.Arguments.getLength();
                for( INT32 i = 0; i < nCount; i++ )
                {
                    // Convert elements
                    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                    unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
                    xSbxArray->Put( xVar, sal::static_int_cast< USHORT >(i+1) );
                }

                pLib->Call( aMethodName, xSbxArray );

                // get the return value from the Param-Array, if requestet
                if( pRet )
                {
                    SbxVariable* pVar = xSbxArray->Get( 0 );
                    if( pVar )
                    {
                        // #95792 Avoid a second call
                        USHORT nFlags = pVar->GetFlags();
                        pVar->SetFlag( SBX_NO_BROADCAST );
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
void BasicAllListener_Impl::firing( const AllEventObject& Event ) throw ( RuntimeException )
{
    firing_impl( Event, NULL );
}

Any BasicAllListener_Impl::approveFiring( const AllEventObject& Event ) throw ( RuntimeException )
{
    Any aRetAny;
    firing_impl( Event, &aRetAny );
    return aRetAny;
}

//========================================================================
// Methods of XEventListener
void BasicAllListener_Impl ::disposing(const EventObject& ) throw ( RuntimeException )
{
    SolarMutexGuard guard;

    xSbxObj.Clear();
}



//*************************************************************************
//  class InvocationToAllListenerMapper
//  helper class to map XInvocation to XAllListener (also in project eventattacher!)
//*************************************************************************
class InvocationToAllListenerMapper : public WeakImplHelper1< XInvocation >
{
public:
    InvocationToAllListenerMapper( const Reference< XIdlClass >& ListenerType,
        const Reference< XAllListener >& AllListener, const Any& Helper );

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(void) throw( RuntimeException );
    virtual Any SAL_CALL invoke(const ::rtl::OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual void SAL_CALL setValue(const ::rtl::OUString& PropertyName, const Any& Value)
        throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual Any SAL_CALL getValue(const ::rtl::OUString& PropertyName) throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const ::rtl::OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const ::rtl::OUString& Name) throw( RuntimeException );

private:
    Reference< XIdlReflection >  m_xCoreReflection;
    Reference< XAllListener >    m_xAllListener;
    Reference< XIdlClass >       m_xListenerType;
    Any                          m_Helper;
};


// Function to replace AllListenerAdapterService::createAllListerAdapter
Reference< XInterface > createAllListenerAdapter
(
    const Reference< XInvocationAdapterFactory >& xInvocationAdapterFactory,
    const Reference< XIdlClass >& xListenerType,
    const Reference< XAllListener >& xListener,
    const Any& Helper
)
{
    Reference< XInterface > xAdapter;
    if( xInvocationAdapterFactory.is() && xListenerType.is() && xListener.is() )
    {
       Reference< XInvocation > xInvocationToAllListenerMapper =
            (XInvocation*)new InvocationToAllListenerMapper( xListenerType, xListener, Helper );
        Type aListenerType( xListenerType->getTypeClass(), xListenerType->getName() );
        xAdapter = xInvocationAdapterFactory->createAdapter( xInvocationToAllListenerMapper, aListenerType );
    }
    return xAdapter;
}


//--------------------------------------------------------------------------------------------------
// InvocationToAllListenerMapper
InvocationToAllListenerMapper::InvocationToAllListenerMapper
    ( const Reference< XIdlClass >& ListenerType, const Reference< XAllListener >& AllListener, const Any& Helper )
        : m_xAllListener( AllListener )
        , m_xListenerType( ListenerType )
        , m_Helper( Helper )
{
}

//*************************************************************************
Reference< XIntrospectionAccess > SAL_CALL InvocationToAllListenerMapper::getIntrospection(void)
    throw( RuntimeException )
{
    return Reference< XIntrospectionAccess >();
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::invoke(const ::rtl::OUString& FunctionName, const Sequence< Any >& Params,
    Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException,
        InvocationTargetException, RuntimeException )
{
    (void)OutParamIndex;
    (void)OutParam     ;

    Any aRet;

    // Check if to firing or approveFiring has to be called
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( FunctionName );
    sal_Bool bApproveFiring = sal_False;
    if( !xMethod.is() )
        return aRet;
    Reference< XIdlClass > xReturnType = xMethod->getReturnType();
    Sequence< Reference< XIdlClass > > aExceptionSeq = xMethod->getExceptionTypes();
    if( ( xReturnType.is() && xReturnType->getTypeClass() != TypeClass_VOID ) ||
        aExceptionSeq.getLength() > 0 )
    {
        bApproveFiring = sal_True;
    }
    else
    {
        Sequence< ParamInfo > aParamSeq = xMethod->getParameterInfos();
        sal_uInt32 nParamCount = aParamSeq.getLength();
        if( nParamCount > 1 )
        {
            const ParamInfo* pInfos = aParamSeq.getConstArray();
            for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
            {
                if( pInfos[ i ].aMode != ParamMode_IN )
                {
                    bApproveFiring = sal_True;
                    break;
                }
            }
        }
    }

    AllEventObject aAllEvent;
    aAllEvent.Source = (OWeakObject*) this;
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

//*************************************************************************
void SAL_CALL InvocationToAllListenerMapper::setValue(const ::rtl::OUString& PropertyName, const Any& Value)
    throw( UnknownPropertyException, CannotConvertException,
           InvocationTargetException, RuntimeException )
{
    (void)PropertyName;
    (void)Value;
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::getValue(const ::rtl::OUString& PropertyName)
    throw( UnknownPropertyException, RuntimeException )
{
    (void)PropertyName;

    return Any();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasMethod(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( Name );
    return xMethod.is();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasProperty(const ::rtl::OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlField > xField = m_xListenerType->getField( Name );
    return xField.is();
}

//========================================================================
// Uno-Service erzeugen
// 1. Parameter == Prefix-Name of the macro
// 2. Parameter == fully qualified name of the listener
void SbRtl_CreateUnoListener( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
//RTLFUNC(CreateUnoListener)
{
    (void)bWrite;

    // We need 2 parameters
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    String aPrefixName = rPar.Get(1)->GetString();
    String aListenerClassName = rPar.Get(2)->GetString();

    // get the CoreReflection
    Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
    if( !xCoreReflection.is() )
        return;

    // get the AllListenerAdapterService
    Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
    if( !xFactory.is() )
        return;

    // search the class
    Reference< XIdlClass > xClass = xCoreReflection->forName( aListenerClassName );
    if( !xClass.is() )
        return;

    // From 1999-11-30: get the InvocationAdapterFactory
    Reference< XInvocationAdapterFactory > xInvocationAdapterFactory = Reference< XInvocationAdapterFactory >(
        xFactory->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.InvocationAdapterFactory")) ), UNO_QUERY );

    BasicAllListener_Impl * p;
    Reference< XAllListener > xAllLst = p = new BasicAllListener_Impl( aPrefixName );
    Any aTmp;
    Reference< XInterface > xLst = createAllListenerAdapter( xInvocationAdapterFactory, xClass, xAllLst, aTmp );
    if( !xLst.is() )
        return;

    ::rtl::OUString aClassName = xClass->getName();
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
    refVar->PutObject( p->xSbxObj );
}

//========================================================================
// Represents the DefaultContext property of the ProcessServiceManager
// in the Basic runtime system.
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    SbxVariableRef refVar = rPar.Get(0);

    Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
    Reference< XPropertySet> xPSMPropertySet( xFactory, UNO_QUERY );
    if( xPSMPropertySet.is() )
    {
        Any aContextAny = xPSMPropertySet->getPropertyValue(
            String( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) );

        SbUnoObjectRef xUnoObj = new SbUnoObject
            ( String( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ),
              aContextAny );
        refVar->PutObject( (SbUnoObject*)xUnoObj );
    }
    else
    {
        refVar->PutObject( NULL );
    }
}

//========================================================================
// Creates a Basic wrapper object for a strongly typed Uno value
// 1. parameter: Uno type as full qualified type name, e.g. "byte[]"
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite )
{
    (void)pBasic;
    (void)bWrite;

    // 2 parameters needed
    if ( rPar.Count() != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // get the name of the class of the struct
    String aTypeName = rPar.Get(1)->GetString();
    SbxVariable* pVal = rPar.Get(2);

    // Check the type
    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    Any aRet;
    try
    {
        aRet = xTypeAccess->getByHierarchicalName( aTypeName );
    }
    catch( NoSuchElementException& e1 )
    {
        String aNoSuchElementExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.NoSuchElementException" ) );
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e1, aNoSuchElementExceptionName ) );
        return;
    }
    Reference< XTypeDescription > xTypeDesc;
    aRet >>= xTypeDesc;
    TypeClass eTypeClass = xTypeDesc->getTypeClass();
    Type aDestType( eTypeClass, aTypeName );


    // Preconvert value
    Any aVal = sbxToUnoValueImpl( pVal );
    Any aConvertedVal = convertAny( aVal, aDestType );

    /*
    // Convert
    Reference< XTypeConverter > xConverter = getTypeConverter_Impl();
    try
    {
        aConvertedVal = xConverter->convertTo( aVal, aDestType );
    }
    catch( IllegalArgumentException& e1 )
    {
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( ::cppu::getCaughtException() ) );
        return;
    }
    catch( CannotConvertException& e2 )
    {
        String aCannotConvertExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.IllegalArgumentException" ) );
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e2, aCannotConvertExceptionName ) );
        return;
    }
    */

    SbxVariableRef refVar = rPar.Get(0);
    SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aConvertedVal );
    refVar->PutObject( xUnoAnyObject );
}

//==========================================================================

typedef WeakImplHelper1< XInvocation > ModuleInvocationProxyHelper;

class ModuleInvocationProxy : public ModuleInvocationProxyHelper
{
    ::rtl::OUString     m_aPrefix;
    SbxObjectRef        m_xScopeObj;
    bool                m_bProxyIsClassModuleObject;

public:
    ModuleInvocationProxy( const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj );
    ~ModuleInvocationProxy()
    {}

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection() throw();
    virtual void SAL_CALL setValue( const ::rtl::OUString& rProperty, const Any& rValue )
        throw( UnknownPropertyException );
    virtual Any SAL_CALL getValue( const ::rtl::OUString& rProperty )
        throw( UnknownPropertyException );
    virtual sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& rName ) throw();
    virtual sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& rProp ) throw();

    virtual Any SAL_CALL invoke( const ::rtl::OUString& rFunction,
                                 const Sequence< Any >& rParams,
                                 Sequence< sal_Int16 >& rOutParamIndex,
                                 Sequence< Any >& rOutParam )
        throw( CannotConvertException, InvocationTargetException );
};

ModuleInvocationProxy::ModuleInvocationProxy( const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj )
    : m_aPrefix( aPrefix + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_") ) )
    , m_xScopeObj( xScopeObj )
{
    m_bProxyIsClassModuleObject = xScopeObj.Is() ? xScopeObj->ISA(SbClassModuleObject) : false;
}

Reference< XIntrospectionAccess > SAL_CALL ModuleInvocationProxy::getIntrospection() throw()
{
    return Reference< XIntrospectionAccess >();
}

void SAL_CALL ModuleInvocationProxy::setValue( const ::rtl::OUString& rProperty, const Any& rValue ) throw( UnknownPropertyException )
{
    if( !m_bProxyIsClassModuleObject )
        throw UnknownPropertyException();

    SolarMutexGuard guard;

    ::rtl::OUString aPropertyFunctionName( RTL_CONSTASCII_USTRINGPARAM( "Property Set ") );
    aPropertyFunctionName += m_aPrefix;
    aPropertyFunctionName += rProperty;

    SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxCLASS_METHOD );
    SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
    if( pMeth == NULL )
    {
        // TODO: Check vba behavior concernig missing function
        //StarBASIC::Error( SbERR_NO_METHOD, aFunctionName );
        throw UnknownPropertyException();
    }

    // Setup parameter
    SbxArrayRef xArray = new SbxArray;
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    unoToSbxValue( (SbxVariable*)xVar, rValue );
    xArray->Put( xVar, 1 );

    // Call property method
    SbxVariableRef xValue = new SbxVariable;
    pMeth->SetParameters( xArray );
    pMeth->Call( xValue );
    //aRet = sbxToUnoValue( xValue );
    pMeth->SetParameters( NULL );

    // TODO: OutParameter?

    // throw InvocationTargetException();

    //return aRet;

}

Any SAL_CALL ModuleInvocationProxy::getValue( const ::rtl::OUString& rProperty ) throw( UnknownPropertyException )
{
    if( !m_bProxyIsClassModuleObject )
        throw UnknownPropertyException();

    SolarMutexGuard guard;

    ::rtl::OUString aPropertyFunctionName( RTL_CONSTASCII_USTRINGPARAM( "Property Get ") );
    aPropertyFunctionName += m_aPrefix;
    aPropertyFunctionName += rProperty;

    SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxCLASS_METHOD );
    SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
    if( pMeth == NULL )
    {
        // TODO: Check vba behavior concernig missing function
        //StarBASIC::Error( SbERR_NO_METHOD, aFunctionName );
        throw UnknownPropertyException();
    }

    // Call method
    SbxVariableRef xValue = new SbxVariable;
    pMeth->Call( xValue );
    Any aRet = sbxToUnoValue( xValue );
    return aRet;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasMethod( const ::rtl::OUString& ) throw()
{
    return sal_False;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasProperty( const ::rtl::OUString& ) throw()
{
    return sal_False;
}

Any SAL_CALL ModuleInvocationProxy::invoke( const ::rtl::OUString& rFunction,
                                            const Sequence< Any >& rParams,
                                            Sequence< sal_Int16 >&,
                                            Sequence< Any >& )
    throw( CannotConvertException, InvocationTargetException )
{
    SolarMutexGuard guard;

    Any aRet;
    if( !m_xScopeObj.Is() )
        return aRet;

    ::rtl::OUString aFunctionName = m_aPrefix;
    aFunctionName += rFunction;

    SbxVariable* p = m_xScopeObj->Find( aFunctionName, SbxCLASS_METHOD );
    SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
    if( pMeth == NULL )
    {
        // TODO: Check vba behavior concernig missing function
        //StarBASIC::Error( SbERR_NO_METHOD, aFunctionName );
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
            unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
            xArray->Put( xVar, sal::static_int_cast< USHORT >(i+1) );
        }
    }

    // Call method
    SbxVariableRef xValue = new SbxVariable;
    if( xArray.Is() )
        pMeth->SetParameters( xArray );
    pMeth->Call( xValue );
    aRet = sbxToUnoValue( xValue );
    pMeth->SetParameters( NULL );

    // TODO: OutParameter?

    return aRet;
}

Reference< XInterface > createComListener( const Any& aControlAny, const ::rtl::OUString& aVBAType,
                                           const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj )
{
    Reference< XInterface > xRet;

    Reference< XComponentContext > xContext = getComponentContext_Impl();
    Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );

    Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPrefix, xScopeObj );

    Sequence<Any> args( 3 );
    args[0] <<= aControlAny;
    args[1] <<= aVBAType;
    args[2] <<= xProxy;

    try
    {
        xRet = xServiceMgr->createInstanceWithArgumentsAndContext(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.custom.UnoComListener")),
            args, xContext );
    }
    catch( const Exception& )
    {
        implHandleAnyException( ::cppu::getCaughtException() );
    }

    return xRet;
}

// Handle module implements mechanism for OLE types
bool SbModule::createCOMWrapperForIface( Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject )
{
    // For now: Take first interface that allows to instantiate COM wrapper
    // TODO: Check if support for multiple interfaces is needed

    Reference< XComponentContext > xContext = getComponentContext_Impl();
    Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() );
    Reference< XSingleServiceFactory > xComImplementsFactory
    (
        xServiceMgr->createInstanceWithContext(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.custom.ComImplementsFactory")), xContext ),
        UNO_QUERY
    );
    if( !xComImplementsFactory.is() )
        return false;

    bool bSuccess = false;

    SbxArray* pModIfaces = pClassData->mxIfaces;
    USHORT nCount = pModIfaces->Count();
    for( USHORT i = 0 ; i < nCount ; ++i )
    {
        SbxVariable* pVar = pModIfaces->Get( i );
        ::rtl::OUString aIfaceName = pVar->GetName();

        if( aIfaceName.getLength() != 0 )
        {
            ::rtl::OUString aPureIfaceName = aIfaceName;
            sal_Int32 indexLastDot = aIfaceName.lastIndexOf('.');
            if ( indexLastDot > -1 )
                aPureIfaceName = aIfaceName.copy( indexLastDot + 1 );

            Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPureIfaceName, pProxyClassModuleObject );

            Sequence<Any> args( 2 );
            args[0] <<= aIfaceName;
            args[1] <<= xProxy;

            Reference< XInterface > xRet;
            bSuccess = false;
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
                o_rRetAny <<= xRet;
                break;
            }
        }
     }

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
