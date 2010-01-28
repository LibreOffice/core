/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_package.cxx,v $
 * $Revision: 1.34.16.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_package.hrc"
#include "dp_backend.h"
#include "dp_ucb.h"
#include "dp_interact.h"
#include "dp_dependencies.hxx"
#include "dp_platform.hxx"
#include "dp_description.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_identifier.hxx"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "ucbhelper/content.hxx"
#include "svtools/inettype.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/makesequence.hxx"
#include "comphelper/sequence.hxx"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/graphic/XGraphic.hpp"
#include "com/sun/star/graphic/XGraphicProvider.hpp"
#include "com/sun/star/io/XOutputStream.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/task/InteractionClassification.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/ucb/XInteractionReplaceExistingData.hpp"
#include "com/sun/star/ucb/NameClashResolveRequest.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/UnsupportedCommandException.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/packages/manifest/XManifestReader.hpp"
#include "com/sun/star/packages/manifest/XManifestWriter.hpp"
#include "com/sun/star/deployment/DependencyException.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/LicenseIndividualAgreementException.hpp"
#include "com/sun/star/deployment/PlatformException.hpp"
#include "com/sun/star/xml/dom/XDocumentBuilder.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"

#include <vector>
#include <stdio.h>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace bundle {
namespace {

typedef cppu::ImplInheritanceHelper1<PackageRegistryBackend,
                                     lang::XServiceInfo> ImplBaseT;

//==============================================================================
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

        Reference<deployment::XPackage> bindBundleItem(
            OUString const & url, OUString const & mediaType,
            Reference<XCommandEnvironment> const & xCmdEnv,
            bool notifyDetectionError = true );

        typedef ::std::vector< Reference<deployment::XPackage> > t_packagevec;
        void scanBundle(
            t_packagevec & bundle,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );
        void scanLegacyBundle(
            t_packagevec & bundle,
            OUString const & url,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv,
            bool skip_registration = false );

        bool checkPlatform(
            css::uno::Reference< css::ucb::XCommandEnvironment > const &  environment);

        bool checkDependencies(
            css::uno::Reference< css::ucb::XCommandEnvironment > const &
                environment,
            ExtensionDescription const & description);
            // throws css::uno::RuntimeException,
            // css::deployment::DeploymentException

        ::sal_Bool checkLicense(
            css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
            ExtensionDescription const& description, bool bInstalled,
            OUString const & aContextName )
                throw (css::deployment::DeploymentException,
                    css::ucb::CommandFailedException,
                    css::ucb::CommandAbortedException,
                    css::uno::RuntimeException);
        // @throws DeploymentException
        OUString getTextFromURL(
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
            const OUString& licenseUrl);

        DescriptionInfoset getDescriptionInfoset();

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



    public:
        PackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url,
            OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool legacyBundle )
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType ),
              m_url_expanded( expandUnoRcUrl( url ) ),
              m_legacyBundle( legacyBundle ),
              m_pBundle( 0 )
            {}

        // XPackage
        virtual sal_Bool SAL_CALL isBundle() throw (RuntimeException);
        virtual Sequence< Reference<deployment::XPackage> > SAL_CALL getBundle(
            Reference<task::XAbortChannel> const & xAbortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv )
            throw (deployment::DeploymentException,
                   CommandFailedException, CommandAbortedException,
                   lang::IllegalArgumentException, RuntimeException);
        virtual OUString SAL_CALL getDescription() throw (RuntimeException);
        virtual void SAL_CALL exportTo(
            OUString const & destFolderURL, OUString const & newTitle,
            sal_Int32 nameClashAction,
            Reference<XCommandEnvironment> const & xCmdEnv )
            throw (CommandFailedException, CommandAbortedException,
                   RuntimeException);

        virtual ::sal_Bool SAL_CALL checkPrerequisites(
            const css::uno::Reference< css::task::XAbortChannel >& xAbortChannel,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
            ::sal_Bool bInstalled, OUString const & aContextName)
            throw (css::deployment::DeploymentException,
                css::ucb::CommandFailedException,
                css::ucb::CommandAbortedException,
                css::uno::RuntimeException);

        virtual ::sal_Bool SAL_CALL checkDependencies(
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv )
            throw (css::deployment::DeploymentException,
                css::ucb::CommandFailedException,
                css::uno::RuntimeException);

        virtual beans::Optional<OUString> SAL_CALL getIdentifier()
            throw (RuntimeException);

        virtual OUString SAL_CALL getVersion() throw (RuntimeException);

        virtual Sequence<OUString> SAL_CALL getUpdateInformationURLs()
            throw (RuntimeException);

        virtual css::beans::StringPair SAL_CALL getPublisherInfo() throw (css::uno::RuntimeException);

        virtual OUString SAL_CALL getDisplayName() throw (RuntimeException);
        virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL getIcon( ::sal_Bool bHighContrast ) throw (css::uno::RuntimeException);
    };
    friend class PackageImpl;

    Reference<deployment::XPackageRegistry> m_xRootRegistry;
    const Reference<deployment::XPackageTypeInfo> m_xBundleTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xLegacyBundleTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

    virtual void SAL_CALL disposing();

