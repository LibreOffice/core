/*************************************************************************
 *
 *  $RCSfile: tablecontainer.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-07 15:26:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBA_CORE_TABLECONTAINER_HXX_
#include "tablecontainer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_TABLE_HXX_
#include "table.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

using namespace dbaccess;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

//==========================================================================
//= OTableContainer
//==========================================================================
DBG_NAME(OTableContainer)
//------------------------------------------------------------------------------
OTableContainer::OTableContainer(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xCon)
    :m_rParent(_rParent)
    ,m_rMutex(_rMutex)
    ,m_bConstructed(sal_False)
    ,m_xConnection(_xCon)
{
    DBG_CTOR(OTableContainer, NULL);
}

//------------------------------------------------------------------------------
OTableContainer::~OTableContainer()
{
    dispose();
    DBG_DTOR(OTableContainer, NULL);
}

//------------------------------------------------------------------------------
/** compare two strings
*/
extern int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
NameCompare( const void* pFirst, const void* pSecond)
{
    return reinterpret_cast< const ::rtl::OUString* >(pFirst)->compareTo(*reinterpret_cast< const ::rtl::OUString* >(pSecond));
}
// -------------------------------------------------------------------------
void OTableContainer::construct(const Reference< XNameAccess >& _rxMasterContainer,
                                const Sequence< ::rtl::OUString >& _rTableFilter,
                                const Sequence< ::rtl::OUString >& _rTableTypeFilter)
{
    m_xMasterTables = _rxMasterContainer;

    sal_Int32   nTableFilterLen = _rTableFilter.getLength();

    sal_Bool bNoTableFilters = ((nTableFilterLen == 1) && _rTableFilter[0].equalsAsciiL("%", 1));
    if(!bNoTableFilters)
    {
        Sequence< ::rtl::OUString > aTableFilter        = _rTableFilter;
        Sequence< ::rtl::OUString > aTableTypeFilter    = _rTableTypeFilter;
        // build sorted versions of the filter sequences, so the visibility decision is faster
        qsort(aTableFilter.getArray(), nTableFilterLen, sizeof(::rtl::OUString), NameCompare);

        // as we want to modify nTableFilterLen, remember this

        // for wildcard search : remove all table filters which are a wildcard expression and build a WilCard
        // for them
        ::std::vector< WildCard > aWCSearch; // contains the wildcards for the table filter
        ::rtl::OUString* pTableFilters = aTableFilter.getArray();
        sal_Int32 nShiftPos = 0;
        String sCurrentWCExpression;
        for (sal_Int32 i=0; i<nTableFilterLen; ++i)
        {
            if (pTableFilters->indexOf('%') != -1)
            {
                sCurrentWCExpression = sal_Unicode('*');
                sCurrentWCExpression += (const sal_Unicode*)pTableFilters[i].replace('%', '*');
                sCurrentWCExpression += sal_Unicode('*');
                aWCSearch.push_back(WildCard(sCurrentWCExpression));
            }
            else
            {
                if (nShiftPos != i)
                    pTableFilters[nShiftPos] = pTableFilters[i];
                ++nShiftPos;
            }
        }
        // now aTableFilter contains nShiftPos non-wc-strings and aWCSearch all wc-strings
        aTableFilter.realloc(nShiftPos);
        nTableFilterLen = nShiftPos;

        Sequence< ::rtl::OUString> aNames = m_xMasterTables->getElementNames();
        const ::rtl::OUString* pBegin   = aNames.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if(isNameValid(*pBegin,aTableFilter,aTableTypeFilter,aWCSearch))
            {
                m_aTablesIndexed.push_back(m_aTables.insert(Tables::value_type(*pBegin, NULL)).first);
            }
        }
    }
    else
    {
        // no filter so insert all names
        Sequence< ::rtl::OUString> aNames = m_xMasterTables->getElementNames();
        const ::rtl::OUString* pBegin   = aNames.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
            m_aTablesIndexed.push_back(m_aTables.insert(Tables::value_type(*pBegin, NULL)).first);
    }
    m_bConstructed = sal_True;
}
//------------------------------------------------------------------------------
void OTableContainer::construct(const Sequence< ::rtl::OUString >& _rTableFilter, const Sequence< ::rtl::OUString >& _rTableTypeFilter)
{
    // build sorted versions of the filter sequences, so the visibility decision is faster
    Sequence< ::rtl::OUString > aTableFilter(_rTableFilter);
    sal_Int32   nTableFilterLen = aTableFilter.getLength();

    if (nTableFilterLen)
        qsort(aTableFilter.getArray(), nTableFilterLen, sizeof(::rtl::OUString), NameCompare);

    sal_Bool bNoTableFilters = ((nTableFilterLen == 1) && _rTableFilter[0].equalsAsciiL("%", 1));
        // as we want to modify nTableFilterLen, remember this

    // for wildcard search : remove all table filters which are a wildcard expression and build a WilCard
    // for them
    ::rtl::OUString* pTableFilters = aTableFilter.getArray();
    ::std::vector< WildCard > aWCSearch;
    sal_Int32 nShiftPos = 0;
    String sCurrentWCExpression;
    for (sal_Int32 i=0; i<nTableFilterLen; ++i)
    {
        if (pTableFilters->indexOf('%') != -1)
        {
            sCurrentWCExpression = sal_Unicode('*');
            sCurrentWCExpression += (const sal_Unicode*)pTableFilters[i].replace('%', '*');
            sCurrentWCExpression += sal_Unicode('*');
            aWCSearch.push_back(WildCard(sCurrentWCExpression));
        }
        else
        {
            if (nShiftPos != i)
                pTableFilters[nShiftPos] = pTableFilters[i];
            ++nShiftPos;
        }
    }
    // now aTableFilter contains nShiftPos non-wc-strings and aWCSearch all wc-strings
    aTableFilter.realloc(nShiftPos);
    nTableFilterLen = nShiftPos;

    try
    {
        Reference< XDatabaseMetaData > xMetaData = m_xConnection.is() ? m_xConnection->getMetaData() : Reference< XDatabaseMetaData >();
        if (xMetaData.is())
        {
            const ::rtl::OUString sAll = ::rtl::OUString::createFromAscii("%");
            Reference< XResultSet > xTables = xMetaData->getTables(Any(), sAll, sAll, _rTableTypeFilter);
            Reference< XRow > xCurrentRow(xTables, UNO_QUERY);
            if (xCurrentRow.is())
            {

                    // after creation the set is positioned before the first record, per definitionem

                ::rtl::OUString sCatalog, sSchema, sName, sType;
                ::rtl::OUString sComposedName;

                // we first collect the names and construct the OTable objects later, as the ctor of the table may need
                // another result set from the connection, and some drivers support only one statement per connection
                ::std::vector< ::rtl::OUString > aCatalogs, aSchemas, aNames, aTypes, aDescs, aComposedNames;

                String sWCCompare;
                sal_Bool bFilterMatch;
                while (xTables->next())
                {
                    sCatalog    = xCurrentRow->getString(1);
                    sSchema     = xCurrentRow->getString(2);
                    sName       = xCurrentRow->getString(3);
                    // we're not interested in the "wasNull", as the getStrings would return an empty string in
                    // that case, which is sufficient here

                    composeTableName(xMetaData, sCatalog, sSchema, sName, sComposedName, sal_False);
                    bFilterMatch =  bNoTableFilters
                                ||  ((nTableFilterLen != 0) && (NULL != bsearch(&sComposedName, aTableFilter.getConstArray(), nTableFilterLen, sizeof(::rtl::OUString), NameCompare)));
                    // the table is allowed to "pass" if we had no filters at all or any of the non-wildcard filters matches

                    if (!bFilterMatch && aWCSearch.size())
                    {   // or if one of the wildcrad expression matches
                        sWCCompare += (const sal_Unicode*)sComposedName;
                        for (   ::std::vector< WildCard >::const_iterator aLoop = aWCSearch.begin();
                                aLoop != aWCSearch.end() && !bFilterMatch;
                                ++aLoop
                            )
                            bFilterMatch = aLoop->Matches(sWCCompare);
                    }

                    if (bFilterMatch)
                    {   // the table name is allowed (not filtered out)
                        aCatalogs.push_back(sCatalog);
                        aSchemas.push_back(sSchema);
                        aNames.push_back(sName);
                        aTypes.push_back(xCurrentRow->getString(4));
                        aDescs.push_back(xCurrentRow->getString(5));
                        aComposedNames.push_back(sComposedName);
                    }
                }

                // dispose the tables result set, in case the connection can handle only one concurrent statement
                // (the table object creation will need it's own statements)
                disposeComponent(xTables);
                for (sal_Int32 i=0; i<aCatalogs.size(); ++i)
                {
                    Reference<XPropertySet> xTable;
                    try
                    {   // the ctor is allowed to throw an exception
                        xTable = new ODBTable(m_xConnection, NULL,aCatalogs[i], aSchemas[i], aNames[i], aTypes[i], aDescs[i]);
                    }
                    catch(SQLException&)
                    {
                    }
                    if (xTable.is())
                        m_aTablesIndexed.push_back(m_aTables.insert(Tables::value_type(aComposedNames[i], xTable)).first);
                }
            }
            else
                DBG_ERROR("OTableContainer::construct : did not get a XRow from the tables result set !");
        }
        else
            DBG_ERROR("OTableContainer::construct : no connection meta data !");
    }
    catch (SQLException&)
    {
        DBG_ERROR("OTableContainer::construct : catched an SQL-Exception !");
        dispose();
        return;
    }

    m_bConstructed = sal_True;
}

