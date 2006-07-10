/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DColumns.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:24:58 $
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

#ifndef _CONNECTIVITY_DBASE_COLUMNS_HXX_
#include "dbase/DColumns.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

using namespace connectivity::dbase;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

typedef file::OColumns ODbaseColumns_BASE;
sdbcx::ObjectType ODbaseColumns::createObject(const ::rtl::OUString& _rName)
{

    ODbaseTable* pTable = (ODbaseTable*)m_pTable;

    //  Reference< XFastPropertySet> xCol(pTable->getColumns()[_rName],UNO_QUERY);
    ::vos::ORef<OSQLColumns> aCols = pTable->getTableColumns();
    OSQLColumns::const_iterator aIter = find(aCols->begin(),aCols->end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));

    sdbcx::ObjectType xRet;
    if(aIter != aCols->end())
        xRet = sdbcx::ObjectType(*aIter,UNO_QUERY);
    return xRet;
}

// -------------------------------------------------------------------------
void ODbaseColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseColumns::createDescriptor()
{
    return new sdbcx::OColumn(isCaseSensitive());
}
// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType ODbaseColumns::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    m_pTable->addColumn( descriptor );
    return createObject( _rForName );
}
// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------
// XDrop
void ODbaseColumns::dropObject(sal_Int32 _nPos,const ::rtl::OUString /*_sElementName*/)
{
    if(!m_pTable->isNew())
        m_pTable->dropColumn(_nPos);
}
// -----------------------------------------------------------------------------

