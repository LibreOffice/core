/*************************************************************************
 *
 *  $RCSfile: dp_package.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:10:51 $
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
#include "dp_interact.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "svtools/inettype.hxx"
#include "comphelper/anytostring.hxx"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/io/XOutputStream.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/task/InteractionClassification.hpp"
#include "com/sun/star/ucb/XInteractionReplaceExistingData.hpp"
#include "com/sun/star/ucb/NameClashResolveRequest.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/UnsupportedCommandException.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/packages/manifest/XManifestReader.hpp"
#include "com/sun/star/packages/manifest/XManifestWriter.hpp"
#include <list>
#include <vector>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace bundle {

//==============================================================================
class BackendImpl : public PackageRegistryBackend
{
    OUString m_strPackageBundle;

protected:
    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

    virtual void SAL_CALL disposing();

public:
    inline BackendImpl( Sequence<Any> const & args,
                        Reference<XComponentContext> const & xComponentContext,
                        OUString const & implName,
                        Sequence<OUString> const & supportedMediaTypes,
                        Reference<deployment::XPackageRegistry>
                        const & xRootRegistry )
        : PackageRegistryBackend(
            args, xComponentContext, implName, supportedMediaTypes ),
          m_strPackageBundle( getResourceString(RID_STR_PACKAGE_BUNDLE) ),
          m_xRootRegistry( xRootRegistry )
        {}

    Reference<deployment::XPackageRegistry> m_xRootRegistry;
};

//______________________________________________________________________________
void BackendImpl::disposing()
{
    m_xRootRegistry.clear();
    PackageRegistryBackend::disposing();
}

//==============================================================================
class PackageImpl : public ::dp_registry::backend::Package
{
protected:
    OUString m_url_expanded;
    bool m_legacyBundle;
    typedef ::std::list< ::std::pair<OUString, OUString> > t_bundleInfos;
    t_bundleInfos m_bundleInfos;
    Sequence< Reference<deployment::XPackage> > m_bundle;
    bool m_bundleInit;

    typedef ::std::vector< Reference<deployment::XPackage> > t_packagevec;
    void scanLegacyBundle(
        t_packagevec & bundle,
        OUString const & url,
        ::rtl::Reference<AbortChannel> const & abortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv,
        bool skip_registration = false );

    // Package
    virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
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
                 bool legacyBundle,
                 Reference<XCommandEnvironment> const & xCmdEnv );

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
    virtual void SAL_CALL exportTo(
        OUString const & destFolderURL, OUString const & newTitle,
        sal_Int32 nameClashAction,
        Reference<XCommandEnvironment> const & xCmdEnv )
        throw (CommandFailedException, CommandAbortedException,
               RuntimeException);
};

//==============================================================================
Reference<deployment::XPackageRegistry> create(
    Reference<deployment::XPackageRegistry> const & xRootRegistry,
    OUString const & context, OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext )
{
    Sequence<Any> args(
        cachePath.getLength() == 0 ? 1 : 3 );
    args[ 0 ] <<= context;
    if (cachePath.getLength() > 0) {
        args[ 1 ] <<= cachePath;
        args[ 2 ] <<= readOnly;
    }

    OUString const mediaTypes [] = {
        OUSTR("application/vnd.sun.star.package-bundle"),
        OUSTR("application/vnd.sun.star.legacy-package-bundle")
    };

    return new BackendImpl( args, xComponentContext,
                            OUSTR("com.sun.star.comp.deployment."
                                  "bundle.PackageRegistryBackend"),
                            Sequence<OUString>(
                                mediaTypes, ARLEN(mediaTypes) ),
                            xRootRegistry );
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
                mediaType = OUSTR("application/vnd.sun.star.package-bundle");
            else if (title.endsWithIgnoreAsciiCaseAsciiL(
                         RTL_CONSTASCII_STRINGPARAM(".zip") ))
                mediaType =
                    OUSTR("application/vnd.sun.star.legacy-package-bundle");
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
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.package-bundle"))
                return new PackageImpl(
                    this, url, mediaType,
                    extract_throw<OUString>(
                        ucbContent.getPropertyValue( OUSTR("Title") ) ),
                    m_strPackageBundle, false, xCmdEnv );
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.legacy-package-bundle"))
                return new PackageImpl(
                    this, url, mediaType,
                    extract_throw<OUString>(
                        ucbContent.getPropertyValue( OUSTR("Title") ) ),
                    m_strPackageBundle, true, xCmdEnv );
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
    bool legacyBundle,
    Reference<XCommandEnvironment> const & xCmdEnv )
    : Package( myBackend, url, mediaType,
               name, name /* display-name */, description ),
      m_url_expanded( expand_url( url ) ),
      m_legacyBundle( legacyBundle ),
      m_bundleInit( false )
{
    if (! legacyBundle)
    {
        ::ucb::Content manifestContent;
        if (create_ucb_content(
                &manifestContent,
                make_url( m_url_expanded, OUSTR("META-INF/manifest.xml") ),
                xCmdEnv, false /* no throw */ ))
        {
            bool baseIsExpandURL = url.matchAsciiL(
                RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") );
            lang::Locale const & officeLocale = getOfficeLocale();
            OUString descrFile;
            lang::Locale descrFileLocale;

            Reference<XComponentContext> xContext(
                myBackend->getComponentContext() );
            Reference<packages::manifest::XManifestReader> xManifestReader(
                xContext->getServiceManager()->createInstanceWithContext(
                    OUSTR("com.sun.star.packages.manifest.ManifestReader"),
                    xContext ), UNO_QUERY_THROW );
            Sequence< Sequence<beans::PropertyValue> > manifestSeq(
                xManifestReader->readManifestSequence(
                    manifestContent.openStream() ) );
            Sequence<beans::PropertyValue> const * pmanifestSeq =
                manifestSeq.getConstArray();
            for ( sal_Int32 pos = manifestSeq.getLength(); pos--; )
            {
                OUString path, mediaType;
                beans::PropertyValue const * pattribs =
                    pmanifestSeq[ pos ].getConstArray();
                for ( sal_Int32 i = pmanifestSeq[ pos ].getLength(); i--; )
                {
                    if (path.getLength() > 0 && mediaType.getLength() > 0)
                        break;
                    if (pattribs[i].Name.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM("FullPath") ))
                        pattribs[i].Value >>= path;
                    else if (pattribs[i].Name.equalsAsciiL(
                                 RTL_CONSTASCII_STRINGPARAM("MediaType") ))
                        pattribs[i].Value >>= mediaType;
                }

                if (path.getLength() == 0 || mediaType.getLength() == 0 ||
                    mediaType.equalsAsciiL( // opt: exclude common text/xml
                        RTL_CONSTASCII_STRINGPARAM("text/xml") ))
                    continue;

                String type, subType;
                INetContentTypeParameterList params;
                if (! INetContentTypes::parse(
                        mediaType, type, subType, &params ))
                    continue;

                INetContentTypeParameter const * param = params.find(
                    ByteString("platform") );
                if (param != 0 && !platform_fits( param->m_sValue ))
                    continue;
                if (path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("/") ))
                    continue; // exclude root folder

                if (type.EqualsIgnoreCaseAscii("application") &&
                    subType.EqualsIgnoreCaseAscii(
                        "vnd.sun.star.package-bundle-description"))
                {
                    // check locale:
                    INetContentTypeParameter const * param =
                        params.find( ByteString("locale") );
                    if (param == 0) {
                        if (descrFile.getLength() == 0)
                            descrFile = path;
                    }
                    else
                    {
                        // match best locale:
                        lang::Locale locale( toLocale(param->m_sValue) );
                        if (locale.Language == officeLocale.Language)
                        {
                            if (descrFileLocale.Country == officeLocale.Country
                                && locale.Country != officeLocale.Country)
                                continue;
                            if (descrFileLocale.Variant == officeLocale.Variant
                                && locale.Variant != officeLocale.Variant)
                                continue;
                            descrFile = path;
                            descrFileLocale = locale;
                        }
                    }
                }

                if (baseIsExpandURL) {
                    // encode once more for vnd.sun.star.expand schema:
                    // vnd.sun.star.expand:$UNO_...
                    // will expand to file-url
                    path = ::rtl::Uri::encode(
                        path, rtl_UriCharClassUric,
                        rtl_UriEncodeIgnoreEscapes,
                        RTL_TEXTENCODING_UTF8 );
                }
                path = make_url( url, path );
                m_bundleInfos.push_back(
                    ::std::pair<OUString, OUString>(path, mediaType) );
            }

            if (descrFile.getLength() > 0)
            {
                ::ucb::Content descrFileContent;
                if (create_ucb_content( &descrFileContent,
                                        make_url(
                                            m_url_expanded, descrFile ),
                                        xCmdEnv, false /* no throw */ ))
                {
                    // patch description:
                    ::rtl::ByteSequence bytes( readFile( descrFileContent ) );
                    ::rtl::OUStringBuffer buf;
                    buf.append( OUString( reinterpret_cast<sal_Char const *>(
                                              bytes.getConstArray() ),
                                          bytes.getLength(),
                                          RTL_TEXTENCODING_UTF8 ) );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\n") );
                    buf.append( getDescription() );
                    m_description = buf.makeStringAndClear();
                }
            }
        }
    }
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
beans::Optional< beans::Ambiguous<sal_Bool> > PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard & guard,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    Sequence< Reference<deployment::XPackage> > bundle(
        getBundle( abortChannel.get(), xCmdEnv ) );
    Reference<deployment::XPackage> const * pbundle = bundle.getConstArray();
    bool reg = false;
    bool present = false;
    bool ambig = false;
    for ( sal_Int32 pos = bundle.getLength(); pos--; )
    {
        Reference<deployment::XPackage> const & xPackage = pbundle[ pos ];
        Reference<task::XAbortChannel> xSubAbortChannel(
            xPackage->createAbortChannel() );
        AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
        beans::Optional< beans::Ambiguous<sal_Bool> > option(
            xPackage->isRegistered( xSubAbortChannel, xCmdEnv ) );
        if (option.IsPresent)
        {
            beans::Ambiguous<sal_Bool> const & status = option.Value;
            if (present)
            {
                if (reg != (status.Value != sal_False)) {
                    ambig = true;
                    reg = false;
                    break;
                }
            }
            else
            {
                reg = status.Value;
                present = true;
            }
        }
    }
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        present, beans::Ambiguous<sal_Bool>(reg, ambig) );
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
        for ( sal_Int32 pos = 0; pos < len; ++pos ) {
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
        for ( sal_Int32 pos = bundle.getLength(); pos--; ) {
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
    if (smallIcon) {
        sal_uInt16 ret = highContrast
            ? RID_IMG_DEF_PACKAGE_BUNDLE_HC : RID_IMG_DEF_PACKAGE_BUNDLE;
        return makeAny(ret);
    }
    return Package::getIcon( highContrast, smallIcon );
}

//______________________________________________________________________________
void PackageImpl::exportTo(
    OUString const & destFolderURL, OUString const & newTitle,
    sal_Int32 nameClashAction, Reference<XCommandEnvironment> const & xCmdEnv )
    throw (CommandFailedException, CommandAbortedException, RuntimeException)
{
    ::ucb::Content sourceContent( m_url_expanded, xCmdEnv );
    OUString title(newTitle);
    if (title.getLength() == 0)
        sourceContent.getPropertyValue( OUSTR("Title") ) >>= title;
    OUString destURL( make_url( destFolderURL, ::rtl::Uri::encode(
                                    title, rtl_UriCharClassPchar,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 ) ) );

    if (nameClashAction == NameClash::ASK)
    {
        if (create_ucb_content(
                0, destURL, xCmdEnv, false /* no throw */ ) &&
            !interactContinuation(
                makeAny( NameClashResolveRequest(
                             OUSTR("file already exists: ") + title,
                             static_cast<OWeakObject *>(this),
                             task::InteractionClassification_QUERY,
                             destFolderURL, title, OUString() ) ),
                XInteractionReplaceExistingData::static_type(), xCmdEnv )) {
            return;
        }
    }
    else if (nameClashAction != NameClash::OVERWRITE) {
        throw CommandFailedException(
            OUSTR("unsupported nameClashAction!"),
            static_cast<OWeakObject *>(this), Any() );
    }
    erase_path( destURL, xCmdEnv );

    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.zip://") );
    buf.append( ::rtl::Uri::encode( destURL,
                                    rtl_UriCharClassRegName,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 ) );
    buf.append( static_cast<sal_Unicode>('/') );
    OUString destFolder( buf.makeStringAndClear() );

    ProgressLevel progress( xCmdEnv );
    ::ucb::Content destFolderContent( destFolder, xCmdEnv );
    // transfer every item of folder into zip:
    Reference<sdbc::XResultSet> xResultSet(
        sourceContent.createCursor( Sequence<OUString>(),
                                 ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
    while (xResultSet->next()) {
        ::ucb::Content subContent(
            Reference<XContentAccess>(
                xResultSet, UNO_QUERY_THROW )->queryContent(), xCmdEnv );
        if (! destFolderContent.transferContent(
                subContent, ::ucb::InsertOperation_COPY,
                OUString(), NameClash::OVERWRITE ))
            throw RuntimeException( OUSTR("UCB transferContent() failed!"),
                                    static_cast<OWeakObject *>(this) );
        progress.update();
    }

    // assure META-INF folder:
    ::ucb::Content metainfFolderContent;
    create_folder( &metainfFolderContent,
                   make_url( destFolderContent.getURL(), OUSTR("META-INF") ),
                   xCmdEnv );

    if (m_legacyBundle)
    {
        // easy to migrate legacy bundles to new format:
        // just export them once using a .uno.pkg name!
        // set detected media-types of any bundle item:

        // collect all manifest entries:
        Sequence< Reference<deployment::XPackage> > bundle;
        try {
            bundle = getBundle( Reference<task::XAbortChannel>(), xCmdEnv );
        }
        // xxx todo: think about exception specs:
        catch (deployment::DeploymentException &) {
            OSL_ASSERT( 0 );
        }
        catch (lang::IllegalArgumentException &) {
            OSL_ASSERT( 0 );
        }

        ::std::vector< Sequence<beans::PropertyValue> > manifest;
        manifest.reserve( bundle.getLength() );
        sal_Int32 baseURLlen = m_url_expanded.getLength();
        Reference<deployment::XPackage> const *pbundle = bundle.getConstArray();
        OUString strMediaType = OUSTR("MediaType");
        OUString strFullPath = OUSTR("FullPath");
        for ( sal_Int32 pos = bundle.getLength(); pos--; )
        {
            Reference<deployment::XPackage> const & xPackage = pbundle[ pos ];
            OUString url_( expand_url( xPackage->getURL() ) );
            OSL_ASSERT( url_.getLength() > baseURLlen );
            Sequence<beans::PropertyValue> attribs( 2 );
            beans::PropertyValue * pattribs = attribs.getArray();
            pattribs[ 0 ].Name = strFullPath;
            pattribs[ 0 ].Value <<= url_.copy( baseURLlen + 1 );
            pattribs[ 1 ].Name = strMediaType;
            pattribs[ 1 ].Value <<= xPackage->getMediaType();
            manifest.push_back( attribs );
        }

        // write into pipe:
        Reference<XComponentContext> xContext(
            getMyBackend()->getComponentContext() );
        Reference<packages::manifest::XManifestWriter> xManifestWriter(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.packages.manifest.ManifestWriter"),
                xContext ), UNO_QUERY_THROW );
        Reference<io::XOutputStream> xPipe(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.io.Pipe"), xContext ), UNO_QUERY_THROW );
        xManifestWriter->writeManifestSequence(
            xPipe, Sequence< Sequence<beans::PropertyValue> >(
                &manifest[ 0 ], manifest.size() ) );

        // write buffered pipe data to content:
        ::ucb::Content manifestContent(
            make_url( metainfFolderContent.getURL(), OUSTR("manifest.xml") ),
            xCmdEnv );
        manifestContent.writeStream(
            Reference<io::XInputStream>( xPipe, UNO_QUERY_THROW ),
            true /* replace existing */ );
    }
    else
    {
        // overwrite manifest.xml:
        ::ucb::Content manifestContent(
            make_url( m_url_expanded, OUSTR("META-INF/manifest.xml") ),
            xCmdEnv );
        if (! metainfFolderContent.transferContent(
                manifestContent, ::ucb::InsertOperation_COPY,
                OUString(), NameClash::OVERWRITE ))
            throw RuntimeException( OUSTR("UCB transferContent() failed!"),
                                    static_cast<OWeakObject *>(this) );
    }

    // xxx todo: obsolete in the future
    try {
        destFolderContent.executeCommand( OUSTR("flush"), Any() );
    }
    catch (UnsupportedCommandException &) {
    }
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
        try {
            if (m_legacyBundle)
            {
                // legacy packages allow script.xlb, dialog.xlb in bundle
                // root folder:
                OUString mediaType;
                // probe for script.xlb:
                if (create_ucb_content(
                        0, make_url( m_url_expanded, OUSTR("script.xlb") ),
                        xCmdEnv, false /* no throw */ )) {
                    mediaType = OUSTR("application/vnd.sun.star.basic-library");
                }
                // probe for dialog.xlb:
                else if (create_ucb_content(
                             0, make_url( m_url_expanded, OUSTR("dialog.xlb") ),
                             xCmdEnv, false /* no throw */ ))
                    mediaType = OUSTR("application/vnd.sun.star."
                                      "dialog-library");

                if (mediaType.getLength() > 0)
                {
                    Reference<deployment::XPackage> xPackage(
                        getMyBackend()->m_xRootRegistry->bindPackage(
                            getURL(),
                            mediaType, xCmdEnv ) );
                    OSL_ASSERT( xPackage.is() );
                    bundle.push_back( xPackage );
                    // continue scanning:
                }
                scanLegacyBundle( bundle, getURL(),
                                  AbortChannel::get(xAbortChannel), xCmdEnv );
            }
            else
            {
                // .uno.pkg:
                ::rtl::Reference<AbortChannel> abortChannel(
                    AbortChannel::get(xAbortChannel) );
                t_bundleInfos::const_iterator iPos( m_bundleInfos.begin() );
                t_bundleInfos::const_iterator const iEnd( m_bundleInfos.end() );
                for ( ; iPos != iEnd; ++iPos ) {
                    checkAborted( abortChannel );
                    try {
                        Reference<deployment::XPackage> xPackage(
                            getMyBackend()->m_xRootRegistry->bindPackage(
                                iPos->first, iPos->second, xCmdEnv ) );
                        OSL_ASSERT( xPackage.is() );
                        bundle.push_back( xPackage );
                    }
                    catch (lang::IllegalArgumentException &) {
                    }
                }
            }
        }
        catch (RuntimeException &) {
            throw;
        }
        catch (CommandFailedException &) {
            throw;
        }
        catch (CommandAbortedException &) {
            throw;
        }
        catch (deployment::DeploymentException &) {
            throw;
        }
        catch (Exception &) {
            Any exc( ::cppu::getCaughtException() );
            throw deployment::DeploymentException(
                OUSTR("error scanning bundle: ") + getURL(),
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
            else {
                pret[ lower_end ] = *iPos;
                ++lower_end;
            }
        }
        OSL_ASSERT( lower_end == upper_end );

        ::osl::MutexGuard guard( getMutex() );
        if (! m_bundleInit) {
            m_bundle = ret;
            m_bundleInit = true;
        }
    }
    return m_bundle;
}

//______________________________________________________________________________
void PackageImpl::scanLegacyBundle(
    t_packagevec & bundle,
    OUString const & url,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool skip_registration )
{
    ::ucb::Content ucbContent( url, xCmdEnv );

    // check for platform pathes:
    OUString title( extract_throw<OUString>(
                        ucbContent.getPropertyValue( OUSTR("Title") ) ) );
    if (title.endsWithIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM(".plt") ) &&
        !platform_fits( title.copy( 0, title.getLength() - 4 ) ))
        return;
    if (title.endsWithIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("skip_registration") ))
        skip_registration = true;

    OUString ar [] = { OUSTR("Title"), OUSTR("IsFolder") };
    Reference<sdbc::XResultSet> xResultSet(
        ucbContent.createCursor( Sequence<OUString>( ar, ARLEN(ar) ),
                                 ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
    while (xResultSet->next())
    {
        checkAborted( abortChannel );

        Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
        OUString title( xRow->getString( 1 /* Title */ ) );
        OUString title_enc( ::rtl::Uri::encode( title, rtl_UriCharClassPchar,
                                                rtl_UriEncodeIgnoreEscapes,
                                                RTL_TEXTENCODING_UTF8 ) );
        if (url.matchAsciiL(
                RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") )) {
            // encode once more for vnd.sun.star.expand schema:
            // vnd.sun.star.expand:$UNO_... will expand to file-url
            title_enc = ::rtl::Uri::encode( title_enc, rtl_UriCharClassUric,
                                            rtl_UriEncodeIgnoreEscapes,
                                            RTL_TEXTENCODING_UTF8 );
        }
        OUString path( make_url( url, title_enc ) );

        OUString mediaType;
        try {
            Reference<deployment::XPackage> xPackage(
                getMyBackend()->m_xRootRegistry->bindPackage(
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
        catch (lang::IllegalArgumentException &) {
        }

        if (mediaType.getLength() == 0 ||
            // script.xlb, dialog.xlb can be met everywhere:
            mediaType.matchIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "application/vnd.sun.star.basic-library") ) ||
            mediaType.matchIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "application/vnd.sun.star.dialog-library") ))
        {
            if (xRow->getBoolean( 2 /* IsFolder */ )) // recurse into folder:
                scanLegacyBundle(
                    bundle, path, abortChannel, xCmdEnv, skip_registration );
        }
    }
}

} // namespace bundle
} // namespace backend
} // namespace dp_registry