//------------------------------------------------------------------------------
void OTableContainer::dispose()
{
    MutexGuard aGuard(m_rMutex);
//  for (ConstTablesIterator i = m_aTables.begin(); i != m_aTables.end(); ++i)
//  {
//      i->second->dispose();
//      i->second->release();
//  }
    m_aTablesIndexed.clear();
        //  !!! do this before clearing the map which the vector elements refer to !!!
    m_aTables.clear();
    m_xMasterTables = NULL;
    m_xConnection   = NULL;
    m_bConstructed  = sal_False;
}

// XServiceInfo
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2(OTableContainer, "com.sun.star.sdb.dbaccess.OTableContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)

// ::com::sun::star::container::XElementAccess
//------------------------------------------------------------------------------
Type OTableContainer::getElementType(  ) throw(RuntimeException)
{
    return::getCppuType(static_cast<Reference<XPropertySet>*>(NULL));
}
//------------------------------------------------------------------------------
sal_Bool OTableContainer::hasElements(void) throw( RuntimeException )
{
    return getCount() != 0;
}

// ::com::sun::star::container::XEnumerationAccess
//------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XEnumeration >  OTableContainer::createEnumeration(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_rMutex);
    return new OEnumerationByName( static_cast< ::com::sun::star::container::XNameAccess* >(this));
}

