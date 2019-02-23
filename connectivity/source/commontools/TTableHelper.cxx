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

#include <sal/config.h>
#include <sal/log.hxx>

#include <connectivity/TTableHelper.hxx>
#include <com/sun/star/sdb/tools/XTableRename.hpp>
#include <com/sun/star/sdb/tools/XTableAlteration.hpp>
#include <com/sun/star/sdb/tools/XKeyAlteration.hpp>
#include <com/sun/star/sdb/tools/XIndexAlteration.hpp>
#include <connectivity/sdbcx/VKey.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <TConnection.hxx>

#include <o3tl/functional.hxx>

#include <iterator>
#include <set>

using namespace ::comphelper;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace
{
    /// helper class for column property change events which holds the OComponentDefinition weak
class OTableContainerListener:
    public ::cppu::WeakImplHelper< XContainerListener >
{
    OTableHelper* m_pComponent;
    std::map< OUString,bool> m_aRefNames;

protected:
    virtual ~OTableContainerListener() override {}
public:
    explicit OTableContainerListener(OTableHelper* _pComponent) : m_pComponent(_pComponent){}
    // noncopyable
    OTableContainerListener(const OTableContainerListener&) = delete;
    const OTableContainerListener& operator=(const OTableContainerListener&) = delete;
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& /*Event*/ ) override
    {
    }
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override
    {
        OUString sName;
        Event.Accessor  >>= sName;
        if ( m_aRefNames.find(sName) != m_aRefNames.end() )
            m_pComponent->refreshKeys();
    }
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override
    {
        OUString sOldComposedName,sNewComposedName;
        Event.ReplacedElement   >>= sOldComposedName;
        Event.Accessor          >>= sNewComposedName;
        if ( sOldComposedName != sNewComposedName && m_aRefNames.find(sOldComposedName) != m_aRefNames.end() )
            m_pComponent->refreshKeys();
    }
    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& /*_rSource*/ ) override
    {
    }
    void clear() { m_pComponent = nullptr; }
    void add(const OUString& _sRefName) { m_aRefNames.emplace(_sRefName,true); }
};
}
namespace connectivity
{
    static OUString lcl_getServiceNameForSetting(const Reference< css::sdbc::XConnection >& _xConnection,const OUString& i_sSetting)
    {
        OUString sSupportService;
        Any aValue;
        if ( ::dbtools::getDataSourceSetting(_xConnection,i_sSetting,aValue) )
        {
            aValue >>= sSupportService;
        }
        return sSupportService;
    }
    struct OTableHelperImpl
    {
        TKeyMap  m_aKeys;
        // helper services which can be provided by extensions
        Reference< css::sdb::tools::XTableRename>      m_xRename;
        Reference< css::sdb::tools::XTableAlteration>  m_xAlter;
        Reference< css::sdb::tools::XKeyAlteration>    m_xKeyAlter;
        Reference< css::sdb::tools::XIndexAlteration>  m_xIndexAlter;

        Reference< css::sdbc::XDatabaseMetaData >      m_xMetaData;
        Reference< css::sdbc::XConnection >            m_xConnection;
        rtl::Reference<OTableContainerListener>        m_xTablePropertyListener;
        std::vector< ColumnDesc >                    m_aColumnDesc;
        explicit OTableHelperImpl(const Reference< css::sdbc::XConnection >& _xConnection)
            : m_xConnection(_xConnection)
        {
            try
            {
                m_xMetaData = m_xConnection->getMetaData();
                Reference<XMultiServiceFactory> xFac(_xConnection,UNO_QUERY);
                if ( xFac.is() )
                {
                    m_xRename.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,"TableRenameServiceName")),UNO_QUERY);
                    m_xAlter.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,"TableAlterationServiceName")),UNO_QUERY);
                    m_xKeyAlter.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,"KeyAlterationServiceName")),UNO_QUERY);
                    m_xIndexAlter.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,"IndexAlterationServiceName")),UNO_QUERY);
                }
            }
            catch(const Exception&)
            {
            }
        }
    };
}

