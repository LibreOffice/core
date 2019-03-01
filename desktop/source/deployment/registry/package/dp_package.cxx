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


#include <strings.hrc>
#include <dp_package.hxx>
#include <dp_backend.h>
#include <dp_ucb.h>
#include <dp_interact.h>
#include <dp_dependencies.hxx>
#include <dp_platform.hxx>
#include <dp_descriptioninfoset.hxx>
#include <dp_identifier.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <ucbhelper/content.hxx>
#include <svl/inettype.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XInteractionReplaceExistingData.hpp>
#include <com/sun/star/ucb/NameClashResolveRequest.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>
#include <com/sun/star/packages/manifest/ManifestWriter.hpp>
#include <com/sun/star/deployment/DependencyException.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionRemovedException.hpp>
#include <com/sun/star/deployment/LicenseException.hpp>
#include <com/sun/star/deployment/PlatformException.hpp>
#include <com/sun/star/deployment/Prerequisites.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/deployment/XPackageManager.hpp>
#include <boost/optional.hpp>
#include <tools/diagnose_ex.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "dp_extbackenddb.hxx"
using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace dp_registry {
namespace backend {
namespace bundle {
namespace {

typedef cppu::ImplInheritanceHelper<PackageRegistryBackend,
                                     lang::XServiceInfo> ImplBaseT;


class BackendImpl : public ImplBaseT
{
    class PackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const;
        /** constains the old tooltip description for the Extension Manager GUI in OOo v.2.x
            We keep it for backward compatibility.
        */
        OUString m_oldDescription;
        OUString m_url_expanded;
        const bool m_legacyBundle;
        Sequence< Reference<deployment::XPackage> > m_bundle;
        Sequence< Reference<deployment::XPackage> > * m_pBundle;

        ExtensionBackendDb::Data m_dbData;

        Reference<deployment::XPackage> bindBundleItem(
            OUString const & url, OUString const & mediaType,
            bool bRemoved, //that is, using data base information
            OUString const & identifier,
            Reference<ucb::XCommandEnvironment> const & xCmdEnv,
            bool notifyDetectionError = true );

        typedef std::vector< Reference<deployment::XPackage> > t_packagevec;
        void scanBundle(
            t_packagevec & bundle,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<ucb::XCommandEnvironment> const & xCmdEnv );
        void scanLegacyBundle(
            t_packagevec & bundle,
            OUString const & url,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<ucb::XCommandEnvironment> const & xCmdEnv,
            bool skip_registration = false );
        std::vector<Reference<deployment::XPackage> > getPackagesFromDb(
            Reference<ucb::XCommandEnvironment> const & xCmdEnv);
        bool checkPlatform(
            Reference<ucb::XCommandEnvironment > const &  environment);

        bool checkDependencies(
            Reference<ucb::XCommandEnvironment > const &
                environment,
            DescriptionInfoset const & description);
            // throws css::uno::RuntimeException,
            // css::deployment::DeploymentException

        /// @throws deployment::DeploymentException
        /// @throws ucb::CommandFailedException
        /// @throws ucb::CommandAbortedException
        /// @throws RuntimeException
        bool checkLicense(
            Reference< ucb::XCommandEnvironment > const & xCmdEnv,
            DescriptionInfoset const & description, bool bNoLicenseChecking);
        // @throws DeploymentException
        OUString getTextFromURL(
            const Reference< ucb::XCommandEnvironment >& xCmdEnv,
            const OUString& licenseUrl);

        DescriptionInfoset getDescriptionInfoset();

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<ucb::XCommandEnvironment> const & xCmdEnv ) override;
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<ucb::XCommandEnvironment> const & xCmdEnv ) override;

        virtual void SAL_CALL disposing() override;


    public:
        PackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url,
            OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool legacyBundle,
            bool bRemoved,
            OUString const & identifier);

        // XPackage
        virtual sal_Bool SAL_CALL isBundle() override;

        virtual Sequence< Reference<deployment::XPackage> > SAL_CALL getBundle(
            Reference<task::XAbortChannel> const & xAbortChannel,
            Reference<ucb::XCommandEnvironment> const & xCmdEnv ) override;
        virtual OUString SAL_CALL getDescription() override;

        virtual OUString SAL_CALL getLicenseText() override;

        virtual void SAL_CALL exportTo(
            OUString const & destFolderURL, OUString const & newTitle,
            sal_Int32 nameClashAction,
            Reference<ucb::XCommandEnvironment> const & xCmdEnv ) override;

        virtual ::sal_Int32 SAL_CALL checkPrerequisites(
            const Reference< task::XAbortChannel >& xAbortChannel,
            const Reference< ucb::XCommandEnvironment >& xCmdEnv,
            sal_Bool noLicenseChecking) override;

        virtual sal_Bool SAL_CALL checkDependencies(
            const Reference< ucb::XCommandEnvironment >& xCmdEnv ) override;

        virtual beans::Optional<OUString> SAL_CALL getIdentifier() override;

        virtual OUString SAL_CALL getVersion() override;

        virtual Sequence<OUString> SAL_CALL getUpdateInformationURLs() override;

        virtual beans::StringPair SAL_CALL getPublisherInfo() override;

        virtual OUString SAL_CALL getDisplayName() override;

        virtual Reference< graphic::XGraphic > SAL_CALL
        getIcon( sal_Bool bHighContrast ) override;
    };
    friend class PackageImpl;

    Reference<deployment::XPackageRegistry> m_xRootRegistry;
    const Reference<deployment::XPackageTypeInfo> m_xBundleTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xLegacyBundleTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

    std::unique_ptr<ExtensionBackendDb> m_backendDb;

    void addDataToDb(OUString const & url, ExtensionBackendDb::Data const & data);
    ExtensionBackendDb::Data readDataFromDb(OUString const & url);
    void revokeEntryFromDb(OUString const & url);

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        bool bRemoved, OUString const & identifier,
        Reference<ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual void SAL_CALL disposing() override;

public:
    BackendImpl(
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xComponentContext,
        Reference<deployment::XPackageRegistry> const & xRootRegistry );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( OUString const& name ) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() override;
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType) override;

    using ImplBaseT::disposing;
};

