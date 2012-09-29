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

#include <string.h>

#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/itemcontainer.hxx>
#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <cppuhelper/implbase1.hxx>
#include <comphelper/servicehelper.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

const int PROPHANDLE_UINAME     = 1;
const int PROPCOUNT             = 1;
const char PROPNAME_UINAME[]    = "UIName";

namespace framework
{

/**
 * The class which implements the PropertySetInfo interface.
 */
extern "C"
{
static int SAL_CALL compare_OUString_Property_Impl( const void *arg1, const void *arg2 ) SAL_THROW(())
{
   return ((::rtl::OUString *)arg1)->compareTo( ((Property *)arg2)->Name );
}
}

class OPropertySetHelperInfo_Impl
    : public WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
    Sequence < Property > aInfos;

public:
    OPropertySetHelperInfo_Impl( IPropertyArrayHelper & rHelper_ ) SAL_THROW(());

    // XPropertySetInfo-Methoden
    virtual Sequence< Property > SAL_CALL getProperties(void) throw(::com::sun::star::uno::RuntimeException);
    virtual Property SAL_CALL getPropertyByName(const ::rtl::OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& PropertyName) throw(::com::sun::star::uno::RuntimeException);
};


/**
 * Create an object that implements XPropertySetInfo IPropertyArrayHelper.
 */
OPropertySetHelperInfo_Impl::OPropertySetHelperInfo_Impl(
    IPropertyArrayHelper & rHelper_ )
    SAL_THROW(())
    :aInfos( rHelper_.getProperties() )
{
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
Sequence< Property > OPropertySetHelperInfo_Impl::getProperties(void) throw(::com::sun::star::uno::RuntimeException)
{
    return aInfos;
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
Property OPropertySetHelperInfo_Impl::getPropertyByName( const ::rtl::OUString & PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    Property * pR;
    pR = (Property *)bsearch( &PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    if( !pR ) {
        throw UnknownPropertyException();
    }

    return *pR;
}

/**
 * Return the sequence of properties, which are provided throug the constructor.
 */
sal_Bool OPropertySetHelperInfo_Impl::hasPropertyByName( const ::rtl::OUString & PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    Property * pR;
    pR = (Property *)bsearch( &PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR != NULL;
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_6     (   ConstItemContainer                                              ,
                            OWeakObject                                                     ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider         ),
                            DIRECT_INTERFACE( ::com::sun::star::container::XElementAccess   ),
                            DIRECT_INTERFACE( ::com::sun::star::container::XIndexAccess     ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet     ),
                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet         ),
                            DIRECT_INTERFACE( ::com::sun::star::lang::XUnoTunnel            )
                        )

DEFINE_XTYPEPROVIDER_6  (   ConstItemContainer                          ,
                            ::com::sun::star::lang::XTypeProvider       ,
                            ::com::sun::star::container::XIndexAccess   ,
                            ::com::sun::star::container::XElementAccess ,
                            ::com::sun::star::beans::XFastPropertySet   ,
                            ::com::sun::star::beans::XPropertySet       ,
                            ::com::sun::star::lang::XUnoTunnel
                        )

ConstItemContainer::ConstItemContainer() : ::cppu::OWeakObject()
{
}

ConstItemContainer::ConstItemContainer( const ItemContainer& rItemContainer )
{
    ShareGuard aLock( rItemContainer.m_aShareMutex );
    copyItemContainer( rItemContainer.m_aItemVector );
}

ConstItemContainer::ConstItemContainer( const Reference< XIndexAccess >& rSourceContainer, sal_Bool bFastCopy )
{
    // We also have to copy the UIName property
    try
    {
        Reference< XPropertySet > xPropSet( rSourceContainer, UNO_QUERY );
        if ( xPropSet.is() )
        {
            xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ))) >>= m_aUIName;
        }
    }
    catch ( const Exception& )
    {
    }

    if ( rSourceContainer.is() )
    {
        try
        {
            sal_Int32 nCount = rSourceContainer->getCount();
            m_aItemVector.reserve(nCount);
            if ( bFastCopy )
            {
                for ( sal_Int32 i = 0; i < nCount; i++ )
                {
                    Sequence< PropertyValue > aPropSeq;
                    if ( rSourceContainer->getByIndex( i ) >>= aPropSeq )
                        m_aItemVector.push_back( aPropSeq );
                }
            }
            else
            {
                for ( sal_Int32 i = 0; i < nCount; i++ )
                {
                    Sequence< PropertyValue > aPropSeq;
                    if ( rSourceContainer->getByIndex( i ) >>= aPropSeq )
                    {
                        sal_Int32 nContainerIndex = -1;
                        Reference< XIndexAccess > xIndexAccess;
                        for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
                        {
                            if ( aPropSeq[j].Name == "ItemDescriptorContainer" )
                            {
                                aPropSeq[j].Value >>= xIndexAccess;
                                nContainerIndex = j;
                                break;
                            }
                        }

                        if ( xIndexAccess.is() && nContainerIndex >= 0 )
                            aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess );

                        m_aItemVector.push_back( aPropSeq );
                    }
                }
            }
        }
        catch ( const IndexOutOfBoundsException& )
        {
        }
    }
}

