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

#ifndef INCLUDED_SVX_DBTOOLSCLIENT_HXX
#define INCLUDED_SVX_DBTOOLSCLIENT_HXX

#include <connectivity/virtualdbtools.hxx>
#include <osl/mutex.hxx>
#include <osl/module.h>
#include <svx/svxdllapi.h>
#include <tools/solar.h>
#include <unotools/sharedunocomponent.hxx>

namespace svxform
{
    typedef ::utl::SharedUNOComponent< ::com::sun::star::sdbc::XConnection > SharedConnection;

    /** base class for classes which want to use dbtools features with load-on-call
        of the dbtools lib.
    */
    class SVX_DLLPUBLIC ODbtoolsClient
    {
    private:
        static  sal_Int32       s_nClients;
        static oslModule        s_hDbtoolsModule;
        static ::connectivity::simple::createDataAccessToolsFactoryFunction
                                s_pFactoryCreationFunc;

        mutable bool            m_bCreateAlready;

    private:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessToolsFactory > m_xDataAccessFactory;

    protected:
        ODbtoolsClient();
        virtual ~ODbtoolsClient();

        virtual bool ensureLoaded() const;

    protected:
        const ::rtl::Reference< ::connectivity::simple::IDataAccessToolsFactory >&
            getFactory() const { return m_xDataAccessFactory; }

    private:
        static void registerClient();
        static void revokeClient();
    };

    class SVX_DLLPUBLIC OStaticDataAccessTools : public ODbtoolsClient
    {
    protected:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessTools >    m_xDataAccessTools;

    protected:
        virtual bool ensureLoaded() const SAL_OVERRIDE;

    public:
        OStaticDataAccessTools();

        const ::rtl::Reference< ::connectivity::simple::IDataAccessTools >& getDataAccessTools() const { return m_xDataAccessTools; }

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> getNumberFormats(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
            sal_Bool _bAllowDefault
        ) const;

        sal_Int32 getDefaultNumberFormat(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _xTypes,
            const ::com::sun::star::lang::Locale& _rLocale );

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection_withFeedback(
            const OUString& _rDataSourceName,
            const OUString& _rUser,
            const OUString& _rPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext
        ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException) );

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> connectRowset(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext,
            sal_Bool _bSetAsActiveConnection
        ) const SAL_THROW ( ( ::com::sun::star::sdbc::SQLException
                            , ::com::sun::star::lang::WrappedTargetException
                            , ::com::sun::star::uno::RuntimeException) );

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getRowSetConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet)
                const SAL_THROW ( (::com::sun::star::uno::RuntimeException) );

        void TransferFormComponentProperties(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxOld,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxNew,
            const ::com::sun::star::lang::Locale& _rLocale
        ) const;

        OUString quoteName(
            const OUString& _rQuote,
            const OUString& _rName
        ) const;

        OUString composeTableNameForSelect(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable
        ) const;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > getDataSource(
                const OUString& _rsRegisteredName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext
            ) const;

        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::INSERT
            @param      _rxCursorSet    the property set
        */
        bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::UPDATE
            @param      _rxCursorSet    the property set
        */
        bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            getFieldsByCommandDescriptor(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const sal_Int32 _nCommandType,
                const OUString& _rCommand,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxKeepFieldsAlive,
                ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
            )   SAL_THROW( ( ) );

        bool isEmbeddedInDatabase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent,
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxActualConnection
        );

        bool isEmbeddedInDatabase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent
        );
    };

    class SVX_DLLPUBLIC DBToolsObjectFactory : public ODbtoolsClient
    {
    public:
        DBToolsObjectFactory();
        virtual ~DBToolsObjectFactory();

        ::std::auto_ptr< ::dbtools::FormattedColumnValue >  createFormattedColumnValue(
            const css::uno::Reference<css::uno::XComponentContext>& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );
    };

}   // namespace svxform


#endif // INCLUDED_SVX_DBTOOLSCLIENT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
