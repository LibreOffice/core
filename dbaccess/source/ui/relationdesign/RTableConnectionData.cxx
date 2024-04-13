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

#include <RTableConnectionData.hxx>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <strings.hrc>
#include <strings.hxx>
#include <core_resource.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <osl/diagnose.h>

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

ORelationTableConnectionData::ORelationTableConnectionData()
    :m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(Cardinality::Undefined)
{
}

ORelationTableConnectionData::ORelationTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                                                            const TTableWindowData::value_type& _pReferencedTable,
                                                            const OUString& rConnName )
    :OTableConnectionData( _pReferencingTable, _pReferencedTable )
    ,m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(Cardinality::Undefined)
{
    m_aConnName = rConnName;

    if ( !m_aConnName.isEmpty() )
        SetCardinality();
}

ORelationTableConnectionData::ORelationTableConnectionData( const ORelationTableConnectionData& rConnData )
    :OTableConnectionData( rConnData )
{
    *this = rConnData;
}

ORelationTableConnectionData::~ORelationTableConnectionData()
{
}

void ORelationTableConnectionData::DropRelation()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // delete relation
    Reference< XIndexAccess> xKeys = getReferencingTable()->getKeys();
    if( m_aConnName.isEmpty() || !xKeys.is() )
        return;

    const sal_Int32 nCount = xKeys->getCount();
    for(sal_Int32 i = 0;i < nCount;++i)
    {
        Reference< XPropertySet> xKey(xKeys->getByIndex(i),UNO_QUERY);
        OSL_ENSURE(xKey.is(),"Key is not valid!");
        if(xKey.is())
        {
            OUString sName;
            xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
            if(sName == m_aConnName)
            {
                Reference< XDrop> xDrop(xKeys,UNO_QUERY);
                OSL_ENSURE(xDrop.is(),"can't drop key because we haven't a drop interface!");
                if(xDrop.is())
                    xDrop->dropByIndex(i);
                break;
            }
        }
    }
}

void ORelationTableConnectionData::ChangeOrientation()
{
    // exchange Source- and DestFieldName of the lines
    OUString sTempString;
    for (auto const& elem : m_vConnLineData)
    {
        sTempString = elem->GetSourceFieldName();
        elem->SetSourceFieldName( elem->GetDestFieldName() );
        elem->SetDestFieldName( sTempString );
    }

    // adapt member
    std::swap( m_pReferencingTable, m_pReferencedTable );
}

void ORelationTableConnectionData::SetCardinality()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_nCardinality = Cardinality::Undefined;

    if( IsSourcePrimKey() )
    {
        if( IsDestPrimKey() )
            m_nCardinality = Cardinality::OneOne;
        else
            m_nCardinality = Cardinality::OneMany;
    }

    if( IsDestPrimKey() )
    {
        if( !IsSourcePrimKey() )
            m_nCardinality = Cardinality::ManyOne;
    }

}

bool ORelationTableConnectionData::checkPrimaryKey(const Reference< XPropertySet>& i_xTable,EConnectionSide _eEConnectionSide) const
{
    // check if Table has the primary key column depending on _eEConnectionSide
    sal_uInt16  nPrimKeysCount      = 0,
            nValidLinesCount    = 0;
    const Reference< XNameAccess> xKeyColumns = dbtools::getPrimaryKeyColumns_throw(i_xTable);
    if ( xKeyColumns.is() )
    {
        Sequence< OUString> aKeyColumns = xKeyColumns->getElementNames();

        for (auto& keyColumn : aKeyColumns)
        {
            for (auto const& elem : m_vConnLineData)
            {
                ++nValidLinesCount;
                if (elem->GetFieldName(_eEConnectionSide) == keyColumn)
                {
                    ++nPrimKeysCount;
                    break;
                }
            }
        }
        if ( nPrimKeysCount != aKeyColumns.getLength() )
            return false;
    }
    return nPrimKeysCount && nPrimKeysCount == nValidLinesCount;
}