OTableHelper::OTableHelper( sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection,
                           bool _bCase)
    :OTable_TYPEDEF(_pTables,_bCase)
    ,m_pImpl(new OTableHelperImpl(_xConnection))
{
}

OTableHelper::OTableHelper( sdbcx::OCollection* _pTables,
                            const Reference< XConnection >& _xConnection,
                            bool _bCase,
                            const OUString& Name,
                            const OUString& Type,
                            const OUString& Description ,
                            const OUString& SchemaName,
                            const OUString& CatalogName
                        ) : OTable_TYPEDEF(_pTables,
                                            _bCase,
                                            Name,
                                          Type,
                                          Description,
                                          SchemaName,
                                          CatalogName)
                        ,m_pImpl(new OTableHelperImpl(_xConnection))
{
}

OTableHelper::~OTableHelper()
{
}

void SAL_CALL OTableHelper::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_pImpl->m_xTablePropertyListener.is() )
    {
        m_pTables->removeContainerListener(m_pImpl->m_xTablePropertyListener.get());
        m_pImpl->m_xTablePropertyListener->clear();
        m_pImpl->m_xTablePropertyListener.clear();
    }
    OTable_TYPEDEF::disposing();

    m_pImpl->m_xConnection  = nullptr;
    m_pImpl->m_xMetaData    = nullptr;

}


namespace
{
    /** collects ColumnDesc's from a resultset produced by XDatabaseMetaData::getColumns
    */
    void lcl_collectColumnDescs_throw( const Reference< XResultSet >& _rxResult, std::vector< ColumnDesc >& _out_rColumns )
    {
        Reference< XRow > xRow( _rxResult, UNO_QUERY_THROW );
        OUString sName;
        OrdinalPosition nOrdinalPosition( 0 );
        while ( _rxResult->next() )
        {
            sName = xRow->getString( 4 );           // COLUMN_NAME
            sal_Int32       nField5 = xRow->getInt(5);
            OUString aField6 = xRow->getString(6);
            sal_Int32       nField7 = xRow->getInt(7)
                        ,   nField9 = xRow->getInt(9)
                        ,   nField11= xRow->getInt(11);
            OUString  sField12 = xRow->getString(12)
                            ,sField13 = xRow->getString(13);
            nOrdinalPosition = xRow->getInt( 17 );  // ORDINAL_POSITION
            _out_rColumns.push_back( ColumnDesc( sName,nField5,aField6,nField7,nField9,nField11,sField12,sField13, nOrdinalPosition ) );
        }
    }

    /** checks a given array of ColumnDesc's whether it has reasonable ordinal positions. If not,
        they will be normalized to be the array index.
    */
    void lcl_sanitizeColumnDescs( std::vector< ColumnDesc >& _rColumns )
    {
        if ( _rColumns.empty() )
            return;

        // collect all used ordinals
        std::set< OrdinalPosition > aUsedOrdinals;
        for ( const auto& collect : _rColumns )
            aUsedOrdinals.insert( collect.nOrdinalPosition );

        // we need to have as much different ordinals as we have different columns
        bool bDuplicates = aUsedOrdinals.size() != _rColumns.size();
        // and it needs to be a continuous range
        size_t nOrdinalsRange = *aUsedOrdinals.rbegin() - *aUsedOrdinals.begin() + 1;
        bool bGaps = nOrdinalsRange != _rColumns.size();

        // if that's not the case, normalize it
        if ( bGaps || bDuplicates )
        {
            OSL_FAIL( "lcl_sanitizeColumnDescs: database did provide invalid ORDINAL_POSITION values!" );

            OrdinalPosition nNormalizedPosition = 1;
            for ( auto& normalize : _rColumns )
                normalize.nOrdinalPosition = nNormalizedPosition++;
            return;
        }

        // what's left is that the range might not be from 1 to <column count>, but for instance
        // 0 to <column count>-1.
        size_t nOffset = *aUsedOrdinals.begin() - 1;
        for ( auto& offset : _rColumns )
            offset.nOrdinalPosition -= nOffset;
    }
}


