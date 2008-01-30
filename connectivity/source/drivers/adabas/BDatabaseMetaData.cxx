/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BDatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:48:45 $
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
#include "precompiled_connectivity.hxx"

#ifndef _CONNECTIVITY_ADABAS_BDATABASEMETADATA_HXX_
#include "adabas/BDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
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
        Reference< XResultSet > xRes = OAdabasDatabaseMetaData_BASE::getTypeInfo();

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
//                          aRow[3] = sal_Int32(15);
//                          break;
                        case DataType::REAL:
//                          aRow[3] = sal_Int32(16);
//                          break;
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



