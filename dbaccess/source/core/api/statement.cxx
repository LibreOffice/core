/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statement.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:34:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef _DBA_COREAPI_STATEMENT_HXX_
#include <statement.hxx>
#endif
#ifndef _DBA_COREAPI_RESULTSET_HXX_
#include <resultset.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;
using namespace dbtools;

DBG_NAME(OStatementBase)

//--------------------------------------------------------------------------
OStatementBase::OStatementBase(const Reference< XConnection > & _xConn,
                               const Reference< XInterface > & _xStatement)
    :OSubComponent(m_aMutex, _xConn)
    ,OPropertySetHelper(OComponentHelper::rBHelper)
    ,m_bUseBookmarks( sal_False )
    ,m_bEscapeProcessing( sal_True )

{
    DBG_CTOR(OStatementBase, NULL);
    OSL_ENSURE(_xStatement.is() ,"Statement is NULL!");
    m_xAggregateAsSet.set(_xStatement,UNO_QUERY);
    m_xAggregateAsCancellable = Reference< ::com::sun::star::util::XCancellable > (m_xAggregateAsSet, UNO_QUERY);
}

//--------------------------------------------------------------------------
OStatementBase::~OStatementBase()
{
    DBG_DTOR(OStatementBase, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OStatementBase::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XWarningsSupplier > *)0 ),
                           ::getCppuType( (const Reference< XCloseable > *)0 ),
                           ::getCppuType( (const Reference< XMultipleResults > *)0 ),
                           ::getCppuType( (const Reference< XPreparedBatchExecution > *)0 ),
                           ::getCppuType( (const Reference< ::com::sun::star::util::XCancellable > *)0 ),
                            OSubComponent::getTypes() );
    Reference< XGeneratedResultSet > xGRes(m_xAggregateAsSet, UNO_QUERY);
    if ( xGRes.is() )
        aTypes = OTypeCollection(::getCppuType( (const Reference< XGeneratedResultSet > *)0 ),aTypes.getTypes());

    return aTypes.getTypes();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OStatementBase::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OSubComponent::queryInterface( rType );
    if (!aIface.hasValue())
    {
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ),
                    static_cast< XWarningsSupplier * >( this ),
                    static_cast< XCloseable * >( this ),
                    static_cast< XPreparedBatchExecution * >( this ),
                    static_cast< XMultipleResults * >( this ),
                    static_cast< ::com::sun::star::util::XCancellable * >( this ));
        if ( !aIface.hasValue() )
        {
            Reference< XGeneratedResultSet > xGRes(m_xAggregateAsSet, UNO_QUERY);
            if ( ::getCppuType( (const Reference< XGeneratedResultSet > *)0 ) == rType && xGRes.is() )
                aIface = ::cppu::queryInterface(rType,static_cast< XGeneratedResultSet * >( this ));
        }
    }
    return aIface;
}

//--------------------------------------------------------------------------
void OStatementBase::acquire() throw ()
{
    OSubComponent::acquire();
}

//--------------------------------------------------------------------------
void OStatementBase::release() throw ()
{
    OSubComponent::release();
}

//------------------------------------------------------------------------------
void OStatementBase::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_aResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_aResultSet = NULL;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OStatementBase::disposing()
{
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
//------------------------------------------------------------------------------
void OStatementBase::close(void) throw( SQLException, RuntimeException )
{
    {
        MutexGuard aGuard( m_aMutex );
        ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    }
    dispose();
}

// OPropertySetHelper
//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OStatementBase::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatementBase::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(10)
        DECL_PROP0(CURSORNAME,              ::rtl::OUString);
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
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OStatementBase::getInfoHelper()
{
    return *getArrayHelper();
}

//------------------------------------------------------------------------------
sal_Bool OStatementBase::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue ) throw( IllegalArgumentException  )
{
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
                ::rtl::OUString sPropName;
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

//------------------------------------------------------------------------------
void OStatementBase::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
{
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
                ::rtl::OUString sPropName;
                getInfoHelper().fillPropertyMembersByHandle( &sPropName, NULL, nHandle );
                m_xAggregateAsSet->setPropertyValue( sPropName, rValue );
            }
            break;
    }
}

//------------------------------------------------------------------------------
void OStatementBase::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
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
                ::rtl::OUString sPropName;
                const_cast< OStatementBase* >( this )->getInfoHelper().fillPropertyMembersByHandle( &sPropName, NULL, nHandle );
                rValue = m_xAggregateAsSet->getPropertyValue( sPropName );
            }
            break;
    }
}

// XWarningsSupplier
//------------------------------------------------------------------------------
Any OStatementBase::getWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    return Reference< XWarningsSupplier >(m_xAggregateAsSet, UNO_QUERY)->getWarnings();
}

