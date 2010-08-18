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
#include "precompiled_dbaccess.hxx"
#ifndef DBACCESS_INDEXES_HXX_
#include "CIndexes.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_INDEXTYPE_HPP_
#include <com/sun/star/sdbc/IndexType.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbaccess;
using namespace cppu;


ObjectType OIndexes::createObject(const ::rtl::OUString& _rName)
{
    ObjectType xRet;
    if ( m_xIndexes.is() && m_xIndexes->hasByName(_rName) )
        xRet.set(m_xIndexes->getByName(_rName),UNO_QUERY);
    else
        xRet = OIndexesHelper::createObject(_rName);

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createDescriptor()
{
    Reference<XDataDescriptorFactory> xData( m_xIndexes,UNO_QUERY);
    if(xData.is())
        return xData->createDataDescriptor();
    else
        return OIndexesHelper::createDescriptor();
}
// -------------------------------------------------------------------------
// XAppend
ObjectType OIndexes::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference<XAppend> xData( m_xIndexes,UNO_QUERY);
    if ( !xData.is() )
        return OIndexesHelper::appendObject( _rForName, descriptor );

    xData->appendByDescriptor(descriptor);
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OIndexes::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if ( m_xIndexes.is() )
    {
        Reference<XDrop> xData( m_xIndexes,UNO_QUERY);
        if ( xData.is() )
            xData->dropByName(_sElementName);
    }
    else
        OIndexesHelper::dropObject(_nPos,_sElementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OIndexes::disposing(void)
{
    if ( m_xIndexes.is() )
        clear_NoDispose();
    else
        OIndexesHelper::disposing();
}
// -----------------------------------------------------------------------------




