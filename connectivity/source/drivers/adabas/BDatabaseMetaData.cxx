/*************************************************************************
 *
 *  $RCSfile: BDatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-24 06:12:05 $
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
    ::rtl::OUString aValue = ::rtl::OUString::createFromAscii("sdbc:adabas:");
    aValue += OAdabasDatabaseMetaData_BASE::getURLImpl();
    return aValue;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasDatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -----------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OAdabasDatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{

    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet();
    Reference< XResultSet > xNewRes = pResult;
    pResult->setTypeInfoMap();
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
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getString    (1));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getInt       (2));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getInt       (3));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getString    (4));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getString    (5));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getString    (6));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getInt       (7));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getBoolean   (8));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getShort     (9));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getBoolean   (10));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getInt       (11));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getBoolean   (12));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getString    (13));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getShort     (14));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getShort     (15));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getInt       (16));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator(xRow->getInt       (17));
                if(xRow->wasNull())
                    aRow[nPos-1]->setNull();
                aRow[nPos++] = new ::connectivity::ODatabaseMetaDataResultSet::ORowSetValueDecorator((sal_Int16)xRow->getInt(18));
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



