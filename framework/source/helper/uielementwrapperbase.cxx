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

#include <helper/uielementwrapperbase.hxx>
#include <general.h>
#include <properties.h>
#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/svapp.hxx>
#include <comphelper/sequence.hxx>

const int UIELEMENT_PROPHANDLE_RESOURCEURL  = 1;
const int UIELEMENT_PROPHANDLE_TYPE         = 2;
const int UIELEMENT_PROPHANDLE_FRAME        = 3;
const int UIELEMENT_PROPCOUNT               = 3;
const char UIELEMENT_PROPNAME_RESOURCEURL[] = "ResourceURL";
const char UIELEMENT_PROPNAME_TYPE[] = "Type";
const char UIELEMENT_PROPNAME_FRAME[] = "Frame";

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace framework
{

UIElementWrapperBase::UIElementWrapperBase( sal_Int16 nType )
    :   ThreadHelpBase              ( &Application::GetSolarMutex()                      )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
    ,   m_nType                     ( nType                                             )
    ,   m_bInitialized              ( sal_False                                         )
    ,   m_bDisposed                 ( sal_False                                         )
{
}

UIElementWrapperBase::~UIElementWrapperBase()
{
}

Any SAL_CALL UIElementWrapperBase::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aRet = UIElementWrapperBase_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}

Sequence< Type > SAL_CALL UIElementWrapperBase::getTypes(  ) throw(RuntimeException)
{
    return comphelper::concatSequences(
        UIElementWrapperBase_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

void SAL_CALL UIElementWrapperBase::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIElementWrapperBase::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIElementWrapperBase::initialize( const Sequence< Any >& aArguments )
throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bInitialized )
    {
        for ( sal_Int32 n = 0; n < aArguments.getLength(); n++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[n] >>= aPropValue )
            {
                if ( aPropValue.Name == "ResourceURL" )
                    aPropValue.Value >>= m_aResourceURL;
                else if ( aPropValue.Name == "Frame" )
                {
                    Reference< XFrame > xFrame;
                    aPropValue.Value >>= xFrame;
                    m_xWeakFrame = xFrame;
                }
            }
        }

        m_bInitialized = sal_True;
    }
}


::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL UIElementWrapperBase::getFrame() throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( m_xWeakFrame );
    return xFrame;
}

OUString SAL_CALL UIElementWrapperBase::getResourceURL() throw (::com::sun::star::uno::RuntimeException)
{
    return m_aResourceURL;
}

::sal_Int16 SAL_CALL UIElementWrapperBase::getType() throw (::com::sun::star::uno::RuntimeException)
{
    return m_nType;
}


void SAL_CALL UIElementWrapperBase::update() throw (::com::sun::star::uno::RuntimeException)
{
    
}


sal_Bool SAL_CALL UIElementWrapperBase::convertFastPropertyValue( Any&       /*aConvertedValue*/ ,
                                                                  Any&       /*aOldValue*/       ,
                                                                  sal_Int32  /*nHandle*/         ,
                                                                  const Any& /*aValue*/             ) throw( com::sun::star::lang::IllegalArgumentException )
{
    
    
    return sal_False ;
}

void SAL_CALL UIElementWrapperBase::setFastPropertyValue_NoBroadcast(   sal_Int32               /*nHandle*/ ,
                                                                        const com::sun::star::uno::Any&    /*aValue*/  ) throw( com::sun::star::uno::Exception )
{
}

void SAL_CALL UIElementWrapperBase::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                          sal_Int32      nHandle                ) const
{
    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue <<= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue <<= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
            Reference< XFrame > xFrame( m_xWeakFrame );
            aValue <<= xFrame;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL UIElementWrapperBase::getInfoHelper()
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

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL UIElementWrapperBase::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
{
    
    
    
    static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        
        if( pInfo == NULL )
        {
            
            
            static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > UIElementWrapperBase::impl_getStaticPropertyDescriptor()
{
    
    
    
    
    
    

    const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_FRAME), UIELEMENT_PROPHANDLE_FRAME          , ::getCppuType((Reference< XFrame >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_RESOURCEURL), UIELEMENT_PROPHANDLE_RESOURCEURL    , ::getCppuType((sal_Int16*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_TYPE), UIELEMENT_PROPHANDLE_TYPE           , ::getCppuType((const OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY )
    };
    
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, UIELEMENT_PROPCOUNT );
    
    return lPropertyDescriptor;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