//Used to find a XPackage with a particular URL
class XPackage_eq
{
    OUString m_URL;
public:
    explicit XPackage_eq(const OUString & s) : m_URL(s) {}
    bool operator() (const Reference<deployment::XPackage> & p) const
    {
        return m_URL == p->getURL();
    }
};


BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext,
    Reference<deployment::XPackageRegistry> const & xRootRegistry )
    : ImplBaseT( args, xComponentContext ),
      m_xRootRegistry( xRootRegistry ),
      m_xBundleTypeInfo( new Package::TypeInfo(
                             "application/vnd.sun.star.package-bundle",
                             "*.oxt;*.uno.pkg",
                             DpResId(RID_STR_PACKAGE_BUNDLE)
                             ) ),
      m_xLegacyBundleTypeInfo( new Package::TypeInfo(
                                   "application/vnd.sun.star.legacy-package-bundle",
                                   "*.zip",
                                   m_xBundleTypeInfo->getShortDescription()
                                   ) ),
    m_typeInfos(2)
{
    m_typeInfos[ 0 ] = m_xBundleTypeInfo;
    m_typeInfos[ 1 ] = m_xLegacyBundleTypeInfo;

    if (!transientMode())
    {
        OUString dbFile = makeURL(getCachePath(), getImplementationName());
        dbFile = makeURL(dbFile, "backenddb.xml");
        m_backendDb.reset(
            new ExtensionBackendDb(getComponentContext(), dbFile));
   }
}


void BackendImpl::disposing()
{
    m_xRootRegistry.clear();
    PackageRegistryBackend::disposing();
}

// XServiceInfo
OUString BackendImpl::getImplementationName()
{
    return OUString("com.sun.star.comp.deployment.bundle.PackageRegistryBackend");
}

sal_Bool BackendImpl::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> BackendImpl::getSupportedServiceNames()
{
    return { OUString(BACKEND_SERVICE_NAME) };
}

// XPackageRegistry

Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes()
{
    return m_typeInfos;
}

void BackendImpl::packageRemoved(OUString const & url, OUString const & /*mediaType*/)
{
    //Notify the backend responsible for processing the different media
    //types that this extension was removed.
    ExtensionBackendDb::Data data = readDataFromDb(url);
    for (auto const& item : data.items)
    {
        m_xRootRegistry->packageRemoved(item.first, item.second);
    }

    if (m_backendDb)
        m_backendDb->removeEntry(url);
}


