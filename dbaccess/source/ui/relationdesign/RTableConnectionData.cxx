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

#include "RTableConnectionData.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "dbustrings.hrc"
#include "dbu_rel.hrc"
#include "UITools.hxx"
#include "moduledbu.hxx"
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>

using namespace dbaui;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

DBG_NAME(ORelationTableConnectionData)
// class ORelationTableConnectionData
ORelationTableConnectionData::ORelationTableConnectionData()
    :OTableConnectionData()
    ,m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(CARDINAL_UNDEFINED)
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
}

ORelationTableConnectionData::ORelationTableConnectionData( const TTableWindowData::value_type& _pReferencingTable,
                                                            const TTableWindowData::value_type& _pReferencedTable,
                                                            const OUString& rConnName )
    :OTableConnectionData( _pReferencingTable, _pReferencedTable )
    ,m_nUpdateRules(KeyRule::NO_ACTION)
    ,m_nDeleteRules(KeyRule::NO_ACTION)
    ,m_nCardinality(CARDINAL_UNDEFINED)
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
    m_aConnName = rConnName;

    if ( m_aConnName.Len() )
        SetCardinality();
}

ORelationTableConnectionData::ORelationTableConnectionData( const ORelationTableConnectionData& rConnData )
    :OTableConnectionData( rConnData )
{
    DBG_CTOR(ORelationTableConnectionData,NULL);
    *this = rConnData;
}

ORelationTableConnectionData::~ORelationTableConnectionData()
{
    DBG_DTOR(ORelationTableConnectionData,NULL);
}

sal_Bool ORelationTableConnectionData::DropRelation()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    ::osl::MutexGuard aGuard( m_aMutex );
    // Relation loeschen
    Reference< XIndexAccess> xKeys = getReferencingTable()->getKeys();
    if( m_aConnName.Len() && xKeys.is() )
    {
        const sal_Int32 nCount = xKeys->getCount();
        for(sal_Int32 i = 0;i < nCount;++i)
        {
            Reference< XPropertySet> xKey(xKeys->getByIndex(i),UNO_QUERY);
            OSL_ENSURE(xKey.is(),"Key is not valid!");
            if(xKey.is())
            {
                OUString sName;
                xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
                if(String(sName) == m_aConnName)
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
    return sal_True;
}

void ORelationTableConnectionData::ChangeOrientation()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    // Source- und DestFieldName der Linien austauschen
    OUString sTempString;
    OConnectionLineDataVec::iterator aIter = m_vConnLineData.begin();
    OConnectionLineDataVec::iterator aEnd = m_vConnLineData.end();
    for(;aIter != aEnd;++aIter)
    {
        sTempString = (*aIter)->GetSourceFieldName();
        (*aIter)->SetSourceFieldName( (*aIter)->GetDestFieldName() );
        (*aIter)->SetDestFieldName( sTempString );
    }

    // Member anpassen
    TTableWindowData::value_type pTemp = m_pReferencingTable;
    m_pReferencingTable = m_pReferencedTable;
    m_pReferencedTable = pTemp;
}

void ORelationTableConnectionData::SetCardinality()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    ::osl::MutexGuard aGuard( m_aMutex );
    m_nCardinality = CARDINAL_UNDEFINED;

    if( IsSourcePrimKey() )
    {
        if( IsDestPrimKey() )
            m_nCardinality = CARDINAL_ONE_ONE;
        else
            m_nCardinality = CARDINAL_ONE_MANY;
    }

    if( IsDestPrimKey() )
    {
        if( !IsSourcePrimKey() )
            m_nCardinality = CARDINAL_MANY_ONE;
    }

}

