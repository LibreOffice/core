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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <uno/mapping.hxx>
#include <uno/dispatcher.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XUnloadingPreference.hpp>

#include <unordered_map>
#include <unordered_set>

using namespace com::sun::star;
using namespace css::uno;
using namespace css::beans;
using namespace css::registry;
using namespace css::lang;
using namespace css::container;
using namespace cppu;
using namespace osl;
using namespace std;

namespace {

Sequence< OUString > retrieveAsciiValueList(
    const Reference< XSimpleRegistry > &xReg, const OUString &keyName )
{
    Reference< XEnumerationAccess > xAccess( xReg, UNO_QUERY );
    Sequence< OUString > seq;
    if( xAccess.is() )
    {
        Reference< XEnumeration > xEnum = xAccess->createEnumeration();
        while( xEnum.is() && xEnum->hasMoreElements() )
        {
            Reference< XSimpleRegistry > xTempReg;
            xEnum->nextElement() >>= xTempReg;
            if( xTempReg.is() )
            {
                Sequence< OUString > seq2 = retrieveAsciiValueList( xTempReg, keyName );

                if( seq2.getLength() )
                {
                    sal_Int32 n1Len = seq.getLength();
                    sal_Int32 n2Len = seq2.getLength();

                    seq.realloc( n1Len + n2Len );
                    const OUString *pSource = seq2.getConstArray();
                    OUString *pTarget = seq.getArray();
                    for( int i = 0 ; i < n2Len ; i ++ )
                    {
                        pTarget[i+n1Len] = pSource[i];
                    }
                }
            }
        }
    }
    else if( xReg.is () )
    {
        try
        {
            Reference< XRegistryKey > rRootKey = xReg->getRootKey();
            if( rRootKey.is() )
            {
                Reference<XRegistryKey > xKey = rRootKey->openKey(keyName);
                if( xKey.is() )
                {
                    seq = xKey->getAsciiListValue();
                }
            }
        }
        catch( InvalidRegistryException & )
        {
        }
        catch (InvalidValueException &)
        {
        }
    }
    return seq;
}

/*****************************************************************************
    Enumeration by ServiceName
*****************************************************************************/
struct hashRef_Impl
{
    size_t operator()(const Reference<XInterface > & rName) const
    {
        // query to XInterface. The cast to XInterface* must be the same for the same object
        Reference<XInterface > x( Reference<XInterface >::query( rName ) );
        return reinterpret_cast<size_t>(x.get());
    }
};

struct equaltoRef_Impl
{
    bool operator()(const Reference<XInterface > & rName1, const Reference<XInterface > & rName2 ) const
        { return rName1 == rName2; }
};

typedef std::unordered_set
<
    Reference<XInterface >,
    hashRef_Impl,
    equaltoRef_Impl
> HashSet_Ref;


class ServiceEnumeration_Impl : public WeakImplHelper< XEnumeration >
{
public:
    explicit ServiceEnumeration_Impl( const Sequence< Reference<XInterface > > & rFactories )
        : aFactories( rFactories )
        , nIt( 0 )
        {}

    // XEnumeration
    sal_Bool SAL_CALL hasMoreElements() override;
    Any SAL_CALL nextElement() override;
private:
    Mutex                               aMutex;
    Sequence< Reference<XInterface > >  aFactories;
    sal_Int32                           nIt;
};

// XEnumeration
sal_Bool ServiceEnumeration_Impl::hasMoreElements()
{
    MutexGuard aGuard( aMutex );
    return nIt != aFactories.getLength();
}

// XEnumeration
Any ServiceEnumeration_Impl::nextElement()
{
    MutexGuard aGuard( aMutex );
    if( nIt == aFactories.getLength() )
        throw NoSuchElementException("no more elements");

    return Any( &aFactories.getConstArray()[nIt++], cppu::UnoType<XInterface>::get());
}


class PropertySetInfo_Impl : public WeakImplHelper< beans::XPropertySetInfo >
{
    Sequence< beans::Property > const m_properties;

public:
    explicit PropertySetInfo_Impl( Sequence< beans::Property > const & properties )
        : m_properties( properties )
        {}

    // XPropertySetInfo impl
    virtual Sequence< beans::Property > SAL_CALL getProperties() override;
    virtual beans::Property SAL_CALL getPropertyByName( OUString const & name ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( OUString const & name ) override;
};

Sequence< beans::Property > PropertySetInfo_Impl::getProperties()
{
    return m_properties;
}

beans::Property PropertySetInfo_Impl::getPropertyByName( OUString const & name )
{
    beans::Property const * p = m_properties.getConstArray();
    for ( sal_Int32 nPos = m_properties.getLength(); nPos--; )
    {
        if (p[ nPos ].Name == name)
            return p[ nPos ];
    }
    throw beans::UnknownPropertyException(
        "unknown property: " + name );
}

sal_Bool PropertySetInfo_Impl::hasPropertyByName( OUString const & name )
{
    beans::Property const * p = m_properties.getConstArray();
    for ( sal_Int32 nPos = m_properties.getLength(); nPos--; )
    {
        if (p[ nPos ].Name == name)
            return true;
    }
    return false;
}


/*****************************************************************************
    Enumeration by implementation
*****************************************************************************/
class ImplementationEnumeration_Impl : public WeakImplHelper< XEnumeration >
{
public:
    explicit ImplementationEnumeration_Impl( const HashSet_Ref & rImplementationMap )
        : aImplementationMap( rImplementationMap )
        , aIt( aImplementationMap.begin() )
        {}

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual Any SAL_CALL nextElement() override;

private:
    Mutex                           aMutex;
    HashSet_Ref                     aImplementationMap;
    HashSet_Ref::iterator           aIt;
};

// XEnumeration
sal_Bool ImplementationEnumeration_Impl::hasMoreElements()
{
    MutexGuard aGuard( aMutex );
    return aIt != aImplementationMap.end();
}

// XEnumeration
Any ImplementationEnumeration_Impl::nextElement()
{
    MutexGuard aGuard( aMutex );
    if( aIt == aImplementationMap.end() )
        throw NoSuchElementException("no more elements");

    Any ret( &(*aIt), cppu::UnoType<XInterface>::get());
    ++aIt;
    return ret;
}

/*****************************************************************************
    Hash tables
*****************************************************************************/
typedef std::unordered_set
<
    OUString
> HashSet_OWString;

typedef std::unordered_multimap
<
    OUString,
    Reference<XInterface >
> HashMultimap_OWString_Interface;

typedef std::unordered_map
<
    OUString,
    Reference<XInterface >
> HashMap_OWString_Interface;

/*****************************************************************************
    class OServiceManager_Listener
*****************************************************************************/
class OServiceManager_Listener : public WeakImplHelper< XEventListener >
{
private:
    WeakReference<XSet > xSMgr;

public:
    explicit OServiceManager_Listener( const Reference<XSet > & rSMgr )
        : xSMgr( rSMgr )
        {}

