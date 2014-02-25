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

#include <statement.hxx>
#include <resultset.hxx>
#include "dbastrings.hrc"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <connectivity/dbexception.hxx>

using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;
using namespace dbtools;


OStatementBase::OStatementBase(const Reference< XConnection > & _xConn,
                               const Reference< XInterface > & _xStatement)
    :OSubComponent(m_aMutex, _xConn)
    ,OPropertySetHelper(OComponentHelper::rBHelper)
    ,m_bUseBookmarks( sal_False )
    ,m_bEscapeProcessing( sal_True )

{
    SAL_INFO("dbaccess", "OStatementBase::OStatementBase" );
    OSL_ENSURE(_xStatement.is() ,"Statement is NULL!");
    m_xAggregateAsSet.set(_xStatement,UNO_QUERY);
    m_xAggregateAsCancellable = Reference< ::com::sun::star::util::XCancellable > (m_xAggregateAsSet, UNO_QUERY);
}

OStatementBase::~OStatementBase()
{
}

// com::sun::star::lang::XTypeProvider
Sequence< Type > OStatementBase::getTypes() throw (RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::getTypes" );
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XWarningsSupplier > *)0 ),
                           ::getCppuType( (const Reference< XCloseable > *)0 ),
                           ::getCppuType( (const Reference< XMultipleResults > *)0 ),
                           ::getCppuType( (const Reference< ::com::sun::star::util::XCancellable > *)0 ),
                            OSubComponent::getTypes() );
    Reference< XGeneratedResultSet > xGRes(m_xAggregateAsSet, UNO_QUERY);
    if ( xGRes.is() )
        aTypes = OTypeCollection(::getCppuType( (const Reference< XGeneratedResultSet > *)0 ),aTypes.getTypes());
    Reference< XPreparedBatchExecution > xPreparedBatchExecution(m_xAggregateAsSet, UNO_QUERY);
    if ( xPreparedBatchExecution.is() )
        aTypes = OTypeCollection(::getCppuType( (const Reference< XPreparedBatchExecution > *)0 ),aTypes.getTypes());

    return aTypes.getTypes();
}

// com::sun::star::uno::XInterface
Any OStatementBase::queryInterface( const Type & rType ) throw (RuntimeException, std::exception)
{
    Any aIface = OSubComponent::queryInterface( rType );
    if (!aIface.hasValue())
    {
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ),
                    static_cast< XWarningsSupplier * >( this ),
                    static_cast< XCloseable * >( this ),
                    static_cast< XMultipleResults * >( this ),
                    static_cast< ::com::sun::star::util::XCancellable * >( this ));
        if ( !aIface.hasValue() )
        {
            Reference< XGeneratedResultSet > xGRes(m_xAggregateAsSet, UNO_QUERY);
            if ( ::getCppuType( (const Reference< XGeneratedResultSet > *)0 ) == rType && xGRes.is() )
                aIface = ::cppu::queryInterface(rType,static_cast< XGeneratedResultSet * >( this ));
        }
        if ( !aIface.hasValue() )
        {
            Reference< XPreparedBatchExecution > xGRes(m_xAggregateAsSet, UNO_QUERY);
            if ( ::getCppuType( (const Reference< XPreparedBatchExecution > *)0 ) == rType && xGRes.is() )
                aIface = ::cppu::queryInterface(rType,static_cast< XPreparedBatchExecution * >( this ));
        }
    }
    return aIface;
}

void OStatementBase::acquire() throw ()
{
    OSubComponent::acquire();
}

void OStatementBase::release() throw ()
{
    OSubComponent::release();
}

