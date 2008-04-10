/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AKeys.cxx,v $
 * $Revision: 1.20 $
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
#include "ado/AKeys.hxx"
#ifndef _CONNECTIVITY_ADO_INDEX_HXX_
#include "ado/AKey.hxx"
#endif
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include "ado/AConnection.hxx"
#include <comphelper/types.hxx>
#include "ado/Awrapado.hxx"
#include <comphelper/property.hxx>
#include <connectivity/dbexception.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;

sdbcx::ObjectType OKeys::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoKey(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}
// -------------------------------------------------------------------------
void OKeys::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeys::createDescriptor()
{
    return new OAdoKey(isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OKeys::appendObject( const ::rtl::OUString&, const Reference< XPropertySet >& descriptor )
{
    OAdoKey* pKey = NULL;
    if ( !getImplementation( pKey, descriptor ) || pKey == NULL)
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not create key: invalid object descriptor." ),
            static_cast<XTypeProvider*>(this)
        );

    // To pass as column parameter to Key's Apppend method
    OLEVariant vOptional;
    vOptional.setNoArg();

#if OSL_DEBUG_LEVEL > 0
    KeyTypeEnum eKey =
#endif
        OAdoKey::Map2KeyRule(getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
#if OSL_DEBUG_LEVEL > 0
    (void)eKey;
#endif

    WpADOKey aKey = pKey->getImpl();
    ::rtl::OUString sName = aKey.get_Name();
    if(!sName.getLength())
        aKey.put_Name(::rtl::OUString::createFromAscii("PrimaryKey") );

    ADOKeys* pKeys = m_aCollection;
    if ( FAILED(pKeys->Append(OLEVariant((ADOKey*)aKey),
                            adKeyPrimary, // must be every time adKeyPrimary
                            vOptional)) )
    {
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
        // just make sure that an SQLExceptionis thrown here
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not append key." ),
            static_cast<XTypeProvider*>(this)
        );
    }

    return new OAdoKey(isCaseSensitive(),m_pConnection,pKey->getImpl());
}
// -------------------------------------------------------------------------
// XDrop
void OKeys::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    if(!m_aCollection.Delete(OLEVariant(_sElementName)))
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
}
// -----------------------------------------------------------------------------