void OTableHelper::refreshColumns()
{
    ::std::vector< OUString> aVector;
    if(!isNew())
    {
        Any aCatalog;
        if ( !m_CatalogName.isEmpty() )
            aCatalog <<= m_CatalogName;

        ::utl::SharedUNOComponent< XResultSet > xResult( getMetaData()->getColumns(
            aCatalog,
            m_SchemaName,
            m_Name,
            "%"
        ) );

        // collect the column names, together with their ordinal position
        m_pImpl->m_aColumnDesc.clear();
        lcl_collectColumnDescs_throw( xResult, m_pImpl->m_aColumnDesc );

        // ensure that the ordinal positions as obtained from the meta data do make sense
        lcl_sanitizeColumnDescs( m_pImpl->m_aColumnDesc );

        // sort by ordinal position
        std::map< OrdinalPosition, OUString > aSortedColumns;
        for (const auto& copy : m_pImpl->m_aColumnDesc)
            aSortedColumns[ copy.nOrdinalPosition ] = copy.sName;

        // copy them to aVector, now that we have the proper ordering
        std::transform(
            aSortedColumns.begin(),
            aSortedColumns.end(),
            std::insert_iterator< ::std::vector< OUString> >( aVector, aVector.begin() ),
            ::o3tl::select2nd< std::map< OrdinalPosition, OUString >::value_type >()
            );
    }

    if(m_xColumns)
        m_xColumns->reFill(aVector);
    else
        m_xColumns = createColumns(aVector);
}

const ColumnDesc* OTableHelper::getColumnDescription(const OUString& _sName) const
{
    const ColumnDesc* pRet = nullptr;
    auto aIter = std::find_if(m_pImpl->m_aColumnDesc.begin(), m_pImpl->m_aColumnDesc.end(),
        [&_sName](const ColumnDesc& rColumnDesc) { return rColumnDesc.sName == _sName; });
    if (aIter != m_pImpl->m_aColumnDesc.end())
        pRet = &*aIter;
    return pRet;
}

void OTableHelper::refreshPrimaryKeys(::std::vector< OUString>& _rNames)
{
    Any aCatalog;
    if ( !m_CatalogName.isEmpty() )
        aCatalog <<= m_CatalogName;
    Reference< XResultSet > xResult = getMetaData()->getPrimaryKeys(aCatalog,m_SchemaName,m_Name);

    if ( xResult.is() )
    {
        std::shared_ptr<sdbcx::KeyProperties> pKeyProps(new sdbcx::KeyProperties(OUString(),KeyType::PRIMARY,0,0));
        OUString aPkName;
        bool bAlreadyFetched = false;
        const Reference< XRow > xRow(xResult,UNO_QUERY);
        while ( xResult->next() )
        {
            pKeyProps->m_aKeyColumnNames.push_back(xRow->getString(4));
            if ( !bAlreadyFetched )
            {
                aPkName = xRow->getString(6);
                SAL_WARN_IF(xRow->wasNull(),"connectivity.commontools", "NULL Primary Key name");
                SAL_WARN_IF(aPkName.isEmpty(),"connectivity.commontools", "empty Primary Key name");
                bAlreadyFetched = true;
            }
        }

        if(bAlreadyFetched)
        {
            SAL_WARN_IF(aPkName.isEmpty(),"connectivity.commontools", "empty Primary Key name");
            SAL_WARN_IF(pKeyProps->m_aKeyColumnNames.empty(),"connectivity.commontools", "Primary Key has no columns");
            m_pImpl->m_aKeys.emplace(aPkName,pKeyProps);
            _rNames.push_back(aPkName);
        }
    } // if ( xResult.is() && xResult->next() )
    ::comphelper::disposeComponent(xResult);
}

