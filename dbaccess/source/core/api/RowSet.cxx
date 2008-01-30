/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RowSet.cxx,v $
 *
 *  $Revision: 1.155 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:29:04 $
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

#include "RowSet.hxx"
#include "dbastrings.hrc"
#include "sdbcoretools.hxx"
#include "SingleSelectQueryComposer.hxx"
#include "module_dba.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWSETVETOEXCEPTION_HPP_
#include <com/sun/star/sdb/RowSetVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWCHANGEACTION_HPP_
#include <com/sun/star/sdb/RowChangeAction.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ERRORCONDITION_HPP_
#include <com/sun/star/sdb/ErrorCondition.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBACCESS_CORE_API_ROWSETCACHE_HXX
#include "RowSetCache.hxx"
#endif
#if OSL_DEBUG_LEVEL > 1
#ifndef _COM_SUN_STAR_SDBC_XDRIVERMANAGER_HPP_
#include <com/sun/star/sdbc/XDriverManager.hpp>
#endif
#endif
#ifndef _DBACORE_DATACOLUMN_HXX_
#include "CRowSetDataColumn.hxx"
#endif
#ifndef DBACCESS_CORE_API_CROWSETCOLUMN_HXX
#include "CRowSetColumn.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBA_CORE_TABLECONTAINER_HXX_
#include "tablecontainer.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XPARAMETERSSUPPLIER_HPP_
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

using namespace utl;
using namespace dbaccess;
using namespace connectivity;
using namespace comphelper;
using namespace dbtools;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::util;
using namespace ::cppu;
using namespace ::osl;

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ORowSet()
{
    static ::dba::OAutoRegistration< ORowSet > aAutoRegistration;
}
// -----------------------------------------------------------------------------

#define NOTIFY_LISTERNERS_CHECK(_rListeners,T,method)                             \
    Sequence< Reference< XInterface > > aListenerSeq = _rListeners.getElements(); \
                                                                                  \
    const Reference< XInterface >* pxIntBegin = aListenerSeq.getConstArray();     \
    const Reference< XInterface >* pxInt = pxIntBegin + aListenerSeq.getLength(); \
                                                                                  \
    _rGuard.clear();                                                              \
    sal_Bool bCheck = sal_True;                                                   \
    while( pxInt > pxIntBegin && bCheck )                                         \
    {                                                                             \
        try                                                                       \
        {                                                                         \
            while( pxInt > pxIntBegin && bCheck )                                 \
            {                                                                     \
                --pxInt;                                                          \
                bCheck = static_cast< T* >( pxInt->get() )->method(aEvt);         \
            }                                                                     \
        }                                                                         \
        catch( RuntimeException& )                                                \
        {                                                                         \
        }                                                                         \
    }                                                                             \
    _rGuard.reset();


//..................................................................
namespace dbaccess
{
//..................................................................


//--------------------------------------------------------------------------
Reference< XInterface > ORowSet_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ORowSet(_rxFactory));
}
//--------------------------------------------------------------------------
ORowSet::ORowSet( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB )
    :ORowSet_BASE1(m_aMutex)
    ,ORowSetBase( _rxORB, ORowSet_BASE1::rBHelper, &m_aMutex )
    ,m_pParameters( NULL )
    ,m_aRowsetListeners(*m_pMutex)
    ,m_aApproveListeners(*m_pMutex)
    ,m_pTables(NULL)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nFetchSize(50)
    ,m_nMaxFieldSize(0)
    ,m_nMaxRows(0)
    ,m_nQueryTimeOut(0)
    ,m_nCommandType(CommandType::COMMAND)
    ,m_nTransactionIsolation(0)
    ,m_nPrivileges(0)
    ,m_nInAppend(0)
    ,m_bUseEscapeProcessing(sal_True)
    ,m_bApplyFilter(sal_False)
    ,m_bCommandFacetsDirty( sal_True )
    ,m_bModified(sal_False)
    ,m_bRebuildConnOnExecute(sal_False)
    ,m_bIsBookmarable(sal_True)
    ,m_bNew(sal_False)
    ,m_bCanUpdateInsertedRows(sal_True)
    ,m_bOwnConnection(sal_False)
{
    m_nResultSetType = ResultSetType::SCROLL_SENSITIVE;
    m_nResultSetConcurrency = ResultSetConcurrency::UPDATABLE;
    m_pMySelf = this;
    m_aActiveConnection <<= m_xActiveConnection;

    sal_Int32 nRBT  = PropertyAttribute::READONLY   | PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;
    sal_Int32 nRT   = PropertyAttribute::READONLY   | PropertyAttribute::TRANSIENT;
    sal_Int32 nBT   = PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;

    m_aPrematureParamValues.resize( 0 );

    // sdb.RowSet Properties
    registerMayBeVoidProperty(PROPERTY_ACTIVE_CONNECTION,PROPERTY_ID_ACTIVE_CONNECTION, PropertyAttribute::MAYBEVOID|PropertyAttribute::TRANSIENT|PropertyAttribute::BOUND, &m_aActiveConnection,   ::getCppuType(reinterpret_cast< Reference< XConnection >* >(NULL)));
    registerProperty(PROPERTY_DATASOURCENAME,       PROPERTY_ID_DATASOURCENAME,         PropertyAttribute::BOUND,       &m_aDataSourceName,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_COMMAND,              PROPERTY_ID_COMMAND,                PropertyAttribute::BOUND,       &m_aCommand,            ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_COMMAND_TYPE,         PROPERTY_ID_COMMAND_TYPE,           PropertyAttribute::BOUND,       &m_nCommandType,        ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_ACTIVECOMMAND,        PROPERTY_ID_ACTIVECOMMAND,          nRBT,                           &m_aActiveCommand,      ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_IGNORERESULT,         PROPERTY_ID_IGNORERESULT,           PropertyAttribute::BOUND,       &m_bIgnoreResult,       ::getBooleanCppuType());
    registerProperty(PROPERTY_FILTER,               PROPERTY_ID_FILTER,                 PropertyAttribute::BOUND,       &m_aFilter,             ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_HAVING_CLAUSE,        PROPERTY_ID_HAVING_CLAUSE,          PropertyAttribute::BOUND,       &m_aHavingClause,       ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_GROUP_BY,             PROPERTY_ID_GROUP_BY,               PropertyAttribute::BOUND,       &m_aGroupBy,            ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_APPLYFILTER,          PROPERTY_ID_APPLYFILTER,            PropertyAttribute::BOUND,       &m_bApplyFilter,        ::getBooleanCppuType());
    registerProperty(PROPERTY_ORDER,                PROPERTY_ID_ORDER,                  PropertyAttribute::BOUND,       &m_aOrder,              ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_PRIVILEGES,           PROPERTY_ID_PRIVILEGES,             nRT,                            &m_nPrivileges,         ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_ISMODIFIED,           PROPERTY_ID_ISMODIFIED,             nBT,                            &m_bModified,           ::getBooleanCppuType());
    registerProperty(PROPERTY_ISNEW,                PROPERTY_ID_ISNEW,                  nRBT,                           &m_bNew,                ::getBooleanCppuType());

    // sdbcx.ResultSet Properties
    registerProperty(PROPERTY_ISBOOKMARKABLE,       PROPERTY_ID_ISBOOKMARKABLE,         nRT,                            &m_bIsBookmarable,      ::getBooleanCppuType());
    registerProperty(PROPERTY_CANUPDATEINSERTEDROWS,PROPERTY_ID_CANUPDATEINSERTEDROWS,  nRT,                            &m_bCanUpdateInsertedRows,      ::getBooleanCppuType());
    // sdbc.ResultSet Properties
    registerProperty(PROPERTY_RESULTSETCONCURRENCY, PROPERTY_ID_RESULTSETCONCURRENCY,   PropertyAttribute::TRANSIENT,   &m_nResultSetConcurrency,::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_RESULTSETTYPE,        PROPERTY_ID_RESULTSETTYPE,          PropertyAttribute::TRANSIENT,   &m_nResultSetType,      ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_FETCHDIRECTION,       PROPERTY_ID_FETCHDIRECTION,         PropertyAttribute::TRANSIENT,   &m_nFetchDirection,     ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_FETCHSIZE,            PROPERTY_ID_FETCHSIZE,              PropertyAttribute::TRANSIENT,   &m_nFetchSize,          ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));

    // sdbc.RowSet Properties
    registerProperty(PROPERTY_URL,                  PROPERTY_ID_URL,                    0,                              &m_aURL,                ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_TRANSACTIONISOLATION, PROPERTY_ID_TRANSACTIONISOLATION,   PropertyAttribute::TRANSIENT,   &m_nTransactionIsolation,::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerMayBeVoidProperty(PROPERTY_TYPEMAP,     PROPERTY_ID_TYPEMAP,                PropertyAttribute::MAYBEVOID|PropertyAttribute::TRANSIENT,  &m_aTypeMap,            ::getCppuType(reinterpret_cast< Reference< XNameAccess >* >(NULL)));
    registerProperty(PROPERTY_ESCAPE_PROCESSING,PROPERTY_ID_ESCAPE_PROCESSING,  PropertyAttribute::BOUND,       &m_bUseEscapeProcessing,::getBooleanCppuType()  );
    registerProperty(PROPERTY_QUERYTIMEOUT,         PROPERTY_ID_QUERYTIMEOUT,           PropertyAttribute::TRANSIENT,   &m_nQueryTimeOut,       ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_MAXFIELDSIZE,         PROPERTY_ID_MAXFIELDSIZE,           PropertyAttribute::TRANSIENT,   &m_nMaxFieldSize,       ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_MAXROWS,              PROPERTY_ID_MAXROWS,                0,                              &m_nMaxRows,            ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)) );
    registerProperty(PROPERTY_USER,                 PROPERTY_ID_USER,                   PropertyAttribute::TRANSIENT,   &m_aUser,               ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_PASSWORD,             PROPERTY_ID_PASSWORD,               PropertyAttribute::TRANSIENT,   &m_aPassword,           ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));

    registerProperty(PROPERTY_UPDATE_CATALOGNAME,   PROPERTY_ID_UPDATE_CATALOGNAME,     PropertyAttribute::BOUND,       &m_aUpdateCatalogName,  ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_UPDATE_SCHEMANAME,    PROPERTY_ID_UPDATE_SCHEMANAME,      PropertyAttribute::BOUND,       &m_aUpdateSchemaName,   ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_UPDATE_TABLENAME,     PROPERTY_ID_UPDATE_TABLENAME,       PropertyAttribute::BOUND,       &m_aUpdateTableName,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}

