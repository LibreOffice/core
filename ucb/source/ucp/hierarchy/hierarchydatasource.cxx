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


/**************************************************************************
                                TODO
 **************************************************************************

 Note: Configuration Management classes do not support XAggregation.
       So I have to wrap the interesting interfaces manually.

 *************************************************************************/
#include "hierarchydatasource.hxx"
#include <osl/diagnose.h>

#include <comphelper/processfactory.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

using namespace com::sun::star;
using namespace hierarchy_ucp;


// describe path of cfg entry
#define CFGPROPERTY_NODEPATH    "nodepath"
// true->async. update; false->sync. update
#define CFGPROPERTY_LAZYWRITE   "lazywrite"

#define READ_SERVICE_NAME      "com.sun.star.ucb.HierarchyDataReadAccess"
#define READWRITE_SERVICE_NAME "com.sun.star.ucb.HierarchyDataReadWriteAccess"

#define CONFIG_DATA_ROOT_KEY          \
                        "/org.openoffice.ucb.Hierarchy/Root"


namespace hcp_impl
{


// HierarchyDataReadAccess Implementation.


class HierarchyDataAccess : public cppu::OWeakObject,
                            public lang::XServiceInfo,
                            public lang::XTypeProvider,
                            public lang::XComponent,
                            public lang::XSingleServiceFactory,
                            public container::XHierarchicalNameAccess,
                            public container::XNameContainer,
                            public util::XChangesNotifier,
                            public util::XChangesBatch
{
    osl::Mutex m_aMutex;
    uno::Reference< uno::XInterface > m_xConfigAccess;
    uno::Reference< lang::XComponent >                   m_xCfgC;
    uno::Reference< lang::XSingleServiceFactory >        m_xCfgSSF;
    uno::Reference< container::XHierarchicalNameAccess > m_xCfgHNA;
    uno::Reference< container::XNameContainer >          m_xCfgNC;
    uno::Reference< container::XNameReplace >            m_xCfgNR;
    uno::Reference< container::XNameAccess >             m_xCfgNA;
    uno::Reference< container::XElementAccess >          m_xCfgEA;
    uno::Reference< util::XChangesNotifier >             m_xCfgCN;
    uno::Reference< util::XChangesBatch >                m_xCfgCB;
    bool m_bReadOnly;

public:
    HierarchyDataAccess( const uno::Reference<
                                        uno::XInterface > & xConfigAccess,
                         bool bReadOnly );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XComponent
    virtual void SAL_CALL
    dispose() override;
    virtual void SAL_CALL
    addEventListener( const uno::Reference< lang::XEventListener > & xListener ) override;
    virtual void SAL_CALL
    removeEventListener( const uno::Reference<
                            lang::XEventListener > & aListener ) override;

    // XSingleServiceFactory
    virtual uno::Reference< uno::XInterface > SAL_CALL
    createInstance() override;
    virtual uno::Reference< uno::XInterface > SAL_CALL
    createInstanceWithArguments( const uno::Sequence< uno::Any > & aArguments ) override;

    // XHierarchicalNameAccess
    virtual uno::Any SAL_CALL
    getByHierarchicalName( const OUString & aName ) override;
    virtual sal_Bool SAL_CALL
    hasByHierarchicalName( const OUString & aName ) override;

    // XNameContainer
    virtual void SAL_CALL
    insertByName( const OUString & aName, const uno::Any & aElement ) override;
    virtual void SAL_CALL
    removeByName( const OUString & Name ) override;

    // XNameReplace ( base of XNameContainer )
    virtual void SAL_CALL
    replaceByName( const OUString & aName, const uno::Any & aElement ) override;

    // XNameAccess ( base of XNameReplace )
    virtual uno::Any SAL_CALL
    getByName( const OUString & aName ) override;
    virtual uno::Sequence< OUString > SAL_CALL
    getElementNames() override;
    virtual sal_Bool SAL_CALL
    hasByName( const OUString & aName ) override;

    // XElementAccess ( base of XNameAccess )
    virtual uno::Type SAL_CALL
    getElementType() override;
    virtual sal_Bool SAL_CALL
    hasElements() override;

    // XChangesNotifier
    virtual void SAL_CALL
    addChangesListener( const uno::Reference<
                            util::XChangesListener > & aListener ) override;
    virtual void SAL_CALL
    removeChangesListener( const uno::Reference<
                            util::XChangesListener > & aListener ) override;

    // XChangesBatch
    virtual void SAL_CALL
    commitChanges() override;
    virtual sal_Bool SAL_CALL
    hasPendingChanges() override;
    virtual uno::Sequence< util::ElementChange > SAL_CALL
    getPendingChanges() override;
};

} // namespace hcp_impl

