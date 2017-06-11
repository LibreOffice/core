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

#include "ado/AIndexes.hxx"
#include "ado/AIndex.hxx"
#include "ado/AConnection.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include "strings.hrc"

using namespace ::comphelper;


using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

sdbcx::ObjectType OIndexes::createObject(const OUString& _rName)
{
    return new OAdoIndex(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}

void OIndexes::impl_refresh()
{
    m_aCollection.Refresh();
}

Reference< XPropertySet > OIndexes::createDescriptor()
{
    return new OAdoIndex(isCaseSensitive(),m_pConnection);
}

// XAppend
sdbcx::ObjectType OIndexes::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OAdoIndex* pIndex = nullptr;
    if ( !getImplementation(pIndex,descriptor) || pIndex == nullptr )
        m_pConnection->throwGenericSQLException( STR_INVALID_INDEX_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    ADOIndexes* pIndexes = m_aCollection;
    if ( FAILED( pIndexes->Append( OLEVariant( _rForName ), OLEVariant( pIndex->getImpl() ) ) ) )
    {
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
        m_pConnection->throwGenericSQLException( STR_INVALID_INDEX_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );
    }

    return new OAdoIndex(isCaseSensitive(),m_pConnection,pIndex->getImpl());
}

// XDrop
void OIndexes::dropObject(sal_Int32 /*_nPos*/,const OUString& _sElementName)
{
    m_aCollection.Delete(_sElementName);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
