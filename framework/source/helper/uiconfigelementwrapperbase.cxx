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

#include <helper/uiconfigelementwrapperbase.hxx>
#include <general.h>
#include <properties.h>
#include <threadhelp/resetableguard.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>

#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>
#include <comphelper/sequence.hxx>

const int UIELEMENT_PROPHANDLE_CONFIGSOURCE     = 1;
const int UIELEMENT_PROPHANDLE_FRAME            = 2;
const int UIELEMENT_PROPHANDLE_PERSISTENT       = 3;
const int UIELEMENT_PROPHANDLE_RESOURCEURL      = 4;
const int UIELEMENT_PROPHANDLE_TYPE             = 5;
const int UIELEMENT_PROPHANDLE_XMENUBAR         = 6;
const int UIELEMENT_PROPHANDLE_CONFIGLISTENER   = 7;
const int UIELEMENT_PROPHANDLE_NOCLOSE          = 8;
const int UIELEMENT_PROPCOUNT                   = 8;
const char UIELEMENT_PROPNAME_CONFIGLISTENER[] = "ConfigListener";
const char UIELEMENT_PROPNAME_CONFIGSOURCE[] = "ConfigurationSource";
const char UIELEMENT_PROPNAME_FRAME[] = "Frame";
const char UIELEMENT_PROPNAME_PERSISTENT[] = "Persistent";
const char UIELEMENT_PROPNAME_RESOURCEURL[] = "ResourceURL";
const char UIELEMENT_PROPNAME_TYPE[] = "Type";
const char UIELEMENT_PROPNAME_XMENUBAR[] = "XMenuBar";
const char UIELEMENT_PROPNAME_NOCLOSE[] = "NoClose";
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace ::com::sun::star::ui;

namespace framework
{

UIConfigElementWrapperBase::UIConfigElementWrapperBase( sal_Int16 nType )
    :   ThreadHelpBase              ( &Application::GetSolarMutex()                      )
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    ,   m_nType                     ( nType                                             )
    ,   m_bPersistent               ( sal_True                                          )
    ,   m_bInitialized              ( sal_False                                         )
    ,   m_bConfigListener           ( sal_False                                         )
    ,   m_bConfigListening          ( sal_False                                         )
    ,   m_bDisposed                 ( sal_False                                         )
    ,   m_bNoClose                  ( sal_False                                         )
    ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
{
}

UIConfigElementWrapperBase::~UIConfigElementWrapperBase()
{
}

Any SAL_CALL UIConfigElementWrapperBase::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aRet = UIConfigElementWrapperBase_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}

Sequence< Type > SAL_CALL UIConfigElementWrapperBase::getTypes(  ) throw(RuntimeException)
{
    return comphelper::concatSequences(
        UIConfigElementWrapperBase_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

// XComponent
void SAL_CALL UIConfigElementWrapperBase::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIConfigElementWrapperBase::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), aListener );
}

// XEventListener
void SAL_CALL UIConfigElementWrapperBase::disposing( const EventObject& )
throw( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    m_xConfigSource.clear();
}

void SAL_CALL UIConfigElementWrapperBase::initialize( const Sequence< Any >& aArguments )
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
                if ( aPropValue.Name == UIELEMENT_PROPNAME_CONFIGSOURCE )
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_CONFIGSOURCE, aPropValue.Value );
                else if ( aPropValue.Name == UIELEMENT_PROPNAME_FRAME )
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_FRAME, aPropValue.Value );
                else if ( aPropValue.Name == UIELEMENT_PROPNAME_PERSISTENT )
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_PERSISTENT, aPropValue.Value );
                else if ( aPropValue.Name == UIELEMENT_PROPNAME_RESOURCEURL )
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_RESOURCEURL, aPropValue.Value );
                else if ( aPropValue.Name == UIELEMENT_PROPNAME_TYPE )
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_TYPE, aPropValue.Value );
                else if ( aPropValue.Name == UIELEMENT_PROPNAME_CONFIGLISTENER )
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_CONFIGLISTENER, aPropValue.Value );
                else if ( aPropValue.Name == UIELEMENT_PROPNAME_NOCLOSE )
                    setFastPropertyValue_NoBroadcast( UIELEMENT_PROPHANDLE_NOCLOSE, aPropValue.Value );
            }
        }

        m_bInitialized = sal_True;
    }
}