// PackageRegistryBackend

Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    bool bRemoved, OUString const & identifier,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.isEmpty())
    {
        // detect media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ))
        {
            if (ucbContent.isFolder())
            {
                //Every .oxt, uno.pkg file must contain a META-INF folder
                ::ucbhelper::Content metaInfContent;
                if (create_ucb_content(
                    &metaInfContent, makeURL( url, "META-INF" ),
                    xCmdEnv, false /* no throw */ ))
                {
                     mediaType = "application/vnd.sun.star.package-bundle";
                }
                //No support of legacy bundles, because every folder could be one.
            }
            else
            {
                const OUString title( StrTitle::getTitle( ucbContent ) );
                if (title.endsWithIgnoreAsciiCase(".oxt") ||
                    title.endsWithIgnoreAsciiCase(".uno.pkg"))
                    mediaType = "application/vnd.sun.star.package-bundle";
                else if (title.endsWithIgnoreAsciiCase(".zip"))
                    mediaType = "application/vnd.sun.star.legacy-package-bundle";
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

            //In case a XPackage is created for a removed extension, we cannot
            //obtain the name
            OUString name;
            if (!bRemoved)
            {
                ::ucbhelper::Content ucbContent(
                    url, xCmdEnv, getComponentContext() );
                name = StrTitle::getTitle( ucbContent );
            }
            if (subType.equalsIgnoreAsciiCase("vnd.sun.star.package-bundle"))
            {
                return new PackageImpl(
                    this, url, name, m_xBundleTypeInfo, false, bRemoved,
                    identifier);
            }
            else if (subType.equalsIgnoreAsciiCase( "vnd.sun.star.legacy-package-bundle"))
            {
                return new PackageImpl(
                    this, url, name, m_xLegacyBundleTypeInfo, true, bRemoved,
                    identifier);
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

void BackendImpl::addDataToDb(
    OUString const & url, ExtensionBackendDb::Data const & data)
{
    if (m_backendDb)
        m_backendDb->addEntry(url, data);
}

ExtensionBackendDb::Data BackendImpl::readDataFromDb(
    OUString const & url)
{
    ExtensionBackendDb::Data data;
    if (m_backendDb)
        data = m_backendDb->getEntry(url);
    return data;
}

void BackendImpl::revokeEntryFromDb(OUString const & url)
{
    if (m_backendDb)
        m_backendDb->revokeEntry(url);
}


BackendImpl::PackageImpl::PackageImpl(
    ::rtl::Reference<PackageRegistryBackend> const & myBackend,
    OUString const & url,
    OUString const & name,
    Reference<deployment::XPackageTypeInfo> const & xPackageType,
    bool legacyBundle, bool bRemoved, OUString const & identifier)
    : Package( myBackend, url, name, name /* display-name */,
               xPackageType, bRemoved, identifier),
      m_url_expanded( expandUnoRcUrl( url ) ),
      m_legacyBundle( legacyBundle ),
      m_pBundle( nullptr )
{
    if (bRemoved)
        m_dbData = getMyBackend()->readDataFromDb(url);
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

void BackendImpl::PackageImpl::disposing()
{
    sal_Int32 len = m_bundle.getLength();
    Reference<deployment::XPackage> const * p = m_bundle.getConstArray();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
        try_dispose( p[ pos ] );
    m_bundle.realloc( 0 );

    Package::disposing();
}

// Package

beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    //In case the object was created for a removed extension (m_bRemoved = true)
    //but the extension is not registered, then bundle will be empty. Then
    //the return value will be Optional<...>.IsPresent= false. Although this is
    //not true, this does not matter. Then registerPackage or revokePackage
    //would never be called for the items. But since the extension is removed
    //and not registered anyway, this does not matter.
    const Sequence< Reference<deployment::XPackage> > bundle(
        getBundle( abortChannel.get(), xCmdEnv ) );

    bool reg = false;
    bool present = false;
    bool ambig = false;
    for ( sal_Int32 pos = bundle.getLength(); pos--; )
    {
        Reference<deployment::XPackage> const & xPackage = bundle[ pos ];
        Reference<task::XAbortChannel> xSubAbortChannel(
            xPackage->createAbortChannel() );
        AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
        beans::Optional< beans::Ambiguous<sal_Bool> > option(
            xPackage->isRegistered( xSubAbortChannel, xCmdEnv ) );

        //present = true if at least one bundle item has this value.
        //reg = true if all bundle items have an option value (option.IsPresent == 1)
        //and all have value of true (option.Value.Value == true)
        //If not, then the bundle has the status of not registered and ambiguous.
        if (option.IsPresent)
        {
            beans::Ambiguous<sal_Bool> const & status = option.Value;
            if (present)
            {
                //we never come here in the first iteration
                if (reg != bool(status.Value)) {

                    ambig = true;
                    reg = false;
                    break;
                }
            }
            else
            {
                //we always come here in the first iteration
                reg = status.Value;
                present = true;
            }
        }
    }
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        present, beans::Ambiguous<sal_Bool>(reg, ambig) );
}

OUString BackendImpl::PackageImpl::getTextFromURL(
    const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
    const OUString& licenseUrl)
{
    try
    {
        ::ucbhelper::Content descContent(
            licenseUrl, xCmdEnv, getMyBackend()->getComponentContext());
        std::vector<sal_Int8> seq = dp_misc::readFile(descContent);
        return OUString( reinterpret_cast<sal_Char const *>(
            seq.data()), seq.size(), RTL_TEXTENCODING_UTF8);
    }
    catch (const css::uno::Exception&)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
                "Could not read file " + licenseUrl, nullptr, exc);
    }

}

DescriptionInfoset BackendImpl::PackageImpl::getDescriptionInfoset()
{
    return dp_misc::getDescriptionInfoset(m_url_expanded);
}

bool BackendImpl::PackageImpl::checkPlatform(
    css::uno::Reference< css::ucb::XCommandEnvironment > const &  environment)
{
    bool ret = false;
    DescriptionInfoset info(getDescriptionInfoset());
    Sequence<OUString> platforms(info.getSupportedPlatforms());
    if (hasValidPlatform(platforms))
    {
        ret = true;
    }
    else
    {
        ret = false;
        OUString msg(
            "unsupported platform");
        Any e(
            css::deployment::PlatformException(
                msg, static_cast<OWeakObject *>(this), this));
        if (!interactContinuation(
                e, cppu::UnoType< css::task::XInteractionApprove >::get(),
                environment, nullptr, nullptr))
        {
            throw css::deployment::DeploymentException(
                msg, static_cast<OWeakObject *>(this), e);
        }
    }
    return ret;
}


bool BackendImpl::PackageImpl::checkDependencies(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & environment,
    DescriptionInfoset const & description)
{
    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
        unsatisfied(dp_misc::Dependencies::check(description));

    if (unsatisfied.getLength() == 0) {
        return true;
    } else {
        OUString msg(
            "unsatisfied dependencies");
        Any e(
            css::deployment::DependencyException(
                msg, static_cast<OWeakObject *>(this), unsatisfied));
        if (!interactContinuation(
                e, cppu::UnoType< css::task::XInteractionApprove >::get(),
                environment, nullptr, nullptr))
        {
            throw css::deployment::DeploymentException(
                msg, static_cast<OWeakObject *>(this), e);
        }
        return false;
    }
}

bool BackendImpl::PackageImpl::checkLicense(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
    DescriptionInfoset const & info, bool alreadyInstalled)
{
    try
    {
        ::boost::optional<SimpleLicenseAttributes> simplLicAttr
            = info.getSimpleLicenseAttributes();
        if (! simplLicAttr)
            return true;
        OUString sLic = info.getLocalizedLicenseURL();
        //If we do not get a localized licence then there is an error in the description.xml
        //This should be handled by using a validating parser. Therefore we assume that no
        //license is available.
        if (sLic.isEmpty())
            throw css::deployment::DeploymentException(
                "Could not obtain path to license. Possible error in description.xml", nullptr, Any());
        OUString sHref = m_url_expanded + "/" + sLic;
        OUString sLicense = getTextFromURL(xCmdEnv, sHref);
        ////determine who has to agree to the license
        //check correct value for attribute
        if ( ! (simplLicAttr->acceptBy == "user" || simplLicAttr->acceptBy == "admin"))
            throw css::deployment::DeploymentException(
                "Could not obtain attribute simple-license@accept-by or it has no valid value", nullptr, Any());


        //Only use interaction if there is no version of this extension already installed
        //and the suppress-on-update flag is not set for the new extension
        // alreadyInstalled | bSuppressOnUpdate | show license

        //      0     |      0            |     1
        //      0     |      1            |     1
        //      1     |      0            |     1
        //      1     |      1            |     0

        if ( !(alreadyInstalled && simplLicAttr->suppressOnUpdate))
        {
            css::deployment::LicenseException licExc(
                OUString(), nullptr, getDisplayName(), sLicense,
                simplLicAttr->acceptBy);
            bool approve = false;
            bool abort = false;
            if (! interactContinuation(
                Any(licExc), cppu::UnoType<task::XInteractionApprove>::get(), xCmdEnv, &approve, &abort ))
                throw css::deployment::DeploymentException(
                    "Could not interact with user.", nullptr, Any());

            return approve;
        }
        return true;
    } catch (const css::ucb::CommandFailedException&) {
        throw;
    } catch (const css::ucb::CommandAbortedException&) {
        throw;
    } catch (const css::deployment::DeploymentException&) {
        throw;
    } catch (const css::uno::RuntimeException&) {
        throw;
    } catch (const css::uno::Exception&) {
        Any anyExc = cppu::getCaughtException();
        throw css::deployment::DeploymentException("Unexpected exception", nullptr, anyExc);
    }
}

::sal_Int32 BackendImpl::PackageImpl::checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >&,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
        sal_Bool alreadyInstalled)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    DescriptionInfoset info = getDescriptionInfoset();
    if (!info.hasDescription())
        return 0;

    //always return LICENSE as long as the user did not accept the license
    //so that XExtensonManager::checkPrerequisitesAndEnable will again
    //check the license
    if (!checkPlatform(xCmdEnv))
        return deployment::Prerequisites::PLATFORM |
            deployment::Prerequisites::LICENSE;
    else if(!checkDependencies(xCmdEnv, info))
        return deployment::Prerequisites::DEPENDENCIES |
            deployment::Prerequisites::LICENSE;
    else if(!checkLicense(xCmdEnv, info, alreadyInstalled))
        return deployment::Prerequisites::LICENSE;
    else
        return 0;
}

