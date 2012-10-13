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

#include "dp_sfwk.hrc"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "dp_parceldesc.hxx"
#include "rtl/uri.hxx"
#include "ucbhelper/content.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/servicedecl.hxx"
#include "svl/inettype.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::script;

using ::rtl::OUString;

namespace dp_registry
{
namespace backend
{
namespace sfwk
{

//==============================================================================
class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class PackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;

        Reference< container::XNameContainer > m_xNameCntrPkgHandler;
        OUString m_descr;

        void initPackageHandler();

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

    public:
        PackageImpl(
            ::rtl::Reference<BackendImpl> const & myBackend,
            OUString const & url, OUString const & libType, bool bRemoved,
            OUString const & identifier);
        // XPackage
        virtual OUString SAL_CALL getDescription() throw (RuntimeException);
        virtual OUString SAL_CALL getLicenseText() throw (RuntimeException);
    };
    friend class PackageImpl;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        sal_Bool bRemoved, OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv );

    const Reference<deployment::XPackageTypeInfo> m_xTypeInfo;


public:
    BackendImpl(
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType)
        throw (deployment::DeploymentException,
               uno::RuntimeException);
};

BackendImpl * BackendImpl::PackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (NULL == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            OUSTR("Failed to get the BackendImpl"),
            static_cast<OWeakObject*>(const_cast<PackageImpl *>(this)));
    }
    return pBackend;
}
//______________________________________________________________________________
OUString BackendImpl::PackageImpl::getDescription() throw (RuntimeException)
{
    if (m_descr.isEmpty())
        return Package::getDescription();
    else
        return m_descr;
}

//______________________________________________________________________________
OUString BackendImpl::PackageImpl::getLicenseText() throw (RuntimeException)
{
    return Package::getDescription();
}

//______________________________________________________________________________
BackendImpl::PackageImpl::PackageImpl(
    ::rtl::Reference<BackendImpl> const & myBackend,
    OUString const & url, OUString const & libType, bool bRemoved,
    OUString const & identifier)
    : Package( myBackend.get(), url, OUString(), OUString(),
               myBackend->m_xTypeInfo, bRemoved, identifier),
      m_descr(libType)
{
    initPackageHandler();

    sal_Int32 segmEnd = url.getLength();
    if (!url.isEmpty() && url[ url.getLength() - 1 ] == '/')
        --segmEnd;
    sal_Int32 segmStart = (url.lastIndexOf( '/', segmEnd ) + 1);
    if (segmStart < 0)
        segmStart = 0;
    // name and display name default the same:
    m_displayName = ::rtl::Uri::decode(
        url.copy( segmStart, segmEnd - segmStart ),
        rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
    m_name = m_displayName;

    dp_misc::TRACE(OUSTR("PakageImpl displayName is ") + m_displayName);
}

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_xTypeInfo( new Package::TypeInfo(
                       OUSTR("application/vnd.sun.star.framework-script"),
                       OUString() /* no file filter */,
                       OUSTR("Scripting Framework Script Library"),
                       RID_IMG_SCRIPTLIB ) )
{
    if (! transientMode())
    {
    }
}



// XPackageRegistry
//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return Sequence< Reference<deployment::XPackageTypeInfo> >(&m_xTypeInfo, 1);
}

