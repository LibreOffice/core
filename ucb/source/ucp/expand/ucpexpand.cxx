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


#include "rtl/uri.hxx"
#include "osl/mutex.hxx"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"
#include "com/sun/star/ucb/XContentProvider.hpp"

#define EXPAND_PROTOCOL "vnd.sun.star.expand"
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ARLEN(x) sizeof (x) / sizeof *(x)


using namespace ::com::sun::star;
using ::rtl::OUString;

namespace
{

struct MutexHolder
{
    mutable ::osl::Mutex m_mutex;
};

typedef ::cppu::WeakComponentImplHelper2<
    lang::XServiceInfo, ucb::XContentProvider > t_impl_helper;

//==============================================================================
class ExpandContentProviderImpl : protected MutexHolder, public t_impl_helper
{
    uno::Reference< util::XMacroExpander > m_xMacroExpander;
    OUString expandUri(
        uno::Reference< ucb::XContentIdentifier > const & xIdentifier ) const;

protected:
    inline void check() const;
    virtual void SAL_CALL disposing();

public:
    inline ExpandContentProviderImpl(
        uno::Reference< uno::XComponentContext > const & xComponentContext )
        : t_impl_helper( m_mutex ),
          m_xMacroExpander(
              xComponentContext->getValueByName(
                  OUSTR("/singletons/com.sun.star.util.theMacroExpander") ),
              uno::UNO_QUERY_THROW )
        {}
    virtual ~ExpandContentProviderImpl() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (uno::RuntimeException);
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);

    // XContentProvider
    virtual uno::Reference< ucb::XContent > SAL_CALL queryContent(
        uno::Reference< ucb::XContentIdentifier > const & xIdentifier )
        throw (ucb::IllegalIdentifierException, uno::RuntimeException);
    virtual sal_Int32 SAL_CALL compareContentIds(
        uno::Reference< ucb::XContentIdentifier > const & xId1,
        uno::Reference< ucb::XContentIdentifier > const & xId2 )
        throw (uno::RuntimeException);
};

//______________________________________________________________________________
inline void ExpandContentProviderImpl::check() const
{
    // xxx todo guard?
//     MutexGuard guard( m_mutex );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("expand content provider instance has "
                  "already been disposed!"),
            static_cast< OWeakObject * >(
                const_cast< ExpandContentProviderImpl * >(this) ) );
    }
}

//______________________________________________________________________________
ExpandContentProviderImpl::~ExpandContentProviderImpl() throw ()
{
}

//______________________________________________________________________________
void ExpandContentProviderImpl::disposing()
{
}

//==============================================================================
static uno::Reference< uno::XInterface > SAL_CALL create(
    uno::Reference< uno::XComponentContext > const & xComponentContext )
    SAL_THROW( (uno::Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new ExpandContentProviderImpl( xComponentContext ) );
}

//==============================================================================
static OUString SAL_CALL implName()
{
    return OUSTR("com.sun.star.comp.ucb.ExpandContentProvider");
}

//==============================================================================
static uno::Sequence< OUString > SAL_CALL supportedServices()
{
    OUString names [] = {
        OUSTR("com.sun.star.ucb.ExpandContentProvider"),
        OUSTR("com.sun.star.ucb.ContentProvider")
    };
    return uno::Sequence< OUString >( names, ARLEN(names) );
}

// XServiceInfo
//______________________________________________________________________________
OUString ExpandContentProviderImpl::getImplementationName()
    throw (uno::RuntimeException)
{
    check();
    return implName();
}

//______________________________________________________________________________
uno::Sequence< OUString > ExpandContentProviderImpl::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    check();
    return supportedServices();
}

//______________________________________________________________________________
sal_Bool ExpandContentProviderImpl::supportsService(
    OUString const & serviceName )
    throw (uno::RuntimeException)
{
//     check();
    uno::Sequence< OUString > supported_services( getSupportedServiceNames() );
    OUString const * ar = supported_services.getConstArray();
    for ( sal_Int32 pos = supported_services.getLength(); pos--; )
    {
        if (ar[ pos ].equals( serviceName ))
            return true;
    }
    return false;
}

//______________________________________________________________________________
OUString ExpandContentProviderImpl::expandUri(
    uno::Reference< ucb::XContentIdentifier > const & xIdentifier ) const
{
    OUString uri( xIdentifier->getContentIdentifier() );
    if (uri.compareToAscii(
            RTL_CONSTASCII_STRINGPARAM(EXPAND_PROTOCOL ":") ) != 0)
    {
        throw ucb::IllegalIdentifierException(
            OUSTR("expected protocol " EXPAND_PROTOCOL "!"),
            static_cast< OWeakObject * >(
                const_cast< ExpandContentProviderImpl * >(this) ) );
    }
    // cut protocol
    OUString str( uri.copy( sizeof (EXPAND_PROTOCOL ":") -1 ) );
    // decode uric class chars
    str = ::rtl::Uri::decode(
        str, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
    // expand macro string
    return m_xMacroExpander->expandMacros( str );
}

// XContentProvider
//______________________________________________________________________________
uno::Reference< ucb::XContent > ExpandContentProviderImpl::queryContent(
    uno::Reference< ucb::XContentIdentifier > const & xIdentifier )
    throw (ucb::IllegalIdentifierException, uno::RuntimeException)
{
    check();
    OUString uri( expandUri( xIdentifier ) );

    ::ucbhelper::Content ucb_content;
    if (::ucbhelper::Content::create(
            uri, uno::Reference< ucb::XCommandEnvironment >(), ucb_content ))
    {
        return ucb_content.get();
    }
    else
    {
        return uno::Reference< ucb::XContent >();
    }
}

//______________________________________________________________________________
sal_Int32 ExpandContentProviderImpl::compareContentIds(
    uno::Reference< ucb::XContentIdentifier > const & xId1,
    uno::Reference< ucb::XContentIdentifier > const & xId2 )
    throw (uno::RuntimeException)
{
    check();
    try
    {
        OUString uri1( expandUri( xId1 ) );
        OUString uri2( expandUri( xId2 ) );
        return uri1.compareTo( uri2 );
    }
    catch (const ucb::IllegalIdentifierException & exc)
    {
        (void) exc; // unused
        OSL_FAIL(
            ::rtl::OUStringToOString(
                exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return -1;
    }
}

static const ::cppu::ImplementationEntry s_entries [] =
{
    {
        create,
        implName,
        supportedServices,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpexpand1_component_getFactory(
    const sal_Char * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, s_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
