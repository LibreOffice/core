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

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"

using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;


#include "base.hxx"


namespace stoc_corefl
{

static const sal_Int32 CACHE_SIZE = 256;

#define SERVICENAME "com.sun.star.reflection.CoreReflection"
#define IMPLNAME    "com.sun.star.comp.stoc.CoreReflection"

static Sequence< OUString > core_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString( SERVICENAME );
    return seqNames;
}

static OUString core_getImplementationName()
{
    return OUString(IMPLNAME);
}
//__________________________________________________________________________________________________
IdlReflectionServiceImpl::IdlReflectionServiceImpl(
    const Reference< XComponentContext > & xContext )
    : OComponentHelper( _aComponentMutex )
    , _xMgr( xContext->getServiceManager(), UNO_QUERY )
    , _aElements( CACHE_SIZE )
{
    xContext->getValueByName( OUString(
        "/singletons/com.sun.star.reflection.theTypeDescriptionManager") ) >>= _xTDMgr;
    OSL_ENSURE( _xTDMgr.is(), "### cannot get singleton \"TypeDescriptionManager\" from context!" );
}
//__________________________________________________________________________________________________
IdlReflectionServiceImpl::~IdlReflectionServiceImpl() {}

// XInterface
//__________________________________________________________________________________________________
Any IdlReflectionServiceImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XIdlReflection * >( this ),
        static_cast< XHierarchicalNameAccess * >( this ),
        static_cast< XServiceInfo * >( this ) ) );

    return (aRet.hasValue() ? aRet : OComponentHelper::queryInterface( rType ));
}
//__________________________________________________________________________________________________
void IdlReflectionServiceImpl::acquire() throw()
{
    OComponentHelper::acquire();
}
//__________________________________________________________________________________________________
void IdlReflectionServiceImpl::release() throw()
{
    OComponentHelper::release();
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > IdlReflectionServiceImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( _aComponentMutex );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIdlReflection > *)0 ),
                ::getCppuType( (const Reference< XHierarchicalNameAccess > *)0 ),
                ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                OComponentHelper::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > IdlReflectionServiceImpl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( _aComponentMutex );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}

// XComponent
//__________________________________________________________________________________________________
void IdlReflectionServiceImpl::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    OComponentHelper::dispose();

    MutexGuard aGuard( _aComponentMutex );
    _aElements.clear();
#ifdef TEST_LIST_CLASSES
    OSL_ENSURE( g_aClassNames.empty(), "### idl classes still alive!" );
    ClassNameList::const_iterator iPos( g_aClassNames.begin() );
    while (iPos != g_aClassNames.end())
    {
        OUString aName( *iPos );
        ++iPos;
    }
#endif
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString IdlReflectionServiceImpl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return core_getImplementationName();
}
//__________________________________________________________________________________________________
sal_Bool IdlReflectionServiceImpl::supportsService( const OUString & rServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > IdlReflectionServiceImpl::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return core_getSupportedServiceNames();
}

// XIdlReflection
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlReflectionServiceImpl::getType( const Any & rObj )
    throw(::com::sun::star::uno::RuntimeException)
{
    return (rObj.hasValue() ? forType( rObj.getValueTypeRef() ) : Reference< XIdlClass >());
}

