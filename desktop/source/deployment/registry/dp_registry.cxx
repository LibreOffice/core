/*************************************************************************
 *
 *  $RCSfile: dp_registry.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:09:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_registry.hrc"
#include "dp_misc.h"
#include "dp_resource.h"
#include "dp_interact.h"
#include "dp_ucb.h"
#include "osl/diagnose.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/container/XContentEnumerationAccess.hpp"
#include "com/sun/star/deployment/PackageRegistryBackend.hpp"
#include <vector>
#include <hash_map>
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {

typedef ::cppu::WeakComponentImplHelper1<
    deployment::XPackageRegistry > t_helper;

//==============================================================================
class PackageRegistryImpl : private MutexHolder, public t_helper
{
    struct ci_string_hash
    {
        inline ::std::size_t operator () ( OUString const & str ) const {
            return str.toAsciiLowerCase().hashCode();
        }
    };
    struct ci_string_equals
    {
        inline bool operator () (
            OUString const & str1, OUString const & str2 ) const {
            return str1.equalsIgnoreAsciiCase( str2 );
        }
    };
    typedef ::std::hash_map<
        OUString, Reference<deployment::XPackageRegistry>,
        ci_string_hash, ci_string_equals > t_string2registry;
    t_string2registry m_media2backend;

    typedef ::std::vector<
        Reference<deployment::XPackageRegistry> > t_registryvec;
    t_registryvec m_backends;

    void insertBackend(
        Reference<deployment::XPackageRegistry> const & xBackend );

protected:
    inline void check();
    virtual void SAL_CALL disposing();

    virtual ~PackageRegistryImpl();
    PackageRegistryImpl() : t_helper( getMutex() ) {}
public:
    static Reference<deployment::XPackageRegistry> create(
        OUString const & context,
        OUString const & cachePath, bool readOnly,
        Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Reference<deployment::XPackage> SAL_CALL bindPackage(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException, CommandFailedException,
               lang::IllegalArgumentException, RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedMediaTypes()
        throw (RuntimeException);
};

//______________________________________________________________________________
inline void PackageRegistryImpl::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            OUSTR("PackageRegistry instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
    }
}

//______________________________________________________________________________
void PackageRegistryImpl::disposing()
{
    // dispose all registries:
    t_registryvec::const_iterator iPos( m_backends.begin() );
    t_registryvec::const_iterator const iEnd( m_backends.end() );
    for ( ; iPos != iEnd; ++iPos )
        try_dispose( *iPos );
    m_media2backend = t_string2registry();
    m_backends = t_registryvec();

    t_helper::disposing();
}

//______________________________________________________________________________
PackageRegistryImpl::~PackageRegistryImpl()
{
}

//______________________________________________________________________________
static OUString normalizeMediaType( OUString const & mediaType )
{
    ::rtl::OUStringBuffer buf;
    sal_Int32 index = 0;
    for (;;) {
        buf.append( mediaType.getToken( 0, '/', index ).trim() );
        if (index < 0)
            break;
        buf.append( static_cast< sal_Unicode >('/') );
    }
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
void PackageRegistryImpl::insertBackend(
    Reference<deployment::XPackageRegistry> const & xBackend )
{
    m_backends.push_back( xBackend );

    Sequence<OUString> mtypes( xBackend->getSupportedMediaTypes() );
    OUString const * pmtypes = mtypes.getConstArray();
    sal_Int32 len = mtypes.getLength();
    for ( sal_Int32 pos = 0; pos < len; ++pos ) {
        OUString mediaType( normalizeMediaType( pmtypes[ pos ] ) );
        ::std::pair<t_string2registry::iterator, bool> mr_insertion(
            m_media2backend.insert(
                t_string2registry::value_type(mediaType, xBackend) ) );
#if OSL_DEBUG_LEVEL > 0
        if (! mr_insertion.second) {
            ::rtl::OUStringBuffer buf;
            buf.appendAscii(
                RTL_CONSTASCII_STRINGPARAM(
                    "more than one PackageRegistryBackend for "
                    "media-type=\"") );
            buf.append( mediaType );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" => ") );
            buf.append( Reference<lang::XServiceInfo>(
                            xBackend, UNO_QUERY_THROW )->
                        getImplementationName() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            buf.makeStringAndClear(),
                            RTL_TEXTENCODING_UTF8 ) );
        }
#endif
    }
}

namespace backend {
namespace bundle {
Reference<deployment::XPackageRegistry> create(
    Reference<deployment::XPackageRegistry> const & xRootRegistry,
    OUString const & context, OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext );
}
}

//______________________________________________________________________________
Reference<deployment::XPackageRegistry> PackageRegistryImpl::create(
    OUString const & context,
    OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext )
{
    PackageRegistryImpl * that = new PackageRegistryImpl;
    Reference<deployment::XPackageRegistry> xRet(that);

    // auto-detect all registered package registries:
    Reference<container::XEnumeration> xEnum(
        Reference<container::XContentEnumerationAccess>(
            xComponentContext->getServiceManager(),
            UNO_QUERY_THROW )->createContentEnumeration(
                OUSTR("com.sun.star.deployment.PackageRegistryBackend") ) );
    if (xEnum.is())
    {
        while (xEnum->hasMoreElements())
        {
            Any element( xEnum->nextElement() );
            Sequence<Any> registryArgs(
                cachePath.getLength() == 0 ? 1 : 3 );
            registryArgs[ 0 ] <<= context;
            if (cachePath.getLength() > 0)
            {
                Reference<lang::XServiceInfo> xServiceInfo(
                    element, UNO_QUERY_THROW );
                OUString registryCachePath(
                    make_url( cachePath,
                              ::rtl::Uri::encode(
                                  xServiceInfo->getImplementationName(),
                                  rtl_UriCharClassPchar,
                                  rtl_UriEncodeIgnoreEscapes,
                                  RTL_TEXTENCODING_UTF8 ) ) );
                registryArgs[ 1 ] <<= registryCachePath;
                registryArgs[ 2 ] <<= readOnly;
                if (! readOnly)
                    create_folder( 0, registryCachePath,
                                   Reference<XCommandEnvironment>() );
            }

            Reference<deployment::XPackageRegistry> xBackend;
            Reference<lang::XSingleComponentFactory> xFac( element, UNO_QUERY );
            if (xFac.is()) {
                xBackend.set(
                    xFac->createInstanceWithArgumentsAndContext(
                        registryArgs, xComponentContext ), UNO_QUERY );
            }
            else {
                Reference<lang::XSingleServiceFactory> xSingleServiceFac(
                    element, UNO_QUERY_THROW );
                xBackend.set(
                    xSingleServiceFac->createInstanceWithArguments(
                        registryArgs ), UNO_QUERY );
            }
            if (! xBackend.is()) {
                throw DeploymentException(
                    OUSTR("cannot instantiate PackageRegistryBackend service: ")
                    + Reference<lang::XServiceInfo>(
                        element, UNO_QUERY_THROW )->getImplementationName(),
                    static_cast<OWeakObject *>(that) );
            }

            that->insertBackend( xBackend );
        }
    }

    // insert bundle be:
    that->insertBackend(
        ::dp_registry::backend::bundle::create(
            that, context, cachePath, readOnly, xComponentContext ) );

    return xRet;
}

// XPackageRegistry
//______________________________________________________________________________
Reference<deployment::XPackage> PackageRegistryImpl::bindPackage(
    OUString const & url, OUString const & mediaType,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException, CommandFailedException,
           lang::IllegalArgumentException, RuntimeException)
{
    check();
    if (mediaType.getLength() == 0)
    {
        // iterate all backends:
        t_registryvec::const_iterator iPos( m_backends.begin() );
        t_registryvec::const_iterator const iEnd( m_backends.end() );
        for ( ; iPos != iEnd; ++iPos )
        {
            try {
                return (*iPos)->bindPackage( url, mediaType, xCmdEnv );
            }
            catch (lang::IllegalArgumentException &) {
            }
        }
        throw lang::IllegalArgumentException(
            getResourceString(RID_STR_CANNOT_DETECT_MEDIA_TYPE) + url,
            static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }
    else
    {
        // get backend:
        t_string2registry::const_iterator iFind(
            m_media2backend.find( normalizeMediaType(mediaType) ) );
        if (iFind == m_media2backend.end())
        {
            // xxx todo: more sophisticated media-type argument parsing...
            sal_Int32 q = mediaType.indexOf( ';' );
            if (q >= 0) {
                iFind = m_media2backend.find(
                    normalizeMediaType(
                        /* cut parameters: */ mediaType.copy( 0, q ) ) );
            }
        }
        if (iFind == m_media2backend.end())
            throw lang::IllegalArgumentException(
                getResourceString(RID_STR_UNSUPPORTED_MEDIA_TYPE) + mediaType,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );

        return iFind->second->bindPackage( url, mediaType, xCmdEnv );
    }
}

//______________________________________________________________________________
Sequence<OUString> PackageRegistryImpl::getSupportedMediaTypes()
    throw (RuntimeException)
{
    throw RuntimeException( OUSTR("unexpected!"), 0 );
}

//==============================================================================
Reference<deployment::XPackageRegistry> SAL_CALL create(
    OUString const & context,
    OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext )
{
    return PackageRegistryImpl::create(
        context, cachePath, readOnly, xComponentContext );
}

}

