/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FCatalog.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:54:30 $
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

#ifndef _CONNECTIVITY_FILE_CATALOG_HXX_
#include "file/FCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_BCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_TABLES_HXX_
#include "file/FTables.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
//#ifndef _CONNECTIVITY_FILE_OEMPTYCOLLECTION_HXX_
//#include "file/FEmptyCollection.hxx"
//#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

// -------------------------------------------------------------------------
using namespace connectivity::file;
// -------------------------------------------------------------------------
OFileCatalog::OFileCatalog(OConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OFileCatalog::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    typedef connectivity::sdbcx::OCatalog OFileCatalog_BASE;
    m_xMetaData = NULL;
    OFileCatalog_BASE::disposing();
}
// -----------------------------------------------------------------------------
::rtl::OUString OFileCatalog::buildName(const Reference< XRow >& _xRow)
{
    return _xRow->getString(3);
}
// -------------------------------------------------------------------------
void OFileCatalog::refreshTables()
{
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes;
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        ::rtl::OUString::createFromAscii("%"),::rtl::OUString::createFromAscii("%"),aTypes);
    fillNames(xResult,aVector);

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(m_xMetaData,*this,m_aMutex,aVector);
}

// -------------------------------------------------------------------------
Any SAL_CALL OFileCatalog::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XGroupsSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XUsersSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XViewsSupplier>*)0))
        return Any();


    typedef sdbcx::OCatalog OFileCatalog_BASE;
    return OFileCatalog_BASE::queryInterface(rType);
}
// -----------------------------------------------------------------------------
Sequence< Type > SAL_CALL OFileCatalog::getTypes(  ) throw(RuntimeException)
{
    typedef sdbcx::OCatalog OFileCatalog_BASE;

    Sequence< Type > aTypes = OFileCatalog_BASE::getTypes();
    ::std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XGroupsSupplier>*)0) ||
            *pBegin == ::getCppuType((const Reference<XUsersSupplier>*)0)   ||
            *pBegin == ::getCppuType((const Reference<XViewsSupplier>*)0)))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    const Type *pTypes = aOwnTypes.empty() ? 0 : &aOwnTypes[0];
    return Sequence< Type >(pTypes, aOwnTypes.size());
}
// -----------------------------------------------------------------------------


