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

#include <strings.hxx>
#include <FilteredContainer.hxx>
#include <RefreshListener.hxx>
#include <sdbcoretools.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <tools/wldcrd.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <optional>
#include <sal/log.hxx>

namespace dbaccess
{
    using namespace dbtools;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::container;
    using namespace ::osl;
    using namespace ::comphelper;
    using namespace ::cppu;
    using namespace ::connectivity::sdbcx;

/** creates a vector of WildCards and reduce the _rTableFilter of the length of WildCards
*/
static sal_Int32 createWildCardVector(Sequence< OUString >& _rTableFilter, std::vector< WildCard >& _rOut)
{
    // for wildcard search : remove all table filters which are a wildcard expression and build a WildCard
    // for them
    OUString* pTableFilters = _rTableFilter.getArray();
    OUString* pEnd          = pTableFilters + _rTableFilter.getLength();
    sal_Int32 nShiftPos = 0;
    for (sal_Int32 i=0; pEnd != pTableFilters; ++pTableFilters,++i)
    {
        if (pTableFilters->indexOf('%') != -1)
        {
            _rOut.emplace_back(pTableFilters->replace('%', '*'));
        }
        else
        {
            if (nShiftPos != i)
            {
                _rTableFilter.getArray()[nShiftPos] = _rTableFilter.getArray()[i];
            }
            ++nShiftPos;
        }
    }
    // now aTableFilter contains nShiftPos non-wc-strings and aWCSearch all wc-strings
    _rTableFilter.realloc(nShiftPos);
    return nShiftPos;
}

    static bool lcl_isElementAllowed(std::u16string_view _rName,
                                const Sequence< OUString >& _rTableFilter,
                                const std::vector< WildCard >& _rWCSearch )
    {
        bool bFilterMatch = std::find(_rTableFilter.begin(), _rTableFilter.end(), _rName) != _rTableFilter.end();
        // the table is allowed to "pass" if we had no filters at all or any of the non-wildcard filters matches
        if (!bFilterMatch && !_rWCSearch.empty())
        {   // or if one of the wildcard expression matches
            for (auto const& elem : _rWCSearch)
            {
                bFilterMatch = elem.Matches( _rName );
                if (bFilterMatch)
                    break;
            }
        }

        return bFilterMatch;
    }

    typedef ::std::optional< OUString >    OptionalString;

    namespace {

    struct TableInfo
    {
        OptionalString  sComposedName;
        OptionalString  sType;
        OptionalString  sCatalog;
        OptionalString  sSchema;
        OptionalString  sName;

        explicit TableInfo( const OUString& _composedName )
            : sComposedName( _composedName )
        {
        }

        TableInfo( const OUString& _catalog, const OUString& _schema, const OUString& _name,
            const OUString& _type )
            :sType( _type )
            ,sCatalog( _catalog )
            ,sSchema( _schema )
            ,sName( _name )
        {
        }
    };

    }

    typedef std::vector< TableInfo >    TableInfos;

    static void lcl_ensureComposedName( TableInfo& _io_tableInfo, const Reference< XDatabaseMetaData >& _metaData )
    {
        if ( !_metaData.is() )
            throw RuntimeException("lcl_ensureComposedName: _metaData cannot be null!");

        if ( !_io_tableInfo.sComposedName )
        {
            OSL_ENSURE( !!_io_tableInfo.sCatalog && !!_io_tableInfo.sSchema && !!_io_tableInfo.sName, "lcl_ensureComposedName: How should I composed the name from nothing!?" );

            _io_tableInfo.sComposedName = OptionalString(
                composeTableName( _metaData, *_io_tableInfo.sCatalog, *_io_tableInfo.sSchema, *_io_tableInfo.sName,
                false, ::dbtools::EComposeRule::InDataManipulation )
            );
        }
    }

