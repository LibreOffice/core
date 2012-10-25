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

#include "connectivity/TTableHelper.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/implementationreference.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/sdbcx/VCollection.hxx"
#include <unotools/sharedunocomponent.hxx>
#include "TConnection.hxx"

#include <o3tl/compat_functional.hxx>

#include <iterator>

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
typedef ::cppu::WeakImplHelper1 < XContainerListener > OTableContainerListener_BASE;
class OTableContainerListener : public OTableContainerListener_BASE
{
    OTableHelper* m_pComponent;
    ::std::map< ::rtl::OUString,bool> m_aRefNames;

    OTableContainerListener(const OTableContainerListener&);
    void operator =(const OTableContainerListener&);
protected:
    virtual ~OTableContainerListener(){}
public:
    OTableContainerListener(OTableHelper* _pComponent) : m_pComponent(_pComponent){}
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& /*Event*/ ) throw (RuntimeException)
    {
    }
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (RuntimeException)
    {
        ::rtl::OUString sName;
        Event.Accessor  >>= sName;
        if ( m_aRefNames.find(sName) != m_aRefNames.end() )
            m_pComponent->refreshKeys();
    }
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (RuntimeException)
    {
        ::rtl::OUString sOldComposedName,sNewComposedName;
        Event.ReplacedElement   >>= sOldComposedName;
        Event.Accessor          >>= sNewComposedName;
        if ( sOldComposedName != sNewComposedName && m_aRefNames.find(sOldComposedName) != m_aRefNames.end() )
            m_pComponent->refreshKeys();
    }
    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& /*_rSource*/ ) throw (RuntimeException)
    {
    }
    void clear() { m_pComponent = NULL; }
    inline void add(const ::rtl::OUString& _sRefName) { m_aRefNames.insert(::std::map< ::rtl::OUString,bool>::value_type(_sRefName,true)); }
};
}
namespace connectivity
{
    ::rtl::OUString lcl_getServiceNameForSetting(const Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,const ::rtl::OUString& i_sSetting)
    {
        ::rtl::OUString sSupportService;
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
        Reference< ::com::sun::star::sdb::tools::XTableRename>      m_xRename;
        Reference< ::com::sun::star::sdb::tools::XTableAlteration>  m_xAlter;
        Reference< ::com::sun::star::sdb::tools::XKeyAlteration>    m_xKeyAlter;
        Reference< ::com::sun::star::sdb::tools::XIndexAlteration>  m_xIndexAlter;

        Reference< ::com::sun::star::sdbc::XDatabaseMetaData >      m_xMetaData;
        Reference< ::com::sun::star::sdbc::XConnection >            m_xConnection;
        ::comphelper::ImplementationReference< OTableContainerListener,XContainerListener>
                                    m_xTablePropertyListener;
        ::std::vector< ColumnDesc > m_aColumnDesc;
        OTableHelperImpl(const Reference< ::com::sun::star::sdbc::XConnection >& _xConnection)
            : m_xConnection(_xConnection)
        {
            try
            {
                m_xMetaData = m_xConnection->getMetaData();
                Reference<XMultiServiceFactory> xFac(_xConnection,UNO_QUERY);
                if ( xFac.is() )
                {
                    static const ::rtl::OUString s_sTableRename(RTL_CONSTASCII_USTRINGPARAM("TableRenameServiceName"));
                    m_xRename.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,s_sTableRename)),UNO_QUERY);
                    static const ::rtl::OUString s_sTableAlteration(RTL_CONSTASCII_USTRINGPARAM("TableAlterationServiceName"));
                    m_xAlter.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,s_sTableAlteration)),UNO_QUERY);
                    static const ::rtl::OUString s_sKeyAlteration(RTL_CONSTASCII_USTRINGPARAM("KeyAlterationServiceName"));
                    m_xKeyAlter.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,s_sKeyAlteration)),UNO_QUERY);
                    static const ::rtl::OUString s_sIndexAlteration(RTL_CONSTASCII_USTRINGPARAM("IndexAlterationServiceName"));
                    m_xIndexAlter.set(xFac->createInstance(lcl_getServiceNameForSetting(m_xConnection,s_sIndexAlteration)),UNO_QUERY);
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
                           sal_Bool _bCase)
    :OTable_TYPEDEF(_pTables,_bCase)
    ,m_pImpl(new OTableHelperImpl(_xConnection))
{
}
// -------------------------------------------------------------------------
OTableHelper::OTableHelper( sdbcx::OCollection* _pTables,
                            const Reference< XConnection >& _xConnection,
                            sal_Bool _bCase,
                            const ::rtl::OUString& _Name,
                            const ::rtl::OUString& _Type,
                            const ::rtl::OUString& _Description ,
                            const ::rtl::OUString& _SchemaName,
                            const ::rtl::OUString& _CatalogName
                        ) : OTable_TYPEDEF(_pTables,
                                            _bCase,
                                            _Name,
                                          _Type,
                                          _Description,
                                          _SchemaName,
                                          _CatalogName)
                        ,m_pImpl(new OTableHelperImpl(_xConnection))
{
}
// -----------------------------------------------------------------------------
OTableHelper::~OTableHelper()
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableHelper::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_pImpl->m_xTablePropertyListener.is() )
    {
        m_pTables->removeContainerListener(m_pImpl->m_xTablePropertyListener.getRef());
        m_pImpl->m_xTablePropertyListener->clear();
        m_pImpl->m_xTablePropertyListener.dispose();
    }
    OTable_TYPEDEF::disposing();

    m_pImpl->m_xConnection  = NULL;
    m_pImpl->m_xMetaData    = NULL;

}