using namespace hcp_impl;


// HierarchyDataSource Implementation.


HierarchyDataSource::HierarchyDataSource(
        const uno::Reference< uno::XComponentContext > & rxContext )
: m_xContext( rxContext ),
  m_pDisposeEventListeners( nullptr )
{
}


// virtual
HierarchyDataSource::~HierarchyDataSource()
{
}


// XInterface methods.
void SAL_CALL HierarchyDataSource::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL HierarchyDataSource::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL HierarchyDataSource::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< lang::XServiceInfo* >(this)),
                                               (static_cast< lang::XComponent* >(this)),
                                               (static_cast< lang::XMultiServiceFactory* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_4( HierarchyDataSource,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      lang::XComponent,
                      lang::XMultiServiceFactory );


// XServiceInfo methods.

XSERVICEINFO_COMMOM_IMPL( HierarchyDataSource,
                          OUString( "com.sun.star.comp.ucb.HierarchyDataSource" ) )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
HierarchyDataSource_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX =
        static_cast<css::lang::XServiceInfo*>(new HierarchyDataSource( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString >
HierarchyDataSource::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 2 );
    aSNS[ 0 ] = "com.sun.star.ucb.DefaultHierarchyDataSource";
    aSNS[ 1 ] = "com.sun.star.ucb.HierarchyDataSource";
    return aSNS;
}

ONE_INSTANCE_SERVICE_FACTORY_IMPL( HierarchyDataSource );


// XComponent methods.


// virtual
void SAL_CALL HierarchyDataSource::dispose()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }
}


// virtual
void SAL_CALL HierarchyDataSource::addEventListener(
                    const uno::Reference< lang::XEventListener > & Listener )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners.reset(
            new comphelper::OInterfaceContainerHelper2( m_aMutex ) );

    m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL HierarchyDataSource::removeEventListener(
                    const uno::Reference< lang::XEventListener > & Listener )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


// XMultiServiceFactory methods.


// virtual
uno::Reference< uno::XInterface > SAL_CALL
HierarchyDataSource::createInstance( const OUString & aServiceSpecifier )
{
    // Create view to root node.

    beans::PropertyValue aProp;
    aProp.Name = CFGPROPERTY_NODEPATH;
    aProp.Value <<= OUString( CONFIG_DATA_ROOT_KEY  );

    uno::Sequence< uno::Any > aArguments( 1 );
    aArguments[ 0 ] <<= aProp;

    return createInstanceWithArguments( aServiceSpecifier, aArguments, false );
}


// virtual
uno::Reference< uno::XInterface > SAL_CALL
HierarchyDataSource::createInstanceWithArguments(
                                const OUString & ServiceSpecifier,
                                const uno::Sequence< uno::Any > & Arguments )
{
    return createInstanceWithArguments( ServiceSpecifier, Arguments, true );
}


// virtual
uno::Sequence< OUString > SAL_CALL
HierarchyDataSource::getAvailableServiceNames()
{
    uno::Sequence< OUString > aNames( 2 );
    aNames[ 0 ] = READ_SERVICE_NAME;
    aNames[ 1 ] = READWRITE_SERVICE_NAME;
    return aNames;
}


// Non-interface methods


