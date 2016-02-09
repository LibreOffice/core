/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ado/AGroups.hxx"
#include "ado/AGroup.hxx"
#include "ado/ATable.hxx"
#include "ado/AConnection.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/ado_res.hrc"

using namespace comphelper;
using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;


sdbcx::ObjectType OGroups::createObject(const OUString& _rName)
{
    return new OAdoGroup(m_pCatalog,isCaseSensitive(),_rName);
}

void OGroups::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}

Reference< XPropertySet > OGroups::createDescriptor()
{
    return new OAdoGroup(m_pCatalog,isCaseSensitive());
}

// XAppend
sdbcx::ObjectType OGroups::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OAdoGroup* pGroup = NULL;
    if ( !getImplementation(pGroup,descriptor) || pGroup == NULL )
        m_pCatalog->getConnection()->throwGenericSQLException( STR_INVALID_GROUP_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    m_aCollection.Append( pGroup->getImpl() );
    return createObject( _rForName );
}

// XDrop
void OGroups::dropObject(sal_Int32 /*_nPos*/,const OUString& _sElementName)
{
    m_aCollection.Delete(_sElementName);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