void OTableHelper::refreshForeignKeys(::std::vector< OUString>& _rNames)
{
    Any aCatalog;
    if ( !m_CatalogName.isEmpty() )
        aCatalog <<= m_CatalogName;
    Reference< XResultSet > xResult = getMetaData()->getImportedKeys(aCatalog,m_SchemaName,m_Name);
    Reference< XRow > xRow(xResult,UNO_QUERY);

    if ( xRow.is() )
    {
        std::shared_ptr<sdbcx::KeyProperties> pKeyProps;
        OUString aName,sCatalog,aSchema,sOldFKName;
        while( xResult->next() )
        {
            // this must be outside the "if" because we have to call in a right order
            sCatalog    = xRow->getString(1);
            if ( xRow->wasNull() )
                sCatalog.clear();
            aSchema     = xRow->getString(2);
            aName       = xRow->getString(3);

            const OUString sForeignKeyColumn = xRow->getString(8);
            const sal_Int32 nUpdateRule = xRow->getInt(10);
            const sal_Int32 nDeleteRule = xRow->getInt(11);
            const OUString sFkName = xRow->getString(12);

            if ( !sFkName.isEmpty() && !xRow->wasNull() )
            {
                if ( sOldFKName != sFkName )
                {
                    if ( pKeyProps.get() )
                        m_pImpl->m_aKeys.emplace(sOldFKName,pKeyProps);

                    const OUString sReferencedName = ::dbtools::composeTableName(getMetaData(),sCatalog,aSchema,aName,false,::dbtools::EComposeRule::InDataManipulation);
                    pKeyProps.reset(new sdbcx::KeyProperties(sReferencedName,KeyType::FOREIGN,nUpdateRule,nDeleteRule));
                    pKeyProps->m_aKeyColumnNames.push_back(sForeignKeyColumn);
                    _rNames.push_back(sFkName);
                    if ( m_pTables->hasByName(sReferencedName) )
                    {
                        if ( !m_pImpl->m_xTablePropertyListener.is() )
                            m_pImpl->m_xTablePropertyListener = new OTableContainerListener(this);
                        m_pTables->addContainerListener(m_pImpl->m_xTablePropertyListener.get());
                        m_pImpl->m_xTablePropertyListener->add(sReferencedName);
                    } // if ( m_pTables->hasByName(sReferencedName) )
                    sOldFKName = sFkName;
                } // if ( sOldFKName != sFkName )
                else if ( pKeyProps.get() )
                {
                    pKeyProps->m_aKeyColumnNames.push_back(sForeignKeyColumn);
                }
            }
        } // while( xResult->next() )
        if ( pKeyProps.get() )
            m_pImpl->m_aKeys.emplace(sOldFKName,pKeyProps);
        ::comphelper::disposeComponent(xResult);
    }
}

void OTableHelper::refreshKeys()
{
    m_pImpl->m_aKeys.clear();

    ::std::vector< OUString> aNames;

    if(!isNew())
    {
        refreshPrimaryKeys(aNames);
        refreshForeignKeys(aNames);
        m_xKeys = createKeys(aNames);
    } // if(!isNew())
    else if (!m_xKeys )
        m_xKeys = createKeys(aNames);
    /*if(m_pKeys)
        m_pKeys->reFill(aVector);
    else*/

}

void OTableHelper::refreshIndexes()
{
    ::std::vector< OUString> aVector;
    if(!isNew())
    {
        // fill indexes
        Any aCatalog;
        if ( !m_CatalogName.isEmpty() )
            aCatalog <<= m_CatalogName;
        Reference< XResultSet > xResult = getMetaData()->getIndexInfo(aCatalog,m_SchemaName,m_Name,false,false);

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            OUString sCatalogSep = getMetaData()->getCatalogSeparator();
            OUString sPreviousRoundName;
            while( xResult->next() )
            {
                OUString aName = xRow->getString(5);
                if(!aName.isEmpty())
                    aName += sCatalogSep;
                aName += xRow->getString(6);
                if ( !aName.isEmpty() )
                {
                    // don't insert the name if the last one we inserted was the same
                    if (sPreviousRoundName != aName)
                        aVector.push_back(aName);
                }
                sPreviousRoundName = aName;
            }
            ::comphelper::disposeComponent(xResult);
        }
    }

    if(m_xIndexes)
        m_xIndexes->reFill(aVector);
    else
        m_xIndexes = createIndexes(aVector);
}

