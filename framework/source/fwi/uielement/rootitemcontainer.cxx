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

#include <string.h>

#include <comphelper/servicehelper.hxx>
#include <comphelper/sequence.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/itemcontainer.hxx>
#include <uielement/constitemcontainer.hxx>
#include <threadhelp/resetableguard.hxx>
#include <general.h>
#include <properties.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

const char WRONG_TYPE_EXCEPTION[] = "Type must be com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >";

const int PROPHANDLE_UINAME     = 1;
const int PROPCOUNT             = 1;
const char PROPNAME_UINAME[]    = "UIName";

namespace framework
{

RootItemContainer::RootItemContainer()
    :   ThreadHelpBase              ( )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
{
}

RootItemContainer::RootItemContainer( const Reference< XIndexAccess >& rSourceContainer )
    :   ThreadHelpBase              ( )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
{
    // We also have to copy the UIName property
    try
    {
        Reference< XPropertySet > xPropSet( rSourceContainer, UNO_QUERY );
        if ( xPropSet.is() )
        {
            xPropSet->getPropertyValue("UIName") >>= m_aUIName;
        }
    }
    catch ( const Exception& )
    {
    }

    if ( rSourceContainer.is() )
    {
        sal_Int32 nCount = rSourceContainer->getCount();
        try
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
        catch ( const IndexOutOfBoundsException& )
        {
        }
    }
}

RootItemContainer::~RootItemContainer()
{
}

Any SAL_CALL RootItemContainer::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aRet = RootItemContainer_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}
//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL RootItemContainer::getTypes(  ) throw(RuntimeException)
{
    return comphelper::concatSequences(
        RootItemContainer_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

Reference< XIndexAccess > RootItemContainer::deepCopyContainer( const Reference< XIndexAccess >& rSubContainer )
{
    Reference< XIndexAccess > xReturn;
    if ( rSubContainer.is() )
    {
        ConstItemContainer* pSource = ConstItemContainer::GetImplementation( rSubContainer );
        ItemContainer* pSubContainer( 0 );
        if ( pSource )
            pSubContainer = new ItemContainer( *pSource, m_aShareMutex );
        else
            pSubContainer = new ItemContainer( rSubContainer, m_aShareMutex );
        xReturn = Reference< XIndexAccess >( static_cast< OWeakObject* >( pSubContainer ), UNO_QUERY );
    }

    return xReturn;
}

// XUnoTunnel
sal_Int64 RootItemContainer::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( RootItemContainer::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    return 0;
}

namespace
{
    class theRootItemContainerUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theRootItemContainerUnoTunnelId > {};
}

const Sequence< sal_Int8 >& RootItemContainer::GetUnoTunnelId() throw()
{
    return theRootItemContainerUnoTunnelId::get().getSeq();
}

RootItemContainer* RootItemContainer::GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( rxIFace, ::com::sun::star::uno::UNO_QUERY );
    return xUT.is() ? reinterpret_cast< RootItemContainer* >(sal::static_int_cast< sal_IntPtr >(
                          xUT->getSomething( RootItemContainer::GetUnoTunnelId() ))) : NULL;
}

// XElementAccess
sal_Bool SAL_CALL RootItemContainer::hasElements()
throw ( RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    return ( !m_aItemVector.empty() );
}

// XIndexAccess
sal_Int32 SAL_CALL RootItemContainer::getCount()
throw ( RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    return m_aItemVector.size();
}

Any SAL_CALL RootItemContainer::getByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    if ( sal_Int32( m_aItemVector.size()) > Index )
        return makeAny( m_aItemVector[Index] );
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XIndexContainer
void SAL_CALL RootItemContainer::insertByIndex( sal_Int32 Index, const Any& aItem )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    Sequence< PropertyValue > aSeq;
    if ( aItem >>= aSeq )
    {
        ShareGuard aLock( m_aShareMutex );
        if ( sal_Int32( m_aItemVector.size()) == Index )
            m_aItemVector.push_back( aSeq );
        else if ( sal_Int32( m_aItemVector.size()) >Index )
        {
            std::vector< Sequence< PropertyValue > >::iterator aIter = m_aItemVector.begin();
            aIter += Index;
            m_aItemVector.insert( aIter, aSeq );
        }
        else
            throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
    }
    else
        throw IllegalArgumentException( OUString( WRONG_TYPE_EXCEPTION ),
                                        (OWeakObject *)this, 2 );
}

void SAL_CALL RootItemContainer::removeByIndex( sal_Int32 nIndex )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ShareGuard aLock( m_aShareMutex );
    if ( (sal_Int32)m_aItemVector.size() > nIndex )
    {
        m_aItemVector.erase(m_aItemVector.begin() + nIndex);
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

void SAL_CALL RootItemContainer::replaceByIndex( sal_Int32 Index, const Any& aItem )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    Sequence< PropertyValue > aSeq;
    if ( aItem >>= aSeq )
    {
        ShareGuard aLock( m_aShareMutex );
        if ( sal_Int32( m_aItemVector.size()) > Index )
            m_aItemVector[Index] = aSeq;
        else
            throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
    }
    else
        throw IllegalArgumentException( OUString( WRONG_TYPE_EXCEPTION ),
                                        (OWeakObject *)this, 2 );
}

Reference< XInterface > SAL_CALL RootItemContainer::createInstanceWithContext( const Reference< XComponentContext >& )
throw ( Exception, RuntimeException)
{
    return (OWeakObject *)(new ItemContainer( m_aShareMutex ));
}

Reference< XInterface > SAL_CALL RootItemContainer::createInstanceWithArgumentsAndContext( const Sequence< Any >&, const Reference< XComponentContext >& )
throw (Exception, RuntimeException)
{
    return (OWeakObject *)(new ItemContainer( m_aShareMutex ));
}

// XPropertySet helper
sal_Bool SAL_CALL RootItemContainer::convertFastPropertyValue( Any&       aConvertedValue ,
                                                               Any&       aOldValue       ,
                                                               sal_Int32  nHandle         ,
                                                               const Any& aValue             )
throw( com::sun::star::lang::IllegalArgumentException )
{
    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case PROPHANDLE_UINAME:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_aUIName),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;
    }

    // Return state of operation.
    return bReturn ;
}

void SAL_CALL RootItemContainer::setFastPropertyValue_NoBroadcast( sal_Int32               nHandle ,
                                                                   const com::sun::star::uno::Any&    aValue  )
throw( com::sun::star::uno::Exception )
{
    switch( nHandle )
    {
        case PROPHANDLE_UINAME:
            aValue >>= m_aUIName;
            break;
    }
}

void SAL_CALL RootItemContainer::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                       sal_Int32                 nHandle                ) const
{
    switch( nHandle )
    {
        case PROPHANDLE_UINAME:
            aValue <<= m_aUIName;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL RootItemContainer::getInfoHelper()
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

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL RootItemContainer::getPropertySetInfo()
throw (::com::sun::star::uno::RuntimeException)
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > RootItemContainer::impl_getStaticPropertyDescriptor()
{
    // Create a property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( OUString(PROPNAME_UINAME), PROPHANDLE_UINAME ,
                                         ::getCppuType((const OUString*)NULL),
                                         com::sun::star::beans::PropertyAttribute::TRANSIENT )
    };
    // Use it to initialize sequence!
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, PROPCOUNT );
    // Return "PropertyDescriptor"
    return lPropertyDescriptor;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