sal_Bool BackendImpl::PackageImpl::checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    DescriptionInfoset info = getDescriptionInfoset();
    if (!info.hasDescription())
        return true;

    return checkDependencies(xCmdEnv, info);
}

beans::Optional<OUString> BackendImpl::PackageImpl::getIdentifier()
{
    OUString identifier;
    if (m_bRemoved)
        identifier = m_identifier;
    else
        identifier = dp_misc::generateIdentifier(
            getDescriptionInfoset().getIdentifier(), m_name);

    return beans::Optional<OUString>(
        true, identifier);
}

OUString BackendImpl::PackageImpl::getVersion()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return getDescriptionInfoset().getVersion();
}

Sequence<OUString> BackendImpl::PackageImpl::getUpdateInformationURLs()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return getDescriptionInfoset().getUpdateInformationUrls();
}

beans::StringPair BackendImpl::PackageImpl::getPublisherInfo()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    std::pair< OUString, OUString > aInfo = getDescriptionInfoset().getLocalizedPublisherNameAndURL();
    beans::StringPair aStrPair( aInfo.first, aInfo.second );
    return aStrPair;
}


uno::Reference< graphic::XGraphic > BackendImpl::PackageImpl::getIcon( sal_Bool bHighContrast )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    uno::Reference< graphic::XGraphic > xGraphic;

    OUString aIconURL = getDescriptionInfoset().getIconURL( bHighContrast );
    if ( !aIconURL.isEmpty() )
    {
        OUString aFullIconURL = m_url_expanded + "/" + aIconURL;

        uno::Reference< XComponentContext > xContext( getMyBackend()->getComponentContext() );
        uno::Reference< graphic::XGraphicProvider > xGraphProvider( graphic::GraphicProvider::create(xContext) );

        uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
        aMediaProps[0].Name = "URL";
        aMediaProps[0].Value <<= aFullIconURL;

        xGraphic = xGraphProvider->queryGraphic( aMediaProps );
    }

    return xGraphic;
}


void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool startup,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    const Sequence< Reference<deployment::XPackage> > bundle(
        getBundle( abortChannel.get(), xCmdEnv ) );

    if (doRegisterPackage)
    {
        ExtensionBackendDb::Data data;
        const sal_Int32 len = bundle.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            checkAborted(abortChannel);
            Reference<deployment::XPackage> const & xPackage = bundle[ pos ];
            Reference<task::XAbortChannel> xSubAbortChannel(
                xPackage->createAbortChannel() );
            AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
            try {
                xPackage->registerPackage( startup, xSubAbortChannel, xCmdEnv );
            }
            catch (const Exception &)
            {
               //We even try a rollback if the user cancelled the action (CommandAbortedException)
                //in order to prevent invalid database entries.
                Any exc( ::cppu::getCaughtException() );
                // try to handle exception, notify:
                bool approve = false, abort = false;
                if (! interactContinuation(
                        Any( lang::WrappedTargetException(
                                 "bundle item registration error!",
                                 static_cast<OWeakObject *>(this), exc ) ),
                        cppu::UnoType<task::XInteractionApprove>::get(), xCmdEnv,
                        &approve, &abort )) {
                    OSL_ASSERT( !approve && !abort );
                    if (m_legacyBundle) // default for legacy packages: ignore
                        continue;
                    // no selection at all, so rethrow;
                    // no C++ rethrow after getCaughtException(),
                    // see cppuhelper/exc_hlp.hxx:
                    ::cppu::throwException(exc);
                }
                if (approve && !abort) // ignore error, just continue
                    continue;

                {
                    ProgressLevel progress( xCmdEnv, "rollback..." );
                    // try rollback
                    for ( ; pos--; )
                    {
                        try {
                            bundle[ pos ]->revokePackage(
                                startup, xSubAbortChannel, xCmdEnv );
                        }
                        catch (const Exception &)
                        {
                            SAL_WARN( "desktop", exceptionToString( cppu::getCaughtException() ) );
                            // ignore any errors of rollback
                        }
                    }
                    progress.update( "rollback finished." );
                }

                deployment::DeploymentException dpExc;
                if (exc >>= dpExc) {
                    throw ucb::CommandFailedException(
                        dpExc.Message, dpExc.Context, dpExc.Cause );
                }
                else {
                    // rethrow CommandFailedException
                    ::cppu::throwException(exc);
                }
            }
            data.items.emplace_back(xPackage->getURL(),
                                 xPackage->getPackageType()->getMediaType());
        }
        getMyBackend()->addDataToDb(getURL(), data);
    }
    else
    {
        // revoke in reverse order:
        for ( sal_Int32 pos = bundle.getLength(); pos--; )
        {
            checkAborted(abortChannel);
            Reference<deployment::XPackage> const & xPackage = bundle[ pos ];
            Reference<task::XAbortChannel> xSubAbortChannel(
                xPackage->createAbortChannel() );
            AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
            try {
                bundle[ pos ]->revokePackage(
                    startup, xSubAbortChannel, xCmdEnv );
            }
            catch (const RuntimeException &) {
                throw;
            }
            catch (const ucb::CommandAbortedException &) {
                throw;
            }
            catch (const Exception &) {
                // CommandFailedException, DeploymentException:
                Any exc( ::cppu::getCaughtException() );
                // try to handle exception, notify:
                bool approve = false, abort = false;
                if (! interactContinuation(
                        Any( lang::WrappedTargetException(
                                 "bundle item revocation error!",
                                 static_cast<OWeakObject *>(this), exc ) ),
                        cppu::UnoType<task::XInteractionApprove>::get(), xCmdEnv,
                        &approve, &abort )) {
                    OSL_ASSERT( !approve && !abort );
                    if (m_legacyBundle) // default for legacy packages: ignore
                        continue;
                    // no selection at all, so rethrow
                    // no C++ rethrow after getCaughtException(),
                    // see cppuhelper/exc_hlp.hxx:
                    ::cppu::throwException(exc);
                }
                // ignore errors when revoking, although abort may have been
                // selected
            }
        }
        getMyBackend()->revokeEntryFromDb(getURL());
    }
}