    // XEventListener
    virtual void SAL_CALL disposing(const EventObject & rEvt ) override;
};

void OServiceManager_Listener::disposing(const EventObject & rEvt )
{
    Reference<XSet > x( xSMgr );
    if( !x.is() )
        return;

    try
    {
        x->remove( Any( &rEvt.Source, cppu::UnoType<XInterface>::get()) );
    }
    catch( const IllegalArgumentException & )
    {
        OSL_FAIL( "IllegalArgumentException caught" );
    }
    catch( const NoSuchElementException & )
    {
        OSL_FAIL( "NoSuchElementException caught" );
    }
}


/*****************************************************************************
    class OServiceManager
*****************************************************************************/
struct OServiceManagerMutex
{
    Mutex m_mutex;
};

typedef WeakComponentImplHelper<
    lang::XMultiServiceFactory, lang::XMultiComponentFactory, lang::XServiceInfo,
    lang::XInitialization,
    container::XSet, container::XContentEnumerationAccess,
    beans::XPropertySet > t_OServiceManager_impl;

class OServiceManager
    : public OServiceManagerMutex
    , public t_OServiceManager_impl
{
public:
    explicit OServiceManager( Reference< XComponentContext > const & xContext );

    // XInitialization
    void SAL_CALL initialize( Sequence< Any > const & args ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XMultiComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        OUString const & rServiceSpecifier, Reference< XComponentContext > const & xContext ) override;
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        OUString const & rServiceSpecifier,
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext ) override;
//      virtual Sequence< OUString > SAL_CALL getAvailableServiceNames()
//          throw (RuntimeException);

    // XMultiServiceFactory
    virtual Sequence< OUString > SAL_CALL getAvailableServiceNames() override;
    virtual Reference<XInterface > SAL_CALL createInstance(const OUString &) override;
    virtual Reference<XInterface > SAL_CALL createInstanceWithArguments(const OUString &, const Sequence<Any >& Arguments) override;

    // The same as the getAvailableServiceNames, but only unique names
    Sequence< OUString > getUniqueAvailableServiceNames(
        HashSet_OWString & aNameSet );

    // XElementAccess
    virtual Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    virtual Reference<XEnumeration > SAL_CALL createEnumeration() override;

    // XSet
    virtual sal_Bool SAL_CALL has( const Any & Element ) override;
    virtual void SAL_CALL insert( const Any & Element ) override;
    virtual void SAL_CALL remove( const Any & Element ) override;

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    virtual Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XPropertySet
    Reference<XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    void SAL_CALL setPropertyValue(const OUString& PropertyName, const Any& aValue) override;
    Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    void SAL_CALL addPropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener) override;
    void SAL_CALL removePropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener) override;
    void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener) override;
    void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener) override;

protected:
    bool is_disposed() const;
    void check_undisposed() const;
    virtual void SAL_CALL disposing() override;

    bool haveFactoryWithThisImplementation(const OUString& aImplName);

    virtual Sequence< Reference< XInterface > > queryServiceFactories(
        const OUString& aServiceName, Reference< XComponentContext > const & xContext );

    Reference< XComponentContext >  m_xContext;

    Reference< beans::XPropertySetInfo > m_xPropertyInfo;

    // factories which have been loaded and not inserted( by XSet::insert)
    // are remembered by this set.
    HashSet_Ref m_SetLoadedFactories;
private:

    Reference<XEventListener >      getFactoryListener();


    HashMultimap_OWString_Interface m_ServiceMap;
    HashSet_Ref                     m_ImplementationMap;
    HashMap_OWString_Interface      m_ImplementationNameMap;
    Reference<XEventListener >      xFactoryListener;
    bool                            m_bInDisposing;
};


bool OServiceManager::is_disposed() const
{
    // ought to be guarded by m_mutex:
    return (m_bInDisposing || rBHelper.bDisposed);
}


void OServiceManager::check_undisposed() const
{
    if (is_disposed())
    {
        throw lang::DisposedException(
            "service manager instance has already been disposed!",
            static_cast<OWeakObject *>(const_cast<OServiceManager *>(this)) );
    }
}


