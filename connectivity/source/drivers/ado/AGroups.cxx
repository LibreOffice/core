/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AGroups.cxx,v $
 * $Revision: 1.12 $
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
#include "ado/AGroups.hxx"
#ifndef _CONNECTIVITY_ADO_GROUP_HXX_
#include "ado/AGroup.hxx"
#endif
#include "ado/ATable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>

using namespace comphelper;
using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

typedef connectivity::sdbcx::OCollection OCollection_TYPE;
// -------------------------------------------------------------------------
sdbcx::ObjectType OGroups::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoGroup(m_pCatalog,isCaseSensitive(),_rName);
}
// -------------------------------------------------------------------------
void OGroups::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OGroups::createDescriptor()
{
    return new OAdoGroup(m_pCatalog,isCaseSensitive());
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OGroups::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OAdoGroup* pGroup = NULL;
    if ( !getImplementation(pGroup,descriptor) || pGroup == NULL )
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not create group: invalid object descriptor." ),
            static_cast<XTypeProvider*>(this)
        );

    m_aCollection.Append( pGroup->getImpl() );
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OGroups::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    m_aCollection.Delete(_sElementName);
}
// -----------------------------------------------------------------------------