//__________________________________________________________________________________________________
inline Reference< XIdlClass > IdlReflectionServiceImpl::constructClass(
    typelib_TypeDescription * pTypeDescr )
{
    OSL_ENSURE( pTypeDescr->eTypeClass != typelib_TypeClass_TYPEDEF, "### unexpected typedef!" );

    switch (pTypeDescr->eTypeClass)
    {
    case typelib_TypeClass_VOID:
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
    case typelib_TypeClass_STRING:
    case typelib_TypeClass_ANY:
        return new IdlClassImpl( this, pTypeDescr->pTypeName, pTypeDescr->eTypeClass, pTypeDescr );

    case TypeClass_ENUM:
        return new EnumIdlClassImpl( this, pTypeDescr->pTypeName, pTypeDescr->eTypeClass, pTypeDescr );

    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_UNION:
    case typelib_TypeClass_EXCEPTION:
        return new CompoundIdlClassImpl( this, pTypeDescr->pTypeName, pTypeDescr->eTypeClass, pTypeDescr );

    case typelib_TypeClass_ARRAY:
    case typelib_TypeClass_SEQUENCE:
        return new ArrayIdlClassImpl( this, pTypeDescr->pTypeName, pTypeDescr->eTypeClass, pTypeDescr );

    case typelib_TypeClass_INTERFACE:
        return new InterfaceIdlClassImpl( this, pTypeDescr->pTypeName, pTypeDescr->eTypeClass, pTypeDescr );

    case typelib_TypeClass_TYPE:
        return new IdlClassImpl( this, pTypeDescr->pTypeName, pTypeDescr->eTypeClass, pTypeDescr );

    default:
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE( "### corereflection type unsupported: " );
        OString aName( OUStringToOString( pTypeDescr->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "%s", aName.getStr() );
        OSL_TRACE( "\n" );
#endif
        return Reference< XIdlClass >();
    }
}
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlReflectionServiceImpl::forName( const OUString & rTypeName )
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XIdlClass > xRet;
    Any aAny( _aElements.getValue( rTypeName ) );

    if (aAny.hasValue())
    {
        if (aAny.getValueTypeClass() == TypeClass_INTERFACE)
            xRet = *(const Reference< XIdlClass > *)aAny.getValue();
    }
    else
    {
        // try to get _type_ by name
        typelib_TypeDescription * pTD = 0;
        typelib_typedescription_getByName( &pTD, rTypeName.pData );
        if (pTD)
        {
            if ((xRet = constructClass( pTD )).is())
                _aElements.setValue( rTypeName, makeAny( xRet ) ); // update
            typelib_typedescription_release( pTD );
        }
    }

    return xRet;
}

// XHierarchicalNameAccess
//__________________________________________________________________________________________________
Any IdlReflectionServiceImpl::getByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
    Any aRet( _aElements.getValue( rName ) );
    if (! aRet.hasValue())
    {
        aRet = _xTDMgr->getByHierarchicalName( rName );
        if (aRet.getValueTypeClass() == TypeClass_INTERFACE)
        {
            // type retrieved from tdmgr
            OSL_ASSERT( (*(Reference< XInterface > *)aRet.getValue())->queryInterface(
                ::getCppuType( (const Reference< XTypeDescription > *)0 ) ).hasValue() );

            css::uno::Reference< css::reflection::XConstantTypeDescription >
                ctd;
            if (aRet >>= ctd)
            {
                aRet = ctd->getConstantValue();
            }
            else
            {
                // if you are interested in a type then CALL forName()!!!
                // this way is NOT recommended for types, because this method looks for constants first

                // if td manager found some type, it will be in the cache (hopefully.. we just got it)
                // so the second retrieving via c typelib callback chain should succeed...

                // try to get _type_ by name
                typelib_TypeDescription * pTD = 0;
                typelib_typedescription_getByName( &pTD, rName.pData );

                aRet.clear(); // kick XTypeDescription interface

                if (pTD)
                {
                    Reference< XIdlClass > xIdlClass( constructClass( pTD ) );
                    aRet.setValue( &xIdlClass, ::getCppuType( (const Reference< XIdlClass > *)0 ) );
                    typelib_typedescription_release( pTD );
                }
            }
        }
        // else is enum member(?)

        // update
        if (aRet.hasValue())
            _aElements.setValue( rName, aRet );
        else
        {
            throw NoSuchElementException( rName, Reference< XInterface >() );
        }
    }
    return aRet;
}
//__________________________________________________________________________________________________
sal_Bool IdlReflectionServiceImpl::hasByHierarchicalName( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    try
    {
        return getByHierarchicalName( rName ).hasValue();
    }
    catch (NoSuchElementException &)
    {
    }
    return sal_False;
}