sal_Bool ORelationTableConnectionData::checkPrimaryKey(const Reference< XPropertySet>& i_xTable,EConnectionSide _eEConnectionSide) const
{
    // check if Table has the primary key column dependig on _eEConnectionSide
    sal_uInt16  nPrimKeysCount      = 0,
            nValidLinesCount    = 0;
    const Reference< XNameAccess> xKeyColumns = dbtools::getPrimaryKeyColumns_throw(i_xTable);
    if ( xKeyColumns.is() )
    {
        Sequence< OUString> aKeyColumns = xKeyColumns->getElementNames();
        const OUString* pKeyIter = aKeyColumns.getConstArray();
        const OUString* pKeyEnd  = pKeyIter + aKeyColumns.getLength();

        for(;pKeyIter != pKeyEnd;++pKeyIter)
        {
            OConnectionLineDataVec::const_iterator aIter = m_vConnLineData.begin();
            OConnectionLineDataVec::const_iterator aEnd = m_vConnLineData.end();
            for(;aIter != aEnd;++aIter)
            {
                ++nValidLinesCount;
                if ( (*aIter)->GetFieldName(_eEConnectionSide) == *pKeyIter )
                {
                    ++nPrimKeysCount;
                    break;
                }
            }
        }
        if ( nPrimKeysCount != aKeyColumns.getLength() )
            return sal_False;
    }
    if ( !nPrimKeysCount || nPrimKeysCount != nValidLinesCount )
        return sal_False;

    return sal_True;
}

sal_Bool ORelationTableConnectionData::IsConnectionPossible()
{
    DBG_CHKTHIS(ORelationTableConnectionData,NULL);
    ::osl::MutexGuard aGuard( m_aMutex );

    // Wenn die SourceFelder ein PrimKey sind, ist nur die Orientierung falsch
    if ( IsSourcePrimKey() && !IsDestPrimKey() )
        ChangeOrientation();

    return sal_True;
}

OConnectionLineDataRef ORelationTableConnectionData::CreateLineDataObj()
{
    return new OConnectionLineData();
}

OConnectionLineDataRef ORelationTableConnectionData::CreateLineDataObj( const OConnectionLineData& rConnLineData )
{
    return new OConnectionLineData( rConnLineData );
}

void ORelationTableConnectionData::CopyFrom(const OTableConnectionData& rSource)
{
    // wie in der Basisklasse zurueckziehen auf das (nicht-virtuelle) operator=
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
        std::vector< OConnectionLineDataRef >::const_iterator aIter = lhs.m_vConnLineData.begin();
        std::vector< OConnectionLineDataRef >::const_iterator aEnd = lhs.m_vConnLineData.end();
        for (sal_Int32 i = 0; aIter != aEnd; ++aIter,++i)
        {
            if ( *(rhs.m_vConnLineData[i]) != **aIter )
                break;
        }
        bEqual = aIter == aEnd;
    }
    return bEqual;
}

}

sal_Bool ORelationTableConnectionData::Update()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // Alte Relation loeschen
    {
        DropRelation();
        if( !IsConnectionPossible() )
            return sal_False;
    }

    // reassign the keys because the orientaion could be changed
    Reference<XPropertySet> xTableProp(getReferencingTable()->getTable());
    Reference< XIndexAccess> xKeys ( getReferencingTable()->getKeys());

    if ( !xKeys.is() )
        return sal_False;
    // Neue Relation erzeugen
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
        OUString sKeyName = sSourceName;
        sKeyName += getReferencedTable()->GetTableName();

        xKey->setPropertyValue(PROPERTY_NAME,makeAny(sKeyName));
        xKey->setPropertyValue(PROPERTY_TYPE,makeAny(KeyType::FOREIGN));
        xKey->setPropertyValue(PROPERTY_REFERENCEDTABLE,makeAny(OUString(getReferencedTable()->GetTableName())));
        xKey->setPropertyValue(PROPERTY_UPDATERULE, makeAny(GetUpdateRules()));
        xKey->setPropertyValue(PROPERTY_DELETERULE, makeAny(GetDeleteRules()));
    }

    Reference<XColumnsSupplier> xColSup(xKey,UNO_QUERY);
    if ( xColSup.is() )
    {
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);
        Reference<XAppend> xColumnAppend(xColumns,UNO_QUERY);
        if ( xColumnFactory.is() )
        {
            OConnectionLineDataVec::iterator aIter = m_vConnLineData.begin();
            OConnectionLineDataVec::iterator aEnd = m_vConnLineData.end();
            for(;aIter != aEnd;++aIter)
            {
                if(!((*aIter)->GetSourceFieldName().isEmpty() || (*aIter)->GetDestFieldName().isEmpty()))
                {
                    Reference<XPropertySet> xColumn;
                    xColumn = xColumnFactory->createDataDescriptor();
                    if ( xColumn.is() )
                    {
                        xColumn->setPropertyValue(PROPERTY_NAME,makeAny((*aIter)->GetSourceFieldName()));
                        xColumn->setPropertyValue(PROPERTY_RELATEDCOLUMN,makeAny((*aIter)->GetDestFieldName()));
                        xColumnAppend->appendByDescriptor(xColumn);
                    }
                }
            }

            if ( xColumns->hasElements() )
                xAppend->appendByDescriptor(xKey);
        }
        // to get the key we have to reget it because after append it is no longer valid
    }

    // get the name of foreign key // search for columns
    m_aConnName = OUString();
