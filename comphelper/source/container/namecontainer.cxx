/*************************************************************************
 *
 *  $RCSfile: namecontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-21 10:18:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

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

    maProperties[ aName ] = aElement;
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

    return maProperties.size() != 0;
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
