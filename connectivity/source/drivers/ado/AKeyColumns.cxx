/*************************************************************************
 *
 *  $RCSfile: AKeyColumns.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:20 $
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

#ifndef _CONNECTIVITY_ADABAS_KEYCOLUMNS_HXX_
#include "adabas/BKeyColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_KEYCOLUMN_HXX_
#include "adabas/BKeyColumn.hxx"
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
// -------------------------------------------------------------------------
staruno::Reference< starcontainer::XNamed > OKeyColumns::createObject(const ::rtl::OUString& _rName)
{

    staruno::Reference< starsdbc::XResultSet >
        xResult = m_pTable->getConnection()->getMetaData()->getImportedKeys(staruno::Any(),
                    m_pTable->getSchema(),m_pTable->getName());

    ::rtl::OUString aRefColumnName;
    if(xResult.is())
    {
        staruno::Reference< starsdbc::XRow > xRow(xResult,staruno::UNO_QUERY);
        ::rtl::OUString aDot = ::rtl::OUString::createFromAscii(".");
        while(xResult->next())
        {
            if(xRow->getString(8) == _rName)
            {
                aRefColumnName = xRow->getString(4);
                break;
            }
        }
    }

     xResult = m_pTable->getConnection()->getMetaData()->getColumns(staruno::Any(),
        m_pTable->getSchema(),m_pTable->getName(),_rName);

    staruno::Reference< starcontainer::XNamed > xRet = NULL;
    if(xResult.is())
    {
        staruno::Reference< starsdbc::XRow > xRow(xResult,staruno::UNO_QUERY);
        if(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                OKeyColumn* pRet = new OKeyColumn(aRefColumnName,
                                                    _rName,
                                                    xRow->getString(6),
                                                    xRow->getString(13),
                                                    xRow->getInt(11),
                                                    xRow->getInt(7),
                                                    xRow->getInt(9),
                                                    xRow->getInt(5),
                                                    sal_False);
                xRet = pRet;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
staruno::Reference< starbeans::XPropertySet > OKeyColumns::createEmptyObject()
{
    OKeyColumn* pNew = new OKeyColumn();
    return pNew;
}
// -------------------------------------------------------------------------

