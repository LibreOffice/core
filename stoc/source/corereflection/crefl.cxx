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
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <uno/lbnames.h>

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::reflection;
using namespace css::registry;
using namespace cppu;
using namespace osl;


#include "base.hxx"


namespace stoc_corefl
{

static const sal_Int32 CACHE_SIZE = 256;

#define IMPLNAME    "com.sun.star.comp.stoc.CoreReflection"

static Sequence< OUString > core_getSupportedServiceNames()
{
    Sequence< OUString > seqNames { "com.sun.star.reflection.CoreReflection" };
    return seqNames;
}

static OUString core_getImplementationName()
{
    return OUString(IMPLNAME);
}

IdlReflectionServiceImpl::IdlReflectionServiceImpl(
    const Reference< XComponentContext > & xContext )
    : OComponentHelper( _aComponentMutex )
    , _xMgr( xContext->getServiceManager(), UNO_QUERY )
    , _aElements( CACHE_SIZE )
{
    xContext->getValueByName(
        "/singletons/com.sun.star.reflection.theTypeDescriptionManager" ) >>= _xTDMgr;
    OSL_ENSURE( _xTDMgr.is(), "### cannot get singleton \"TypeDescriptionManager\" from context!" );
}

IdlReflectionServiceImpl::~IdlReflectionServiceImpl() {}

// XInterface

Any IdlReflectionServiceImpl::queryInterface( const Type & rType )
    throw(css::uno::RuntimeException, std::exception)
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XIdlReflection * >( this ),
        static_cast< XHierarchicalNameAccess * >( this ),
        static_cast< XServiceInfo * >( this ) ) );

    return (aRet.hasValue() ? aRet : OComponentHelper::queryInterface( rType ));
}

void IdlReflectionServiceImpl::acquire() throw()
{
    OComponentHelper::acquire();
}

void IdlReflectionServiceImpl::release() throw()
{
    OComponentHelper::release();
}

// XTypeProvider