public:
    BackendImpl(
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xComponentContext,
        Reference<deployment::XPackageRegistry> const & xRootRegistry );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const& name )
        throw (RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);

    using ImplBaseT::disposing;
};

//Used to find a XPackage with a particular URL
class XPackage_eq : public std::unary_function<Reference<deployment::XPackage>, bool>
{
    OUString m_URL;
public:
    explicit XPackage_eq(const OUString & s) : m_URL(s) {}
    bool operator() (const Reference<deployment::XPackage> & p) const
    {
        return m_URL.equals(p->getURL());
    }
};

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext,
    Reference<deployment::XPackageRegistry> const & xRootRegistry )
    : ImplBaseT( args, xComponentContext ),
      m_xRootRegistry( xRootRegistry ),
      m_xBundleTypeInfo( new Package::TypeInfo(
                             OUSTR("application/vnd.sun.star.package-bundle"),
                             OUSTR("*.oxt;*.uno.pkg"),
                             getResourceString(RID_STR_PACKAGE_BUNDLE),
                             RID_IMG_DEF_PACKAGE_BUNDLE,
                             RID_IMG_DEF_PACKAGE_BUNDLE_HC ) ),
      m_xLegacyBundleTypeInfo( new Package::TypeInfo(
                                   OUSTR("application/"
                                         "vnd.sun.star.legacy-package-bundle"),
                                   OUSTR("*.zip"),
                                   m_xBundleTypeInfo->getShortDescription(),
                                   RID_IMG_DEF_PACKAGE_BUNDLE,
                                   RID_IMG_DEF_PACKAGE_BUNDLE_HC ) ),
      m_typeInfos( 2 )
{
    m_typeInfos[ 0 ] = m_xBundleTypeInfo;
    m_typeInfos[ 1 ] = m_xLegacyBundleTypeInfo;
}

//______________________________________________________________________________
void BackendImpl::disposing()
{
    m_xRootRegistry.clear();
    PackageRegistryBackend::disposing();
}

// XServiceInfo
OUString BackendImpl::getImplementationName() throw (RuntimeException)
{
    return OUSTR("com.sun.star.comp.deployment.bundle.PackageRegistryBackend");
}

sal_Bool BackendImpl::supportsService( OUString const& name )
    throw (RuntimeException)
{
    return getSupportedServiceNames()[0].equals(name);
}

Sequence<OUString> BackendImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return comphelper::makeSequence(
        OUString::createFromAscii(BACKEND_SERVICE_NAME) );
}