void OStatementBase::disposeResultSet()
{
    SAL_INFO("dbaccess", "OStatementBase::disposeResultSet" );
    // free the cursor if alive
    Reference< XComponent > xComp(m_aResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_aResultSet = NULL;
}

// OComponentHelper
void OStatementBase::disposing()
{
    SAL_INFO("dbaccess", "OStatementBase::disposing" );
    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    // free pending results
    disposeResultSet();

    // free the original statement
    {
        MutexGuard aCancelGuard(m_aCancelMutex);
        m_xAggregateAsCancellable = NULL;
    }

    if ( m_xAggregateAsSet.is() )
    {
        try
        {
            Reference< XCloseable > (m_xAggregateAsSet, UNO_QUERY)->close();
        }
        catch(RuntimeException& )
        {// don't care for anymore
        }
    }
    m_xAggregateAsSet = NULL;

    // free the parent at last
    OSubComponent::disposing();
}

// XCloseable
void OStatementBase::close(void) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatementBase::close" );
    {
        MutexGuard aGuard( m_aMutex );
        ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    }
    dispose();
}

// OPropertySetHelper
Reference< XPropertySetInfo > OStatementBase::getPropertySetInfo() throw (RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::getPropertySetInfo" );
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* OStatementBase::createArrayHelper( ) const
{
    //SAL_INFO("dbaccess", "OStatementBase::createArrayHelper" );
    BEGIN_PROPERTY_HELPER(10)
        DECL_PROP0(CURSORNAME,              OUString);
        DECL_PROP0_BOOL(ESCAPE_PROCESSING);
        DECL_PROP0(FETCHDIRECTION,          sal_Int32);
        DECL_PROP0(FETCHSIZE,               sal_Int32);
        DECL_PROP0(MAXFIELDSIZE,            sal_Int32);
        DECL_PROP0(MAXROWS,                 sal_Int32);
        DECL_PROP0(QUERYTIMEOUT,            sal_Int32);
        DECL_PROP0(RESULTSETCONCURRENCY,    sal_Int32);
        DECL_PROP0(RESULTSETTYPE,           sal_Int32);
        DECL_PROP0_BOOL(USEBOOKMARKS);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& OStatementBase::getInfoHelper()
{
    //SAL_INFO("dbaccess", "OStatementBase::getInfoHelper" );
    return *getArrayHelper();
}

sal_Bool OStatementBase::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue ) throw( IllegalArgumentException  )
{
    //SAL_INFO("dbaccess", "OStatementBase::convertFastPropertyValue" );
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_USEBOOKMARKS:
            bModified = ::comphelper::tryPropertyValue( rConvertedValue, rOldValue, rValue, m_bUseBookmarks );
            break;

        case PROPERTY_ID_ESCAPE_PROCESSING:
            bModified = ::comphelper::tryPropertyValue( rConvertedValue, rOldValue, rValue, m_bEscapeProcessing );
            break;

        default:
            if ( m_xAggregateAsSet.is() )
            {
                // get the property name
                OUString sPropName;
                getInfoHelper().fillPropertyMembersByHandle( &sPropName, NULL, nHandle );

                // now set the value
                Any aCurrentValue = m_xAggregateAsSet->getPropertyValue( sPropName );
                if ( aCurrentValue != rValue )
                {
                    rOldValue = aCurrentValue;
                    rConvertedValue = rValue;
                    bModified = sal_True;
                }
            }
            break;
    }
    return bModified;
}

void OStatementBase::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception, std::exception)
{
    //SAL_INFO("dbaccess", "OStatementBase::setFastPropertyValue_NoBroadcast" );
    switch ( nHandle )
    {
        case PROPERTY_ID_USEBOOKMARKS:
        {
            m_bUseBookmarks = ::comphelper::getBOOL( rValue );
            if ( m_xAggregateAsSet.is() && m_xAggregateAsSet->getPropertySetInfo()->hasPropertyByName( PROPERTY_USEBOOKMARKS ) )
                m_xAggregateAsSet->setPropertyValue( PROPERTY_USEBOOKMARKS, rValue );
        }
        break;

        case PROPERTY_ID_ESCAPE_PROCESSING:
            m_bEscapeProcessing = ::comphelper::getBOOL( rValue );
            if ( m_xAggregateAsSet.is() )
                m_xAggregateAsSet->setPropertyValue( PROPERTY_ESCAPE_PROCESSING, rValue );
            break;

        default:
            if ( m_xAggregateAsSet.is() )
            {
                OUString sPropName;
                getInfoHelper().fillPropertyMembersByHandle( &sPropName, NULL, nHandle );
                m_xAggregateAsSet->setPropertyValue( sPropName, rValue );
            }
            break;
    }
}

