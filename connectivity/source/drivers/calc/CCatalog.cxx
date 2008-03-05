/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CCatalog.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:25:59 $
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


#ifndef _CONNECTIVITY_CALC_CATALOG_HXX_
#include "calc/CCatalog.hxx"
#endif

#ifndef _CONNECTIVITY_CALC_CONNECTION_HXX_
#include "calc/CConnection.hxx"
#endif
#ifndef _CONNECTIVITY_CALC_TABLES_HXX_
#include "calc/CTables.hxx"
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
using namespace connectivity::calc;
// -------------------------------------------------------------------------
OCalcCatalog::OCalcCatalog(OCalcConnection* _pCon) : file::OFileCatalog(_pCon)
{
}
// -------------------------------------------------------------------------
void OCalcCatalog::refreshTables()
{
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes;
    OCalcConnection::ODocHolder aDocHodler(((OCalcConnection*)m_pConnection));
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
        m_pTables = new OCalcTables(m_xMetaData,*this,m_aMutex,aVector);

    // this avoids that the document will be loaded a 2nd time when one table will be accessed.
    if ( m_pTables && m_pTables->hasElements() )
        m_pTables->getByIndex(0);
}
// -----------------------------------------------------------------------------