//------------------------------------------------------------------------------
void OStatementBase::clearWarnings(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XWarningsSupplier >(m_xAggregateAsSet, UNO_QUERY)->clearWarnings();
}

// ::com::sun::star::util::XCancellable
//------------------------------------------------------------------------------
void OStatementBase::cancel(void) throw( RuntimeException )
{
    // no blocking as cancel is typically called from a different thread
    ClearableMutexGuard aCancelGuard(m_aCancelMutex);
    if (m_xAggregateAsCancellable.is())
        m_xAggregateAsCancellable->cancel();
    // else do nothing
}

// XMultipleResults
//------------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OStatementBase::getResultSet(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsMultipleResultSets())
        throwFunctionSequenceException(*this);

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getResultSet();
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatementBase::getUpdateCount(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsMultipleResultSets())
        throwFunctionSequenceException(*this);

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getUpdateCount();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OStatementBase::getMoreResults(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsMultipleResultSets())
        throwFunctionSequenceException(*this);
        throwFunctionSequenceException(*this);

    // free the previous results
    disposeResultSet();

    return Reference< XMultipleResults >(m_xAggregateAsSet, UNO_QUERY)->getMoreResults();
}

// XPreparedBatchExecution
//------------------------------------------------------------------------------
void SAL_CALL OStatementBase::addBatch(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);

    Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->addBatch();
}

//------------------------------------------------------------------------------
void SAL_CALL OStatementBase::clearBatch(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    // first check the meta data
    Reference<XDatabaseMetaData> xMeta = Reference< XConnection > (m_xParent, UNO_QUERY)->getMetaData();
    if (!xMeta.is() && !xMeta->supportsBatchUpdates())
        throwFunctionSequenceException(*this);

    Reference< XPreparedBatchExecution >(m_xAggregateAsSet, UNO_QUERY)->clearBatch();
}

//------------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OStatementBase::executeBatch(  ) throw(SQLException, RuntimeException)
{
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
// -----------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OStatementBase::getGeneratedValues(  ) throw (SQLException, RuntimeException)
{
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
//------------------------------------------------------------------------------
OStatement::OStatement( const Reference< XConnection >& _xConn, const Reference< XInterface > & _xStatement )
    :OStatementBase( _xConn, _xStatement )
    ,m_bAttemptedComposerCreation( false )
{
    m_xAggregateStatement.set( _xStatement, UNO_QUERY_THROW );
}

//------------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( OStatement, OStatementBase, OStatement_IFACE );
IMPLEMENT_FORWARD_XTYPEPROVIDER2( OStatement, OStatementBase, OStatement_IFACE );

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OStatement::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OStatement");
}

//------------------------------------------------------------------------------
sal_Bool OStatement::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OStatement::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = SERVICE_SDBC_STATEMENT;
    return aSNS;
}

// XStatement
//------------------------------------------------------------------------------
Reference< XResultSet > OStatement::executeQuery( const rtl::OUString& _rSQL ) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();
    Reference< XResultSet > xResultSet;

    ::rtl::OUString sSQL( impl_doEscapeProcessing_nothrow( _rSQL ) );

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

//------------------------------------------------------------------------------
sal_Int32 OStatement::executeUpdate( const rtl::OUString& _rSQL ) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    ::rtl::OUString sSQL( impl_doEscapeProcessing_nothrow( _rSQL ) );
    return m_xAggregateStatement->executeUpdate( sSQL );
}

//------------------------------------------------------------------------------
sal_Bool OStatement::execute( const rtl::OUString& _rSQL ) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    disposeResultSet();

    ::rtl::OUString sSQL( impl_doEscapeProcessing_nothrow( _rSQL ) );
    return m_xAggregateStatement->execute( sSQL );
}

//------------------------------------------------------------------------------
Reference< XConnection > OStatement::getConnection(void) throw( SQLException, RuntimeException )
{
    return Reference< XConnection >( m_xParent, UNO_QUERY );
}

// -----------------------------------------------------------------------------
void SAL_CALL OStatement::disposing()
{
    OStatementBase::disposing();
    m_xComposer.clear();
    m_xAggregateStatement.clear();
}

// -----------------------------------------------------------------------------
::rtl::OUString OStatement::impl_doEscapeProcessing_nothrow( const ::rtl::OUString& _rSQL ) const
{
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

        ::rtl::OUString sLowLevelSQL = m_xComposer->getQueryWithSubstitution();
        return sLowLevelSQL;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return _rSQL;
}

// -----------------------------------------------------------------------------
bool OStatement::impl_ensureComposer_nothrow() const
{
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
