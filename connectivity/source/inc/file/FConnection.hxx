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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCONNECTION_HXX

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/OSubComponent.hxx>
#include <connectivity/CommonTools.hxx>
#include "OTypeInfo.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <connectivity/sqlparse.hxx>
#include <connectivity/sqliterator.hxx>
#include "TConnection.hxx"
#include "file/filedllapi.hxx"
#include <map>

namespace connectivity
{
    namespace file
    {
        class ODatabaseMetaData;
        class OFileDriver;

        class OOO_DLLPUBLIC_FILE OConnection :
                            public connectivity::OMetaConnection,
                            public connectivity::OSubComponent<OConnection, connectivity::OMetaConnection>
        {
            friend class connectivity::OSubComponent<OConnection, connectivity::OMetaConnection>;

        protected:

            // Data attributes

            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbcx::XTablesSupplier>         m_xCatalog;

            OUString                    m_aFilenameExtension;
            OFileDriver*                m_pDriver;      //  Pointer to the owning
                                                        //  driver object
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XDynamicResultSet >    m_xDir; // directory
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>              m_xContent;

            bool                    m_bClosed;
            bool                    m_bAutoCommit;
            bool                    m_bReadOnly;
            bool                    m_bShowDeleted;
            bool                    m_bCaseSensitiveExtension;
            bool                    m_bCheckSQL92;
            bool                        m_bDefaultTextEncoding;


            void throwUrlNotValid(const OUString & _rsUrl,const OUString & _rsMessage);

            virtual ~OConnection();
        public:

            OConnection(OFileDriver*    _pDriver);

            virtual void construct(const OUString& _rUrl, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo )
                throw( css::sdbc::SQLException,
                       css::uno::RuntimeException,
                       css::uno::DeploymentException,
                       std::exception);

            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XConnection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            //XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            // no interface methods
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XDynamicResultSet > getDir() const;
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>  getContent() const { return m_xContent; }
            // create a catalog or return the catalog already created
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > createCatalog();

            bool                matchesExtension( const OUString& _rExt ) const;

            inline const OUString&    getExtension()            const { return m_aFilenameExtension; }
            inline bool         isCaseSensitveExtension()   const { return m_bCaseSensitiveExtension; }
            inline OFileDriver*     getDriver()                 const { return m_pDriver; }
            inline bool         showDeleted()               const { return m_bShowDeleted; }
            inline bool         isCheckEnabled()            const { return m_bCheckSQL92; }
            inline bool             isTextEncodingDefaulted()   const { return m_bDefaultTextEncoding; }

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
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
