/*************************************************************************
 *
 *  $RCSfile: dp_package.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 12:16:29 $
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

#include "dp_package.hrc"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "svtools/inettype.hxx"
#include "comphelper/anytostring.hxx"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include <vector>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry
{
namespace backend
{
namespace package
{

//==============================================================================
class BackendImpl : public PackageRegistryBackend
{
    OUString m_strPackageBundle;

protected:
    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

public:
    inline BackendImpl( Reference<XComponentContext> const & xComponentContext,
                        OUString const & implName,
                        Sequence<OUString> const & supportedMediaTypes )
        : PackageRegistryBackend(
            xComponentContext, implName, supportedMediaTypes ),
          m_strPackageBundle( getResourceString(RID_STR_PACKAGE_BUNDLE) )
        {}
};

//==============================================================================
class PackageImpl : public ::dp_registry::backend::Package
{
protected:
    OUString m_scanURL;
    bool m_legacyPackage;

    Sequence< Reference<deployment::XPackage> > m_bundle;
    bool m_bundleInit;

    typedef ::std::vector< Reference<deployment::XPackage> > t_packagevec;
    void scanBundle(
        t_packagevec & bundle,
        OUString const & url,
        ::rtl::Reference<AbortChannel> const & abortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv,
        bool skip_registration = false );

    // Package
    virtual bool isRegistered_(
        ::osl::ResettableMutexGuard & guard,
        ::rtl::Reference<AbortChannel> const & abortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv );
    virtual void processPackage_(
        ::osl::ResettableMutexGuard & guard,
        bool registerPackage,
        ::rtl::Reference<AbortChannel> const & abortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv );

    virtual void SAL_CALL disposing();

    inline BackendImpl * getMyBackend() const
        { return static_cast<BackendImpl *>(m_myBackend.get()); }

public:
    PackageImpl( ::rtl::Reference<PackageRegistryBackend> const & myBackend,
                 OUString const & url,
                 OUString const & mediaType,
                 OUString const & name,
                 OUString const & description,
                 bool legacyPackage );

    // XPackage
    virtual sal_Bool SAL_CALL isBundle() throw (RuntimeException);
    virtual Sequence< Reference<deployment::XPackage> > SAL_CALL getBundle(
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException,
               CommandFailedException, CommandAbortedException,
               lang::IllegalArgumentException, RuntimeException);
    virtual Any SAL_CALL getIcon( sal_Bool highContrast, sal_Bool smallIcon )
        throw (RuntimeException);
};

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR(
        "com.sun.star.comp.deployment.package.PackageRegistryBackend");
}

//==============================================================================
Reference<XInterface> SAL_CALL create(
    Reference<XComponentContext> const & xComponentContext )
    SAL_THROW( (Exception) )
{
    OUString const mediaTypes [] = {
        OUSTR("application/vnd.sun.star.uno-package"),
        OUSTR("application/vnd.sun.star.uno-legacy-package")
    };
    return static_cast< ::cppu::OWeakObject * >(
        new BackendImpl( xComponentContext, getImplementationName(),
                         Sequence<OUString>(
                             mediaTypes, ARLEN(mediaTypes) ) ) );
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.getLength() == 0)
    {
        // detect media-type:
        ::ucb::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ))
        {
            OUString title( extract_throw<OUString>(
                                ucbContent.getPropertyValue(
                                    OUSTR("Title") ) ) );
            if (title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".uno.pkg") ))
                mediaType = OUSTR("application/vnd.sun.star.uno-package");
            else if (title.endsWithIgnoreAsciiCaseAsciiL(
                         RTL_CONSTASCII_STRINGPARAM(".zip") ))
                mediaType =
                    OUSTR("application/vnd.sun.star.uno-legacy-package");
        }
        if (mediaType.getLength() == 0)
            throw lang::IllegalArgumentException(
                m_strCannotDetectMediaType + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.EqualsIgnoreCaseAscii("application"))
        {
            ::ucb::Content ucbContent( url, xCmdEnv );
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.uno-package"))
            {
                return new PackageImpl(
                    this, url, mediaType,
                    extract_throw<OUString>(
                        ucbContent.getPropertyValue( OUSTR("Title") ) ),
                    m_strPackageBundle, false );
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.uno-legacy-package"))
            {
                return new PackageImpl(
                    this, url, mediaType,
                    extract_throw<OUString>(
                        ucbContent.getPropertyValue( OUSTR("Title") ) ),
                    m_strPackageBundle, true );
            }
        }
    }
    throw lang::IllegalArgumentException(
        m_strUnsupportedMediaType + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//##############################################################################

//______________________________________________________________________________
PackageImpl::PackageImpl(
    ::rtl::Reference<PackageRegistryBackend> const & myBackend,
    OUString const & url,
    OUString const & mediaType,
    OUString const & name,
    OUString const & description,
    bool legacyPackage )
    : Package( myBackend, url, mediaType,
               name, name /* display-name */, description ),
      m_legacyPackage( legacyPackage ),
      m_bundleInit( false )
{
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://") );
    buf.append( ::rtl::Uri::encode( url, rtl_UriCharClassRegName,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 ) );
    buf.append( static_cast<sal_Unicode>('/') );
    m_scanURL = buf.makeStringAndClear();
}

