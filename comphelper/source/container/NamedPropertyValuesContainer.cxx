/*************************************************************************
 *
 *  $RCSfile: NamedPropertyValuesContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-09 19:20:35 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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


#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef __SGI_STL_MAP
#include <map>
#endif


using namespace com::sun::star;

DECLARE_STL_USTRINGACCESS_MAP( uno::Sequence<beans::PropertyValue>, NamedPropertyValues );

uno::Sequence< rtl::OUString > SAL_CALL NamedPropertyValuesContainer_getSupportedServiceNames() throw();
rtl::OUString SAL_CALL NamedPropertyValuesContainer_getImplementationName() throw();
uno::Reference< uno::XInterface > SAL_CALL NamedPropertyValuesContainer_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception );

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

    maProperties[ aName ] = aProps;
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
    return maProperties.size() != 0;
}

//XServiceInfo
::rtl::OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return NamedPropertyValuesContainer_getImplementationName();
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.NamedPropertyValues" ) );
    return aServiceName == ServiceName;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return NamedPropertyValuesContainer_getSupportedServiceNames();
}


uno::Sequence< rtl::OUString > SAL_CALL NamedPropertyValuesContainer_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.NamedPropertyValues" ) );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

rtl::OUString SAL_CALL NamedPropertyValuesContainer_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NamedPropertyValuesContainer" ) );
}

uno::Reference< uno::XInterface > SAL_CALL NamedPropertyValuesContainer_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new NamedPropertyValuesContainer();
}