void OStatementBase::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    //SAL_INFO("dbaccess", "OStatementBase::getFastPropertyValue" );
    switch (nHandle)
    {
        case PROPERTY_ID_USEBOOKMARKS:
            rValue <<= m_bUseBookmarks;
            break;

        case PROPERTY_ID_ESCAPE_PROCESSING:
            // don't rely on our aggregate - if it implements this wrong, and always returns
            // TRUE here, then we would loop in impl_doEscapeProcessing_nothrow
            rValue <<= m_bEscapeProcessing;
            break;

        default:
            if ( m_xAggregateAsSet.is() )
            {
                OUString sPropName;
                const_cast< OStatementBase* >( this )->getInfoHelper().fillPropertyMembersByHandle( &sPropName, NULL, nHandle );
                rValue = m_xAggregateAsSet->getPropertyValue( sPropName );
            }
            break;
    }
}

// XWarningsSupplier
Any OStatementBase::getWarnings(void) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatementBase::getWarnings" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XWarningsSupplier >(m_xAggregateAsSet, UNO_QUERY)->getWarnings();
}

void OStatementBase::clearWarnings(void) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatementBase::clearWarnings" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XWarningsSupplier >(m_xAggregateAsSet, UNO_QUERY)->clearWarnings();
}

// ::com::sun::star::util::XCancellable
void OStatementBase::cancel(void) throw( RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatementBase::cancel" );
    // no blocking as cancel is typically called from a different thread
    ClearableMutexGuard aCancelGuard(m_aCancelMutex);
    if (m_xAggregateAsCancellable.is())
        m_xAggregateAsCancellable->cancel();
    // else do nothing
}

// XMultipleResults
Reference< XResultSet > SAL_CALL OStatementBase::getResultSet(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::getResultSet" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsMultipleResultSets())
        throwFunctionSequenceException(*this);

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getResultSet();
}

sal_Int32 SAL_CALL OStatementBase::getUpdateCount(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::getUpdateCount" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsMultipleResultSets())
        throwFunctionSequenceException(*this);

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getUpdateCount();
}

sal_Bool SAL_CALL OStatementBase::getMoreResults(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::getMoreResults" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsMultipleResultSets())
        throwFunctionSequenceException(*this);

    // free the previous results
    disposeResultSet();

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getMoreResults();
}

// XPreparedBatchExecution
void SAL_CALL OStatementBase::addBatch(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::addBatch" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);

    Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->addBatch();
}

void SAL_CALL OStatementBase::clearBatch(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::clearBatch" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);

    Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->clearBatch();
}

Sequence< sal_Int32 > SAL_CALL OStatementBase::executeBatch(  ) throw(SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::executeBatch" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);

    // free the previous results
    disposeResultSet();

    return Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->executeBatch();
}

Reference< XResultSet > SAL_CALL OStatementBase::getGeneratedValues(  ) throw (SQLException, RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatementBase::getGeneratedValues" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    Reference< XGeneratedResultSet > xGRes(m_xAggregateAsSet, UNO_QUERY);

    if ( xGRes.is() )
        return xGRes->getGeneratedValues(  );
    return Reference< XResultSet >();
}

//************************************************************
//  OStatement
//************************************************************
OStatement::OStatement( const Reference< XConnection >& _xConn, const Reference< XInterface > & _xStatement )
    :OStatementBase( _xConn, _xStatement )
    ,m_bAttemptedComposerCreation( false )
{
    SAL_INFO("dbaccess", "OStatement::OStatement" );
    m_xAggregateStatement.set( _xStatement, UNO_QUERY_THROW );
}

IMPLEMENT_FORWARD_XINTERFACE2( OStatement, OStatementBase, OStatement_IFACE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( OStatement, OStatementBase, OStatement_IFACE );

// XServiceInfo
OUString OStatement::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatement::getImplementationName" );
    return OUString("com.sun.star.sdb.OStatement");
}