// -------------------------------------------------------------------------
namespace
{
    /** collects ColumnDesc's from a resultset produced by XDatabaseMetaData::getColumns
    */
    void lcl_collectColumnDescs_throw( const Reference< XResultSet >& _rxResult, ::std::vector< ColumnDesc >& _out_rColumns )
    {
        Reference< XRow > xRow( _rxResult, UNO_QUERY_THROW );
        ::rtl::OUString sName;
        OrdinalPosition nOrdinalPosition( 0 );
        while ( _rxResult->next() )
        {
            sName = xRow->getString( 4 );           // COLUMN_NAME
            sal_Int32       nField5 = xRow->getInt(5);
            ::rtl::OUString aField6 = xRow->getString(6);
            sal_Int32       nField7 = xRow->getInt(7)
                        ,   nField9 = xRow->getInt(9)
                        ,   nField11= xRow->getInt(11);
            ::rtl::OUString  sField12 = xRow->getString(12)
                            ,sField13 = xRow->getString(13);
            nOrdinalPosition = xRow->getInt( 17 );  // ORDINAL_POSITION
            _out_rColumns.push_back( ColumnDesc( sName,nField5,aField6,nField7,nField9,nField11,sField12,sField13, nOrdinalPosition ) );
        }
    }

    /** checks a given array of ColumnDesc's whether it has reasonable ordinal positions. If not,
        they will be normalized to be the array index.
    */
    void lcl_sanitizeColumnDescs( ::std::vector< ColumnDesc >& _rColumns )
    {
        if ( _rColumns.empty() )
            return;

        // collect all used ordinals
        ::std::set< OrdinalPosition > aUsedOrdinals;
        for (   ::std::vector< ColumnDesc >::iterator collect = _rColumns.begin();
                collect != _rColumns.end();
                ++collect
            )
            aUsedOrdinals.insert( collect->nOrdinalPosition );

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
            for (   ::std::vector< ColumnDesc >::iterator normalize = _rColumns.begin();
                    normalize != _rColumns.end();
                    ++normalize
                )
                normalize->nOrdinalPosition = nNormalizedPosition++;
            return;
        }

        // what's left is that the range might not be from 1 to <column count>, but for instance
        // 0 to <column count>-1.
        size_t nOffset = *aUsedOrdinals.begin() - 1;
        for (   ::std::vector< ColumnDesc >::iterator offset = _rColumns.begin();
                offset != _rColumns.end();
                ++offset
            )
            offset->nOrdinalPosition -= nOffset;
    }
}

