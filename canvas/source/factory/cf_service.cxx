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
#include <sal/log.hxx>

#include <algorithm>
#include <mutex>
#include <utility>
#include <vector>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/functional.hxx>
#include <o3tl/string_view.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <comphelper/configuration.hxx>
#include <officecfg/Office/Canvas.hxx>

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


    mutable std::mutex                m_mutex;
    Reference<XComponentContext>      m_xContext;
    AvailVector                       m_aAvailableImplementations;
    AvailVector                       m_aAcceleratedImplementations;
    AvailVector                       m_aAAImplementations;
    mutable CacheVector               m_aCachedImplementations;
    mutable bool                      m_bCacheHasForcedLastImpl;
    mutable bool                      m_bCacheHasUseAcceleratedEntry;
    mutable bool                      m_bCacheHasUseAAEntry;

    void checkConfigFlag( bool& r_bFlag,
                          bool& r_CacheFlag,
                          bool bCurrentConfigValue ) const;
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
    m_xContext(xContext),
    m_bCacheHasForcedLastImpl(),
    m_bCacheHasUseAcceleratedEntry(),
    m_bCacheHasUseAAEntry()
{
    if (!comphelper::IsFuzzing())
    {
        try
        {
            // read out configuration for preferred services:

            Reference<container::XNameAccess> xNameAccess = officecfg::Office::Canvas::CanvasServiceList::get();
            Reference<container::XHierarchicalNameAccess> xHierarchicalNameAccess(
                xNameAccess, UNO_QUERY_THROW);


            for (auto& serviceName : xNameAccess->getElementNames())
            {
                Reference<container::XNameAccess> xEntryNameAccess(
                    xHierarchicalNameAccess->getByHierarchicalName(serviceName),
                    UNO_QUERY );

                if( xEntryNameAccess.is() )
                {
                    Sequence<OUString> implementationList;
                    if( xEntryNameAccess->getByName(u"PreferredImplementations"_ustr) >>= implementationList )
                    {
                        m_aAvailableImplementations.emplace_back(serviceName, implementationList);
                    }
                    if( xEntryNameAccess->getByName(u"AcceleratedImplementations"_ustr) >>= implementationList )
                    {
                        m_aAcceleratedImplementations.emplace_back(serviceName, implementationList);
                    }
                    if( xEntryNameAccess->getByName(u"AntialiasingImplementations"_ustr) >>= implementationList )
                    {
                        m_aAAImplementations.emplace_back(serviceName, implementationList);
                    }
                }
            }
        }
        catch (const RuntimeException &)
        {
            throw;
        }
        catch (const Exception&)
        {
        }
    }

    if (m_aAvailableImplementations.empty())
    {
        // Ugh. Looks like configuration is borked. Fake minimal
        // setup.
        m_aAvailableImplementations.emplace_back(u"com.sun.star.rendering.Canvas"_ustr,
            Sequence<OUString>{ u"com.sun.star.comp.rendering.Canvas.VCL"_ustr } );

        m_aAvailableImplementations.emplace_back(u"com.sun.star.rendering.SpriteCanvas"_ustr,
            Sequence<OUString>{ u"com.sun.star.comp.rendering.SpriteCanvas.VCL"_ustr } );
    }
}

CanvasFactory::~CanvasFactory()
{
}


// XServiceInfo
OUString CanvasFactory::getImplementationName()
{
    return u"com.sun.star.comp.rendering.CanvasFactory"_ustr;
}

sal_Bool CanvasFactory::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

Sequence<OUString> CanvasFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.rendering.CanvasFactory"_ustr };
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
                                     bool bCurrentConfigValue ) const
{
    r_bFlag = bCurrentConfigValue;

    if( r_CacheFlag != r_bFlag )
    {
        // cache is invalid, because of different order of
        // elements
        r_CacheFlag = r_bFlag;
        m_aCachedImplementations.clear();
    }
}