void ORelationTableConnectionData::IsConnectionPossible()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // if the SourceFields are a PrimKey, it's only the orientation which is wrong
    if ( IsSourcePrimKey() && !IsDestPrimKey() )
        ChangeOrientation();
}

void ORelationTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    // retract to the (non-virtual) operator= like in the base class
    *this = *static_cast<const ORelationTableConnectionData*>(&rSource);
}

ORelationTableConnectionData& ORelationTableConnectionData::operator=( const ORelationTableConnectionData& rConnData )
{
    if (&rConnData == this)
        return *this;

    OTableConnectionData::operator=( rConnData );
    m_nUpdateRules = rConnData.GetUpdateRules();
    m_nDeleteRules = rConnData.GetDeleteRules();
    m_nCardinality = rConnData.GetCardinality();

    return *this;
}

namespace dbaui
{
bool operator==(const ORelationTableConnectionData& lhs, const ORelationTableConnectionData& rhs)
{
    bool bEqual = (lhs.m_nUpdateRules == rhs.m_nUpdateRules)
        && (lhs.m_nDeleteRules == rhs.m_nDeleteRules)
        && (lhs.m_nCardinality == rhs.m_nCardinality)
        && (lhs.getReferencingTable() == rhs.getReferencingTable())
        && (lhs.getReferencedTable() == rhs.getReferencedTable())
        && (lhs.m_aConnName == rhs.m_aConnName)
        && (lhs.m_vConnLineData.size() == rhs.m_vConnLineData.size());

    if ( bEqual )
    {
        sal_Int32 i = 0;
        for (auto const& elem : lhs.m_vConnLineData)
        {
            if ( *(rhs.m_vConnLineData[i]) != *elem )
            {
                bEqual = false;
                break;
            }
            ++i;
        }
    }
    return bEqual;
}

}

