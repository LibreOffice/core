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

#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <o3tl/string_view.hxx>
#include <ucbhelper/macros.hxx>
#include <mutex>
#include <utility>

using namespace com::sun::star;
using namespace hierarchy_ucp;


// describe path of cfg entry
constexpr OUString CFGPROPERTY_NODEPATH = u"nodepath"_ustr;

constexpr OUString READ_SERVICE_NAME = u"com.sun.star.ucb.HierarchyDataReadAccess"_ustr;
constexpr OUString READWRITE_SERVICE_NAME = u"com.sun.star.ucb.HierarchyDataReadWriteAccess"_ustr;

constexpr OUString CONFIG_DATA_ROOT_KEY =  u"/org.openoffice.ucb.Hierarchy/Root"_ustr;


namespace hcp_impl
{


// HierarchyDataReadAccess Implementation.

namespace {

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
    std::mutex m_aMutex;
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
    HierarchyDataAccess( uno::Reference<
                                        uno::XInterface > xConfigAccess,
                         bool bReadOnly );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        noexcept override;
    virtual void SAL_CALL release()
        noexcept override;

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
private:
    template<class T>
    css::uno::Reference<T> ensureOrigInterface(css::uno::Reference<T>& x);
};

}

} // namespace hcp_impl

using namespace hcp_impl;


// HierarchyDataSource Implementation.


HierarchyDataSource::HierarchyDataSource(
        uno::Reference< uno::XComponentContext > xContext )
: m_xContext(std::move( xContext ))
{
}


// virtual
HierarchyDataSource::~HierarchyDataSource()
{
}

// XServiceInfo methods.
OUString SAL_CALL HierarchyDataSource::getImplementationName()                       \
{
    return u"com.sun.star.comp.ucb.HierarchyDataSource"_ustr;
}
sal_Bool SAL_CALL HierarchyDataSource::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}
css::uno::Sequence< OUString > HierarchyDataSource::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.DefaultHierarchyDataSource"_ustr, u"com.sun.star.ucb.HierarchyDataSource"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_HierarchyDataSource_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new HierarchyDataSource(context));
}


// XComponent methods.


// virtual
void SAL_CALL HierarchyDataSource::dispose()
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_aDisposeEventListeners.getLength(aGuard) )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_aDisposeEventListeners.disposeAndClear( aGuard, aEvt );
    }
}


// virtual
void SAL_CALL HierarchyDataSource::addEventListener(
                    const uno::Reference< lang::XEventListener > & Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.addInterface( aGuard, Listener );
}


// virtual
void SAL_CALL HierarchyDataSource::removeEventListener(
                    const uno::Reference< lang::XEventListener > & Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.removeInterface( aGuard, Listener );
}


// XMultiServiceFactory methods.


// virtual
uno::Reference< uno::XInterface > SAL_CALL
HierarchyDataSource::createInstance( const OUString & aServiceSpecifier )
{
    // Create view to root node.

    beans::PropertyValue aProp = comphelper::makePropertyValue(CFGPROPERTY_NODEPATH,
                                                               CONFIG_DATA_ROOT_KEY);

    uno::Sequence< uno::Any > aArguments{ uno::Any(aProp) };

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
    return { READ_SERVICE_NAME, READWRITE_SERVICE_NAME };
}


// Non-interface methods


