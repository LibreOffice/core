/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
   return ((OUString *)arg1)->compareTo( ((Property *)arg2)->Name );
}
}

class OPropertySetHelperInfo_Impl
    : public WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
    Sequence < Property > aInfos;

public:
    OPropertySetHelperInfo_Impl( IPropertyArrayHelper & rHelper_ ) SAL_THROW(());

    
    virtual Sequence< Property > SAL_CALL getProperties(void) throw(::com::sun::star::uno::RuntimeException);
    virtual Property SAL_CALL getPropertyByName(const OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& PropertyName) throw(::com::sun::star::uno::RuntimeException);
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
Property OPropertySetHelperInfo_Impl::getPropertyByName( const OUString & PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
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
sal_Bool OPropertySetHelperInfo_Impl::hasPropertyByName( const OUString & PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    Property * pR;
    pR = (Property *)bsearch( &PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                              sizeof( Property ),
                              compare_OUString_Property_Impl );
    return pR != NULL;
}

ConstItemContainer::ConstItemContainer()
{
}

ConstItemContainer::ConstItemContainer( const ItemContainer& rItemContainer )
{
    ShareGuard aLock( rItemContainer.m_aShareMutex );
    copyItemContainer( rItemContainer.m_aItemVector );
}

ConstItemContainer::ConstItemContainer( const Reference< XIndexAccess >& rSourceContainer, sal_Bool bFastCopy )
{
    
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


sal_Bool SAL_CALL ConstItemContainer::hasElements()
throw ( RuntimeException )
{
    return ( !m_aItemVector.empty() );
}


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
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}


Reference< XPropertySetInfo > SAL_CALL ConstItemContainer::getPropertySetInfo()
throw (::com::sun::star::uno::RuntimeException)
{
    
    
    
    static Reference< XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        
        if( pInfo == NULL )
        {
            
            
            static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

void SAL_CALL ConstItemContainer::setPropertyValue( const OUString&, const Any& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

Any SAL_CALL ConstItemContainer::getPropertyValue( const OUString& PropertyName )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if ( PropertyName == PROPNAME_UINAME )
        return makeAny( m_aUIName );

    throw UnknownPropertyException();
}

void SAL_CALL ConstItemContainer::addPropertyChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ConstItemContainer::removePropertyChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    
}

void SAL_CALL ConstItemContainer::addVetoableChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    
}

void SAL_CALL ConstItemContainer::removeVetoableChangeListener( const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& )
throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    
}


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
    
    
    
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        
        if( pInfoHelper == NULL )
        {
            
            
            
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > ConstItemContainer::impl_getStaticPropertyDescriptor()
{
    
    
    
    
    
    

    const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( OUString(PROPNAME_UINAME), PROPHANDLE_UINAME ,
                                         ::getCppuType((const OUString*)NULL),
                                         com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY  )
    };
    
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, PROPCOUNT );
    
    return lPropertyDescriptor;
}

Reference < XPropertySetInfo > ConstItemContainer::createPropertySetInfo(
    IPropertyArrayHelper & rProperties ) SAL_THROW(())
{
    return static_cast< XPropertySetInfo * >( new OPropertySetHelperInfo_Impl( rProperties ) );
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
