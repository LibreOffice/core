/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ECatalog.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:59:19 $
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


#ifndef _CONNECTIVITY_FLAT_CATALOG_HXX_
#include "flat/ECatalog.hxx"
#endif

#ifndef _CONNECTIVITY_FLAT_DCONNECTION_HXX_
#include "flat/EConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_TABLES_HXX_
#include "flat/ETables.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

// -------------------------------------------------------------------------
using namespace connectivity::flat;
// -------------------------------------------------------------------------
OFlatCatalog::OFlatCatalog(OFlatConnection* _pCon) : file::OFileCatalog(_pCon)
{
}
// -------------------------------------------------------------------------
void OFlatCatalog::refreshTables()
{
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes;
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        ::rtl::OUString::createFromAscii("%"),::rtl::OUString::createFromAscii("%"),aTypes);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(3));
    }
    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OFlatTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -----------------------------------------------------------------------------



