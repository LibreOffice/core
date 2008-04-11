/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: namecontainer.cxx,v $
 * $Revision: 1.6 $
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
#include <comphelper/namecontainer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <comphelper/stl_types.hxx>

DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Any, SvGenericNameContainerMapImpl );

namespace comphelper
{
    class NameContainerImpl
    {
    public:
        osl::Mutex maMutex;
    };

    /** this is the base helper class for NameContainer thats also declared in this header. */
    class NameContainer : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XNameContainer >, private NameContainerImpl
    {
    public:
        NameContainer( ::com::sun::star::uno::Type aType );
        virtual ~NameContainer();

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
        virtual sal_Bool SAL_CALL hasElements(  )
            throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
            throw(::com::sun::star::uno::RuntimeException);

    private:
        SvGenericNameContainerMapImpl maProperties;
        const ::com::sun::star::uno::Type maType;
    };
}

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


NameContainer::NameContainer( ::com::sun::star::uno::Type aType )
: maType( aType )
{
}

NameContainer::~NameContainer()
{
}

// XNameContainer
void SAL_CALL NameContainer::insertByName( const rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException,
        WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( maMutex );

    if( maProperties.find( aName ) != maProperties.end() )
        throw ElementExistException();

    if( aElement.getValueType() != maType )
        throw IllegalArgumentException();

    maProperties.insert( SvGenericNameContainerMapImpl::value_type(aName,aElement));
}

void SAL_CALL NameContainer::removeByName( const ::rtl::OUString& Name )
    throw(NoSuchElementException, WrappedTargetException,
        RuntimeException)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( Name );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    maProperties.erase( aIter );
}

// XNameReplace

void SAL_CALL NameContainer::replaceByName( const ::rtl::OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException,
        WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter( maProperties.find( aName ) );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    if( aElement.getValueType() != maType )
        throw IllegalArgumentException();

    (*aIter).second = aElement;
}

// XNameAccess

Any SAL_CALL NameContainer::getByName( const ::rtl::OUString& aName )
    throw(NoSuchElementException, WrappedTargetException,
        RuntimeException)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    return (*aIter).second;
}

Sequence< ::rtl::OUString > SAL_CALL NameContainer::getElementNames(  )
    throw(RuntimeException)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.begin();
    const SvGenericNameContainerMapImpl::iterator aEnd = maProperties.end();

    Sequence< rtl::OUString > aNames( maProperties.size() );
    rtl::OUString* pNames = aNames.getArray();

    while( aIter != aEnd )
    {
        *pNames++ = (*aIter++).first;
    }

    return aNames;
}

sal_Bool SAL_CALL NameContainer::hasByName( const ::rtl::OUString& aName )
    throw(RuntimeException)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( aName );
    return aIter != maProperties.end();
}

sal_Bool SAL_CALL NameContainer::hasElements(  )
    throw(RuntimeException)
{
    MutexGuard aGuard( maMutex );

    return !maProperties.empty();
}

Type SAL_CALL NameContainer::getElementType()
    throw( RuntimeException )
{
    return maType;
}

Reference< XNameContainer > comphelper::NameContainer_createInstance( Type aType )
{
    return (XNameContainer*) new NameContainer( aType );
}