ConstItemContainer::~ConstItemContainer()
{
}

// private
void ConstItemContainer::copyItemContainer( const std::vector< Sequence< PropertyValue > >& rSourceVector )
{
    const sal_uInt32 nCount = rSourceVector.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        sal_Int32 nContainerIndex = -1;
        Sequence< PropertyValue > aPropSeq( rSourceVector[i] );
        Reference< XIndexAccess > xIndexAccess;
        for ( sal_Int32 j = 0; j < aPropSeq.getLength(); j++ )
        {
            if ( aPropSeq[j].Name == "ItemDescriptorContainer" )
            {
                aPropSeq[j].Value >>= xIndexAccess;
                nContainerIndex = j;
                break;
            }
        }

        if ( xIndexAccess.is() && nContainerIndex >= 0 )
            aPropSeq[nContainerIndex].Value <<= deepCopyContainer( xIndexAccess );

        m_aItemVector.push_back( aPropSeq );
    }
}

Reference< XIndexAccess > ConstItemContainer::deepCopyContainer( const Reference< XIndexAccess >& rSubContainer )
{
    Reference< XIndexAccess > xReturn;
    if ( rSubContainer.is() )
    {
        ItemContainer*      pSource = ItemContainer::GetImplementation( rSubContainer );
        ConstItemContainer* pSubContainer( 0 );
        if ( pSource )
            pSubContainer = new ConstItemContainer( *pSource );
        else
            pSubContainer = new ConstItemContainer( rSubContainer );
        xReturn = Reference< XIndexAccess >( static_cast< OWeakObject* >( pSubContainer ), UNO_QUERY );
    }

    return xReturn;
}

// XUnoTunnel
sal_Int64 ConstItemContainer::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( ConstItemContainer::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
    {
        return reinterpret_cast< sal_Int64 >( this );
    }
    return 0;
}

namespace
{
    class theConstItemContainerUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theConstItemContainerUnoTunnelId > {};
}

const Sequence< sal_Int8 >& ConstItemContainer::GetUnoTunnelId() throw()
{
    return theConstItemContainerUnoTunnelId::get().getSeq();
}

ConstItemContainer* ConstItemContainer::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY );
    return xUT.is() ? reinterpret_cast< ConstItemContainer* >(sal::static_int_cast< sal_IntPtr >(
                          xUT->getSomething( ConstItemContainer::GetUnoTunnelId() ))) : NULL;
}

// XElementAccess
sal_Bool SAL_CALL ConstItemContainer::hasElements()
throw ( RuntimeException )
{
    return ( !m_aItemVector.empty() );
}

// XIndexAccess
sal_Int32 SAL_CALL ConstItemContainer::getCount()
throw ( RuntimeException )
{
    return m_aItemVector.size();
}

Any SAL_CALL ConstItemContainer::getByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if ( sal_Int32( m_aItemVector.size()) > Index )
        return makeAny( m_aItemVector[Index] );
    else
        throw IndexOutOfBoundsException( ::rtl::OUString(), (OWeakObject *)this );
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL ConstItemContainer::getPropertySetInfo()
throw (::com::sun::star::uno::RuntimeException)
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static Reference< XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

void SAL_CALL ConstItemContainer::setPropertyValue( const ::rtl::OUString&, const Any& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

Any SAL_CALL ConstItemContainer::getPropertyValue( const ::rtl::OUString& PropertyName )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if ( PropertyName == PROPNAME_UINAME )
        return makeAny( m_aUIName );

    throw UnknownPropertyException();
}

void SAL_CALL ConstItemContainer::addPropertyChangeListener( const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ConstItemContainer::removePropertyChangeListener( const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // Only read-only properties - do nothing
}

void SAL_CALL ConstItemContainer::addVetoableChangeListener( const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // Only read-only properties - do nothing
}

void SAL_CALL ConstItemContainer::removeVetoableChangeListener( const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    // Only read-only properties - do nothing
}

// XFastPropertySet
void SAL_CALL ConstItemContainer::setFastPropertyValue( sal_Int32, const ::com::sun::star::uno::Any& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

Any SAL_CALL ConstItemContainer::getFastPropertyValue( sal_Int32 nHandle )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if ( nHandle == PROPHANDLE_UINAME )
        return makeAny( m_aUIName );

    throw UnknownPropertyException();
}

::cppu::IPropertyArrayHelper& SAL_CALL ConstItemContainer::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > ConstItemContainer::impl_getStaticPropertyDescriptor()
{
    // Create a property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( rtl::OUString(PROPNAME_UINAME), PROPHANDLE_UINAME ,
                                         ::getCppuType((const rtl::OUString*)NULL),
                                         com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY  )
    };
    // Use it to initialize sequence!
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, PROPCOUNT );
    // Return "PropertyDescriptor"
    return lPropertyDescriptor;
}

Reference < XPropertySetInfo > ConstItemContainer::createPropertySetInfo(
    IPropertyArrayHelper & rProperties ) SAL_THROW(())
{
    return static_cast< XPropertySetInfo * >( new OPropertySetHelperInfo_Impl( rProperties ) );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