uno::Reference< uno::XInterface > SAL_CALL
HierarchyDataSource::createInstanceWithArguments(
                                const OUString & ServiceSpecifier,
                                const uno::Sequence< uno::Any > & Arguments,
                                bool bCheckArgs )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Check service specifier.
    bool bReadOnly  = ServiceSpecifier == READ_SERVICE_NAME;
    bool bReadWrite = !bReadOnly && ServiceSpecifier == READWRITE_SERVICE_NAME;

    if ( !bReadOnly && !bReadWrite )
    {
        OSL_FAIL( "HierarchyDataSource::createInstanceWithArguments - "
                    "Unsupported service specifier!" );
        return uno::Reference< uno::XInterface >();
    }

    uno::Sequence< uno::Any > aNewArgs( Arguments );

    bool bHasLazyWriteProp = bReadOnly; // property must be added only if
                                        // a writable view is requested.
    if ( bCheckArgs )
    {
        // Check arguments.
        bool bHasNodePath = false;
        sal_Int32 nCount = Arguments.getLength();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            beans::PropertyValue aProp;
            if ( Arguments[ n ] >>= aProp )
            {
                if ( aProp.Name == CFGPROPERTY_NODEPATH )
                {
                    OUString aPath;
                    if ( aProp.Value >>= aPath )
                    {
                        bHasNodePath = true;

                        // Create path to data inside the configuration.
                        OUString aConfigPath;
                        if ( !createConfigPath( aPath, aConfigPath ) )
                        {
                            OSL_FAIL( "HierarchyDataSource::"
                                "createInstanceWithArguments - "
                                "Invalid node path!" );
                            return uno::Reference< uno::XInterface >();
                        }

                        aProp.Value <<= aConfigPath;

                        // Set new path in arguments.
                        aNewArgs[ n ] <<= aProp;

                        if ( bHasLazyWriteProp )
                            break;
                    }
                    else
                    {
                        OSL_FAIL( "HierarchyDataSource::createInstanceWithArguments - "
                            "Invalid type for property 'nodepath'!" );
                        return uno::Reference< uno::XInterface >();
                    }
                }
                else if ( aProp.Name == CFGPROPERTY_LAZYWRITE )
                {
                    if ( aProp.Value.getValueType() == cppu::UnoType<bool>::get() )
                    {
                        bHasLazyWriteProp = true;

                        if ( bHasNodePath )
                            break;
                    }
                    else
                    {
                        OSL_FAIL( "HierarchyDataSource::createInstanceWithArguments - "
                            "Invalid type for property 'lazywrite'!" );
                        return uno::Reference< uno::XInterface >();
                    }
                }
            }
        }

        if ( !bHasNodePath )
        {
            OSL_FAIL( "HierarchyDataSource::createInstanceWithArguments - "
                        "No 'nodepath' property!" );
            return uno::Reference< uno::XInterface >();
        }
    }

    // Create Configuration Provider.
    uno::Reference< lang::XMultiServiceFactory > xProv = getConfigProvider();
    if ( !xProv.is() )
        return uno::Reference< uno::XInterface >();

    uno::Reference< uno::XInterface > xConfigAccess;
    try
    {
        if ( bReadOnly )
        {
            // Create configuration read-only access object.
            xConfigAccess = xProv->createInstanceWithArguments(
                                "com.sun.star.configuration.ConfigurationAccess",
                                aNewArgs );
        }
        else
        {
            // Append 'lazywrite' property value, if not already present.
            if ( !bHasLazyWriteProp )
            {
                sal_Int32 nLen = aNewArgs.getLength();
                aNewArgs.realloc( nLen + 1 );

                beans::PropertyValue aProp;
                aProp.Name = CFGPROPERTY_LAZYWRITE;
                aProp.Value <<= true;
                aNewArgs[ nLen ] <<= aProp;
            }

            // Create configuration read-write access object.
            xConfigAccess = xProv->createInstanceWithArguments(
                                "com.sun.star.configuration.ConfigurationUpdateAccess",
                                aNewArgs );
        }
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "HierarchyDataSource::createInstanceWithArguments - "
                    "Cannot instantiate configuration access!" );
        throw;
    }

    if ( !xConfigAccess.is() )
    {
        OSL_FAIL( "HierarchyDataSource::createInstanceWithArguments - "
                    "Cannot instantiate configuration access!" );
        return xConfigAccess;
    }

    return uno::Reference< uno::XInterface >(
                static_cast< cppu::OWeakObject * >(
                    new HierarchyDataAccess( xConfigAccess, bReadOnly ) ) );
}


