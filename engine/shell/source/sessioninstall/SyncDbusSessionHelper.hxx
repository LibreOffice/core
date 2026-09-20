/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SHELL_SOURCE_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX
#define INCLUDED_SHELL_SOURCE_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX

#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <cppuhelper/implbase.hxx>

namespace shell::sessioninstall
{
    class SyncDbusSessionHelper : public ::cppu::WeakImplHelper< ::org::freedesktop::PackageKit::XSyncDbusSessionHelper, css::lang::XServiceInfo >
    {
        public:
            SyncDbusSessionHelper(cpo::uno::Reference< cpo::uno::XComponentContext> const&);

            // XServiceInfo
            virtual OUString getImplementationName() override;
            virtual bool supportsService(const OUString& ServiceName) override;
            virtual cpo::uno::Sequence< OUString > getSupportedServiceNames () override;

            // XModify Methods
            virtual void InstallPackageFiles( const cpo::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void InstallProvideFiles( const cpo::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void InstallCatalogs( const cpo::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void InstallPackageNames( const cpo::uno::Sequence< OUString >& packages, const OUString& interaction ) override;

            virtual void InstallMimeTypes( const cpo::uno::Sequence< OUString >& mimeTypes, const OUString& interaction ) override;

            virtual void InstallFontconfigResources( const cpo::uno::Sequence< OUString >& resources, const OUString& interaction ) override;

            virtual void InstallGStreamerResources( const cpo::uno::Sequence< OUString >& resources, const OUString& interaction ) override;

            virtual void InstallResources( const cpo::uno::Sequence< OUString >& /* types */, const cpo::uno::Sequence< OUString >& /* resources */, const OUString& /* interaction */ ) override
                { throw cpo::uno::RuntimeException(u"InstallResources is not implemented"_ustr); } // not implemented

            virtual void RemovePackageByFiles( const cpo::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void InstallPrinterDrivers( const cpo::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            // XQuery Methods
            virtual void IsInstalled( const OUString& /* package_name */, const OUString& /* interaction */, bool& /* installed */ ) override;

            virtual void SearchFile( const OUString& /* file_name */, const OUString& /* interaction */, bool& /* installed */, OUString& /* package_name */ ) override
                { throw cpo::uno::RuntimeException(u"SearchFile is not implemented"_ustr); } // not implemented

        private:
            SyncDbusSessionHelper( const SyncDbusSessionHelper& ) = delete;
            SyncDbusSessionHelper& operator=( const SyncDbusSessionHelper& ) = delete;
    };
}

#endif // INCLUDED_SHELL_SOURCE_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
