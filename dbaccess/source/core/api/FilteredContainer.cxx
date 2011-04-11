/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_dbaccess.hxx"

#include "dbastrings.hrc"
#include "FilteredContainer.hxx"
#include "RefreshListener.hxx"
#include "sdbcoretools.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <connectivity/dbtools.hxx>
#include <tools/wldcrd.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/logfile.hxx>
#include <boost/optional.hpp>

namespace dbaccess
{
    using namespace dbtools;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::container;
    using namespace ::osl;
    using namespace ::comphelper;
    using namespace ::cppu;
    using namespace ::connectivity::sdbcx;

/** creates a vector of WildCards and reduce the _rTableFilter of the length of WildsCards
*/
sal_Int32 createWildCardVector(Sequence< ::rtl::OUString >& _rTableFilter, ::std::vector< WildCard >& _rOut)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OFilteredContainer::createWildCardVector" );
    // for wildcard search : remove all table filters which are a wildcard expression and build a WilCard
    // for them
    ::rtl::OUString* pTableFilters = _rTableFilter.getArray();
    ::rtl::OUString* pEnd          = pTableFilters + _rTableFilter.getLength();
    sal_Int32 nShiftPos = 0;
    for (sal_Int32 i=0; pEnd != pTableFilters; ++pTableFilters,++i)
    {
        if (pTableFilters->indexOf('%') != -1)
        {
            _rOut.push_back(WildCard(pTableFilters->replace('%', '*')));
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

    bool lcl_isElementAllowed(  const ::rtl::OUString& _rName,
                                const Sequence< ::rtl::OUString >& _rTableFilter,
                                const ::std::vector< WildCard >& _rWCSearch )
    {
        sal_Int32 nTableFilterLen = _rTableFilter.getLength();

        const ::rtl::OUString* tableFilter = _rTableFilter.getConstArray();
        const ::rtl::OUString* tableFilterEnd = _rTableFilter.getConstArray() + nTableFilterLen;
        bool bFilterMatch = ::std::find( tableFilter, tableFilterEnd, _rName ) != tableFilterEnd;
        // the table is allowed to "pass" if we had no filters at all or any of the non-wildcard filters matches
        if (!bFilterMatch && !_rWCSearch.empty())
        {   // or if one of the wildcrad expression matches
            for (   ::std::vector< WildCard >::const_iterator aLoop = _rWCSearch.begin();
                    aLoop != _rWCSearch.end() && !bFilterMatch;
                    ++aLoop
                )
                bFilterMatch = aLoop->Matches( _rName );
        }

        return bFilterMatch;
    }

    typedef ::boost::optional< ::rtl::OUString >    OptionalString;
    struct TableInfo
    {
        OptionalString  sComposedName;
        OptionalString  sType;
        OptionalString  sCatalog;
        OptionalString  sSchema;
        OptionalString  sName;

        TableInfo( const ::rtl::OUString& _composedName )
            :sComposedName( _composedName )
        {
        }

        TableInfo( const ::rtl::OUString& _catalog, const ::rtl::OUString& _schema, const ::rtl::OUString& _name,
            const ::rtl::OUString& _type )
            :sComposedName()
            ,sType( _type )
            ,sCatalog( _catalog )
            ,sSchema( _schema )
            ,sName( _name )
        {
        }
    };
    typedef ::std::vector< TableInfo >    TableInfos;

    void lcl_ensureComposedName( TableInfo& _io_tableInfo, const Reference< XDatabaseMetaData >& _metaData )
    {
        if ( !_metaData.is() )
            throw RuntimeException();

        if ( !_io_tableInfo.sComposedName )
        {
            OSL_ENSURE( !!_io_tableInfo.sCatalog && !!_io_tableInfo.sSchema && !!_io_tableInfo.sName, "lcl_ensureComposedName: How should I composed the name from nothing!?" );

            _io_tableInfo.sComposedName = OptionalString(
                composeTableName( _metaData, *_io_tableInfo.sCatalog, *_io_tableInfo.sSchema, *_io_tableInfo.sName,
                sal_False, ::dbtools::eInDataManipulation )
            );
        }
    }

    void lcl_ensureType( TableInfo& _io_tableInfo, const Reference< XDatabaseMetaData >& _metaData, const Reference< XNameAccess >& _masterContainer )
    {
        if ( !!_io_tableInfo.sType )
            return;

        lcl_ensureComposedName( _io_tableInfo, _metaData );

        if ( !_masterContainer.is() )
            throw RuntimeException();

        ::rtl::OUString sTypeName;
        try
        {
            Reference< XPropertySet > xTable( _masterContainer->getByName( *_io_tableInfo.sComposedName ), UNO_QUERY_THROW );
            OSL_VERIFY( xTable->getPropertyValue( PROPERTY_TYPE ) >>= sTypeName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        _io_tableInfo.sType = OptionalString( sTypeName );
    }

    connectivity::TStringVector lcl_filter( const TableInfos& _unfilteredTables,
        const Sequence< ::rtl::OUString >& _tableFilter, const Sequence< ::rtl::OUString >& _tableTypeFilter,
        const Reference< XDatabaseMetaData >& _metaData, const Reference< XNameAccess >& _masterContainer )
    {
        TableInfos aFilteredTables;

        // first, filter for the table names
        sal_Int32 nTableFilterCount = _tableFilter.getLength();
        sal_Bool dontFilterTableNames = ( ( nTableFilterCount == 1 ) && _tableFilter[0].equalsAsciiL( "%", 1 ) );
        if( dontFilterTableNames )
        {
            aFilteredTables = _unfilteredTables;
        }
        else
        {
            // for wildcard search : remove all table filters which are a wildcard expression and build a WildCard
            // for them
            ::std::vector< WildCard > aWildCardTableFilter;
            Sequence< ::rtl::OUString > aNonWildCardTableFilter = _tableFilter;
            nTableFilterCount = createWildCardVector( aNonWildCardTableFilter, aWildCardTableFilter );

            TableInfos aUnfilteredTables( _unfilteredTables );
            aUnfilteredTables.reserve( nTableFilterCount + ( aWildCardTableFilter.size() * 10 ) );

            for (   TableInfos::iterator table = aUnfilteredTables.begin();
                    table != aUnfilteredTables.end();
                    ++table
                )
            {
                lcl_ensureComposedName( *table, _metaData );

                if ( lcl_isElementAllowed( *table->sComposedName, aNonWildCardTableFilter, aWildCardTableFilter ) )
                    aFilteredTables.push_back( *table );
            }
        }

        // second, filter for the table types
        sal_Int32 nTableTypeFilterCount = _tableTypeFilter.getLength();
        sal_Bool dontFilterTableTypes = ( ( nTableTypeFilterCount == 1 ) && _tableTypeFilter[0].equalsAsciiL( "%", 1 ) );
        dontFilterTableTypes = dontFilterTableTypes || ( nTableTypeFilterCount == 0 );
            // (for TableTypeFilter, unlike TableFilter, "empty" means "do not filter at all")
        if ( !dontFilterTableTypes )
        {
            TableInfos aUnfilteredTables;
            aUnfilteredTables.swap( aFilteredTables );

            const ::rtl::OUString* pTableTypeFilterBegin = _tableTypeFilter.getConstArray();
            const ::rtl::OUString* pTableTypeFilterEnd = pTableTypeFilterBegin + _tableTypeFilter.getLength();

            for (   TableInfos::iterator table = aUnfilteredTables.begin();
                    table != aUnfilteredTables.end();
                    ++table
                )
            {
                // ensure that we know the table type
                lcl_ensureType( *table, _metaData, _masterContainer );

                if ( ::std::find( pTableTypeFilterBegin, pTableTypeFilterEnd, *table->sType ) != pTableTypeFilterEnd )
                    aFilteredTables.push_back( *table );
            }
        }

        connectivity::TStringVector aReturn;
        for (   TableInfos::iterator table = aFilteredTables.begin();
                table != aFilteredTables.end();
                ++table
            )
        {
            lcl_ensureComposedName( *table, _metaData );
            aReturn.push_back( *table->sComposedName );
        }
        return aReturn;
    }

    //==========================================================================
    //= OViewContainer
    //==========================================================================
    OFilteredContainer::OFilteredContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 sal_Bool _bCase,
                                 IRefreshListener*  _pRefreshListener,
                                 ::dbtools::IWarningsContainer* _pWarningsContainer
                                 ,oslInterlockedCount& _nInAppend)
        :OCollection(_rParent,_bCase,_rMutex,::std::vector< ::rtl::OUString>())
        ,m_bConstructed(sal_False)
        ,m_pWarningsContainer(_pWarningsContainer)
        ,m_pRefreshListener(_pRefreshListener)
        ,m_nInAppend(_nInAppend)
        ,m_xConnection(_xCon)
    {
    }

    void OFilteredContainer::construct(const Reference< XNameAccess >& _rxMasterContainer,
                                    const Sequence< ::rtl::OUString >& _rTableFilter,
                                    const Sequence< ::rtl::OUString >& _rTableTypeFilter)
    {
        try
        {
            Reference<XConnection> xCon = m_xConnection;
            if ( xCon.is() )
                m_xMetaData = xCon->getMetaData();
        }
        catch(SQLException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        m_xMasterContainer = _rxMasterContainer;

        if ( m_xMasterContainer.is() )
        {
            addMasterContainerListener();

            TableInfos aUnfilteredTables;

            Sequence< ::rtl::OUString > aNames = m_xMasterContainer->getElementNames();
            const ::rtl::OUString*  name = aNames.getConstArray();
            const ::rtl::OUString*  nameEnd = name + aNames.getLength();
            for ( ; name != nameEnd; ++name )
                aUnfilteredTables.push_back( TableInfo( *name ) );

            reFill( lcl_filter( aUnfilteredTables,
                _rTableFilter, _rTableTypeFilter, m_xMetaData, m_xMasterContainer ) );

            m_bConstructed = sal_True;
        }
        else
        {
            construct( _rTableFilter, _rTableTypeFilter );
        }
    }

    void OFilteredContainer::construct(const Sequence< ::rtl::OUString >& _rTableFilter, const Sequence< ::rtl::OUString >& _rTableTypeFilter)
    {
        // build sorted versions of the filter sequences, so the visibility decision is faster
        Sequence< ::rtl::OUString > aTableFilter(_rTableFilter);

        // for wildcard search : remove all table filters which are a wildcard expression and build a WildCard
        // for them
        ::std::vector< WildCard > aWCSearch;
        createWildCardVector(aTableFilter,aWCSearch);

        try
        {
            Reference< XConnection > xCon( m_xConnection, UNO_SET_THROW );
            m_xMetaData.set( xCon->getMetaData(), UNO_SET_THROW );

            // create a table table filter suitable for the XDatabaseMetaData::getTables call,
            // taking into account both the externally-provided table type filter, and any
            // table type restriction which is inherent to the container
            Sequence< ::rtl::OUString > aTableTypeFilter;
            ::rtl::OUString sInherentTableTypeRestriction( getTableTypeRestriction() );
            if ( sInherentTableTypeRestriction.getLength() )
            {
                if ( _rTableTypeFilter.getLength() != 0 )
                {
                    const ::rtl::OUString* tableType    = _rTableTypeFilter.getConstArray();
                    const ::rtl::OUString* tableTypeEnd = tableType + _rTableTypeFilter.getLength();
                    for ( ; tableType != tableTypeEnd; ++tableType )
                    {
                        if ( *tableType == sInherentTableTypeRestriction )
                            break;
                    }
                    if ( tableType == tableTypeEnd )
                    {   // the only table type which can be part of this container is not allowed
                        // by the externally provided table type filter.
                        m_bConstructed = sal_True;
                        return;
                    }
                }
                aTableTypeFilter.realloc( 1 );
                aTableTypeFilter[0] = sInherentTableTypeRestriction;
            }
            else
            {
                // no container-inherent restriction for the table types
                if ( _rTableTypeFilter.getLength() == 0 )
                {   // no externally-provided table type filter => use the default filter
                    getAllTableTypeFilter( aTableTypeFilter );
                }
                else
                {
                    aTableTypeFilter = _rTableTypeFilter;
                }
            }

            static const ::rtl::OUString sAll(RTL_CONSTASCII_USTRINGPARAM("%"));
            Reference< XResultSet > xTables = m_xMetaData->getTables( Any(), sAll, sAll, aTableTypeFilter );
            Reference< XRow > xCurrentRow( xTables, UNO_QUERY_THROW );

            TableInfos aUnfilteredTables;

            ::rtl::OUString sCatalog, sSchema, sName, sType;
            while ( xTables->next() )
            {
                sCatalog    = xCurrentRow->getString(1);
                sSchema     = xCurrentRow->getString(2);
                sName       = xCurrentRow->getString(3);
                sType       = xCurrentRow->getString(4);

                aUnfilteredTables.push_back( TableInfo( sCatalog, sSchema, sName, sType ) );
            }

            reFill( lcl_filter( aUnfilteredTables,
                _rTableFilter, aTableTypeFilter, m_xMetaData, NULL ) );

            disposeComponent( xTables );
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
            disposing();
            return;
        }

        m_bConstructed = sal_True;
    }

    void OFilteredContainer::disposing()
    {
        OCollection::disposing();

        if ( m_xMasterContainer.is() )
            removeMasterContainerListener();

        m_xMasterContainer  = NULL;
        m_xMetaData         = NULL;
        m_pWarningsContainer = NULL;
        m_pRefreshListener  = NULL;
        m_bConstructed      = sal_False;
    }

    void OFilteredContainer::impl_refresh() throw(RuntimeException)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OFilteredContainer::impl_refresh" );
        if ( m_pRefreshListener )
        {
            m_bConstructed = sal_False;
            Reference<XRefreshable> xRefresh(m_xMasterContainer,UNO_QUERY);
            if ( xRefresh.is() )
                xRefresh->refresh();
            m_pRefreshListener->refresh(this);
        }
    }

    ::rtl::OUString OFilteredContainer::getNameForObject(const ObjectType& _xObject)
    {
        OSL_ENSURE( _xObject.is(), "OFilteredContainer::getNameForObject: Object is NULL!" );
        return ::dbtools::composeTableName( m_xMetaData, _xObject, ::dbtools::eInDataManipulation, false, false, false );
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

    void OFilteredContainer::getAllTableTypeFilter( Sequence< ::rtl::OUString >& /* [out] */ _rFilter ) const
    {
        sal_Int32 nFilterMode = FILTER_MODE_MIX_ALL;
            // for compatibility reasons, this is the default: we used this way before we
            // introduced the TableTypeFilterMode setting

        // obtain the data source we belong to, and the TableTypeFilterMode setting
        Any aFilterModeSetting;
        if ( getDataSourceSetting( getDataSource( (Reference< XInterface >)m_rParent ), "TableTypeFilterMode", aFilterModeSetting ) )
        {
            OSL_VERIFY( aFilterModeSetting >>= nFilterMode );
        }

        const ::rtl::OUString sAll( RTL_CONSTASCII_USTRINGPARAM( "%" ) );
        const ::rtl::OUString sView( RTL_CONSTASCII_USTRINGPARAM( "VIEW" ) );
        const ::rtl::OUString sTable( RTL_CONSTASCII_USTRINGPARAM( "TABLE" ) );

        switch ( nFilterMode )
        {
        default:
            OSL_FAIL( "OTableContainer::getAllTableTypeFilter: unknown TableTypeFilterMode!" );
        case FILTER_MODE_MIX_ALL:
            _rFilter.realloc( 3 );
            _rFilter[0] = sView;
            _rFilter[1] = sTable;
            _rFilter[2] = sAll;
            break;
        case FILTER_MODE_FIXED:
            _rFilter.realloc( 2 );
            _rFilter[0] = sView;
            _rFilter[1] = sTable;
            break;
        case FILTER_MODE_WILDCARD:
            _rFilter.realloc( 1 );
            _rFilter[0] = sAll;
            break;
        case FILTER_MODE_STANDARD:
            _rFilter.realloc( 0 );
            break;
        }
    }

} // namespace
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