    static void lcl_ensureType( TableInfo& _io_tableInfo, const Reference< XDatabaseMetaData >& _metaData, const Reference< XNameAccess >& _masterContainer )
    {
        if ( !!_io_tableInfo.sType )
            return;

        lcl_ensureComposedName( _io_tableInfo, _metaData );

        if ( !_masterContainer.is() )
            throw RuntimeException("lcl_ensureType: _masterContainer cannot be null!");

        OUString sTypeName;
        try
        {
            Reference< XPropertySet > xTable( _masterContainer->getByName( *_io_tableInfo.sComposedName ), UNO_QUERY_THROW );
            OSL_VERIFY( xTable->getPropertyValue( PROPERTY_TYPE ) >>= sTypeName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        _io_tableInfo.sType = OptionalString( sTypeName );
    }

    static ::std::vector< OUString> lcl_filter( TableInfos&& _unfilteredTables,
        const Sequence< OUString >& _tableFilter, const Sequence< OUString >& _tableTypeFilter,
        const Reference< XDatabaseMetaData >& _metaData, const Reference< XNameAccess >& _masterContainer )
    {
        TableInfos aFilteredTables;

        // first, filter for the table names
        sal_Int32 nTableFilterCount = _tableFilter.getLength();
        bool dontFilterTableNames = ( ( nTableFilterCount == 1 ) && _tableFilter[0] == "%" );
        if( dontFilterTableNames )
        {
            aFilteredTables = std::move(_unfilteredTables);
        }
        else
        {
            // for wildcard search : remove all table filters which are a wildcard expression and build a WildCard
            // for them
            std::vector< WildCard > aWildCardTableFilter;
            Sequence< OUString > aNonWildCardTableFilter = _tableFilter;
            nTableFilterCount = createWildCardVector( aNonWildCardTableFilter, aWildCardTableFilter );

            TableInfos aUnfilteredTables( std::move(_unfilteredTables) );
            aUnfilteredTables.reserve( nTableFilterCount + ( aWildCardTableFilter.size() * 10 ) );

            for (auto & unfilteredTable : aUnfilteredTables)
            {
                lcl_ensureComposedName(unfilteredTable, _metaData);

                if ( lcl_isElementAllowed( *unfilteredTable.sComposedName, aNonWildCardTableFilter, aWildCardTableFilter ) )
                    aFilteredTables.push_back(unfilteredTable);
            }
        }

        // second, filter for the table types
        sal_Int32 nTableTypeFilterCount = _tableTypeFilter.getLength();
        bool dontFilterTableTypes = ( ( nTableTypeFilterCount == 1 ) && _tableTypeFilter[0] == "%" );
        dontFilterTableTypes = dontFilterTableTypes || ( nTableTypeFilterCount == 0 );
            // (for TableTypeFilter, unlike TableFilter, "empty" means "do not filter at all")
        if ( !dontFilterTableTypes )
        {
            TableInfos aUnfilteredTables;
            aUnfilteredTables.swap( aFilteredTables );

            for (auto & unfilteredTable : aUnfilteredTables)
            {
                // ensure that we know the table type
                lcl_ensureType( unfilteredTable, _metaData, _masterContainer );

                if (std::find(_tableTypeFilter.begin(), _tableTypeFilter.end(), *unfilteredTable.sType) != _tableTypeFilter.end())
                    aFilteredTables.push_back(unfilteredTable);
            }
        }

        ::std::vector< OUString> aReturn;
        for (auto & filteredTable : aFilteredTables)
        {
            lcl_ensureComposedName(filteredTable, _metaData);
            aReturn.push_back(*filteredTable.sComposedName);
        }
        return aReturn;
    }

    // OViewContainer
    OFilteredContainer::OFilteredContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 bool _bCase,
                                 IRefreshListener*  _pRefreshListener,
                                 std::atomic<std::size_t>& _nInAppend)
        :OCollection(_rParent,_bCase,_rMutex,std::vector< OUString>())
        ,m_bConstructed(false)
        ,m_pRefreshListener(_pRefreshListener)
        ,m_nInAppend(_nInAppend)
        ,m_xConnection(_xCon)
    {
    }

    void OFilteredContainer::construct(const Reference< XNameAccess >& _rxMasterContainer,
                                    const Sequence< OUString >& _rTableFilter,
                                    const Sequence< OUString >& _rTableTypeFilter)
    {
        try
        {
            Reference<XConnection> xCon = m_xConnection;
            if ( xCon.is() )
                m_xMetaData = xCon->getMetaData();
        }
        catch(SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        m_xMasterContainer = _rxMasterContainer;

        if ( m_xMasterContainer.is() )
        {
            addMasterContainerListener();

            TableInfos aUnfilteredTables;

            Sequence<OUString> aNames = m_xMasterContainer->getElementNames();
            aUnfilteredTables.reserve(aNames.getLength());
            for (auto& name : aNames)
                aUnfilteredTables.emplace_back(name);

            reFill( lcl_filter( std::move(aUnfilteredTables),
                _rTableFilter, _rTableTypeFilter, m_xMetaData, m_xMasterContainer ) );

            m_bConstructed = true;
        }
        else
        {
            construct( _rTableFilter, _rTableTypeFilter );
        }
    }

    void OFilteredContainer::construct(const Sequence< OUString >& _rTableFilter, const Sequence< OUString >& _rTableTypeFilter)
    {
        // build sorted versions of the filter sequences, so the visibility decision is faster
        Sequence< OUString > aTableFilter(_rTableFilter);

        // for wildcard search : remove all table filters which are a wildcard expression and build a WildCard
        // for them
        std::vector< WildCard > aWCSearch;
        createWildCardVector(aTableFilter,aWCSearch);

        try
        {
            Reference< XConnection > xCon( m_xConnection, UNO_SET_THROW );
            m_xMetaData.set( xCon->getMetaData(), UNO_SET_THROW );

            // create a table filter suitable for the XDatabaseMetaData::getTables call,
            // taking into account both the externally-provided table type filter, and any
            // table type restriction which is inherent to the container
            Sequence< OUString > aTableTypeFilter;
            OUString sInherentTableTypeRestriction( getTableTypeRestriction() );
            if ( !sInherentTableTypeRestriction.isEmpty() )
            {
                if ( _rTableTypeFilter.hasElements() )
                {
                    if (std::find(_rTableTypeFilter.begin(), _rTableTypeFilter.end(),
                                  sInherentTableTypeRestriction)
                        == _rTableTypeFilter.end())
                    {   // the only table type which can be part of this container is not allowed
                        // by the externally provided table type filter.
                        m_bConstructed = true;
                        return;
                    }
                }
                aTableTypeFilter = { sInherentTableTypeRestriction };
            }
            else
            {
                // no container-inherent restriction for the table types
                if ( !_rTableTypeFilter.hasElements() )
                {   // no externally-provided table type filter => use the default filter
                    getAllTableTypeFilter( aTableTypeFilter );
                }
                else
                {
                    aTableTypeFilter = _rTableTypeFilter;
                }
            }

            static constexpr OUString sAll = u"%"_ustr;
            Reference< XResultSet > xTables = m_xMetaData->getTables( Any(), sAll, sAll, aTableTypeFilter );
            Reference< XRow > xCurrentRow( xTables, UNO_QUERY_THROW );

            TableInfos aUnfilteredTables;

            OUString sCatalog, sSchema, sName, sType;
            while ( xTables->next() )
            {
                sCatalog    = xCurrentRow->getString(1);
                sSchema     = xCurrentRow->getString(2);
                sName       = xCurrentRow->getString(3);
                sType       = xCurrentRow->getString(4);

                aUnfilteredTables.emplace_back( sCatalog, sSchema, sName, sType );
            }

            reFill( lcl_filter( std::move(aUnfilteredTables),
                _rTableFilter, aTableTypeFilter, m_xMetaData, nullptr ) );

            disposeComponent( xTables );
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
            disposing();
            return;
        }

        m_bConstructed = true;
    }

    void OFilteredContainer::disposing()
    {
        OCollection::disposing();

        if ( m_xMasterContainer.is() )
            removeMasterContainerListener();

        m_xMasterContainer  = nullptr;
        m_xMetaData         = nullptr;
        m_pRefreshListener  = nullptr;
        m_bConstructed      = false;
    }

    void OFilteredContainer::impl_refresh()
    {
        if ( m_pRefreshListener )
        {
            m_bConstructed = false;
            Reference<XRefreshable> xRefresh(m_xMasterContainer,UNO_QUERY);
            if ( xRefresh.is() )
                xRefresh->refresh();
            m_pRefreshListener->refresh(this);
        }
    }

    OUString OFilteredContainer::getNameForObject(const ObjectType& _xObject)
    {
        OSL_ENSURE( _xObject.is(), "OFilteredContainer::getNameForObject: Object is NULL!" );
        return ::dbtools::composeTableName( m_xMetaData, _xObject, ::dbtools::EComposeRule::InDataManipulation, false );
    }

    // multiple to obtain all tables from XDatabaseMetaData::getTables, via passing a particular
    // table type filter:
    // adhere to the standard, which requests to pass a NULL table type filter, if
    // you want to retrieve all tables
    #define FILTER_MODE_STANDARD 0
    // only pass %, which is not allowed by the standard, but understood by some drivers
    #define FILTER_MODE_WILDCARD 1
    // only pass TABLE and VIEW
    #define FILTER_MODE_FIXED    2
    // do the thing which showed to be the safest way, understood by nearly all
    // drivers, even the ones which do not understand the standard
    #define FILTER_MODE_MIX_ALL  3

    void OFilteredContainer::getAllTableTypeFilter( Sequence< OUString >& /* [out] */ _rFilter ) const
    {
        sal_Int32 nFilterMode = FILTER_MODE_MIX_ALL;
            // for compatibility reasons, this is the default: we used this way before we
            // introduced the TableTypeFilterMode setting

        // obtain the data source we belong to, and the TableTypeFilterMode setting
        Any aFilterModeSetting;
        if ( getDataSourceSetting( getDataSource( Reference< XInterface >(m_rParent) ), "TableTypeFilterMode", aFilterModeSetting ) )
        {
            OSL_VERIFY( aFilterModeSetting >>= nFilterMode );
        }

        static constexpr OUString sAll( u"%"_ustr  );
        static constexpr OUString sView( u"VIEW"_ustr  );
        static constexpr OUString sTable( u"TABLE"_ustr  );

        switch ( nFilterMode )
        {
        default:
            SAL_WARN("dbaccess",  "OTableContainer::getAllTableTypeFilter: unknown TableTypeFilterMode!" );
            [[fallthrough]];
        case FILTER_MODE_MIX_ALL:
            _rFilter = { sView, sTable, sAll };
            break;
        case FILTER_MODE_FIXED:
            _rFilter = { sView, sTable };
            break;
        case FILTER_MODE_WILDCARD:
            _rFilter = { sAll };
            break;
        case FILTER_MODE_STANDARD:
            _rFilter.realloc( 0 );
            break;
        }
    }

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