// XUpdatable
void SAL_CALL UIConfigElementWrapperBase::update() throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementInserted( const ::com::sun::star::ui::ConfigurationEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

void SAL_CALL UIConfigElementWrapperBase::elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    // can be implemented by derived class
}

// XPropertySet helper
sal_Bool SAL_CALL UIConfigElementWrapperBase::convertFastPropertyValue( Any&       aConvertedValue ,
                                                                        Any&       aOldValue       ,
                                                                        sal_Int32  nHandle         ,
                                                                        const Any& aValue             ) throw( com::sun::star::lang::IllegalArgumentException )
{
    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bConfigListener),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_xConfigSource),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame( m_xWeakFrame );
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(xFrame),
                        aValue,
                        aOldValue,
                        aConvertedValue);
        }
        break;

        case UIELEMENT_PROPHANDLE_PERSISTENT:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bPersistent),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_aResourceURL),
                        aValue,
                        aOldValue,
                        aConvertedValue);
            break;

        case UIELEMENT_PROPHANDLE_TYPE :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_nType),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;

        case UIELEMENT_PROPHANDLE_XMENUBAR :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_xMenuBar),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;

        case UIELEMENT_PROPHANDLE_NOCLOSE:
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_bNoClose),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;
    }

    // Return state of operation.
    return bReturn ;
}

void SAL_CALL UIConfigElementWrapperBase::setFastPropertyValue_NoBroadcast(   sal_Int32               nHandle ,
                                                                        const com::sun::star::uno::Any&    aValue  ) throw( com::sun::star::uno::Exception )
{
    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
        {
            bool bBool( m_bConfigListener );
            aValue >>= bBool;
            if ( m_bConfigListener != bBool )
            {
                if ( m_bConfigListening )
                {
                    if ( m_xConfigSource.is() && !bBool )
                    {
                        try
                        {
                            Reference< XUIConfiguration > xUIConfig( m_xConfigSource, UNO_QUERY );
                            if ( xUIConfig.is() )
                            {
                                xUIConfig->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                                m_bConfigListening = sal_False;
                            }
                        }
                        catch ( const Exception& )
                        {
                        }
                    }
                }
                else
                {
                    if ( m_xConfigSource.is() && bBool )
                    {
                        try
                        {
                            Reference< XUIConfiguration > xUIConfig( m_xConfigSource, UNO_QUERY );
                            if ( xUIConfig.is() )
                            {
                                xUIConfig->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                                m_bConfigListening = sal_True;
                            }
                        }
                        catch ( const Exception& )
                        {
                        }
                    }
                }

                m_bConfigListener = bBool;
            }
        }
        break;
        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            aValue >>= m_xConfigSource;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame;

            aValue >>= xFrame;
            m_xWeakFrame = xFrame;
            break;
        }
        case UIELEMENT_PROPHANDLE_PERSISTENT:
        {
            sal_Bool bBool( m_bPersistent );
            aValue >>= bBool;
            m_bPersistent = bBool;
            break;
        }
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue >>= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue >>= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_XMENUBAR:
            aValue >>= m_xMenuBar;
            break;
        case UIELEMENT_PROPHANDLE_NOCLOSE:
        {
            sal_Bool bBool( m_bNoClose );
            aValue >>= bBool;
            m_bNoClose = bBool;
            break;
        }
    }
}

