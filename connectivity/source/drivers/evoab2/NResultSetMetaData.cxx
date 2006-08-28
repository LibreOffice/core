 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-28 14:52:58 $
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

#ifndef _CONNECTIVITY_EVOAB_RESULTSETMETADATA_HXX_
#include "NResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_DATABASEMETADATA_HXX_
#include "NDatabaseMetaData.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#include "NDebug.hxx"

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

OEvoabResultSetMetaData::OEvoabResultSetMetaData(const ::rtl::OUString& _aTableName)
    : m_aTableName(_aTableName),
      m_aEvoabFields()
{

}
// -------------------------------------------------------------------------
OEvoabResultSetMetaData::~OEvoabResultSetMetaData()
{
}
// -------------------------------------------------------------------------
void OEvoabResultSetMetaData::setEvoabFields(const ::vos::ORef<connectivity::OSQLColumns> &xColumns) throw(SQLException)
{
        OSQLColumns::const_iterator aIter;
        static const ::rtl::OUString aName(::rtl::OUString::createFromAscii("Name"));

        for (aIter = xColumns->begin(); aIter != xColumns->end(); ++aIter)
        {
                ::rtl::OUString aFieldName;

                (*aIter)->getPropertyValue(aName) >>= aFieldName;
                guint nFieldNumber = findEvoabField(aFieldName);
                if (nFieldNumber == (guint)-1)
                    ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii("Invalid column name: ") + aFieldName,
                        NULL);
                m_aEvoabFields.push_back(nFieldNumber);
        }
}

// -------------------------------------------------------------------------
void OEvoabResultSetMetaData::checkColumnIndex(sal_Int32 nColumnNum)  throw(SQLException, RuntimeException)
{
    if( nColumnNum <= 0 || nColumnNum > getColumnCount() )
        dbtools::throwInvalidIndexException( *this );
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnDisplaySize( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return 50;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnType( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
        sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldType (nField);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    return m_aEvoabFields.size();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isCaseSensitive( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getSchemaName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldName( nField );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnTypeName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldTypeName( nField );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnLabel( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    const ColumnProperty *pSpecs = getField(nField);
    GParamSpec *pSpec = pSpecs->pField;
    rtl::OUString aLabel;

    if( pSpec )
            aLabel = rtl::OStringToOUString( g_param_spec_get_nick( (GParamSpec *) pSpec ),
                                             RTL_TEXTENCODING_UTF8 );
    return aLabel;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnServiceName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getTableName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return m_aTableName;//::rtl::OUString::createFromAscii("TABLE");
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getCatalogName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabResultSetMetaData::isCurrency( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isAutoIncrement( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isSigned( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getPrecision( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getScale( sal_Int32 /*nColumnNum*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::isNullable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isSearchable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isReadOnly( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isDefinitelyWritable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isWritable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
