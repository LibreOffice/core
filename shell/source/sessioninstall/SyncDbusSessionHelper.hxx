/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SHELL_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX
#define SHELL_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <cppuhelper/implbase1.hxx>

namespace shell { namespace sessioninstall
{
    class SyncDbusSessionHelper : public ::cppu::WeakImplHelper1< ::org::freedesktop::PackageKit::XSyncDbusSessionHelper >
    {
        public:
            SyncDbusSessionHelper(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const&);
            virtual ~SyncDbusSessionHelper() {}
            // XModify Methods
            virtual void SAL_CALL InstallPackageNames( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* packages */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException);

            virtual void SAL_CALL InstallPackageFiles( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* files */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL InstallProvideFiles( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* files */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL InstallCatalogs( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* files */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL InstallMimeTypes( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* mime_types */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL InstallFontconfigResources( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* resources */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL InstalliGStreamerResources( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* resources */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL InstallResources( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* types */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* resources */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL RemovePackageByFiles( SAL_UNUSED_PARAMETER ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* files */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            virtual void SAL_CALL InstallPrinterDrivers( ::sal_uInt32 /* xid */, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* files */, const ::rtl::OUString& /* interaction */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented
            // XQuery Methods
            virtual void SAL_CALL IsInstalled( const ::rtl::OUString& /* package_name */, const ::rtl::OUString& /* interaction */, ::sal_Bool& /* installed */ ) throw (::com::sun::star::uno::RuntimeException);

            virtual void SAL_CALL SearchFile( const ::rtl::OUString& /* file_name */, const ::rtl::OUString& /* interaction */, ::sal_Bool& /* installed */, ::rtl::OUString& /* package_name */ ) throw (::com::sun::star::uno::RuntimeException)
                { throw ::com::sun::star::uno::RuntimeException(); } // not implemented

        private:
            SyncDbusSessionHelper(); // never implemented
            SyncDbusSessionHelper( const SyncDbusSessionHelper& ); // never implemented
            SyncDbusSessionHelper& operator=( const SyncDbusSessionHelper& ); // never implemented
    };
}}

#endif // SHELL_SESSIONINSTALL_SYNCDBUSSESSIONHELPER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
