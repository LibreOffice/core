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

#include "dbu_reghelper.hxx"
#include "dbu_resource.hrc"
#include "dbu_uno.hrc"
#include "dbustrings.hrc"
#include "moduledbu.hxx"
#include "sqlmessage.hxx"
#include "WCopyTable.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/application/XCopyTableWizard.hpp>
#include <com/sun/star/sdb/application/CopyTableContinuation.hpp>
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XBlob.hpp>
#include <com/sun/star/sdbc/XClob.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/sdbc/ConnectionPool.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/proparrhlp.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ustrbuf.hxx>
#include <svtools/genericunodialog.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::sdb::application::XCopyTableWizard;
    using ::com::sun::star::sdb::application::XCopyTableListener;
    using ::com::sun::star::sdb::application::CopyTableRowEvent;
    using ::com::sun::star::beans::Optional;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::lang::NotInitializedException;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XDataSource;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::task::InteractionHandler;
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::sdb::DatabaseContext;
    using ::com::sun::star::sdb::XDatabaseContext;
    using ::com::sun::star::sdb::XDocumentDataSource;
    using ::com::sun::star::sdb::XCompletedConnection;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::sdbcx::XTablesSupplier;
    using ::com::sun::star::sdb::XQueriesSupplier;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::sdbc::XPreparedStatement;
    using ::com::sun::star::sdb::XSingleSelectQueryComposer;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::sdbcx::XColumnsSupplier;
    using ::com::sun::star::sdbc::XParameters;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdbc::XRow;
    using ::com::sun::star::sdbc::XBlob;
    using ::com::sun::star::sdbc::XClob;
    using ::com::sun::star::sdbcx::XRowLocate;
    using ::com::sun::star::sdbc::XResultSetMetaDataSupplier;
    using ::com::sun::star::sdbc::XResultSetMetaData;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdb::SQLContext;
    using ::com::sun::star::sdbc::ConnectionPool;
    using ::com::sun::star::sdbc::XConnectionPool;
    using ::com::sun::star::sdbc::XDriverManager;
    using ::com::sun::star::sdbc::DriverManager;
    using ::com::sun::star::beans::PropertyValue;

    namespace CopyTableOperation = ::com::sun::star::sdb::application::CopyTableOperation;
    namespace CopyTableContinuation = ::com::sun::star::sdb::application::CopyTableContinuation;
    namespace CommandType = ::com::sun::star::sdb::CommandType;
    namespace DataType = ::com::sun::star::sdbc::DataType;

    typedef ::utl::SharedUNOComponent< XConnection >    SharedConnection;

    // CopyTableWizard
    typedef ::svt::OGenericUnoDialog        CopyTableWizard_DialogBase;
    typedef ::cppu::ImplInheritanceHelper1  <   CopyTableWizard_DialogBase
                                            ,   XCopyTableWizard
                                            >   CopyTableWizard_Base;
    class CopyTableWizard
            :public CopyTableWizard_Base
            ,public ::comphelper::OPropertyArrayUsageHelper< CopyTableWizard >
    {
    public:
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // XServiceInfo - static methods
        static Sequence< OUString >  getSupportedServiceNames_Static(void) throw( RuntimeException );
        static OUString              getImplementationName_Static(void) throw( RuntimeException );
        static Reference< XInterface >      Create( const Reference< XMultiServiceFactory >& );

        // XCopyTableWizard
        virtual ::sal_Int16 SAL_CALL getOperation() throw (RuntimeException);
        virtual void SAL_CALL setOperation( ::sal_Int16 _operation ) throw (IllegalArgumentException, RuntimeException);
        virtual OUString SAL_CALL getDestinationTableName() throw (RuntimeException);
        virtual void SAL_CALL setDestinationTableName( const OUString& _destinationTableName ) throw (RuntimeException);
        virtual Optional< OUString > SAL_CALL getCreatePrimaryKey() throw (RuntimeException);
        virtual void SAL_CALL setCreatePrimaryKey( const Optional< OUString >& _newPrimaryKey ) throw (IllegalArgumentException, RuntimeException);
        virtual sal_Bool SAL_CALL getUseHeaderLineAsColumnNames() throw (RuntimeException);
        virtual void SAL_CALL setUseHeaderLineAsColumnNames( sal_Bool _bUseHeaderLineAsColumnNames ) throw (RuntimeException);
        virtual void SAL_CALL addCopyTableListener( const Reference< XCopyTableListener >& Listener ) throw (RuntimeException);
        virtual void SAL_CALL removeCopyTableListener( const Reference< XCopyTableListener >& Listener ) throw (RuntimeException);

        // XCopyTableWizard::XExecutableDialog
        virtual void SAL_CALL setTitle( const OUString& aTitle ) throw (RuntimeException);
        virtual ::sal_Int16 SAL_CALL execute(  ) throw (RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);

        // XPropertySet
        virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    public:
        ::osl::Mutex&   getMutex() { return m_aMutex; }
        bool            isInitialized() const { return m_xSourceConnection.is() && m_pSourceObject.get() && m_xDestConnection.is(); }

    protected:
        CopyTableWizard( const Reference< XComponentContext >& _rxORB );
        ~CopyTableWizard();

        // OGenericUnoDialog overridables
        virtual Dialog* createDialog( Window* _pParent );
        virtual void executedDialog( sal_Int16 _nExecutionResult );

    private:
        /// ensures our current attribute values are reflected in the dialog
        void    impl_attributesToDialog_nothrow( OCopyTableWizard& _rDialog ) const;

        /// ensures the current dialog settings are reflected in our attributes
        void    impl_dialogToAttributes_nothrow( const OCopyTableWizard& _rDialog );

        /** returns our typed dialog

            @throws ::com::sun::star::uno::RuntimeException
                if we don't have a dialog at the moment the method is called
        */
        OCopyTableWizard&
                impl_getDialog_throw();

        /** returns our typed dialog

            @throws ::com::sun::star::uno::RuntimeException
                if we don't have a dialog at the moment the method is called
        */
        const OCopyTableWizard&
                impl_getDialog_throw() const;

        /** ensures the given argument sequence contains a valid data access descriptor at the given position
            @param _rAllArgs
                the arguments as passed to ->initialize
            @param _nArgPos
                the position within ->_rAllArgs which contains the data access descriptor
            @param _out_rxConnection
                will, upon successful return, contain the connection for the data source
            @param _out_rxDocInteractionHandler
                will, upon successful return, contain the interaction handler which could
                be deduced from database document described by the descriptor, if any.
                (It is possible that the descriptor does not allow to deduce a database document,
                in which case <code>_out_rxDocInteractionHandler</code> will be <NULL/>.)
            @return the data access descriptor
        */
        Reference< XPropertySet >
                impl_ensureDataAccessDescriptor_throw(
                    const Sequence< Any >& _rAllArgs,
                    const sal_Int16 _nArgPos,
                    SharedConnection& _out_rxConnection,
                    Reference< XInteractionHandler >& _out_rxDocInteractionHandler
                ) const;

        /** extracts the source object (table or query) described by the given descriptor,
            relative to m_xSourceConnection
        */
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< ICopyTableSourceObject >
                impl_extractSourceObject_throw(
                    const Reference< XPropertySet >& _rxDescriptor,
                    sal_Int32& _out_rCommandType
                ) const;
        SAL_WNODEPRECATED_DECLARATIONS_POP

        /** extracts the result set to copy records from, and the selection-related aspects, if any.

            Effectively, this method extracts m_xSourceResultSet, m_aSourceSelection, and m_bSourceSelectionBookmarks.

            If an inconsistent/insufficent sub set of those properties is present in the descriptor, and exception
            is thrown.
        */
        void    impl_extractSourceResultSet_throw(
                    const Reference< XPropertySet >& i_rDescriptor
                );

        /** checks whether the given copy source descriptor contains settings which are not
            supported (yet)

            Throws an IllegalArgumentException if the descriptor contains a valid setting, which is
            not yet supported.
        */
        void    impl_checkForUnsupportedSettings_throw(
            const Reference< XPropertySet >& _rxSourceDescriptor ) const;

        /** obtaines the connection described by the given data access descriptor

            If needed and possible, the method will ask the user, using the interaction
            handler associated with the database described by the descriptor.

            All errors are handled with the InteractionHandler associated with the data source,
            if there is one. Else, they will be silenced (but asserted in non-product builds).

            @param _rxDataSourceDescriptor
                the data access descriptor describing the data source whose connection
                should be obtained. Must not be <NULL/>.
            @param _out_rxDocInteractionHandler
                the interaction handler which could be deduced from the descriptor

            @throws RuntimeException
                if anything goes seriously wrong.
        */
        SharedConnection
                impl_extractConnection_throw(
                    const Reference< XPropertySet >& _rxDataSourceDescriptor,
                    Reference< XInteractionHandler >& _out_rxDocInteractionHandler
                ) const;

        /** actually copies the table

            This method is called after the dialog has been successfully executed.
        */
        void    impl_doCopy_nothrow();

        /** creates the INSERT INTO statement
            @param  _xTable The destination table.
        */
        OUString impl_getServerSideCopyStatement_throw( const Reference< XPropertySet >& _xTable );

        /** creates the statement which, when executed, will produce the source data to copy

            If the source object refers to a query which contains parameters, those parameters
            are filled in, using an interaction handler.
        */
        ::utl::SharedUNOComponent< XPreparedStatement >
                impl_createSourceStatement_throw() const;

        /** copies the data rows from the given source result set to the given destination table
        */
        void    impl_copyRows_throw(
                    const Reference< XResultSet >& _rxSourceResultSet,
                    const Reference< XPropertySet >& _rxDestTable
                );

        /** processes an error which occurred during copying

            First, all listeners are ask. If a listener tells to cancel or continue copying, this is reported to the
            method's caller. If a listener tells to ask the user, this is done, and the user's decision is
            reported to the method's caller.

            @return
                <TRUE/> if and only if copying should be continued.
        */
        bool    impl_processCopyError_nothrow(
                    const CopyTableRowEvent& _rEvent );

private:
        Reference<XComponentContext>    m_xContext;

        // attributes
        sal_Int16                       m_nOperation;
        OUString                        m_sDestinationTable;
        Optional< OUString >            m_aPrimaryKeyName;
        sal_Bool                        m_bUseHeaderLineAsColumnNames;

        // source
        SharedConnection                m_xSourceConnection;
        sal_Int32                       m_nCommandType;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< ICopyTableSourceObject >
                                        m_pSourceObject;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        Reference< XResultSet >         m_xSourceResultSet;
        Sequence< Any >                 m_aSourceSelection;
        sal_Bool                        m_bSourceSelectionBookmarks;

        // destination
        SharedConnection                m_xDestConnection;

        // other
        Reference< XInteractionHandler > m_xInteractionHandler;
        ::cppu::OInterfaceContainerHelper
                                        m_aCopyTableListeners;
        sal_Int16                       m_nOverrideExecutionResult;
    };