OUString BackendImpl::PackageImpl::getDescription()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    const OUString sRelativeURL(getDescriptionInfoset().getLocalizedDescriptionURL());
    OUString sDescription;
    if (!sRelativeURL.isEmpty())
    {
        OUString sURL = m_url_expanded + "/" + sRelativeURL;

        try
        {
            sDescription = getTextFromURL( css::uno::Reference< css::ucb::XCommandEnvironment >(), sURL );
        }
        catch ( const css::deployment::DeploymentException& )
        {
            SAL_WARN( "desktop", exceptionToString( cppu::getCaughtException() ) );
        }
    }

    if (!sDescription.isEmpty())
        return sDescription;
    return m_oldDescription;
}


OUString BackendImpl::PackageImpl::getLicenseText()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    OUString sLicense;
    DescriptionInfoset aInfo = getDescriptionInfoset();

    ::boost::optional< SimpleLicenseAttributes > aSimplLicAttr = aInfo.getSimpleLicenseAttributes();
    if ( aSimplLicAttr )
    {
        OUString aLicenseURL = aInfo.getLocalizedLicenseURL();

        if ( !aLicenseURL.isEmpty() )
        {
            OUString aFullURL = m_url_expanded + "/" + aLicenseURL;
            sLicense = getTextFromURL( Reference< ucb::XCommandEnvironment >(), aFullURL);
        }
    }

    return sLicense;
}


void BackendImpl::PackageImpl::exportTo(
    OUString const & destFolderURL, OUString const & newTitle,
    sal_Int32 nameClashAction, Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    ::ucbhelper::Content sourceContent(
        m_url_expanded, xCmdEnv, getMyBackend()->getComponentContext() );
    OUString title(newTitle);
    if (title.isEmpty())
        sourceContent.getPropertyValue( "Title" ) >>= title;
    OUString destURL( makeURL( destFolderURL, ::rtl::Uri::encode(
                                   title, rtl_UriCharClassPchar,
                                   rtl_UriEncodeIgnoreEscapes,
                                   RTL_TEXTENCODING_UTF8 ) ) );

    if (nameClashAction == ucb::NameClash::ASK)
    {
        if (create_ucb_content(
                nullptr, destURL, xCmdEnv, false /* no throw */ )) {
            bool replace = false, abort = false;
            if (! interactContinuation(
                    Any( ucb::NameClashResolveRequest(
                             "file already exists: " + title,
                             static_cast<OWeakObject *>(this),
                             task::InteractionClassification_QUERY,
                             destFolderURL, title, OUString() ) ),
                    cppu::UnoType<ucb::XInteractionReplaceExistingData>::get(), xCmdEnv,
                    &replace, &abort ) || !replace) {
                return;
            }
        }
    }
    else if (nameClashAction != ucb::NameClash::OVERWRITE) {
        throw ucb::CommandFailedException("unsupported nameClashAction!",
            static_cast<OWeakObject *>(this), Any() );
    }
    erase_path( destURL, xCmdEnv );

    OUStringBuffer buf;
    buf.append( "vnd.sun.star.zip://" );
    buf.append( ::rtl::Uri::encode( destURL,
                                    rtl_UriCharClassRegName,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 ) );
    buf.append( '/' );
    OUString destFolder( buf.makeStringAndClear() );

    ::ucbhelper::Content destFolderContent(
        destFolder, xCmdEnv, getMyBackend()->getComponentContext() );
    {
        // transfer every item of folder into zip:
        Reference<sdbc::XResultSet> xResultSet(
            sourceContent.createCursor( Sequence<OUString>() ) );
        ProgressLevel progress( xCmdEnv, OUString() );
        while (xResultSet->next())
        {
            ::ucbhelper::Content subContent(
                Reference<ucb::XContentAccess>(
                    xResultSet, UNO_QUERY_THROW )->queryContent(),
                xCmdEnv, getMyBackend()->getComponentContext() );
            destFolderContent.transferContent(
                    subContent, ::ucbhelper::InsertOperation::Copy,
                    OUString(), ucb::NameClash::OVERWRITE );
            progress.update( Any() ); // animating progress bar
        }
    }

    // assure META-INF folder:
    ::ucbhelper::Content metainfFolderContent;
    create_folder( &metainfFolderContent,
                   makeURL( destFolderContent.getURL(), "META-INF" ),
                   xCmdEnv );

    if (m_legacyBundle)
    {
        // easy to migrate legacy bundles to new format:
        // just export them once using a .oxt name!
        // set detected media-types of any bundle item:

        // collect all manifest entries:
        Sequence< Reference<deployment::XPackage> > bundle;
        try {
            bundle = getBundle( Reference<task::XAbortChannel>(), xCmdEnv );
        }
        // xxx todo: think about exception specs:
        catch (const deployment::DeploymentException &) {
            SAL_WARN( "desktop", exceptionToString( cppu::getCaughtException() ) );
        }
        catch (const lang::IllegalArgumentException & exc) {
            SAL_WARN( "desktop", exceptionToString(Any(exc)) );
        }

        std::vector< Sequence<beans::PropertyValue> > manifest;
        manifest.reserve( bundle.getLength() );
        sal_Int32 baseURLlen = m_url_expanded.getLength();
        Reference<deployment::XPackage> const *pbundle = bundle.getConstArray();
        const OUString strMediaType( "MediaType" );
        const OUString strFullPath( "FullPath" );
        const OUString strIsFolder( "IsFolder" );
        for ( sal_Int32 pos = bundle.getLength(); pos--; )
        {
            Reference<deployment::XPackage> const & xPackage = pbundle[ pos ];
            OUString url_( expandUnoRcUrl( xPackage->getURL() ) );
            OSL_ASSERT( url_.getLength() >= baseURLlen );
            OUString fullPath;
            if (url_.getLength() > baseURLlen)
                fullPath = url_.copy( baseURLlen + 1 );
            ::ucbhelper::Content ucbContent(
                url_, xCmdEnv, getMyBackend()->getComponentContext() );
            if (ucbContent.getPropertyValue(strIsFolder).get<bool>())
                fullPath += "/";
            Sequence<beans::PropertyValue> attribs( 2 );
            beans::PropertyValue * pattribs = attribs.getArray();
            pattribs[ 0 ].Name = strFullPath;
            pattribs[ 0 ].Value <<= fullPath;
            pattribs[ 1 ].Name = strMediaType;
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                xPackage->getPackageType() );
            OUString mediaType;
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is())
                mediaType = xPackageType->getMediaType();
            else
                mediaType = "unknown";
            pattribs[ 1 ].Value <<= mediaType;
            manifest.push_back( attribs );
        }

        // write into pipe:
        Reference<XComponentContext> xContext(
            getMyBackend()->getComponentContext() );
        Reference<packages::manifest::XManifestWriter> xManifestWriter =
            packages::manifest::ManifestWriter::create( xContext );
        Reference<io::XOutputStream> xPipe( io::Pipe::create(xContext), UNO_QUERY_THROW );
        xManifestWriter->writeManifestSequence(
            xPipe, comphelper::containerToSequence(manifest) );

        // write buffered pipe data to content:
        ::ucbhelper::Content manifestContent(
            makeURL( metainfFolderContent.getURL(), "manifest.xml" ),
            xCmdEnv, getMyBackend()->getComponentContext() );
        manifestContent.writeStream(
            Reference<io::XInputStream>( xPipe, UNO_QUERY_THROW ),
            true /* replace existing */ );
    }
    else
    {
        bool bSuccess = false;
        try
        {
            // overwrite manifest.xml:
            ::ucbhelper::Content manifestContent;
            if ( ! create_ucb_content(
                &manifestContent,
                makeURL( m_url_expanded, "META-INF/manifest.xml" ),
                xCmdEnv, false ) )
            {
                OSL_FAIL( "### missing META-INF/manifest.xml file!" );
                return;
            }

            metainfFolderContent.transferContent(
                  manifestContent, ::ucbhelper::InsertOperation::Copy,
                  OUString(), ucb::NameClash::OVERWRITE );
            bSuccess = true;
        }
        catch (const css::ucb::ContentCreationException &e)
        {
            SAL_WARN(
                "desktop.deployment", "exception on overwriting manifest: " << e);
        }

        if (!bSuccess)
            throw RuntimeException( "UCB transferContent() failed!",
                                    static_cast<OWeakObject *>(this) );
    }

    // xxx todo: maybe obsolete in the future
    try {
        destFolderContent.executeCommand( "flush", Any() );
    }
    catch (const ucb::UnsupportedCommandException &) {
    }
}


