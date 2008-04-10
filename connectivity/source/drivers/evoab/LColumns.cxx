/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LColumns.cxx,v $
 * $Revision: 1.6 $
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

#include "LColumns.hxx"
#include "LTable.hxx"
#include "connectivity/sdbcx/VColumn.hxx"

using namespace connectivity::evoab;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;


sdbcx::ObjectType OEvoabColumns::createObject(const ::rtl::OUString& _rName)
{

    OEvoabTable* pTable = (OEvoabTable*)m_pTable;
    ::vos::ORef<OSQLColumns> aCols = pTable->getTableColumns();
    OSQLColumns::const_iterator aIter = find(aCols->begin(),aCols->end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));
    sdbcx::ObjectType xRet;
    if(aIter != aCols->end())
        xRet = sdbcx::ObjectType(*aIter,UNO_QUERY);
    return xRet;
}
// -------------------------------------------------------------------------