// MethodGuard
class CopyTableAccessGuard
{
public:
    CopyTableAccessGuard( CopyTableWizard& _rWizard )
        :m_rWizard( _rWizard )
    {
        m_rWizard.getMutex().acquire();
        if ( !m_rWizard.isInitialized() )
            throw NotInitializedException();
    }

    ~CopyTableAccessGuard()
    {
        m_rWizard.getMutex().release();
    }

private:
    CopyTableWizard&    m_rWizard;
};

CopyTableWizard::CopyTableWizard( const Reference< XComponentContext >& _rxORB )
    :CopyTableWizard_Base( _rxORB )
    ,m_xContext( _rxORB )
    ,m_nOperation( CopyTableOperation::CopyDefinitionAndData )
    ,m_sDestinationTable()
    ,m_aPrimaryKeyName( sal_False,  "ID" )
    ,m_bUseHeaderLineAsColumnNames( sal_True )
    ,m_xSourceConnection()
    ,m_nCommandType( CommandType::COMMAND )
    ,m_pSourceObject()
    ,m_xSourceResultSet()
    ,m_aSourceSelection()
    ,m_bSourceSelectionBookmarks( sal_True )
    ,m_xDestConnection()
    ,m_aCopyTableListeners( m_aMutex )
    ,m_nOverrideExecutionResult( -1 )
{
}

CopyTableWizard::~CopyTableWizard()
{
    acquire();

    // protect some members whose dtor might potentially throw
    try { m_xSourceConnection.clear();  }
    catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
    try { m_xDestConnection.clear();  }
    catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

    // TODO: shouldn't we have explicit disposal support? If a listener is registered
    // at our instance, and perhaps holds this our instance by a hard ref, then we'll never
    // be destroyed.
    // However, adding XComponent support to the GenericUNODialog probably requires
    // some thinking - would it break existing clients which do not call a dispose, then?
}

Reference< XInterface > CopyTableWizard::Create( const Reference< XMultiServiceFactory >& _rxFactory )
{
    return *( new CopyTableWizard( comphelper::getComponentContext(_rxFactory) ) );
}

OUString SAL_CALL CopyTableWizard::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

OUString CopyTableWizard::getImplementationName_Static() throw(RuntimeException)
{
    return OUString( "org.openoffice.comp.dbu.CopyTableWizard" );
}

::comphelper::StringSequence SAL_CALL CopyTableWizard::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

::comphelper::StringSequence CopyTableWizard::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = "com.sun.star.sdb.application.CopyTableWizard";
    return aSupported;
}

Reference< XPropertySetInfo > SAL_CALL CopyTableWizard::getPropertySetInfo() throw(RuntimeException)
{
    Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::sal_Int16 SAL_CALL CopyTableWizard::getOperation() throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    return m_nOperation;
}

void SAL_CALL CopyTableWizard::setOperation( ::sal_Int16 _operation ) throw (IllegalArgumentException, RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );

    if  (   ( _operation != CopyTableOperation::CopyDefinitionAndData )
        &&  ( _operation != CopyTableOperation::CopyDefinitionOnly )
        &&  ( _operation != CopyTableOperation::CreateAsView )
        &&  ( _operation != CopyTableOperation::AppendData )
        )
        throw IllegalArgumentException( OUString(), *this, 1 );

    if  (   ( _operation == CopyTableOperation::CreateAsView )
        &&  !OCopyTableWizard::supportsViews( m_xDestConnection )
        )
        throw IllegalArgumentException(
            String( ModuleRes( STR_CTW_NO_VIEWS_SUPPORT ) ),
            *this,
            1
        );

    m_nOperation = _operation;
}

