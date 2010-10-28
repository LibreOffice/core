/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle andor its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "PrivateRow.hxx"

using namespace dbaccess;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star;

::sal_Bool SAL_CALL OPrivateRow::wasNull(  ) throw (SQLException, RuntimeException)
    {
        return m_aRow[m_nPos].isNull();
    }
    ::rtl::OUString SAL_CALL OPrivateRow::getString( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::sal_Bool SAL_CALL OPrivateRow::getBoolean( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::sal_Int8 SAL_CALL OPrivateRow::getByte( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::sal_Int16 SAL_CALL OPrivateRow::getShort( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::sal_Int32 SAL_CALL OPrivateRow::getInt( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::sal_Int64 SAL_CALL OPrivateRow::getLong( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    float SAL_CALL OPrivateRow::getFloat( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    double SAL_CALL OPrivateRow::getDouble( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    Sequence< ::sal_Int8 > SAL_CALL OPrivateRow::getBytes( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::com::sun::star::util::Date SAL_CALL OPrivateRow::getDate( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::com::sun::star::util::Time SAL_CALL OPrivateRow::getTime( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    ::com::sun::star::util::DateTime SAL_CALL OPrivateRow::getTimestamp( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos];
    }
    Reference< ::com::sun::star::io::XInputStream > SAL_CALL OPrivateRow::getBinaryStream( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return Reference< ::com::sun::star::io::XInputStream >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< ::com::sun::star::io::XInputStream > SAL_CALL OPrivateRow::getCharacterStream( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return Reference< ::com::sun::star::io::XInputStream >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Any SAL_CALL OPrivateRow::getObject( ::sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >&  ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].makeAny();
    }
    Reference< XRef > SAL_CALL OPrivateRow::getRef( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return Reference< XRef >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< XBlob > SAL_CALL OPrivateRow::getBlob( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return Reference< XBlob >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< XClob > SAL_CALL OPrivateRow::getClob( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return Reference< XClob >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< XArray > SAL_CALL OPrivateRow::getArray( ::sal_Int32 columnIndex ) throw (SQLException, RuntimeException)
    {
        m_nPos = columnIndex;
        return Reference< XArray >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }

