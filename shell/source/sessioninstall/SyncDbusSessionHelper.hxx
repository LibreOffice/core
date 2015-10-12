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
            SyncDbusSessionHelper(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const&);
            virtual ~SyncDbusSessionHelper() {}

            // XModify Methods
            virtual void SAL_CALL InstallPackageFiles( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& files, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallProvideFiles( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& files, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallCatalogs( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& files, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallPackageNames( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& packages, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallMimeTypes( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& mimeTypes, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallFontconfigResources( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& resources, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallGStreamerResources( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& resources, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallResources( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< OUString >& /* types */, const ::com::sun::star::uno::Sequence< OUString >& /* resources */, const OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException, std::exception) override
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented

            virtual void SAL_CALL RemovePackageByFiles( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& files, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL InstallPrinterDrivers( ::sal_uInt32 xid, const ::com::sun::star::uno::Sequence< OUString >& files, const OUString& interaction ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XQuery Methods
            virtual void SAL_CALL IsInstalled( const OUString& /* package_name */, const OUString& /* interaction */, sal_Bool& /* installed */ ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL SearchFile( const OUString& /* file_name */, const OUString& /* interaction */, sal_Bool& /* installed */, OUString& /* package_name */ ) throw (::com::sun::star::uno::RuntimeException, std::exception) override
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented

        private:
            SyncDbusSessionHelper( const SyncDbusSessionHelper& ) = delete;
            SyncDbusSessionHelper& operator=( const SyncDbusSessionHelper& ) = delete;
    };
}}

#endif // INCLUDED_SHELL_SOURCE_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
