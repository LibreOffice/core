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

#ifndef DBACCESS_CORE_API_ROWSET_HXX
#define DBACCESS_CORE_API_ROWSET_HXX

#include "apitools.hxx"
#include "RowSetBase.hxx"

#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/XRowsChangeBroadcaster.hpp>

#include <cppuhelper/compbase12.hxx>
#include <connectivity/paramwrapper.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/warningscontainer.hxx>

namespace dbaccess
{
    typedef ::cppu::WeakAggComponentImplHelper12    <   ::com::sun::star::sdb::XResultSetAccess
                                                    ,   ::com::sun::star::sdb::XRowSetApproveBroadcaster
                                                    ,   ::com::sun::star::sdb::XRowsChangeBroadcaster
                                                    ,   ::com::sun::star::sdbcx::XDeleteRows
                                                    ,   ::com::sun::star::sdbc::XParameters
                                                    ,   ::com::sun::star::lang::XEventListener
                                                    ,   ::com::sun::star::sdbc::XResultSetUpdate
                                                    ,   ::com::sun::star::sdbc::XRowUpdate
                                                    ,   ::com::sun::star::util::XCancellable
                                                    ,   ::com::sun::star::sdb::XCompletedExecution
                                                    ,   ::com::sun::star::sdb::XParametersSupplier
                                                    ,   ::com::sun::star::sdbc::XWarningsSupplier
                                                    >   ORowSet_BASE1;

