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

#include "ado/AKeys.hxx"
#include "ado/AKey.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include "ado/AConnection.hxx"
#include <comphelper/types.hxx>
#include "ado/Awrapado.hxx"
#include <comphelper/property.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/ado_res.hrc"

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;

sdbcx::ObjectType OKeys::createObject(const OUString& _rName)
{
    return new OAdoKey(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}

void OKeys::impl_refresh()
{
    m_aCollection.Refresh();
}

Reference< XPropertySet > OKeys::createDescriptor()
{
    return new OAdoKey(isCaseSensitive(),m_pConnection);
}

// XAppend
sdbcx::ObjectType OKeys::appendObject( const OUString&, const Reference< XPropertySet >& descriptor )
{
    OAdoKey* pKey = nullptr;
    if ( !getImplementation( pKey, descriptor ) || pKey == nullptr)
        m_pConnection->throwGenericSQLException( STR_INVALID_KEY_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    // To pass as column parameter to Key's Append method
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
    OUString sName = aKey.get_Name();
    if(!sName.getLength())
        aKey.put_Name("PrimaryKey");

    ADOKeys* pKeys = m_aCollection;
    if ( FAILED(pKeys->Append(OLEVariant(static_cast<ADOKey*>(aKey)),
                            adKeyPrimary, // must be every time adKeyPrimary
                            vOptional)) )
    {
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
        // just make sure that an SQLExceptionis thrown here
        m_pConnection->throwGenericSQLException( STR_INVALID_KEY_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );
    }

    return new OAdoKey(isCaseSensitive(),m_pConnection,pKey->getImpl());
}

// XDrop
void OKeys::dropObject(sal_Int32 /*_nPos*/,const OUString& _sElementName)
{
    if(!m_aCollection.Delete(OLEVariant(_sElementName).getString()))
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
