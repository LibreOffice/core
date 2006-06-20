/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ucpexpand.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:18:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace
{

struct MutexHolder
{
    mutable ::osl::Mutex m_mutex;
};

typedef ::cppu::WeakComponentImplHelper2<
    lang::XServiceInfo, XContentProvider > t_impl_helper;

//==============================================================================
class ExpandContentProviderImpl : protected MutexHolder, public t_impl_helper
{
    Reference< util::XMacroExpander > m_xMacroExpander;
    OUString expandUri(
        Reference< XContentIdentifier > const & xIdentifier ) const;

protected:
    inline void check() const;
    virtual void SAL_CALL disposing();

public:
    inline ExpandContentProviderImpl(
        Reference< XComponentContext > const & xComponentContext )
        : t_impl_helper( m_mutex ),
          m_xMacroExpander(
              xComponentContext->getValueByName(
                  OUSTR("/singletons/com.sun.star.util.theMacroExpander") ),
              UNO_QUERY_THROW )
        {}
    virtual ~ExpandContentProviderImpl() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XContentProvider
    virtual Reference< XContent > SAL_CALL queryContent(
        Reference< XContentIdentifier > const & xIdentifier )
        throw (IllegalIdentifierException, RuntimeException);
    virtual sal_Int32 SAL_CALL compareContentIds(
        Reference< XContentIdentifier > const & xId1,
        Reference< XContentIdentifier > const & xId2 )
        throw (RuntimeException);
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
static Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
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
static Sequence< OUString > SAL_CALL supportedServices()
{
    OUString names [] = {
        OUSTR("com.sun.star.ucb.ExpandContentProvider"),
        OUSTR("com.sun.star.ucb.ContentProvider")
    };
    return Sequence< OUString >( names, ARLEN(names) );
}

// XServiceInfo
//______________________________________________________________________________
OUString ExpandContentProviderImpl::getImplementationName()
    throw (RuntimeException)
{
    check();
    return implName();
}

//______________________________________________________________________________
Sequence< OUString > ExpandContentProviderImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    check();
    return supportedServices();
}

//______________________________________________________________________________
sal_Bool ExpandContentProviderImpl::supportsService(
    OUString const & serviceName )
    throw (RuntimeException)
{
//     check();
    Sequence< OUString > supported_services( getSupportedServiceNames() );
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
    Reference< XContentIdentifier > const & xIdentifier ) const
{
    OUString uri( xIdentifier->getContentIdentifier() );
    if (uri.compareToAscii(
            RTL_CONSTASCII_STRINGPARAM(EXPAND_PROTOCOL ":") ) != 0)
    {
        throw IllegalIdentifierException(
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
Reference< XContent > ExpandContentProviderImpl::queryContent(
    Reference< XContentIdentifier > const & xIdentifier )
    throw (IllegalIdentifierException, RuntimeException)
{
    check();
    OUString uri( expandUri( xIdentifier ) );

    ::ucb::Content ucb_content;
    if (::ucb::Content::create(
            uri, Reference< XCommandEnvironment >(), ucb_content ))
    {
        return ucb_content.get();
    }
    else
    {
        return Reference< XContent >();
    }
}

//______________________________________________________________________________
sal_Int32 ExpandContentProviderImpl::compareContentIds(
    Reference< XContentIdentifier > const & xId1,
    Reference< XContentIdentifier > const & xId2 )
    throw (RuntimeException)
{
    check();
    try
    {
        OUString uri1( expandUri( xId1 ) );
        OUString uri2( expandUri( xId2 ) );
        return uri1.compareTo( uri2 );
    }
    catch (IllegalIdentifierException & exc)
    {
        (void) exc; // unused
        OSL_ENSURE(
            0, ::rtl::OUStringToOString(
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

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_writeInfoHelper(
        pServiceManager, pRegistryKey, s_entries );
}

void * SAL_CALL component_getFactory(
    const sal_Char * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, s_entries );
}

}