// XPackageRegistry
//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return m_typeInfos;
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
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ))
        {
            const OUString title( ucbContent.getPropertyValue(
                                      StrTitle::get() ).get<OUString>() );
            if (title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".oxt") ) ||
                title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".uno.pkg") ))
                mediaType = OUSTR("application/vnd.sun.star.package-bundle");
            else if (title.endsWithIgnoreAsciiCaseAsciiL(
                         RTL_CONSTASCII_STRINGPARAM(".zip") ))
                mediaType =
                    OUSTR("application/vnd.sun.star.legacy-package-bundle");
        }
        if (mediaType.getLength() == 0)
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
            ::ucbhelper::Content ucbContent( url, xCmdEnv );
            if (subType.EqualsIgnoreCaseAscii("vnd.sun.star.package-bundle")) {
                return new PackageImpl(
                    this, url, ucbContent.getPropertyValue(
                        StrTitle::get() ).get<OUString>(),
                    m_xBundleTypeInfo, false );
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.legacy-package-bundle")) {
                return new PackageImpl(
                    this, url, ucbContent.getPropertyValue(
                        StrTitle::get() ).get<OUString>(),
                    m_xLegacyBundleTypeInfo, true );
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//##############################################################################

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
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    const Sequence< Reference<deployment::XPackage> > bundle(
        getBundle( abortChannel.get(), xCmdEnv ) );
    bool reg = false;
    bool present = false;
    bool ambig = false;
    for ( sal_Int32 pos = bundle.getLength(); pos--; )
    {
        Reference<deployment::XPackage> const & xPackage = bundle[ pos ];
        //disregard executable (application/vnd.sun.star.executable)
        //it will not be disabled/enabled.
        OUString sType = xPackage->getPackageType()->getMediaType();
        if (sType.equals(OUSTR("application/vnd.sun.star.executable")))
            continue;

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

OUString BackendImpl::PackageImpl::getTextFromURL(
    const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
    const OUString& licenseUrl)
{
    try
    {
        ::ucbhelper::Content descContent(licenseUrl, xCmdEnv);
        ::rtl::ByteSequence seq = dp_misc::readFile(descContent);
        return OUString( reinterpret_cast<sal_Char const *>(
            seq.getConstArray()), seq.getLength(), RTL_TEXTENCODING_UTF8);
    }
    catch (css::uno::Exception&)
    {
        Any exc( ::cppu::getCaughtException() );
            throw css::deployment::DeploymentException(
                OUSTR("Could not read file ") + licenseUrl, 0, exc);
    }

}

DescriptionInfoset BackendImpl::PackageImpl::getDescriptionInfoset()
{
    css::uno::Reference< css::xml::dom::XNode > root;
    try {
        root =
            ExtensionDescription(
                getMyBackend()->getComponentContext(), m_url_expanded,
                css::uno::Reference< css::ucb::XCommandEnvironment >()).
            getRootElement();
    } catch (NoDescriptionException &) {
    } catch (css::deployment::DeploymentException & e) {
        throw RuntimeException(
            (OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.deployment.DeploymentException: ")) +
             e.Message),
            static_cast< OWeakObject * >(this));
    }
    return DescriptionInfoset(getMyBackend()->getComponentContext(), root);
}

bool BackendImpl::PackageImpl::checkPlatform(
    css::uno::Reference< css::ucb::XCommandEnvironment > const &  environment)
{
    bool ret = false;
    DescriptionInfoset info(getDescriptionInfoset());
    Sequence<OUString> platforms(info.getSupportedPlaforms());
    if (hasValidPlatform(platforms))
    {
        ret = true;
    }
    else
    {
        ret = false;
        rtl::OUString msg(
            RTL_CONSTASCII_USTRINGPARAM("unsupported platform"));
        Any e(
            css::deployment::PlatformException(
                msg, static_cast<OWeakObject *>(this), this));
        if (!interactContinuation(
                e, cppu::UnoType< css::task::XInteractionApprove >::get(),
                environment, NULL, NULL))
        {
            throw css::deployment::DeploymentException(
                msg, static_cast<OWeakObject *>(this), e);
        }
    }
    return ret;
}


bool BackendImpl::PackageImpl::checkDependencies(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & environment,
    ExtensionDescription const & description)
{
    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
        unsatisfied(
            dp_misc::Dependencies::check(
                DescriptionInfoset(
                    getMyBackend()->getComponentContext(),
                    description.getRootElement())));
    if (unsatisfied.getLength() == 0) {
        return true;
    } else {
        rtl::OUString msg(
            RTL_CONSTASCII_USTRINGPARAM("unsatisfied dependencies"));
        Any e(
            css::deployment::DependencyException(
                msg, static_cast<OWeakObject *>(this), unsatisfied));
        if (!interactContinuation(
                e, cppu::UnoType< css::task::XInteractionApprove >::get(),
                environment, NULL, NULL))
        {
            throw css::deployment::DeploymentException(
                msg, static_cast<OWeakObject *>(this), e);
        }
        return false;
    }
}

::sal_Bool BackendImpl::PackageImpl::checkLicense(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
    ExtensionDescription const & desc, bool bInstalled, OUString const & aContextName)
        throw (css::deployment::DeploymentException,
            css::ucb::CommandFailedException,
            css::ucb::CommandAbortedException,
            css::uno::RuntimeException)
{
    try
    {
        css::uno::Reference<css::xml::dom::XNode> xRoot = desc.getRootElement();
        css::uno::Reference<css::xml::xpath::XXPathAPI> xPath =
            getDescriptionInfoset().getXpath();

        css::uno::Reference<css::xml::dom::XNode> nodeSimpleLic;
        try {
            nodeSimpleLic = xPath->selectSingleNode(xRoot,
            OUSTR("/desc:description/desc:registration/desc:simple-license"));
        } catch (css::xml::xpath::XPathException &) {
            // ignore
        }

        if (!nodeSimpleLic.is())
            return true;
        OUString sLic = getDescriptionInfoset().getLocalizedLicenseURL();
        //If we do not get a localized licence then there is an error in the description.xml
        //This should be handled by using a validating parser. Therefore we assume that no
        //license is available.
        if (sLic.getLength() == 0)
            throw css::deployment::DeploymentException(
                OUSTR("Could not obtain path to license. Possible error in description.xml"), 0, Any());
        OUString sHref = desc.getExtensionRootUrl() + OUSTR("/") + sLic;
           OUString sLicense = getTextFromURL(xCmdEnv, sHref);
        //determine who has to agree to the license
        css::uno::Reference<css::xml::xpath::XXPathObject> nodeAttribWho3 =
            xPath->eval(nodeSimpleLic,
            OUSTR("@accept-by"));
        OUString sAccept = nodeAttribWho3->getString().trim();
        //check correct value for attribute
        if ( ! (sAccept.equals(OUSTR("user")) || sAccept.equals(OUSTR("admin"))))
            throw css::deployment::DeploymentException(
                OUSTR("Could not obtain attribute simple-lincense@accept-by or it has no valid value"), 0, Any());

        //If if @accept-by="user" then every user needs to accept the license before it can be installed.
        //Therefore we must prevent the installation as shared extension.
        OSL_ASSERT(aContextName.getLength());
        if (sAccept.equals(OUSTR("user")) && aContextName.equals(OUSTR("shared")))
        {
        css::deployment::LicenseIndividualAgreementException exc =
            css::deployment::LicenseIndividualAgreementException(OUString(), 0, m_name);

            bool approve = false;
            bool abort = false;
            if (! interactContinuation(
                Any(exc), task::XInteractionApprove::static_type(), xCmdEnv, &approve, &abort ))
                throw css::deployment::DeploymentException(
                    OUSTR("Could not interact with user."), 0, Any());
               if (abort == true)
                return false;
            //We should always prevent installation
            OSL_ASSERT(0);
        }

        //determine optional attribute simple-license@suppressOnUpdate
        css::uno::Reference<css::xml::dom::XElement> elemSimpleLic(nodeSimpleLic, css::uno::UNO_QUERY_THROW);
        sal_Bool bSuppress = sal_False;
        if (elemSimpleLic->hasAttribute(OUSTR("suppress-on-update")))
        {
            if (elemSimpleLic->getAttribute(OUSTR("suppress-on-update")).equals(OUSTR("true")))
                bSuppress = sal_True;
        }

        //Only use interaction if there is no version of this extension already installed
        //and the suppress-on-update flag is not set for the new extension
        // bInstalled | bSuppress | show license
        //----------------------------------------
        //      0     |      0    |     1
        //      0     |      1    |     1
        //      1     |      0    |     1
        //      1     |      1    |     0

        if ( !(bInstalled && bSuppress))
        {
            css::deployment::LicenseException licExc =
                css::deployment::LicenseException(OUString(), 0, m_name, sLicense);
            bool approve = false;
            bool abort = false;
            if (! interactContinuation(
                Any(licExc), task::XInteractionApprove::static_type(), xCmdEnv, &approve, &abort ))
                throw css::deployment::DeploymentException(
                    OUSTR("Could not interact with user."), 0, Any());

            if (approve == true)
                return true;
            else
                return false;
                //throw css::deployment::DeploymentException(
                //    OUSTR("Extension Manager: User declined the license."),
                //    static_cast<OWeakObject*>(this),
                //    Any( css::deployment::LicenseException(OUSTR("User declined the license."), 0, m_name, sLicense)));
        }
        return true;
    } catch (css::ucb::CommandFailedException&) {
        throw;
    } catch (css::ucb::CommandAbortedException&) {
        throw;
    } catch (css::deployment::DeploymentException&) {
        throw;
    } catch (css::uno::RuntimeException&) {
        throw;
    } catch (css::uno::Exception&) {
        Any anyExc = cppu::getCaughtException();
        throw css::deployment::DeploymentException(OUSTR("Unexpected exception"), 0, anyExc);
    }
}

::sal_Bool BackendImpl::PackageImpl::checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >&,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv,
        sal_Bool bInstalled, OUString const & aContextName)
        throw (css::deployment::DeploymentException,
            css::ucb::CommandFailedException,
            css::ucb::CommandAbortedException,
            css::uno::RuntimeException)
{
    std::auto_ptr<ExtensionDescription> spDescription;
    try {
        spDescription.reset(
            new ExtensionDescription(
                getMyBackend()->getComponentContext(),
                m_url_expanded,
                xCmdEnv));
    } catch (NoDescriptionException& ) {
        return sal_True;
    }
    return checkPlatform(xCmdEnv)
        && checkDependencies(xCmdEnv, *spDescription)
        && checkLicense(xCmdEnv, *spDescription, bInstalled, aContextName);
}

::sal_Bool BackendImpl::PackageImpl::checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv )
        throw (css::deployment::DeploymentException,
            css::ucb::CommandFailedException,
            css::uno::RuntimeException)
{
    std::auto_ptr<ExtensionDescription> spDescription;
    try {
        spDescription.reset(
            new ExtensionDescription( getMyBackend()->getComponentContext(), m_url_expanded, xCmdEnv ));
    } catch (NoDescriptionException& ) {
        return sal_True;
    }
    return checkDependencies(xCmdEnv, *spDescription);
}