sal_Bool BackendImpl::PackageImpl::isBundle()
{
    return true;
}


Sequence< Reference<deployment::XPackage> > BackendImpl::PackageImpl::getBundle(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    Sequence< Reference<deployment::XPackage> > * pBundle = m_pBundle;
    if (pBundle == nullptr)
    {
        t_packagevec bundle;
        if (m_bRemoved)
        {
            bundle = getPackagesFromDb(xCmdEnv);
        }
        else
        {
            try {
                if (m_legacyBundle)
                {
                    // .zip legacy packages allow script.xlb, dialog.xlb in bundle
                    // root folder:
                    OUString mediaType;
                    // probe for script.xlb:
                    if (create_ucb_content(
                            nullptr, makeURL( m_url_expanded, "script.xlb" ),
                            xCmdEnv, false /* no throw */ )) {
                        mediaType = "application/vnd.sun.star.basic-library";
                    }
                    // probe for dialog.xlb:
                    else if (create_ucb_content(
                                 nullptr, makeURL( m_url_expanded, "dialog.xlb" ),
                                 xCmdEnv, false /* no throw */ ))
                        mediaType = "application/vnd.sun.star.dialog-library";

                    if (!mediaType.isEmpty()) {
                        const Reference<deployment::XPackage> xPackage(
                            bindBundleItem( getURL(), mediaType, false, OUString(),
                                            xCmdEnv ) );
                        if (xPackage.is())
                            bundle.push_back( xPackage );
                        // continue scanning:
                    }
                    scanLegacyBundle( bundle, getURL(),
                                      AbortChannel::get(xAbortChannel), xCmdEnv );
                }
                else
                {
                    // .oxt:
                    scanBundle( bundle, AbortChannel::get(xAbortChannel), xCmdEnv );
                }

            }
            catch (const RuntimeException &) {
                throw;
            }
            catch (const ucb::CommandFailedException &) {
                throw;
            }
            catch (const ucb::CommandAbortedException &) {
                throw;
            }
            catch (const deployment::DeploymentException &) {
                throw;
            }
            catch (const Exception &) {
                Any exc( ::cppu::getCaughtException() );
                throw deployment::DeploymentException(
                    "error scanning bundle: " + getURL(),
                    static_cast<OWeakObject *>(this), exc );
            }
        }

        // sort: schema before config data, typelibs before components:
        Sequence< Reference<deployment::XPackage> > ret( bundle.size() );
        Reference<deployment::XPackage> * pret = ret.getArray();
        sal_Int32 lower_end = 0;
        sal_Int32 upper_end = ret.getLength();
        for (auto const& elem : bundle)
        {
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                elem->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is())
            {
                const OUString mediaType( xPackageType->getMediaType() );
                OUString type, subType;
                INetContentTypeParameterList params;
                if (INetContentTypes::parse( mediaType, type, subType, &params ) &&
                    type.equalsIgnoreAsciiCase("application") &&
                    (subType.equalsIgnoreAsciiCase( "vnd.sun.star.uno-component") ||
                     subType.equalsIgnoreAsciiCase( "vnd.sun.star.configuration-data")))
                {
                    --upper_end;
                    pret[ upper_end ] = elem;
                    continue;
                }
            }
            pret[ lower_end ] = elem;
            ++lower_end;
        }
        OSL_ASSERT( lower_end == upper_end );

        const ::osl::MutexGuard guard( getMutex() );
        pBundle = m_pBundle;
        if (pBundle == nullptr) {
            m_bundle = ret;
            pBundle = &m_bundle;
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            m_pBundle = pBundle;
        }
    }
    else {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return *pBundle;
}