typedef WeakComponentImplHelper<
    lang::XMultiServiceFactory, lang::XMultiComponentFactory, lang::XServiceInfo,
    container::XSet, container::XContentEnumerationAccess,
    beans::XPropertySet > t_OServiceManagerWrapper_impl;

class OServiceManagerWrapper : public OServiceManagerMutex, public t_OServiceManagerWrapper_impl
{
    Reference< XComponentContext > m_xContext;
    Reference< XMultiComponentFactory > m_root;
    Reference< XMultiComponentFactory > const & getRoot() const
    {
        if (! m_root.is())
        {
            throw lang::DisposedException(
                "service manager instance has already been disposed!" );
        }
        return m_root;
    }

protected:
    virtual void SAL_CALL disposing() override;

public:
    explicit OServiceManagerWrapper(
        Reference< XComponentContext > const & xContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
        { return Reference< XServiceInfo >(getRoot(), UNO_QUERY_THROW)->getImplementationName(); }
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override
        { return Reference< XServiceInfo >(getRoot(), UNO_QUERY_THROW)->supportsService( ServiceName ); }
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override
        { return Reference< XServiceInfo >(getRoot(), UNO_QUERY_THROW)->getSupportedServiceNames(); }

    // XMultiComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        OUString const & rServiceSpecifier, Reference< XComponentContext > const & xContext ) override
        { return getRoot()->createInstanceWithContext( rServiceSpecifier, xContext ); }
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        OUString const & rServiceSpecifier,
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext ) override
        { return getRoot()->createInstanceWithArgumentsAndContext( rServiceSpecifier, rArguments, xContext ); }
//      virtual Sequence< OUString > SAL_CALL getAvailableServiceNames()
//          throw (RuntimeException);

    // XMultiServiceFactory
    virtual Sequence< OUString > SAL_CALL getAvailableServiceNames() override
        { return getRoot()->getAvailableServiceNames(); }
    virtual Reference<XInterface > SAL_CALL createInstance(const OUString & name) override
        { return getRoot()->createInstanceWithContext( name, m_xContext ); }
    virtual Reference<XInterface > SAL_CALL createInstanceWithArguments(const OUString & name, const Sequence<Any >& Arguments) override
        { return getRoot()->createInstanceWithArgumentsAndContext( name, Arguments, m_xContext ); }

    // XElementAccess
    virtual Type SAL_CALL getElementType() override
        { return Reference< XElementAccess >(getRoot(), UNO_QUERY_THROW)->getElementType(); }
    virtual sal_Bool SAL_CALL hasElements() override
        { return Reference< XElementAccess >(getRoot(), UNO_QUERY_THROW)->hasElements(); }

    // XEnumerationAccess
    virtual Reference<XEnumeration > SAL_CALL createEnumeration() override
        { return Reference< XEnumerationAccess >(getRoot(), UNO_QUERY_THROW)->createEnumeration(); }

    // XSet
    virtual sal_Bool SAL_CALL has( const Any & Element ) override
        { return Reference< XSet >(getRoot(), UNO_QUERY_THROW)->has( Element ); }
    virtual void SAL_CALL insert( const Any & Element ) override
        { Reference< XSet >(getRoot(), UNO_QUERY_THROW)->insert( Element ); }
    virtual void SAL_CALL remove( const Any & Element ) override
        { Reference< XSet >(getRoot(), UNO_QUERY_THROW)->remove( Element ); }

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    virtual Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) override
        { return Reference< XContentEnumerationAccess >(getRoot(), UNO_QUERY_THROW)->createContentEnumeration( aServiceName ); }

    // XPropertySet
    Reference<XPropertySetInfo > SAL_CALL getPropertySetInfo() override
        { return Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->getPropertySetInfo(); }

    void SAL_CALL setPropertyValue(const OUString& PropertyName, const Any& aValue) override;
    Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;

    void SAL_CALL addPropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener) override
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->addPropertyChangeListener( PropertyName, aListener ); }
    void SAL_CALL removePropertyChangeListener(const OUString& PropertyName, const Reference<XPropertyChangeListener >& aListener) override
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->removePropertyChangeListener( PropertyName, aListener ); }
    void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener) override
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->addVetoableChangeListener( PropertyName, aListener ); }
    void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const Reference<XVetoableChangeListener >& aListener) override
        { Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->removeVetoableChangeListener( PropertyName, aListener ); }
};

void SAL_CALL OServiceManagerWrapper::setPropertyValue(
    const OUString& PropertyName, const Any& aValue )
{
    if ( PropertyName == "DefaultContext" )
    {
        Reference< XComponentContext > xContext;
        if (!(aValue >>= xContext))
        {
            throw IllegalArgumentException(
                "no XComponentContext given!",
                static_cast<OWeakObject *>(this), 1 );
        }

        MutexGuard aGuard( m_mutex );
        m_xContext = xContext;

    }
    else
    {
        Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->setPropertyValue( PropertyName, aValue );
    }
}

Any SAL_CALL OServiceManagerWrapper::getPropertyValue(
    const OUString& PropertyName )
{
    if ( PropertyName == "DefaultContext" )
    {
        MutexGuard aGuard( m_mutex );
        if( m_xContext.is() )
            return makeAny( m_xContext );
        else
            return Any();
    }
    else
    {
        return Reference< XPropertySet >(getRoot(), UNO_QUERY_THROW)->getPropertyValue( PropertyName );
    }
}

void OServiceManagerWrapper::disposing()
{
    m_xContext.clear();

// no m_root->dispose(), because every context disposes its service manager...
    m_root.clear();
}