//______________________________________________________________________________
void PackageImpl::disposing()
{
    sal_Int32 len = m_bundle.getLength();
    Reference<deployment::XPackage> const * p = m_bundle.getConstArray();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
        try_dispose( p[ pos ] );
    m_bundle.realloc( 0 );

    Package::disposing();
}

// Package
//______________________________________________________________________________
bool PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    Sequence< Reference<deployment::XPackage> > bundle(
        getBundle( abortChannel.get(), xCmdEnv ) );
    if (bundle.getLength() == 0)
        throw beans::UnknownPropertyException(
            OUSTR("registration cannot bet determined: ") + m_url,
            static_cast<OWeakObject *>(this) );

    Reference<deployment::XPackage> const * pbundle = bundle.getConstArray();
    bool ret = true;
    bool init = false;
    for ( sal_Int32 pos = bundle.getLength(); pos--; )
    {
        Reference<deployment::XPackage> const & xPackage = pbundle[ pos ];
        Reference<task::XAbortChannel> xSubAbortChannel(
            xPackage->createAbortChannel() );
        AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
        bool b = xPackage->isRegistered( xSubAbortChannel, xCmdEnv );
        if (init)
        {
            if (ret != b)
                throw beans::UnknownPropertyException(
                    OUSTR("registration status is ambiguous: ") + m_url,
                    static_cast<OWeakObject *>(this) );
        }
        else
        {
            ret = b;
            init = true;
        }
    }
    return ret;
}

//______________________________________________________________________________
void PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard & guard,
    bool registerPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    Sequence< Reference<deployment::XPackage> > bundle(
        getBundle( abortChannel.get(), xCmdEnv ) );
    Reference<deployment::XPackage> const * pbundle = bundle.getConstArray();

    if (registerPackage)
    {
        sal_Int32 len = bundle.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            Reference<deployment::XPackage> const & xPackage = pbundle[ pos ];
            Reference<task::XAbortChannel> xSubAbortChannel(
                xPackage->createAbortChannel() );
            AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
            xPackage->registerPackage( xSubAbortChannel, xCmdEnv );
        }
    }
    else
    {
        // revoke in reverse order:
        for ( sal_Int32 pos = bundle.getLength(); pos--; )
        {
            Reference<deployment::XPackage> const & xPackage = pbundle[ pos ];
            Reference<task::XAbortChannel> xSubAbortChannel(
                xPackage->createAbortChannel() );
            AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
            pbundle[ pos ]->revokePackage( xSubAbortChannel, xCmdEnv );
        }
    }
}

