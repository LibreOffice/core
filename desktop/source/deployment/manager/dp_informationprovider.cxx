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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XUpdateInformationProvider.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XAbortChannel.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <ucbhelper/content.hxx>

#include <dp_dependencies.hxx>
#include <dp_descriptioninfoset.hxx>
#include <dp_identifier.hxx>
#include <dp_version.hxx>
#include <dp_update.hxx>

namespace beans      = com::sun::star::beans ;
namespace deployment = com::sun::star::deployment ;
namespace lang       = com::sun::star::lang ;
namespace task       = com::sun::star::task ;
namespace css_ucb    = com::sun::star::ucb ;
namespace uno        = com::sun::star::uno ;
namespace xml = com::sun::star::xml ;


namespace dp_info {

namespace {

class PackageInformationProvider :
        public ::cppu::WeakImplHelper< deployment::XPackageInformationProvider, lang::XServiceInfo >

{
    public:
    explicit PackageInformationProvider( uno::Reference< uno::XComponentContext >const& xContext);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPackageInformationProvider
    virtual OUString SAL_CALL getPackageLocation( const OUString& extensionId ) override;
    virtual uno::Sequence< uno::Sequence< OUString > > SAL_CALL isUpdateAvailable( const OUString& extensionId ) override;
    virtual uno::Sequence< uno::Sequence< OUString > > SAL_CALL getExtensionList() override;

private:

    uno::Reference< uno::XComponentContext> mxContext;

    OUString getPackageLocation( const OUString& repository,
                                      std::u16string_view _sExtensionId );

    uno::Reference< deployment::XUpdateInformationProvider > mxUpdateInformation;
};

}

PackageInformationProvider::PackageInformationProvider( uno::Reference< uno::XComponentContext > const& xContext) :
    mxContext( xContext ),
    mxUpdateInformation( deployment::UpdateInformationProvider::create( xContext ) )
{
}

// XServiceInfo
OUString PackageInformationProvider::getImplementationName()
{
    return u"com.sun.star.comp.deployment.PackageInformationProvider"_ustr;
}

sal_Bool PackageInformationProvider::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > PackageInformationProvider::getSupportedServiceNames()
{
    // a private one:
    return { u"com.sun.star.comp.deployment.PackageInformationProvider"_ustr };
}

OUString PackageInformationProvider::getPackageLocation(
    const OUString & repository,
    std::u16string_view _rExtensionId )
{
    OUString aLocationURL;
    uno::Reference<deployment::XExtensionManager> xManager =
        deployment::ExtensionManager::get(mxContext);

    if ( xManager.is() )
    {
        const uno::Sequence< uno::Reference< deployment::XPackage > > packages(
                xManager->getDeployedExtensions(
                    repository,
                    uno::Reference< task::XAbortChannel >(),
                    uno::Reference< css_ucb::XCommandEnvironment > () ) );

        for ( int pos = packages.getLength(); pos--; )
        {
            try
            {
                const beans::Optional< OUString > aID = packages[ pos ]->getIdentifier();
                if ( aID.IsPresent && (aID.Value == _rExtensionId ) )
                {
                    aLocationURL = packages[ pos ]->getURL();
                    break;
                }
            }
            catch ( uno::RuntimeException & ) {}
        }
    }

    return aLocationURL;
}


OUString SAL_CALL
PackageInformationProvider::getPackageLocation( const OUString& _sExtensionId )
{
    OUString aLocationURL = getPackageLocation( u"user"_ustr, _sExtensionId );

    if ( aLocationURL.isEmpty() )
    {
        aLocationURL = getPackageLocation( u"shared"_ustr, _sExtensionId );
    }
    if ( aLocationURL.isEmpty() )
    {
        aLocationURL = getPackageLocation( u"bundled"_ustr, _sExtensionId );
    }
    if ( !aLocationURL.isEmpty() )
    {
        try
        {
            ::ucbhelper::Content aContent( aLocationURL, nullptr, mxContext );
            aLocationURL = aContent.getURL();
        }
        catch (const css::ucb::ContentCreationException&)
        {
            TOOLS_WARN_EXCEPTION("desktop.deployment", "ignoring");
        }
    }
    return aLocationURL;
}

uno::Sequence< uno::Sequence< OUString > > SAL_CALL
PackageInformationProvider::isUpdateAvailable( const OUString& _sExtensionId )
{
    uno::Sequence< uno::Sequence< OUString > > aList;

    uno::Reference<deployment::XExtensionManager> extMgr =
        deployment::ExtensionManager::get(mxContext);

    if (!extMgr.is())
    {
        OSL_ASSERT(false);
        return aList;
    }
    std::vector<std::pair<uno::Reference<deployment::XPackage>, uno::Any > > errors;
    dp_misc::UpdateInfoMap updateInfoMap;
    if (!_sExtensionId.isEmpty())
    {
        std::vector<uno::Reference<deployment::XPackage> > vecExtensions;
        uno::Reference<deployment::XPackage> extension;
        try
        {
            extension = dp_misc::getExtensionWithHighestVersion(
                extMgr->getExtensionsWithSameIdentifier(
                    _sExtensionId, _sExtensionId, uno::Reference<css_ucb::XCommandEnvironment>()));
            vecExtensions.push_back(extension);
        }
        catch (lang::IllegalArgumentException &)
        {
            OSL_ASSERT(false);
        }
        updateInfoMap = dp_misc::getOnlineUpdateInfos(
            mxContext, extMgr, mxUpdateInformation, &vecExtensions, errors);
    }
    else
    {
        updateInfoMap = dp_misc::getOnlineUpdateInfos(
            mxContext, extMgr, mxUpdateInformation, nullptr, errors);
    }

    int nCount = 0;
    for (auto const& updateInfo : updateInfoMap)
    {
        dp_misc::UpdateInfo const & info = updateInfo.second;

        OUString sOnlineVersion;
        if (info.info.is())
        {
            // check, if there are unsatisfied dependencies and ignore this online update
            dp_misc::DescriptionInfoset infoset(mxContext, info.info);
            uno::Sequence< uno::Reference< xml::dom::XElement > >
                ds( dp_misc::Dependencies::check( infoset ) );
            if ( ! ds.hasElements() )
                sOnlineVersion = info.version;
        }

        OUString sVersionUser;
        OUString sVersionShared;
        OUString sVersionBundled;
        uno::Sequence< uno::Reference< deployment::XPackage> > extensions;
        try {
            extensions = extMgr->getExtensionsWithSameIdentifier(
                dp_misc::getIdentifier(info.extension), info.extension->getName(),
                uno::Reference<css_ucb::XCommandEnvironment>());
        } catch (const lang::IllegalArgumentException&) {
            TOOLS_WARN_EXCEPTION("desktop.deployment", "ignoring");
            continue;
        }
        OSL_ASSERT(extensions.getLength() == 3);
        if (extensions[0].is() )
            sVersionUser = extensions[0]->getVersion();
        if (extensions[1].is() )
            sVersionShared = extensions[1]->getVersion();
        if (extensions[2].is() )
            sVersionBundled = extensions[2]->getVersion();

        bool bSharedReadOnly = extMgr->isReadOnlyRepository(u"shared"_ustr);

        dp_misc::UPDATE_SOURCE sourceUser = dp_misc::isUpdateUserExtension(
            bSharedReadOnly, sVersionUser, sVersionShared, sVersionBundled, sOnlineVersion);
        dp_misc::UPDATE_SOURCE sourceShared = dp_misc::isUpdateSharedExtension(
            bSharedReadOnly, sVersionShared, sVersionBundled, sOnlineVersion);

        OUString updateVersionUser;
        OUString updateVersionShared;
        if (sourceUser != dp_misc::UPDATE_SOURCE_NONE)
            updateVersionUser = dp_misc::getHighestVersion(
                sVersionShared, sVersionBundled, sOnlineVersion);
        if (sourceShared  != dp_misc::UPDATE_SOURCE_NONE)
            updateVersionShared = dp_misc::getHighestVersion(
                OUString(), sVersionBundled, sOnlineVersion);
        OUString updateVersion;
        if (dp_misc::compareVersions(updateVersionUser, updateVersionShared) == dp_misc::GREATER)
            updateVersion = updateVersionUser;
        else
            updateVersion = updateVersionShared;
        if (!updateVersion.isEmpty())
        {

            OUString aNewEntry[2];
            aNewEntry[0] = updateInfo.first;
            aNewEntry[1] = updateVersion;
            aList.realloc( ++nCount );
            aList.getArray()[ nCount-1 ] = ::uno::Sequence< OUString >( aNewEntry, 2 );
        }
    }
    return aList;
}


uno::Sequence< uno::Sequence< OUString > > SAL_CALL PackageInformationProvider::getExtensionList()
{
    const uno::Reference<deployment::XExtensionManager> mgr =
        deployment::ExtensionManager::get(mxContext);

    if (!mgr.is())
        return uno::Sequence< uno::Sequence< OUString > >();

    const uno::Sequence< uno::Sequence< uno::Reference<deployment::XPackage > > >
        allExt =  mgr->getAllExtensions(
            uno::Reference< task::XAbortChannel >(),
            uno::Reference< css_ucb::XCommandEnvironment > () );

    uno::Sequence< uno::Sequence< OUString > > retList;

    sal_Int32 cAllIds = allExt.getLength();
    retList.realloc(cAllIds);
    auto pretList = retList.getArray();

    for (sal_Int32 i = 0; i < cAllIds; i++)
    {
        //The inner sequence contains extensions with the same identifier from
        //all the different repositories, that is user, share, bundled.
        const uno::Sequence< uno::Reference< deployment::XPackage > > &
            seqExtension = allExt[i];
        sal_Int32 cExt = seqExtension.getLength();
        OSL_ASSERT(cExt == 3);
        for (sal_Int32 j = 0; j < cExt; j++)
        {
            //ToDo according to the old code the first found extension is used
            //even if another one with the same id has a better version.
            uno::Reference< deployment::XPackage > const & xExtension( seqExtension[j] );
            if (xExtension.is())
            {
                pretList[i] = { dp_misc::getIdentifier(xExtension), xExtension->getVersion() };
                break;
            }
        }
    }
    return retList;
}


} // namespace dp_info

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_deployment_PackageInformationProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new dp_info::PackageInformationProvider(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