bool ORelationTableConnectionData::Update()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // delete old relation
    {
        DropRelation();
        IsConnectionPossible();
    }

    // reassign the keys because the orientation could be changed
    Reference<XPropertySet> xTableProp(getReferencingTable()->getTable());
    Reference< XIndexAccess> xKeys ( getReferencingTable()->getKeys());

    if ( !xKeys.is() )
        return false;
    // create new relation
    Reference<XDataDescriptorFactory> xKeyFactory(xKeys,UNO_QUERY);
    OSL_ENSURE(xKeyFactory.is(),"No XDataDescriptorFactory Interface!");
    Reference<XAppend> xAppend(xKeyFactory,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    Reference<XPropertySet> xKey = xKeyFactory->createDataDescriptor();
    OSL_ENSURE(xKey.is(),"Key is null!");
    if ( xKey.is() && xTableProp.is() )
    {
        // build a foreign key name
        OUString sSourceName;
        xTableProp->getPropertyValue(PROPERTY_NAME) >>= sSourceName;
        OUString sKeyName = sSourceName + getReferencedTable()->GetTableName();

        xKey->setPropertyValue(PROPERTY_NAME,Any(sKeyName));
        xKey->setPropertyValue(PROPERTY_TYPE,Any(KeyType::FOREIGN));
        // get the full name of the tables to ensure uniqueness across catalogs and schema
        xKey->setPropertyValue(PROPERTY_REFERENCEDTABLE,Any(getReferencedTable()->GetComposedName()));
        xKey->setPropertyValue(PROPERTY_UPDATERULE, Any(GetUpdateRules()));
        xKey->setPropertyValue(PROPERTY_DELETERULE, Any(GetDeleteRules()));
    }

    Reference<XColumnsSupplier> xColSup(xKey,UNO_QUERY);
    if ( xColSup.is() )
    {
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);
        Reference<XAppend> xColumnAppend(xColumns,UNO_QUERY);
        if ( xColumnFactory.is() )
        {
            for (auto const& elem : m_vConnLineData)
            {
                if(!(elem->GetSourceFieldName().isEmpty() || elem->GetDestFieldName().isEmpty()))
                {
                    Reference<XPropertySet> xColumn = xColumnFactory->createDataDescriptor();
                    if ( xColumn.is() )
                    {
                        xColumn->setPropertyValue(PROPERTY_NAME,Any(elem->GetSourceFieldName()));
                        xColumn->setPropertyValue(PROPERTY_RELATEDCOLUMN,Any(elem->GetDestFieldName()));
                        xColumnAppend->appendByDescriptor(xColumn);
                    }
                }
            }

            if ( xColumns->hasElements() )
                xAppend->appendByDescriptor(xKey);
        }
        // to get the key we have to reget it because after append it is no longer valid
    }

    // get the name of foreign key; search for columns
    m_aConnName.clear();
    xKey.clear();
    bool bDropRelation = false;
    for(sal_Int32 i=0;i<xKeys->getCount();++i)
    {
        xKeys->getByIndex(i) >>= xKey;
        OSL_ENSURE(xKey.is(),"Key is not valid!");
        if ( xKey.is() )
        {
            OUString sReferencedTable;
            xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
            if ( sReferencedTable == getReferencedTable()->GetComposedName() )
            {
                xColSup.set(xKey,UNO_QUERY_THROW);
                try
                {
                    Reference<XNameAccess> xColumns = xColSup->getColumns();
                    Sequence< OUString> aNames = xColumns->getElementNames();
                    const OUString* pIter = aNames.begin();
                    const OUString* pEnd = aNames.end();

                    OUString sName,sRelatedColumn;
                    for ( ; pIter != pEnd ; ++pIter )
                    {
                        Reference<XPropertySet> xColumn(xColumns->getByName(*pIter),UNO_QUERY_THROW);
                        xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
                        xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN)   >>= sRelatedColumn;

                        if (std::none_of(m_vConnLineData.begin(), m_vConnLineData.end(),
                                         [&sName, &sRelatedColumn](auto& elem) {
                                             return elem->GetSourceFieldName() == sName
                                                    && elem->GetDestFieldName() == sRelatedColumn;
                                         }))
                            break;
                    }
                    if ( pIter == pEnd )
                    {
                        xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
                        m_aConnName = sName;
                        bDropRelation = !aNames.hasElements(); // the key contains no column, so it isn't valid and we have to drop it
                        //here we already know our column structure so we don't have to recreate the table connection data
                        xColSup.clear();
                        break;
                    }
                }
                catch(Exception&)
                {
                }
            }
        }
        xKey.clear();
    }
    if ( bDropRelation )
    {
        DropRelation();
        OUString sError(DBA_RES(STR_QUERY_REL_COULD_NOT_CREATE));
        ::dbtools::throwGenericSQLException(sError,nullptr);
    }

    // The fields the relation marks may not be the same as our LineDatas mark after the relation has been updated
    if ( xColSup.is() )
    {
        OConnectionLineDataVec().swap(m_vConnLineData);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Sequence< OUString> aNames = xColumns->getElementNames();

        m_vConnLineData.reserve( aNames.getLength() );
        Reference<XPropertySet> xColumn;
        OUString sName,sRelatedColumn;

        for (auto& colName : aNames)
        {
            xColumns->getByName(colName) >>= xColumn;
            if ( xColumn.is() )
            {
                OConnectionLineDataRef pNewData = new OConnectionLineData();

                xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
                xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN)   >>= sRelatedColumn;

                pNewData->SetSourceFieldName(sName);
                pNewData->SetDestFieldName(sRelatedColumn);
                m_vConnLineData.push_back(pNewData);
            }
        }
    }
    // NOTE : the caller is responsible for updating any other objects referencing the old LineDatas (for instance a ConnLine)

    // determine cardinality
    SetCardinality();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