ORowSet::~ORowSet()
{
    if ( !m_rBHelper.bDisposed && !m_rBHelper.bInDispose )
    {
        OSL_ENSURE(0, "Please check who doesn't dispose this component!");
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

// -----------------------------------------------------------------------------
void ORowSet::getPropertyDefaultByHandle( sal_Int32 _nHandle, Any& _rDefault ) const
{
    switch( _nHandle )
    {
        case PROPERTY_ID_COMMAND_TYPE:
            _rDefault <<= static_cast<sal_Int32>(CommandType::COMMAND);
            break;
        case PROPERTY_ID_IGNORERESULT:
            _rDefault <<= sal_False;
            break;
        case PROPERTY_ID_APPLYFILTER:
            _rDefault <<= sal_False;
            break;
        case PROPERTY_ID_ISMODIFIED:
            _rDefault <<= sal_False;
            break;
        case PROPERTY_ID_ISBOOKMARKABLE:
            _rDefault <<= sal_True;
            break;
        case PROPERTY_ID_CANUPDATEINSERTEDROWS:
            _rDefault <<= sal_True;
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            _rDefault <<= ResultSetType::SCROLL_INSENSITIVE;
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            _rDefault <<= ResultSetConcurrency::UPDATABLE;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            _rDefault <<= FetchDirection::FORWARD;
            break;
        case PROPERTY_ID_FETCHSIZE:
            _rDefault <<= static_cast<sal_Int32>(1);
            break;
        case PROPERTY_ID_ESCAPE_PROCESSING:
            _rDefault <<= sal_True;
            break;
    }
}
// -------------------------------------------------------------------------
//  typedef ::comphelper::OPropertyArrayUsageHelper<ORowSet> ORowSet_Prop;

void SAL_CALL ORowSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISMODIFIED:
            m_bModified = cppu::any2bool(rValue);
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            if( m_nResultSetType == ResultSetType::FORWARD_ONLY)
                throw Exception(); // else run through
        default:
            OPropertyStateContainer::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }

    if  (   ( nHandle == PROPERTY_ID_ACTIVE_CONNECTION )
        ||  ( nHandle == PROPERTY_ID_DATASOURCENAME )
        ||  ( nHandle == PROPERTY_ID_COMMAND )
        ||  ( nHandle == PROPERTY_ID_COMMAND_TYPE )
        ||  ( nHandle == PROPERTY_ID_IGNORERESULT )
        ||  ( nHandle == PROPERTY_ID_FILTER )
        ||  ( nHandle == PROPERTY_ID_HAVING_CLAUSE )
        ||  ( nHandle == PROPERTY_ID_GROUP_BY )
        ||  ( nHandle == PROPERTY_ID_APPLYFILTER )
        ||  ( nHandle == PROPERTY_ID_ORDER )
        ||  ( nHandle == PROPERTY_ID_URL )
        ||  ( nHandle == PROPERTY_ID_USER )
        )
    {
        m_bCommandFacetsDirty = sal_True;
    }


    switch(nHandle)
    {
        case PROPERTY_ID_ACTIVE_CONNECTION:
            // the new connection
            {
                Reference< XConnection > xNewConnection(m_aActiveConnection,UNO_QUERY);
                setActiveConnection(xNewConnection, sal_False);
            }

            m_bOwnConnection        = sal_False;
            m_bRebuildConnOnExecute = sal_False;
            break;

        case PROPERTY_ID_DATASOURCENAME:
            if(!m_xStatement.is())
            {
                Reference< XConnection >  xNewConn;
                Any aNewConn;
                aNewConn <<= xNewConn;
                setFastPropertyValue(PROPERTY_ID_ACTIVE_CONNECTION, aNewConn);
            }
            else
                m_bRebuildConnOnExecute = sal_True;
            break;
        case PROPERTY_ID_FETCHSIZE:
            if(m_pCache)
            {
                m_pCache->setMaxRowSize(m_nFetchSize);
                fireRowcount();
            }
            break;
        case PROPERTY_ID_URL:
            // is the connection-to-be-built determined by the url (which is the case if m_aDataSourceName is empty) ?
            if (!m_aDataSourceName.getLength())
            {
                // are we active at the moment ?
                if (m_xStatement.is())
                    // yes -> the next execute needs to rebuild our connection because of this new property
                    m_bRebuildConnOnExecute = sal_True;
                else
                {   // no -> drop our active connection (if we have one) as it doesn't correspond to this new property value anymore
                    Reference< XConnection >  xNewConn;
                    Any aNewConn;
                    aNewConn <<= xNewConn;
                    setFastPropertyValue(PROPERTY_ID_ACTIVE_CONNECTION, aNewConn);
                }
            }
            m_bOwnConnection = sal_True;
            break;
        case PROPERTY_ID_TYPEMAP:
            ::cppu::extractInterface(m_xTypeMap,m_aTypeMap);
            break;
        default:
            break;
    };
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_pCache)
    {
        switch(nHandle)
        {
        case PROPERTY_ID_ISMODIFIED:
            rValue.setValue(&m_bModified,::getCppuBooleanType());
            break;
        case PROPERTY_ID_ISNEW:
            rValue.setValue(&m_bNew,::getCppuBooleanType());
            break;
        case PROPERTY_ID_PRIVILEGES:
            rValue <<= m_pCache->m_nPrivileges;
            break;
        case PROPERTY_ID_ACTIVE_CONNECTION:
            rValue <<= m_xActiveConnection;
            break;
        case PROPERTY_ID_TYPEMAP:
            rValue <<= m_xTypeMap;
            break;
        default:
            ORowSetBase::getFastPropertyValue(rValue,nHandle);
        };
    }
    else
    {
        switch(nHandle)
        {
            case PROPERTY_ID_ACTIVE_CONNECTION:
                rValue <<= m_xActiveConnection;
                break;
            case PROPERTY_ID_TYPEMAP:
                rValue <<= m_xTypeMap;
                break;
            default:
                ORowSetBase::getFastPropertyValue(rValue,nHandle);
        }
    }
}
// -------------------------------------------------------------------------
// com::sun::star::XTypeProvider
Sequence< Type > SAL_CALL ORowSet::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                            ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                            ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                           ::comphelper::concatSequences(ORowSet_BASE1::getTypes(),ORowSetBase::getTypes()));
    return aTypes.getTypes();
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL ORowSet::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -------------------------------------------------------------------------

// com::sun::star::XInterface
Any SAL_CALL ORowSet::queryInterface( const Type & rType ) throw (RuntimeException)
{
    return ORowSet_BASE1::queryInterface( rType);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::acquire() throw()
{
    ORowSet_BASE1::acquire();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::release() throw()
{
    ORowSet_BASE1::release();
}
// -------------------------------------------------------------------------

// com::sun::star::XUnoTunnel
sal_Int64 SAL_CALL ORowSet::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}
// -------------------------------------------------------------------------
// com::sun::star::XAggregation
Any SAL_CALL ORowSet::queryAggregation( const Type& rType ) throw(RuntimeException)
{
    Any aRet(ORowSetBase::queryInterface(rType));
    if (!aRet.hasValue())
        aRet = ORowSet_BASE1::queryAggregation(rType);
    return aRet;
}
//------------------------------------------------------------------------------
rtl::OUString ORowSet::getImplementationName_static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.dba.ORowSet");
}
// -------------------------------------------------------------------------
// ::com::sun::star::XServiceInfo
::rtl::OUString SAL_CALL ORowSet::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_static();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSet::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ORowSet::getSupportedServiceNames_static(  ) throw (RuntimeException)
{
    Sequence< rtl::OUString > aSNS( 5 );
    aSNS[0] = SERVICE_SDBC_RESULTSET;
    aSNS[1] = SERVICE_SDBC_ROWSET;
    aSNS[2] = SERVICE_SDBCX_RESULTSET;
    aSNS[3] = SERVICE_SDB_RESULTSET;
    aSNS[4] = SERVICE_SDB_ROWSET;
    return aSNS;
}
// -------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ORowSet::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_static();
}
//------------------------------------------------------------------------------
Reference< XInterface > ORowSet::Create(const Reference< XComponentContext >& _rxContext)
{
    ::comphelper::ComponentContext aContext( _rxContext );
    return ORowSet_CreateInstance( aContext.getLegacyServiceFactory() );
}
// -------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL ORowSet::disposing()
{
    OPropertyStateContainer::disposing();

    MutexGuard aGuard(m_aMutex);
    EventObject aDisposeEvent;
    aDisposeEvent.Source = static_cast< XComponent* >(this);
    m_aRowsetListeners.disposeAndClear( aDisposeEvent );
    m_aApproveListeners.disposeAndClear( aDisposeEvent );

    freeResources( true );

    // remove myself as dispose listener
    Reference< XComponent >  xComponent(m_xActiveConnection, UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xEvt;
        query_aggregation(this,xEvt);
        xComponent->removeEventListener(xEvt);
    }

    m_aActiveConnection = Any(); // the any conatains a reference too
    if(m_bOwnConnection)
        ::comphelper::disposeComponent(m_xActiveConnection);
    m_xActiveConnection = NULL;


    ORowSetBase::disposing();
}
// -------------------------------------------------------------------------
void ORowSet::freeResources( bool _bComplete )
{
    MutexGuard aGuard(m_aMutex);

    // free all clones
    for (connectivity::OWeakRefArray::iterator i = m_aClones.begin(); m_aClones.end() != i; i++)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aClones.clear();

    if ( _bComplete )
    {
        // the columns must be disposed before the querycomposer is disposed because
        // their owner can be the composer
        TDataColumns().swap(m_aDataColumns);// clear and resize capacity
        m_xColumns      = NULL;
        if ( m_pColumns )
            m_pColumns->disposing();
        // dispose the composer to avoid that everbody knows that the querycomposer is eol
        try { ::comphelper::disposeComponent( m_xComposer ); }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
            m_xComposer = NULL;
        }

        DELETEZ(m_pCache);

        impl_resetTables_nothrow();

        m_xStatement    = NULL;
        m_xTypeMap      = NULL;

        m_aBookmark     = Any();
        m_bBeforeFirst  = sal_True;
        m_bAfterLast    = sal_False;
        m_bNew          = sal_False;
        m_bModified     = sal_False;
        m_bLastKnownRowCountFinal = sal_False;
        m_nLastKnownRowCount      = 0;
        if ( m_aOldRow.isValid() )
            m_aOldRow->clearRow();

        impl_disposeParametersContainer_nothrow();

        m_bCommandFacetsDirty = sal_True;
    }
}

// -------------------------------------------------------------------------
void ORowSet::setActiveConnection( Reference< XConnection >& _rxNewConn, sal_Bool _bFireEvent )
{
    if (_rxNewConn.get() == m_xActiveConnection.get())
        // nothing to do
        return;

    // remove the event listener for the old connection
    Reference< XComponent >  xComponent(m_xActiveConnection, UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xListener;
        query_aggregation(this, xListener);
        xComponent->removeEventListener(xListener);
    }

    // if we owned the connection, remember it for later disposing
    if(m_bOwnConnection)
        m_xOldConnection = m_xActiveConnection;

    // for firing the PropertyChangeEvent
    sal_Int32 nHandle = PROPERTY_ID_ACTIVE_CONNECTION;
    Any aOldConnection; aOldConnection <<= m_xActiveConnection;
    Any aNewConnection; aNewConnection <<= _rxNewConn;

    // set the new connection
    m_xActiveConnection = _rxNewConn;
    if (m_xActiveConnection.is())
        m_aActiveConnection <<= m_xActiveConnection;
    else
        m_aActiveConnection.clear();

    // fire the event
    if (_bFireEvent)
        fire(&nHandle, &aNewConnection, &aOldConnection, 1, sal_False);

    // register as event listener for the new connection
    xComponent.set(m_xActiveConnection,UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xListener;
        query_aggregation(this, xListener);
        xComponent->addEventListener(xListener);
    }
}

// -------------------------------------------------------------------------
// ::com::sun::star::XEventListener
void SAL_CALL ORowSet::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException)
{
    // close rowset because the connection is going to be deleted (someone told me :-)
    Reference<XConnection> xCon(Source.Source,UNO_QUERY);
    if(m_xActiveConnection == xCon)
    {
        close();
        {
            MutexGuard aGuard( m_aMutex );
            Reference< XConnection > xXConnection;
            setActiveConnection( xXConnection );
        }
    }
}
// -------------------------------------------------------------------------