bool isBundle_( OUString const & mediaType )
{
    // xxx todo: additional parsing?
    return !mediaType.isEmpty() &&
        (mediaType.matchIgnoreAsciiCase( "application/vnd.sun.star.package-bundle") ||
         mediaType.matchIgnoreAsciiCase( "application/vnd.sun.star.legacy-package-bundle"));
}


Reference<deployment::XPackage> BackendImpl::PackageImpl::bindBundleItem(
    OUString const & url, OUString const & mediaType,
    bool bRemoved, OUString const & identifier,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv,
    bool notifyDetectionError )
{
    // ignore any nested bundles:
    if (isBundle_(mediaType))
        return Reference<deployment::XPackage>();

    Reference<deployment::XPackage>xPackage;
    try {
        try {
            xPackage.set( getMyBackend()->m_xRootRegistry->bindPackage(
                              url, mediaType, bRemoved, identifier, xCmdEnv ) );
            OSL_ASSERT( xPackage.is() );
        } catch (css::lang::IllegalArgumentException & e) {
            css::uno::Any exc(cppu::getCaughtException());
            throw css::lang::WrappedTargetException(
                "wrapped: " + e.Message, e.Context, exc);
        }
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const ucb::CommandFailedException &) {
        // ignore already handled error
    }
    catch (const Exception &) {
        const Any exc( ::cppu::getCaughtException() );
        if (notifyDetectionError ||
            !exc.isExtractableTo( cppu::UnoType<lang::IllegalArgumentException>::get()) )
        {
            (void)interactContinuation(
                Any( lang::WrappedTargetException("bundle item error!",
                         static_cast<OWeakObject *>(this), exc ) ),
                cppu::UnoType<task::XInteractionApprove>::get(), xCmdEnv, nullptr, nullptr );
        }
    }

    if (xPackage.is()) {
        const Reference<deployment::XPackageTypeInfo> xPackageType(
            xPackage->getPackageType() );
        OSL_ASSERT( xPackageType.is() );
        // ignore any nested bundles:
        if (xPackageType.is() && isBundle_( xPackageType->getMediaType() ))
            xPackage.clear();
    }
    return xPackage;
}


void BackendImpl::PackageImpl::scanBundle(
    t_packagevec & bundle,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    OSL_ASSERT( !m_legacyBundle );

    OUString mfUrl( makeURL( m_url_expanded, "META-INF/manifest.xml" ) );
    ::ucbhelper::Content manifestContent;
    if (! create_ucb_content(
            &manifestContent, mfUrl, xCmdEnv, false /* no throw */ ))
    {
        SAL_WARN(
            "desktop.deployment",
            "cannot create UCB Content for <" << mfUrl << ">" );
        return;
    }


    const LanguageTag& officeLocale = getOfficeLanguageTag();
    const std::vector< OUString > officeFallbacks( officeLocale.getFallbackStrings( true));
    const size_t nPenaltyMax = std::numeric_limits<size_t>::max();
    size_t descrPenalty = nPenaltyMax;
    OUString descrFile;

    const Reference<XComponentContext> xContext(
        getMyBackend()->getComponentContext() );
    Reference<packages::manifest::XManifestReader> xManifestReader =
        packages::manifest::ManifestReader::create( xContext );
    const Sequence< Sequence<beans::PropertyValue> > manifestSeq(
        xManifestReader->readManifestSequence( manifestContent.openStream() ) );
    const OUString packageRootURL( getURL() );
    for ( sal_Int32 pos = manifestSeq.getLength(); pos--; )
    {
        OUString fullPath, mediaType;
        Sequence<beans::PropertyValue> const & attribs = manifestSeq[ pos ];
        for ( sal_Int32 i = attribs.getLength(); i--; )
        {
            if (!(fullPath.isEmpty() || mediaType.isEmpty()))
                break;
            if ( attribs[i].Name == "FullPath" )
                attribs[i].Value >>= fullPath;
            else if ( attribs[i].Name == "MediaType" )
                attribs[i].Value >>= mediaType;
        }

        if ( fullPath.isEmpty() || mediaType.isEmpty() || mediaType == "text/xml" )// opt: exclude common text/xml
            continue;

        OUString type, subType;
        INetContentTypeParameterList params;
        if (! INetContentTypes::parse( mediaType, type, subType, &params ))
            continue;

        {
            auto const iter = params.find("platform");
            if (iter != params.end() && !platform_fits(iter->second.m_sValue))
                continue;
        }
        const OUString url( makeURL( packageRootURL, fullPath ) );

        // check for bundle description:
        if (type.equalsIgnoreAsciiCase("application") &&
            subType.equalsIgnoreAsciiCase( "vnd.sun.star.package-bundle-description"))
        {
            // check locale:
            auto const iter = params.find("locale");
            if (iter == params.end())
            {
                if (descrFile.isEmpty())
                    descrFile = url;
            }
            else {
                // match best locale:
                LanguageTag descrTag(iter->second.m_sValue);
                if (officeLocale.getLanguage() == descrTag.getLanguage())
                {
                    size_t nPenalty = nPenaltyMax;
                    const std::vector< OUString > descrFallbacks( descrTag.getFallbackStrings( true));
                    for (size_t o=0; o < officeFallbacks.size() && nPenalty == nPenaltyMax; ++o)
                    {
                        for (size_t d=0; d < descrFallbacks.size() && nPenalty == nPenaltyMax; ++d)
                        {
                            if (officeFallbacks[o] == descrFallbacks[d])
                            {
                                // The last fallbacks are always language-only
                                // fallbacks, so we _will_ have _some_ match if
                                // we ever entered the overall if() condition.
                                nPenalty = o * 1000 + d;
                                if (descrPenalty > nPenalty)
                                {
                                    descrPenalty = nPenalty;
                                    descrFile = url;
                                }
                            }
                        }
                    }
                }
                // TODO: we could break here if descrPenalty==0 for an exact
                // match of officeLocale, but the previous code didn't; are
                // there side effects?
            }
            continue;
        }

        checkAborted( abortChannel );

        //We make sure that we only create one XPackage for a particular URL.
        //Sometime programmers insert the same URL several times in the manifest
        //which may lead to DisposedExceptions.
        if (std::none_of(bundle.begin(), bundle.end(), XPackage_eq(url)))
        {
            const Reference<deployment::XPackage> xPackage(
                bindBundleItem( url, mediaType, false, OUString(), xCmdEnv ) );
            if (xPackage.is())
                bundle.push_back( xPackage );
        }
        else
        {
            SAL_WARN("desktop.deployment", "manifest.xml contains a duplicate entry (from " << url << ")");
        }
    }

    if (!descrFile.isEmpty())
    {
        ::ucbhelper::Content descrFileContent;
        if (create_ucb_content( &descrFileContent, descrFile,
                                xCmdEnv, false /* no throw */ ))
        {
            // patch description:
            std::vector<sal_Int8> bytes( readFile( descrFileContent ) );
            OUStringBuffer buf;
            if ( !bytes.empty() )
            {
                buf.append( OUString( reinterpret_cast<sal_Char const *>(
                                          bytes.data() ),
                                      bytes.size(), RTL_TEXTENCODING_UTF8 ) );
            }
            else
            {
                buf.append( Package::getDescription() );
            }
            m_oldDescription = buf.makeStringAndClear();
        }
    }
}


