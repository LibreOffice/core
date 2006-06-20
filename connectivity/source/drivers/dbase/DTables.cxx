/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DTables.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:21:24 $
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

#ifndef _CONNECTIVITY_DBASE_TABLES_HXX_
#include "dbase/DTables.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _CONNECTIVITY_FILE_CATALOG_HXX_
#include "file/FCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_BCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _CONNECTIVITY_DBASE_CATALOG_HXX_
#include "dbase/DCatalog.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
namespace starutil      = ::com::sun::star::util;

sdbcx::ObjectType ODbaseTables::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aSchema;
    ODbaseTable* pRet = new ODbaseTable(this,(ODbaseConnection*)static_cast<OFileCatalog&>(m_rParent).getConnection(),
                                        _rName,::rtl::OUString::createFromAscii("TABLE"));

    sdbcx::ObjectType xRet = pRet;
    pRet->construct();
    return xRet;
}
// -------------------------------------------------------------------------
void ODbaseTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<ODbaseCatalog*>(&m_rParent)->refreshTables();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseTables::createEmptyObject()
{
    return new ODbaseTable(this,(ODbaseConnection*)static_cast<OFileCatalog&>(m_rParent).getConnection());
}
typedef connectivity::sdbcx::OCollection ODbaseTables_BASE_BASE;
// -------------------------------------------------------------------------
// XAppend
void ODbaseTables::appendObject( const Reference< XPropertySet >& descriptor )
{
    Reference<XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        ODbaseTable* pTable = reinterpret_cast< ODbaseTable* >( xTunnel->getSomething(ODbaseTable::getUnoTunnelImplementationId()) );
        if(pTable)
        {
            pTable->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
            try
            {
                if(!pTable->CreateImpl())
                    throw SQLException();
            }
            catch(SQLException&)
            {
                throw;
            }
            catch(Exception&)
            {
                throw SQLException();
            }
        }
    }
}
// -------------------------------------------------------------------------
// XDrop
void ODbaseTables::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< XUnoTunnel> xTunnel;
    try
    {
        xTunnel.set(getObject(_nPos),UNO_QUERY);
    }
    catch(const Exception&)
    {
        if(ODbaseTable::Drop_Static(ODbaseTable::getEntry(static_cast<OFileCatalog&>(m_rParent).getConnection(),_sElementName),sal_False,NULL))
            return;
    }

    if ( xTunnel.is() )
    {
        ODbaseTable* pTable = reinterpret_cast< ODbaseTable* >( xTunnel->getSomething(ODbaseTable::getUnoTunnelImplementationId()) );
        if(pTable)
            pTable->DropImpl();
    }
    else
        throw SQLException(::rtl::OUString::createFromAscii("Can't drop table ") + _sElementName,static_cast<XTypeProvider*>(this),OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ERRORMSG_SEQUENCE),1000,Any());

}
// -------------------------------------------------------------------------
Any SAL_CALL ODbaseTables::queryInterface( const Type & rType ) throw(RuntimeException)
{
    typedef sdbcx::OCollection OTables_BASE;
    return OTables_BASE::queryInterface(rType);
}
// -----------------------------------------------------------------------------