OServiceManagerWrapper::OServiceManagerWrapper(
    Reference< XComponentContext > const & xContext )
    : t_OServiceManagerWrapper_impl( m_mutex )
    , m_xContext( xContext )
    , m_root( xContext->getServiceManager() )
{
    if (! m_root.is())
    {
        throw RuntimeException(
            "no service manager to wrap" );
    }
}


/**
 * Create a ServiceManager
 */
OServiceManager::OServiceManager( Reference< XComponentContext > const & xContext )
    : t_OServiceManager_impl( m_mutex )
    , m_xContext( xContext )
    , m_bInDisposing( false )
{}

// XComponent
void OServiceManager::dispose()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;
    t_OServiceManager_impl::dispose();
}

void OServiceManager::disposing()
{
    // dispose all factories
    HashSet_Ref aImpls;
    {
        MutexGuard aGuard( m_mutex );
        m_bInDisposing = true;
        aImpls = m_ImplementationMap;
    }
    for( const auto& rxImpl : aImpls )
    {
        try
        {
            Reference<XComponent > xComp( Reference<XComponent >::query( rxImpl ) );
            if( xComp.is() )
                xComp->dispose();
        }
        catch (const RuntimeException & exc)
        {
            SAL_INFO("stoc", "RuntimeException occurred upon disposing factory: " << exc);
        }
    }

    // dispose
    HashSet_Ref aImplMap;
    {
        MutexGuard aGuard( m_mutex );
        // erase all members
        m_ServiceMap = HashMultimap_OWString_Interface();
        aImplMap = m_ImplementationMap;
        m_ImplementationMap = HashSet_Ref();
        m_ImplementationNameMap = HashMap_OWString_Interface();
        m_SetLoadedFactories= HashSet_Ref();
    }

    m_xContext.clear();

    // not only the Event should hold the object
    OSL_ASSERT( m_refCount != 1 );
}

// XPropertySet
Reference<XPropertySetInfo > OServiceManager::getPropertySetInfo()
{
    check_undisposed();
    if (! m_xPropertyInfo.is())
    {
        Sequence< beans::Property > seq( 1 );
        seq[ 0 ] = beans::Property(
            "DefaultContext", -1, cppu::UnoType<decltype(m_xContext)>::get(), 0 );
        Reference< beans::XPropertySetInfo > xInfo( new PropertySetInfo_Impl( seq ) );

        MutexGuard aGuard( m_mutex );
        if (! m_xPropertyInfo.is())
        {
            m_xPropertyInfo = xInfo;
        }
    }
    return m_xPropertyInfo;
}

void OServiceManager::setPropertyValue(
    const OUString& PropertyName, const Any& aValue )
{
    check_undisposed();
    if ( PropertyName != "DefaultContext" )
    {
        throw UnknownPropertyException(
            "unknown property " + PropertyName,
            static_cast<OWeakObject *>(this) );
    }

    Reference< XComponentContext > xContext;
    if (!(aValue >>= xContext))
    {
        throw IllegalArgumentException(
            "no XComponentContext given!",
            static_cast<OWeakObject *>(this), 1 );
    }

    MutexGuard aGuard( m_mutex );
    m_xContext = xContext;
}

Any OServiceManager::getPropertyValue(const OUString& PropertyName)
{
    check_undisposed();
    if ( PropertyName == "DefaultContext" )
    {
        MutexGuard aGuard( m_mutex );
        if( m_xContext.is() )
            return makeAny( m_xContext );
        else
            return Any();
    }
    else
    {
        UnknownPropertyException except;
        except.Message =  "ServiceManager : unknown property " + PropertyName;
        throw except;
    }
}

void OServiceManager::addPropertyChangeListener(
    const OUString&, const Reference<XPropertyChangeListener >&)
{
    check_undisposed();
    throw UnknownPropertyException("unsupported");
}

void OServiceManager::removePropertyChangeListener(
    const OUString&, const Reference<XPropertyChangeListener >&)
{
    check_undisposed();
    throw UnknownPropertyException("unsupported");
}

void OServiceManager::addVetoableChangeListener(
    const OUString&, const Reference<XVetoableChangeListener >&)
{
    check_undisposed();
    throw UnknownPropertyException("unsupported");
}

void OServiceManager::removeVetoableChangeListener(
    const OUString&, const Reference<XVetoableChangeListener >&)
{
    check_undisposed();
    throw UnknownPropertyException("unsupported");
}

// OServiceManager
Reference<XEventListener > OServiceManager::getFactoryListener()
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    if( !xFactoryListener.is() )
        xFactoryListener = new OServiceManager_Listener( this );
    return xFactoryListener;
}

// XMultiServiceFactory, XContentEnumeration
Sequence< OUString > OServiceManager::getUniqueAvailableServiceNames(
    HashSet_OWString & aNameSet )
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    for( const auto& rEntry : m_ServiceMap )
        aNameSet.insert( rEntry.first );

    /* do not return the implementation names
    HashMap_OWString_Interface      m_ImplementationNameMap;
    HashMap_OWString_Interface::iterator aIt = m_ImplementationNameMap.begin();
    while( aIt != m_ImplementationNameMap.end() )
        aNameSet.insert( (*aIt++).first );
    */

    return comphelper::containerToSequence(aNameSet);
}

// XMultiComponentFactory
Reference< XInterface > OServiceManager::createInstanceWithContext(
    OUString const & rServiceSpecifier,
    Reference< XComponentContext > const & xContext )
{
    check_undisposed();
#if OSL_DEBUG_LEVEL > 0
    Reference< beans::XPropertySet > xProps( xContext->getServiceManager(), UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        Reference< XComponentContext > xDefContext;
        xProps->getPropertyValue( "DefaultContext" ) >>= xDefContext;
        OSL_ENSURE(
            xContext == xDefContext,
            "### default context of service manager singleton differs from context holding it!" );
    }
#endif

    Sequence< Reference< XInterface > > factories(
        queryServiceFactories( rServiceSpecifier, xContext ) );
    Reference< XInterface > const * p = factories.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < factories.getLength(); ++nPos )
    {
        try
        {
            Reference< XInterface > const & xFactory = p[ nPos ];
            if (xFactory.is())
            {
                Reference< XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
                if (xFac.is())
                {
                    return xFac->createInstanceWithContext( xContext );
                }
                else
                {
                    Reference< XSingleServiceFactory > xFac2( xFactory, UNO_QUERY );
                    if (xFac2.is())
                    {
                        SAL_INFO("stoc", "ignoring given context raising service " << rServiceSpecifier << "!!!");
                        return xFac2->createInstance();
                    }
                }
            }
        }
        catch (const lang::DisposedException & exc)
        {
            SAL_INFO("stoc", "DisposedException occurred: " << exc);
        }
    }

    return Reference< XInterface >();
}
// XMultiComponentFactory
Reference< XInterface > OServiceManager::createInstanceWithArgumentsAndContext(
    OUString const & rServiceSpecifier,
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
{
    check_undisposed();
#if OSL_DEBUG_LEVEL > 0
    Reference< beans::XPropertySet > xProps( xContext->getServiceManager(), UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    if (xProps.is())
    {
        Reference< XComponentContext > xDefContext;
        xProps->getPropertyValue( "DefaultContext" ) >>= xDefContext;
        OSL_ENSURE(
            xContext == xDefContext,
            "### default context of service manager singleton differs from context holding it!" );
    }
#endif

    Sequence< Reference< XInterface > > factories(
        queryServiceFactories( rServiceSpecifier, xContext ) );
    Reference< XInterface > const * p = factories.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < factories.getLength(); ++nPos )
    {
        try
        {
            Reference< XInterface > const & xFactory = p[ nPos ];
            if (xFactory.is())
            {
                Reference< XSingleComponentFactory > xFac( xFactory, UNO_QUERY );
                if (xFac.is())
                {
                    return xFac->createInstanceWithArgumentsAndContext( rArguments, xContext );
                }
                else
                {
                    Reference< XSingleServiceFactory > xFac2( xFactory, UNO_QUERY );
                    if (xFac2.is())
                    {
                        SAL_INFO("stoc", "ignoring given context raising service " << rServiceSpecifier << "!!!");
                        return xFac2->createInstanceWithArguments( rArguments );
                    }
                }
            }
        }
        catch (const lang::DisposedException & exc)
        {
            SAL_INFO("stoc", "DisposedException occurred: " << exc);
        }
    }

    return Reference< XInterface >();
}

// XMultiServiceFactory, XMultiComponentFactory, XContentEnumeration
Sequence< OUString > OServiceManager::getAvailableServiceNames()
{
    check_undisposed();
    // all names
    HashSet_OWString aNameSet;
    return getUniqueAvailableServiceNames( aNameSet );
}

// XMultibleServiceFactory
Reference<XInterface > OServiceManager::createInstance(
    const OUString& rServiceSpecifier )
{
    return createInstanceWithContext(
        rServiceSpecifier, m_xContext );
}

// XMultibleServiceFactory
Reference<XInterface > OServiceManager::createInstanceWithArguments(
    const OUString& rServiceSpecifier,
    const Sequence<Any >& rArguments )
{
    return createInstanceWithArgumentsAndContext(
        rServiceSpecifier, rArguments, m_xContext );
}

// XInitialization
void OServiceManager::initialize( Sequence< Any > const & )
{
    check_undisposed();
    OSL_FAIL( "not impl!" );
}

// XServiceInfo
OUString OServiceManager::getImplementationName()
{
    return OUString("com.sun.star.comp.stoc.OServiceManager");
}

// XServiceInfo
sal_Bool OServiceManager::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > OServiceManager::getSupportedServiceNames()
{
    Sequence< OUString > seqNames(2);
    seqNames[0] = "com.sun.star.lang.MultiServiceFactory";
    seqNames[1] = "com.sun.star.lang.ServiceManager";
    return seqNames;
}


Sequence< Reference< XInterface > > OServiceManager::queryServiceFactories(
    const OUString& aServiceName, Reference< XComponentContext > const & )
{
    Sequence< Reference< XInterface > > ret;

    MutexGuard aGuard( m_mutex );
    ::std::pair<
          HashMultimap_OWString_Interface::iterator,
          HashMultimap_OWString_Interface::iterator> p(
              m_ServiceMap.equal_range( aServiceName ) );

    if (p.first == p.second) // no factories
    {
        // no service found, look for an implementation
        HashMap_OWString_Interface::iterator aIt = m_ImplementationNameMap.find( aServiceName );
        if( aIt != m_ImplementationNameMap.end() )
        {
            Reference< XInterface > const & x = aIt->second;
            // an implementation found
            ret = Sequence< Reference< XInterface > >( &x, 1 );
        }
    }
    else
    {
        ::std::vector< Reference< XInterface > > vec;
        vec.reserve( 4 );
        while (p.first != p.second)
        {
            vec.push_back( p.first->second );
            ++p.first;
        }
        ret = Sequence< Reference< XInterface > >( vec.data(), vec.size() );
    }

    return ret;
}

// XContentEnumerationAccess
Reference<XEnumeration > OServiceManager::createContentEnumeration(
    const OUString& aServiceName )
{
    check_undisposed();
    Sequence< Reference< XInterface > > factories(
        OServiceManager::queryServiceFactories( aServiceName, m_xContext ) );
    if (factories.getLength())
        return new ServiceEnumeration_Impl( factories );
    else
        return Reference< XEnumeration >();
}

// XEnumeration
Reference<XEnumeration > OServiceManager::createEnumeration()
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    return new ImplementationEnumeration_Impl( m_ImplementationMap );
}