uno::Reference< lang::XMultiServiceFactory >
HierarchyDataSource::getConfigProvider()
{
    if ( !m_xConfigProvider.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if ( !m_xConfigProvider.is() )
        {
            try
            {
                m_xConfigProvider = configuration::theDefaultProvider::get( m_xContext );
            }
            catch ( uno::Exception const & )
            {
                OSL_FAIL( "HierarchyDataSource::getConfigProvider - "
                               "caught exception!" );
            }
        }
    }

    return m_xConfigProvider;
}


bool HierarchyDataSource::createConfigPath(
                const OUString & rInPath, OUString & rOutPath )
{
    if ( !rInPath.isEmpty() )
    {
        if ( rInPath.startsWith( "/" ) )
        {
            OSL_FAIL( "HierarchyDataSource::createConfigPath - "
                        "Leading slash in node path!" );
            return false;
        }

        if ( rInPath.endsWith( "/" ) )
        {
            OSL_FAIL( "HierarchyDataSource::createConfigPath - "
                        "Trailing slash in node path!" );
            return false;
        }

        rOutPath = CONFIG_DATA_ROOT_KEY "/" + rInPath;
    }
    else
    {
        rOutPath = CONFIG_DATA_ROOT_KEY;
    }

    return true;
}


// HierarchyDataAccess Implementation.