OUString OTableHelper::getRenameStart() const
{
    OUString sSql("RENAME ");
    if ( m_Type == "VIEW" )
        sSql += " VIEW ";
    else
        sSql += " TABLE ";

    return sSql;
}

// XRename
void SAL_CALL OTableHelper::rename( const OUString& newName )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef __GNUC__
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    if(!isNew())
    {
        if ( m_pImpl->m_xRename.is() )
        {
            m_pImpl->m_xRename->rename(this,newName);
        }
        else
        {
            OUString sSql = getRenameStart();

            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(getMetaData(),newName,sCatalog,sSchema,sTable,::dbtools::EComposeRule::InDataManipulation);

            OUString sComposedName;
            sComposedName = ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,true,::dbtools::EComposeRule::InDataManipulation);
            sSql += sComposedName
                 + " TO ";
            sComposedName = ::dbtools::composeTableName(getMetaData(),sCatalog,sSchema,sTable,true,::dbtools::EComposeRule::InDataManipulation);
            sSql += sComposedName;

            Reference< XStatement > xStmt = m_pImpl->m_xConnection->createStatement(  );
            if ( xStmt.is() )
            {
                xStmt->execute(sSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }

        OTable_TYPEDEF::rename(newName);
    }
    else
        ::dbtools::qualifiedNameComponents(getMetaData(),newName,m_CatalogName,m_SchemaName,m_Name,::dbtools::EComposeRule::InTableDefinitions);
}

Reference< XDatabaseMetaData> OTableHelper::getMetaData() const
{
    return m_pImpl->m_xMetaData;
}

void SAL_CALL OTableHelper::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef __GNUC__
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    Reference< XPropertySet > xOld(
        m_xColumns->getByIndex(index), css::uno::UNO_QUERY);
    if(xOld.is())
        alterColumnByName(getString(xOld->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),descriptor);
}


OUString SAL_CALL OTableHelper::getName()
{
    OUString sComposedName;
    sComposedName = ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,false,::dbtools::EComposeRule::InDataManipulation);
    return sComposedName;
}

std::shared_ptr<sdbcx::KeyProperties> OTableHelper::getKeyProperties(const OUString& _sName) const
{
    std::shared_ptr<sdbcx::KeyProperties> pKeyProps;
    TKeyMap::const_iterator aFind = m_pImpl->m_aKeys.find(_sName);
    if ( aFind != m_pImpl->m_aKeys.end() )
    {
        pKeyProps = aFind->second;
    }
    else // only a fall back
    {
        OSL_FAIL("No key with the given name found");
        pKeyProps.reset(new sdbcx::KeyProperties());
    }

    return pKeyProps;
}

void OTableHelper::addKey(const OUString& _sName,const std::shared_ptr<sdbcx::KeyProperties>& _aKeyProperties)
{
    m_pImpl->m_aKeys.emplace(_sName,_aKeyProperties);
}

OUString OTableHelper::getTypeCreatePattern() const
{
    return OUString();
}

Reference< XConnection> const & OTableHelper::getConnection() const
{
    return m_pImpl->m_xConnection;
}

Reference< css::sdb::tools::XTableRename> const & OTableHelper::getRenameService() const
{
    return m_pImpl->m_xRename;
}

Reference< css::sdb::tools::XTableAlteration> const & OTableHelper::getAlterService() const
{
    return m_pImpl->m_xAlter;
}

Reference< css::sdb::tools::XKeyAlteration> const & OTableHelper::getKeyService() const
{
    return m_pImpl->m_xKeyAlter;
}

Reference< css::sdb::tools::XIndexAlteration> const & OTableHelper::getIndexService() const
{
    return m_pImpl->m_xIndexAlter;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