OUString SAL_CALL CopyTableWizard::getDestinationTableName() throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    return m_sDestinationTable;
}

void SAL_CALL CopyTableWizard::setDestinationTableName( const OUString& _destinationTableName ) throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    m_sDestinationTable = _destinationTableName;
}

Optional< OUString > SAL_CALL CopyTableWizard::getCreatePrimaryKey() throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    return m_aPrimaryKeyName;
}

void SAL_CALL CopyTableWizard::setCreatePrimaryKey( const Optional< OUString >& _newPrimaryKey ) throw (IllegalArgumentException, RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );

    if ( _newPrimaryKey.IsPresent && !OCopyTableWizard::supportsPrimaryKey( m_xDestConnection ) )
        throw IllegalArgumentException(
                String( ModuleRes( STR_CTW_NO_PRIMARY_KEY_SUPPORT ) ),
            *this,
            1
        );

    m_aPrimaryKeyName = _newPrimaryKey;
}

sal_Bool SAL_CALL CopyTableWizard::getUseHeaderLineAsColumnNames() throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    return m_bUseHeaderLineAsColumnNames;
}

void SAL_CALL CopyTableWizard::setUseHeaderLineAsColumnNames( sal_Bool _bUseHeaderLineAsColumnNames ) throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    m_bUseHeaderLineAsColumnNames = _bUseHeaderLineAsColumnNames;
}

void SAL_CALL CopyTableWizard::addCopyTableListener( const Reference< XCopyTableListener >& _rxListener ) throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    if ( _rxListener.is() )
        m_aCopyTableListeners.addInterface( _rxListener );
}

void SAL_CALL CopyTableWizard::removeCopyTableListener( const Reference< XCopyTableListener >& _rxListener ) throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    if ( _rxListener.is() )
        m_aCopyTableListeners.removeInterface( _rxListener );
}

void SAL_CALL CopyTableWizard::setTitle( const OUString& _rTitle ) throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );
    CopyTableWizard_DialogBase::setTitle( _rTitle );
}

::sal_Int16 SAL_CALL CopyTableWizard::execute(  ) throw (RuntimeException)
{
    CopyTableAccessGuard aGuard( *this );

    m_nOverrideExecutionResult = -1;
    sal_Int16 nExecutionResult = CopyTableWizard_DialogBase::execute();
    if ( m_nOverrideExecutionResult )
        nExecutionResult = m_nOverrideExecutionResult;

    return nExecutionResult;
}

OCopyTableWizard& CopyTableWizard::impl_getDialog_throw()
{
    OCopyTableWizard* pWizard = dynamic_cast< OCopyTableWizard* >( m_pDialog );
    if ( !pWizard )
        throw DisposedException( OUString(), *this );
    return *pWizard;
}

const OCopyTableWizard& CopyTableWizard::impl_getDialog_throw() const
{
    const OCopyTableWizard* pWizard = dynamic_cast< const OCopyTableWizard* >( m_pDialog );
    if ( !pWizard )
        throw DisposedException( OUString(), *const_cast< CopyTableWizard* >( this ) );
    return *pWizard;
}

void CopyTableWizard::impl_attributesToDialog_nothrow( OCopyTableWizard& _rDialog ) const
{
    // primary key column
    _rDialog.setCreatePrimaryKey( m_aPrimaryKeyName.IsPresent, m_aPrimaryKeyName.Value );
    _rDialog.setUseHeaderLine(m_bUseHeaderLineAsColumnNames);

    // everything else was passed at construction time already
}

void CopyTableWizard::impl_dialogToAttributes_nothrow( const OCopyTableWizard& _rDialog )
{
    m_aPrimaryKeyName.IsPresent = _rDialog.shouldCreatePrimaryKey();
    if ( m_aPrimaryKeyName.IsPresent )
        m_aPrimaryKeyName.Value = _rDialog.getPrimaryKeyName();
    else
        m_aPrimaryKeyName.Value = OUString();

    m_sDestinationTable = _rDialog.getName();

    m_nOperation = _rDialog.getOperation();
    m_bUseHeaderLineAsColumnNames = _rDialog.UseHeaderLine();
}

namespace
{
    /** tries to obtain the InteractionHandler associated with a given data source

        If the data source is a sdb-level data source, it will have a DatabaseDocument associated
        with it. This doocument may have an InteractionHandler used while loading it.

        @throws RuntimeException
            if it occures during invoking any of the data source's methods, or if any of the involved
            components violates its contract by not providing the required interfaces
    */
    Reference< XInteractionHandler > lcl_getInteractionHandler_throw( const Reference< XDataSource >& _rxDataSource, const Reference< XInteractionHandler >& _rFallback )
    {
        Reference< XInteractionHandler > xHandler( _rFallback );

        // try to obtain the document model
        Reference< XModel > xDocumentModel;
        Reference< XDocumentDataSource > xDocDataSource( _rxDataSource, UNO_QUERY );
        if ( xDocDataSource.is() )
            xDocumentModel.set( xDocDataSource->getDatabaseDocument(), UNO_QUERY_THROW );

        // see whether the document model can provide a handler
        if ( xDocumentModel.is() )
        {
            ::comphelper::NamedValueCollection aModelArgs( xDocumentModel->getArgs() );
            xHandler = aModelArgs.getOrDefault( "InteractionHandler", xHandler );
        }

        return xHandler;
    }
    /** tries to obtain the InteractionHandler associated with a given connection

        If the connection belongs to a sdb-level data source, then this data source
        is examined for an interaction handler. Else, <NULL/> is returned.

        @throws RuntimeException
            if it occures during invoking any of the data source's methods, or if any of the involved
            components violates its contract by not providing the required interfaces
    */
    Reference< XInteractionHandler > lcl_getInteractionHandler_throw( const Reference< XConnection >& _rxConnection, const Reference< XInteractionHandler >& _rFallback )
    {
        // try whether there is a data source which the connection belongs to
        Reference< XDataSource > xDataSource;
        Reference< XChild > xAsChild( _rxConnection, UNO_QUERY );
        if ( xAsChild.is() )
            xDataSource = xDataSource.query( xAsChild->getParent() );

        if ( xDataSource.is() )
            return lcl_getInteractionHandler_throw( xDataSource, _rFallback );

        return _rFallback;
    }
}

Reference< XPropertySet > CopyTableWizard::impl_ensureDataAccessDescriptor_throw(
    const Sequence< Any >& _rAllArgs, const sal_Int16 _nArgPos, SharedConnection& _out_rxConnection,
    Reference< XInteractionHandler >& _out_rxDocInteractionHandler ) const
{
    Reference< XPropertySet > xDescriptor;
    _rAllArgs[ _nArgPos ] >>= xDescriptor;

    // the descriptor must be non-NULL, of course
    bool bIsValid = xDescriptor.is();

    // it must support the proper service
    if ( bIsValid )
    {
        Reference< XServiceInfo > xSI( xDescriptor, UNO_QUERY );
        bIsValid =  (   xSI.is()
                    &&  xSI->supportsService( "com.sun.star.sdb.DataAccessDescriptor" ) );
    }

    // it must be able to provide a connection
    if ( bIsValid )
    {
        _out_rxConnection = impl_extractConnection_throw( xDescriptor, _out_rxDocInteractionHandler );
        bIsValid = _out_rxConnection.is();
    }

    if ( !bIsValid )
    {
        throw IllegalArgumentException(
            String( ModuleRes( STR_CTW_INVALID_DATA_ACCESS_DESCRIPTOR ) ),
            *const_cast< CopyTableWizard* >( this ),
            _nArgPos + 1
        );
    }

    return xDescriptor;
}

