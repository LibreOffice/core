/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FilteredContainer.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:28:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBACCESS_CORE_FILTERED_CONTAINER_HXX
#include "FilteredContainer.hxx"
#endif
#ifndef DBA_CORE_REFRESHLISTENER_HXX
#include "RefreshListener.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif

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

    //------------------------------------------------------------------------------
    /** creates a vector of WildCards and reduce the _rTableFilter of the length of WildsCards
    */
    sal_Int32 createWildCardVector(Sequence< ::rtl::OUString >& _rTableFilter, ::std::vector< WildCard >& _rOut)
    {
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


    //==========================================================================
    //= OViewContainer
    //==========================================================================
    OFilteredContainer::OFilteredContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 sal_Bool _bCase,
                                 IRefreshListener*  _pRefreshListener,
                                 IWarningsContainer* _pWarningsContainer
                                 ,oslInterlockedCount& _nInAppend)
        :OCollection(_rParent,_bCase,_rMutex,::std::vector< ::rtl::OUString>())
        ,m_pWarningsContainer(_pWarningsContainer)
        ,m_pRefreshListener(_pRefreshListener)
        ,m_nInAppend(_nInAppend)
        ,m_xConnection(_xCon)
        ,m_bConstructed(sal_False)
    {

    }
    // -------------------------------------------------------------------------
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
        }

        m_xMasterContainer = _rxMasterContainer;

        if(m_xMasterContainer.is())
        {
            addMasterContainerListener();
            sal_Int32   nTableFilterLen = _rTableFilter.getLength();

            connectivity::TStringVector aTableNames;
            sal_Bool bNoTableFilters = ((nTableFilterLen == 1) && _rTableFilter[0].equalsAsciiL("%", 1));
            if(!bNoTableFilters)
            {
                Sequence< ::rtl::OUString > aTableFilter        = _rTableFilter;
                Sequence< ::rtl::OUString > aTableTypeFilter    = _rTableTypeFilter;

                // build sorted versions of the filter sequences, so the visibility decision is faster
                ::std::sort( aTableFilter.getArray(), aTableFilter.getArray() + nTableFilterLen );

                // as we want to modify nTableFilterLen, remember this

                // for wildcard search : remove all table filters which are a wildcard expression and build a WilCard
                // for them
                ::std::vector< WildCard > aWCSearch; // contains the wildcards for the table filter
                nTableFilterLen = createWildCardVector(aTableFilter,aWCSearch);

                aTableNames.reserve(nTableFilterLen + (aWCSearch.size() * 10));

                Sequence< ::rtl::OUString> aNames = m_xMasterContainer->getElementNames();
                const ::rtl::OUString* pBegin   = aNames.getConstArray();
                const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                {
                    if(isNameValid(*pBegin,aTableFilter,aTableTypeFilter,aWCSearch))
                        aTableNames.push_back(*pBegin);
                }
            }
            else
            {
                // no filter so insert all names
                Sequence< ::rtl::OUString> aNames = m_xMasterContainer->getElementNames();
                const ::rtl::OUString* pBegin   = aNames.getConstArray();
                const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
                aTableNames = connectivity::TStringVector(pBegin,pEnd);
            }
            reFill(aTableNames);
            m_bConstructed = sal_True;
        }
        else
        {
            construct(_rTableFilter,_rTableTypeFilter);
        }
    }
    //------------------------------------------------------------------------------
    void OFilteredContainer::construct(const Sequence< ::rtl::OUString >& _rTableFilter, const Sequence< ::rtl::OUString >& _rTableTypeFilter)
    {
        try
        {
            Reference<XConnection> xCon = m_xConnection;
            if ( xCon.is() )
                m_xMetaData = xCon->getMetaData();
        }
        catch(SQLException&)
        {
        }
        // build sorted versions of the filter sequences, so the visibility decision is faster
        Sequence< ::rtl::OUString > aTableFilter(_rTableFilter);
        sal_Int32   nTableFilterLen = aTableFilter.getLength();

        if (nTableFilterLen)
            ::std::sort( aTableFilter.getArray(), aTableFilter.getArray() + nTableFilterLen );

        sal_Bool bNoTableFilters = ((nTableFilterLen == 1) && _rTableFilter[0].equalsAsciiL("%", 1));
            // as we want to modify nTableFilterLen, remember this

        // for wildcard search : remove all table filters which are a wildcard expression and build a WilCard
        // for them
        ::std::vector< WildCard > aWCSearch;
        nTableFilterLen = createWildCardVector(aTableFilter,aWCSearch);

        try
        {
            if (m_xMetaData.is())
            {
                static const ::rtl::OUString sAll = ::rtl::OUString::createFromAscii("%");
                Sequence< ::rtl::OUString > sTableTypes = getTableTypeFilter(_rTableTypeFilter);
                if ( m_bConstructed && sTableTypes.getLength() == 0 )
                    return;

                Reference< XResultSet > xTables = m_xMetaData->getTables(Any(), sAll, sAll, sTableTypes);
                Reference< XRow > xCurrentRow(xTables, UNO_QUERY);
                if (xCurrentRow.is())
                {

                        // after creation the set is positioned before the first record, per definitionem

                    ::rtl::OUString sCatalog, sSchema, sName, sType;
                    ::rtl::OUString sComposedName;

                    // we first collect the names and construct the OTable objects later, as the ctor of the table may need
                    // another result set from the connection, and some drivers support only one statement per connection

                    sal_Bool bFilterMatch;
                    while (xTables->next())
                    {
                        sCatalog    = xCurrentRow->getString(1);
                        sSchema     = xCurrentRow->getString(2);
                        sName       = xCurrentRow->getString(3);
#if OSL_DEBUG_LEVEL > 0
                        ::rtl::OUString sTableType = xCurrentRow->getString(4);
#endif
                        // we're not interested in the "wasNull", as the getStrings would return an empty string in
                        // that case, which is sufficient here

                        sComposedName = composeTableName( m_xMetaData, sCatalog, sSchema, sName, sal_False, ::dbtools::eInDataManipulation );

                        const ::rtl::OUString* tableFilter = aTableFilter.getConstArray();
                        const ::rtl::OUString* tableFilterEnd = aTableFilter.getConstArray() + nTableFilterLen;
                        bool composedNameInFilter = ::std::find( tableFilter, tableFilterEnd, sComposedName ) != tableFilterEnd;

                        bFilterMatch =  bNoTableFilters
                                    ||  (   ( nTableFilterLen != 0 )
                                        &&  composedNameInFilter
                                        );

                        // the table is allowed to "pass" if we had no filters at all or any of the non-wildcard filters matches

                        if (!bFilterMatch && aWCSearch.size())
                        {   // or if one of the wildcrad expression matches
                            for (   ::std::vector< WildCard >::const_iterator aLoop = aWCSearch.begin();
                                    aLoop != aWCSearch.end() && !bFilterMatch;
                                    ++aLoop
                                )
                                bFilterMatch = aLoop->Matches(sComposedName);
                        }

                        if (bFilterMatch)
                        {   // the table name is allowed (not filtered out)
                            insertElement(sComposedName,NULL);
                        }
                    }

                    // dispose the tables result set, in case the connection can handle only one concurrent statement
                    // (the table object creation will need it's own statements)
                    disposeComponent(xTables);
                }
                else
                    OSL_ENSURE(0,"OFilteredContainer::construct : did not get a XRow from the tables result set !");
            }
            else
                OSL_ENSURE(0,"OFilteredContainer::construct : no connection meta data !");
        }
        catch (SQLException&)
        {
            OSL_ENSURE(0,"OFilteredContainer::construct: caught an SQL-Exception !");
            disposing();
            return;
        }

        m_bConstructed = sal_True;
    }
    //------------------------------------------------------------------------------
    void OFilteredContainer::disposing()
    {
        OCollection::disposing();
        removeMasterContainerListener();

        m_xMasterContainer  = NULL;
        m_xMetaData         = NULL;
        m_pWarningsContainer = NULL;
        m_pRefreshListener  = NULL;
        m_bConstructed      = sal_False;
    }
    // -------------------------------------------------------------------------
    void OFilteredContainer::impl_refresh() throw(RuntimeException)
    {
        if ( m_pRefreshListener )
        {
            m_bConstructed = sal_False;
            Reference<XRefreshable> xRefresh(m_xMasterContainer,UNO_QUERY);
            if ( xRefresh.is() )
                xRefresh->refresh();
            m_pRefreshListener->refresh(this);
        }
    }
    // -------------------------------------------------------------------------
    sal_Bool OFilteredContainer::isNameValid(   const ::rtl::OUString& _rName,
                                            const Sequence< ::rtl::OUString >& _rTableFilter,
                                            const Sequence< ::rtl::OUString >& /*_rTableTypeFilter*/,
                                            const ::std::vector< WildCard >& _rWCSearch) const
    {
        sal_Int32 nTableFilterLen = _rTableFilter.getLength();

        const ::rtl::OUString* tableFilter = _rTableFilter.getConstArray();
        const ::rtl::OUString* tableFilterEnd = _rTableFilter.getConstArray() + nTableFilterLen;
        bool bFilterMatch = ::std::find( tableFilter, tableFilterEnd, _rName ) != tableFilterEnd;
        // the table is allowed to "pass" if we had no filters at all or any of the non-wildcard filters matches
        if (!bFilterMatch && !_rWCSearch.empty())
        {   // or if one of the wildcrad expression matches
            String sWCCompare = (const sal_Unicode*)_rName;
            for (   ::std::vector< WildCard >::const_iterator aLoop = _rWCSearch.begin();
                    aLoop != _rWCSearch.end() && !bFilterMatch;
                    ++aLoop
                )
                bFilterMatch = aLoop->Matches(sWCCompare);
        }

        return bFilterMatch;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString OFilteredContainer::getNameForObject(const ObjectType& _xObject)
    {
        OSL_ENSURE(_xObject.is(),"OTables::getNameForObject: Object is NULL!");
        return ::dbtools::composeTableName( m_xMetaData, _xObject, ::dbtools::eInDataManipulation, false, false, false );
    }
// ..............................................................................
} // namespace
// ..............................................................................