// ::com::sun::star::container::XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 OTableContainer::getCount(void) throw( RuntimeException )
{
    return m_aTablesIndexed.size();
}

//------------------------------------------------------------------------------
Any OTableContainer::getByIndex(sal_Int32 _nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if ((_nIndex < 0) || (_nIndex > m_aTablesIndexed.size()))
        throw IndexOutOfBoundsException();

    Reference< XPropertySet > xReturn = m_aTablesIndexed[_nIndex]->second;
    if(!xReturn.is()) // special case
    {
        OSL_ENSHURE(m_xMasterTables.is(),"getByIndex: m_xMasterTables must be set!");
        Reference<XPropertySet> xProp;
        m_xMasterTables->getByName(m_aTablesIndexed[_nIndex]->first) >>= xProp;
        Reference<XColumnsSupplier > xSup(xProp,UNO_QUERY);

        xReturn = new ODBTable(m_xConnection,
                            xSup,
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_CATALOGNAME)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_SCHEMANAME)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_NAME)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_TYPE)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_DESCRIPTION)));
        m_aTablesIndexed[_nIndex]->second = xReturn;
    }
    return makeAny(xReturn);
}

// ::com::sun::star::container::XNameAccess
//------------------------------------------------------------------------------
Any OTableContainer::getByName(const rtl::OUString& _rName) throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    TablesIterator aPos = m_aTables.find(_rName);
    if (aPos == m_aTables.end())
        throw NoSuchElementException();

    Reference< XPropertySet > xReturn = aPos->second;
    if(!xReturn.is()) // special case
    {
        OSL_ENSHURE(m_xMasterTables.is(),"getByName: m_xMasterTables must be set!");
        Reference<XPropertySet> xProp;
        m_xMasterTables->getByName(_rName) >>= xProp;
        Reference<XColumnsSupplier > xSup(xProp,UNO_QUERY);

        xReturn = new ODBTable(m_xConnection,
                            xSup,
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_CATALOGNAME)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_SCHEMANAME)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_NAME)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_TYPE)),
                            comphelper::getString(xProp->getPropertyValue(PROPERTY_DESCRIPTION)));

        aPos->second = xReturn;
    }
    return makeAny(xReturn);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OTableContainer::hasByName( const ::rtl::OUString& _rName ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_aTables.find(_rName) != m_aTables.end();
}