    class OTableContainer;
    class ORowSet : public comphelper::OBaseMutex
                    , public ORowSet_BASE1
                    , public ORowSetBase
                    , public ::comphelper::OPropertyArrayUsageHelper<ORowSet>
    {
        friend class ORowSetClone;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xOldConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xActiveConnection;
        ::com::sun::star::uno::Any                                                      m_aActiveConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xTypeMap;
        ::com::sun::star::uno::Any                                                      m_aTypeMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement >  m_xStatement;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >   m_xComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumns; // the columns from a table or query

        connectivity::OWeakRefArray                 m_aClones;
        /** our parameters as XPropertySet instances and ORowSetValue instances
        */
        ::dbtools::param::ParametersContainerRef    m_pParameters;
        /** our parameters values, used when we do not yet have a parameters container
            (since we have not been executed, yet)
        */
        ORowSetValueVector                          m_aPrematureParamValues;
        ORowSetValueVector                          m_aParameterValueForCache;
        ::std::vector<bool>                         m_aParametersSet;
        ::std::vector<bool>                         m_aReadOnlyDataColumns;

        ::cppu::OInterfaceContainerHelper           m_aRowsetListeners;
        ::cppu::OInterfaceContainerHelper           m_aApproveListeners;
        ::cppu::OInterfaceContainerHelper           m_aRowsChangeListener;

        ::dbtools::WarningsContainer                m_aWarnings;

        OTableContainer*                            m_pTables;

        OUString                               m_aCommand;
        OUString                               m_aDataSourceName;
        OUString                               m_aURL;
        OUString                               m_aUser;
        OUString                               m_aPassword;
        OUString                               m_aFilter;
        OUString                               m_aHavingClause;
        OUString                               m_aGroupBy;
        OUString                               m_aOrder;
        OUString                               m_aActiveCommand;
        OUString                               m_aCursorName;
        OUString                               m_aUpdateCatalogName; // is set by a query
        OUString                               m_aUpdateSchemaName; // is set by a query
        OUString                               m_aUpdateTableName; // is set by a query

        sal_Int32                   m_nFetchDirection;
        sal_Int32                   m_nFetchSize;
        sal_Int32                   m_nMaxFieldSize;
        sal_Int32                   m_nMaxRows;
        sal_Int32                   m_nQueryTimeOut;
        sal_Int32                   m_nCommandType;
        sal_Int32                   m_nTransactionIsolation;
        sal_Int32                   m_nPrivileges;
        sal_Int32                   m_nLastKnownRowCount;
        oslInterlockedCount         m_nInAppend;
        sal_Bool                    m_bLastKnownRowCountFinal;
        sal_Bool                    m_bUseEscapeProcessing ;
        sal_Bool                    m_bApplyFilter ;
        sal_Bool                    m_bCommandFacetsDirty;  // any of the facets which define the active command is dirty
        sal_Bool                    m_bModified ;
        sal_Bool                    m_bRebuildConnOnExecute ;
        sal_Bool                    m_bIsBookmarkable ;
        sal_Bool                    m_bNew ;
        sal_Bool                    m_bCanUpdateInsertedRows;
        sal_Bool                    m_bOwnConnection;
        sal_Bool                    m_bPropChangeNotifyEnabled;

    private:
        /** builds m_aActiveCommand from our settings

            @return
                whether we should use escape processing before executing the actual command. This is determined
                from our own EscapeProcessing property, and possibly overruled by the respective property
                of a query we're based on.
        */
        sal_Bool        impl_buildActiveCommand_throw();

        /** initializes our query composer, and everything which has to do with it

            If we don't use escape processing, then we don't have a composer, and everything
            related to it. Nonetheless, _out_rCommandToExecute and the return value are properly
            initialized.

            @param _out_rCommandToExecute
                The command which is to be executed, according to the current settings -
                it is built from our active command plus our current filter/order criterions.

            @return
                whether we should use escape processing before executing the actual command. This is determined
                from our own EscapeProcessing property, and possibly overruled by the respective property
                of a query we're based on.
                Effectively, this value determines whether or not we actually have an composer (m_xComposer)
                and dependent information (such as the parameters container).

            @precond
                m_xActiveConnection points to a valid SDB-level connection

            @throws com::sun::star::sdb::SQLException
                if an database-related error occurred

            @throws com::sun::star::uno::RuntimeException
                if any of the components involved throws a com::sun::star::uno::RuntimeException
        */
        sal_Bool        impl_initComposer_throw( OUString& _out_rCommandToExecute );

        /** returns the table container of our active connection

            If our connection is able to provide a tables container, this one is returned.
            Else, if m_pTables is not <NULL/>, this one will returned.
            Else, m_pTables will be constructed and returned.

            @precond m_xActiveConnection is not <NULL/>
            @throws ::com::sun::star::sdbc::SQLException
                if retrieving or constructing the tables container goes wrong

            @see impl_resetTables_nothrow
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    impl_getTables_throw();

        /** cleans up m_pTables, and resets it to <NULL/>
        */
        void        impl_resetTables_nothrow();

        /** prepares and executes our command
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
                        impl_prepareAndExecute_throw();

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >  calcConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxHandler) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException );
        // free clones and ParseTree. Plus, if _bComplete is <TRUE/>, *all* other associated resources
        void freeResources( bool _bComplete );

        /// informs the clones (and ourself) that we are going to delete a record with a given bookmark
        void notifyRowSetAndClonesRowDelete( const ::com::sun::star::uno::Any& _rBookmark );

        /// inform the clones (and ourself) that we have deleted a record with a given bookmark
        void notifyRowSetAndClonesRowDeleted( const ::com::sun::star::uno::Any& _rBookmark, sal_Int32 _nPos );

        void checkUpdateIterator();
        const connectivity::ORowSetValue& getInsertValue(sal_Int32 columnIndex);
        void setParameter(sal_Int32 parameterIndex, const connectivity::ORowSetValue& x);
        // resizes the parameter vector if necessary
        ::connectivity::ORowSetValue& getParameterStorage( sal_Int32 parameterIndex );

        void updateValue(sal_Int32 columnIndex,const connectivity::ORowSetValue& x);
        void checkUpdateConditions(sal_Int32 columnIndex);
        void impl_rebuild_throw(::osl::ResettableMutexGuard& _rGuard);
        // set all data columns to writeable
        void impl_setDataColumnsWriteable_throw();
        // restore the old state of the data column read-only state
        void impl_restoreDataColumnsWriteable_throw();

    protected:
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue,sal_Int32 nHandle) const;
        virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const;

        virtual void fireRowcount();
                void notifyAllListenersRowBeforeChange(::osl::ResettableMutexGuard& _rGuard,const ::com::sun::star::sdb::RowChangeEvent &rEvt);
                void notifyAllListenersRowChanged(::osl::ResettableMutexGuard& _rGuard,const ::com::sun::star::sdb::RowsChangeEvent& rEvt);
        virtual sal_Bool notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& _rGuard);
        virtual void notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& _rGuard);
        virtual void notifyAllListeners(::osl::ResettableMutexGuard& _rGuard);

        virtual void        doCancelModification( );
        virtual sal_Bool    isModification( );
        virtual sal_Bool    isModified( );
        virtual sal_Bool    isNew( );
        virtual sal_Bool    isPropertyChangeNotificationEnabled() const;

        virtual ~ORowSet();

    public:
        ORowSet(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

    // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::uno::XAggregation
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    // OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XCloseable
        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // ::com::sun::star::sdbc::XResultSet
        virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XCompletedExecution
        virtual void SAL_CALL executeWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XParametersSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getParameters(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRowUpdate
        virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateString( sal_Int32 columnIndex, const OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRowSet
        virtual void SAL_CALL execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addRowSetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRowSetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdb::XRowSetApproveBroadcaster
        virtual void SAL_CALL addRowSetApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRowSetApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdb::XRowsChangeBroadcaster
        virtual void SAL_CALL addRowsChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowsChangeListener >& listener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRowsChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowsChangeListener >& listener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdb::XResultSetAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL createResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XCancellable
        virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XParameters
        virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setString( sal_Int32 parameterIndex, const OUString& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBytes( sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBinaryStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCharacterStream( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObjectWithInfo( sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setRef( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBlob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setClob( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setArray( sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& x ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearParameters(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // XWarningsSupplier
        virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearWarnings(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    protected:
        /** implement the <method>execute</method>, without calling the approve listeners and without building a new
            connection
            @param      _rClearForNotification      mutex to clear before doing the final notifications
        */
        void    execute_NoApprove_NoNewConn(::osl::ResettableMutexGuard& _rClearForNotification);

        /** call the RowSetApproveListeners<p/>
            throws an RowSetVetoException if one of the listeners vetoed
        */
        void    approveExecution() throw (::com::sun::star::sdb::RowSetVetoException, ::com::sun::star::uno::RuntimeException);

        /// set m_xActiveConnection, fire a PropertyChangeEvent if necessary, do the event listener handling etc
        void setActiveConnection( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxNewConn, sal_Bool _bFireEvent = sal_True );

        void implCancelRowUpdates( sal_Bool _bNotifyModified ) SAL_THROW( ( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException ) );

        /** sets the given result set type/concurrency at the given statement, while respecting
            possibly related data source settings
        */
        void        setStatementResultSetType(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxStatement,
            sal_Int32 _nDesiredResultSetType,
            sal_Int32 _nDesiredResultSetConcurrency
        );

        /** initializes a given RowSet column with the ColumnSettings (width, format, hidden, etc.) from a
            template column.

            If the template column supports any of the known column settings, they're plain copied. If not,
            the template column is examined for a TableName and Name property, and the table column described
            by those is used to find and copy the column settings.
        */
        void    impl_initializeColumnSettings_nothrow(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxTemplateColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxRowSetColumn
        );

        /** initializes our parameters container (m_pParameters) according to the parameter columns as
            obtained from our composer
        */
        void    impl_initParametersContainer_nothrow();
        /** disposes our parameters container
        */
        void    impl_disposeParametersContainer_nothrow();

    protected:
        using ORowSetBase::getFastPropertyValue;
        using ORowSetBase::firePropertyChange;
        using ORowSetBase::doCancelModification;
        using ORowSetBase::isModification;
        using ORowSetBase::isModified;
        using ORowSetBase::isNew;
    };