#define ENSURE_ORIG_INTERFACE( interface_name, member_name )    \
    m_xCfg##member_name;                                        \
    if ( !m_xCfg##member_name.is() )                            \
    {                                                           \
        osl::Guard< osl::Mutex > aGuard( m_aMutex );            \
        if ( !m_xCfg##member_name.is() )                        \
            m_xCfg##member_name.set( m_xConfigAccess, uno::UNO_QUERY ); \
        xOrig = m_xCfg##member_name;                            \
    }


HierarchyDataAccess::HierarchyDataAccess( const uno::Reference<
                                            uno::XInterface > & xConfigAccess,
                                          bool bReadOnly )
: m_xConfigAccess( xConfigAccess ),
  m_bReadOnly( bReadOnly )
{
}

// XInterface methods.
void SAL_CALL HierarchyDataAccess::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL HierarchyDataAccess::release()
    throw()
{
    OWeakObject::release();
}

// virtual
uno::Any SAL_CALL HierarchyDataAccess::queryInterface( const uno::Type & aType )
{
    // Interfaces supported in read-only and read-write mode.
    uno::Any aRet = cppu::queryInterface( aType,
                static_cast< lang::XTypeProvider * >( this ),
                static_cast< lang::XServiceInfo * >( this ),
                static_cast< lang::XComponent * >( this ),
                static_cast< container::XHierarchicalNameAccess * >( this ),
                static_cast< container::XNameAccess * >( this ),
                static_cast< container::XElementAccess * >( this ),
                static_cast< util::XChangesNotifier * >( this ) );

    // Interfaces supported only in read-write mode.
    if ( !aRet.hasValue() && !m_bReadOnly )
    {
        aRet = cppu::queryInterface( aType,
                static_cast< lang::XSingleServiceFactory * >( this ),
                static_cast< container::XNameContainer * >( this ),
                static_cast< container::XNameReplace * >( this ),
                static_cast< util::XChangesBatch * >( this ) );
    }

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( aType );
}


// XTypeProvider methods.


XTYPEPROVIDER_COMMON_IMPL( HierarchyDataAccess );


// virtual
uno::Sequence< uno::Type > SAL_CALL HierarchyDataAccess::getTypes()
{
    if ( m_bReadOnly )
    {
        static cppu::OTypeCollection s_aReadOnlyTypes(
                    CPPU_TYPE_REF( lang::XTypeProvider ),
                    CPPU_TYPE_REF( lang::XServiceInfo ),
                    CPPU_TYPE_REF( lang::XComponent ),
                    CPPU_TYPE_REF( container::XHierarchicalNameAccess ),
                    CPPU_TYPE_REF( container::XNameAccess ),
                    CPPU_TYPE_REF( util::XChangesNotifier ) );

        return s_aReadOnlyTypes.getTypes();
    }
    else
    {
        static cppu::OTypeCollection s_aReadWriteTypes(
                    CPPU_TYPE_REF( lang::XTypeProvider ),
                    CPPU_TYPE_REF( lang::XServiceInfo ),
                    CPPU_TYPE_REF( lang::XComponent ),
                    CPPU_TYPE_REF( lang::XSingleServiceFactory ),
                    CPPU_TYPE_REF( container::XHierarchicalNameAccess ),
                    CPPU_TYPE_REF( container::XNameContainer ),
                    CPPU_TYPE_REF( util::XChangesBatch ),
                    CPPU_TYPE_REF( util::XChangesNotifier ) );

        return s_aReadWriteTypes.getTypes();
    }
}


// XServiceInfo methods.

OUString SAL_CALL HierarchyDataAccess::getImplementationName()
{
    return OUString("com.sun.star.comp.ucb.HierarchyDataAccess");
}

sal_Bool SAL_CALL HierarchyDataAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL HierarchyDataAccess::getSupportedServiceNames()
{
    return { READ_SERVICE_NAME, READWRITE_SERVICE_NAME };
}


// XComponent methods.


// virtual
void SAL_CALL HierarchyDataAccess::dispose()
{
    uno::Reference< lang::XComponent > xOrig
        = ENSURE_ORIG_INTERFACE( lang::XComponent, C );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XComponent!" );
    xOrig->dispose();
}


// virtual
void SAL_CALL HierarchyDataAccess::addEventListener(
                    const uno::Reference< lang::XEventListener > & xListener )
{
    uno::Reference< lang::XComponent > xOrig
        = ENSURE_ORIG_INTERFACE( lang::XComponent, C );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XComponent!" );
    xOrig->addEventListener( xListener );
}


// virtual
void SAL_CALL HierarchyDataAccess::removeEventListener(
                    const uno::Reference< lang::XEventListener > & aListener )
{
    uno::Reference< lang::XComponent > xOrig
        = ENSURE_ORIG_INTERFACE( lang::XComponent, C );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XComponent!" );
    xOrig->removeEventListener( aListener );
}


// XHierarchicalNameAccess methods.


// virtual
uno::Any SAL_CALL HierarchyDataAccess::getByHierarchicalName(
                                                const OUString & aName )
{
    uno::Reference< container::XHierarchicalNameAccess > xOrig
        = ENSURE_ORIG_INTERFACE( container::XHierarchicalNameAccess, HNA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : "
                "Data source is not an XHierarchicalNameAccess!" );
    return xOrig->getByHierarchicalName( aName );
}


// virtual
sal_Bool SAL_CALL HierarchyDataAccess::hasByHierarchicalName(
                                                const OUString & aName )
{
    uno::Reference< container::XHierarchicalNameAccess > xOrig
        = ENSURE_ORIG_INTERFACE( container::XHierarchicalNameAccess, HNA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : "
                "Data source is not an XHierarchicalNameAccess!" );
    return xOrig->hasByHierarchicalName( aName );
}


// XNameAccess methods.


// virtual
uno::Any SAL_CALL HierarchyDataAccess::getByName( const OUString & aName )
{
    uno::Reference< container::XNameAccess > xOrig
        = ENSURE_ORIG_INTERFACE( container::XNameAccess, NA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XNameAccess!" );
    return xOrig->getByName( aName );
}


// virtual
uno::Sequence< OUString > SAL_CALL HierarchyDataAccess::getElementNames()
{
    uno::Reference< container::XNameAccess > xOrig
        = ENSURE_ORIG_INTERFACE( container::XNameAccess, NA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XNameAccess!" );
    return xOrig->getElementNames();
}


// virtual
sal_Bool SAL_CALL HierarchyDataAccess::hasByName( const OUString & aName )
{
    uno::Reference< container::XNameAccess > xOrig
        = ENSURE_ORIG_INTERFACE( container::XNameAccess, NA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XNameAccess!" );
    return xOrig->hasByName( aName );
}


// XElementAccess methods.


// virtual
uno::Type SAL_CALL HierarchyDataAccess::getElementType()
{
    uno::Reference< container::XElementAccess > xOrig
        = ENSURE_ORIG_INTERFACE( container::XElementAccess, EA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XElementAccess!" );
    return xOrig->getElementType();
}


// virtual
sal_Bool SAL_CALL HierarchyDataAccess::hasElements()
{
    uno::Reference< container::XElementAccess > xOrig
        = ENSURE_ORIG_INTERFACE( container::XElementAccess, EA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XElementAccess!" );
    return xOrig->hasElements();
}


// XChangesNotifier methods.


// virtual
void SAL_CALL HierarchyDataAccess::addChangesListener(
                const uno::Reference< util::XChangesListener > & aListener )
{
    uno::Reference< util::XChangesNotifier > xOrig
        = ENSURE_ORIG_INTERFACE( util::XChangesNotifier, CN );

    OSL_ENSURE( xOrig.is(),
            "HierarchyDataAccess : Data source is not an XChangesNotifier!" );
    xOrig->addChangesListener( aListener );
}


// virtual
void SAL_CALL HierarchyDataAccess::removeChangesListener(
                const uno::Reference< util::XChangesListener > & aListener )
{
    uno::Reference< util::XChangesNotifier > xOrig
        = ENSURE_ORIG_INTERFACE( util::XChangesNotifier, CN );

    OSL_ENSURE( xOrig.is(),
            "HierarchyDataAccess : Data source is not an XChangesNotifier!" );
    xOrig->removeChangesListener( aListener );
}


// XSingleServiceFactory methods.


// virtual
uno::Reference< uno::XInterface > SAL_CALL HierarchyDataAccess::createInstance()
{
    uno::Reference< lang::XSingleServiceFactory > xOrig
        = ENSURE_ORIG_INTERFACE( lang::XSingleServiceFactory, SSF );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XSingleServiceFactory!" );
    return xOrig->createInstance();
}


// virtual
uno::Reference< uno::XInterface > SAL_CALL
HierarchyDataAccess::createInstanceWithArguments(
                            const uno::Sequence< uno::Any > & aArguments )
{
    uno::Reference< lang::XSingleServiceFactory > xOrig
        = ENSURE_ORIG_INTERFACE( lang::XSingleServiceFactory, SSF );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XSingleServiceFactory!" );
    return xOrig->createInstanceWithArguments( aArguments );
}


// XNameContainer methods.


// virtual
void SAL_CALL
HierarchyDataAccess::insertByName( const OUString & aName,
                                   const uno::Any & aElement )
{
    uno::Reference< container::XNameContainer > xOrig
        = ENSURE_ORIG_INTERFACE( container::XNameContainer, NC );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XNameContainer!" );
    xOrig->insertByName( aName, aElement );
}


// virtual
void SAL_CALL
HierarchyDataAccess::removeByName( const OUString & Name )
{
    uno::Reference< container::XNameContainer > xOrig
        = ENSURE_ORIG_INTERFACE( container::XNameContainer, NC );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XNameContainer!" );
    xOrig->removeByName( Name );
}


// XNameReplace methods.


// virtual
void SAL_CALL HierarchyDataAccess::replaceByName( const OUString & aName,
                                                  const uno::Any & aElement )
{
    uno::Reference< container::XNameReplace > xOrig
        = ENSURE_ORIG_INTERFACE( container::XNameReplace, NR );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XNameReplace!" );
    xOrig->replaceByName( aName, aElement );
}


// XChangesBatch methods.


// virtual
void SAL_CALL HierarchyDataAccess::commitChanges()
{
    uno::Reference< util::XChangesBatch > xOrig
        = ENSURE_ORIG_INTERFACE( util::XChangesBatch, CB );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XChangesBatch!" );
    xOrig->commitChanges();
}


// virtual
sal_Bool SAL_CALL HierarchyDataAccess::hasPendingChanges()
{
    uno::Reference< util::XChangesBatch > xOrig
        = ENSURE_ORIG_INTERFACE( util::XChangesBatch, CB );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XChangesBatch!" );
    return xOrig->hasPendingChanges();
}


// virtual
uno::Sequence< util::ElementChange > SAL_CALL
HierarchyDataAccess::getPendingChanges()
{
    uno::Reference< util::XChangesBatch > xOrig
        = ENSURE_ORIG_INTERFACE( util::XChangesBatch, CB );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XChangesBatch!" );
    return xOrig->getPendingChanges();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
