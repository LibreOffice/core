/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
#include "precompiled_connectivity.hxx"

#include "adabas/BDatabaseMetaData.hxx"
#include "FDatabaseMetaDataResultSet.hxx"

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

::rtl::OUString SAL_CALL OAdabasDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aValue(RTL_CONSTASCII_USTRINGPARAM("sdbc:adabas:"));
    aValue += OAdabasDatabaseMetaData_BASE::getURLImpl();
    return aValue;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasDatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -----------------------------------------------------------------------------
Reference< XResultSet > OAdabasDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xNewRes = pResult;
    static ::connectivity::ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        aRows.reserve(19);
        Reference< XResultSet > xRes = OAdabasDatabaseMetaData_BASE::impl_getTypeInfo_throw();

        if(xRes.is())
        {
            ::connectivity::ODatabaseMetaDataResultSet::ORow aRow(19);

            Reference< XRow> xRow(xRes,UNO_QUERY);
            while(xRes->next())
            {
                sal_Int32 nPos = 1;
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (1));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (2));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (3));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (4));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (5));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (6));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (7));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getBoolean   (8));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getShort     (9));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getBoolean   (10));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (11));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getBoolean   (12));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getString    (13));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getShort     (14));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getShort     (15));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (16));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator(xRow->getInt       (17));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ORowSetValueDecorator((sal_Int16)xRow->getInt(18));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();

                // we have to fix some incorrect entries
                if(!aRow[2]->getValue().isNull())
                {
                    switch((sal_Int32)aRow[2]->getValue())
                    {
                        case DataType::FLOAT:
                        case DataType::REAL:
                        case DataType::DOUBLE:
                            aRow[2]->setValue(DataType::DOUBLE);
                            aRow[3]->setValue(sal_Int32(18));
                            break;
                        case DataType::TIMESTAMP:
                            aRow[3]->setValue(sal_Int32(27));
                            break;
                        default:
                            break;
                    }
                }
                aRows.push_back(aRow);
            }
        }
    }
    pResult->setRows(aRows);
    return xNewRes;
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