    //************************************************************
    //  ORowSetClone
    //************************************************************
    class ORowSetClone : public comphelper::OBaseMutex
                         ,public OSubComponent
                         ,public ORowSetBase
                         ,public ::comphelper::OPropertyArrayUsageHelper < ORowSetClone >
    {
    protected:
        ORowSet*                    m_pParent;
        sal_Int32                   m_nFetchDirection;
        sal_Int32                   m_nFetchSize;
        sal_Bool                    m_bIsBookmarkable;

    protected:
        // the clone can not insert anything
        virtual void        doCancelModification( );
        virtual sal_Bool    isModification( );
        virtual sal_Bool    isModified( );
        virtual sal_Bool    isNew( );

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);
    public:
        ORowSetClone( const css::uno::Reference<css::uno::XComponentContext>& _rContext, ORowSet& rParent, ::osl::Mutex* _pMutex );
        virtual ~ORowSetClone();

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
        {
            return getUnoTunnelImplementationId();
        }

    // com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

    // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    // OComponentHelper
        virtual void SAL_CALL disposing(void);

    // ::com::sun::star::sdbc::XCloseable
        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
        {
            return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
        }

    // ::com::sun::star::sdbc::XRowSet
        virtual void SAL_CALL execute(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addRowSetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRowSetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener ) throw(::com::sun::star::uno::RuntimeException);

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    protected:
        using ORowSetBase::doCancelModification;
        using ORowSetBase::isModification;
        using ORowSetBase::isModified;
        using ORowSetBase::isNew;
        using ORowSetBase::rowDeleted;
    };

}
#endif // DBACCESS_CORE_API_ROWSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