// XElementAccess
Type OServiceManager::getElementType()
{
    check_undisposed();
    return cppu::UnoType<XInterface>::get();
}

// XElementAccess
sal_Bool OServiceManager::hasElements()
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    return !m_ImplementationMap.empty();
}

// XSet
sal_Bool OServiceManager::has( const Any & Element )
{
    check_undisposed();
    if( Element.getValueTypeClass() == TypeClass_INTERFACE )
    {
        Reference<XInterface > xEle( Element, UNO_QUERY_THROW );
        MutexGuard aGuard( m_mutex );
        return m_ImplementationMap.find( xEle ) !=
            m_ImplementationMap.end();
    }
    else if (auto implName = o3tl::tryAccess<OUString>(Element))
    {
        MutexGuard aGuard( m_mutex );
        return m_ImplementationNameMap.find( *implName ) !=
            m_ImplementationNameMap.end();
    }
    return false;
}

// XSet
void OServiceManager::insert( const Any & Element )
{
    check_undisposed();
    if( Element.getValueTypeClass() != TypeClass_INTERFACE )
    {
        throw IllegalArgumentException(
            "exception interface, got " + Element.getValueType().getTypeName(),
            Reference< XInterface >(), 0 );
    }
    Reference<XInterface > xEle( Element, UNO_QUERY_THROW );

    {
    MutexGuard aGuard( m_mutex );
    HashSet_Ref::iterator aIt = m_ImplementationMap.find( xEle );
    if( aIt != m_ImplementationMap.end() )
    {
        throw ElementExistException( "element already exists!" );
    }

    // put into the implementation hashmap
    m_ImplementationMap.insert( xEle );

    // put into the implementation name hashmap
    Reference<XServiceInfo > xInfo( Reference<XServiceInfo >::query( xEle ) );
    if( xInfo.is() )
    {
        OUString aImplName = xInfo->getImplementationName();
        if( !aImplName.isEmpty() )
            m_ImplementationNameMap[ aImplName ] = xEle;

        //put into the service map
        Sequence< OUString > aServiceNames = xInfo->getSupportedServiceNames();
        const OUString * pArray = aServiceNames.getConstArray();
        for( sal_Int32 i = 0; i < aServiceNames.getLength(); i++ )
        {
            m_ServiceMap.emplace(
                pArray[i], *o3tl::doAccess<Reference<XInterface>>(Element) );
        }
    }
    }
    // add the disposing listener to the factory
    Reference<XComponent > xComp( Reference<XComponent >::query( xEle ) );
    if( xComp.is() )
        xComp->addEventListener( getFactoryListener() );
}

// helper function
bool OServiceManager::haveFactoryWithThisImplementation(const OUString& aImplName)
{
    return ( m_ImplementationNameMap.find(aImplName) != m_ImplementationNameMap.end());
}

// XSet
void OServiceManager::remove( const Any & Element )
{
    if (is_disposed())
        return;

    Reference<XInterface > xEle;
    if (Element.getValueTypeClass() == TypeClass_INTERFACE)
    {
        xEle.set( Element, UNO_QUERY_THROW );
    }
    else if (auto implName = o3tl::tryAccess<OUString>(Element))
    {
        MutexGuard aGuard( m_mutex );
        HashMap_OWString_Interface::const_iterator const iFind(
            m_ImplementationNameMap.find( *implName ) );
        if (iFind == m_ImplementationNameMap.end())
        {
            throw NoSuchElementException(
                "element is not in: " + *implName,
                static_cast< OWeakObject * >(this) );
        }
        xEle = iFind->second;
    }
    else
    {
        throw IllegalArgumentException(
            "expected interface or string, got " + Element.getValueType().getTypeName(),
            Reference< XInterface >(), 0 );
    }

    // remove the disposing listener from the factory
    Reference<XComponent > xComp( Reference<XComponent >::query( xEle ) );
    if( xComp.is() )
        xComp->removeEventListener( getFactoryListener() );

    MutexGuard aGuard( m_mutex );
    HashSet_Ref::iterator aIt = m_ImplementationMap.find( xEle );
    if( aIt == m_ImplementationMap.end() )
    {
        throw NoSuchElementException(
            "element not found",
            static_cast< OWeakObject * >(this) );
    }
    //First remove all factories which have been loaded by ORegistryServiceManager.
    m_SetLoadedFactories.erase( *aIt);
    //Remove from the implementation map. It contains all factories of m_SetLoadedFactories
    //which have been added directly through XSet, that is not via ORegistryServiceManager
    m_ImplementationMap.erase( aIt );

    // remove from the implementation name hashmap
    Reference<XServiceInfo > xInfo( Reference<XServiceInfo >::query( xEle ) );
    if( xInfo.is() )
    {
        OUString aImplName = xInfo->getImplementationName();
        if( !aImplName.isEmpty() )
            m_ImplementationNameMap.erase( aImplName );
    }

    //remove from the service map
    Reference<XServiceInfo > xSF( Reference<XServiceInfo >::query( xEle ) );
    if( !xSF.is() )
        return;

    Sequence< OUString > aServiceNames = xSF->getSupportedServiceNames();
    const OUString * pArray = aServiceNames.getConstArray();
    for( sal_Int32 i = 0; i < aServiceNames.getLength(); i++ )
    {
        pair<HashMultimap_OWString_Interface::iterator, HashMultimap_OWString_Interface::iterator> p =
            m_ServiceMap.equal_range( pArray[i] );

        while( p.first != p.second )
        {
            if( xEle == (*p.first).second )
            {
                m_ServiceMap.erase( p.first );
                break;
            }
            ++p.first;
        }
    }
}

