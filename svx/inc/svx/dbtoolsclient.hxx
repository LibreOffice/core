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

#ifndef SVX_DBTOOLSCLIENT_HXX
#define SVX_DBTOOLSCLIENT_HXX

#include <connectivity/virtualdbtools.hxx>
#include <osl/mutex.hxx>
#include <osl/module.h>
#include <tools/solar.h>
#include <unotools/sharedunocomponent.hxx>
#include "svx/svxdllapi.h"

//........................................................................
namespace svxform
{
//........................................................................

    typedef ::utl::SharedUNOComponent< ::com::sun::star::sdbc::XConnection > SharedConnection;

    //====================================================================
    //= ODbtoolsClient
    //====================================================================
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

        mutable sal_Bool            m_bCreateAlready;

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

    //====================================================================
    //= OStaticDataAccessTools
    //====================================================================
    class SVX_DLLPUBLIC OStaticDataAccessTools : public ODbtoolsClient
    {
    protected:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessTools >    m_xDataAccessTools;

    protected:
        virtual bool ensureLoaded() const;

    public:
        OStaticDataAccessTools();

        const ::rtl::Reference< ::connectivity::simple::IDataAccessTools >& getDataAccessTools() const { return m_xDataAccessTools; }

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> getNumberFormats(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConn,
            sal_Bool _bAllowDefault
        ) const;

        // ------------------------------------------------
        sal_Int32 getDefaultNumberFormat(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _xTypes,
            const ::com::sun::star::lang::Locale& _rLocale );

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection_withFeedback(
            const ::rtl::OUString& _rDataSourceName,
            const ::rtl::OUString& _rUser,
            const ::rtl::OUString& _rPwd,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
        ) const SAL_THROW ( (::com::sun::star::sdbc::SQLException) );

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> connectRowset(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
            sal_Bool _bSetAsActiveConnection
        ) const SAL_THROW ( ( ::com::sun::star::sdbc::SQLException
                            , ::com::sun::star::lang::WrappedTargetException
                            , ::com::sun::star::uno::RuntimeException) );

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getRowSetConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxRowSet)
                const SAL_THROW ( (::com::sun::star::uno::RuntimeException) );

        // ------------------------------------------------
        void TransferFormComponentProperties(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxOld,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxNew,
            const ::com::sun::star::lang::Locale& _rLocale
        ) const;

        // ------------------------------------------------
        ::rtl::OUString quoteName(
            const ::rtl::OUString& _rQuote,
            const ::rtl::OUString& _rName
        ) const;

        // ------------------------------------------------
        ::rtl::OUString composeTableNameForSelect(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xTable
        ) const;

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > getDataSource(
                const ::rtl::OUString& _rsRegisteredName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext
            ) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::INSERT
            @param      _rxCursorSet    the property set
        */
        sal_Bool canInsert(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        /** check if the property "Privileges" supports ::com::sun::star::sdbcx::Privilege::UPDATE
            @param      _rxCursorSet    the property set
        */
        sal_Bool canUpdate(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxCursorSet) const;

        // ------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            getFieldsByCommandDescriptor(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const sal_Int32 _nCommandType,
                const ::rtl::OUString& _rCommand,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxKeepFieldsAlive,
                ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL
            )   SAL_THROW( ( ) );

        // ------------------------------------------------
        bool isEmbeddedInDatabase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent,
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxActualConnection
        );

        // ------------------------------------------------
        bool isEmbeddedInDatabase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent
        );
    };

    //====================================================================
    //= DBToolsObjectFactory
    //====================================================================
    class SVX_DLLPUBLIC DBToolsObjectFactory : public ODbtoolsClient
    {
    public:
        DBToolsObjectFactory();
        ~DBToolsObjectFactory();

        // ------------------------------------------------
        ::std::auto_ptr< ::dbtools::FormattedColumnValue >  createFormattedColumnValue(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_DBTOOLSCLIENT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
