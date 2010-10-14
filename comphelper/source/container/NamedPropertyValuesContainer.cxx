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
#include "precompiled_comphelper.hxx"

#include "comphelper_module.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/stl_types.hxx>


#include <map>


using namespace com::sun::star;

DECLARE_STL_USTRINGACCESS_MAP( uno::Sequence<beans::PropertyValue>, NamedPropertyValues );

class NamedPropertyValuesContainer : public cppu::WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
public:
    NamedPropertyValuesContainer() throw();
    virtual ~NamedPropertyValuesContainer() throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException,
            ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static versions (used for component registration)
    static ::rtl::OUString SAL_CALL getImplementationName_static();
    static uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
    static uno::Reference< uno::XInterface > SAL_CALL Create( const uno::Reference< uno::XComponentContext >& );

private:
    NamedPropertyValues maProperties;
};

NamedPropertyValuesContainer::NamedPropertyValuesContainer() throw()
{
}

NamedPropertyValuesContainer::~NamedPropertyValuesContainer() throw()
{
}

// XNameContainer
void SAL_CALL NamedPropertyValuesContainer::insertByName( const rtl::OUString& aName, const uno::Any& aElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if( maProperties.find( aName ) != maProperties.end() )
        throw container::ElementExistException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps ) )
        throw lang::IllegalArgumentException();

    maProperties.insert(  NamedPropertyValues::value_type(aName ,aProps) );
}

void SAL_CALL NamedPropertyValuesContainer::removeByName( const ::rtl::OUString& Name )
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
        ::com::sun::star::uno::RuntimeException)
{
    NamedPropertyValues::iterator aIter = maProperties.find( Name );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    maProperties.erase( aIter );
}

// XNameReplace
void SAL_CALL NamedPropertyValuesContainer::replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps) )
        throw lang::IllegalArgumentException();

    (*aIter).second = aProps;
}

// XNameAccess
::com::sun::star::uno::Any SAL_CALL NamedPropertyValuesContainer::getByName( const ::rtl::OUString& aName )
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
        ::com::sun::star::uno::RuntimeException)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Any aElement;

    aElement <<= (*aIter).second;

    return aElement;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL NamedPropertyValuesContainer::getElementNames(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    NamedPropertyValues::iterator aIter = maProperties.begin();
    const NamedPropertyValues::iterator aEnd = maProperties.end();

    uno::Sequence< rtl::OUString > aNames( maProperties.size() );
    rtl::OUString* pNames = aNames.getArray();

    while( aIter != aEnd )
    {
        *pNames++ = (*aIter++).first;
    }

    return aNames;
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasByName( const ::rtl::OUString& aName )
    throw(::com::sun::star::uno::RuntimeException)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    return aIter != maProperties.end();
}

// XElementAccess
::com::sun::star::uno::Type SAL_CALL NamedPropertyValuesContainer::getElementType(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType((uno::Sequence<beans::PropertyValue> *)0);
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasElements(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return !maProperties.empty();
}

//XServiceInfo
::rtl::OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getImplementationName_static();
}

::rtl::OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName_static(  )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NamedPropertyValuesContainer" ) );
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.NamedPropertyValues" ) );
    return aServiceName == ServiceName;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}


::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames_static(  )
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.NamedPropertyValues" ) );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL NamedPropertyValuesContainer::Create(
                const uno::Reference< uno::XComponentContext >&)
{
    return (cppu::OWeakObject*)new NamedPropertyValuesContainer();
}

void createRegistryInfo_NamedPropertyValuesContainer()
{
    static ::comphelper::module::OAutoRegistration< NamedPropertyValuesContainer > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