/*****************************************************************************
    class ORegistryServiceManager
*****************************************************************************/
class ORegistryServiceManager : public OServiceManager
{
public:
    explicit ORegistryServiceManager( Reference< XComponentContext > const & xContext );

    // XInitialization
    void SAL_CALL initialize(const Sequence< Any >& Arguments) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
        { return OUString("com.sun.star.comp.stoc.ORegistryServiceManager"); }

    Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XMultiServiceFactory
    Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

    // XContentEnumerationAccess
    //Sequence< OUString >          getAvailableServiceNames() throw( (Exception) );
    Reference<XEnumeration > SAL_CALL createContentEnumeration(const OUString& aServiceName) override;

    // XComponent
    void SAL_CALL dispose() override;

    // OServiceManager
    Reference<XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;

protected:
    //OServiceManager
    Sequence< Reference< XInterface > > queryServiceFactories(
        const OUString& aServiceName, Reference< XComponentContext > const & xContext ) override;
private:
    Reference<XRegistryKey >        getRootKey();
    Reference<XInterface > loadWithImplementationName(
        const OUString & rImplName, Reference< XComponentContext > const & xContext );
    Sequence<OUString>          getFromServiceName(const OUString& serviceName) const;
    Reference<XInterface > loadWithServiceName(
        const OUString & rImplName, Reference< XComponentContext > const & xContext );
    void                        fillAllNamesFromRegistry( HashSet_OWString & );

    bool                    m_searchedRegistry;
    Reference<XSimpleRegistry > m_xRegistry;    // readonly property Registry
    Reference<XRegistryKey >    m_xRootKey;

#if OSL_DEBUG_LEVEL > 0
    bool m_init;
#endif
};

/**
 * Create a ServiceManager
 */
ORegistryServiceManager::ORegistryServiceManager( Reference< XComponentContext > const & xContext )
    : OServiceManager( xContext )
    , m_searchedRegistry(false)
#if OSL_DEBUG_LEVEL > 0
    , m_init( false )
#endif
{
}

// XComponent
void ORegistryServiceManager::dispose()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;
    OServiceManager::dispose();
    // dispose
    MutexGuard aGuard( m_mutex );
    // erase all members
    m_xRegistry.clear();
    m_xRootKey.clear();
}

/**
 * Return the root key of the registry. The Default registry service is ordered
 * if no registry is set.
 */
//Reference<XServiceProvider > create_DefaultRegistry_ServiceProvider();

Reference<XRegistryKey > ORegistryServiceManager::getRootKey()
{
    if( !m_xRootKey.is() )
    {
        MutexGuard aGuard( m_mutex );
        //  DefaultRegistry suchen !!!!
        if( !m_xRegistry.is() && !m_searchedRegistry )
        {
            // NB. we only search this once
            m_searchedRegistry = true;

            m_xRegistry.set(
                createInstanceWithContext(
                    "com.sun.star.registry.DefaultRegistry",
                    m_xContext ),
                UNO_QUERY );
        }
        if( m_xRegistry.is() && !m_xRootKey.is() )
            m_xRootKey = m_xRegistry->getRootKey();
    }

    return m_xRootKey;
}

/**
 * Create a service provider from the registry with an implementation name
 */
Reference<XInterface > ORegistryServiceManager::loadWithImplementationName(
    const OUString& name, Reference< XComponentContext > const & xContext )
{
    Reference<XInterface > ret;

    Reference<XRegistryKey > xRootKey = getRootKey();
    if( !xRootKey.is() )
        return ret;

    try
    {
        OUString implementationName = "/IMPLEMENTATIONS/" + name;
        Reference<XRegistryKey > xImpKey = m_xRootKey->openKey(implementationName);

        if( xImpKey.is() )
        {
            Reference< lang::XMultiServiceFactory > xMgr;
            if (xContext.is())
                xMgr.set( xContext->getServiceManager(), UNO_QUERY_THROW );
            else
                xMgr.set( this );
            ret = createSingleRegistryFactory( xMgr, name, xImpKey );
            insert( makeAny( ret ) );
            // Remember this factory as loaded in contrast to inserted ( XSet::insert)
            // factories. Those loaded factories in this set are candidates for being
            // released on an unloading notification.
            m_SetLoadedFactories.insert( ret);
        }
    }
    catch (InvalidRegistryException &)
    {
    }

    return ret;
}

/**
 * Return all implementation out of the registry.
 */
Sequence<OUString> ORegistryServiceManager::getFromServiceName(
    const OUString& serviceName ) const
{
    OUStringBuffer buf;
    buf.append( "/SERVICES/" );
    buf.append( serviceName );
    return retrieveAsciiValueList( m_xRegistry, buf.makeStringAndClear() );
}

/**
 * Create a service provider from the registry
 */
