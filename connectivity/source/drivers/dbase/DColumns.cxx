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
#include "dbase/DColumns.hxx"
#include "dbase/DTable.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/property.hxx>

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

    ::rtl::Reference<OSQLColumns> aCols = pTable->getTableColumns();
    OSQLColumns::Vector::const_iterator aIter = find(aCols->get().begin(),aCols->get().end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));

    sdbcx::ObjectType xRet;
    if(aIter != aCols->get().end())
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