sal_Bool OStatement::supportsService( const OUString& _rServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > OStatement::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    SAL_INFO("dbaccess", "OStatement::getSupportedServiceNames" );
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = SERVICE_SDBC_STATEMENT;
    return aSNS;
}

// XStatement
Reference< XResultSet > OStatement::executeQuery( const OUString& _rSQL ) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatement::executeQuery" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();
    Reference< XResultSet > xResultSet;

    OUString sSQL( impl_doEscapeProcessing_nothrow( _rSQL ) );

    Reference< XResultSet > xInnerResultSet = m_xAggregateStatement->executeQuery( sSQL );
    Reference< XConnection > xConnection( m_xParent, UNO_QUERY_THROW );

    if ( xInnerResultSet.is() )
    {
        Reference< XDatabaseMetaData > xMeta = xConnection->getMetaData();
        sal_Bool bCaseSensitive = xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers();
        xResultSet = new OResultSet( xInnerResultSet, *this, bCaseSensitive );

        // keep the resultset weak
        m_aResultSet = xResultSet;
    }

    return xResultSet;
}

sal_Int32 OStatement::executeUpdate( const OUString& _rSQL ) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatement::executeUpdate" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    OUString sSQL( impl_doEscapeProcessing_nothrow( _rSQL ) );
    return m_xAggregateStatement->executeUpdate( sSQL );
}

sal_Bool OStatement::execute( const OUString& _rSQL ) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatement::execute" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    OUString sSQL( impl_doEscapeProcessing_nothrow( _rSQL ) );
    return m_xAggregateStatement->execute( sSQL );
}

void OStatement::addBatch( const OUString& _rSQL ) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatement::execute" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);

    OUString sSQL( impl_doEscapeProcessing_nothrow( _rSQL ) );
    Reference< XBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->addBatch( sSQL );
}

void OStatement::clearBatch( ) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatement::execute" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);

    Reference< XBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->clearBatch();
}

Sequence< sal_Int32 > OStatement::executeBatch( ) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatement::execute" );
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);
    return Reference< XBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->executeBatch( );
}


Reference< XConnection > OStatement::getConnection(void) throw( SQLException, RuntimeException, std::exception )
{
    SAL_INFO("dbaccess", "OStatement::getConnection" );
    return Reference< XConnection >( m_xParent, UNO_QUERY );
}

void SAL_CALL OStatement::disposing()
{
    SAL_INFO("dbaccess", "OStatement::disposing" );
    OStatementBase::disposing();
    m_xComposer.clear();
    m_xAggregateStatement.clear();
}

OUString OStatement::impl_doEscapeProcessing_nothrow( const OUString& _rSQL ) const
{
    SAL_INFO("dbaccess", "OStatement::impl_doEscapeProcessing_nothrow" );
    if ( !m_bEscapeProcessing )
        return _rSQL;
    try
    {
        if ( !impl_ensureComposer_nothrow() )
            return _rSQL;

        bool bParseable = false;
        try { m_xComposer->setQuery( _rSQL ); bParseable = true; }
        catch( const SQLException& ) { }

        if ( !bParseable )
            // if we cannot parse it, silently accept this. The driver is probably able to cope with it then
            return _rSQL;

        OUString sLowLevelSQL = m_xComposer->getQueryWithSubstitution();
        return sLowLevelSQL;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return _rSQL;
}

bool OStatement::impl_ensureComposer_nothrow() const
{
    SAL_INFO("dbaccess", "OStatement::impl_ensureComposer_nothrow" );
    if ( m_bAttemptedComposerCreation )
        return m_xComposer.is();

    const_cast< OStatement* >( this )->m_bAttemptedComposerCreation = true;
    try
    {
        Reference< XMultiServiceFactory > xFactory( m_xParent, UNO_QUERY_THROW );
        const_cast< OStatement* >( this )->m_xComposer.set( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return m_xComposer.is();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
