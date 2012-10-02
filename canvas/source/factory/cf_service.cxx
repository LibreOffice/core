/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <osl/mutex.hxx>
#include <osl/process.h>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <boost/bind.hpp>
#include <vector>
#include <utility>
#include <o3tl/compat_functional.hxx>
#include <algorithm>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace
{

OUString SAL_CALL getImplName()
{
    return OUSTR("com.sun.star.comp.rendering.CanvasFactory");
}

Sequence<OUString> SAL_CALL getSuppServices()
{
    OUString name = OUSTR("com.sun.star.rendering.CanvasFactory");
    return Sequence<OUString>(&name, 1);
}

//==============================================================================
class CanvasFactory
    : public ::cppu::WeakImplHelper3< lang::XServiceInfo,
                                      lang::XMultiComponentFactory,
                                      lang::XMultiServiceFactory >
{
    typedef std::pair<OUString,Sequence<OUString> > AvailPair;
    typedef std::pair<OUString,OUString>            CachePair;
    typedef std::vector< AvailPair >                AvailVector;
    typedef std::vector< CachePair >                CacheVector;


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
    virtual ~CanvasFactory();
    CanvasFactory( Reference<XComponentContext> const & xContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XMultiComponentFactory
    virtual Sequence<OUString> SAL_CALL getAvailableServiceNames()
        throw (RuntimeException);
    virtual Reference<XInterface> SAL_CALL createInstanceWithContext(
        OUString const & name,
        Reference<XComponentContext> const & xContext ) throw (Exception);
    virtual Reference<XInterface> SAL_CALL
    createInstanceWithArgumentsAndContext(
        OUString const & name,
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xContext ) throw (Exception);

    // XMultiServiceFactory
    virtual Reference<XInterface> SAL_CALL createInstance(
        OUString const & name )
        throw (Exception);
    virtual Reference<XInterface> SAL_CALL createInstanceWithArguments(
        OUString const & name, Sequence<Any> const & args )
        throw (Exception);
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
            makeAny( beans::PropertyValue(
                         OUSTR("nodepath"), -1,
                         makeAny( OUSTR("/org.openoffice.Office.Canvas") ),
                         beans::PropertyState_DIRECT_VALUE ) ) );

        m_xCanvasConfigNameAccess.set(
            xConfigProvider->createInstanceWithArguments(
                OUSTR("com.sun.star.configuration.ConfigurationAccess"),
                Sequence<Any>( &propValue, 1 ) ),
            UNO_QUERY_THROW );

        propValue = makeAny(
            beans::PropertyValue(
                OUSTR("nodepath"), -1,
                makeAny( OUSTR("/org.openoffice.Office.Canvas/CanvasServiceList") ),
                beans::PropertyState_DIRECT_VALUE ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                OUSTR("com.sun.star.configuration.ConfigurationAccess"),
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
                if( (xEntryNameAccess->getByName( OUSTR("PreferredImplementations") ) >>= implementationList) )
                    m_aAvailableImplementations.push_back( std::make_pair(*pCurr,implementationList) );
                if( (xEntryNameAccess->getByName( OUSTR("AcceleratedImplementations") ) >>= implementationList) )
                    m_aAcceleratedImplementations.push_back( std::make_pair(*pCurr,implementationList) );
                if( (xEntryNameAccess->getByName( OUSTR("AntialiasingImplementations") ) >>= implementationList) )
                    m_aAAImplementations.push_back( std::make_pair(*pCurr,implementationList) );
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
        Sequence<OUString> aServices(1);
        aServices[0] = OUSTR("com.sun.star.comp.rendering.Canvas.VCL");
        m_aAvailableImplementations.push_back( std::make_pair(OUSTR("com.sun.star.rendering.Canvas"),
                                                              aServices) );

        aServices[0] = OUSTR("com.sun.star.comp.rendering.SpriteCanvas.VCL");
        m_aAvailableImplementations.push_back( std::make_pair(OUSTR("com.sun.star.rendering.SpriteCanvas"),
                                                              aServices) );
    }
}

CanvasFactory::~CanvasFactory()
{
}

//------------------------------------------------------------------------------
Reference<XInterface> create( Reference<XComponentContext> const & xContext )
{
    return static_cast< ::cppu::OWeakObject * >(
        new CanvasFactory( xContext ) );
}

// XServiceInfo
//______________________________________________________________________________
OUString CanvasFactory::getImplementationName() throw (RuntimeException)
{
    return getImplName();
}

//______________________________________________________________________________
sal_Bool CanvasFactory::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    return serviceName.equals(getSuppServices()[0]);
}

//______________________________________________________________________________
Sequence<OUString> CanvasFactory::getSupportedServiceNames()
    throw (RuntimeException)
{
    return getSuppServices();
}

// XMultiComponentFactory
//______________________________________________________________________________
Sequence<OUString> CanvasFactory::getAvailableServiceNames()
    throw (RuntimeException)
{
    Sequence<OUString> aServiceNames(m_aAvailableImplementations.size());
    std::transform(m_aAvailableImplementations.begin(),
                   m_aAvailableImplementations.end(),
                   aServiceNames.getArray(),
                   o3tl::select1st<AvailPair>());
    return aServiceNames;
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstanceWithContext(
    OUString const & name, Reference<XComponentContext> const & xContext )
    throw (Exception)
{
    return createInstanceWithArgumentsAndContext(
        name, Sequence<Any>(), xContext );
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::use(
    OUString const & serviceName,
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext ) const
{
    try {
        return m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            serviceName, args, xContext);
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

//______________________________________________________________________________
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

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::lookupAndUse(
    OUString const & serviceName, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext ) const
{
    ::osl::MutexGuard guard(m_mutex);

    // forcing last entry from impl list, if config flag set
    bool bForceLastEntry(false);
    checkConfigFlag( bForceLastEntry,
                     m_bCacheHasForcedLastImpl,
                     OUSTR("ForceSafeServiceImpl") );

    // use anti-aliasing canvas, if config flag set (or not existing)
    bool bUseAAEntry(true);
    checkConfigFlag( bUseAAEntry,
                     m_bCacheHasUseAAEntry,
                     OUSTR("UseAntialiasingCanvas") );

    // use accelerated canvas, if config flag set (or not existing)
    bool bUseAcceleratedEntry(true);
    checkConfigFlag( bUseAcceleratedEntry,
                     m_bCacheHasUseAcceleratedEntry,
                     OUSTR("UseAcceleratedCanvas") );

    // try to reuse last working implementation for given service name
    const CacheVector::iterator aEnd(m_aCachedImplementations.end());
    CacheVector::iterator aMatch;
    if( (aMatch=std::find_if(m_aCachedImplementations.begin(),
                             aEnd,
                             boost::bind(&OUString::equals,
                                         boost::cref(serviceName),
                                         boost::bind(
                                             o3tl::select1st<CachePair>(),
                                             _1)))) != aEnd )
    {
        Reference<XInterface> xCanvas( use( aMatch->second, args, xContext ) );
        if(xCanvas.is())
            return xCanvas;
    }

    // lookup in available service list
    const AvailVector::const_iterator aAvailEnd(m_aAvailableImplementations.end());
    AvailVector::const_iterator aAvailImplsMatch;
    if( (aAvailImplsMatch=std::find_if(m_aAvailableImplementations.begin(),
                                       aAvailEnd,
                                       boost::bind(&OUString::equals,
                                                   boost::cref(serviceName),
                                                   boost::bind(
                                                       o3tl::select1st<AvailPair>(),
                                                       _1)))) == aAvailEnd )
    {
        return Reference<XInterface>();
    }

    const AvailVector::const_iterator aAAEnd(m_aAAImplementations.end());
    AvailVector::const_iterator aAAImplsMatch;
    if( (aAAImplsMatch=std::find_if(m_aAAImplementations.begin(),
                                    aAAEnd,
                                    boost::bind(&OUString::equals,
                                                boost::cref(serviceName),
                                                boost::bind(
                                                    o3tl::select1st<AvailPair>(),
                                                    _1)))) == aAAEnd )
    {
        return Reference<XInterface>();
    }

    const AvailVector::const_iterator aAccelEnd(m_aAcceleratedImplementations.end());
    AvailVector::const_iterator aAccelImplsMatch;
    if( (aAccelImplsMatch=std::find_if(m_aAcceleratedImplementations.begin(),
                                       aAccelEnd,
                                       boost::bind(&OUString::equals,
                                                   boost::cref(serviceName),
                                                   boost::bind(
                                                       o3tl::select1st<AvailPair>(),
                                                       _1)))) == aAccelEnd )
    {
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
            std::find_if(pFirstAccelImpl,
                         pEndAccelImpl,
                         boost::bind(&OUString::equals,
                                     boost::cref(aCurrName),
                                     boost::bind(
                                         &OUString::trim,
                                         _1))) != pEndAccelImpl );

        // check whether given canvas service is listed in the
        // sequence of "antialiasing canvas implementations"
        const bool bIsAAImpl(
            std::find_if(pFirstAAImpl,
                         pEndAAImpl,
                         boost::bind(&OUString::equals,
                                     boost::cref(aCurrName),
                                     boost::bind(
                                         &OUString::trim,
                                         _1))) != pEndAAImpl );

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

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstanceWithArgumentsAndContext(
    OUString const & preferredOne, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext ) throw (Exception)
{
    Reference<XInterface> xCanvas(
        lookupAndUse( preferredOne, args, xContext ) );
    if(xCanvas.is())
        return xCanvas;

    // last resort: try service name directly
    return use( preferredOne, args, xContext );
}

// XMultiServiceFactory
//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstance( OUString const & name )
    throw (Exception)
{
    return createInstanceWithArgumentsAndContext(
        name, Sequence<Any>(), m_xContext );
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstanceWithArguments(
    OUString const & name, Sequence<Any> const & args ) throw (Exception)
{
    return createInstanceWithArgumentsAndContext(
        name, args, m_xContext );
}

const ::cppu::ImplementationEntry s_entries [] = {
    {
        create,
        getImplName,
        getSuppServices,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

} // anon namespace

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL canvasfactory_component_getFactory(
    sal_Char const * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, s_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
