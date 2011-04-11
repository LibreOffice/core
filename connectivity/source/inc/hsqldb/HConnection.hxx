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

#ifndef CONNECTIVITY_HSQLDB_CONNECTION_HXX
#define CONNECTIVITY_HSQLDB_CONNECTION_HXX

#include "connectivity/ConnectionWrapper.hxx"
/** === begin UNO includes === **/
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#ifndef __com_sun_star_sdb_application_XTableUIProvider_hpp__
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#endif
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
/** === end UNO includes === **/
#include <cppuhelper/compbase2.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <memory>

namespace connectivity
{
    namespace hsqldb
    {
        class SAL_NO_VTABLE IMethodGuardAccess
        {
        public:
            virtual ::osl::Mutex&   getMutex() const = 0;
            virtual void            checkDisposed() const = 0;
        };

        //==========================================================================
        //= OHsqlConnection - wraps all methods to the real connection from the driver
        //= but when disposed it doesn't dispose the real connection
        //==========================================================================
        typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::util::XFlushable
                                                ,   ::com::sun::star::sdb::application::XTableUIProvider
                                                >   OHsqlConnection_BASE;

        class OHsqlConnection   :public ::comphelper::OBaseMutex
                                ,public OHsqlConnection_BASE
                                ,public OConnectionWrapper
                                ,public IMethodGuardAccess
        {
        private:
            ::cppu::OInterfaceContainerHelper                                                           m_aFlushListeners;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >                         m_xDriver;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >            m_xORB;
            bool                                                                                        m_bIni;
            bool                                                                                        m_bReadOnly;

        protected:
            virtual void SAL_CALL disposing(void);
            virtual ~OHsqlConnection();

        public:
            OHsqlConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > _rxDriver,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB
            );

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            DECLARE_XTYPEPROVIDER()
            DECLARE_XINTERFACE( )

            // IMethodGuardAccess
            virtual ::osl::Mutex&   getMutex() const;
            virtual void            checkDisposed() const;

            // XFlushable
            virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);

            // XTableUIProvider
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL getTableIcon( const ::rtl::OUString& TableName, ::sal_Int32 ColorMode ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getTableEditor( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& DocumentUI, const ::rtl::OUString& TableName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        private:

            /** retrieves our table container
                @return
                    our table container. Guaranteed to not be <NULL/>.
                @throws ::com::sun::star::lang::WrappedTargetException
                    if a non-RuntimeException is caught during obtaining the container.
                @throws ::com::sun::star::uno::RuntimeException
                    if a serious error occurs
                @precond
                    We're not disposed.
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    impl_getTableContainer_throw();

            /** checks whether the given table name denotes an existing table
                @param _rTableName
                    the fully name of the table to check for existence
                @throws ::com::sun::star::lang::IllegalArgumentException
                    if the name does not denote an existing table
                @precond
                    We're not disposed.
            */
            void    impl_checkExistingTable_throw( const ::rtl::OUString& _rTableName );

            /** checks whether the given table name refers to a HSQL TEXT TABLE
            */
            bool    impl_isTextTable_nothrow( const ::rtl::OUString& _rTableName );

            /** retrieves the icon for HSQL TEXT TABLEs
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                impl_getTextTableIcon_nothrow();
        };

        //==========================================================================
        //= OHsqlConnection
        //==========================================================================
        class MethodGuard : public ::osl::MutexGuard
        {
        private:
            typedef ::osl::MutexGuard   BaseGuard;

        public:
            MethodGuard( const IMethodGuardAccess& _rComponent )
                :BaseGuard( _rComponent.getMutex() )
            {
                _rComponent.checkDisposed();
            }
        };
    }
}
#endif // CONNECTIVITY_HSQLDB_CONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