xKey.clear();
    bool bDropRelation = false;
    for(sal_Int32 i=0;i<xKeys->getCount();++i)
    {
        xKeys->getByIndex(i) >>= xKey;
        OSL_ENSURE(xKey.is(),"Key is not valid!");
        if ( xKey.is() )
        {
            sal_Int32 nType = 0;
            xKey->getPropertyValue(PROPERTY_TYPE) >>= nType;
            OUString sReferencedTable;
            xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
            if ( sReferencedTable == OUString(getReferencedTable()->GetTableName()) )
            {
                xColSup.set(xKey,UNO_QUERY_THROW);
                try
                {
                    Reference<XNameAccess> xColumns = xColSup->getColumns();
                    Sequence< OUString> aNames = xColumns->getElementNames();
                    const OUString* pIter = aNames.getConstArray();
                    const OUString* pEnd = pIter + aNames.getLength();

                    Reference<XPropertySet> xColumn;
                    OUString sName,sRelatedColumn;
                    for ( ; pIter != pEnd ; ++pIter )
                    {
                        xColumn.set(xColumns->getByName(*pIter),UNO_QUERY_THROW);
                        xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
                        xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN)   >>= sRelatedColumn;

                        OConnectionLineDataVec::iterator aIter = m_vConnLineData.begin();
                        OConnectionLineDataVec::iterator aEnd = m_vConnLineData.end();
                        for(;aIter != aEnd;++aIter)
                        {
                            if(    (*aIter)->GetSourceFieldName() == sName
                                && (*aIter)->GetDestFieldName() == sRelatedColumn )
                            {
                                break;
                            }
                        }
                        if ( aIter == m_vConnLineData.end() )
                            break;
                    }
                    if ( pIter == pEnd )
                    {
                        xKey->getPropertyValue(PROPERTY_NAME) >>= sName;
                        m_aConnName = sName;
                        bDropRelation = aNames.getLength() == 0; // the key contains no column, so it isn't valid and we have to drop it
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
        String sError(ModuleRes(STR_QUERY_REL_COULD_NOT_CREATE));
        ::dbtools::throwGenericSQLException(sError,NULL);
    }

    // The fields the relation marks may not be the same as our LineDatas mark after the relation has been updated
    if ( xColSup.is() )
    {
        OConnectionLineDataVec().swap(m_vConnLineData);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Sequence< OUString> aNames = xColumns->getElementNames();
        const OUString* pIter = aNames.getConstArray();
        const OUString* pEnd = pIter + aNames.getLength();

        m_vConnLineData.reserve( aNames.getLength() );
        Reference<XPropertySet> xColumn;
        OUString sName,sRelatedColumn;

        for(;pIter != pEnd;++pIter)
        {
            xColumns->getByName(*pIter) >>= xColumn;
            if ( xColumn.is() )
            {
                OConnectionLineDataRef pNewData = CreateLineDataObj();

                xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
                xColumn->getPropertyValue(PROPERTY_RELATEDCOLUMN)   >>= sRelatedColumn;

                pNewData->SetSourceFieldName(sName);
                pNewData->SetDestFieldName(sRelatedColumn);
                m_vConnLineData.push_back(pNewData);
            }
        }
    }
    // NOTE : the caller is responsible for updating any other objects referencing the old LineDatas (for instance a ConnLine)

    // Kardinalitaet bestimmen
    SetCardinality();

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