uno::Reference< uno::XInterface >
HierarchyDataSource::createInstanceWithArguments(
                                std::u16string_view ServiceSpecifier,
                                const uno::Sequence< uno::Any > & Arguments,
                                bool bCheckArgs )
{
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
    auto aNewArgsRange = asNonConstRange(aNewArgs);

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
                        aNewArgsRange[ n ] <<= aProp;

                        break;
                    }
                    else
                    {
                        OSL_FAIL( "HierarchyDataSource::createInstanceWithArguments - "
                            "Invalid type for property 'nodepath'!" );
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
                                u"com.sun.star.configuration.ConfigurationAccess"_ustr,
                                aNewArgs );
        }
        else
        {
            // Create configuration read-write access object.
            xConfigAccess = xProv->createInstanceWithArguments(
                                u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr,
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

    return cppu::getXWeak(new HierarchyDataAccess(xConfigAccess, bReadOnly));
}


uno::Reference< lang::XMultiServiceFactory >
HierarchyDataSource::getConfigProvider()
{
    if ( !m_xConfigProvider.is() )
    {
        std::unique_lock aGuard( m_aMutex );
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
                std::u16string_view rInPath, OUString & rOutPath )
{
    if ( !rInPath.empty() )
    {
        if ( o3tl::starts_with( rInPath, u"/" ) )
        {
            OSL_FAIL( "HierarchyDataSource::createConfigPath - "
                        "Leading slash in node path!" );
            return false;
        }

        if ( o3tl::ends_with( rInPath, u"/" ) )
        {
            OSL_FAIL( "HierarchyDataSource::createConfigPath - "
                        "Trailing slash in node path!" );
            return false;
        }

        rOutPath = CONFIG_DATA_ROOT_KEY + "/" + rInPath;
    }
    else
    {
        rOutPath = CONFIG_DATA_ROOT_KEY;
    }

    return true;
}


// HierarchyDataAccess Implementation.

template<class T>
css::uno::Reference<T> HierarchyDataAccess::ensureOrigInterface(css::uno::Reference<T>& x)
{
    if ( x.is() )
        return x;
    std::scoped_lock aGuard( m_aMutex );
    if ( !x.is() )
       x.set( m_xConfigAccess, uno::UNO_QUERY );
    return x;
}


HierarchyDataAccess::HierarchyDataAccess( uno::Reference<
                                            uno::XInterface > xConfigAccess,
                                          bool bReadOnly )
: m_xConfigAccess(std::move( xConfigAccess )),
  m_bReadOnly( bReadOnly )
{
}

// XInterface methods.
void SAL_CALL HierarchyDataAccess::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL HierarchyDataAccess::release()
    noexcept
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
    return u"com.sun.star.comp.ucb.HierarchyDataAccess"_ustr;
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
        = ensureOrigInterface( m_xCfgC );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XComponent!" );
    xOrig->dispose();
}


// virtual
void SAL_CALL HierarchyDataAccess::addEventListener(
                    const uno::Reference< lang::XEventListener > & xListener )
{
    uno::Reference< lang::XComponent > xOrig
        = ensureOrigInterface( m_xCfgC );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XComponent!" );
    xOrig->addEventListener( xListener );
}


// virtual
void SAL_CALL HierarchyDataAccess::removeEventListener(
                    const uno::Reference< lang::XEventListener > & aListener )
{
    uno::Reference< lang::XComponent > xOrig
        = ensureOrigInterface( m_xCfgC );

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
        = ensureOrigInterface( m_xCfgHNA );

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
        = ensureOrigInterface( m_xCfgHNA );

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
        = ensureOrigInterface( m_xCfgNA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XNameAccess!" );
    return xOrig->getByName( aName );
}


// virtual
uno::Sequence< OUString > SAL_CALL HierarchyDataAccess::getElementNames()
{
    uno::Reference< container::XNameAccess > xOrig
        = ensureOrigInterface( m_xCfgNA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XNameAccess!" );
    return xOrig->getElementNames();
}


// virtual
sal_Bool SAL_CALL HierarchyDataAccess::hasByName( const OUString & aName )
{
    uno::Reference< container::XNameAccess > xOrig
        = ensureOrigInterface( m_xCfgNA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XNameAccess!" );
    return xOrig->hasByName( aName );
}


// XElementAccess methods.


// virtual
uno::Type SAL_CALL HierarchyDataAccess::getElementType()
{
    uno::Reference< container::XElementAccess > xOrig
        = ensureOrigInterface( m_xCfgEA );

    OSL_ENSURE( xOrig.is(),
                "HierarchyDataAccess : Data source is not an XElementAccess!" );
    return xOrig->getElementType();
}


// virtual
sal_Bool SAL_CALL HierarchyDataAccess::hasElements()
{
    uno::Reference< container::XElementAccess > xOrig
        = ensureOrigInterface( m_xCfgEA );

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
        = ensureOrigInterface( m_xCfgCN );

    OSL_ENSURE( xOrig.is(),
            "HierarchyDataAccess : Data source is not an XChangesNotifier!" );
    xOrig->addChangesListener( aListener );
}


// virtual
void SAL_CALL HierarchyDataAccess::removeChangesListener(
                const uno::Reference< util::XChangesListener > & aListener )
{
    uno::Reference< util::XChangesNotifier > xOrig
        = ensureOrigInterface( m_xCfgCN );

    OSL_ENSURE( xOrig.is(),
            "HierarchyDataAccess : Data source is not an XChangesNotifier!" );
    xOrig->removeChangesListener( aListener );
}


// XSingleServiceFactory methods.


// virtual
uno::Reference< uno::XInterface > SAL_CALL HierarchyDataAccess::createInstance()
{
    uno::Reference< lang::XSingleServiceFactory > xOrig
        = ensureOrigInterface( m_xCfgSSF );

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
        = ensureOrigInterface( m_xCfgSSF );

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
        = ensureOrigInterface( m_xCfgNC );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XNameContainer!" );
    xOrig->insertByName( aName, aElement );
}


// virtual
void SAL_CALL
HierarchyDataAccess::removeByName( const OUString & Name )
{
    uno::Reference< container::XNameContainer > xOrig
        = ensureOrigInterface( m_xCfgNC );

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
        = ensureOrigInterface( m_xCfgNR );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XNameReplace!" );
    xOrig->replaceByName( aName, aElement );
}


// XChangesBatch methods.


// virtual
void SAL_CALL HierarchyDataAccess::commitChanges()
{
    uno::Reference< util::XChangesBatch > xOrig
        = ensureOrigInterface( m_xCfgCB );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XChangesBatch!" );
    xOrig->commitChanges();
}


// virtual
sal_Bool SAL_CALL HierarchyDataAccess::hasPendingChanges()
{
    uno::Reference< util::XChangesBatch > xOrig
        = ensureOrigInterface( m_xCfgCB );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XChangesBatch!" );
    return xOrig->hasPendingChanges();
}


// virtual
uno::Sequence< util::ElementChange > SAL_CALL
HierarchyDataAccess::getPendingChanges()
{
    uno::Reference< util::XChangesBatch > xOrig
        = ensureOrigInterface( m_xCfgCB );

    OSL_ENSURE( xOrig.is(),
        "HierarchyDataAccess : Data source is not an XChangesBatch!" );
    return xOrig->getPendingChanges();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
