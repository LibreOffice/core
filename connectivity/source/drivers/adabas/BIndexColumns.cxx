/*************************************************************************
 *
 *  $RCSfile: BIndexColumns.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:19 $
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

#ifndef _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_
#include "adabas/BIndexColumns.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEXCOLUMN_HXX_
#include "connectivity/sdbcx/VIndexColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif

using namespace connectivity::adabas;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
Reference< XNamed > OIndexColumns::createObject(const ::rtl::OUString& _rName)
{

        Reference< XResultSet > xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getIndexInfo(Any(),
        m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getName(),sal_False,sal_False);

    sal_Bool bAsc = sal_True;
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aD(::rtl::OUString::createFromAscii("D"));
        while(xResult->next())
        {
            if(xRow->getString(9) == _rName)
                bAsc = xRow->getString(10) != aD;
        }
    }

         xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getColumns(Any(),
        m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getName(),_rName);

        Reference< XNamed > xRet = NULL;
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                OIndexColumn* pRet = new OIndexColumn(bAsc,
                                                    _rName,
                                                    xRow->getString(6),
                                                    xRow->getString(13),
                                                    xRow->getInt(11),
                                                    xRow->getInt(7),
                                                    xRow->getInt(9),
                                                    xRow->getInt(5),
                                                    sal_False,sal_False,sal_False,sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexColumns::createEmptyObject()
{
    OIndexColumn* pNew = new OIndexColumn(sal_True);
    return pNew;
}
// -------------------------------------------------------------------------