// XPackage
//______________________________________________________________________________
Any PackageImpl::getIcon( sal_Bool highContrast, sal_Bool smallIcon )
    throw (RuntimeException)
{
    OSL_ASSERT( smallIcon );
    if (smallIcon)
    {
        sal_uInt16 ret = highContrast
            ? RID_IMG_DEF_PACKAGE_BUNDLE_HC : RID_IMG_DEF_PACKAGE_BUNDLE;
        return makeAny(ret);
    }
    return Package::getIcon( highContrast, smallIcon );
}

//______________________________________________________________________________
sal_Bool PackageImpl::isBundle() throw (RuntimeException)
{
    return true;
}

//______________________________________________________________________________
Sequence< Reference<deployment::XPackage> > PackageImpl::getBundle(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    if (! m_bundleInit)
    {
        t_packagevec bundle;
        try
        {
            if (m_legacyPackage)
            {
                // legacy packages allow script.xlb, dialog.xlb in bundle
                // root folder:
                OUString mediaType;
                // probe for script.xlb:
                if (create_ucb_content(
                        0, make_url( m_scanURL, OUSTR("script.xlb") ),
                        xCmdEnv, false /* no throw */ ))
                    mediaType = OUSTR("application/vnd.sun.star."
                                      "basic-script");
                // probe for dialog.xlb:
                else if (create_ucb_content(
                             0, make_url( m_scanURL, OUSTR("dialog.xlb") ),
                             xCmdEnv, false /* no throw */ ))
                    mediaType = OUSTR("application/vnd.sun.star."
                                      "dialog-library");
                if (mediaType.getLength() > 0)
                {
                    Reference<deployment::XPackage> xPackage(
                        getMyBackend()->getRootRegistry()->bindPackage(
                            m_scanURL, mediaType, xCmdEnv ) );
                    OSL_ASSERT( xPackage.is() );
                    bundle.push_back( xPackage );
                }
            }
            scanBundle( bundle, m_scanURL,
                        AbortChannel::get(xAbortChannel), xCmdEnv );
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (CommandFailedException &)
        {
            throw;
        }
        catch (CommandAbortedException &)
        {
            throw;
        }
        catch (deployment::DeploymentException &)
        {
            throw;
        }
        catch (Exception &)
        {
            Any exc( ::cppu::getCaughtException() );
            throw deployment::DeploymentException(
                OUSTR("error scanning bundle: ") + m_url,
                static_cast<OWeakObject *>(this), exc );
        }

        // sort: schema before config data, typelibs before components:
        Sequence< Reference<deployment::XPackage> > ret( bundle.size() );
        Reference<deployment::XPackage> * pret = ret.getArray();
        sal_Int32 lower_end = 0;
        sal_Int32 upper_end = ret.getLength();
        t_packagevec::const_iterator iPos( bundle.begin() );
        t_packagevec::const_iterator const iEnd( bundle.end() );
        for ( ; iPos != iEnd; ++iPos )
        {
            OUString mediaType( (*iPos)->getMediaType() );
            String type, subType;
            INetContentTypeParameterList params;
            if (INetContentTypes::parse( mediaType, type, subType, &params ) &&
                type.EqualsIgnoreCaseAscii("application") &&
                (subType.EqualsIgnoreCaseAscii(
                    "vnd.sun.star.uno-component") ||
                 subType.EqualsIgnoreCaseAscii(
                     "application/vnd.sun.star.configuration-data")))
            {
                --upper_end;
                pret[ upper_end ] = *iPos;
            }
            else
            {
                pret[ lower_end ] = *iPos;
                ++lower_end;
            }
        }
        OSL_ASSERT( lower_end == upper_end );

        ::osl::MutexGuard guard( getMutex() );
        if (! m_bundleInit)
        {
            m_bundle = ret;
            m_bundleInit = true;
        }
    }
    return m_bundle;
}

//______________________________________________________________________________
void PackageImpl::scanBundle(
    t_packagevec & bundle,
    OUString const & url,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool skip_registration )
{
    ::ucb::Content ucbContent( url, xCmdEnv );

    if (m_legacyPackage)
    {
        // check for platform pathes:
        OUString title( extract_throw<OUString>(
                            ucbContent.getPropertyValue( OUSTR("Title") ) ) );
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".plt") ))
        {
            if (! platform_fits( title.copy( 0, title.getLength() - 4 ) ))
                return;
        }

        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM("skip_registration") ))
            skip_registration = true;
    }

    OUString ar [] = { OUSTR("Title"), OUSTR("IsFolder") };
    Reference<sdbc::XResultSet> xResultSet(
        ucbContent.createCursor( Sequence<OUString>( ar, ARLEN(ar) ),
                                  ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
    while (xResultSet->next())
    {
        checkAborted( abortChannel );

        Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
        OUString title( xRow->getString( 1 /* Title */ ) );
        OUString path( make_url( url, ::rtl::Uri::encode(
                                     title, rtl_UriCharClassPchar,
                                     rtl_UriEncodeIgnoreEscapes,
                                     RTL_TEXTENCODING_UTF8 ) ) );
        if (m_legacyPackage)
        {
            OUString mediaType;
            try
            {
                Reference<deployment::XPackage> xPackage(
                    getMyBackend()->getRootRegistry()->bindPackage(
                        path, OUString(), xCmdEnv ) );
                OSL_ASSERT( xPackage.is() );
                mediaType = xPackage->getMediaType();

                if (skip_registration &&
                    // xxx todo: additional media-type parsing?
                    mediaType.matchIgnoreAsciiCaseAsciiL(
                        RTL_CONSTASCII_STRINGPARAM(
                            "application/vnd.sun.star.uno-component") ))
                    continue;

                bundle.push_back( xPackage );
            }
            catch (lang::IllegalArgumentException &)
            {
            }
            if (mediaType.getLength() == 0 ||
                // script.xlb, dialog.xlb can be met everywhere:
                mediaType.matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(
                        "application/vnd.sun.star.basic-script") ) ||
                mediaType.matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(
                        "application/vnd.sun.star.dialog-library") ))
            {
                if (xRow->getBoolean( 2 /* IsFolder */ ))
                {
                    // recurse into folder:
                    scanBundle( bundle, path,
                                abortChannel, xCmdEnv, skip_registration );
                }
            }
        }
        else
        {
            OUString mediaType;
            Reference<XContentAccess> xContentAccess( xRow, UNO_QUERY_THROW );
            ::ucb::Content item( xContentAccess->queryContent(), xCmdEnv );
            try
            {
                item.getPropertyValue( OUSTR("MediaType") ) >>= mediaType;
            }
            catch (beans::UnknownPropertyException &)
            {
            }
            if (mediaType.getLength() > 0)
            {
                String type, subType;
                INetContentTypeParameterList params;
                if (INetContentTypes::parse(
                        mediaType, type, subType, &params ))
                {
                    INetContentTypeParameter const * param = params.find(
                        ByteString("platform") );
                    if (param != 0 && !platform_fits( param->m_sValue ))
                        continue;
                }

                Reference<deployment::XPackage> xPackage(
                    getMyBackend()->getRootRegistry()->bindPackage(
                        path, mediaType, xCmdEnv ) );
                OSL_ASSERT( xPackage.is() );
                bundle.push_back( xPackage );
            }
            else
            {
                if (xRow->getBoolean( 2 /* IsFolder */ ))
                {
                    // recurse into folder:
                    scanBundle( bundle, path,
                                abortChannel, xCmdEnv, skip_registration );
                }
                else
                {
                    // ignore this item:
                    // xxx todo: discuss whether it is sensible to ignore files
                    // within .uno.pkg packages that don't have a media-type
                    // entry
                }
            }
        }
    }
}

} // namespace package
} // namespace backend
} // namespace dp_registry

