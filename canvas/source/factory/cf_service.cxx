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

#include <algorithm>
#include <utility>
#include <vector>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>
#include <o3tl/functional.hxx>
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLWrapper.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace
{

class CanvasFactory
    : public ::cppu::WeakImplHelper< lang::XServiceInfo,
                                      lang::XMultiComponentFactory,
                                      lang::XMultiServiceFactory >
{
    typedef std::pair< OUString, Sequence< OUString > > AvailPair;
    typedef std::pair< OUString, OUString >             CachePair;
    typedef std::vector< AvailPair >                    AvailVector;
    typedef std::vector< CachePair >                    CacheVector;


    mutable ::osl::Mutex              m_mutex;
    Reference<XComponentContext>      m_xContext;
    Reference<container::XNameAccess> m_xCanvasConfigNameAccess;
    AvailVector                       m_aAvailableImplementations;
    AvailVector                       m_aAcceleratedImplementations;
    AvailVector                       m_aAAImplementations;
    mutable CacheVector               m_aCachedImplementations;
    mutable bool                      m_bCacheHasForcedLastImpl;
    mutable bool                      m_bCacheHasUseAcceleratedEntry;
    mutable bool                      m_bCacheHasUseAAEntry;

    void checkConfigFlag( bool& r_bFlag,
                          bool& r_CacheFlag,
                          const OUString& nodeName ) const;
    Reference<XInterface> use(
        OUString const & serviceName,
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xContext ) const;
    Reference<XInterface> lookupAndUse(
        OUString const & serviceName, Sequence<Any> const & args,
        Reference<XComponentContext> const & xContext ) const;

public:
    virtual ~CanvasFactory() override;
    explicit CanvasFactory( Reference<XComponentContext> const & xContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName ) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XMultiComponentFactory
    virtual Sequence<OUString> SAL_CALL getAvailableServiceNames() override;
    virtual Reference<XInterface> SAL_CALL createInstanceWithContext(
        OUString const & name,
        Reference<XComponentContext> const & xContext ) override;
    virtual Reference<XInterface> SAL_CALL
    createInstanceWithArgumentsAndContext(
        OUString const & name,
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xContext ) override;

    // XMultiServiceFactory
    virtual Reference<XInterface> SAL_CALL createInstance(
        OUString const & name ) override;
    virtual Reference<XInterface> SAL_CALL createInstanceWithArguments(
        OUString const & name, Sequence<Any> const & args ) override;
};

CanvasFactory::CanvasFactory( Reference<XComponentContext> const & xContext ) :
    m_mutex(),
    m_xContext(xContext),
    m_xCanvasConfigNameAccess(),
    m_aAvailableImplementations(),
    m_aAcceleratedImplementations(),
    m_aAAImplementations(),
    m_aCachedImplementations(),
    m_bCacheHasForcedLastImpl(),
    m_bCacheHasUseAcceleratedEntry(),
    m_bCacheHasUseAAEntry()
{
    try
    {
        // read out configuration for preferred services:
        Reference<lang::XMultiServiceFactory> xConfigProvider(
            configuration::theDefaultProvider::get( m_xContext ) );

        Any propValue(
            Any( beans::PropertyValue(
                         "nodepath", -1,
                         Any( OUString("/org.openoffice.Office.Canvas") ),
                         beans::PropertyState_DIRECT_VALUE ) ) );

        m_xCanvasConfigNameAccess.set(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                Sequence<Any>( &propValue, 1 ) ),
            UNO_QUERY_THROW );

        propValue = Any(
            beans::PropertyValue(
                "nodepath", -1,
                Any( OUString("/org.openoffice.Office.Canvas/CanvasServiceList") ),
                beans::PropertyState_DIRECT_VALUE ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );
        Reference<container::XHierarchicalNameAccess> xHierarchicalNameAccess(
            xNameAccess, UNO_QUERY_THROW);

        Sequence<OUString> serviceNames = xNameAccess->getElementNames();
        const OUString* pCurr = serviceNames.getConstArray();
        const OUString* const pEnd = pCurr + serviceNames.getLength();
        while( pCurr != pEnd )
        {
            Reference<container::XNameAccess> xEntryNameAccess(
                xHierarchicalNameAccess->getByHierarchicalName(*pCurr),
                UNO_QUERY );

            if( xEntryNameAccess.is() )
            {
                Sequence<OUString> implementationList;
                if( (xEntryNameAccess->getByName("PreferredImplementations") >>= implementationList) )
                {
                    m_aAvailableImplementations.push_back( std::make_pair(*pCurr,implementationList) );
                }
                if( (xEntryNameAccess->getByName("AcceleratedImplementations") >>= implementationList) )
                {
                    m_aAcceleratedImplementations.push_back( std::make_pair(*pCurr,implementationList) );
                }
                if( (xEntryNameAccess->getByName("AntialiasingImplementations") >>= implementationList) )
                {
                    m_aAAImplementations.push_back( std::make_pair(*pCurr,implementationList) );
                }

            }

            ++pCurr;
        }
    }
    catch (const RuntimeException &)
    {
        throw;
    }
    catch (const Exception&)
    {
    }

    if( m_aAvailableImplementations.empty() )
    {
        // Ugh. Looks like configuration is borked. Fake minimal
        // setup.
        Sequence<OUString> aServices { "com.sun.star.comp.rendering.Canvas.VCL" };
        m_aAvailableImplementations.push_back( std::make_pair(OUString("com.sun.star.rendering.Canvas"),
                                                              aServices) );

        aServices[0] = "com.sun.star.comp.rendering.SpriteCanvas.VCL";
        m_aAvailableImplementations.push_back( std::make_pair(OUString("com.sun.star.rendering.SpriteCanvas"),
                                                              aServices) );
    }
}

CanvasFactory::~CanvasFactory()
{
}


// XServiceInfo
OUString CanvasFactory::getImplementationName()
{
    return OUString("com.sun.star.comp.rendering.CanvasFactory");
}

sal_Bool CanvasFactory::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

Sequence<OUString> CanvasFactory::getSupportedServiceNames()
{
    return { "com.sun.star.rendering.CanvasFactory" };
}

// XMultiComponentFactory
Sequence<OUString> CanvasFactory::getAvailableServiceNames()
{
    Sequence<OUString> aServiceNames(m_aAvailableImplementations.size());
    std::transform(m_aAvailableImplementations.begin(),
                   m_aAvailableImplementations.end(),
                   aServiceNames.getArray(),
                   o3tl::select1st< AvailPair >());
    return aServiceNames;
}

Reference<XInterface> CanvasFactory::createInstanceWithContext(
    OUString const & name, Reference<XComponentContext> const & xContext )
{
    return createInstanceWithArgumentsAndContext(
        name, Sequence<Any>(), xContext );
}


Reference<XInterface> CanvasFactory::use(
    OUString const & serviceName,
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext ) const
{
    try {
        return m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            serviceName, args, xContext);
    }
    catch (css::lang::IllegalArgumentException &)
    {
        return Reference<XInterface>();
    }
    catch (const RuntimeException &)
    {
        throw;
    }
    catch (const Exception &)
    {
        return Reference<XInterface>();
    }
}


