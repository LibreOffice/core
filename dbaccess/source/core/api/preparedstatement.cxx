/*************************************************************************
 *
 *  $RCSfile: preparedstatement.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-25 07:30:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _DBA_COREAPI_PREPAREDSTATEMENT_HXX_
#include <preparedstatement.hxx>
#endif
#ifndef _DBA_COREAPI_RESULTSET_HXX_
#include <resultset.hxx>
#endif
#ifndef _DBA_COREAPI_RESULTCOLUMN_HXX_
#include <resultcolumn.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
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
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;

DBG_NAME(OPreparedStatement);

//--------------------------------------------------------------------------
OPreparedStatement::OPreparedStatement(const Reference< XConnection > & _xConn,
                                      const Reference< XInterface > & _xStatement)
                   :OStatementBase(_xConn, _xStatement)
                   ,m_aColumns(*this, m_aMutex, _xConn->getMetaData()->supportsMixedCaseQuotedIdentifiers(),::std::vector< ::rtl::OUString>())
{
    DBG_CTOR(OPreparedStatement, NULL);
    m_xAggregateAsParameters = Reference< XParameters > (m_xAggregateAsSet, UNO_QUERY);
}

//--------------------------------------------------------------------------
OPreparedStatement::~OPreparedStatement()
{
    DBG_DTOR(OPreparedStatement, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OPreparedStatement::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XServiceInfo > *)0 ),
                           ::getCppuType( (const Reference< XPreparedStatement > *)0 ),
                           ::getCppuType( (const Reference< XParameters > *)0 ),
                           ::getCppuType( (const Reference< XResultSetMetaDataSupplier > *)0 ),
                           ::getCppuType( (const Reference< XColumnsSupplier > *)0 ),
                            OStatementBase::getTypes() );

    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OPreparedStatement::getImplementationId() throw (RuntimeException)
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

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OPreparedStatement::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OStatementBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XServiceInfo * >( this ),
                    static_cast< XParameters * >( this ),
                    static_cast< XColumnsSupplier * >( this ),
                    static_cast< XResultSetMetaDataSupplier * >( this ),
                    static_cast< XPreparedBatchExecution * >( this ),
                    static_cast< XMultipleResults * >( this ),
                    static_cast< XPreparedStatement * >( this ));
    return aIface;
}

//--------------------------------------------------------------------------
void OPreparedStatement::acquire() throw (RuntimeException)
{
    OStatementBase::acquire();
}

//--------------------------------------------------------------------------
void OPreparedStatement::release() throw (RuntimeException)
{
    OStatementBase::release();
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OPreparedStatement::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OPreparedStatement");
}

//------------------------------------------------------------------------------
sal_Bool OPreparedStatement::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OPreparedStatement::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS.getArray()[0] = SERVICE_SDBC_PREPAREDSTATEMENT;
    aSNS.getArray()[1] = SERVICE_SDB_PREPAREDSTATMENT;
    return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OPreparedStatement::disposing()
{
    {
        MutexGuard aGuard(m_aMutex);
        m_aColumns.disposing();
        m_xAggregateAsParameters = NULL;
    }
    OStatementBase::disposing();
}

// ::com::sun::star::sdbcx::XColumnsSupplier
//------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > OPreparedStatement::getColumns(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    // do we have to populate the columns
    if (!m_aColumns.isInitialized())
    {
        // get the metadata
        Reference< XResultSetMetaData > xMetaData = Reference< XResultSetMetaDataSupplier >(m_xAggregateAsSet, UNO_QUERY)->getMetaData();
        // do we have columns
        try
        {
            for (sal_Int32 i = 0, nCount = xMetaData->getColumnCount(); i < nCount; ++i)
            {
                // retrieve the name of the column
                rtl::OUString aName = xMetaData->getColumnName(i + 1);
                OResultColumn* pColumn = new OResultColumn(xMetaData, i + 1);
                m_aColumns.append(aName, pColumn);
            }
        }
        catch (SQLException)
        {
        }
        m_aColumns.setInitialized();
    }
    return &m_aColumns;
}

// XResultSetMetaDataSupplier
//------------------------------------------------------------------------------
Reference< XResultSetMetaData > OPreparedStatement::getMetaData(void) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    return Reference< XResultSetMetaDataSupplier >(m_xAggregateAsSet, UNO_QUERY)->getMetaData();
}

// XPreparedStatement
//------------------------------------------------------------------------------
Reference< XResultSet >  OPreparedStatement::executeQuery() throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    disposeResultSet();

    Reference< XResultSet > xResultSet;
    Reference< XResultSet > xDrvResultSet = Reference< XPreparedStatement >(m_xAggregateAsSet, UNO_QUERY)->executeQuery();
    if (xDrvResultSet.is())
    {
        xResultSet = new OResultSet(xDrvResultSet, *this, m_aColumns.isCaseSensitive());

        // keep the resultset weak
        m_aResultSet = xResultSet;
    }
    return xResultSet;
}

//------------------------------------------------------------------------------
sal_Int32 OPreparedStatement::executeUpdate() throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    disposeResultSet();

    return Reference< XPreparedStatement >(m_xAggregateAsSet, UNO_QUERY)->executeUpdate();
}

//------------------------------------------------------------------------------
sal_Bool OPreparedStatement::execute() throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    disposeResultSet();
    return Reference< XPreparedStatement >(m_xAggregateAsSet, UNO_QUERY)->execute();
}

//------------------------------------------------------------------------------
Reference< XConnection > OPreparedStatement::getConnection(void) throw( SQLException, RuntimeException )
{
    return Reference< XConnection > (m_xParent, UNO_QUERY);
}

// XParameters
//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setNull(parameterIndex, sqlType);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setObjectNull(parameterIndex, sqlType, typeName);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setBoolean(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setByte(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setShort(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setInt(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setLong(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setFloat(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setDouble(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setString(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setBytes(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setDate(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setTime(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setTimestamp(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setBinaryStream(parameterIndex, x, length);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setCharacterStream(parameterIndex, x, length);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setObject(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setObjectWithInfo(parameterIndex, x, targetSqlType, scale);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setRef(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setBlob(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setClob(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->setArray(parameterIndex, x);
}

//------------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    m_xAggregateAsParameters->clearParameters();
}