void SAL_CALL UIConfigElementWrapperBase::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                                sal_Int32                 nHandle   ) const
{
    switch( nHandle )
    {
        case UIELEMENT_PROPHANDLE_CONFIGLISTENER:
            aValue <<= m_bConfigListener;
            break;
        case UIELEMENT_PROPHANDLE_CONFIGSOURCE:
            aValue <<= m_xConfigSource;
            break;
        case UIELEMENT_PROPHANDLE_FRAME:
        {
            Reference< XFrame > xFrame( m_xWeakFrame );
            aValue <<= xFrame;
            break;
        }
        case UIELEMENT_PROPHANDLE_PERSISTENT:
            aValue <<= m_bPersistent;
            break;
        case UIELEMENT_PROPHANDLE_RESOURCEURL:
            aValue <<= m_aResourceURL;
            break;
        case UIELEMENT_PROPHANDLE_TYPE:
            aValue <<= m_nType;
            break;
        case UIELEMENT_PROPHANDLE_XMENUBAR:
            aValue <<= m_xMenuBar;
            break;
        case UIELEMENT_PROPHANDLE_NOCLOSE:
            aValue <<= m_bNoClose;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL UIConfigElementWrapperBase::getInfoHelper()
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

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL UIConfigElementWrapperBase::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
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

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > UIConfigElementWrapperBase::impl_getStaticPropertyDescriptor()
{
    // Create property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_CONFIGLISTENER), UIELEMENT_PROPHANDLE_CONFIGLISTENER , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_CONFIGSOURCE), UIELEMENT_PROPHANDLE_CONFIGSOURCE   , ::getCppuType((const Reference< ::com::sun::star::ui::XUIConfigurationManager >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_FRAME), UIELEMENT_PROPHANDLE_FRAME          , ::getCppuType((const Reference< com::sun::star::frame::XFrame >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_NOCLOSE), UIELEMENT_PROPHANDLE_NOCLOSE        , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_PERSISTENT), UIELEMENT_PROPHANDLE_PERSISTENT     , ::getCppuType((const sal_Bool*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_RESOURCEURL), UIELEMENT_PROPHANDLE_RESOURCEURL    , ::getCppuType((const OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_TYPE), UIELEMENT_PROPHANDLE_TYPE           , ::getCppuType((const OUString*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY ),
        com::sun::star::beans::Property( OUString(UIELEMENT_PROPNAME_XMENUBAR), UIELEMENT_PROPHANDLE_XMENUBAR       , ::getCppuType((const Reference< com::sun::star::awt::XMenuBar >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY )
    };
    // Use it to initialize sequence!
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, UIELEMENT_PROPCOUNT );
    // Return "PropertyDescriptor"
    return lPropertyDescriptor;
}
void SAL_CALL UIConfigElementWrapperBase::setSettings( const Reference< XIndexAccess >& xSettings ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );


    if ( xSettings.is() )
    {
        // Create a copy of the data if the container is not const
        Reference< XIndexReplace > xReplace( xSettings, UNO_QUERY );
        if ( xReplace.is() )
            m_xConfigData = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( xSettings ) ), UNO_QUERY );
        else
            m_xConfigData = xSettings;

        if ( m_xConfigSource.is() && m_bPersistent )
        {
            OUString aResourceURL( m_aResourceURL );
            Reference< XUIConfigurationManager > xUICfgMgr( m_xConfigSource );

            aLock.unlock();

            try
            {
                xUICfgMgr->replaceSettings( aResourceURL, m_xConfigData );
            }
            catch( const NoSuchElementException& )
            {
            }
        }
        else if ( !m_bPersistent )
        {
            // Transient menubar => Fill menubar with new data
            impl_fillNewData();
        }
    }
}
void UIConfigElementWrapperBase::impl_fillNewData()
{
}
Reference< XIndexAccess > SAL_CALL UIConfigElementWrapperBase::getSettings( sal_Bool bWriteable ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );


    if ( bWriteable )
        return Reference< XIndexAccess >( static_cast< OWeakObject * >( new RootItemContainer( m_xConfigData ) ), UNO_QUERY );

    return m_xConfigData;
}

Reference< XFrame > SAL_CALL UIConfigElementWrapperBase::getFrame() throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    Reference< XFrame > xFrame( m_xWeakFrame );
    return xFrame;
}

OUString SAL_CALL UIConfigElementWrapperBase::getResourceURL() throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    return m_aResourceURL;
}

::sal_Int16 SAL_CALL UIConfigElementWrapperBase::getType() throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    return m_nType;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
