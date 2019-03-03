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

#include <unordered_map>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <sal/log.hxx>

#include <uno/lbnames.h>
#include <uno/mapping.hxx>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/component_context.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <comphelper/sequence.hxx>

#include <memory>

#define SMGR_SINGLETON "/singletons/com.sun.star.lang.theServiceManager"
#define TDMGR_SINGLETON "/singletons/com.sun.star.reflection.theTypeDescriptionManager"
#define AC_SINGLETON "/singletons/com.sun.star.security.theAccessController"

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace cppu
{

static void try_dispose( Reference< XInterface > const & xInstance )
{
    Reference< lang::XComponent > xComp( xInstance, UNO_QUERY );
    if (xComp.is())
    {
        xComp->dispose();
    }
}

static void try_dispose( Reference< lang::XComponent > const & xComp )
{
    if (xComp.is())
    {
        xComp->dispose();
    }
}

class DisposingForwarder
    : public WeakImplHelper< lang::XEventListener >
{
    Reference< lang::XComponent > m_xTarget;

    explicit DisposingForwarder( Reference< lang::XComponent > const & xTarget )
        : m_xTarget( xTarget )
    {
        OSL_ASSERT( m_xTarget.is() );
    }
public:
    // listens at source for disposing, then disposes target
    static inline void listen(
        Reference< lang::XComponent > const & xSource,
        Reference< lang::XComponent > const & xTarget );

    virtual void SAL_CALL disposing( lang::EventObject const & rSource ) override;
};

inline void DisposingForwarder::listen(
    Reference< lang::XComponent > const & xSource,
    Reference< lang::XComponent > const & xTarget )
{
    if (xSource.is())
    {
        xSource->addEventListener( new DisposingForwarder( xTarget ) );
    }
}

void DisposingForwarder::disposing( lang::EventObject const & )
{
    m_xTarget->dispose();
    m_xTarget.clear();
}


struct MutexHolder
{
protected:
    Mutex m_mutex;
};


class ComponentContext
    : private MutexHolder
    , public WeakComponentImplHelper< XComponentContext,
                                      container::XNameContainer >
{
protected:
    Reference< XComponentContext > m_xDelegate;

    struct ContextEntry
    {
        Any value;
        bool lateInit;

        ContextEntry( Any const & value_, bool lateInit_ )
            : value( value_ )
            , lateInit( lateInit_ )
            {}
    };
    typedef std::unordered_map< OUString, ContextEntry  > t_map;
    t_map m_map;

    Reference< lang::XMultiComponentFactory > m_xSMgr;

protected:
    Any lookupMap( OUString const & rName );

    virtual void SAL_CALL disposing() override;
public:
    ComponentContext(
        ContextEntry_Init const * pEntries, sal_Int32 nEntries,
        Reference< XComponentContext > const & xDelegate );

    // XComponentContext
    virtual Any SAL_CALL getValueByName( OUString const & rName ) override;
    virtual Reference<lang::XMultiComponentFactory> SAL_CALL getServiceManager() override;

    // XNameContainer
    virtual void SAL_CALL insertByName(
        OUString const & name, Any const & element ) override;
    virtual void SAL_CALL removeByName( OUString const & name ) override;
    // XNameReplace
    virtual void SAL_CALL replaceByName(
        OUString const & name, Any const & element ) override;
    // XNameAccess
    virtual Any SAL_CALL getByName( OUString const & name ) override;
    virtual Sequence<OUString> SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( OUString const & name ) override;
    // XElementAccess
    virtual Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;
};

// XNameContainer

void ComponentContext::insertByName(
    OUString const & name, Any const & element )
{
    ContextEntry entry(
            element,
            /* lateInit_: */
            name.startsWith( "/singletons/" ) &&
            !element.hasValue() );
    MutexGuard guard( m_mutex );
    std::pair<t_map::iterator, bool> insertion( m_map.emplace(
        name, entry ) );
    if (! insertion.second)
        throw container::ElementExistException(
            "element already exists: " + name,
            static_cast<OWeakObject *>(this) );
}


void ComponentContext::removeByName( OUString const & name )
{
    MutexGuard guard( m_mutex );
    t_map::iterator iFind( m_map.find( name ) );
    if (iFind == m_map.end())
        throw container::NoSuchElementException(
            "no such element: " + name,
            static_cast<OWeakObject *>(this) );

    m_map.erase(iFind);
}

// XNameReplace

void ComponentContext::replaceByName(
    OUString const & name, Any const & element )
{
    MutexGuard guard( m_mutex );
    t_map::iterator iFind( m_map.find( name ) );
    if (iFind == m_map.end())
        throw container::NoSuchElementException(
            "no such element: " + name,
            static_cast<OWeakObject *>(this) );
    if (name.startsWith( "/singletons/" ) &&
        !element.hasValue())
    {
        iFind->second.value.clear();
        iFind->second.lateInit = true;
    }
    else
    {
        iFind->second.value = element;
        iFind->second.lateInit = false;
    }
}

// XNameAccess

Any ComponentContext::getByName( OUString const & name )
{
    return getValueByName( name );
}


Sequence<OUString> ComponentContext::getElementNames()
{
    MutexGuard guard( m_mutex );
    return comphelper::mapKeysToSequence(m_map);
}


sal_Bool ComponentContext::hasByName( OUString const & name )
{
    MutexGuard guard( m_mutex );
    return m_map.find( name ) != m_map.end();
}

// XElementAccess

Type ComponentContext::getElementType()
{
    return cppu::UnoType<void>::get();
}


sal_Bool ComponentContext::hasElements()
{
    MutexGuard guard( m_mutex );
    return ! m_map.empty();
}


Any ComponentContext::lookupMap( OUString const & rName )
{
    ResettableMutexGuard guard( m_mutex );
    t_map::iterator iFind( m_map.find( rName ) );
    if (iFind == m_map.end())
        return Any();

    ContextEntry& rFindEntry = iFind->second;
    if (! rFindEntry.lateInit)
        return rFindEntry.value;

    // late init singleton entry
    Reference< XInterface > xInstance;
    guard.clear();

    try
    {
        Any usesService( getValueByName( rName + "/service" ) );
        Any args_( getValueByName( rName + "/arguments" ) );
        Sequence<Any> args;
        if (args_.hasValue() && !(args_ >>= args))
        {
            args.realloc( 1 );
            args[ 0 ] = args_;
        }

        Reference< lang::XSingleComponentFactory > xFac;
        if (usesService >>= xFac) // try via factory
        {
            xInstance = args.getLength()
                ? xFac->createInstanceWithArgumentsAndContext( args, this )
                : xFac->createInstanceWithContext( this );
        }
        else
        {
            Reference< lang::XSingleServiceFactory > xFac2;
            if (usesService >>= xFac2)
            {
                // try via old XSingleServiceFactory
                xInstance = args.getLength()
                    ? xFac2->createInstanceWithArguments( args )
                    : xFac2->createInstance();
            }
            else if (m_xSMgr.is()) // optionally service name
            {
                OUString serviceName;
                if ((usesService >>= serviceName) &&
                    !serviceName.isEmpty())
                {
                    xInstance = args.getLength()
                        ? m_xSMgr->createInstanceWithArgumentsAndContext(
                            serviceName, args, this )
                        : m_xSMgr->createInstanceWithContext(
                            serviceName, this );
                }
            }
        }
    }
    catch (const RuntimeException &)
    {
        throw;
    }
    catch (const Exception & exc)
    {
        SAL_WARN(
            "cppuhelper",
            "exception occurred raising singleton \"" << rName << "\": "
            << exc);
    }

    SAL_WARN_IF(!xInstance.is(),
            "cppuhelper", "no service object raising singleton " << rName);

    Any ret;
    guard.reset();
    iFind = m_map.find( rName );
    if (iFind != m_map.end())
    {
        ContextEntry & rEntry = iFind->second;
        if (rEntry.lateInit)
        {
            rEntry.value <<= xInstance;
            rEntry.lateInit = false;
            return rEntry.value;
        }
        ret = rEntry.value;
    }
    guard.clear();
    if (ret != xInstance) {
        try_dispose( xInstance );
    }
    return ret;
}


Any ComponentContext::getValueByName( OUString const & rName )
{
    // to determine the root context:
    if ( rName == "_root" )
    {
        if (m_xDelegate.is())
            return m_xDelegate->getValueByName( rName );
        return Any( Reference<XComponentContext>(this) );
    }

    Any ret( lookupMap( rName ) );
    if (!ret.hasValue() && m_xDelegate.is())
    {
        return m_xDelegate->getValueByName( rName );
    }
    return ret;
}

Reference< lang::XMultiComponentFactory > ComponentContext::getServiceManager()
{
    if ( !m_xSMgr.is() )
    {
        throw DeploymentException(
            "null component context service manager",
            static_cast<OWeakObject *>(this) );
    }
    return m_xSMgr;
}

void ComponentContext::disposing()
{
    Reference< lang::XComponent > xTDMgr, xAC; // to be disposed separately

    // dispose all context objects
    for ( auto& [rName, rEntry] : m_map )
    {
        // service manager disposed separately
        if (!m_xSMgr.is() ||
            !rName.startsWith( SMGR_SINGLETON ))
        {
            if (rEntry.lateInit)
            {
                // late init
                MutexGuard guard( m_mutex );
                if (rEntry.lateInit)
                {
                    rEntry.value.clear(); // release factory
                    rEntry.lateInit = false;
                    continue;
                }
            }

            Reference< lang::XComponent > xComp;
            rEntry.value >>= xComp;
            if (xComp.is())
            {
                if ( rName == TDMGR_SINGLETON )
                {
                    xTDMgr = xComp;
                }
                else if ( rName == AC_SINGLETON )
                {
                    xAC = xComp;
                }
                else // dispose immediately
                {
                    xComp->dispose();
                }
            }
        }
    }

    // dispose service manager
    try_dispose( m_xSMgr );
    m_xSMgr.clear();
    // dispose ac
    try_dispose( xAC );
    // dispose tdmgr; revokes callback from cppu runtime
    try_dispose( xTDMgr );

    m_map.clear();

    // Hack to terminate any JNI bridge's AsynchronousFinalizer thread (as JNI
    // proxies get finalized with arbitrary delay, so the bridge typically does
    // not dispose itself early enough before the process exits):
    uno_Environment ** envs;
    sal_Int32 envCount;
    uno_getRegisteredEnvironments(
        &envs, &envCount, &rtl_allocateMemory, OUString("java").pData);
    assert(envCount >= 0);
    assert(envCount == 0 || envs != nullptr);
    if (envs) {
        for (sal_Int32 i = 0; i != envCount; ++i) {
            assert(envs[i] != nullptr);
            assert(envs[i]->dispose != nullptr);
            (*envs[i]->dispose)(envs[i]);
        }
        std::free(envs);
    }
}

ComponentContext::ComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
    : WeakComponentImplHelper( m_mutex ),
      m_xDelegate( xDelegate )
{
    for ( sal_Int32 nPos = 0; nPos < nEntries; ++nPos )
    {
        ContextEntry_Init const & rEntry = pEntries[ nPos ];

        if ( rEntry.name == SMGR_SINGLETON )
        {
            rEntry.value >>= m_xSMgr;
        }

        if (rEntry.bLateInitService)
        {
            // singleton entry
            m_map.emplace( rEntry.name, ContextEntry( Any(), true ) );
            // service
            m_map.emplace( rEntry.name + "/service", ContextEntry( rEntry.value, false ) );
            // initial-arguments are provided as optional context entry
        }
        else
        {
            // only value, no late init factory nor string
            m_map.emplace( rEntry.name, ContextEntry( rEntry.value, false ) );
        }
    }

    if (!m_xSMgr.is() && m_xDelegate.is())
    {
        // wrap delegate's smgr XPropertySet into new smgr
        Reference< lang::XMultiComponentFactory > xMgr( m_xDelegate->getServiceManager() );
        if (xMgr.is())
        {
            osl_atomic_increment( &m_refCount );
            try
            {
                // create new smgr based on delegate's one
                m_xSMgr.set(
                    xMgr->createInstanceWithContext(
                        "com.sun.star.comp.stoc.OServiceManagerWrapper", xDelegate ),
                    UNO_QUERY );
                // patch DefaultContext property of new one
                Reference< beans::XPropertySet > xProps( m_xSMgr, UNO_QUERY );
                OSL_ASSERT( xProps.is() );
                if (xProps.is())
                {
                    Reference< XComponentContext > xThis( this );
                    xProps->setPropertyValue( "DefaultContext", Any( xThis ) );
                }
            }
            catch (...)
            {
                osl_atomic_decrement( &m_refCount );
                throw;
            }
            osl_atomic_decrement( &m_refCount );
            OSL_ASSERT( m_xSMgr.is() );
        }
    }
}


extern "C" { static void s_createComponentContext_v(va_list * pParam)
{
    ContextEntry_Init const  * pEntries     = va_arg(*pParam, ContextEntry_Init const *);
    sal_Int32                  nEntries     = va_arg(*pParam, sal_Int32);
    XComponentContext        * pDelegatee   = va_arg(*pParam, XComponentContext *);
    void                    ** ppContext    = va_arg(*pParam, void **);
    uno::Mapping             * pTarget2curr = va_arg(*pParam, uno::Mapping *);

    Reference<XComponentContext> xDelegate(pDelegatee, SAL_NO_ACQUIRE);
    Reference<XComponentContext> xContext;

    if (nEntries > 0)
    {
        try
        {
            ComponentContext * p = new ComponentContext( pEntries, nEntries, xDelegate );
            xContext.set(p);
            // listen delegate for disposing, to dispose this (wrapping) context first.
            DisposingForwarder::listen( Reference< lang::XComponent >::query( xDelegate ), p );
        }
        catch (Exception & exc)
        {
            SAL_WARN( "cppuhelper", exc );
            xContext.clear();
        }
    }
    else
    {
        xContext = xDelegate;
    }

    *ppContext = pTarget2curr->mapInterface(xContext.get(), cppu::UnoType<decltype(xContext)>::get());
}}

Reference< XComponentContext > SAL_CALL createComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    Reference< XComponentContext > const & xDelegate )
{
    uno::Environment curr_env(Environment::getCurrent());
    uno::Environment source_env(CPPU_CURRENT_LANGUAGE_BINDING_NAME);

    uno::Mapping curr2source(curr_env, source_env);
    uno::Mapping source2curr(source_env, curr_env);

    std::unique_ptr<ContextEntry_Init[]> mapped_entries(new ContextEntry_Init[nEntries]);
    for (sal_Int32 nPos = 0; nPos < nEntries; ++ nPos)
    {
        mapped_entries[nPos].bLateInitService = pEntries[nPos].bLateInitService;
        mapped_entries[nPos].name             = pEntries[nPos].name;

        uno_type_any_constructAndConvert(&mapped_entries[nPos].value,
                                         const_cast<void *>(pEntries[nPos].value.getValue()),
                                         pEntries[nPos].value.getValueTypeRef(),
                                         curr2source.get());
    }

    void * mapped_delegate = curr2source.mapInterface(xDelegate.get(), cppu::UnoType<decltype(xDelegate)>::get());
    XComponentContext * pXComponentContext = nullptr;
    source_env.invoke(s_createComponentContext_v, mapped_entries.get(), nEntries, mapped_delegate, &pXComponentContext, &source2curr);
    mapped_entries.reset();

    return Reference<XComponentContext>(pXComponentContext, SAL_NO_ACQUIRE);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
