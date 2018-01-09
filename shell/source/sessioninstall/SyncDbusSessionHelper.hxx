/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SHELL_SOURCE_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX
#define INCLUDED_SHELL_SOURCE_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <cppuhelper/implbase.hxx>

namespace shell { namespace sessioninstall
{
    class SyncDbusSessionHelper : public ::cppu::WeakImplHelper< ::org::freedesktop::PackageKit::XSyncDbusSessionHelper >
    {
        public:
            SyncDbusSessionHelper(css::uno::Reference< css::uno::XComponentContext> const&);

            // XModify Methods
            virtual void SAL_CALL InstallPackageFiles( const css::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void SAL_CALL InstallProvideFiles( const css::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void SAL_CALL InstallCatalogs( const css::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void SAL_CALL InstallPackageNames( const css::uno::Sequence< OUString >& packages, const OUString& interaction ) override;

            virtual void SAL_CALL InstallMimeTypes( const css::uno::Sequence< OUString >& mimeTypes, const OUString& interaction ) override;

            virtual void SAL_CALL InstallFontconfigResources( const css::uno::Sequence< OUString >& resources, const OUString& interaction ) override;

            virtual void SAL_CALL InstallGStreamerResources( const css::uno::Sequence< OUString >& resources, const OUString& interaction ) override;

            virtual void SAL_CALL InstallResources( const css::uno::Sequence< OUString >& /* types */, const css::uno::Sequence< OUString >& /* resources */, const OUString& /* interaction */ ) override
                { throw css::uno::RuntimeException(); } // not implemented

            virtual void SAL_CALL RemovePackageByFiles( const css::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            virtual void SAL_CALL InstallPrinterDrivers( const css::uno::Sequence< OUString >& files, const OUString& interaction ) override;

            // XQuery Methods
            virtual void SAL_CALL IsInstalled( const OUString& /* package_name */, const OUString& /* interaction */, sal_Bool& /* installed */ ) override;

            virtual void SAL_CALL SearchFile( const OUString& /* file_name */, const OUString& /* interaction */, sal_Bool& /* installed */, OUString& /* package_name */ ) override
                { throw css::uno::RuntimeException(); } // not implemented

        private:
            SyncDbusSessionHelper( const SyncDbusSessionHelper& ) = delete;
            SyncDbusSessionHelper& operator=( const SyncDbusSessionHelper& ) = delete;
    };
}}

#endif // INCLUDED_SHELL_SOURCE_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