namespace
{
    bool lcl_hasNonEmptyStringValue_throw( const Reference< XPropertySet >& _rxDescriptor,
        const Reference< XPropertySetInfo > _rxPSI, const OUString& _rPropertyName )
    {
        OUString sValue;
        if ( _rxPSI->hasPropertyByName( _rPropertyName ) )
        {
            OSL_VERIFY( _rxDescriptor->getPropertyValue( _rPropertyName ) >>= sValue );
        }
        return !sValue.isEmpty();
    }
}

void CopyTableWizard::impl_checkForUnsupportedSettings_throw( const Reference< XPropertySet >& _rxSourceDescriptor ) const
{
    OSL_PRECOND( _rxSourceDescriptor.is(), "CopyTableWizard::impl_checkForUnsupportedSettings_throw: illegal argument!" );
    Reference< XPropertySetInfo > xPSI( _rxSourceDescriptor->getPropertySetInfo(), UNO_SET_THROW );
    OUString sUnsupportedSetting;

    const OUString aSettings[] = {
        OUString(PROPERTY_FILTER), OUString(PROPERTY_ORDER), OUString(PROPERTY_HAVING_CLAUSE), OUString(PROPERTY_GROUP_BY)
    };
    for ( size_t i=0; i < sizeof( aSettings ) / sizeof( aSettings[0] ); ++i )
    {
        if ( lcl_hasNonEmptyStringValue_throw( _rxSourceDescriptor, xPSI, aSettings[i] ) )
        {
            sUnsupportedSetting = aSettings[i];
            break;
        }
    }

    if ( !sUnsupportedSetting.isEmpty() )
    {
        OUString sMessage(
            OUString(String(ModuleRes(STR_CTW_ERROR_UNSUPPORTED_SETTING))).
            replaceFirst("$name$", sUnsupportedSetting));
        throw IllegalArgumentException(
            sMessage,
            *const_cast< CopyTableWizard* >( this ),
            1
        );
    }

}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr< ICopyTableSourceObject > CopyTableWizard::impl_extractSourceObject_throw( const Reference< XPropertySet >& _rxDescriptor, sal_Int32& _out_rCommandType ) const
{
    OSL_PRECOND( _rxDescriptor.is() && m_xSourceConnection.is(), "CopyTableWizard::impl_extractSourceObject_throw: illegal arguments!" );

    Reference< XPropertySetInfo > xPSI( _rxDescriptor->getPropertySetInfo(), UNO_SET_THROW );
    if  (   !xPSI->hasPropertyByName( PROPERTY_COMMAND )
        ||  !xPSI->hasPropertyByName( PROPERTY_COMMAND_TYPE )
        )
        throw IllegalArgumentException("Expecting a table or query specification.",
                                       // TODO: resource
                                       *const_cast< CopyTableWizard* >( this ), 1);

    OUString sCommand;
    _out_rCommandType = CommandType::COMMAND;
    OSL_VERIFY( _rxDescriptor->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
    OSL_VERIFY( _rxDescriptor->getPropertyValue( PROPERTY_COMMAND_TYPE ) >>= _out_rCommandType );

    ::std::auto_ptr< ICopyTableSourceObject > pSourceObject;
    Reference< XNameAccess > xContainer;
    switch ( _out_rCommandType )
    {
    case CommandType::TABLE:
    {
        Reference< XTablesSupplier > xSuppTables( m_xSourceConnection.getTyped(), UNO_QUERY );
        if ( xSuppTables.is() )
            xContainer.set( xSuppTables->getTables(), UNO_SET_THROW );
    }
    break;
    case CommandType::QUERY:
    {
        Reference< XQueriesSupplier > xSuppQueries( m_xSourceConnection.getTyped(), UNO_QUERY );
        if ( xSuppQueries.is() )
            xContainer.set( xSuppQueries->getQueries(), UNO_SET_THROW );
    }
    break;
    default:
        throw IllegalArgumentException(
            String( ModuleRes( STR_CTW_ONLY_TABLES_AND_QUERIES_SUPPORT ) ),
            *const_cast< CopyTableWizard* >( this ),
            1
        );
    }

    if ( xContainer.is() )
    {
        pSourceObject.reset( new ObjectCopySource( m_xSourceConnection,
            Reference< XPropertySet >( xContainer->getByName( sCommand ), UNO_QUERY_THROW ) ) );
    }
    else
    {
        // our source connection is an SDBC level connection only, not a SDBCX level one
        // Which means it cannot provide the to-be-copied object as component.

        if ( _out_rCommandType == CommandType::QUERY )
            // we cannot copy a query if the connection cannot provide it ...
            throw IllegalArgumentException(
                String(ModuleRes( STR_CTW_ERROR_NO_QUERY )),
                *const_cast< CopyTableWizard* >( this ),
                1
            );
        pSourceObject.reset( new NamedTableCopySource( m_xSourceConnection, sCommand ) );
    }

    return pSourceObject;
}

SAL_WNODEPRECATED_DECLARATIONS_POP

void CopyTableWizard::impl_extractSourceResultSet_throw( const Reference< XPropertySet >& i_rDescriptor )
{
    Reference< XPropertySetInfo > xPSI( i_rDescriptor->getPropertySetInfo(), UNO_SET_THROW );

    // extract relevant settings
    if ( xPSI->hasPropertyByName( PROPERTY_RESULT_SET ) )
        m_xSourceResultSet.set( i_rDescriptor->getPropertyValue( PROPERTY_RESULT_SET ), UNO_QUERY );

    if ( xPSI->hasPropertyByName( PROPERTY_SELECTION ) )
        OSL_VERIFY( i_rDescriptor->getPropertyValue( PROPERTY_SELECTION ) >>= m_aSourceSelection );

    if ( xPSI->hasPropertyByName( PROPERTY_BOOKMARK_SELECTION ) )
        OSL_VERIFY( i_rDescriptor->getPropertyValue( PROPERTY_BOOKMARK_SELECTION ) >>= m_bSourceSelectionBookmarks );

    // sanity checks
    const bool bHasResultSet = m_xSourceResultSet.is();
    const bool bHasSelection = ( m_aSourceSelection.getLength() != 0 );
    if ( bHasSelection && !bHasResultSet )
        throw IllegalArgumentException("A result set is needed when specifying a selection to copy.",
                                       // TODO: resource
                                       *this, 1);

    if ( bHasSelection && m_bSourceSelectionBookmarks )
    {
        Reference< XRowLocate > xRowLocate( m_xSourceResultSet, UNO_QUERY );
        if ( !xRowLocate.is() )
        {
            ::dbtools::throwGenericSQLException(
                String( ModuleRes( STR_CTW_COPY_SOURCE_NEEDS_BOOKMARKS ) ),
                *this
            );
        }
    }
}

SharedConnection CopyTableWizard::impl_extractConnection_throw( const Reference< XPropertySet >& _rxDataSourceDescriptor,
    Reference< XInteractionHandler >& _out_rxDocInteractionHandler ) const
{
    SharedConnection xConnection;

    OSL_PRECOND( _rxDataSourceDescriptor.is(), "CopyTableWizard::impl_extractConnection_throw: no descriptor!" );
    if ( !_rxDataSourceDescriptor.is() )
        return xConnection;

    Reference< XInteractionHandler > xInteractionHandler;

    do
    {
    Reference< XPropertySetInfo > xPSI( _rxDataSourceDescriptor->getPropertySetInfo(), UNO_SET_THROW );

    // if there's an ActiveConnection, use it
    if ( xPSI->hasPropertyByName( PROPERTY_ACTIVE_CONNECTION ) )
    {
        Reference< XConnection > xPure;
        OSL_VERIFY( _rxDataSourceDescriptor->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xPure );
        xConnection.reset( xPure, SharedConnection::NoTakeOwnership );
    }
    if ( xConnection.is() )
    {
        xInteractionHandler = lcl_getInteractionHandler_throw( xConnection.getTyped(), m_xInteractionHandler );
        OSL_POSTCOND( xInteractionHandler.is(), "CopyTableWizard::impl_extractConnection_throw: lcl_getInteractionHandler_throw returned nonsense!" );
        break;
    }

    // there could be a DataSourceName or a DatabaseLocation, describing the css.sdb.DataSource
    OUString sDataSource, sDatabaseLocation;
    if ( xPSI->hasPropertyByName( PROPERTY_DATASOURCENAME ) )
        OSL_VERIFY( _rxDataSourceDescriptor->getPropertyValue( PROPERTY_DATASOURCENAME ) >>= sDataSource );
    if ( xPSI->hasPropertyByName( PROPERTY_DATABASE_LOCATION ) )
        OSL_VERIFY( _rxDataSourceDescriptor->getPropertyValue( PROPERTY_DATABASE_LOCATION ) >>= sDatabaseLocation );

    // need a DatabaseContext for loading the data source
    Reference< XDatabaseContext > xDatabaseContext = DatabaseContext::create( m_xContext );
    Reference< XDataSource > xDataSource;
    if ( !sDataSource.isEmpty() )
        xDataSource.set( xDatabaseContext->getByName( sDataSource ), UNO_QUERY_THROW );
    if ( !xDataSource.is() && !sDatabaseLocation.isEmpty() )
        xDataSource.set( xDatabaseContext->getByName( sDatabaseLocation ), UNO_QUERY_THROW );

    if ( xDataSource.is() )
    {
        // first, try connecting with completion
        xInteractionHandler = lcl_getInteractionHandler_throw( xDataSource, m_xInteractionHandler );
        OSL_POSTCOND( xInteractionHandler.is(), "CopyTableWizard::impl_extractConnection_throw: lcl_getInteractionHandler_throw returned nonsense!" );
        if ( xInteractionHandler.is() )
        {
            Reference< XCompletedConnection > xInteractiveConnection( xDataSource, UNO_QUERY );
            if ( xInteractiveConnection.is() )
                xConnection.reset( xInteractiveConnection->connectWithCompletion( xInteractionHandler ), SharedConnection::TakeOwnership );
        }

        // interactively connecting was not successful or possible -> connect without interaction
        if ( !xConnection.is() )
        {
            xConnection.reset( xDataSource->getConnection( OUString(), OUString() ), SharedConnection::TakeOwnership );
        }
    }

    if ( xConnection.is() )
        break;

    // finally, there could be a ConnectionResource/ConnectionInfo
    OUString sConnectionResource;
    Sequence< PropertyValue > aConnectionInfo;
    if ( xPSI->hasPropertyByName( PROPERTY_CONNECTION_RESOURCE ) )
        OSL_VERIFY( _rxDataSourceDescriptor->getPropertyValue( PROPERTY_CONNECTION_RESOURCE ) >>= sConnectionResource );
    if ( xPSI->hasPropertyByName( PROPERTY_CONNECTION_INFO ) )
        OSL_VERIFY( _rxDataSourceDescriptor->getPropertyValue( PROPERTY_CONNECTION_INFO ) >>= aConnectionInfo );

    Reference< XDriverManager > xDriverManager;
    try {
        xDriverManager.set( ConnectionPool::create( m_xContext ), UNO_QUERY_THROW );
    } catch( const Exception& ) {  }
    if ( !xDriverManager.is() )
        // no connection pool installed
        xDriverManager.set( DriverManager::create( m_xContext ), UNO_QUERY_THROW );

    if ( aConnectionInfo.getLength() )
        xConnection.set( xDriverManager->getConnectionWithInfo( sConnectionResource, aConnectionInfo ), UNO_SET_THROW );
    else
        xConnection.set( xDriverManager->getConnection( sConnectionResource ), UNO_SET_THROW );
    }
    while ( false );

    if ( xInteractionHandler != m_xInteractionHandler )
        _out_rxDocInteractionHandler = xInteractionHandler;

    return xConnection;
}

::utl::SharedUNOComponent< XPreparedStatement > CopyTableWizard::impl_createSourceStatement_throw() const
{
    OSL_PRECOND( m_xSourceConnection.is(), "CopyTableWizard::impl_createSourceStatement_throw: illegal call!" );
    if ( !m_xSourceConnection.is() )
        throw RuntimeException( "CopyTableWizard::impl_createSourceStatement_throw: illegal call!", *const_cast< CopyTableWizard* >( this ));

    ::utl::SharedUNOComponent< XPreparedStatement > xStatement;
    switch ( m_nCommandType )
    {
    case CommandType::TABLE:
        xStatement.set( m_pSourceObject->getPreparedSelectStatement(), UNO_SET_THROW );
        break;

    case CommandType::QUERY:
    {
        OUString sQueryCommand( m_pSourceObject->getSelectStatement() );
        xStatement.set( m_pSourceObject->getPreparedSelectStatement(), UNO_SET_THROW );

        // check whether we have to fill in parameter values
        // create and fill a composer

        Reference< XMultiServiceFactory > xFactory( m_xSourceConnection, UNO_QUERY );
        ::utl::SharedUNOComponent< XSingleSelectQueryComposer > xComposer;
        if ( xFactory.is() )
            // note: connections below the sdb-level are allowed to not support the XMultiServiceFactory interface
            xComposer.set( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY );

        if ( xComposer.is() )
        {
            xComposer->setQuery( sQueryCommand );

            Reference< XParameters > xStatementParams( xStatement, UNO_QUERY );
            OSL_ENSURE( xStatementParams.is(), "CopyTableWizard::impl_createSourceStatement_throw: no access to the statement's parameters!" );
                // the statement should be a css.sdbc.PreparedStatement (this is what
                // we created), and a prepared statement is required to support XParameters
            if ( xStatementParams.is() )
            {
                OSL_ENSURE( m_xInteractionHandler.is(),
                   "CopyTableWizard::impl_createSourceStatement_throw: no interaction handler for the parameters request!" );
                // we should always have an interaction handler - as last fallback, we create an own one in ::initialize

                if ( m_xInteractionHandler.is() )
                    ::dbtools::askForParameters( xComposer, xStatementParams, m_xSourceConnection, m_xInteractionHandler );
            }
        }
    }
    break;

    default:
        // this should not have survived initialization phase
        throw RuntimeException("No case matched, this should not have survived the initialization phase", *const_cast< CopyTableWizard* >( this ));
    }

    return xStatement;
}

namespace
{
    class ValueTransfer
    {
    public:
        ValueTransfer( const sal_Int32& _rSourcePos, const sal_Int32& _rDestPos, const ::std::vector< sal_Int32 >& _rColTypes,
            const Reference< XRow >& _rxSource, const Reference< XParameters >& _rxDest )
            :m_rSourcePos( _rSourcePos )
            ,m_rDestPos( _rDestPos )
            ,m_rColTypes( _rColTypes )
            ,m_xSource( _rxSource )
            ,m_xDest( _rxDest )
        {
        }

    template< typename VALUE_TYPE >
    void transferValue( VALUE_TYPE ( SAL_CALL XRow::*_pGetter )( sal_Int32 ),
        void (SAL_CALL XParameters::*_pSetter)( sal_Int32, VALUE_TYPE ) )
    {
        VALUE_TYPE value( (m_xSource.get()->*_pGetter)( m_rSourcePos ) );
        if ( m_xSource->wasNull() )
            m_xDest->setNull( m_rDestPos, m_rColTypes[ m_rSourcePos ] );
        else
            (m_xDest.get()->*_pSetter)( m_rDestPos, value );
    }
 template< typename VALUE_TYPE >
    void transferComplexValue( VALUE_TYPE ( SAL_CALL XRow::*_pGetter )( sal_Int32 ),
        void (SAL_CALL XParameters::*_pSetter)( sal_Int32, const VALUE_TYPE& ) )
    {
        const VALUE_TYPE value( (m_xSource.get()->*_pGetter)( m_rSourcePos ) );
               {
        if ( m_xSource->wasNull() )
            m_xDest->setNull( m_rDestPos, m_rColTypes[ m_rSourcePos ] );
        else
            (m_xDest.get()->*_pSetter)( m_rDestPos, value );
               }
    }
    private:
        const sal_Int32&                    m_rSourcePos;
        const sal_Int32&                    m_rDestPos;
        const ::std::vector< sal_Int32 >    m_rColTypes;
        const Reference< XRow >             m_xSource;
        const Reference< XParameters >      m_xDest;
    };
}

bool CopyTableWizard::impl_processCopyError_nothrow( const CopyTableRowEvent& _rEvent )
{
    Reference< XCopyTableListener > xListener;
    try
    {
        ::cppu::OInterfaceIteratorHelper aIter( m_aCopyTableListeners );
        while ( aIter.hasMoreElements() )
        {
            xListener.set( aIter.next(), UNO_QUERY_THROW );
            sal_Int16 nListenerChoice = xListener->copyRowError( _rEvent );
            switch ( nListenerChoice )
            {
            case CopyTableContinuation::Proceed:            return true;    // continue copying
            case CopyTableContinuation::CallNextHandler:    continue;       // continue the loop, ask next listener
            case CopyTableContinuation::Cancel:             return false;   // cancel copying
            case CopyTableContinuation::AskUser:            break;          // stop asking the listeners, ask the user

            default:
                SAL_WARN("dbaccess.ui", "CopyTableWizard::impl_processCopyError_nothrow: invalid listener response!" );
                // ask next listener
                continue;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // no listener felt responsible for the error, or a listener told to ask the user

    try
    {
        SQLContext aError;
        aError.Context = *this;
        aError.Message = String( ModuleRes( STR_ERROR_OCCURRED_WHILE_COPYING ) );

        ::dbtools::SQLExceptionInfo aInfo( _rEvent.Error );
        if ( aInfo.isValid() )
            aError.NextException = _rEvent.Error;
        else
        {
            // a non-SQL exception happened
            Exception aException;
            OSL_VERIFY( _rEvent.Error >>= aException );
            SQLContext aContext;
            aContext.Context = aException.Context;
            aContext.Message = aException.Message;
            aContext.Details = _rEvent.Error.getValueTypeName();
            aError.NextException <<= aContext;
        }

        ::rtl::Reference< ::comphelper::OInteractionRequest > xRequest( new ::comphelper::OInteractionRequest( makeAny( aError ) ) );

        ::rtl::Reference< ::comphelper::OInteractionApprove > xYes = new ::comphelper::OInteractionApprove;
        xRequest->addContinuation( xYes.get() );
        xRequest->addContinuation( new ::comphelper::OInteractionDisapprove );

        OSL_ENSURE( m_xInteractionHandler.is(),
            "CopyTableWizard::impl_processCopyError_nothrow: we always should have an interaction handler!" );
        if ( m_xInteractionHandler.is() )
            m_xInteractionHandler->handle( xRequest.get() );

        if ( xYes->wasSelected() )
            // continue copying
            return true;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // cancel copying
    return false;
}

void CopyTableWizard::impl_copyRows_throw( const Reference< XResultSet >& _rxSourceResultSet,
    const Reference< XPropertySet >& _rxDestTable )
{
    OSL_PRECOND( m_xDestConnection.is(), "CopyTableWizard::impl_copyRows_throw: illegal call!" );
    if ( !m_xDestConnection.is() )
        throw RuntimeException( "m_xDestConnection is set to null, CopyTableWizard::impl_copyRows_throw: illegal call!", *this );

    Reference< XDatabaseMetaData > xDestMetaData( m_xDestConnection->getMetaData(), UNO_QUERY_THROW );

    const OCopyTableWizard& rWizard             = impl_getDialog_throw();
    ODatabaseExport::TPositions aColumnMapping  = rWizard.GetColumnPositions();
    bool bAutoIncrement                         = rWizard.shouldCreatePrimaryKey();

    Reference< XRow > xRow              ( _rxSourceResultSet, UNO_QUERY_THROW );
    Reference< XRowLocate > xRowLocate  ( _rxSourceResultSet, UNO_QUERY_THROW );

    Reference< XResultSetMetaDataSupplier > xSuppResMeta( _rxSourceResultSet, UNO_QUERY_THROW );
    Reference< XResultSetMetaData> xMeta( xSuppResMeta->getMetaData() );

    // we need a vector which all types
    sal_Int32 nCount = xMeta->getColumnCount();
    ::std::vector< sal_Int32 > aSourceColTypes;
    aSourceColTypes.reserve( nCount + 1 );
    aSourceColTypes.push_back( -1 ); // just to avoid a everytime i-1 call

    ::std::vector< sal_Int32 > aSourcePrec;
    aSourcePrec.reserve( nCount + 1 );
    aSourcePrec.push_back( -1 ); // just to avoid a everytime i-1 call

    for ( sal_Int32 k=1; k <= nCount; ++k )
    {
        aSourceColTypes.push_back( xMeta->getColumnType( k ) );
        aSourcePrec.push_back( xMeta->getPrecision( k ) );
    }

    // now create, fill and execute the prepared statement
    Reference< XPreparedStatement > xStatement( ODatabaseExport::createPreparedStatment( xDestMetaData, _rxDestTable, aColumnMapping ), UNO_SET_THROW );
    Reference< XParameters > xStatementParams( xStatement, UNO_QUERY_THROW );

    const bool bSelectedRecordsOnly = m_aSourceSelection.getLength() != 0;
    const Any* pSelectedRow         = m_aSourceSelection.getConstArray();
    const Any* pSelEnd              = pSelectedRow + m_aSourceSelection.getLength();

    sal_Int32 nRowCount = 0;
    bool bContinue = false;

    CopyTableRowEvent aCopyEvent;
    aCopyEvent.Source = *this;
    aCopyEvent.SourceData = _rxSourceResultSet;

    do // loop as long as there are more rows or the selection ends
    {
        bContinue = false;
        if ( bSelectedRecordsOnly )
        {
            if ( pSelectedRow != pSelEnd )
            {
                if ( m_bSourceSelectionBookmarks )
                {
                    bContinue = xRowLocate->moveToBookmark( *pSelectedRow );
                }
                else
                {
                    sal_Int32 nPos = 0;
                    OSL_VERIFY( *pSelectedRow >>= nPos );
                    bContinue = _rxSourceResultSet->absolute( nPos );
                }
                ++pSelectedRow;
            }
        }
        else
            bContinue = _rxSourceResultSet->next();

        if ( !bContinue )
        {
            break;
        }

        ++nRowCount;
        sal_Bool bInsertAutoIncrement = sal_True;
        ODatabaseExport::TPositions::const_iterator aPosIter = aColumnMapping.begin();
        ODatabaseExport::TPositions::const_iterator aPosEnd = aColumnMapping.end();

        aCopyEvent.Error.clear();
        try
        {
            // notify listeners
            m_aCopyTableListeners.notifyEach( &XCopyTableListener::copyingRow, aCopyEvent );

            sal_Int32 nDestColumn( 0 );
            sal_Int32 nSourceColumn( 1 );
            ValueTransfer aTransfer( nSourceColumn, nDestColumn, aSourceColTypes, xRow, xStatementParams );

            for ( ; aPosIter != aPosEnd; ++aPosIter )
            {
                nDestColumn = aPosIter->first;
                if ( nDestColumn == COLUMN_POSITION_NOT_FOUND )
                {
                    ++nSourceColumn;
                    // otherwise we don't get the correct value when only the 2nd source column was selected
                    continue;
                }

                if ( bAutoIncrement && bInsertAutoIncrement )
                {
                    xStatementParams->setInt( 1, nRowCount );
                    bInsertAutoIncrement = sal_False;
                    continue;
                }

                if ( ( nSourceColumn < 1 ) || ( nSourceColumn >= (sal_Int32)aSourceColTypes.size() ) )
                {   // ( we have to check here against 1 because the parameters are 1 based)
                    ::dbtools::throwSQLException("Internal error: invalid column type index.",
                                                 ::dbtools::SQL_INVALID_DESCRIPTOR_INDEX, *this);
                }

                switch ( aSourceColTypes[ nSourceColumn ] )
                {
                    case DataType::DOUBLE:
                    case DataType::REAL:
                        aTransfer.transferValue( &XRow::getDouble, &XParameters::setDouble );
                        break;

                    case DataType::CHAR:
                    case DataType::VARCHAR:
                    case DataType::LONGVARCHAR:
                    case DataType::DECIMAL:
                    case DataType::NUMERIC:
                        aTransfer.transferComplexValue( &XRow::getString, &XParameters::setString );
                        break;

                    case DataType::BIGINT:
                        aTransfer.transferValue( &XRow::getLong, &XParameters::setLong );
                        break;

                    case DataType::FLOAT:
                        aTransfer.transferValue( &XRow::getFloat, &XParameters::setFloat );
                        break;

                    case DataType::LONGVARBINARY:
                    case DataType::BINARY:
                    case DataType::VARBINARY:
                        aTransfer.transferComplexValue( &XRow::getBytes, &XParameters::setBytes );
                        break;

                    case DataType::DATE:
                        aTransfer.transferComplexValue( &XRow::getDate, &XParameters::setDate );
                        break;

                    case DataType::TIME:
                        aTransfer.transferComplexValue( &XRow::getTime, &XParameters::setTime );
                        break;

                    case DataType::TIMESTAMP:
                        aTransfer.transferComplexValue( &XRow::getTimestamp, &XParameters::setTimestamp );
                        break;

                    case DataType::BIT:
                        if ( aSourcePrec[nSourceColumn] > 1 )
                        {
                            aTransfer.transferComplexValue( &XRow::getBytes, &XParameters::setBytes );
                            break;
                        }
                        // run through
                    case DataType::BOOLEAN:
                        aTransfer.transferValue( &XRow::getBoolean, &XParameters::setBoolean );
                        break;

                    case DataType::TINYINT:
                        aTransfer.transferValue( &XRow::getByte, &XParameters::setByte );
                        break;

                    case DataType::SMALLINT:
                        aTransfer.transferValue( &XRow::getShort, &XParameters::setShort );
                        break;

                    case DataType::INTEGER:
                        aTransfer.transferValue( &XRow::getInt, &XParameters::setInt );
                        break;

                    case DataType::BLOB:
                        aTransfer.transferComplexValue( &XRow::getBlob, &XParameters::setBlob );
                        break;

                    case DataType::CLOB:
                        aTransfer.transferComplexValue( &XRow::getClob, &XParameters::setClob );
                        break;

                    default:
                    {
                        OUString aMessage( String( ModuleRes( STR_CTW_UNSUPPORTED_COLUMN_TYPE ) ) );

                        aMessage = aMessage.replaceAt( aMessage.indexOfAsciiL( "$type$", 6 ), 6, OUString::number( aSourceColTypes[ nSourceColumn ] ) );
                        aMessage = aMessage.replaceAt( aMessage.indexOfAsciiL( "$pos$", 5 ), 5, OUString::number( nSourceColumn ) );

                        ::dbtools::throwSQLException(
                            aMessage,
                            ::dbtools::SQL_INVALID_SQL_DATA_TYPE,
                            *this
                        );
                    }
                }
                ++nSourceColumn;
            }
            xStatement->executeUpdate();

            // notify listeners
            m_aCopyTableListeners.notifyEach( &XCopyTableListener::copiedRow, aCopyEvent );
        }
        catch( const Exception& )
        {
            aCopyEvent.Error = ::cppu::getCaughtException();
        }

        if ( aCopyEvent.Error.hasValue() )
            bContinue = impl_processCopyError_nothrow( aCopyEvent );
    }
    while( bContinue );
}

void CopyTableWizard::impl_doCopy_nothrow()
{
    Any aError;

    try
    {
        OCopyTableWizard& rWizard( impl_getDialog_throw() );

        WaitObject aWO( rWizard.GetParent() );
        Reference< XPropertySet > xTable;

        switch ( rWizard.getOperation() )
        {
            case CopyTableOperation::CopyDefinitionOnly:
            case CopyTableOperation::CopyDefinitionAndData:
            {
                xTable = rWizard.createTable();

                if( !xTable.is() )
                {
                    SAL_WARN("dbaccess.ui", "CopyTableWizard::impl_doCopy_nothrow: createTable should throw here, shouldn't it?" );
                    break;
                }

                if( CopyTableOperation::CopyDefinitionOnly == rWizard.getOperation() )
                    break;
            }
            // run through

            case CopyTableOperation::AppendData:
            {

                if ( !xTable.is() )
                {
                    xTable = rWizard.createTable();
                    if ( !xTable.is() )
                    {
                        SAL_WARN("dbaccess.ui", "CopyTableWizard::impl_doCopy_nothrow: createTable should throw here, shouldn't it?" );
                        break;
                    }
                }

                ::utl::SharedUNOComponent< XPreparedStatement > xSourceStatement;
                ::utl::SharedUNOComponent< XResultSet > xSourceResultSet;

                if ( m_xSourceResultSet.is() )
                {
                    xSourceResultSet.reset( m_xSourceResultSet, ::utl::SharedUNOComponent< XResultSet >::NoTakeOwnership );
                }
                else
                {
                    const bool bIsSameConnection = ( m_xSourceConnection.getTyped() == m_xDestConnection.getTyped() );
                    const bool bIsTable = ( CommandType::TABLE == m_nCommandType );
                    bool bDone = false;
                    if ( bIsSameConnection && bIsTable )
                    {
                        // try whether the server supports copying via SQL
                        try
                        {
                            m_xDestConnection->createStatement()->executeUpdate( impl_getServerSideCopyStatement_throw(xTable) );
                            bDone = true;
                        }
                        catch( const Exception& )
                        {
                            // this is allowed.
                        }
                    }

                    if ( !bDone )
                    {
                        xSourceStatement.set( impl_createSourceStatement_throw(), UNO_SET_THROW );
                        xSourceResultSet.set( xSourceStatement->executeQuery(), UNO_SET_THROW );
                    }
                }

                if ( xSourceResultSet.is() )
                    impl_copyRows_throw( xSourceResultSet, xTable );
            }
            break;

            case CopyTableOperation::CreateAsView:
                rWizard.createView();
                break;

            default:
                SAL_WARN("dbaccess.ui", "CopyTableWizard::impl_doCopy_nothrow: What operation, please?" );
                break;
        }
    }
    catch( const Exception& )
    {
        aError = ::cppu::getCaughtException();

        // silence the error of the user cancelling the parameter's dialog
        SQLException aSQLError;
        if ( ( aError >>= aSQLError ) && ( aSQLError.ErrorCode == ::dbtools::ParameterInteractionCancelled ) )
        {
            aError.clear();
            m_nOverrideExecutionResult = RET_CANCEL;
        }
    }

    if ( aError.hasValue() && m_xInteractionHandler.is() )
    {
        try
        {
            ::rtl::Reference< ::comphelper::OInteractionRequest > xRequest( new ::comphelper::OInteractionRequest( aError ) );
            m_xInteractionHandler->handle( xRequest.get() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

OUString CopyTableWizard::impl_getServerSideCopyStatement_throw(const Reference< XPropertySet >& _xTable)
{
    const Reference<XColumnsSupplier> xDestColsSup(_xTable,UNO_QUERY_THROW);
    const Sequence< OUString> aDestColumnNames = xDestColsSup->getColumns()->getElementNames();
    const Sequence< OUString > aColumnNames = m_pSourceObject->getColumnNames();
    const Reference< XDatabaseMetaData > xDestMetaData( m_xDestConnection->getMetaData(), UNO_QUERY_THROW );
    const OUString sQuote = xDestMetaData->getIdentifierQuoteString();
    OUStringBuffer sColumns;
    // 1st check if the columns matching
    const OCopyTableWizard& rWizard             = impl_getDialog_throw();
    ODatabaseExport::TPositions aColumnMapping  = rWizard.GetColumnPositions();
    ODatabaseExport::TPositions::const_iterator aPosIter = aColumnMapping.begin();
    for ( sal_Int32 i = 0; aPosIter != aColumnMapping.end() ; ++aPosIter,++i )
    {
        if ( COLUMN_POSITION_NOT_FOUND != aPosIter->second )
        {
            if ( !sColumns.isEmpty() )
                sColumns.append(",");
            sColumns.append(sQuote + aDestColumnNames[aPosIter->second - 1] + sQuote);
        }
    }
    const OUString sComposedTableName = ::dbtools::composeTableName( xDestMetaData, _xTable, ::dbtools::eInDataManipulation, false, false, true );
    OUString sSql("INSERT INTO " + sComposedTableName + " ( " + sColumns.makeStringAndClear() + " ) ( " + m_pSourceObject->getSelectStatement() + " )");

    return sSql;
}

void SAL_CALL CopyTableWizard::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( isInitialized() )
        throw AlreadyInitializedException( OUString(), *this );

    sal_Int32 nArgCount( _rArguments.getLength() );
    if ( ( nArgCount != 2 ) && ( nArgCount != 3 ) )
        throw IllegalArgumentException(
            String( ModuleRes( STR_CTW_ILLEGAL_PARAMETER_COUNT ) ),
            *this,
            1
        );

    try
    {
        if ( nArgCount == 3 )
        {   // ->createWithInteractionHandler
            if ( !( _rArguments[2] >>= m_xInteractionHandler ) )
                throw IllegalArgumentException(
                    String(ModuleRes( STR_CTW_ERROR_INVALID_INTERACTIONHANDLER )),
                    *this,
                    3
                );
        }
        if ( !m_xInteractionHandler.is() )
            m_xInteractionHandler.set( InteractionHandler::createWithParent(m_xContext, 0), UNO_QUERY );

        Reference< XInteractionHandler > xSourceDocHandler;
        Reference< XPropertySet > xSourceDescriptor( impl_ensureDataAccessDescriptor_throw( _rArguments, 0, m_xSourceConnection, xSourceDocHandler ) );
        impl_checkForUnsupportedSettings_throw( xSourceDescriptor );
        m_pSourceObject = impl_extractSourceObject_throw( xSourceDescriptor, m_nCommandType );
        impl_extractSourceResultSet_throw( xSourceDescriptor );

        Reference< XInteractionHandler > xDestDocHandler;
        impl_ensureDataAccessDescriptor_throw( _rArguments, 1, m_xDestConnection, xDestDocHandler );

        if ( xDestDocHandler.is() && !m_xInteractionHandler.is() )
            m_xInteractionHandler = xDestDocHandler;
    }
    catch( const RuntimeException& ) { throw; }
    catch( const SQLException& ) { throw; }
    catch( const Exception& )
    {
        throw WrappedTargetException(
            String( ModuleRes( STR_CTW_ERROR_DURING_INITIALIZATION ) ),
            *this,
            ::cppu::getCaughtException()
        );
    }
}

::cppu::IPropertyArrayHelper& CopyTableWizard::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* CopyTableWizard::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

Dialog* CopyTableWizard::createDialog( Window* _pParent )
{
    OSL_PRECOND( isInitialized(), "CopyTableWizard::createDialog: not initialized!" );
        // this should have been prevented in ::execute already

    OCopyTableWizard* pWizard = new OCopyTableWizard(
        _pParent,
        m_sDestinationTable,
        m_nOperation,
        *m_pSourceObject,
        m_xSourceConnection.getTyped(),
        m_xDestConnection.getTyped(),
        m_xContext,
        m_xInteractionHandler
    );

    impl_attributesToDialog_nothrow( *pWizard );

    return pWizard;
}

void CopyTableWizard::executedDialog( sal_Int16 _nExecutionResult )
{
    CopyTableWizard_DialogBase::executedDialog( _nExecutionResult );

    if ( _nExecutionResult == RET_OK )
        impl_doCopy_nothrow();

    // do this after impl_doCopy_nothrow: The attributes may change during copying, for instance
    // if the user entered an unqualified table name
    impl_dialogToAttributes_nothrow( impl_getDialog_throw() );
}

} // namespace dbaui

extern "C" void SAL_CALL createRegistryInfo_CopyTableWizard()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::CopyTableWizard > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
