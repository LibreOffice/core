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

#ifndef DBAUI_SUBCOMPONENTCONTROLLER_HXX
#define DBAUI_SUBCOMPONENTCONTROLLER_HXX

#include "genericcontroller.hxx"

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
#include <cppuhelper/implbase2.hxx>

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= DBSubComponentController
    //====================================================================
    class DBSubComponentController;

    typedef ::cppu::ImplInheritanceHelper2  <   OGenericUnoController
                                            ,   ::com::sun::star::document::XScriptInvocationContext
                                            ,   ::com::sun::star::util::XModifiable
                                            >   DBSubComponentController_Base;

    struct DBSubComponentController_Impl;
    class DBACCESS_DLLPUBLIC DBSubComponentController : public DBSubComponentController_Base
    {
    private:
        ::std::auto_ptr<DBSubComponentController_Impl> m_pImpl;

    private:
        /** forces usage of a connection which we do not own
            <p>To be used from within XInitialization::initialize, resp. impl_initialize, only.</p>
        */
        void        initializeConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxForeignConn );

    protected:
        // OGenericUnoController - initialization
        virtual void impl_initialize();

        // OGenericUnoController
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getPrivateModel() const;

                sal_Bool impl_isModified() const;
        virtual void     impl_onModifyChanged();

    public:

        sal_Bool        isReadOnly()            const;
        sal_Bool        isEditable()            const;
        void            setEditable(sal_Bool _bEditable);

        // ----------------------------------------------------------------
        // asking for connection-related stuff

        sal_Bool    isConnected() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >
                    getMetaData( ) const;

        // ----------------------------------------------------------------
        // access to the data source / document
        OUString getDataSourceName() const;
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >&
                    getDataSource() const;
        sal_Bool    haveDataSource() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
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
        sal_Bool hasError() const;

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
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >&
                    getConnection() const;

        /** returns the number formatter
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getNumberFormatter() const;

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL attachModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel) throw( ::com::sun::star::uno::RuntimeException );

        // XScriptInvocationContext
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (::com::sun::star::uno::RuntimeException);

        // XModifiable
        virtual ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setModified( ::sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        DBSubComponentController(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxORB);
        virtual ~DBSubComponentController();

        virtual void        disconnect();
        virtual void        reconnect( sal_Bool _bUI );
                sal_Bool    ensureConnected( sal_Bool _bUI )    { if ( !isConnected() ) reconnect( _bUI ); return isConnected(); }

        /** called when our connection is beeing disposed
            <p>The default implementation does a reconnect</p>
        */
        virtual void losingConnection( );

    protected:
        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // OComponentHelper
        virtual void SAL_CALL disposing();

        // XInterface
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        sal_Int32 getCurrentStartNumber() const;

    private:
        DBSubComponentController();    // never implemented
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // DBAUI_SUBCOMPONENTCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