void CanvasFactory::checkConfigFlag( bool& r_bFlag,
                                     bool& r_CacheFlag,
                                     const OUString& nodeName ) const
{
    if( m_xCanvasConfigNameAccess.is() )
    {
        m_xCanvasConfigNameAccess->getByName( nodeName ) >>= r_bFlag;

        if( r_CacheFlag != r_bFlag )
        {
            // cache is invalid, because of different order of
            // elements
            r_CacheFlag = r_bFlag;
            m_aCachedImplementations.clear();
        }
    }
}


Reference<XInterface> CanvasFactory::lookupAndUse(
    OUString const & serviceName, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext ) const
{
    ::osl::MutexGuard guard(m_mutex);

    // forcing last entry from impl list, if config flag set
    bool bForceLastEntry(false);
    checkConfigFlag( bForceLastEntry,
                     m_bCacheHasForcedLastImpl,
                     "ForceSafeServiceImpl" );

    // tdf#93870 - force VCL canvas in OpenGL mode for now.
#if HAVE_FEATURE_OPENGL
    if( OpenGLWrapper::isVCLOpenGLEnabled() )
        bForceLastEntry = true;
#endif

    // use anti-aliasing canvas, if config flag set (or not existing)
    bool bUseAAEntry(true);
    checkConfigFlag( bUseAAEntry,
                     m_bCacheHasUseAAEntry,
                     "UseAntialiasingCanvas" );

    // use accelerated canvas, if config flag set (or not existing)
    bool bUseAcceleratedEntry(true);
    checkConfigFlag( bUseAcceleratedEntry,
                     m_bCacheHasUseAcceleratedEntry,
                     "UseAcceleratedCanvas" );

    // try to reuse last working implementation for given service name
    const CacheVector::iterator aEnd(m_aCachedImplementations.end());
    CacheVector::iterator aMatch;
    if( (aMatch=std::find_if(
                    m_aCachedImplementations.begin(),
                    aEnd,
                    [&serviceName](CachePair const& cp)
                    { return serviceName.equals(cp.first); }
                    )) != aEnd) {
        Reference<XInterface> xCanvas( use( aMatch->second, args, xContext ) );
        if(xCanvas.is())
            return xCanvas;
    }

    // lookup in available service list
    const AvailVector::const_iterator aAvailEnd(m_aAvailableImplementations.end());
    AvailVector::const_iterator aAvailImplsMatch;
    if( (aAvailImplsMatch=std::find_if(
                    m_aAvailableImplementations.begin(),
                    aAvailEnd,
                    [&serviceName](AvailPair const& ap)
                    { return serviceName.equals(ap.first); }
                    )) == aAvailEnd ) {
        return Reference<XInterface>();
    }

    const AvailVector::const_iterator aAAEnd(m_aAAImplementations.end());
    AvailVector::const_iterator aAAImplsMatch;
    if( (aAAImplsMatch=std::find_if(
                    m_aAAImplementations.begin(),
                    aAAEnd,
                    [&serviceName](AvailPair const& ap)
                    { return serviceName.equals(ap.first); }
                    )) == aAAEnd) {
        return Reference<XInterface>();
    }

    const AvailVector::const_iterator aAccelEnd(m_aAcceleratedImplementations.end());
    AvailVector::const_iterator aAccelImplsMatch;
    if( (aAccelImplsMatch=std::find_if(
                    m_aAcceleratedImplementations.begin(),
                    aAccelEnd,
                    [&serviceName](AvailPair const& ap)
                    { return serviceName.equals(ap.first); }
                    )) == aAccelEnd ) {
        return Reference<XInterface>();
    }

    const Sequence<OUString> aPreferredImpls( aAvailImplsMatch->second );
    const OUString* pCurrImpl = aPreferredImpls.getConstArray();
    const OUString* const pEndImpl = pCurrImpl + aPreferredImpls.getLength();

    const Sequence<OUString> aAAImpls( aAAImplsMatch->second );
    const OUString* const pFirstAAImpl = aAAImpls.getConstArray();
    const OUString* const pEndAAImpl = pFirstAAImpl + aAAImpls.getLength();

    const Sequence<OUString> aAccelImpls( aAccelImplsMatch->second );
    const OUString* const pFirstAccelImpl = aAccelImpls.getConstArray();
    const OUString* const pEndAccelImpl = pFirstAccelImpl + aAccelImpls.getLength();

    // force last entry from impl list, if config flag set
    if( bForceLastEntry )
        pCurrImpl = pEndImpl-1;

    while( pCurrImpl != pEndImpl )
    {
        const OUString aCurrName(pCurrImpl->trim());

        // check whether given canvas service is listed in the
        // sequence of "accelerated canvas implementations"
        const bool bIsAcceleratedImpl(
            std::any_of(pFirstAccelImpl,
                         pEndAccelImpl,
                         [&aCurrName](OUString const& src)
                         { return aCurrName.equals(src.trim()); }
                ));

        // check whether given canvas service is listed in the
        // sequence of "antialiasing canvas implementations"
        const bool bIsAAImpl(
            std::any_of(pFirstAAImpl,
                         pEndAAImpl,
                         [&aCurrName](OUString const& src)
                         { return aCurrName.equals(src.trim()); }
                ));

        // try to instantiate canvas *only* if either accel and AA
        // property match preference, *or*, if there's a mismatch, only
        // go for a less capable canvas (that effectively let those
        // pour canvas impls still work as fallbacks, should an
        // accelerated/AA one fail). Property implies configuration:
        // http://en.wikipedia.org/wiki/Truth_table#Logical_implication
        if( (!bIsAAImpl || bUseAAEntry) && (!bIsAcceleratedImpl || bUseAcceleratedEntry) )
        {
            Reference<XInterface> xCanvas(
                use( pCurrImpl->trim(), args, xContext ) );

            if(xCanvas.is())
            {
                if( aMatch != aEnd )
                {
                    // cache entry exists, replace dysfunctional
                    // implementation name
                    aMatch->second = pCurrImpl->trim();
                }
                else
                {
                    // new service name, add new cache entry
                    m_aCachedImplementations.push_back(std::make_pair(serviceName,
                                                                      pCurrImpl->trim()));
                }

                return xCanvas;
            }
        }

        ++pCurrImpl;
    }

    return Reference<XInterface>();
}


Reference<XInterface> CanvasFactory::createInstanceWithArgumentsAndContext(
    OUString const & preferredOne, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext )
{
    Reference<XInterface> xCanvas(
        lookupAndUse( preferredOne, args, xContext ) );
    if(xCanvas.is())
        return xCanvas;

    // last resort: try service name directly
    return use( preferredOne, args, xContext );
}

// XMultiServiceFactory

Reference<XInterface> CanvasFactory::createInstance( OUString const & name )
{
    return createInstanceWithArgumentsAndContext(
        name, Sequence<Any>(), m_xContext );
}


Reference<XInterface> CanvasFactory::createInstanceWithArguments(
    OUString const & name, Sequence<Any> const & args )
{
    return createInstanceWithArgumentsAndContext(
        name, args, m_xContext );
}

} // anon namespace


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_rendering_CanvasFactory_get_implementation(css::uno::XComponentContext* context,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new CanvasFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
