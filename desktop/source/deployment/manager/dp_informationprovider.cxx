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

#include <cppuhelper/implbase3.hxx>
#include "comphelper/servicedecl.hxx"

#include "com/sun/star/deployment/UpdateInformationProvider.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XPackageInformationProvider.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/deployment/XUpdateInformationProvider.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"

#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ustring.hxx"
#include "ucbhelper/content.hxx"

#include "dp_dependencies.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_identifier.hxx"
#include "dp_version.hxx"
#include "dp_misc.h"
#include "dp_update.hxx"

namespace beans      = com::sun::star::beans ;
namespace deployment = com::sun::star::deployment ;
namespace lang       = com::sun::star::lang ;
namespace registry   = com::sun::star::registry ;
namespace task       = com::sun::star::task ;
namespace css_ucb    = com::sun::star::ucb ;
namespace uno        = com::sun::star::uno ;
namespace xml = com::sun::star::xml ;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace dp_info {

class PackageInformationProvider :
        public ::cppu::WeakImplHelper1< deployment::XPackageInformationProvider >

{
    public:
                 PackageInformationProvider( uno::Reference< uno::XComponentContext >const& xContext);
    virtual     ~PackageInformationProvider();

    static uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    // XPackageInformationProvider
    virtual rtl::OUString SAL_CALL getPackageLocation( const rtl::OUString& extensionId )
        throw ( uno::RuntimeException );
    virtual uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL isUpdateAvailable( const rtl::OUString& extensionId )
        throw ( uno::RuntimeException );
    virtual uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL getExtensionList()
        throw ( uno::RuntimeException );
//---------
private:

    uno::Reference< uno::XComponentContext> mxContext;

    rtl::OUString getPackageLocation( const rtl::OUString& repository,
                                      const rtl::OUString& _sExtensionId );

    uno::Reference< deployment::XUpdateInformationProvider > mxUpdateInformation;
};

//------------------------------------------------------------------------------

PackageInformationProvider::PackageInformationProvider( uno::Reference< uno::XComponentContext > const& xContext) :
    mxContext( xContext ),
    mxUpdateInformation( deployment::UpdateInformationProvider::create( xContext ) )
{
}

//------------------------------------------------------------------------------

PackageInformationProvider::~PackageInformationProvider()
{
}

//------------------------------------------------------------------------------
rtl::OUString PackageInformationProvider::getPackageLocation(
    const rtl::OUString & repository,
    const rtl::OUString& _rExtensionId )
{
    rtl::OUString aLocationURL;
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
                const rtl::OUString aName = packages[ pos ]->getName();
                const beans::Optional< rtl::OUString > aID = packages[ pos ]->getIdentifier();
                if ( aID.IsPresent && aID.Value.compareTo( _rExtensionId ) == 0 )
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

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
PackageInformationProvider::getPackageLocation( const rtl::OUString& _sExtensionId )
    throw ( uno::RuntimeException )
{
    rtl::OUString aLocationURL = getPackageLocation( UNISTRING("user"), _sExtensionId );

    if ( aLocationURL.isEmpty() )
    {
        aLocationURL = getPackageLocation( UNISTRING("shared"), _sExtensionId );
    }
    if ( aLocationURL.isEmpty() )
    {
        aLocationURL = getPackageLocation( UNISTRING("bundled"), _sExtensionId );
    }
    if ( !aLocationURL.isEmpty() )
    {
        ::ucbhelper::Content aContent( aLocationURL, NULL, mxContext );
        aLocationURL = aContent.getURL();
    }
    return aLocationURL;
}

//------------------------------------------------------------------------------

uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL
PackageInformationProvider::isUpdateAvailable( const rtl::OUString& _sExtensionId )
    throw ( uno::RuntimeException )
{
    uno::Sequence< uno::Sequence< rtl::OUString > > aList;

    uno::Reference<deployment::XExtensionManager> extMgr =
        deployment::ExtensionManager::get(mxContext);

    if (!extMgr.is())
    {
        OSL_ASSERT(0);
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
            OSL_ASSERT(0);
        }
        updateInfoMap = dp_misc::getOnlineUpdateInfos(
            mxContext, extMgr, mxUpdateInformation, &vecExtensions, errors);
    }
    else
    {
        updateInfoMap = dp_misc::getOnlineUpdateInfos(
            mxContext, extMgr, mxUpdateInformation, NULL, errors);
    }

    int nCount = 0;
    for (dp_misc::UpdateInfoMap::iterator i(updateInfoMap.begin()); i != updateInfoMap.end(); ++i)
    {
        dp_misc::UpdateInfo const & info = i->second;

        rtl::OUString sOnlineVersion;
        if (info.info.is())
        {
            // check, if there are unsatisfied dependencies and ignore this online update
            dp_misc::DescriptionInfoset infoset(mxContext, info.info);
            uno::Sequence< uno::Reference< xml::dom::XElement > >
                ds( dp_misc::Dependencies::check( infoset ) );
            if ( ! ds.getLength() )
                sOnlineVersion = info.version;
        }

        rtl::OUString sVersionUser;
        rtl::OUString sVersionShared;
        rtl::OUString sVersionBundled;
        uno::Sequence< uno::Reference< deployment::XPackage> > extensions;
        try {
            extensions = extMgr->getExtensionsWithSameIdentifier(
                dp_misc::getIdentifier(info.extension), info.extension->getName(),
                uno::Reference<css_ucb::XCommandEnvironment>());
        } catch (lang::IllegalArgumentException& ) {
            OSL_ASSERT(0);
        }
        OSL_ASSERT(extensions.getLength() == 3);
        if (extensions[0].is() )
            sVersionUser = extensions[0]->getVersion();
        if (extensions[1].is() )
            sVersionShared = extensions[1]->getVersion();
        if (extensions[2].is() )
            sVersionBundled = extensions[2]->getVersion();

        bool bSharedReadOnly = extMgr->isReadOnlyRepository(OUSTR("shared"));

        dp_misc::UPDATE_SOURCE sourceUser = dp_misc::isUpdateUserExtension(
            bSharedReadOnly, sVersionUser, sVersionShared, sVersionBundled, sOnlineVersion);
        dp_misc::UPDATE_SOURCE sourceShared = dp_misc::isUpdateSharedExtension(
            bSharedReadOnly, sVersionShared, sVersionBundled, sOnlineVersion);

        rtl::OUString updateVersionUser;
        rtl::OUString updateVersionShared;
        if (sourceUser != dp_misc::UPDATE_SOURCE_NONE)
            updateVersionUser = dp_misc::getHighestVersion(
                rtl::OUString(), sVersionShared, sVersionBundled, sOnlineVersion);
        if (sourceShared  != dp_misc::UPDATE_SOURCE_NONE)
            updateVersionShared = dp_misc::getHighestVersion(
                rtl::OUString(), rtl::OUString(), sVersionBundled, sOnlineVersion);
        rtl::OUString updateVersion;
        if (dp_misc::compareVersions(updateVersionUser, updateVersionShared) == dp_misc::GREATER)
            updateVersion = updateVersionUser;
        else
            updateVersion = updateVersionShared;
        if (!updateVersion.isEmpty())
        {

            rtl::OUString aNewEntry[2];
            aNewEntry[0] = i->first;
            aNewEntry[1] = updateVersion;
            aList.realloc( ++nCount );
            aList[ nCount-1 ] = ::uno::Sequence< rtl::OUString >( aNewEntry, 2 );
        }
    }
    return aList;
}

//------------------------------------------------------------------------------
uno::Sequence< uno::Sequence< rtl::OUString > > SAL_CALL PackageInformationProvider::getExtensionList()
    throw ( uno::RuntimeException )
{
    const uno::Reference<deployment::XExtensionManager> mgr =
        deployment::ExtensionManager::get(mxContext);

    if (!mgr.is())
        return uno::Sequence< uno::Sequence< rtl::OUString > >();

    const uno::Sequence< uno::Sequence< uno::Reference<deployment::XPackage > > >
        allExt =  mgr->getAllExtensions(
            uno::Reference< task::XAbortChannel >(),
            uno::Reference< css_ucb::XCommandEnvironment > () );

    uno::Sequence< uno::Sequence< rtl::OUString > > retList;

    sal_Int32 cAllIds = allExt.getLength();
    retList.realloc(cAllIds);

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
            //ToDo according to the old code the first found extenions is used
            //even if another one with the same id has a better version.
            uno::Reference< deployment::XPackage > const & xExtension( seqExtension[j] );
            if (xExtension.is())
            {
                rtl::OUString aNewEntry[2];
                aNewEntry[0] = dp_misc::getIdentifier(xExtension);
                aNewEntry[1] = xExtension->getVersion();
                retList[i] = ::uno::Sequence< rtl::OUString >( aNewEntry, 2 );
                break;
            }
        }
    }
    return retList;
}


//------------------------------------------------------------------------------

namespace sdecl = comphelper::service_decl;
sdecl::class_<PackageInformationProvider> servicePIP;
extern sdecl::ServiceDecl const serviceDecl(
    servicePIP,
    // a private one:
    "com.sun.star.comp.deployment.PackageInformationProvider",
    "com.sun.star.comp.deployment.PackageInformationProvider" );

} // namespace dp_info


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
