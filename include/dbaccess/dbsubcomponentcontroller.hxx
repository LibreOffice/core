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

#ifndef INCLUDED_DBACCESS_DBSUBCOMPONENTCONTROLLER_HXX
#define INCLUDED_DBACCESS_DBSUBCOMPONENTCONTROLLER_HXX

#include <dbaccess/genericcontroller.hxx>

#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <connectivity/dbmetadata.hxx>
#include <cppuhelper/implbase.hxx>

#include <memory>


namespace dbaui
{



    //= DBSubComponentController

    class DBSubComponentController;

    typedef ::cppu::ImplInheritanceHelper<   OGenericUnoController
                                         ,   css::document::XScriptInvocationContext
                                         ,   css::util::XModifiable
                                         >   DBSubComponentController_Base;

    struct DBSubComponentController_Impl;
    class DBACCESS_DLLPUBLIC DBSubComponentController : public DBSubComponentController_Base
    {
    private:
        ::std::unique_ptr<DBSubComponentController_Impl> m_pImpl;

    private:
        /** forces usage of a connection which we do not own
            <p>To be used from within XInitialization::initialize, resp. impl_initialize, only.</p>
        */
        void        initializeConnection( const css::uno::Reference< css::sdbc::XConnection >& _rxForeignConn );

    protected:
        // OGenericUnoController - initialization
        virtual void impl_initialize() override;

        // OGenericUnoController
        virtual void      Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

        virtual css::uno::Reference< css::frame::XModel > getPrivateModel() const override;

                bool     impl_isModified() const;
        virtual void     impl_onModifyChanged();

    public:

        bool            isReadOnly()            const;
        bool            isEditable()            const;
        void            setEditable(bool _bEditable);


        // asking for connection-related stuff

        bool            isConnected() const;

        css::uno::Reference< css::sdbc::XDatabaseMetaData >
                        getMetaData( ) const;


        // access to the data source / document
        OUString getDataSourceName() const;
        const css::uno::Reference< css::beans::XPropertySet >&
                    getDataSource() const;
        bool        haveDataSource() const;

        css::uno::Reference< css::frame::XModel >
                    getDatabaseDocument() const;

        /** provides access to the SDB-level database meta data of the current connection
        */
        const ::dbtools::DatabaseMetaData& getSdbMetaData() const;

        /** appends an error in the current environment.
        */
        void appendError(
                        const OUString& _rErrorMessage,
                        const ::dbtools::StandardSQLState _eSQLState = ::dbtools::SQL_GENERAL_ERROR,
                        const sal_Int32 _nErrorCode = 1000
                     );

        /** clears the error state.
        */
        void clearError();

        /** @return
                <TRUE/> when an error was set otherwise <FALSE/>
        */
        bool hasError() const;

        /** returns the current error
        */
        const ::dbtools::SQLExceptionInfo& getError() const;

        /** displays the current error, or does nothing if there is no current error
        */
        void displayError();

        /** shows an info box with the string conntection lost.
        */
        void connectionLostMessage() const;

        /** gives access to the currently used connection
            @return
                the currently used connection.
        */
        const css::uno::Reference< css::sdbc::XConnection >&
                    getConnection() const;

        /** returns the number formatter
        */
        css::uno::Reference< css::util::XNumberFormatter >    getNumberFormatter() const;

        // css::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL attachModel(const css::uno::Reference< css::frame::XModel > & xModel) throw( css::uno::RuntimeException, std::exception ) override;

        // XScriptInvocationContext
        virtual css::uno::Reference< css::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (css::uno::RuntimeException, std::exception) override;

        // XModifiable
        virtual sal_Bool SAL_CALL isModified(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setModified( sal_Bool bModified ) throw (css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        DBSubComponentController(const css::uno::Reference< css::uno::XComponentContext>& _rxORB);
        virtual ~DBSubComponentController();

        void                disconnect();
        virtual void        reconnect( bool _bUI );
                bool        ensureConnected( bool _bUI )    { if ( !isConnected() ) reconnect( _bUI ); return isConnected(); }

        /** called when our connection is being disposed
            <p>The default implementation does a reconnect</p>
        */
        virtual void losingConnection( );

    protected:
        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        // XInterface
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        sal_Int32 getCurrentStartNumber() const;

    private:
        DBSubComponentController();    // never implemented
    };


}   // namespace dbaui


#endif // INCLUDED_DBACCESS_DBSUBCOMPONENTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