void BackendImpl::packageRemoved(OUString const & /*url*/, OUString const & /*mediaType*/)
        throw (deployment::DeploymentException,
               uno::RuntimeException)
{
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_, sal_Bool bRemoved,
    OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.isEmpty())
    {
        // detect media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ) &&
            ucbContent.isFolder())
        {
            // probe for parcel-descriptor.xml:
            if (create_ucb_content(
                    0, makeURL( url, OUSTR("parcel-descriptor.xml") ),
                    xCmdEnv, false /* no throw */ ))
            {
                mediaType = OUSTR("application/vnd.sun.star.framework-script");
            }
        }
        if (mediaType.isEmpty())
            throw lang::IllegalArgumentException(
                StrCannotDetectMediaType::get() + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.EqualsIgnoreCaseAscii("application"))
        {
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.framework-script"))
            {
                OUString lang = OUString("Script");
                OUString sParcelDescURL = makeURL(
                    url, OUSTR("parcel-descriptor.xml") );

                ::ucbhelper::Content ucb_content;

                if (create_ucb_content( &ucb_content, sParcelDescURL,
                        xCmdEnv, false /* no throw */ ))
                {
                    ParcelDescDocHandler* pHandler =
                        new ParcelDescDocHandler();
                    Reference< xml::sax::XDocumentHandler >
                        xDocHandler = pHandler;

                    Reference<XComponentContext>
                        xContext( getComponentContext() );

                    Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);

                    xParser->setDocumentHandler( xDocHandler );
                    xml::sax::InputSource source;
                    source.aInputStream = ucb_content.openStream();
                    source.sSystemId = ucb_content.getURL();
                    xParser->parseStream( source );

                    if ( pHandler->isParsed() )
                    {
                        lang = pHandler->getParcelLanguage();
                    }
                }

                OUString sfwkLibType = getResourceString( RID_STR_SFWK_LIB );
                // replace %MACRONAME placeholder with language name
                OUString MACRONAME( OUSTR("%MACROLANG" ) );
                sal_Int32 startOfReplace = sfwkLibType.indexOf( MACRONAME );
                sal_Int32 charsToReplace = MACRONAME.getLength();
                sfwkLibType = sfwkLibType.replaceAt( startOfReplace, charsToReplace, lang );
                dp_misc::TRACE("******************************\n");
                dp_misc::TRACE(OUSTR(" BackEnd detected lang = ") + lang + OUSTR("\n"));
                dp_misc::TRACE(OUSTR(" for url ") + sParcelDescURL + OUSTR("\n") );
                dp_misc::TRACE("******************************\n");
                return new PackageImpl( this, url, sfwkLibType, bRemoved, identifier);
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}


void BackendImpl::PackageImpl:: initPackageHandler()
{
    if (m_xNameCntrPkgHandler.is())
        return;

    BackendImpl * that = getMyBackend();
    Any aContext;

    if ( that->m_eContext == CONTEXT_USER )
    {
        aContext  <<= OUSTR("user");
    }
    else if ( that->m_eContext == CONTEXT_SHARED )
    {
        aContext  <<= OUSTR("share");
    }
    else if ( that->m_eContext == CONTEXT_BUNDLED )
    {
        aContext  <<= OUSTR("bundled");
    }
    else
    {
        OSL_ASSERT( 0 );
        // NOT supported at the momemtn // TODO
    }

    Reference< provider::XScriptProviderFactory > xFac(
        that->getComponentContext()->getValueByName(
            OUSTR( "/singletons/com.sun.star.script.provider.theMasterScriptProviderFactory") ), UNO_QUERY );

    if ( xFac.is() )
    {
        Reference< container::XNameContainer > xName( xFac->createScriptProvider( aContext ), UNO_QUERY );
        if ( xName.is() )
        {
            m_xNameCntrPkgHandler.set( xName );
        }
    }
    // TODO what happens if above fails??
}

// Package
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            m_xNameCntrPkgHandler.is() && m_xNameCntrPkgHandler->hasByName(
                m_url ),
            false /* IsAmbiguous */ ) );
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool /* startup */,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    if ( !m_xNameCntrPkgHandler.is() )
    {
        dp_misc::TRACE("no package handler!!!!\n");
        throw RuntimeException( OUSTR("No package Handler " ),
            Reference< XInterface >() );
    }

    if (doRegisterPackage)
    {
        // will throw if it fails
        m_xNameCntrPkgHandler->insertByName( m_url, makeAny( Reference< XPackage >(this) ) );

    }
    else // revokePackage()
    {
        m_xNameCntrPkgHandler->removeByName( m_url );
    }
}

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.sfwk.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace sfwk
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
