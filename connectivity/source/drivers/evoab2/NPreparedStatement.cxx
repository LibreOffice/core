 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NPreparedStatement.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:51:42 $
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

#include <stdio.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _CONNECTIVITY_EVOAB_PREPAREDSTATEMENT_HXX_
#include "NPreparedStatement.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OEvoabPreparedStatement,"com.sun.star.sdbcx.evoab.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OEvoabPreparedStatement::OEvoabPreparedStatement( OEvoabConnection* _pConnection, const ::rtl::OUString& sql)
    :OStatement_BASE2(_pConnection)
    ,m_bPrepared(sal_False)
    ,m_sSqlStatement(sql)
    ,m_nNumParams(0)
{
}
// -----------------------------------------------------------------------------
OEvoabPreparedStatement::~OEvoabPreparedStatement()
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
Any SAL_CALL OEvoabPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OEvoabPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OEvoabPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OEvoabResultSetMetaData(m_pConnection->getCurrentTableName());
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // Reset last warning message

    try {
        clearWarnings ();
        OStatement_BASE2::close();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

    // Remove this Statement object from the Connection object's
    // list
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet> xRS = OStatement_Base::executeQuery( m_sSqlStatement );
    // same as in statement with the difference that this statement also can contain parameter

    return xRS.is();
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OEvoabPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // same as in statement with the difference that this statement also can contain parameter
    return 0;
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OEvoabPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OEvoabPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > rs = OStatement_Base::executeQuery( m_sSqlStatement );
    return rs;
}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OEvoabPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------


void SAL_CALL OEvoabPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}

// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------


void SAL_CALL OEvoabPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OEvoabPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void OEvoabPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            break;
        default:
            OStatement_Base::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}
// -----------------------------------------------------------------------------
void OEvoabPreparedStatement::checkParameterIndex(sal_Int32 _parameterIndex)
{
    if( !_parameterIndex || _parameterIndex > m_nNumParams)
        throw SQLException();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL OEvoabPreparedStatement::getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabPreparedStatement::getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabPreparedStatement::getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}
// -----------------------------------------------------------------------------