// XCloseable
void SAL_CALL ORowSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        MutexGuard aGuard( m_aMutex );
        ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    }
    // additionals things to set
    freeResources( true );
}
// -------------------------------------------------------------------------
// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL ORowSet::getInfoHelper()
{
    typedef ::comphelper::OPropertyArrayUsageHelper<ORowSet> ORowSet_PROP;
    return *ORowSet_PROP::getArrayHelper();
}
// -----------------------------------------------------------------------------
void ORowSet::updateValue(sal_Int32 columnIndex,const ORowSetValue& x)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);
    checkUpdateIterator();

    ::connectivity::ORowSetValue aOldValue((*(*m_aCurrentRow))[columnIndex]);
    m_pCache->updateValue(columnIndex,x);
    // we have to notify all listeners
    (*(*m_aCurrentRow))[columnIndex] = x;
    firePropertyChange(columnIndex-1 ,aOldValue);
    fireProperty(PROPERTY_ID_ISMODIFIED,sal_True,sal_False);
}
// -------------------------------------------------------------------------
// XRowUpdate
void SAL_CALL ORowSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,ORowSetValue());
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );

    checkUpdateConditions(columnIndex);

    checkUpdateIterator();
    ::connectivity::ORowSetValue aOldValue;
    if((*(*m_aCurrentRow))[columnIndex].getTypeKind() == DataType::BLOB)
    {
        m_pCache->updateBinaryStream(columnIndex,x,length);
        aOldValue = (*(*m_aCurrentRow))[columnIndex];
        (*(*m_aCurrentRow))[columnIndex] = makeAny(x);
    }
    else
    {
        Sequence<sal_Int8> aSeq;
        if(x.is())
            x->readSomeBytes(aSeq,length);
        updateValue(columnIndex,aSeq);
        aOldValue = (*(*m_aCurrentRow))[columnIndex];
        (*(*m_aCurrentRow))[columnIndex] = aSeq;
    }

    firePropertyChange(columnIndex-1 ,aOldValue);
    fireProperty(PROPERTY_ID_ISMODIFIED,sal_True,sal_False);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);

    checkUpdateIterator();
    m_pCache->updateCharacterStream(columnIndex,x,length);

    ::connectivity::ORowSetValue aOldValue((*(*m_aCurrentRow))[columnIndex]);
    (*(*m_aCurrentRow))[columnIndex] = makeAny(x);
    firePropertyChange(columnIndex-1 ,aOldValue);
    fireProperty(PROPERTY_ID_ISMODIFIED,sal_True,sal_False);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);

    checkUpdateIterator();

    Any aNewValue = x;

    if ( m_pColumns )
    {
        Reference<XPropertySet> xColumn(m_pColumns->getByIndex(columnIndex-1),UNO_QUERY);
        sal_Int32 nColType = 0;
        xColumn->getPropertyValue(PROPERTY_TYPE) >>= nColType;
        switch( nColType )
        {
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            {
                double nValue = 0;
                if ( x >>= nValue )
                {
                    if ( DataType::TIMESTAMP == nColType )
                        aNewValue <<= dbtools::DBTypeConversion::toDateTime( nValue );
                    else if ( DataType::DATE == nColType )
                        aNewValue <<= dbtools::DBTypeConversion::toDate( nValue );
                    else
                        aNewValue <<= dbtools::DBTypeConversion::toTime( nValue );
                }
                break;
            }
        }
    }

    if (!::dbtools::implUpdateObject(this, columnIndex, aNewValue))
    {   // there is no other updateXXX call which can handle the value in x
        ::connectivity::ORowSetValue aOldValue((*(*m_aCurrentRow))[columnIndex]);
        m_pCache->updateObject(columnIndex,aNewValue);
        // we have to notify all listeners
        (*(*m_aCurrentRow))[columnIndex] = aNewValue;
        firePropertyChange(columnIndex-1 ,aOldValue);
        fireProperty(PROPERTY_ID_ISMODIFIED,sal_True,sal_False);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    checkUpdateConditions(columnIndex);

    checkUpdateIterator();
    ::connectivity::ORowSetValue aOldValue((*(*m_aCurrentRow))[columnIndex]);
    m_pCache->updateNumericObject(columnIndex,x,scale);
    // we have to notify all listeners
    (*(*m_aCurrentRow))[columnIndex] = x;
    firePropertyChange(columnIndex-1 ,aOldValue);
    fireProperty(PROPERTY_ID_ISMODIFIED,sal_True,sal_False);
}
// -------------------------------------------------------------------------

