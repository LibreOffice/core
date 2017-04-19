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

#include "ado/AColumns.hxx"
#include "ado/AColumn.hxx"
#include "ado/AConnection.hxx"
#include "ado/Awrapado.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <algorithm>
#include "strings.hrc"

using namespace connectivity::ado;
using namespace connectivity;
using namespace comphelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

sdbcx::ObjectType OColumns::createObject(const OUString& _rName)
{
    return new OAdoColumn(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}


void OColumns::impl_refresh()
{
    m_aCollection.Refresh();
}

Reference< XPropertySet > OColumns::createDescriptor()
{
    return new OAdoColumn(isCaseSensitive(),m_pConnection);
}

// XAppend
sdbcx::ObjectType OColumns::appendObject( const OUString&, const Reference< XPropertySet >& descriptor )
{
    OAdoColumn* pColumn = nullptr;
    Reference< XPropertySet > xColumn;
    if ( !getImplementation( pColumn, descriptor ) || pColumn == nullptr )
    {
        // m_pConnection->throwGenericSQLException( STR_INVALID_COLUMN_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );
        pColumn = new OAdoColumn(isCaseSensitive(),m_pConnection);
        xColumn = pColumn;
        ::comphelper::copyProperties(descriptor,xColumn);
    }

    WpADOColumn aColumn = pColumn->getColumnImpl();

#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nPrecision;
    sal_Int32 nScale;
    sal_Int32 nType;
    nPrecision = aColumn.get_Precision();
    nScale = aColumn.get_NumericScale();
    nType = ADOS::MapADOType2Jdbc(aColumn.get_Type());
#endif

    OUString sTypeName;
    pColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)) >>= sTypeName;

    const OTypeInfoMap* pTypeInfoMap = m_pConnection->getTypeInfo();
    ::comphelper::UStringMixEqual aCase(false);
    // search for typeinfo where the typename is equal sTypeName
    OTypeInfoMap::const_iterator aFind = std::find_if(pTypeInfoMap->begin(), pTypeInfoMap->end(),
        [&aCase, &sTypeName] (const OTypeInfoMap::value_type& typeInfo) {
            return aCase(typeInfo.second->getDBName(), sTypeName);
        });

    if ( aFind != pTypeInfoMap->end() ) // change column type if necessary
        aColumn.put_Type(aFind->first);

    if ( SUCCEEDED(static_cast<ADOColumns*>(m_aCollection)->Append(OLEVariant(aColumn.get_Name()),aColumn.get_Type(),aColumn.get_DefinedSize())) )
    {
        WpADOColumn aAddedColumn = m_aCollection.GetItem(OLEVariant(aColumn.get_Name()));
        if ( aAddedColumn.IsValid() )
        {
            bool bAutoIncrement = false;
            pColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;
            if ( bAutoIncrement )
                OTools::putValue( aAddedColumn.get_Properties(), OUString("Autoincrement"), bAutoIncrement );

            if ( aFind != pTypeInfoMap->end() &&  aColumn.get_Type() != aAddedColumn.get_Type() ) // change column type if necessary
                aColumn.put_Type(aFind->first);
            aAddedColumn.put_Precision(aColumn.get_Precision());
            aAddedColumn.put_NumericScale(aColumn.get_NumericScale());
            aAddedColumn.put_Attributes(aColumn.get_Attributes());
            aAddedColumn.put_SortOrder(aColumn.get_SortOrder());
            aAddedColumn.put_RelatedColumn(aColumn.get_RelatedColumn());
        }
    }
    ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));

    return new OAdoColumn(isCaseSensitive(),m_pConnection,pColumn->getColumnImpl());
}

// XDrop
void OColumns::dropObject(sal_Int32 /*_nPos*/,const OUString& _sElementName)
{
    if(!m_aCollection.Delete(_sElementName))
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