Reference<XInterface> CanvasFactory::lookupAndUse(
    OUString const & serviceName, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext ) const
{
    std::scoped_lock guard(m_mutex);

    // forcing last entry from impl list, if config flag set
    bool bForceLastEntry(false);
    checkConfigFlag( bForceLastEntry,
                     m_bCacheHasForcedLastImpl,
                     officecfg::Office::Canvas::ForceSafeServiceImpl::get() );

    // use anti-aliasing canvas, if config flag set (or not existing)
    bool bUseAAEntry(true);
    checkConfigFlag( bUseAAEntry,
                     m_bCacheHasUseAAEntry,
                     officecfg::Office::Canvas::UseAntialiasingCanvas::get() );

    // use accelerated canvas, if config flag set (or not existing)
    bool bUseAcceleratedEntry(true);
    checkConfigFlag( bUseAcceleratedEntry,
                     m_bCacheHasUseAcceleratedEntry,
                     officecfg::Office::Canvas::UseAcceleratedCanvas::get() );

    // try to reuse last working implementation for given service name
    const CacheVector::iterator aEnd(m_aCachedImplementations.end());
    auto aMatch = std::find_if(
                    m_aCachedImplementations.begin(),
                    aEnd,
                    [&serviceName](CachePair const& cp)
                    { return serviceName == cp.first; }
                    );
    if( aMatch != aEnd ) {
        Reference<XInterface> xCanvas( use( aMatch->second, args, xContext ) );
        if(xCanvas.is())
            return xCanvas;
    }

    // lookup in available service list
    const AvailVector::const_iterator aAvailEnd(m_aAvailableImplementations.end());
    auto aAvailImplsMatch = std::find_if(
                    m_aAvailableImplementations.begin(),
                    aAvailEnd,
                    [&serviceName](AvailPair const& ap)
                    { return serviceName == ap.first; }
                    );
    if( aAvailImplsMatch == aAvailEnd ) {
        return Reference<XInterface>();
    }

    const AvailVector::const_iterator aAAEnd(m_aAAImplementations.end());
    auto aAAImplsMatch = std::find_if(
                    m_aAAImplementations.begin(),
                    aAAEnd,
                    [&serviceName](AvailPair const& ap)
                    { return serviceName == ap.first; }
                    );
    if( aAAImplsMatch == aAAEnd ) {
        return Reference<XInterface>();
    }

    const AvailVector::const_iterator aAccelEnd(m_aAcceleratedImplementations.end());
    auto aAccelImplsMatch = std::find_if(
                    m_aAcceleratedImplementations.begin(),
                    aAccelEnd,
                    [&serviceName](AvailPair const& ap)
                    { return serviceName == ap.first; }
                    );
    if( aAccelImplsMatch == aAccelEnd ) {
        return Reference<XInterface>();
    }

    const Sequence<OUString> aPreferredImpls( aAvailImplsMatch->second );
    const OUString* pCurrImpl = aPreferredImpls.begin();
    const OUString* const pEndImpl = aPreferredImpls.end();

    const Sequence<OUString> aAAImpls( aAAImplsMatch->second );

    const Sequence<OUString> aAccelImpls( aAccelImplsMatch->second );

    // force last entry from impl list, if config flag set
    if (bForceLastEntry && pCurrImpl != pEndImpl)
        pCurrImpl = pEndImpl-1;

    for(; pCurrImpl != pEndImpl; ++pCurrImpl)
    {
        const OUString aCurrName(pCurrImpl->trim());

        // Skia works only with vclcanvas.
        if( SkiaHelper::isVCLSkiaEnabled() && !aCurrName.endsWith(".VCL"))
            continue;

        // check whether given canvas service is listed in the
        // sequence of "accelerated canvas implementations"
        const bool bIsAcceleratedImpl(
            std::any_of(aAccelImpls.begin(), aAccelImpls.end(),
                         [&aCurrName](OUString const& src)
                         { return aCurrName == o3tl::trim(src); }
                ));

        // check whether given canvas service is listed in the
        // sequence of "antialiasing canvas implementations"
        const bool bIsAAImpl(
            std::any_of(aAAImpls.begin(), aAAImpls.end(),
                         [&aCurrName](OUString const& src)
                         { return aCurrName == o3tl::trim(src); }
                ));

        // try to instantiate canvas *only* if either accel and AA
        // property match preference, *or*, if there's a mismatch, only
        // go for a less capable canvas (that effectively let those
        // pour canvas impls still work as fallbacks, should an
        // accelerated/AA one fail). Property implies configuration:
        // http://en.wikipedia.org/wiki/Truth_table#Logical_implication
        if( (!bIsAAImpl || bUseAAEntry) && (!bIsAcceleratedImpl || bUseAcceleratedEntry) )
        {
            Reference<XInterface> xCanvas(use(aCurrName, args, xContext));

            if(xCanvas.is())
            {
                if( aMatch != aEnd )
                {
                    // cache entry exists, replace dysfunctional
                    // implementation name
                    aMatch->second = aCurrName;
                }
                else
                {
                    // new service name, add new cache entry
                    m_aCachedImplementations.emplace_back(serviceName, aCurrName);
                }

                return xCanvas;
            }
        }
    }

    return Reference<XInterface>();
}


Reference<XInterface> CanvasFactory::createInstanceWithArgumentsAndContext(
    OUString const & preferredOne, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext )
{
    Reference<XInterface> xCanvas(lookupAndUse(preferredOne, args, xContext));
    if (!xCanvas.is())
        // last resort: try service name directly
        xCanvas = use(preferredOne, args, xContext);

    if (xCanvas.is())
    {
        Reference<lang::XServiceName> xServiceName(xCanvas, uno::UNO_QUERY);
        SAL_INFO("canvas", "using " << (xServiceName.is() ? xServiceName->getServiceName()
                                                          : u"(unknown)"_ustr));
    }
    return xCanvas;
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


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_rendering_CanvasFactory_get_implementation(css::uno::XComponentContext* context,
                                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new CanvasFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