beans::Optional<OUString> BackendImpl::PackageImpl::getIdentifier()
    throw (RuntimeException)
{
    return beans::Optional<OUString>(
        true,
        dp_misc::generateIdentifier(
            getDescriptionInfoset().getIdentifier(), m_name));
}

OUString BackendImpl::PackageImpl::getVersion() throw (RuntimeException)
{
    return getDescriptionInfoset().getVersion();
}

Sequence<OUString> BackendImpl::PackageImpl::getUpdateInformationURLs()
    throw (RuntimeException)
{
    return getDescriptionInfoset().getUpdateInformationUrls();
}

beans::StringPair BackendImpl::PackageImpl::getPublisherInfo()
    throw (RuntimeException)
{
    ::std::pair< OUString, OUString > aInfo = getDescriptionInfoset().getLocalizedPublisherNameAndURL();
    beans::StringPair aStrPair( aInfo.first, aInfo.second );
    return aStrPair;
}

//______________________________________________________________________________
uno::Reference< graphic::XGraphic > BackendImpl::PackageImpl::getIcon( sal_Bool bHighContrast )
    throw ( RuntimeException )
{
    uno::Reference< graphic::XGraphic > xGraphic;

    OUString aIconURL = getDescriptionInfoset().getIconURL( bHighContrast );
    if ( aIconURL.getLength() )
    {
        OUString aFullIconURL = m_url_expanded + OUSTR("/") + aIconURL;

        uno::Reference< XComponentContext > xContext( getMyBackend()->getComponentContext() );
        uno::Reference< graphic::XGraphicProvider > xGraphProvider(
                        xContext->getServiceManager()->createInstanceWithContext( OUSTR( "com.sun.star.graphic.GraphicProvider" ), xContext ),
                        uno::UNO_QUERY );

        if ( xGraphProvider.is() )
        {
            uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
            aMediaProps[0].Name = OUSTR( "URL" );
            aMediaProps[0].Value <<= aFullIconURL;

            xGraphic = xGraphProvider->queryGraphic( aMediaProps );
        }
    }

    return xGraphic;
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    const Sequence< Reference<deployment::XPackage> > bundle(
        getBundle( abortChannel.get(), xCmdEnv ) );

    if (doRegisterPackage)
    {
        const sal_Int32 len = bundle.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            checkAborted(abortChannel);
            Reference<deployment::XPackage> const & xPackage = bundle[ pos ];
            Reference<task::XAbortChannel> xSubAbortChannel(
                xPackage->createAbortChannel() );
            AbortChannel::Chain chain( abortChannel, xSubAbortChannel );
            try {
                xPackage->registerPackage( xSubAbortChannel, xCmdEnv );
            }
            catch (RuntimeException &) {
                throw;
            }
            catch (CommandAbortedException &) {
                throw;
            }
            catch (Exception &) {
                // CommandFailedException, DeploymentException:
                Any exc( ::cppu::getCaughtException() );
                // try to handle exception, notify:
                bool approve = false, abort = false;
                if (! interactContinuation(
                        Any( lang::WrappedTargetException(
                                 OUSTR("bundle item registration error!"),
                                 static_cast<OWeakObject *>(this), exc ) ),
                        task::XInteractionApprove::static_type(), xCmdEnv,
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
                    ProgressLevel progress(
                        xCmdEnv, OUSTR("rollback...") );
                    // try rollback
                    for ( ; pos--; )
                    {
                        try {
                            bundle[ pos ]->revokePackage(
                                xSubAbortChannel, xCmdEnv );
                        }
                        catch (RuntimeException &) {
                            throw;
                        }
                        catch (CommandAbortedException &) {
                            throw;
                        }
                        catch (Exception &) {
                            // bundle rollback error:
                            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                                            ::comphelper::anyToString(
                                                ::cppu::getCaughtException() ),
                                            RTL_TEXTENCODING_UTF8 ).getStr() );
                            // ignore any errors of rollback
                        }
                    }
                    progress.update( OUSTR("rollback finished.") );
                }

                deployment::DeploymentException dpExc;
                if (exc >>= dpExc) {
                    throw CommandFailedException(
                        dpExc.Message, dpExc.Context, dpExc.Cause );
                }
                else {
                    // rethrow CommandFailedException
                    ::cppu::throwException(exc);
                }
            }
        }
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
                bundle[ pos ]->revokePackage( xSubAbortChannel, xCmdEnv );
            }
            catch (RuntimeException &) {
                throw;
            }
            catch (CommandAbortedException &) {
                throw;
            }
            catch (Exception &) {
                // CommandFailedException, DeploymentException:
                Any exc( ::cppu::getCaughtException() );
                // try to handle exception, notify:
                bool approve = false, abort = false;
                if (! interactContinuation(
                        Any( lang::WrappedTargetException(
                                 OUSTR("bundle item revocation error!"),
                                 static_cast<OWeakObject *>(this), exc ) ),
                        task::XInteractionApprove::static_type(), xCmdEnv,
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
    }
}

//______________________________________________________________________________
OUString BackendImpl::PackageImpl::getDescription() throw (RuntimeException)
{
    const OUString sRelativeURL(getDescriptionInfoset().getLocalizedDescriptionURL());
    OUString sDescription;
    if (sRelativeURL.getLength())
    {
        OUString sURL = m_url_expanded + OUSTR("/") + sRelativeURL;
        sDescription = getTextFromURL(
            css::uno::Reference< css::ucb::XCommandEnvironment >(), sURL);

    }
    if (sDescription.getLength())
        return sDescription;
    else if(m_oldDescription.getLength())
        return m_oldDescription;
    else
        return OUString();
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::exportTo(
    OUString const & destFolderURL, OUString const & newTitle,
    sal_Int32 nameClashAction, Reference<XCommandEnvironment> const & xCmdEnv )
    throw (CommandFailedException, CommandAbortedException, RuntimeException)
{
    ::ucbhelper::Content sourceContent( m_url_expanded, xCmdEnv );
    OUString title(newTitle);
    if (title.getLength() == 0)
        sourceContent.getPropertyValue( StrTitle::get() ) >>= title;
    OUString destURL( makeURL( destFolderURL, ::rtl::Uri::encode(
                                   title, rtl_UriCharClassPchar,
                                   rtl_UriEncodeIgnoreEscapes,
                                   RTL_TEXTENCODING_UTF8 ) ) );

    if (nameClashAction == NameClash::ASK)
    {
        if (create_ucb_content(
                0, destURL, xCmdEnv, false /* no throw */ )) {
            bool replace = false, abort = false;
            if (! interactContinuation(
                    Any( NameClashResolveRequest(
                             OUSTR("file already exists: ") + title,
                             static_cast<OWeakObject *>(this),
                             task::InteractionClassification_QUERY,
                             destFolderURL, title, OUString() ) ),
                    XInteractionReplaceExistingData::static_type(), xCmdEnv,
                    &replace, &abort ) || !replace) {
                return;
            }
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

    ::ucbhelper::Content destFolderContent( destFolder, xCmdEnv );
    {
        // transfer every item of folder into zip:
        Reference<sdbc::XResultSet> xResultSet(
            sourceContent.createCursor(
                Sequence<OUString>(),
                ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
        ProgressLevel progress( xCmdEnv, OUString() );
        while (xResultSet->next())
        {
            ::ucbhelper::Content subContent(
                Reference<XContentAccess>(
                    xResultSet, UNO_QUERY_THROW )->queryContent(), xCmdEnv );
            if (! destFolderContent.transferContent(
                    subContent, ::ucbhelper::InsertOperation_COPY,
                    OUString(), NameClash::OVERWRITE ))
                throw RuntimeException( OUSTR("UCB transferContent() failed!"),
                                        static_cast<OWeakObject *>(this) );
            progress.update( Any() ); // animating progress bar
        }
    }

    // assure META-INF folder:
    ::ucbhelper::Content metainfFolderContent;
    create_folder( &metainfFolderContent,
                   makeURL( destFolderContent.getURL(), OUSTR("META-INF") ),
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
        catch (deployment::DeploymentException &) {
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            ::comphelper::anyToString(
                                ::cppu::getCaughtException() ),
                            RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        catch (lang::IllegalArgumentException & exc) {
            (void) exc;
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }

        ::std::vector< Sequence<beans::PropertyValue> > manifest;
        manifest.reserve( bundle.getLength() );
        sal_Int32 baseURLlen = m_url_expanded.getLength();
        Reference<deployment::XPackage> const *pbundle = bundle.getConstArray();
        const OUString strMediaType = OUSTR("MediaType");
        const OUString strFullPath = OUSTR("FullPath");
        const OUString strIsFolder = OUSTR("IsFolder");
        for ( sal_Int32 pos = bundle.getLength(); pos--; )
        {
            Reference<deployment::XPackage> const & xPackage = pbundle[ pos ];
            OUString url_( expandUnoRcUrl( xPackage->getURL() ) );
            OSL_ASSERT( url_.getLength() >= baseURLlen );
            OUString fullPath;
            if (url_.getLength() > baseURLlen)
                fullPath = url_.copy( baseURLlen + 1 );
            ::ucbhelper::Content ucbContent( url_, xCmdEnv );
            if (ucbContent.getPropertyValue(strIsFolder).get<bool>())
                fullPath += OUSTR("/");
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
                mediaType = OUSTR("unknown");
            pattribs[ 1 ].Value <<= mediaType;
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
            xPipe, comphelper::containerToSequence(manifest) );

        // write buffered pipe data to content:
        ::ucbhelper::Content manifestContent(
            makeURL( metainfFolderContent.getURL(), OUSTR("manifest.xml") ),
            xCmdEnv );
        manifestContent.writeStream(
            Reference<io::XInputStream>( xPipe, UNO_QUERY_THROW ),
            true /* replace existing */ );
    }
    else
    {
        // overwrite manifest.xml:
        ::ucbhelper::Content manifestContent;
        if ( ! create_ucb_content(
            &manifestContent,
            makeURL( m_url_expanded, OUSTR("META-INF/manifest.xml") ),
            xCmdEnv, false ) )
        {
            OSL_ENSURE( 0, "### missing META-INF/manifest.xml file!" );
            return;
        }

        if (! metainfFolderContent.transferContent(
                manifestContent, ::ucbhelper::InsertOperation_COPY,
                OUString(), NameClash::OVERWRITE ))
            throw RuntimeException( OUSTR("UCB transferContent() failed!"),
                                    static_cast<OWeakObject *>(this) );
    }

    // xxx todo: maybe obsolete in the future
    try {
        destFolderContent.executeCommand( OUSTR("flush"), Any() );
    }
    catch (UnsupportedCommandException &) {
    }
}

//______________________________________________________________________________
sal_Bool BackendImpl::PackageImpl::isBundle() throw (RuntimeException)
{
    return true;
}

//______________________________________________________________________________
Sequence< Reference<deployment::XPackage> > BackendImpl::PackageImpl::getBundle(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    Sequence< Reference<deployment::XPackage> > * pBundle = m_pBundle;
    if (pBundle == 0)
    {
        t_packagevec bundle;
        try {
            if (m_legacyBundle)
            {
                // .zip legacy packages allow script.xlb, dialog.xlb in bundle
                // root folder:
                OUString mediaType;
                // probe for script.xlb:
                if (create_ucb_content(
                        0, makeURL( m_url_expanded, OUSTR("script.xlb") ),
                        xCmdEnv, false /* no throw */ )) {
                    mediaType = OUSTR("application/vnd.sun.star.basic-library");
                }
                // probe for dialog.xlb:
                else if (create_ucb_content(
                             0, makeURL( m_url_expanded, OUSTR("dialog.xlb") ),
                             xCmdEnv, false /* no throw */ ))
                    mediaType = OUSTR("application/vnd.sun.star."
                                      "dialog-library");

                if (mediaType.getLength() > 0) {
                    const Reference<deployment::XPackage> xPackage(
                        bindBundleItem( getURL(), mediaType, xCmdEnv ) );
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
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                (*iPos)->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is()) {
                const OUString mediaType( xPackageType->getMediaType() );
                String type, subType;
                INetContentTypeParameterList params;
                if (INetContentTypes::parse(
                        mediaType, type, subType, &params ) &&
                    type.EqualsIgnoreCaseAscii("application") &&
                    (subType.EqualsIgnoreCaseAscii(
                        "vnd.sun.star.uno-component") ||
                     subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.configuration-data")))
                {
                    --upper_end;
                    pret[ upper_end ] = *iPos;
                    continue;
                }
            }
            pret[ lower_end ] = *iPos;
            ++lower_end;
        }
        OSL_ASSERT( lower_end == upper_end );

        const ::osl::MutexGuard guard( getMutex() );
        pBundle = m_pBundle;
        if (pBundle == 0) {
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

inline bool isBundle_( OUString const & mediaType )
{
    // xxx todo: additional parsing?
    return mediaType.getLength() > 0 &&
        (mediaType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM(
                "application/vnd.sun.star.package-bundle") ) ||
         mediaType.matchIgnoreAsciiCaseAsciiL(
             RTL_CONSTASCII_STRINGPARAM(
                 "application/vnd.sun.star.legacy-package-bundle") ));
}

//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::PackageImpl::bindBundleItem(
    OUString const & url, OUString const & mediaType,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool notifyDetectionError )
{
    // ignore any nested bundles:
    if (isBundle_(mediaType))
        return Reference<deployment::XPackage>();

    Reference<deployment::XPackage>xPackage;
    try {
        xPackage.set( getMyBackend()->m_xRootRegistry->bindPackage(
                          url, mediaType, xCmdEnv ) );
        OSL_ASSERT( xPackage.is() );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException &) {
        // ignore already handled error
    }
    catch (Exception &) {
        const Any exc( ::cppu::getCaughtException() );
        if (notifyDetectionError ||
            !exc.isExtractableTo(
                ::getCppuType( reinterpret_cast<
                               lang::IllegalArgumentException const *>(0) ) ))
        {
            interactContinuation(
                Any( lang::WrappedTargetException(
                         OUSTR("bundle item error!"),
                         static_cast<OWeakObject *>(this), exc ) ),
                task::XInteractionApprove::static_type(), xCmdEnv, 0, 0 );
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

//______________________________________________________________________________
void BackendImpl::PackageImpl::scanBundle(
    t_packagevec & bundle,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OSL_ASSERT( !m_legacyBundle );

    ::ucbhelper::Content manifestContent;
    if (! create_ucb_content(
            &manifestContent,
            makeURL( m_url_expanded, OUSTR("META-INF/manifest.xml") ),
            xCmdEnv, false /* no throw */ ))
    {
        OSL_ENSURE( 0, "### missing META-INF/manifest.xml file!" );
        return;
}


    const lang::Locale officeLocale = getOfficeLocale();
    OUString descrFile;
    lang::Locale descrFileLocale;

    const Reference<XComponentContext> xContext(
        getMyBackend()->getComponentContext() );
    Reference<packages::manifest::XManifestReader> xManifestReader(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.packages.manifest.ManifestReader"),
            xContext ), UNO_QUERY_THROW );
    const Sequence< Sequence<beans::PropertyValue> > manifestSeq(
        xManifestReader->readManifestSequence( manifestContent.openStream() ) );
    const OUString packageRootURL( getURL() );
    for ( sal_Int32 pos = manifestSeq.getLength(); pos--; )
    {
        OUString fullPath, mediaType;
        Sequence<beans::PropertyValue> const & attribs = manifestSeq[ pos ];
        for ( sal_Int32 i = attribs.getLength(); i--; )
        {
            if (fullPath.getLength() > 0 && mediaType.getLength() > 0)
                break;
            if (attribs[i].Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("FullPath") ))
                attribs[i].Value >>= fullPath;
            else if (attribs[i].Name.equalsAsciiL(
                         RTL_CONSTASCII_STRINGPARAM("MediaType") ))
                attribs[i].Value >>= mediaType;
        }

        if (fullPath.getLength() == 0 || mediaType.getLength() == 0 ||
            mediaType.equalsAsciiL( // opt: exclude common text/xml
                RTL_CONSTASCII_STRINGPARAM("text/xml") ))
            continue;

        String type, subType;
        INetContentTypeParameterList params;
        if (! INetContentTypes::parse( mediaType, type, subType, &params ))
            continue;

        INetContentTypeParameter const * param = params.find(
            ByteString("platform") );
        if (param != 0 && !platform_fits( param->m_sValue ))
            continue;
        const OUString url( makeURL( packageRootURL, fullPath ) );

        // check for bundle description:
        if (type.EqualsIgnoreCaseAscii("application") &&
            subType.EqualsIgnoreCaseAscii(
                "vnd.sun.star.package-bundle-description"))
        {
            // check locale:
            param = params.find( ByteString("locale") );
            if (param == 0) {
                if (descrFile.getLength() == 0)
                    descrFile = url;
            }
            else {
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
                    descrFile = url;
                    descrFileLocale = locale;
                }
            }
            continue;
        }

        checkAborted( abortChannel );

        //We make sure that we only create one XPackage for a particular URL.
        //Sometime programmers insert the same URL several times in the manifest
        //which may lead to DisposedExceptions.
        if (bundle.end() == std::find_if(bundle.begin(), bundle.end(), XPackage_eq(url)))
        {
            const Reference<deployment::XPackage> xPackage(
                bindBundleItem( url, mediaType, xCmdEnv ) );
            if (xPackage.is())
                bundle.push_back( xPackage );
        }
        else
        {
            fprintf(stderr, "manifest.xml contains a duplicate entry!\n");
        }
    }

    if (descrFile.getLength() > 0)
    {
        ::ucbhelper::Content descrFileContent;
        if (create_ucb_content( &descrFileContent, descrFile,
                                xCmdEnv, false /* no throw */ ))
        {
            // patch description:
            ::rtl::ByteSequence bytes( readFile( descrFileContent ) );
            ::rtl::OUStringBuffer buf;
            if ( bytes.getLength() )
            {
                buf.append( OUString( reinterpret_cast<sal_Char const *>(
                                          bytes.getConstArray() ),
                                      bytes.getLength(), RTL_TEXTENCODING_UTF8 ) );
            }
            else
            {
                buf.append( Package::getDescription() );
            }
            m_oldDescription = buf.makeStringAndClear();
        }
    }
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::scanLegacyBundle(
    t_packagevec & bundle,
    OUString const & url,
    ::rtl::Reference<AbortChannel> const & abortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool skip_registration )
{
    ::ucbhelper::Content ucbContent( url, xCmdEnv );

    // check for platform pathes:
    const OUString title( ucbContent.getPropertyValue(
                              StrTitle::get() ).get<OUString>() );
    if (title.endsWithIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM(".plt") ) &&
        !platform_fits( title.copy( 0, title.getLength() - 4 ) )) {
        return;
    }
    if (title.endsWithIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("skip_registration") ))
        skip_registration = true;

    OUString ar [] = { StrTitle::get(), OUSTR("IsFolder") };
    Reference<sdbc::XResultSet> xResultSet(
        ucbContent.createCursor(
            Sequence<OUString>( ar, ARLEN(ar) ),
            ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
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
            bindBundleItem( path, OUString() /* detect */, xCmdEnv,
                            false /* ignore detection errors */ ) );
        if (xPackage.is()) {
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                xPackage->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is())
                mediaType = xPackageType->getMediaType();

            if (skip_registration &&
                // xxx todo: additional parsing?
                mediaType.matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(
                        "application/vnd.sun.star.uno-component") ))
                continue;

            bundle.push_back( xPackage );
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
            if (xRow->getBoolean( 2 /* IsFolder */ )) { // recurse into folder:
                scanLegacyBundle(
                    bundle, path, abortChannel, xCmdEnv, skip_registration );
            }
        }
    }
}

OUString BackendImpl::PackageImpl::getDisplayName() throw (RuntimeException)
{
    OUString sName = getDescriptionInfoset().getLocalizedDisplayName();
    if (sName.getLength() == 0)
        return m_displayName;
    else
        return sName;
}

} // anon namespace

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
    return new BackendImpl( args, xComponentContext, xRootRegistry );
}

} // namespace bundle
} // namespace backend
} // namespace dp_registry

