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
#pragma once

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/CommonTools.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <TConnection.hxx>
#include <file/filedllapi.hxx>
#include <unotools/weakref.hxx>

namespace connectivity::sdbcx { class OCatalog; }

namespace connectivity::file
{
    class OFileDriver;

    class OOO_DLLPUBLIC_FILE OConnection : public connectivity::OMetaConnection
    {
    protected:

        // Data attributes

        unotools::WeakReference< ::connectivity::sdbcx::OCatalog >         m_xCatalog;

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
        virtual void construct(const OUString& _rUrl, const cpo::uno::Sequence< css::beans::PropertyValue >& _rInfo );

        // OComponentHelper
        virtual void disposing() override;

        // XServiceInfo
        DECLARE_SERVICE_INFO();

        // XConnection
        virtual css::uno::Reference< css::sdbc::XStatement > createStatement(  ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareStatement( const OUString& sql ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareCall( const OUString& sql ) override;
        virtual OUString nativeSQL( const OUString& sql ) override;
        virtual void setAutoCommit( bool autoCommit ) override;
        virtual bool getAutoCommit(  ) override;
        virtual void commit(  ) override;
        virtual void rollback(  ) override;
        virtual bool isClosed(  ) override final;
        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > getMetaData(  ) override;
        virtual void setReadOnly( bool readOnly ) override;
        virtual bool isReadOnly(  ) override;
        virtual void setCatalog( const OUString& catalog ) override;
        virtual OUString getCatalog(  ) override;
        virtual void setTransactionIsolation( sal_Int32 level ) override;
        virtual sal_Int32 getTransactionIsolation(  ) override;
        virtual css::uno::Reference< css::container::XNameAccess > getTypeMap(  ) override;
        virtual void setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        // XCloseable
        virtual void close(  ) override final;
        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;
        //XUnoTunnel
        virtual sal_Int64 getSomething( const cpo::uno::Sequence< sal_Int8 >& aIdentifier ) override;
        static const cpo::uno::Sequence< sal_Int8 > & getUnoTunnelId();

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