Sequence< Type > IdlReflectionServiceImpl::getTypes()
    throw (css::uno::RuntimeException, std::exception)
{
    static OTypeCollection * s_pTypes = nullptr;
    if (! s_pTypes)
    {
        MutexGuard aGuard( _aComponentMutex );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                cppu::UnoType<XIdlReflection>::get(),
                cppu::UnoType<XHierarchicalNameAccess>::get(),
                cppu::UnoType<XServiceInfo>::get(),
                OComponentHelper::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}

Sequence< sal_Int8 > IdlReflectionServiceImpl::getImplementationId()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XComponent

void IdlReflectionServiceImpl::dispose()
    throw(css::uno::RuntimeException, std::exception)
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

OUString IdlReflectionServiceImpl::getImplementationName()
    throw(css::uno::RuntimeException, std::exception)
{
    return core_getImplementationName();
}

sal_Bool IdlReflectionServiceImpl::supportsService( const OUString & rServiceName )
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > IdlReflectionServiceImpl::getSupportedServiceNames()
    throw(css::uno::RuntimeException, std::exception)
{
    return core_getSupportedServiceNames();
}

// XIdlReflection

Reference< XIdlClass > IdlReflectionServiceImpl::getType( const Any & rObj )
    throw(css::uno::RuntimeException, std::exception)
{
    return (rObj.hasValue() ? forType( rObj.getValueTypeRef() ) : Reference< XIdlClass >());
}


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
    case typelib_TypeClass_EXCEPTION:
        return new CompoundIdlClassImpl( this, pTypeDescr->pTypeName, pTypeDescr->eTypeClass, pTypeDescr );

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

Reference< XIdlClass > IdlReflectionServiceImpl::forName( const OUString & rTypeName )
    throw(css::uno::RuntimeException, std::exception)
{
    Reference< XIdlClass > xRet;
    Any aAny( _aElements.getValue( rTypeName ) );

    if (aAny.hasValue())
    {
        if (aAny.getValueTypeClass() == TypeClass_INTERFACE)
            xRet = *static_cast<const Reference< XIdlClass > *>(aAny.getValue());
    }
    else
    {
        // try to get _type_ by name
        typelib_TypeDescription * pTD = nullptr;
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

Any IdlReflectionServiceImpl::getByHierarchicalName( const OUString & rName )
    throw(css::container::NoSuchElementException, css::uno::RuntimeException, std::exception)
{
    Any aRet( _aElements.getValue( rName ) );
    if (! aRet.hasValue())
    {
        aRet = _xTDMgr->getByHierarchicalName( rName );
        if (aRet.getValueTypeClass() == TypeClass_INTERFACE)
        {
            // type retrieved from tdmgr
            OSL_ASSERT( (*static_cast<Reference< XInterface > const *>(aRet.getValue()))->queryInterface(
                cppu::UnoType<XTypeDescription>::get()).hasValue() );

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
                typelib_TypeDescription * pTD = nullptr;
                typelib_typedescription_getByName( &pTD, rName.pData );

                aRet.clear(); // kick XTypeDescription interface

                if (pTD)
                {
                    Reference< XIdlClass > xIdlClass( constructClass( pTD ) );
                    aRet.setValue( &xIdlClass, cppu::UnoType<XIdlClass>::get());
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
            throw container::NoSuchElementException( rName );
        }
    }
    return aRet;
}

sal_Bool IdlReflectionServiceImpl::hasByHierarchicalName( const OUString & rName )
    throw(css::uno::RuntimeException, std::exception)
{
    try
    {
        return getByHierarchicalName( rName ).hasValue();
    }
    catch (container::NoSuchElementException &)
    {
    }
    return sal_False;
}


Reference< XIdlClass > IdlReflectionServiceImpl::forType( typelib_TypeDescription * pTypeDescr )
    throw(css::uno::RuntimeException)
{
    Reference< XIdlClass > xRet;
    OUString aName( pTypeDescr->pTypeName );
    Any aAny( _aElements.getValue( aName ) );

    if (aAny.hasValue())
    {
        if (aAny.getValueTypeClass() == TypeClass_INTERFACE)
            xRet = *static_cast<const Reference< XIdlClass > *>(aAny.getValue());
    }
    else
    {
        if ((xRet = constructClass( pTypeDescr )).is())
            _aElements.setValue( aName, makeAny( xRet ) ); // update
    }

    return xRet;
}

Reference< XIdlClass > IdlReflectionServiceImpl::forType( typelib_TypeDescriptionReference * pRef )
    throw(css::uno::RuntimeException)
{
    typelib_TypeDescription * pTD = nullptr;
    TYPELIB_DANGER_GET( &pTD, pRef );
    if (pTD)
    {
        Reference< XIdlClass > xRet = forType( pTD );
        TYPELIB_DANGER_RELEASE( pTD );
        return xRet;
    }
    throw RuntimeException(
        "IdlReflectionServiceImpl::forType() failed!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
}


const Mapping & IdlReflectionServiceImpl::getCpp2Uno()
    throw(css::uno::RuntimeException)
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
                    "cannot get c++ to uno mapping!",
                    static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
            }
        }
    }
    return _aCpp2Uno;
}

const Mapping & IdlReflectionServiceImpl::getUno2Cpp()
    throw(css::uno::RuntimeException)
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
                    "cannot get uno to c++ mapping!",
                    static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
            }
        }
    }
    return _aUno2Cpp;
}

uno_Interface * IdlReflectionServiceImpl::mapToUno(
    const Any & rObj, typelib_InterfaceTypeDescription * pTo )
    throw(css::uno::RuntimeException)
{
    Reference< XInterface > xObj;
    if (extract( rObj, pTo, xObj, this ))
        return static_cast<uno_Interface *>(getCpp2Uno().mapInterface( xObj.get(), pTo ));

    throw RuntimeException(
        "illegal object given!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
}


Reference< XInterface > SAL_CALL IdlReflectionServiceImpl_create(
    const Reference< XComponentContext > & xContext )
    throw(css::uno::Exception)
{
    return Reference< XInterface >( static_cast<XWeak *>(static_cast<OWeakObject *>(new IdlReflectionServiceImpl( xContext ))) );
}

}


using namespace stoc_corefl;

static const struct ImplementationEntry g_entries[] =
{
    {
        IdlReflectionServiceImpl_create, core_getImplementationName,
        core_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL reflection_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