// -------------------------------------------------------------------------
void OTableHelper::refreshColumns()
{
    TStringVector aVector;
    if(!isNew())
    {
        Any aCatalog;
        if ( !m_CatalogName.isEmpty() )
            aCatalog <<= m_CatalogName;

        ::utl::SharedUNOComponent< XResultSet > xResult( getMetaData()->getColumns(
            aCatalog,
            m_SchemaName,
            m_Name,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%"))
        ) );

        // collect the column names, together with their ordinal position
        m_pImpl->m_aColumnDesc.clear();
        lcl_collectColumnDescs_throw( xResult, m_pImpl->m_aColumnDesc );

        // ensure that the ordinal positions as obtained from the meta data do make sense
        lcl_sanitizeColumnDescs( m_pImpl->m_aColumnDesc );

        // sort by ordinal position
        ::std::map< OrdinalPosition, ::rtl::OUString > aSortedColumns;
        for (   ::std::vector< ColumnDesc >::const_iterator copy = m_pImpl->m_aColumnDesc.begin();
                copy != m_pImpl->m_aColumnDesc.end();
                ++copy
            )
            aSortedColumns[ copy->nOrdinalPosition ] = copy->sName;

        // copy them to aVector, now that we have the proper ordering
        ::std::transform(
            aSortedColumns.begin(),
            aSortedColumns.end(),
            ::std::insert_iterator< TStringVector >( aVector, aVector.begin() ),
            ::o3tl::select2nd< ::std::map< OrdinalPosition, ::rtl::OUString >::value_type >()
        );
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = createColumns(aVector);
}
// -----------------------------------------------------------------------------
const ColumnDesc* OTableHelper::getColumnDescription(const ::rtl::OUString& _sName) const
{
    const ColumnDesc* pRet = NULL;
    ::std::vector< ColumnDesc >::const_iterator aEnd = m_pImpl->m_aColumnDesc.end();
    for (::std::vector< ColumnDesc >::const_iterator aIter = m_pImpl->m_aColumnDesc.begin();aIter != aEnd;++aIter)
    {
        if ( aIter->sName == _sName )
        {
            pRet = &*aIter;
            break;
        }
    } // for (::std::vector< ColumnDesc >::const_iterator aIter = m_pImpl->m_aColumnDesc.begin();aIter != aEnd;++aIter)
    return pRet;
}
// -------------------------------------------------------------------------
void OTableHelper::refreshPrimaryKeys(TStringVector& _rNames)
{
    Any aCatalog;
    if ( !m_CatalogName.isEmpty() )
        aCatalog <<= m_CatalogName;
    Reference< XResultSet > xResult = getMetaData()->getPrimaryKeys(aCatalog,m_SchemaName,m_Name);

    if ( xResult.is() )
    {
        sdbcx::TKeyProperties pKeyProps(new sdbcx::KeyProperties(::rtl::OUString(),KeyType::PRIMARY,0,0));
        ::rtl::OUString aPkName;
        bool bAlreadyFetched = false;
        const Reference< XRow > xRow(xResult,UNO_QUERY);
        while ( xResult->next() )
        {
            pKeyProps->m_aKeyColumnNames.push_back(xRow->getString(4));
            if ( !bAlreadyFetched )
            {
                aPkName = xRow->getString(6);
                bAlreadyFetched = true;
            }
        }

        m_pImpl->m_aKeys.insert(TKeyMap::value_type(aPkName,pKeyProps));
        _rNames.push_back(aPkName);
    } // if ( xResult.is() && xResult->next() )
    ::comphelper::disposeComponent(xResult);
}
// -------------------------------------------------------------------------
void OTableHelper::refreshForeignKeys(TStringVector& _rNames)
{
    Any aCatalog;
    if ( !m_CatalogName.isEmpty() )
        aCatalog <<= m_CatalogName;
    Reference< XResultSet > xResult = getMetaData()->getImportedKeys(aCatalog,m_SchemaName,m_Name);
    Reference< XRow > xRow(xResult,UNO_QUERY);

    if ( xRow.is() )
    {
        sdbcx::TKeyProperties pKeyProps;
        ::rtl::OUString aName,sCatalog,aSchema,sOldFKName;
        while( xResult->next() )
        {
            // this must be outsid the "if" because we have to call in a right order
            sCatalog    = xRow->getString(1);
            if ( xRow->wasNull() )
                sCatalog = ::rtl::OUString();
            aSchema     = xRow->getString(2);
            aName       = xRow->getString(3);

            const ::rtl::OUString sForeignKeyColumn = xRow->getString(8);
            const sal_Int32 nUpdateRule = xRow->getInt(10);
            const sal_Int32 nDeleteRule = xRow->getInt(11);
            const ::rtl::OUString sFkName = xRow->getString(12);

                if ( pKeyProps.get() )
                {
                }


            if ( !sFkName.isEmpty() && !xRow->wasNull() )
            {
                if ( sOldFKName != sFkName )
                {
                    if ( pKeyProps.get() )
                        m_pImpl->m_aKeys.insert(TKeyMap::value_type(sOldFKName,pKeyProps));

                    const ::rtl::OUString sReferencedName = ::dbtools::composeTableName(getMetaData(),sCatalog,aSchema,aName,sal_False,::dbtools::eInDataManipulation);
                    pKeyProps.reset(new sdbcx::KeyProperties(sReferencedName,KeyType::FOREIGN,nUpdateRule,nDeleteRule));
                    pKeyProps->m_aKeyColumnNames.push_back(sForeignKeyColumn);
                    _rNames.push_back(sFkName);
                    if ( m_pTables->hasByName(sReferencedName) )
                    {
                        if ( !m_pImpl->m_xTablePropertyListener.is() )
                            m_pImpl->m_xTablePropertyListener = ::comphelper::ImplementationReference< OTableContainerListener,XContainerListener>( new OTableContainerListener(this) );
                        m_pTables->addContainerListener(m_pImpl->m_xTablePropertyListener.getRef());
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
            m_pImpl->m_aKeys.insert(TKeyMap::value_type(sOldFKName,pKeyProps));
        ::comphelper::disposeComponent(xResult);
    }
}
// -------------------------------------------------------------------------
void OTableHelper::refreshKeys()
{
    m_pImpl->m_aKeys.clear();

    TStringVector aNames;

    if(!isNew())
    {
        refreshPrimaryKeys(aNames);
        refreshForeignKeys(aNames);
        m_pKeys = createKeys(aNames);
    } // if(!isNew())
    else if (!m_pKeys )
        m_pKeys = createKeys(aNames);
    /*if(m_pKeys)
        m_pKeys->reFill(aVector);
    else*/

}
// -------------------------------------------------------------------------
void OTableHelper::refreshIndexes()
{
    TStringVector aVector;
    if(!isNew())
    {
        // fill indexes
        Any aCatalog;
        if ( !m_CatalogName.isEmpty() )
            aCatalog <<= m_CatalogName;
        Reference< XResultSet > xResult = getMetaData()->getIndexInfo(aCatalog,m_SchemaName,m_Name,sal_False,sal_False);

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            ::rtl::OUString aName;
            ::rtl::OUString sCatalogSep = getMetaData()->getCatalogSeparator();
            ::rtl::OUString sPreviousRoundName;
            while( xResult->next() )
            {
                aName = xRow->getString(5);
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

    if(m_pIndexes)
        m_pIndexes->reFill(aVector);
    else
        m_pIndexes  = createIndexes(aVector);
}
// -----------------------------------------------------------------------------
::rtl::OUString OTableHelper::getRenameStart() const
{
    ::rtl::OUString sSql(RTL_CONSTASCII_USTRINGPARAM("RENAME "));
    if ( m_Type == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW")) )
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" VIEW "));
    else
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TABLE "));

    return sSql;
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OTableHelper::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef GCC
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
            ::rtl::OUString sSql = getRenameStart();
            ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );

            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(getMetaData(),newName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);

            ::rtl::OUString sComposedName;
            sComposedName = ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sal_True,::dbtools::eInDataManipulation);
            sSql += sComposedName
                 + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TO "));
            sComposedName = ::dbtools::composeTableName(getMetaData(),sCatalog,sSchema,sTable,sal_True,::dbtools::eInDataManipulation);
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
        ::dbtools::qualifiedNameComponents(getMetaData(),newName,m_CatalogName,m_SchemaName,m_Name,::dbtools::eInTableDefinitions);
}
// -----------------------------------------------------------------------------
Reference< XDatabaseMetaData> OTableHelper::getMetaData() const
{
    return m_pImpl->m_xMetaData;
}
// -------------------------------------------------------------------------
void SAL_CALL OTableHelper::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef GCC
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    Reference< XPropertySet > xOld;
    if(::cppu::extractInterface(xOld,m_pColumns->getByIndex(index)) && xOld.is())
        alterColumnByName(getString(xOld->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),descriptor);
}

// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTableHelper::getName() throw(RuntimeException)
{
    ::rtl::OUString sComposedName;
    sComposedName = ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sal_False,::dbtools::eInDataManipulation);
    return sComposedName;
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableHelper::acquire() throw()
{
    OTable_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableHelper::release() throw()
{
    OTable_TYPEDEF::release();
}
// -----------------------------------------------------------------------------
sdbcx::TKeyProperties OTableHelper::getKeyProperties(const ::rtl::OUString& _sName) const
{
    sdbcx::TKeyProperties pKeyProps;
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
// -----------------------------------------------------------------------------
void OTableHelper::addKey(const ::rtl::OUString& _sName,const sdbcx::TKeyProperties& _aKeyProperties)
{
    m_pImpl->m_aKeys.insert(TKeyMap::value_type(_sName,_aKeyProperties));
}
// -----------------------------------------------------------------------------
::rtl::OUString OTableHelper::getTypeCreatePattern() const
{
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------
Reference< XConnection> OTableHelper::getConnection() const
{
    return m_pImpl->m_xConnection;
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::sdb::tools::XTableRename>      OTableHelper::getRenameService() const
{
    return m_pImpl->m_xRename;
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::sdb::tools::XTableAlteration>  OTableHelper::getAlterService() const
{
    return m_pImpl->m_xAlter;
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::sdb::tools::XKeyAlteration>  OTableHelper::getKeyService() const
{
    return m_pImpl->m_xKeyAlter;
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::sdb::tools::XIndexAlteration>  OTableHelper::getIndexService() const
{
    return m_pImpl->m_xIndexAlter;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
