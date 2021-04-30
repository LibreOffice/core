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
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <sal/log.hxx>

#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <o3tl/any.hxx>
#include <uno/lbnames.h>

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::reflection;
using namespace cppu;
using namespace osl;


#include "base.hxx"


namespace stoc_corefl
{

IdlReflectionServiceImpl::IdlReflectionServiceImpl(
    const Reference< XComponentContext > & xContext )
    : OComponentHelper( _aComponentMutex )
    , _aElements()
{
    xContext->getValueByName(
        "/singletons/com.sun.star.reflection.theTypeDescriptionManager" ) >>= _xTDMgr;
    OSL_ENSURE( _xTDMgr.is(), "### cannot get singleton \"TypeDescriptionManager\" from context!" );
}

IdlReflectionServiceImpl::~IdlReflectionServiceImpl() {}

// XInterface

Any IdlReflectionServiceImpl::queryInterface( const Type & rType )
{
    Any aRet( ::cppu::queryInterface(
        rType,
        static_cast< XIdlReflection * >( this ),
        static_cast< XHierarchicalNameAccess * >( this ),
        static_cast< XServiceInfo * >( this ) ) );

    return (aRet.hasValue() ? aRet : OComponentHelper::queryInterface( rType ));
}

void IdlReflectionServiceImpl::acquire() noexcept
{
    OComponentHelper::acquire();
}

void IdlReflectionServiceImpl::release() noexcept
{
    OComponentHelper::release();
}

// XTypeProvider

Sequence< Type > IdlReflectionServiceImpl::getTypes()
{
    static OTypeCollection s_aTypes(
        cppu::UnoType<XIdlReflection>::get(),
        cppu::UnoType<XHierarchicalNameAccess>::get(),
        cppu::UnoType<XServiceInfo>::get(),
        OComponentHelper::getTypes() );

    return s_aTypes.getTypes();
}

Sequence< sal_Int8 > IdlReflectionServiceImpl::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XComponent

void IdlReflectionServiceImpl::dispose()
{
    OComponentHelper::dispose();

    MutexGuard aGuard( _aComponentMutex );
    _aElements.clear();
#ifdef TEST_LIST_CLASSES
    OSL_ENSURE( g_aClassNames.empty(), "### idl classes still alive!" );
    for (auto const& className : g_aClassNames)
    {
        OUString aName(className);
    }
#endif
}

// XServiceInfo

OUString IdlReflectionServiceImpl::getImplementationName()
{
    return "com.sun.star.comp.stoc.CoreReflection";
}

sal_Bool IdlReflectionServiceImpl::supportsService( const OUString & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > IdlReflectionServiceImpl::getSupportedServiceNames()
{
    Sequence< OUString > seqNames { "com.sun.star.reflection.CoreReflection" };
    return seqNames;
}

// XIdlReflection

Reference< XIdlClass > IdlReflectionServiceImpl::getType( const Any & rObj )
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

    case typelib_TypeClass_ENUM:
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
        SAL_INFO("stoc", "corereflection type unsupported: " << pTypeDescr->pTypeName);
        return Reference< XIdlClass >();
    }
}

Reference< XIdlClass > IdlReflectionServiceImpl::forName( const OUString & rTypeName )
{
    Reference< XIdlClass > xRet;
    Any aAny( _aElements.getValue( rTypeName ) );

    if (aAny.hasValue())
    {
        aAny >>= xRet;
    }
    else
    {
        // try to get _type_ by name
        typelib_TypeDescription * pTD = nullptr;
        typelib_typedescription_getByName( &pTD, rTypeName.pData );
        if (pTD)
        {
            xRet = constructClass( pTD );
            if (xRet.is())
                _aElements.setValue( rTypeName, makeAny( xRet ) ); // update
            typelib_typedescription_release( pTD );
        }
    }

    return xRet;
}

// XHierarchicalNameAccess

Any IdlReflectionServiceImpl::getByHierarchicalName( const OUString & rName )
{
    Any aRet( _aElements.getValue( rName ) );
    if (! aRet.hasValue())
    {
        aRet = _xTDMgr->getByHierarchicalName( rName );
        if (aRet.getValueTypeClass() == TypeClass_INTERFACE)
        {
            // type retrieved from tdmgr
            OSL_ASSERT( (*o3tl::forceAccess<Reference<XInterface>>(aRet))->queryInterface(
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

                // if td manager found some type, it will be in the cache (hopefully... we just got it)
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
        if (!aRet.hasValue())
            throw container::NoSuchElementException( rName );

        _aElements.setValue( rName, aRet );
    }
    return aRet;
}

sal_Bool IdlReflectionServiceImpl::hasByHierarchicalName( const OUString & rName )
{
    try
    {
        return getByHierarchicalName( rName ).hasValue();
    }
    catch (container::NoSuchElementException &)
    {
    }
    return false;
}


Reference< XIdlClass > IdlReflectionServiceImpl::forType( typelib_TypeDescription * pTypeDescr )
{
    Reference< XIdlClass > xRet;
    OUString aName( pTypeDescr->pTypeName );
    Any aAny( _aElements.getValue( aName ) );

    if (aAny.hasValue())
    {
        aAny >>= xRet;
    }
    else
    {
        xRet = constructClass( pTypeDescr );
        if (xRet.is())
            _aElements.setValue( aName, makeAny( xRet ) ); // update
    }

    return xRet;
}

Reference< XIdlClass > IdlReflectionServiceImpl::forType( typelib_TypeDescriptionReference * pRef )
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
{
    if (! _aCpp2Uno.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _aCpp2Uno.is())
        {
            _aCpp2Uno = Mapping( CPPU_CURRENT_LANGUAGE_BINDING_NAME, UNO_LB_UNO );
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
{
    if (! _aUno2Cpp.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _aUno2Cpp.is())
        {
            _aUno2Cpp = Mapping( UNO_LB_UNO, CPPU_CURRENT_LANGUAGE_BINDING_NAME );
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
{
    Reference< XInterface > xObj;
    if (extract( rObj, pTo, xObj, this ))
        return static_cast<uno_Interface *>(getCpp2Uno().mapInterface( xObj.get(), pTo ));

    throw RuntimeException(
        "illegal object given!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
}

}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_stoc_CoreReflection_get_implementation(
    css::uno::XComponentContext * context,
    css::uno::Sequence<css::uno::Any> const & arguments)
{
    SAL_WARN_IF(
        arguments.hasElements(), "stoc", "unexpected singleton arguments");
    return cppu::acquire(new stoc_corefl::IdlReflectionServiceImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
