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

#include <strings.hrc>
#include <dp_backend.h>
#include <dp_ucb.h>
#include "dp_parceldesc.hxx"
#include <rtl/uri.hxx>
#include <ucbhelper/content.hxx>
#include <svl/inettype.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <cppuhelper/supportsservice.hxx>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::script;


namespace dp_registry::backend::sfwk
{

namespace {

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
            Reference<XCommandEnvironment> const & xCmdEnv ) override;
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;

    public:
        PackageImpl(
            ::rtl::Reference<BackendImpl> const & myBackend,
            OUString const & url, OUString const & libType, bool bRemoved,
            OUString const & identifier);
        // XPackage
        virtual OUString SAL_CALL getDescription() override;
        virtual OUString SAL_CALL getLicenseText() override;
    };
    friend class PackageImpl;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        bool bRemoved, OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv ) override;

    const Reference<deployment::XPackageTypeInfo> m_xTypeInfo;


public:
    BackendImpl(
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() override;
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType) override;
};

}

BackendImpl * BackendImpl::PackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (nullptr == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException("Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<PackageImpl *>(this)));
    }
    return pBackend;
}

OUString BackendImpl::PackageImpl::getDescription()
{
    if (m_descr.isEmpty())
        return Package::getDescription();
    else
        return m_descr;
}

OUString BackendImpl::PackageImpl::getLicenseText()
{
    return Package::getDescription();
}

BackendImpl::PackageImpl::PackageImpl(
    ::rtl::Reference<BackendImpl> const & myBackend,
    OUString const & url, OUString const & libType, bool bRemoved,
    OUString const & identifier)
    : Package( myBackend, url, OUString(), OUString(),
               myBackend->m_xTypeInfo, bRemoved, identifier),
      m_descr(libType)
{
    initPackageHandler();

    sal_Int32 segmEnd = url.getLength();
    if ( url.endsWith("/") )
        --segmEnd;
    sal_Int32 segmStart = url.lastIndexOf( '/', segmEnd ) + 1;
    if (segmStart < 0)
        segmStart = 0;
    // name and display name default the same:
    m_displayName = ::rtl::Uri::decode(
        url.copy( segmStart, segmEnd - segmStart ),
        rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
    m_name = m_displayName;

    dp_misc::TRACE("PackageImpl displayName is " + m_displayName);
}


BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_xTypeInfo( new Package::TypeInfo(
                       "application/vnd.sun.star.framework-script",
                       OUString() /* no file filter */,
                       "Scripting Framework Script Library"
                       ) )
{
}


// XServiceInfo
OUString BackendImpl::getImplementationName()
{
    return "com.sun.star.comp.deployment.sfwk.PackageRegistryBackend";
}

sal_Bool BackendImpl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > BackendImpl::getSupportedServiceNames()
{
    return { BACKEND_SERVICE_NAME };
}

// XPackageRegistry

Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes()
{
    return Sequence< Reference<deployment::XPackageTypeInfo> >(&m_xTypeInfo, 1);
}

void BackendImpl::packageRemoved(OUString const & /*url*/, OUString const & /*mediaType*/)
{
}

// PackageRegistryBackend

Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_, bool bRemoved,
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
                    nullptr, makeURL( url, "parcel-descriptor.xml" ),
                    xCmdEnv, false /* no throw */ ))
            {
                mediaType = "application/vnd.sun.star.framework-script";
            }
        }
        if (mediaType.isEmpty())
            throw lang::IllegalArgumentException(
                StrCannotDetectMediaType() + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    OUString type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.equalsIgnoreAsciiCase("application"))
        {
            if (subType.equalsIgnoreAsciiCase("vnd.sun.star.framework-script"))
            {
                OUString lang = "Script";
                OUString sParcelDescURL = makeURL(
                    url, "parcel-descriptor.xml" );

                ::ucbhelper::Content ucb_content;

                if (create_ucb_content( &ucb_content, sParcelDescURL,
                        xCmdEnv, false /* no throw */ ))
                {
                    rtl::Reference<ParcelDescDocHandler> pHandler =
                        new ParcelDescDocHandler();

                    Reference<XComponentContext>
                        xContext( getComponentContext() );

                    Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(xContext);

                    xParser->setDocumentHandler( pHandler );
                    xml::sax::InputSource source;
                    source.aInputStream = ucb_content.openStream();
                    source.sSystemId = ucb_content.getURL();
                    xParser->parseStream( source );

                    if ( pHandler->isParsed() )
                    {
                        lang = pHandler->getParcelLanguage();
                    }
                }

                OUString sfwkLibType = DpResId( RID_STR_SFWK_LIB );
                // replace %MACRONAME placeholder with language name
                OUString MACRONAME( "%MACROLANG" );
                sal_Int32 startOfReplace = sfwkLibType.indexOf( MACRONAME );
                sal_Int32 charsToReplace = MACRONAME.getLength();
                sfwkLibType = sfwkLibType.replaceAt( startOfReplace, charsToReplace, lang );
                dp_misc::TRACE("******************************\n");
                dp_misc::TRACE(" BackEnd detected lang = " + lang + "\n");
                dp_misc::TRACE(" for url " + sParcelDescURL + "\n");
                dp_misc::TRACE("******************************\n");
                return new PackageImpl( this, url, sfwkLibType, bRemoved, identifier);
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}


void BackendImpl::PackageImpl:: initPackageHandler()
{
    if (m_xNameCntrPkgHandler.is())
        return;

    BackendImpl * that = getMyBackend();
    Any aContext;

    if ( that->m_eContext == Context::User )
    {
        aContext  <<= OUString("user");
    }
    else if ( that->m_eContext == Context::Shared )
    {
        aContext  <<= OUString("share");
    }
    else if ( that->m_eContext == Context::Bundled )
    {
        aContext  <<= OUString("bundled");
    }
    else
    {
        OSL_ASSERT( false );
        // NOT supported at the moment // TODO
    }

    Reference< provider::XScriptProviderFactory > xFac =
        provider::theMasterScriptProviderFactory::get( that->getComponentContext() );

    Reference< container::XNameContainer > xName( xFac->createScriptProvider( aContext ), UNO_QUERY );
    if ( xName.is() )
    {
        m_xNameCntrPkgHandler.set( xName );
    }
    // TODO what happens if above fails??
}

// Package

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
        throw RuntimeException( "No package Handler " );
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

} // namespace dp_registry::backend::sfwk

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_deployment_sfwk_PackageRegistryBackend_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new dp_registry::backend::sfwk::BackendImpl(args, context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