Reference<XInterface > ORegistryServiceManager::loadWithServiceName(
    const OUString& serviceName, Reference< XComponentContext > const & xContext )
{
    Sequence<OUString> implEntries = getFromServiceName( serviceName );
    for (sal_Int32 i = 0; i < implEntries.getLength(); i++)
    {
        Reference< XInterface > x(
            loadWithImplementationName( implEntries.getConstArray()[i], xContext ) );
        if (x.is())
            return x;
    }

    return Reference<XInterface >();
}

/**
 * Return a sequence of all service names from the registry.
 */
void ORegistryServiceManager::fillAllNamesFromRegistry( HashSet_OWString & rSet )
{
    Reference<XRegistryKey > xRootKey = getRootKey();
    if( !xRootKey.is() )
        return;

    try
    {
        Reference<XRegistryKey > xServicesKey = xRootKey->openKey( "SERVICES" );
        // root + /Services + /
        if( xServicesKey.is() )
        {
            sal_Int32 nPrefix = xServicesKey->getKeyName().getLength() +1;
            Sequence<Reference<XRegistryKey > > aKeys = xServicesKey->openKeys();
            for( sal_Int32 i = 0; i < aKeys.getLength(); i++ )
                rSet.insert( aKeys.getConstArray()[i]->getKeyName().copy( nPrefix ) );
        }
    }
    catch (InvalidRegistryException &)
    {
    }
}

// XInitialization
void ORegistryServiceManager::initialize(const Sequence< Any >& Arguments)
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    if (Arguments.getLength() > 0)
    {
        m_xRootKey.clear();
        Arguments[ 0 ] >>= m_xRegistry;
    }
#if OSL_DEBUG_LEVEL > 0
    // to find all bootstrapping processes to be fixed...
    OSL_ENSURE( !m_init, "### second init of service manager instance!" );
    m_init = true;
#endif
}

// XMultiServiceFactory, XContentEnumeration
Sequence< OUString > ORegistryServiceManager::getAvailableServiceNames()
{
    check_undisposed();
    MutexGuard aGuard( m_mutex );
    // all names
    HashSet_OWString aNameSet;

    // all names from the registry
    fillAllNamesFromRegistry( aNameSet );

    return OServiceManager::getUniqueAvailableServiceNames( aNameSet );
}

// XServiceInfo
Sequence< OUString > ORegistryServiceManager::getSupportedServiceNames()
{
    Sequence< OUString > seqNames(2);
    seqNames[0] = "com.sun.star.lang.MultiServiceFactory";
    seqNames[1] = "com.sun.star.lang.RegistryServiceManager";
    return seqNames;
}


// OServiceManager
Sequence< Reference< XInterface > > ORegistryServiceManager::queryServiceFactories(
    const OUString& aServiceName, Reference< XComponentContext > const & xContext )
{
    Sequence< Reference< XInterface > > ret(
        OServiceManager::queryServiceFactories( aServiceName, xContext ) );
    if (ret.getLength())
    {
        return ret;
    }
    else
    {
        MutexGuard aGuard( m_mutex );
        Reference< XInterface > x( loadWithServiceName( aServiceName, xContext ) );
        if (! x.is())
            x = loadWithImplementationName( aServiceName, xContext );
        return Sequence< Reference< XInterface > >( &x, 1 );
    }
}

// XContentEnumerationAccess
Reference<XEnumeration > ORegistryServiceManager::createContentEnumeration(
    const OUString& aServiceName )
{
    check_undisposed();
    MutexGuard aGuard(m_mutex);
    // get all implementation names registered under this service name from the registry
    Sequence<OUString> aImpls = getFromServiceName( aServiceName );
    // load and insert all factories specified by the registry
    sal_Int32 i;
    OUString aImplName;
    for( i = 0; i < aImpls.getLength(); i++ )
    {
        aImplName = aImpls.getConstArray()[i];
        if ( !haveFactoryWithThisImplementation(aImplName) )
        {
            loadWithImplementationName( aImplName, m_xContext );
        }
    }
    // call the superclass to enumerate all contents
    return OServiceManager::createContentEnumeration( aServiceName );
}

// OServiceManager
Reference<XPropertySetInfo > ORegistryServiceManager::getPropertySetInfo()
{
    check_undisposed();
    if (! m_xPropertyInfo.is())
    {
        Sequence< beans::Property > seq( 2 );
        seq[ 0 ] = beans::Property(
            "DefaultContext", -1, cppu::UnoType<decltype(m_xContext)>::get(), 0 );
        seq[ 1 ] = beans::Property(
            "Registry", -1, cppu::UnoType<decltype(m_xRegistry)>::get(),
            beans::PropertyAttribute::READONLY );
        Reference< beans::XPropertySetInfo > xInfo( new PropertySetInfo_Impl( seq ) );

        MutexGuard aGuard( m_mutex );
        if (! m_xPropertyInfo.is())
        {
            m_xPropertyInfo = xInfo;
        }
    }
    return m_xPropertyInfo;
}

Any ORegistryServiceManager::getPropertyValue(const OUString& PropertyName)
{
    check_undisposed();
    if ( PropertyName == "Registry" )
    {
        MutexGuard aGuard( m_mutex );
        if( m_xRegistry.is() )
            return makeAny( m_xRegistry );
        else
            return Any();
    }
    return OServiceManager::getPropertyValue( PropertyName );
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_stoc_OServiceManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OServiceManager(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_stoc_ORegistryServiceManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ORegistryServiceManager(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_stoc_OServiceManagerWrapper_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OServiceManagerWrapper(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