// XResultSetUpdate
void SAL_CALL ORowSet::insertRow(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    // insertRow is not allowd when
    // standing not on the insert row nor
    // when the row isn't modified
    // or the concurency is read only
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    if(!m_pCache || !m_bNew || !m_bModified || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY)
        throwFunctionSequenceException(*this);

    if(m_bModified)
    {
        // remember old value for fire
        sal_Bool bOld = m_bNew;

        ORowSetRow aOldValues;
        if ( !m_aCurrentRow.isNull() )
            aOldValues = new ORowSetValueVector( m_aCurrentRow->getBody() );
        RowChangeEvent aEvt(*this,RowChangeAction::INSERT,1);
        notifyAllListenersRowBeforeChange(aGuard,aEvt);

        sal_Bool bInserted = m_pCache->insertRow();

        // make sure that our row is set to the new inserted row before clearing the insert flags in the cache
        m_pCache->resetInsertRow(bInserted);

        // notification order
        // - column values
        setCurrentRow( sal_False, sal_True, aOldValues, aGuard ); // we don't move here

        // - rowChanged
        notifyAllListenersRowChanged(aGuard,aEvt);

        // - IsModified
        if(!m_bModified)
            fireProperty(PROPERTY_ID_ISMODIFIED,sal_False,sal_True);
        OSL_ENSURE( !m_bModified, "ORowSet::insertRow: just updated, but _still_ modified?" );

        // - IsNew
        if(m_bNew != bOld)
            fireProperty(PROPERTY_ID_ISNEW,m_bNew,bOld);

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    // check if we are inserting a row
    return (m_pCache && ( m_pCache->m_bNew || m_bModified )) ? 0 : ORowSetBase::getRow();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
    // not allowed when standing on insert row
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    if ( !m_pCache || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY || m_bNew || ((m_pCache->m_nPrivileges & Privilege::UPDATE ) != Privilege::UPDATE) )
        throwFunctionSequenceException(*this);


    if(m_bModified)
    {
        ORowSetRow aOldValues;
        if ( !m_aCurrentRow.isNull() )
            aOldValues = new ORowSetValueVector( m_aCurrentRow->getBody() );

        RowChangeEvent aEvt(*this,RowChangeAction::UPDATE,1);
        notifyAllListenersRowBeforeChange(aGuard,aEvt);

        m_pCache->updateRow(m_aCurrentRow.operator ->());
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aOldRow->setRow(new ORowSetValueVector(m_aCurrentRow->getBody()));

        // notification order
        // - column values
        ORowSetBase::firePropertyChange(aOldValues);

        // - rowChanged
        notifyAllListenersRowChanged(aGuard,aEvt);

        // - IsModified
        if(!m_bModified)
            fireProperty(PROPERTY_ID_ISMODIFIED,sal_False,sal_True);
        OSL_ENSURE( !m_bModified, "ORowSet::updateRow: just updated, but _still_ modified?" );
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::deleteRow(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        throwSQLException( "Cannot delete the before-first or after-last row.", SQL_INVALID_CURSOR_POSITION, *this );
        // TODO: resource
    if ( m_bNew )
        throwSQLException( "Cannot delete the insert-row.", SQL_INVALID_CURSOR_POSITION, *this );
        // TODO: resource
    if  ( m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY )
        throwSQLException( "Result set is read only.", SQL_FUNCTION_SEQUENCE_ERROR, *this );
        // TODO: resource
    if ( ( m_pCache->m_nPrivileges & Privilege::DELETE ) != Privilege::DELETE )
        throwSQLException( "DELETE privilege not available.", SQL_FUNCTION_SEQUENCE_ERROR, *this );
        // TODO: resource
    if ( rowDeleted() )
        throwSQLException( "Current row already deleted.", SQL_FUNCTION_SEQUENCE_ERROR, *this );
        // TODO: resource

    // this call position the cache indirect
    Any aBookmarkToDelete( m_aBookmark );
    positionCache( MOVE_NONE_REFRESH_ONLY );
    sal_Int32 nDeletePosition = m_pCache->getRow();

    notifyRowSetAndClonesRowDelete( aBookmarkToDelete );

    ORowSetRow aOldValues;
    if ( m_pCache->m_aMatrixIter != m_pCache->getEnd() && m_pCache->m_aMatrixIter->isValid() )
        aOldValues = new ORowSetValueVector( m_pCache->m_aMatrixIter->getBody() );

    RowChangeEvent aEvt(*this,RowChangeAction::DELETE,1);
    notifyAllListenersRowBeforeChange(aGuard,aEvt);

    m_pCache->deleteRow();
    notifyRowSetAndClonesRowDeleted( aBookmarkToDelete, nDeletePosition );

    ORowSetNotifier aNotifier( this );
        // this will call cancelRowModification on the cache if necessary

    // notification order
    // - rowChanged
    notifyAllListenersRowChanged(aGuard,aEvt);

    // - IsModified
    // - IsNew
    aNotifier.fire( );

    // - RowCount/IsRowCountFinal
    fireRowcount();
}

// -------------------------------------------------------------------------
void ORowSet::implCancelRowUpdates( sal_Bool _bNotifyModified ) SAL_THROW( ( SQLException, RuntimeException ) )
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( *m_pMutex );
    if ( m_bBeforeFirst || m_bAfterLast || rowDeleted() )
        return; // nothing to do so return

    checkCache();
    // cancelRowUpdates is not allowed when:
    // - standing on the insert row
    // - the concurrency is read only
    // - the current row is deleted
    if ( m_bNew || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY )
        throwFunctionSequenceException(*this);

    positionCache( MOVE_NONE_REFRESH_ONLY );

    ORowSetRow aOldValues;
    if ( !m_aCurrentRow.isNull() )
        aOldValues = new ORowSetValueVector( m_aCurrentRow->getBody() );

    m_pCache->cancelRowUpdates();

    m_aBookmark     = m_pCache->getBookmark();
    m_aCurrentRow   = m_pCache->m_aMatrixIter;
    m_aCurrentRow.setBookmark(m_aBookmark);

    // notification order
    // - column values
    ORowSetBase::firePropertyChange(aOldValues);
    // IsModified
    if( !m_bModified && _bNotifyModified )
        fireProperty(PROPERTY_ID_ISMODIFIED,sal_False,sal_True);
}

// -------------------------------------------------------------------------
void SAL_CALL ORowSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    implCancelRowUpdates( sal_True );
}

// -------------------------------------------------------------------------
void SAL_CALL ORowSet::addRowSetListener( const Reference< XRowSetListener >& listener ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    if(listener.is())
        m_aRowsetListeners.addInterface(listener);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::removeRowSetListener( const Reference< XRowSetListener >& listener ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    if(listener.is())
        m_aRowsetListeners.removeInterface(listener);
}
// -----------------------------------------------------------------------------
void ORowSet::notifyAllListeners(::osl::ResettableMutexGuard& _rGuard)
{
    EventObject aEvt(*m_pMySelf);
    _rGuard.clear();
    m_aRowsetListeners.notifyEach( &XRowSetListener::rowSetChanged, aEvt );
    _rGuard.reset();
}
// -------------------------------------------------------------------------
void ORowSet::notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& _rGuard)
{
    EventObject aEvt(*m_pMySelf);
    _rGuard.clear();
    m_aRowsetListeners.notifyEach( &XRowSetListener::cursorMoved, aEvt );
    _rGuard.reset();
}
// -------------------------------------------------------------------------
void ORowSet::notifyAllListenersRowChanged(::osl::ResettableMutexGuard& _rGuard, const EventObject& aEvt)
{
    _rGuard.clear();
    m_aRowsetListeners.notifyEach( &XRowSetListener::rowChanged, aEvt );
    _rGuard.reset();
}
// -------------------------------------------------------------------------
sal_Bool ORowSet::notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& _rGuard)
{
    EventObject aEvt(*m_pMySelf);
    NOTIFY_LISTERNERS_CHECK(m_aApproveListeners,XRowSetApproveListener,approveCursorMove);
    return bCheck;
}
// -------------------------------------------------------------------------
void ORowSet::notifyAllListenersRowBeforeChange(::osl::ResettableMutexGuard& _rGuard,const RowChangeEvent &aEvt)
{
    NOTIFY_LISTERNERS_CHECK(m_aApproveListeners,XRowSetApproveListener,approveRowChange);
    if ( !bCheck )
        m_aErrors.raiseTypedException( sdb::ErrorCondition::ROW_SET_OPERATION_VETOED, *this, ::cppu::UnoType< RowSetVetoException >::get() );
}
// -------------------------------------------------------------------------
void ORowSet::fireRowcount()
{
    sal_Int32 nCurrentRowCount( impl_getRowCount() );
    sal_Bool bCurrentRowCountFinal( m_pCache->m_bRowCountFinal );

    if ( m_nLastKnownRowCount != nCurrentRowCount )
    {
        sal_Int32 nHandle = PROPERTY_ID_ROWCOUNT;
        Any aNew,aOld;
        aNew <<= nCurrentRowCount; aOld <<= m_nLastKnownRowCount;
        fire(&nHandle,&aNew,&aOld,1,sal_False);
        m_nLastKnownRowCount = nCurrentRowCount;
    }
    if ( !m_bLastKnownRowCountFinal && ( m_bLastKnownRowCountFinal != bCurrentRowCountFinal ) )
    {
        sal_Int32 nHandle = PROPERTY_ID_ISROWCOUNTFINAL;
        Any aNew,aOld;
        aNew <<= bool2any( bCurrentRowCountFinal );
        aOld <<= bool2any( m_bLastKnownRowCountFinal );
        fire(&nHandle,&aNew,&aOld,1,sal_False);
        m_bLastKnownRowCountFinal = bCurrentRowCountFinal;
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();
    if ( ( m_pCache->m_nPrivileges & Privilege::INSERT ) != Privilege::INSERT )
        throwSQLException( "No insert privileges", SQL_GENERAL_ERROR, *this );
        // TODO: resource

    if ( notifyAllListenersCursorBeforeMove( aGuard ) )
    {
        // remember old value for fire
        ORowSetRow aOldValues;
        if ( rowDeleted() )
        {
            positionCache( MOVE_FORWARD );
            m_pCache->next();
            setCurrentRow( sal_True, sal_False, aOldValues, aGuard);
        }
        else
            positionCache( MOVE_NONE_REFRESH_ONLY );

        // check before because the resultset could be empty
        if  (   !m_bBeforeFirst
            &&  !m_bAfterLast
            &&  m_pCache->m_aMatrixIter != m_pCache->getEnd()
            &&  m_pCache->m_aMatrixIter->isValid()
            )
            aOldValues = new ORowSetValueVector( m_pCache->m_aMatrixIter->getBody() );

        const sal_Bool bNewState = m_bNew;
        const sal_Bool bModState = m_bModified;

        m_pCache->moveToInsertRow();
        m_aCurrentRow = m_pCache->m_aInsertRow;

        // notification order
        // - column values
        ORowSetBase::firePropertyChange(aOldValues);

        // - cursorMoved
        notifyAllListenersCursorMoved(aGuard);

        // - IsModified
        if ( bModState != m_bModified )
            fireProperty( PROPERTY_ID_ISMODIFIED, m_bModified, bModState );

        // - IsNew
        if ( bNewState != m_bNew )
            fireProperty( PROPERTY_ID_ISNEW, m_bNew, bNewState );

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    if ( !m_pCache->m_bNew && !m_bModified )
        // nothing to do if we're not on the insertion row, and not modified otherwise
        return;

    if ( rowDeleted() )
        // this would perhaps even justify a RuntimeException ....
        // if the current row is deleted, then no write access to this row should be possible. So,
        // m_bModified should be true. Also, as soon as somebody calls moveToInsertRow,
        // our current row should not be deleted anymore. So, we should not have survived the above
        // check "if ( !m_pCache->m_bNew && !m_bModified )"
        throwSQLException( "The current row is deleted.", SQL_FUNCTION_SEQUENCE_ERROR, *this );
        // TODO: resource

    if ( notifyAllListenersCursorBeforeMove( aGuard ) )
    {
        positionCache( MOVE_NONE_REFRESH_ONLY );

        ORowSetNotifier aNotifier( this );

        // notification order
        // - cursorMoved
        notifyAllListenersCursorMoved(aGuard);

        // - IsModified
        // - IsNew
        aNotifier.fire();
    }
}
// -------------------------------------------------------------------------
// XRow
sal_Bool SAL_CALL ORowSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return ( m_pCache && isInsertRow() ) ? (*(*m_pCache->m_aInsertRow))[m_nLastColumnIndex].isNull() : ORowSetBase::wasNull();
}
// -----------------------------------------------------------------------------
const ORowSetValue& ORowSet::getInsertValue(sal_Int32 columnIndex)
{
    checkCache();

    if ( m_pCache && isInsertRow() )
        return  (*(*m_pCache->m_aInsertRow))[m_nLastColumnIndex = columnIndex];

    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORowSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL ORowSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL ORowSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL ORowSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
float SAL_CALL ORowSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
double SAL_CALL ORowSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL ORowSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL ORowSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL ORowSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL ORowSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    if ( m_pCache && isInsertRow() )
    {
        checkCache();
        return new ::comphelper::SequenceInputStream((*(*m_pCache->m_aInsertRow))[m_nLastColumnIndex = columnIndex].getSequence());
    }

    return ORowSetBase::getBinaryStream(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    if(m_pCache && isInsertRow() )
    {
        checkCache();
        return new ::comphelper::SequenceInputStream((*(*m_pCache->m_aInsertRow))[m_nLastColumnIndex = columnIndex].getSequence());
    }

    return ORowSetBase::getCharacterStream(columnIndex);
}
// -------------------------------------------------------------------------
Any SAL_CALL ORowSet::getObject( sal_Int32 columnIndex, const Reference< XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getInsertValue(columnIndex).makeAny();
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL ORowSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return Reference< XRef >();
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ORowSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return Reference< XBlob >();
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL ORowSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return Reference< XClob >();
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ORowSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return Reference< XArray >();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::executeWithCompletion( const Reference< XInteractionHandler >& _rxHandler ) throw(SQLException, RuntimeException)
{
    if (!_rxHandler.is())
        execute();

    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    // tell everybody that we will change the result set
    approveExecution();

    ResettableMutexGuard aGuard( m_aMutex );

    try
    {
        freeResources( m_bCommandFacetsDirty );

        // calc the connection to be used
        if (m_xActiveConnection.is() && m_bRebuildConnOnExecute)
        {
            // there was a setProperty(ActiveConnection), but a setProperty(DataSource) _after_ that, too
            Reference< XConnection > xXConnection;
            setActiveConnection( xXConnection );
        }
        calcConnection( _rxHandler );
        m_bRebuildConnOnExecute = sal_False;

        Reference< XSingleSelectQueryComposer > xComposer = getCurrentSettingsComposer( this, m_aContext.getLegacyServiceFactory() );
        ::dbtools::askForParameters( xComposer, this, m_xActiveConnection, _rxHandler );
    }
    // ensure that only the allowed exceptions leave this block
    catch(SQLException&)
    {
        throw;
    }
    catch(RuntimeException&)
    {
        throw;
    }
    catch(Exception&)
    {
        DBG_ERROR("ORowSet::executeWithCompletion: caught an unexpected exception type while filling in the parameters!");
    }

    // we're done with the parameters, now for the real execution

    //  do the real execute
    execute_NoApprove_NoNewConn(aGuard);
}

// -------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL ORowSet::getParameters(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    if ( m_bCommandFacetsDirty )
        // need to rebuild the parameters, since some property which contributes to the
        // complete command, and thus the parameters, changed
        impl_disposeParametersContainer_nothrow();

    if ( !m_pParameters.get() && m_aCommand.getLength() )
    {
        try
        {
            ::rtl::OUString sNotInterestedIn;
            impl_initComposer_throw( sNotInterestedIn );
        }
        catch( const Exception& )
        {
            // silence it
        }
    }

    return m_pParameters.get();
}

// -------------------------------------------------------------------------
void ORowSet::approveExecution() throw (RowSetVetoException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    EventObject aEvt(*this);

    OInterfaceIteratorHelper aApproveIter( m_aApproveListeners );
    while ( aApproveIter.hasMoreElements() )
    {
        Reference< XRowSetApproveListener > xListener( static_cast< XRowSetApproveListener* >( aApproveIter.next() ) );
        try
        {
            if ( xListener.is() && !xListener->approveRowSetChange( aEvt ) )
                throw RowSetVetoException();
        }
        catch ( const DisposedException& e )
        {
            if ( e.Context == xListener )
                aApproveIter.remove();
        }
        catch ( const RuntimeException& ) { throw; }
        catch ( const RowSetVetoException& ) { throw; }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -------------------------------------------------------------------------
// XRowSet
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::execute(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    // tell everybody that we will change the result set
    approveExecution();

    ResettableMutexGuard aGuard( m_aMutex );
    freeResources( m_bCommandFacetsDirty );

    // calc the connection to be used
    if (m_xActiveConnection.is() && m_bRebuildConnOnExecute) {
        // there was a setProperty(ActiveConnection), but a setProperty(DataSource) _after_ that, too
        Reference< XConnection> xXConnection;
        setActiveConnection( xXConnection );
    }

    calcConnection(NULL);
    m_bRebuildConnOnExecute = sal_False;

    // do the real execute
    execute_NoApprove_NoNewConn(aGuard);
}

//------------------------------------------------------------------------------
void ORowSet::setStatementResultSetType( const Reference< XPropertySet >& _rxStatement, sal_Int32 _nDesiredResultSetType, sal_Int32 _nDesiredResultSetConcurrency )
{
    OSL_ENSURE( _rxStatement.is(), "ORowSet::setStatementResultSetType: invalid statement - this will crash!" );

    sal_Int32 nResultSetType( _nDesiredResultSetType );
    sal_Int32 nResultSetConcurrency( _nDesiredResultSetConcurrency );

    // there *might* be a data source setting which tells use to be more defensive with those settings
    // #i15113# / 2005-02-10 / frank.schoenheit@sun.com
    sal_Bool bRespectDriverRST = sal_False;
    Any aSetting;
    if ( getDataSourceSetting( ::dbaccess::getDataSource( m_xActiveConnection ), "RespectDriverResultSetType", aSetting ) )
    {
        OSL_VERIFY( aSetting >>= bRespectDriverRST );
    }

    if ( bRespectDriverRST )
    {
        // try type/concurrency settings with decreasing usefullness, and rely on what the connection claims
        // to support
        Reference< XDatabaseMetaData > xMeta( m_xActiveConnection->getMetaData() );

        sal_Int32 nCharacteristics[5][2] =
        {   { ResultSetType::SCROLL_SENSITIVE, ResultSetConcurrency::UPDATABLE },
            { ResultSetType::SCROLL_INSENSITIVE, ResultSetConcurrency::UPDATABLE },
            { ResultSetType::SCROLL_SENSITIVE, ResultSetConcurrency::READ_ONLY },
            { ResultSetType::SCROLL_INSENSITIVE, ResultSetConcurrency::READ_ONLY },
            { ResultSetType::FORWARD_ONLY, ResultSetConcurrency::READ_ONLY }
        };
        for ( sal_Int32 i=0; i<5; ++i )
        {
            nResultSetType = nCharacteristics[i][0];
            nResultSetConcurrency = nCharacteristics[i][1];

            // don't try type/concurrency pairs which are more featured than what our caller requested
            if ( nResultSetType > _nDesiredResultSetType )
                continue;
            if ( nResultSetConcurrency > _nDesiredResultSetConcurrency )
                continue;

            if ( xMeta.is() && xMeta->supportsResultSetConcurrency( nResultSetType, nResultSetConcurrency ) )
                break;
        }
    }

    _rxStatement->setPropertyValue( PROPERTY_RESULTSETTYPE, makeAny( nResultSetType ) );
    _rxStatement->setPropertyValue( PROPERTY_RESULTSETCONCURRENCY, makeAny( nResultSetConcurrency ) );
}

// -----------------------------------------------------------------------------
Reference< XResultSet > ORowSet::impl_prepareAndExecute_throw()
{
    ::rtl::OUString sCommandToExecute;
    sal_Bool bUseEscapeProcessing = impl_initComposer_throw( sCommandToExecute );

    Reference< XResultSet> xResultSet;
    try
    {
        m_xStatement = m_xActiveConnection->prepareStatement( sCommandToExecute );
        if ( !m_xStatement.is() )
        {
            SQLException aError;
            aError.Context = *this;
            aError.SQLState = getStandardSQLState( SQL_GENERAL_ERROR );
            aError.Message = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Internal error: no statement object provided by the database driver." ) );
                // TODO: resource
            throw aError;
        }

        Reference< XPropertySet > xStatementProps( m_xStatement, UNO_QUERY_THROW );
        // set the result set type and concurrency
        try
        {
            xStatementProps->setPropertyValue( PROPERTY_USEBOOKMARKS, makeAny( sal_True ) );
            setStatementResultSetType( xStatementProps, m_nResultSetType, m_nResultSetConcurrency );
        }
        catch ( const Exception& )
        {
            // this exception doesn't matter here because when we catch an exception
            // then the driver doesn't support this feature
        }

        Reference< XParameters > xParam( m_xStatement, UNO_QUERY_THROW );
        size_t nParamCount( m_pParameters.is() ? m_pParameters->size() : m_aPrematureParamValues.size() );
        for ( size_t i=1; i<=nParamCount; ++i )
        {
            ORowSetValue& rParamValue( getParameterStorage( (sal_Int32)i ) );
            ::dbtools::setObjectWithInfo( xParam, i, rParamValue.makeAny(), rParamValue.getTypeKind() );
        }

        xResultSet = m_xStatement->executeQuery();
    }
    catch( const SQLException& )
    {
        SQLExceptionInfo aError( ::cppu::getCaughtException() );
        OSL_ENSURE( aError.isValid(), "ORowSet::impl_prepareAndExecute_throw: caught an SQLException which we cannot analyze!" );

        // append information about what we were actually going to execute
        try
        {
            String sQuery = bUseEscapeProcessing && m_xComposer.is() ? m_xComposer->getQuery() : m_aActiveCommand;
            String sInfo( DBA_RES_PARAM( RID_STR_COMMAND_LEADING_TO_ERROR, "$command$", sQuery ) );
            aError.append( SQLExceptionInfo::SQL_CONTEXT, sInfo );
        }
        catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

        // propagate
        aError.doThrow();
    }

    return xResultSet;
}

// -----------------------------------------------------------------------------
void ORowSet::impl_initializeColumnSettings_nothrow( const Reference< XPropertySet >& _rxTemplateColumn, const Reference< XPropertySet >& _rxRowSetColumn )
{
    OSL_ENSURE( _rxTemplateColumn.is() && _rxRowSetColumn.is(),
        "ORowSet::impl_initializeColumnSettings_nothrow: this will crash!" );

    bool bHaveAnyColumnSetting = false;
    try
    {
        Reference< XPropertySetInfo > xInfo( _rxTemplateColumn->getPropertySetInfo(), UNO_QUERY_THROW );

        // a number of properties is plain copied
        const ::rtl::OUString aPropertyNames[] = {
            PROPERTY_ALIGN, PROPERTY_RELATIVEPOSITION, PROPERTY_WIDTH, PROPERTY_HIDDEN, PROPERTY_CONTROLMODEL,
            PROPERTY_HELPTEXT, PROPERTY_CONTROLDEFAULT
        };
        for ( size_t i=0; i<sizeof( aPropertyNames ) / sizeof( aPropertyNames[0] ); ++i )
        {
            if ( xInfo->hasPropertyByName( aPropertyNames[i] ) )
            {
                _rxRowSetColumn->setPropertyValue( aPropertyNames[i], _rxTemplateColumn->getPropertyValue( aPropertyNames[i] ) );
                bHaveAnyColumnSetting = true;
            }
        }

        // the format key is slightly more complex
        sal_Int32 nFormatKey = 0;
        if( xInfo->hasPropertyByName( PROPERTY_NUMBERFORMAT ) )
        {
            _rxTemplateColumn->getPropertyValue( PROPERTY_NUMBERFORMAT ) >>= nFormatKey;
            bHaveAnyColumnSetting = true;
        }
        if ( !nFormatKey && m_xNumberFormatTypes.is() )
            nFormatKey = ::dbtools::getDefaultNumberFormat( _rxTemplateColumn, m_xNumberFormatTypes, SvtSysLocale().GetLocaleData().getLocale() );
        _rxRowSetColumn->setPropertyValue( PROPERTY_NUMBERFORMAT, makeAny( nFormatKey ) );
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
        return;
    }

    if ( bHaveAnyColumnSetting )
        return;

    // the template column could not provide *any* setting. Okay, probably it's a parser column, which
    // does not offer those. However, perhaps the template column referes to a table column, which we
    // can use as new template column
    try
    {
        Reference< XPropertySetInfo > xInfo( _rxTemplateColumn->getPropertySetInfo(), UNO_QUERY_THROW );
        if ( !xInfo->hasPropertyByName( PROPERTY_TABLENAME ) )
            // no chance
            return;

        ::rtl::OUString sTableName;
        OSL_VERIFY( _rxTemplateColumn->getPropertyValue( PROPERTY_TABLENAME ) >>= sTableName );

        Reference< XNameAccess > xTables( impl_getTables_throw(), UNO_QUERY_THROW );
        if ( !xTables->hasByName( sTableName ) )
            // no chance
            return;

        Reference< XColumnsSupplier > xTableColSup( xTables->getByName( sTableName ), UNO_QUERY_THROW );
        Reference< XNameAccess > xTableCols( xTableColSup->getColumns(), UNO_QUERY_THROW );

        ::rtl::OUString sTableColumnName;

        // get the "Name" or (preferred) "RealName" property of the column
        ::rtl::OUString sNamePropertyName( PROPERTY_NAME );
        if ( xInfo->hasPropertyByName( PROPERTY_REALNAME ) )
            sNamePropertyName = PROPERTY_REALNAME;
        OSL_VERIFY( _rxTemplateColumn->getPropertyValue( sNamePropertyName ) >>= sTableColumnName );

        if ( !xTableCols->hasByName( sTableColumnName ) )
            return;

        Reference< XPropertySet > xTableColumn( xTableCols->getByName( sTableColumnName ), UNO_QUERY_THROW );
        impl_initializeColumnSettings_nothrow( xTableColumn, _rxRowSetColumn );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
void ORowSet::execute_NoApprove_NoNewConn(ResettableMutexGuard& _rClearForNotification)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", "ORowSet::execute_NoApprove_NoNewConn" );

    // now we can dispose our old connection
    ::comphelper::disposeComponent(m_xOldConnection);
    m_xOldConnection = NULL;

    // do we need a new statement
    if ( m_bCommandFacetsDirty )
    {
        m_xStatement    = NULL;
        m_xComposer     = NULL;

        Reference< XResultSet > xResultSet( impl_prepareAndExecute_throw() );

        ::rtl::OUString aComposedUpdateTableName;
        if ( m_aUpdateTableName.getLength() )
            aComposedUpdateTableName = composeTableName( m_xActiveConnection->getMetaData(), m_aUpdateCatalogName, m_aUpdateSchemaName, m_aUpdateTableName, sal_False, ::dbtools::eInDataManipulation );

        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", "ORowSet::execute_NoApprove_NoNewConn: creating cache" );
            m_pCache = new ORowSetCache( xResultSet, m_xComposer.get(), m_aContext, aComposedUpdateTableName, m_bModified, m_bNew );
            if ( m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY )
            {
                m_nPrivileges = Privilege::SELECT;
                m_pCache->m_nPrivileges = Privilege::SELECT;
            }
            m_pCache->setMaxRowSize(m_nFetchSize);
            m_aCurrentRow   = m_pCache->createIterator(this);
            m_aOldRow = m_pCache->registerOldRow();
        }

        // get the locale
        //  ConfigManager*  pConfigMgr = ConfigManager::GetConfigManager();
        Locale aLocale = SvtSysLocale().GetLocaleData().getLocale();
        //  pConfigMgr->GetDirectConfigProperty(ConfigManager::LOCALE) >>= aLocale;

        // get the numberformatTypes
        OSL_ENSURE(m_xActiveConnection.is(),"No ActiveConnection");
        Reference< XNumberFormatTypes> xNumberFormatTypes;
        Reference< XNumberFormatsSupplier> xNumberFormat = ::dbtools::getNumberFormats(m_xActiveConnection);
        if ( xNumberFormat.is() )
            m_xNumberFormatTypes.set(xNumberFormat->getNumberFormats(),UNO_QUERY);

        ::vos::ORef< ::connectivity::OSQLColumns> aColumns = new ::connectivity::OSQLColumns();
        ::std::vector< ::rtl::OUString> aNames;
        ::rtl::OUString aDescription;
        sal_Int32 nFormatKey = 0;

        if(!m_xColumns.is())
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aColumnCreateLog, "dbaccess", "frank.schoenheit@sun.com", "ORowSet::execute_NoApprove_NoNewConn::creating columns" );
            // use the meta data
            Reference<XResultSetMetaDataSupplier> xMetaSup(m_xStatement,UNO_QUERY);
            try
            {
                Reference<XResultSetMetaData> xMetaData = xMetaSup->getMetaData();
                if ( xMetaData.is() )
                {
                    sal_Int32 nCount = xMetaData->getColumnCount();
                    m_aDataColumns.reserve(nCount+1);
                    aColumns->reserve(nCount+1);
                    DECLARE_STL_USTRINGACCESS_MAP(int,StringMap);
                    StringMap aColumnMap;
                    for (sal_Int32 i = 0 ; i < nCount; ++i)
                    {
                        // retrieve the name of the column
                        ::rtl::OUString sName = xMetaData->getColumnName(i + 1);
                        // check for duplicate entries
                        if(aColumnMap.find(sName) != aColumnMap.end())
                        {
                            ::rtl::OUString sAlias(sName);
                            sal_Int32 searchIndex=1;
                            while(aColumnMap.find(sAlias) != aColumnMap.end())
                            {
                                (sAlias = sName) += ::rtl::OUString::valueOf(searchIndex++);
                            }
                            sName = sAlias;
                        }
                        ORowSetDataColumn* pColumn = new ORowSetDataColumn( getMetaData(),
                                                                            this,
                                                                            this,
                                                                            i+1,
                                                                            m_xActiveConnection->getMetaData(),
                                                                            aDescription,
                                                                            m_aCurrentRow);
                        aColumnMap.insert(StringMap::value_type(sName,0));
                        aColumns->push_back(pColumn);
                        pColumn->setName(sName);
                        aNames.push_back(sName);
                        m_aDataColumns.push_back(pColumn);

                        try
                        {
                            nFormatKey = 0;
                            if(m_xNumberFormatTypes.is())
                                nFormatKey = ::dbtools::getDefaultNumberFormat(pColumn,m_xNumberFormatTypes,aLocale);


                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_NUMBERFORMAT,makeAny(nFormatKey));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_RELATIVEPOSITION,makeAny(sal_Int32(i+1)));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_WIDTH,makeAny(sal_Int32(227)));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_ALIGN,makeAny((sal_Int32)0));
                            pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_HIDDEN,::cppu::bool2any(sal_False));
                        }
                        catch(Exception&)
                        {
                        }
                    }
                }
            }
            catch (SQLException&)
            {
            }
        }
        else
        {
            // create the rowset columns
            Reference< XResultSetMetaData > xMeta( getMetaData(), UNO_QUERY_THROW );
            sal_Int32 nCount = xMeta->getColumnCount();
            m_aDataColumns.reserve(nCount+1);
            aColumns->reserve(nCount+1);
            ::std::set< Reference< XPropertySet > > aAllColumns;

            for(sal_Int32 i=1; i <= nCount ;++i)
            {
                ::rtl::OUString sName = xMeta->getColumnName(i);

                // retrieve the column number |i|
                Reference<XPropertySet> xColumn;
                {
                    sal_Bool bReFetchName = sal_False;
                    if (m_xColumns->hasByName(sName))
                        m_xColumns->getByName(sName) >>= xColumn;
                    if (!xColumn.is() && m_xColumns->hasByName(xMeta->getColumnLabel(i)))
                        m_xColumns->getByName(xMeta->getColumnLabel(i)) >>= xColumn;
                    // check if column already in the list we need another
                    if ( aAllColumns.find( xColumn ) != aAllColumns.end() )
                    {
                        xColumn = NULL;
                        bReFetchName = sal_True;
                    }
                    if(!xColumn.is())
                    {
                        // no column found so we could look at the position i
                        Reference<XIndexAccess> xIndexAccess(m_xColumns,UNO_QUERY);
                        if(xIndexAccess.is() && i <= xIndexAccess->getCount())
                        {
                            xIndexAccess->getByIndex(i-1) >>= xColumn;
                        }
                        else
                        {
                            Sequence< ::rtl::OUString> aSeq = m_xColumns->getElementNames();
                            if( i <= aSeq.getLength())
                                m_xColumns->getByName(aSeq.getConstArray()[i-1]) >>= xColumn;
                        }
                    }
                    if(bReFetchName && xColumn.is())
                        xColumn->getPropertyValue(PROPERTY_NAME) >>= sName;
                    aAllColumns.insert( xColumn );
                }

                // create a RowSetDataColumn
                {
                    Reference<XPropertySetInfo> xInfo = xColumn.is() ? xColumn->getPropertySetInfo() : Reference<XPropertySetInfo>();
                    if(xInfo.is() && xInfo->hasPropertyByName(PROPERTY_DESCRIPTION))
                        aDescription = comphelper::getString(xColumn->getPropertyValue(PROPERTY_DESCRIPTION));

                    ORowSetDataColumn* pColumn = new ORowSetDataColumn( getMetaData(),
                                                                        this,
                                                                        this,
                                                                        i,
                                                                        m_xActiveConnection->getMetaData(),
                                                                        aDescription,
                                                                        m_aCurrentRow);
                    aColumns->push_back(pColumn);
                    if(!sName.getLength())
                    {
                        if(xColumn.is())
                            xColumn->getPropertyValue(PROPERTY_NAME) >>= sName;
                        else
                            sName = ::rtl::OUString::createFromAscii("Expression1");
                        // TODO: resource
                    }
                    pColumn->setName(sName);
                    aNames.push_back(sName);
                    m_aDataColumns.push_back(pColumn);

                    if ( xColumn.is() )
                        impl_initializeColumnSettings_nothrow( xColumn, pColumn );
                }
            }
        }
        // now create the columns we need
        if(m_pColumns)
            m_pColumns->assign(aColumns,aNames);
        else
        {
            Reference<XDatabaseMetaData> xMeta = m_xActiveConnection->getMetaData();
            m_pColumns = new ORowSetDataColumns(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),
                                                aColumns,*this,m_aColumnsMutex,aNames);
        }
    }
    checkCache();
    // notify the rowset listeners
    notifyAllListeners(_rClearForNotification);
}
// -------------------------------------------------------------------------
// XRowSetApproveBroadcaster
void SAL_CALL ORowSet::addRowSetApproveListener( const Reference< XRowSetApproveListener >& listener ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    m_aApproveListeners.addInterface(listener);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::removeRowSetApproveListener( const Reference< XRowSetApproveListener >& listener ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    m_aApproveListeners.removeInterface(listener);
}
// -------------------------------------------------------------------------

// XResultSetAccess
Reference< XResultSet > SAL_CALL ORowSet::createResultSet(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    if(m_xStatement.is())
    {
        ORowSetClone* pClone = new ORowSetClone( m_aContext, *this, m_pMutex );
        Reference< XResultSet > xRet(pClone);
        m_aClones.push_back(WeakReferenceHelper(xRet));
        return xRet;
    }
    return Reference< XResultSet >();
}
// -------------------------------------------------------------------------

// ::com::sun::star::util::XCancellable
void SAL_CALL ORowSet::cancel(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL ORowSet::deleteRows( const Sequence< Any >& rows ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    if(!m_pCache || m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY)
        throwFunctionSequenceException(*this);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    RowChangeEvent aEvt(*this,RowChangeAction::DELETE,rows.getLength());
    // notify the rowset listeners
    notifyAllListenersRowBeforeChange(aGuard,aEvt);

    Sequence< sal_Int32 > aResults( rows.getLength() );
    const Any* row = rows.getConstArray();
    const Any* rowEnd = rows.getConstArray() + rows.getLength();
    sal_Int32* result = aResults.getArray();
    for ( ; row != rowEnd; ++row, ++result )
    {
        *result = 0;
        if ( !m_pCache->moveToBookmark( *row ) )
            continue;
        sal_Int32 nDeletePosition = m_pCache->getRow();

        // first notify the clones so that they can save their position
        notifyRowSetAndClonesRowDelete( *row );

        // now delete the row
        if ( !m_pCache->deleteRow() )
            continue;
        *result = 1;
        // now notify that we have deleted
        notifyRowSetAndClonesRowDeleted( *row, nDeletePosition );
    }
    aEvt.Rows = aResults.getLength();

    // we have to check if we stand on the insert row and if so we have to reset it
    ORowSetNotifier aNotifier( this );
        // this will call cancelRowModification on the cache if necessary
    // notification order
    // - rowChanged
    notifyAllListenersRowChanged(aGuard,aEvt);

    // - IsModified
    // - IsNew
    aNotifier.fire();

    // - RowCount/IsRowCountFinal
    fireRowcount();

    return aResults;
}
// -----------------------------------------------------------------------------
void ORowSet::notifyRowSetAndClonesRowDelete( const Any& _rBookmark )
{
    // notify ourself
    onDeleteRow( _rBookmark );
    // notify the clones
    for (connectivity::OWeakRefArray::iterator i = m_aClones.begin(); m_aClones.end() != i; i++)
    {
        Reference< XUnoTunnel > xTunnel(i->get(),UNO_QUERY);
        if(xTunnel.is())
        {
            ORowSetClone* pClone = reinterpret_cast<ORowSetClone*>(xTunnel->getSomething(ORowSetClone::getUnoTunnelImplementationId()));
            if(pClone)
                pClone->onDeleteRow( _rBookmark );
        }
    }
}
//------------------------------------------------------------------------------
void ORowSet::notifyRowSetAndClonesRowDeleted( const Any& _rBookmark, sal_Int32 _nPos )
{
    // notify ourself
    onDeletedRow( _rBookmark, _nPos );
    // notify the clones
    for (connectivity::OWeakRefArray::iterator i = m_aClones.begin(); m_aClones.end() != i; i++)
    {
        Reference< XUnoTunnel > xTunnel(i->get(),UNO_QUERY);
        if(xTunnel.is())
        {
            ORowSetClone* pClone = reinterpret_cast<ORowSetClone*>(xTunnel->getSomething(ORowSetClone::getUnoTunnelImplementationId()));
            if(pClone)
                pClone->onDeletedRow( _rBookmark, _nPos );
        }
    }
}
//------------------------------------------------------------------------------
Reference< XConnection >  ORowSet::calcConnection(const Reference< XInteractionHandler >& _rxHandler) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_xActiveConnection.is())
    {
        Reference< XConnection > xNewConn;
        if (m_aDataSourceName.getLength())
        {
            // is it a file url?
            Reference< XNameAccess > xNamingContext;
            if ( m_aContext.createComponent( (::rtl::OUString)SERVICE_SDB_DATABASECONTEXT, xNamingContext ) )
            if (xNamingContext.is() )
            {
                try
                {
                    if(_rxHandler.is())
                    {
                        Reference< XCompletedConnection> xComplConn(xNamingContext->getByName(m_aDataSourceName), UNO_QUERY);
                        if(xComplConn.is())
                            xNewConn = xComplConn->connectWithCompletion(_rxHandler);
                    }
                    else
                    {
                        Reference< XDataSource >  xDataSource(xNamingContext->getByName(m_aDataSourceName), UNO_QUERY);
                        if (xDataSource.is())
                            xNewConn = xDataSource->getConnection(m_aUser, m_aPassword);
                    }
                }
                catch (SQLException &e)
                {
                    throw e;
                }
                catch (Exception&)
                {
                    throw SQLException();
                }
            }
        }
        setActiveConnection(xNewConn);
        m_bOwnConnection = sal_True;
    }
    return m_xActiveConnection;
}
//------------------------------------------------------------------------------
Reference< XNameAccess > ORowSet::impl_getTables_throw()
{
    Reference< XNameAccess > xTables;

    Reference< XTablesSupplier >  xTablesAccess( m_xActiveConnection, UNO_QUERY );
    if ( xTablesAccess.is() )
    {
        xTables.set( xTablesAccess->getTables(), UNO_QUERY_THROW );
    }
    else if ( m_pTables )
    {
        xTables = m_pTables;
    }
    else
    {
        if ( !m_xActiveConnection.is() )
                throw SQLException(DBA_RES(RID_STR_CONNECTION_INVALID),*this,SQLSTATE_GENERAL,1000,Any() );

        sal_Bool bCase = sal_True;
        try
        {
            Reference<XDatabaseMetaData> xMeta = m_xActiveConnection->getMetaData();
            bCase = xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers();
        }
        catch(SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        m_pTables = new OTableContainer(*this,m_aMutex,m_xActiveConnection,bCase,NULL,NULL,NULL,m_nInAppend);
        xTables = m_pTables;
        Sequence< ::rtl::OUString> aTableFilter(1);
        aTableFilter[0] = ::rtl::OUString::createFromAscii("%");
        m_pTables->construct(aTableFilter,Sequence< ::rtl::OUString>());
    }

    return xTables;
}

//------------------------------------------------------------------------------
void ORowSet::impl_resetTables_nothrow()
{
    if ( !m_pTables )
        return;

    try
    {
        m_pTables->dispose();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    DELETEZ( m_pTables );
}

//------------------------------------------------------------------------------
sal_Bool ORowSet::impl_initComposer_throw( ::rtl::OUString& _out_rCommandToExecute )
{
    sal_Bool bUseEscapeProcessing = impl_buildActiveCommand_throw( );
    _out_rCommandToExecute = m_aActiveCommand;
    if ( !bUseEscapeProcessing )
        return bUseEscapeProcessing;

    Reference< XMultiServiceFactory > xFactory( m_xActiveConnection, UNO_QUERY );
    if ( xFactory.is() )
    {
        try
        {
            ::comphelper::disposeComponent( m_xComposer );
            m_xComposer.set( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
        }
        catch (const Exception& ) { m_xComposer = NULL; }
    }
    if ( !m_xComposer.is() )
        m_xComposer = new OSingleSelectQueryComposer( impl_getTables_throw(), m_xActiveConnection, m_aContext );

    m_xComposer->setElementaryQuery( m_aActiveCommand );

    m_xComposer->setFilter( m_bApplyFilter ? m_aFilter : ::rtl::OUString() );
    m_xComposer->setHavingClause( m_bApplyFilter ? m_aHavingClause : ::rtl::OUString() );

    if ( m_bIgnoreResult )
    {   // append a "0=1" filter
        // don't simply overwrite an existent filter, this would lead to problems if this existent
        // filter contains paramters (since a keyset may add parameters itself)
        // 2003-12-12 - #23418# - fs@openoffice.org
        m_xComposer->setElementaryQuery( m_xComposer->getQuery( ) );
        m_xComposer->setFilter( ::rtl::OUString::createFromAscii( "0 = 1" ) );
    }

    m_xComposer->setOrder( m_aOrder );
    m_xComposer->setGroup( m_aGroupBy );

    if ( !m_xColumns.is() )
    {
        Reference< XColumnsSupplier > xCols( m_xComposer, UNO_QUERY_THROW );
        m_xColumns = xCols->getColumns();
    }

    impl_initParametersContainer_nothrow();

    _out_rCommandToExecute = m_xComposer->getQueryWithSubstitution();
    return bUseEscapeProcessing;
}

//------------------------------------------------------------------------------
sal_Bool ORowSet::impl_buildActiveCommand_throw()
{
    // create the sql command
    // from a table name or get the command out of a query (not a view)
    // the last use the command as it is
    sal_Bool bDoEscapeProcessing = m_bUseEscapeProcessing;

    m_aActiveCommand = ::rtl::OUString();
    ::rtl::OUString sCommand;

    if ( !m_aCommand.getLength() )
        return bDoEscapeProcessing;

    switch (m_nCommandType)
    {
        case CommandType::TABLE:
        {
            impl_resetTables_nothrow();

            Reference< XNameAccess > xTables( impl_getTables_throw() );
            if ( xTables->hasByName(m_aCommand) )
            {
                Reference< XPropertySet > xTable;
                try
                {
                    xTables->getByName( m_aCommand ) >>= xTable;
                }
                catch(const WrappedTargetException& e)
                {
                    SQLException e2;
                    if ( e.TargetException >>= e2 )
                        throw e2;
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

                Reference<XColumnsSupplier> xSup(xTable,UNO_QUERY);
                if ( xSup.is() )
                    m_xColumns = xSup->getColumns();

                sCommand = rtl::OUString::createFromAscii("SELECT * FROM ");
                ::rtl::OUString sCatalog, sSchema, sTable;
                ::dbtools::qualifiedNameComponents( m_xActiveConnection->getMetaData(), m_aCommand, sCatalog, sSchema, sTable, ::dbtools::eInDataManipulation );
                sCommand += ::dbtools::composeTableNameForSelect( m_xActiveConnection, sCatalog, sSchema, sTable );
            }
            else
            {
                String sMessage( DBACORE_RESSTRING( RID_STR_TABLE_DOES_NOT_EXIST ) );
                sMessage.SearchAndReplaceAscii( "$table$", m_aCommand );
                throwGenericSQLException(sMessage,*this);
            }
        }
        break;

        case CommandType::QUERY:
        {
            Reference< XQueriesSupplier >  xQueriesAccess(m_xActiveConnection, UNO_QUERY);
            if (xQueriesAccess.is())
            {
                Reference< ::com::sun::star::container::XNameAccess >  xQueries(xQueriesAccess->getQueries());
                if (xQueries->hasByName(m_aCommand))
                {
                    Reference< XPropertySet > xQuery(xQueries->getByName(m_aCommand),UNO_QUERY);
                    OSL_ENSURE(xQuery.is(),"ORowSet::impl_buildActiveCommand_throw: Query is NULL!");
                    if ( xQuery.is() )
                    {
                        xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
                        xQuery->getPropertyValue(PROPERTY_ESCAPE_PROCESSING) >>= bDoEscapeProcessing;

                        ::rtl::OUString aCatalog,aSchema,aTable;
                        xQuery->getPropertyValue(PROPERTY_UPDATE_CATALOGNAME)   >>= aCatalog;
                        xQuery->getPropertyValue(PROPERTY_UPDATE_SCHEMANAME)    >>= aSchema;
                        xQuery->getPropertyValue(PROPERTY_UPDATE_TABLENAME)     >>= aTable;
                        if(aTable.getLength())
                            m_aUpdateTableName = composeTableName( m_xActiveConnection->getMetaData(), aCatalog, aSchema, aTable, sal_False, ::dbtools::eInDataManipulation );

                        Reference<XColumnsSupplier> xSup(xQuery,UNO_QUERY);
                        if(xSup.is())
                            m_xColumns = xSup->getColumns();
                    }
                }
                else
                {
                    String sMessage( DBACORE_RESSTRING( RID_STR_TABLE_DOES_NOT_EXIST ) );
                    sMessage.SearchAndReplaceAscii( "$table$", m_aCommand );
                    throwGenericSQLException(sMessage,*this);
                }
            }
            else
                throw SQLException(DBA_RES(RID_STR_NO_XQUERIESSUPPLIER),*this,::rtl::OUString(),0,Any());
        }
        break;

        default:
            sCommand = m_aCommand;
            break;
    }

    m_aActiveCommand = sCommand;

    if ( !m_aActiveCommand.getLength() )
        throwSQLException( "No SQL command was provided.", SQL_FUNCTION_SEQUENCE_ERROR, *this );
        // TODO: resource

    return bDoEscapeProcessing;
}

//------------------------------------------------------------------------------
void ORowSet::impl_initParametersContainer_nothrow()
{
    OSL_PRECOND( !m_pParameters.is(), "ORowSet::impl_initParametersContainer_nothrow: already initialized the parameters!" );

    m_pParameters = new param::ParameterWrapperContainer( m_xComposer.get() );
    // copy the premature parameters into the final ones
    size_t nParamCount( ::std::min( m_pParameters->size(), m_aPrematureParamValues.size() ) );
    for ( size_t i=0; i<nParamCount; ++i )
    {
        (*m_pParameters)[i] = m_aPrematureParamValues[i];
    }
}

//------------------------------------------------------------------------------
void ORowSet::impl_disposeParametersContainer_nothrow()
{
    if ( !m_pParameters.is() )
        return;

    // copy the actual values to our "premature" ones, to preserve them for later use
    size_t nParamCount( m_pParameters->size() );
    m_aPrematureParamValues.resize( nParamCount );
    for ( size_t i=0; i<nParamCount; ++i )
    {
        m_aPrematureParamValues[i] = (*m_pParameters)[i];
    }

    m_pParameters->dispose();
    m_pParameters = NULL;
}

// -----------------------------------------------------------------------------
ORowSetValue& ORowSet::getParameterStorage(sal_Int32 parameterIndex)
{
    ::connectivity::checkDisposed( ORowSet_BASE1::rBHelper.bDisposed );
    if ( parameterIndex < 1 )
        throwInvalidIndexException( *this );

    if ( m_pParameters.is() )
    {
        if ( (size_t)parameterIndex > m_pParameters->size() )
            throwInvalidIndexException( *this );
        return (*m_pParameters)[ parameterIndex - 1 ];
    }

    if ( m_aPrematureParamValues.size() < (size_t)parameterIndex )
        m_aPrematureParamValues.resize( parameterIndex );
    return m_aPrematureParamValues[ parameterIndex - 1 ];
}
// -------------------------------------------------------------------------
// XParameters
void SAL_CALL ORowSet::setNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    getParameterStorage( parameterIndex ).setNull();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    setNull( parameterIndex, sqlType );
}
// -----------------------------------------------------------------------------
void ORowSet::setParameter(sal_Int32 parameterIndex, const ORowSetValue& x)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    getParameterStorage( parameterIndex ) = x;
}

// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    ORowSetValue& rParamValue( getParameterStorage( parameterIndex ) );

    try
    {
        Sequence <sal_Int8> aData;
        x->readBytes(aData, length);
        rParamValue = aData;
        x->closeInput();
    }
    catch( Exception& )
    {
        throw SQLException();
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    ORowSetValue& rParamValue( getParameterStorage( parameterIndex ) );
    try
    {
        Sequence <sal_Int8> aData;
        rtl::OUString aDataStr;
        // the data is given as character data and the length defines the character length
        sal_Int32 nSize = x->readBytes(aData, length * sizeof(sal_Unicode));
        if (nSize / sizeof(sal_Unicode))
            aDataStr = rtl::OUString((sal_Unicode*)aData.getConstArray(), nSize / sizeof(sal_Unicode));
        rParamValue = aDataStr;
        rParamValue.setTypeKind( DataType::LONGVARCHAR );
        x->closeInput();
    }
    catch( Exception& )
    {
        throw SQLException();
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if ( !::dbtools::implSetObject( this, parameterIndex, x ) )
    {   // there is no other setXXX call which can handle the value in x
        throw SQLException();
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 /*scale*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    ORowSetValue& rParamValue( getParameterStorage( parameterIndex ) );
    setObject( parameterIndex, x );
    rParamValue.setTypeKind( targetSqlType );
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setRef", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setBlob", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setClob", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setArray", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSet::clearParameters(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(ORowSet_BASE1::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    size_t nParamCount( m_pParameters.is() ? m_pParameters->size() : m_aPrematureParamValues.size() );
    for ( size_t i=1; i<=nParamCount; ++i )
        getParameterStorage( (sal_Int32)i ).setNull();
}
// -------------------------------------------------------------------------
void ORowSet::firePropertyChange(sal_Int32 _nPos,const ::connectivity::ORowSetValue& _rOldValue)
{
    OSL_ENSURE(_nPos < (sal_Int32)m_aDataColumns.size(),"nPos is invalid!");
    m_aDataColumns[_nPos]->fireValueChange(_rOldValue);
}

// -----------------------------------------------------------------------------
void ORowSet::doCancelModification( )
{
    //OSL_ENSURE( isModification(), "ORowSet::doCancelModification: invalid call (no cache!)!" );
    if ( isModification() )
        m_pCache->cancelRowModification();
    m_bModified = sal_False;
}

// -----------------------------------------------------------------------------
sal_Bool ORowSet::isModification( )
{
    return isNew();
}

// -----------------------------------------------------------------------------
sal_Bool ORowSet::isModified( )
{
    return m_bModified;
}

// -----------------------------------------------------------------------------
sal_Bool ORowSet::isNew( )
{
    return m_bNew;
}

// -----------------------------------------------------------------------------
void ORowSet::checkUpdateIterator()
{
    if(!m_bModified && !m_bNew)
    {
        m_pCache->setUpdateIterator(m_aCurrentRow);
        m_aCurrentRow = m_pCache->m_aInsertRow;
        m_bModified = sal_True;
    } // if(!m_bModified && !m_bNew)
    else if ( m_bNew ) // here we are modifing a value
        m_bModified = sal_True;
}
// -----------------------------------------------------------------------------
void ORowSet::checkUpdateConditions(sal_Int32 columnIndex)
{
    checkCache();
    if ( columnIndex <= 0 )
        throwSQLException( "Invalid column index", SQL_INVALID_DESCRIPTOR_INDEX, *this );
        // TODO: resource
    if ( rowDeleted() )
        throwSQLException( "Current row is deleted", SQL_INVALID_CURSOR_POSITION, *this );
        // TODO: resource
    if ( m_aCurrentRow.isNull() )
        throwSQLException( "Invalid cursor state", SQL_INVALID_CURSOR_STATE, *this );
        // TODO: resource
    if ( m_nResultSetConcurrency == ResultSetConcurrency::READ_ONLY)
        throwSQLException( "Result set is not writeable", SQL_GENERAL_ERROR, *this );
        // TODO: resource
}
// -----------------------------------------------------------------------------
void SAL_CALL ORowSet::refreshRow(  ) throw(SQLException, RuntimeException)
{

    ORowSetNotifier aNotifier( this );
        // this will call cancelRowModification on the cache if necessary

    // notification order:
    if ( m_bModified && m_pCache )
        // - column values
        implCancelRowUpdates( sal_False ); // do _not_ notify the IsModify - will do this ourself below

    ORowSetBase::refreshRow();

    // - IsModified
    // - IsNew
    aNotifier.fire( );
}
// ***********************************************************
//  ORowSetClone
// ***********************************************************
DBG_NAME(ORowSetClone);
//--------------------------------------------------------------------------
ORowSetClone::ORowSetClone( const ::comphelper::ComponentContext& _rContext, ORowSet& rParent, ::osl::Mutex* _pMutex )
             :OSubComponent(m_aMutex, rParent)
             ,ORowSetBase( _rContext, OComponentHelper::rBHelper, _pMutex )
             ,m_pParent(&rParent)
             ,m_nFetchDirection(rParent.m_nFetchDirection)
             ,m_nFetchSize(rParent.m_nFetchSize)
             ,m_bIsBookmarable(sal_True)
{
    DBG_CTOR(ORowSetClone, NULL);

    m_nResultSetType        = rParent.m_nResultSetType;
    m_nResultSetConcurrency = ResultSetConcurrency::READ_ONLY;
    m_pMySelf               = this;
    m_bClone                = sal_True;
    m_bBeforeFirst          = rParent.m_bBeforeFirst;
    m_bAfterLast            = rParent.m_bAfterLast;
    m_pCache                = rParent.m_pCache;
    m_aBookmark             = rParent.m_aBookmark;
    m_aCurrentRow           = m_pCache->createIterator(this);
    m_xNumberFormatTypes    = rParent.m_xNumberFormatTypes;

    m_aOldRow = m_pCache->registerOldRow();

    ::vos::ORef< ::connectivity::OSQLColumns> aColumns = new ::connectivity::OSQLColumns();
    ::std::vector< ::rtl::OUString> aNames;

    ::rtl::OUString aDescription;
    //  ConfigManager*  pConfigMgr = ConfigManager::GetConfigManager();
    //  Locale aLocale;
    //  pConfigMgr->GetDirectConfigProperty(ConfigManager::LOCALE) >>= aLocale;
    Locale aLocale = SvtSysLocale().GetLocaleData().getLocale();

    Sequence< ::rtl::OUString> aSeq = rParent.m_pColumns->getElementNames();
    const ::rtl::OUString* pBegin   = aSeq.getConstArray();
    const ::rtl::OUString* pEnd     = pBegin + aSeq.getLength();
    aColumns->reserve(aSeq.getLength()+1);
    for(sal_Int32 i=1;pBegin != pEnd ;++pBegin,++i)
    {
        Reference<XPropertySet> xColumn;
        rParent.m_pColumns->getByName(*pBegin) >>= xColumn;
        if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION))
            aDescription = comphelper::getString(xColumn->getPropertyValue(PROPERTY_DESCRIPTION));

        ORowSetColumn* pColumn = new ORowSetColumn( rParent.getMetaData(),
                                                            this,
                                                            i,
                                                            rParent.m_xActiveConnection->getMetaData(),
                                                            aDescription,
                                                            m_aCurrentRow);
        aColumns->push_back(pColumn);
        pColumn->setName(*pBegin);
        aNames.push_back(*pBegin);
        m_aDataColumns.push_back(pColumn);

        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_ALIGN,xColumn->getPropertyValue(PROPERTY_ALIGN));
        sal_Int32 nFormatKey = comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_NUMBERFORMAT));
        if(!nFormatKey && xColumn.is() && m_xNumberFormatTypes.is())
            nFormatKey = ::dbtools::getDefaultNumberFormat(xColumn,m_xNumberFormatTypes,aLocale);
        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_NUMBERFORMAT,makeAny(nFormatKey));
        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_RELATIVEPOSITION,xColumn->getPropertyValue(PROPERTY_RELATIVEPOSITION));
        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_WIDTH,xColumn->getPropertyValue(PROPERTY_WIDTH));
        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_HIDDEN,xColumn->getPropertyValue(PROPERTY_HIDDEN));
        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_CONTROLMODEL,xColumn->getPropertyValue(PROPERTY_CONTROLMODEL));
        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_HELPTEXT,xColumn->getPropertyValue(PROPERTY_HELPTEXT));
        pColumn->setFastPropertyValue_NoBroadcast(PROPERTY_ID_CONTROLDEFAULT,xColumn->getPropertyValue(PROPERTY_CONTROLDEFAULT));

    }
    Reference<XDatabaseMetaData> xMeta = rParent.m_xActiveConnection->getMetaData();
    m_pColumns = new ORowSetDataColumns(xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),
                                        aColumns,*this,m_aMutex,aNames);

    sal_Int32 nRT   = PropertyAttribute::READONLY   | PropertyAttribute::TRANSIENT;

    // sdb.RowSet Properties
    //  registerProperty(PROPERTY_CURSORNAME,       PROPERTY_ID_CURSORNAME,         PropertyAttribute::READONLY,        &m_aDataSourceName,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));

    registerProperty(PROPERTY_RESULTSETCONCURRENCY, PROPERTY_ID_RESULTSETCONCURRENCY,   PropertyAttribute::READONLY,    &m_nResultSetConcurrency,::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_RESULTSETTYPE,        PROPERTY_ID_RESULTSETTYPE,          PropertyAttribute::READONLY,    &m_nResultSetType,      ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_FETCHDIRECTION,       PROPERTY_ID_FETCHDIRECTION,         PropertyAttribute::TRANSIENT,   &m_nFetchDirection,     ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_FETCHSIZE,            PROPERTY_ID_FETCHSIZE,              PropertyAttribute::TRANSIENT,   &m_nFetchSize,          ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_ISBOOKMARKABLE,       PROPERTY_ID_ISBOOKMARKABLE,         nRT,                            &m_bIsBookmarable,      ::getBooleanCppuType());
}

//--------------------------------------------------------------------------
ORowSetClone::~ORowSetClone()
{
    DBG_DTOR(ORowSetClone, NULL);
}
// com::sun::star::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > ORowSetClone::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(OSubComponent::getTypes(),ORowSetBase::getTypes());
}
// com::sun::star::XInterface
//--------------------------------------------------------------------------
Any ORowSetClone::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aRet = ORowSetBase::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OSubComponent::queryInterface(rType);
    return aRet;
}
//------------------------------------------------------------------------------
void ORowSetClone::acquire() throw()
{
    OSubComponent::acquire();
}

//------------------------------------------------------------------------------
void ORowSetClone::release() throw()
{
    OSubComponent::release();
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ORowSetClone::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.ORowSetClone");
}

//------------------------------------------------------------------------------
sal_Bool ORowSetClone::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ORowSetClone::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBC_RESULTSET;
    aSNS[1] = SERVICE_SDB_RESULTSET;
    return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void ORowSetClone::disposing()
{
    MutexGuard aGuard( m_aMutex );
    ORowSetBase::disposing();

    m_pParent   = NULL;
    m_pMutex    = &m_aMutex; // this must be done here because someone could hold a ref to us and try to do something
    OSubComponent::disposing();
}

// XCloseable
//------------------------------------------------------------------------------
void ORowSetClone::close(void) throw( SQLException, RuntimeException )
{
    {
        MutexGuard aGuard( m_aMutex );
        if (OComponentHelper::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}
// -------------------------------------------------------------------------

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSetClone::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL ORowSetClone::getInfoHelper()
{
    typedef ::comphelper::OPropertyArrayUsageHelper<ORowSetClone> ORowSetClone_PROP;
    return *ORowSetClone_PROP::getArrayHelper();
}
// -------------------------------------------------------------------------
//--------------------------------------------------------------------------
Sequence< sal_Int8 > ORowSetClone::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -----------------------------------------------------------------------------
// com::sun::star::XUnoTunnel
sal_Int64 SAL_CALL ORowSetClone::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}
// -----------------------------------------------------------------------------
void SAL_CALL ORowSetClone::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    if ( nHandle == PROPERTY_ID_FETCHSIZE )
    {
        if ( m_pParent )
            m_pParent->setFastPropertyValue_NoBroadcast( nHandle, rValue );
    }

    OPropertyStateContainer::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}

// -----------------------------------------------------------------------------
void ORowSetClone::doCancelModification( )
{
    //OSL_ENSURE( sal_False, "ORowSetClone::doCancelModification: invalid call!" );
}

// -----------------------------------------------------------------------------
sal_Bool ORowSetClone::isModification( )
{
    return sal_False;
}

// -----------------------------------------------------------------------------
sal_Bool ORowSetClone::isModified( )
{
    return sal_False;
}

// -----------------------------------------------------------------------------
sal_Bool ORowSetClone::isNew( )
{
    return sal_False;
}

// -------------------------------------------------------------------------
void SAL_CALL ORowSetClone::execute(  ) throw(SQLException, RuntimeException)
{
    throwFunctionNotSupportedException( "RowSetClone::XRowSet::execute", *this );
}

// -------------------------------------------------------------------------
void SAL_CALL ORowSetClone::addRowSetListener( const Reference< XRowSetListener >& ) throw(RuntimeException)
{
    throwFunctionNotSupportedException( "RowSetClone::XRowSet", *this );
}

// -------------------------------------------------------------------------
void SAL_CALL ORowSetClone::removeRowSetListener( const Reference< XRowSetListener >& ) throw(RuntimeException)
{
    throwFunctionNotSupportedException( "RowSetClone::XRowSet", *this );
}

} // dbaccess
