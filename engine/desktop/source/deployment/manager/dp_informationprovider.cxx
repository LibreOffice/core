/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XAbortChannel.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <ucbhelper/content.hxx>

#include <dp_identifier.hxx>

namespace beans      = css::beans ;
namespace deployment = css::deployment ;
namespace lang       = css::lang ;
namespace task       = css::task ;
namespace css_ucb    = css::ucb ;
namespace uno        = css::uno ;


namespace dp_info {

namespace {

class PackageInformationProvider :
        public ::cppu::WeakImplHelper< deployment::XPackageInformationProvider, lang::XServiceInfo >

{
    public:
    explicit PackageInformationProvider( uno::Reference< uno::XComponentContext >const& xContext);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPackageInformationProvider
    virtual OUString SAL_CALL getPackageLocation( const OUString& extensionId ) override;
    virtual uno::Sequence< uno::Sequence< OUString > > SAL_CALL getExtensionList() override;

private:

    uno::Reference< uno::XComponentContext> mxContext;

    OUString getPackageLocation( const OUString& repository,
                                      std::u16string_view _sExtensionId );
};

}

PackageInformationProvider::PackageInformationProvider( uno::Reference< uno::XComponentContext > const& xContext) :
    mxContext( xContext )
{
}

// XServiceInfo
OUString PackageInformationProvider::getImplementationName()
{
    return u"com.sun.star.comp.deployment.PackageInformationProvider"_ustr;
}

bool PackageInformationProvider::supportsService( const OUString& ServiceName )
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
    css::uno::XComponentContext* context, css::uno::Sequence<cpo::uno::Any> const& )
{
    return cppu::acquire(new dp_info::PackageInformationProvider(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