//__________________________________________________________________________________________________
Reference< XIdlClass > IdlReflectionServiceImpl::forType( typelib_TypeDescription * pTypeDescr )
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XIdlClass > xRet;
    OUString aName( pTypeDescr->pTypeName );
    Any aAny( _aElements.getValue( aName ) );

    if (aAny.hasValue())
    {
        if (aAny.getValueTypeClass() == TypeClass_INTERFACE)
            xRet = *(const Reference< XIdlClass > *)aAny.getValue();
    }
    else
    {
        if (pTypeDescr && (xRet = constructClass( pTypeDescr )).is())
            _aElements.setValue( aName, makeAny( xRet ) ); // update
    }

    return xRet;
}
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlReflectionServiceImpl::forType( typelib_TypeDescriptionReference * pRef )
    throw(::com::sun::star::uno::RuntimeException)
{
    typelib_TypeDescription * pTD = 0;
    TYPELIB_DANGER_GET( &pTD, pRef );
    if (pTD)
    {
        Reference< XIdlClass > xRet = forType( pTD );
        TYPELIB_DANGER_RELEASE( pTD );
        return xRet;
    }
    throw RuntimeException(
        OUString( "IdlReflectionServiceImpl::forType() failed!" ),
        (XWeak *)(OWeakObject *)this );
}

//__________________________________________________________________________________________________
const Mapping & IdlReflectionServiceImpl::getCpp2Uno()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _aCpp2Uno.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _aCpp2Uno.is())
        {
            _aCpp2Uno = Mapping(
                OUString( CPPU_CURRENT_LANGUAGE_BINDING_NAME ),
                OUString( UNO_LB_UNO ) );
            OSL_ENSURE( _aCpp2Uno.is(), "### cannot get c++ to uno mapping!" );
            if (! _aCpp2Uno.is())
            {
                throw RuntimeException(
                    OUString("cannot get c++ to uno mapping!"),
                    (XWeak *)(OWeakObject *)this );
            }
        }
    }
    return _aCpp2Uno;
}
//__________________________________________________________________________________________________
const Mapping & IdlReflectionServiceImpl::getUno2Cpp()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _aUno2Cpp.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _aUno2Cpp.is())
        {
            _aUno2Cpp = Mapping(
                OUString( UNO_LB_UNO ),
                OUString( CPPU_CURRENT_LANGUAGE_BINDING_NAME ) );
            OSL_ENSURE( _aUno2Cpp.is(), "### cannot get uno to c++ mapping!" );
            if (! _aUno2Cpp.is())
            {
                throw RuntimeException(
                    OUString("cannot get uno to c++ mapping!"),
                    (XWeak *)(OWeakObject *)this );
            }
        }
    }
    return _aUno2Cpp;
}
//__________________________________________________________________________________________________
uno_Interface * IdlReflectionServiceImpl::mapToUno(
    const Any & rObj, typelib_InterfaceTypeDescription * pTo )
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > xObj;
    if (extract( rObj, pTo, xObj, this ))
        return (uno_Interface *)getCpp2Uno().mapInterface( xObj.get(), pTo );

    throw RuntimeException(
        OUString("illegal object given!"),
        (XWeak *)(OWeakObject *)this );
}

//==================================================================================================
Reference< XInterface > SAL_CALL IdlReflectionServiceImpl_create(
    const Reference< XComponentContext > & xContext )
    throw(::com::sun::star::uno::Exception)
{
    return Reference< XInterface >( (XWeak *)(OWeakObject *)new IdlReflectionServiceImpl( xContext ) );
}

}


using namespace stoc_corefl;

static const struct ImplementationEntry g_entries[] =
{
    {
        IdlReflectionServiceImpl_create, core_getImplementationName,
        core_getSupportedServiceNames, createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL reflection_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
