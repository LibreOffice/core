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
#pragma once

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/CommonTools.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <TConnection.hxx>
#include <file/filedllapi.hxx>

namespace connectivity::file
    {
        class ODatabaseMetaData;
        class OFileDriver;

        class OOO_DLLPUBLIC_FILE OConnection : public connectivity::OMetaConnection
        {
        protected:

            // Data attributes

            css::uno::WeakReference< css::sdbcx::XTablesSupplier>         m_xCatalog;

            OUString                    m_aFilenameExtension;
            OFileDriver*                m_pDriver;      //  Pointer to the owning
                                                        //  driver object
            css::uno::Reference< css::ucb::XDynamicResultSet >    m_xDir; // directory
            css::uno::Reference< css::ucb::XContent>              m_xContent;

            bool                    m_bAutoCommit;
            bool                    m_bReadOnly;
            bool                    m_bShowDeleted;
            bool                    m_bCaseSensitiveExtension;
            bool                    m_bCheckSQL92;
            bool                    m_bDefaultTextEncoding;


            void throwUrlNotValid(const OUString & _rsUrl,const OUString & _rsMessage);

            virtual ~OConnection() override;
        public:

            OConnection(OFileDriver*    _pDriver);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            /// @throws css::uno::DeploymentException
            virtual void construct(const OUString& _rUrl, const css::uno::Sequence< css::beans::PropertyValue >& _rInfo );

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) override;
            virtual OUString SAL_CALL nativeSQL( const OUString& sql ) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) override;
            virtual void SAL_CALL commit(  ) override;
            virtual void SAL_CALL rollback(  ) override;
            virtual sal_Bool SAL_CALL isClosed(  ) override final;
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) override;
            virtual void SAL_CALL setCatalog( const OUString& catalog ) override;
            virtual OUString SAL_CALL getCatalog(  ) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) override;
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) override;
            virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
            // XCloseable
            virtual void SAL_CALL close(  ) override final;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;
            //XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            // no interface methods
            css::uno::Reference< css::ucb::XDynamicResultSet > getDir() const;
            const css::uno::Reference< css::ucb::XContent>&  getContent() const { return m_xContent; }
            // create a catalog or return the catalog already created
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier > createCatalog();

            bool                matchesExtension( const OUString& _rExt ) const;

            const OUString&    getExtension()            const { return m_aFilenameExtension; }
            bool         isCaseSensitiveExtension()   const { return m_bCaseSensitiveExtension; }
            OFileDriver*     getDriver()                 const { return m_pDriver; }
            bool         showDeleted()               const { return m_bShowDeleted; }
            bool         isCheckEnabled()            const { return m_bCheckSQL92; }
            bool             isTextEncodingDefaulted()   const { return m_bDefaultTextEncoding; }

        public:
            struct GrantAccess
            {
                friend class ODatabaseMetaData;
            private:
                GrantAccess() { }
            };

            void    setCaseSensitiveExtension( bool _bIsCS, GrantAccess ) { m_bCaseSensitiveExtension = _bIsCS; }
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