void BackendImpl::PackageImpl::scanLegacyBundle(
    t_packagevec & bundle,
    OUString const & url,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv,
    bool skip_registration )
{
    ::ucbhelper::Content ucbContent(
        url, xCmdEnv, getMyBackend()->getComponentContext() );

    // check for platform paths:
    const OUString title( StrTitle::getTitle( ucbContent ) );
    if (title.endsWithIgnoreAsciiCase( ".plt" ) &&
        !platform_fits( title.copy( 0, title.getLength() - 4 ) )) {
        return;
    }
    if (title.endsWithIgnoreAsciiCase("skip_registration") )
        skip_registration = true;

    Sequence<OUString> ar { OUString("Title"), OUString("IsFolder") };
    Reference<sdbc::XResultSet> xResultSet( ucbContent.createCursor( ar ) );
    while (xResultSet->next())
    {
        checkAborted( abortChannel );

        const Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
        const OUString title_enc( ::rtl::Uri::encode(
                                      xRow->getString( 1 /* Title */ ),
                                      rtl_UriCharClassPchar,
                                      rtl_UriEncodeIgnoreEscapes,
                                      RTL_TEXTENCODING_UTF8 ) );
        const OUString path( makeURL( url, title_enc ) );

        OUString mediaType;
        const Reference<deployment::XPackage> xPackage(
            bindBundleItem( path, OUString() /* detect */, false, OUString(),
                            xCmdEnv, false /* ignore detection errors */ ) );
        if (xPackage.is()) {
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                xPackage->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is())
                mediaType = xPackageType->getMediaType();

            if (skip_registration &&
                // xxx todo: additional parsing?
                mediaType.matchIgnoreAsciiCase("application/vnd.sun.star.uno-component"))
                continue;

            bundle.push_back( xPackage );
        }

        if (mediaType.isEmpty() ||
            // script.xlb, dialog.xlb can be met everywhere:
            mediaType.matchIgnoreAsciiCase("application/vnd.sun.star.basic-library") ||
            mediaType.matchIgnoreAsciiCase("application/vnd.sun.star.dialog-library"))
        {
            if (xRow->getBoolean( 2 /* IsFolder */ )) { // recurse into folder:
                scanLegacyBundle(
                    bundle, path, abortChannel, xCmdEnv, skip_registration );
            }
        }
    }
}

OUString BackendImpl::PackageImpl::getDisplayName()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    OUString sName = getDescriptionInfoset().getLocalizedDisplayName();
    if (sName.isEmpty())
        return m_displayName;
    else
        return sName;
}

std::vector<Reference<deployment::XPackage> >
BackendImpl::PackageImpl::getPackagesFromDb(
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    std::vector<Reference<deployment::XPackage> > retVector;

    for (auto const& item : m_dbData.items)
    {
        Reference<deployment::XPackage> xExtension =
            bindBundleItem(item.first, item.second, true, m_identifier, xCmdEnv);
        OSL_ASSERT(xExtension.is());
        if (xExtension.is())
            retVector.push_back(xExtension);
    }

    return retVector;
}

} // anon namespace


Reference<deployment::XPackageRegistry> create(
    Reference<deployment::XPackageRegistry> const & xRootRegistry,
    OUString const & context, OUString const & cachePath,
    Reference<XComponentContext> const & xComponentContext )
{
    Sequence<Any> args(cachePath.isEmpty() ? 1 : 3 );
    args[ 0 ] <<= context;
    if (!cachePath.isEmpty()) {
        args[ 1 ] <<= cachePath;
        args[ 2 ] <<= false; // readOnly
    }
    return new BackendImpl( args, xComponentContext, xRootRegistry );
}

} // namespace bundle
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