//------------------------------------------------------------------------------
Sequence< rtl::OUString > OTableContainer::getElementNames(void) throw( RuntimeException )
{
    Sequence< rtl::OUString > aReturn(m_aTables.size());
    ::rtl::OUString* pReturn = aReturn.getArray();

    for (   ConstTablesIterator aLoop = m_aTables.begin();
            aLoop != m_aTables.end();
            ++aLoop, ++pReturn
        )
    {
        *pReturn = aLoop->first;
    }

    return aReturn;
}
// -------------------------------------------------------------------------
sal_Bool OTableContainer::isNameValid(  const ::rtl::OUString& _rName,
                                        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
                                        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter,
                                        const ::std::vector< WildCard >& _rWCSearch) const
{
    sal_Int32 nTableFilterLen = _rTableFilter.getLength();

    sal_Bool bFilterMatch = (NULL != bsearch(&_rName, _rTableFilter.getConstArray(), nTableFilterLen, sizeof(::rtl::OUString), NameCompare));
    // the table is allowed to "pass" if we had no filters at all or any of the non-wildcard filters matches
    if (!bFilterMatch && _rWCSearch.size())
    {   // or if one of the wildcrad expression matches
        String sWCCompare = (const sal_Unicode*)_rName;
        for (   ::std::vector< WildCard >::const_iterator aLoop = _rWCSearch.begin();
                aLoop != _rWCSearch.end() && !bFilterMatch;
                ++aLoop
            )
            bFilterMatch = aLoop->Matches(sWCCompare);
    }

    if (bFilterMatch)
    {// the table name is allowed (not filtered out)
        // no type filter
        if(!_rTableTypeFilter.getLength())
            return sal_True;

        // this is expensive but there is no other way to get the type of the table
        Reference<XPropertySet> xTable;
        m_xMasterTables->getByName(_rName) >>= xTable;
        ::rtl::OUString aTypeName;
        xTable->getPropertyValue(PROPERTY_TYPE) >>= aTypeName;
        const ::rtl::OUString* pTypeBegin   = _rTableTypeFilter.getConstArray();
        const ::rtl::OUString* pTypeEnd     = pTypeBegin + _rTableTypeFilter.getLength();
        for(;pTypeBegin != pTypeEnd;++pTypeBegin)
        {
            if(*pTypeBegin == aTypeName)
                return sal_True; // same as break and then checking
        }
    }
    return sal_False;
}
// -------------------------------------------------------------------------


