/*************************************************************************
 *
 *  $RCSfile: tablecontainer.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:19:18 $
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
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
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
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace dbaccess;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::connectivity::sdbcx;

//==========================================================================
//= OTableContainer
//==========================================================================
DBG_NAME(OTableContainer)
//------------------------------------------------------------------------------
OTableContainer::OTableContainer(const OConfigurationNode& _rTablesConfig,
                                 const OConfigurationTreeRoot& _rCommitLocation,
                                 ::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 IWarningsContainer* _pWarningsContainer)
    :OCollection(_rParent,_xCon->getMetaData()->storesMixedCaseQuotedIdentifiers(),_rMutex,::std::vector< ::rtl::OUString>())
    ,m_bConstructed(sal_False)
    ,m_xConnection(_xCon)
    ,m_xMetaData(_xCon->getMetaData())
    ,m_aCommitLocation(_rCommitLocation)
    ,m_aTablesConfig(_rTablesConfig)
    ,m_pWarningsContainer(_pWarningsContainer)
{
    DBG_CTOR(OTableContainer, NULL);
    m_aTablesConfig.setEscape(m_aTablesConfig.isSetNode());
}

//------------------------------------------------------------------------------
OTableContainer::~OTableContainer()
{
    //  dispose();
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

    if(m_xMasterTables.is())
    {
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
                    m_aElements.push_back(m_aNameMap.insert(ObjectMap::value_type(*pBegin, NULL)).first);
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
                m_aElements.push_back(m_aNameMap.insert(ObjectMap::value_type(*pBegin, NULL)).first);
        }
        m_bConstructed = sal_True;
    }
    else
    {
        construct(_rTableFilter,_rTableTypeFilter);
    }
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
        if (m_xMetaData.is())
        {
            const ::rtl::OUString sAll = ::rtl::OUString::createFromAscii("%");
            Reference< XResultSet > xTables = m_xMetaData->getTables(Any(), sAll, sAll, _rTableTypeFilter);
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

                    composeTableName(m_xMetaData, sCatalog, sSchema, sName, sComposedName, sal_False);
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

                const ::rtl::OUString* pCatalogs = aCatalogs.begin();
                const ::rtl::OUString* pSchemas = aSchemas.begin();
                const ::rtl::OUString* pNames = aNames.begin();
                const ::rtl::OUString* pTypes = aTypes.begin();
                const ::rtl::OUString* pDescs = aDescs.begin();
                const ::rtl::OUString* pComposedNames = aComposedNames.begin();

                const ::rtl::OUString* pCatalogsEnd = aCatalogs.end();

                for (; pCatalogs < pCatalogsEnd; ++pCatalogs, ++pSchemas, ++pNames, ++pTypes, ++pDescs, ++pComposedNames)
                {
                    Reference<XNamed> xTable;
                    OConfigurationNode aTableConfig;
                    if(m_aTablesConfig.isValid())
                    {
                        if(m_aTablesConfig.hasByName(*pComposedNames))
                            aTableConfig = m_aTablesConfig.openNode(*pComposedNames);
                        else
                        {
                            aTableConfig = m_aTablesConfig.createNode(*pComposedNames);
                            m_aCommitLocation.commit();
                        }
                    }
                    try
                    {   // the ctor is allowed to throw an exception
                        xTable = new ODBTable(aTableConfig,m_xMetaData, NULL, *pCatalogs, *pSchemas, *pNames, *pTypes, *pDescs);
                    }
                    catch(SQLException& e)
                    {
                        if (m_pWarningsContainer)
                        {
                            SQLContext aContext;
                            String sMessage(::dba::ResourceManager::loadString(RID_STR_TRIED_OPEN_TABLE));
                            composeTableName(m_xMetaData, *pCatalogs, *pSchemas, *pNames, sComposedName, sal_False);
                            sMessage.SearchAndReplaceAscii("$name$", sComposedName);
                            aContext.Message = sMessage;

                            aContext.NextException <<= e;
                            m_pWarningsContainer->appendWarning(aContext);
                        }
                    }
                    if (xTable.is())
                        m_aElements.push_back(m_aNameMap.insert(ObjectMap::value_type(*pComposedNames, xTable)).first);
                    else if(m_aTablesConfig.isValid())
                    {
                        m_aTablesConfig.removeNode(*pComposedNames);
                        m_aCommitLocation.commit();
                    }

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
        disposing();
        return;
    }

    m_bConstructed = sal_True;
}

//------------------------------------------------------------------------------
void OTableContainer::disposing()
{
    MutexGuard aGuard(m_rMutex);
    OCollection::disposing();

//  m_aElements.clear();
//      //  !!! do this before clearing the map which the vector elements refer to !!!
//  m_aNameMap.clear();
    m_xMasterTables = NULL;
    m_xMetaData     = NULL;
    m_bConstructed  = sal_False;
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableContainer::flush(  ) throw(RuntimeException)
{
    for (ObjectIter i = m_aNameMap.begin(); i != m_aNameMap.end(); ++i)
    {
        if((*i).second.is())
        {
            Reference< ::com::sun::star::util::XFlushable > xFlush((*i).second, UNO_QUERY);
            if(xFlush.is())
                xFlush->flush();
        }
    }

}
// XServiceInfo
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2(OTableContainer, "com.sun.star.sdb.dbaccess.OTableContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)
// -------------------------------------------------------------------------
sal_Bool OTableContainer::isNameValid(  const ::rtl::OUString& _rName,
                                        const Sequence< ::rtl::OUString >& _rTableFilter,
                                        const Sequence< ::rtl::OUString >& _rTableTypeFilter,
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
        ::cppu::extractInterface(xTable,m_xMasterTables->getByName(_rName));
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
void OTableContainer::impl_refresh() throw(RuntimeException)
{
}
// -----------------------------------------------------------------------------
Reference< XNamed > OTableContainer::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSHURE(m_xMasterTables.is(),"getByName: m_xMasterTables must be set!");
    Reference<XPropertySet> xProp;
    if(m_xMasterTables.is() && m_xMasterTables->hasByName(_rName))
        m_xMasterTables->getByName(_rName) >>= xProp;
    Reference<XColumnsSupplier > xSup(xProp,UNO_QUERY);

    ::rtl::OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData,
                                        _rName,
                                        sCatalog,
                                        sSchema,
                                        sTable);

    OConfigurationNode aTableConfig;
    if(m_aTablesConfig.isValid())
    {
        if(m_aTablesConfig.hasByName(_rName))
            aTableConfig = m_aTablesConfig.openNode(_rName);
        else
        {
            aTableConfig = m_aTablesConfig.createNode(_rName);
            m_aCommitLocation.commit();
        }
    }

    if(xProp.is())
        return new ODBTable(aTableConfig,
                            m_xMetaData,
                            xSup,
                            sCatalog,
                            sSchema,
                            sTable,
                            ::comphelper::getString(xProp->getPropertyValue(PROPERTY_TYPE)),
                            ::comphelper::getString(xProp->getPropertyValue(PROPERTY_DESCRIPTION)));
    else
    {
        Any aCatalog;
        if(sCatalog.getLength())
            aCatalog <<= sCatalog;
        ::rtl::OUString sType,sDescription;
        Reference< XResultSet > xRes = m_xMetaData->getTables(aCatalog,sSchema,sTable,Sequence< ::rtl::OUString>());
        if(xRes.is() && xRes->next())
        {
            Reference< XRow > xRow(xRes,UNO_QUERY);
            if(xRow.is())
            {
                sType           = xRow->getString(4);
                sDescription    = xRow->getString(5);
            }
        }
        ::comphelper::disposeComponent(xRes);
        return new ODBTable(aTableConfig,
                            m_xMetaData,
                            xSup,
                            sCatalog,
                            sSchema,
                            sTable,
                            sType,
                            sDescription);
    }
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OTableContainer::createEmptyObject()
{
    Reference< XPropertySet > xRet;
    // frist we have to look if the master tables does support this
    // and if then create a table object as well with the master tables
    Reference<XColumnsSupplier > xMasterColumnsSup;
    Reference<XDataDescriptorFactory> xDataFactory(m_xMasterTables,UNO_QUERY);
    if(xDataFactory.is())
        xMasterColumnsSup = Reference<XColumnsSupplier >(xDataFactory->createDataDescriptor(),UNO_QUERY);

    xRet = new ODBTable(m_xMetaData,xMasterColumnsSup);
    return xRet;
}
// -----------------------------------------------------------------------------
// XAppend
void SAL_CALL OTableContainer::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    // append the new table with a create stmt
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    ObjectMap::iterator aIter = m_aNameMap.find(aName);
    if( aIter != m_aNameMap.end() || (m_xMasterTables.is() && m_xMasterTables->hasByName(aName)))
        throw ElementExistException(aName,*this);

    Reference<XAppend> xAppend(m_xMasterTables,UNO_QUERY);
    if(xAppend.is())
    {
        xAppend->appendByDescriptor(descriptor);
    }
    else
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE TABLE ");
        ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;

        if(m_xMetaData->supportsCatalogsInTableDefinitions())
            descriptor->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
        if(m_xMetaData->supportsSchemasInTableDefinitions())
            descriptor->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;

        descriptor->getPropertyValue(PROPERTY_NAME)         >>= sTable;

        ::dbtools::composeTableName(m_xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_True);
        if(!sComposedName.getLength())
            throw ::dbtools::FunctionSequenceException(*this);

        aSql += sComposedName + ::rtl::OUString::createFromAscii(" (");

        // columns
        Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
        Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
        // check if there are columns
        if(!xColumns.is() || !xColumns->getCount())
            throw ::dbtools::FunctionSequenceException(*this);

        Reference< XPropertySet > xColProp;

        ::rtl::OUString sTypeName;
        sal_Int32       nDataType   = 0;
        sal_Int32       nPrecision  = 0;
        sal_Int32       nScale      = 0;
        for(sal_Int32 i=0;i<xColumns->getCount();++i)
        {
            if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
            {

                aSql += ::dbtools::quoteTableName(m_xMetaData,::comphelper::getString(xColProp->getPropertyValue(PROPERTY_NAME)));

                aSql += ::rtl::OUString::createFromAscii(" ");

                nDataType = nPrecision = nScale = 0;
                xColProp->getPropertyValue(PROPERTY_TYPENAME)   >>= sTypeName;
                xColProp->getPropertyValue(PROPERTY_TYPE)       >>= nDataType;
                xColProp->getPropertyValue(PROPERTY_PRECISION)  >>= nPrecision;
                xColProp->getPropertyValue(PROPERTY_SCALE)      >>= nScale;

                aSql += sTypeName;
                if(nPrecision > 0)
                {
                    aSql += ::rtl::OUString::createFromAscii("(");
                    aSql += ::rtl::OUString::valueOf(nPrecision);
                    if(nScale > 0)
                    {
                        aSql += ::rtl::OUString::createFromAscii(",");
                        aSql += ::rtl::OUString::valueOf(nScale);
                    }
                    aSql += ::rtl::OUString::createFromAscii(")");
                }

                ::rtl::OUString aDefault = ::comphelper::getString(xColProp->getPropertyValue(PROPERTY_DEFAULTVALUE));
                if(aDefault.getLength())
                    aSql += ::rtl::OUString::createFromAscii(" DEFAULT ") + aDefault;

                if(::comphelper::getINT32(xColProp->getPropertyValue(PROPERTY_ISNULLABLE)) == ColumnValue::NO_NULLS)
                    aSql += ::rtl::OUString::createFromAscii(" NOT NULL");

                aSql += ::rtl::OUString::createFromAscii(",");
            }
        }
        //  aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

        // keys

        Reference<XKeysSupplier> xKeySup(descriptor,UNO_QUERY);

        Reference<XIndexAccess> xKeys = xKeySup->getKeys();
        if(xKeys.is())
        {
            sal_Bool bPKey = sal_False;
            for(sal_Int32 i=0;i<xKeys->getCount();++i)
            {
                if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                {

                    sal_Int32 nKeyType      = ::comphelper::getINT32(xColProp->getPropertyValue(PROPERTY_TYPE));

                    if(nKeyType == KeyType::PRIMARY)
                    {
                        if(!bPKey)
                            throw ::dbtools::FunctionSequenceException(*this);

                        bPKey = sal_True;
                        xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                        xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                        if(!xColumns.is() || !xColumns->getCount())
                            throw ::dbtools::FunctionSequenceException(*this);

                        aSql += ::rtl::OUString::createFromAscii(" PRIMARY KEY (");
                        for(sal_Int32 i=0;i<xColumns->getCount();++i)
                        {
                            if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                                aSql += ::dbtools::quoteTableName(m_xMetaData,::comphelper::getString(xColProp->getPropertyValue(PROPERTY_NAME)))
                                        +   ::rtl::OUString::createFromAscii(",");
                        }

                        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
                    }
                    else if(nKeyType == KeyType::UNIQUE)
                    {
                        xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                        xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                        if(!xColumns.is() || !xColumns->getCount())
                            throw ::dbtools::FunctionSequenceException(*this);

                        aSql += ::rtl::OUString::createFromAscii(" UNIQUE (");
                        for(sal_Int32 i=0;i<xColumns->getCount();++i)
                        {
                            if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                                aSql += ::dbtools::quoteTableName(m_xMetaData,::comphelper::getString(xColProp->getPropertyValue(PROPERTY_NAME)))
                                     + ::rtl::OUString::createFromAscii(",");
                        }

                        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
                    }
                    else if(nKeyType == KeyType::FOREIGN)
                    {
                        sal_Int32 nDeleteRule   = getINT32(xColProp->getPropertyValue(PROPERTY_DELETERULE));

                        xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                        xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                        if(!xColumns.is() || !xColumns->getCount())
                            throw ::dbtools::FunctionSequenceException(*this);

                        aSql += ::rtl::OUString::createFromAscii(" FOREIGN KEY ");
                        ::rtl::OUString sRefTable = getString(xColProp->getPropertyValue(PROPERTY_REFERENCEDTABLE));
                        ::dbtools::qualifiedNameComponents(m_xMetaData,
                                                            sRefTable,
                                                            sCatalog,
                                                            sSchema,
                                                            sTable);
                        ::dbtools::composeTableName(m_xMetaData,sCatalog, sSchema, sTable,sComposedName,sal_True);


                        if(!sComposedName.getLength())
                            throw ::dbtools::FunctionSequenceException(*this);
                        aSql += sComposedName + ::rtl::OUString::createFromAscii(" (");

                        for(sal_Int32 i=0;i<xColumns->getCount();++i)
                        {
                            if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                                aSql += ::dbtools::quoteTableName(m_xMetaData,::comphelper::getString(xColProp->getPropertyValue(PROPERTY_NAME)))
                                            + ::rtl::OUString::createFromAscii(",");
                        }

                        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

                        switch(nDeleteRule)
                        {
                            case KeyRule::CASCADE:
                                aSql += ::rtl::OUString::createFromAscii(" ON DELETE CASCADE ");
                                break;
                            case KeyRule::RESTRICT:
                                aSql += ::rtl::OUString::createFromAscii(" ON DELETE RESTRICT ");
                                break;
                            case KeyRule::SET_NULL:
                                aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET NULL ");
                                break;
                            case KeyRule::SET_DEFAULT:
                                aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET DEFAULT ");
                                break;
                            default:
                                ;
                        }
                    }
                }
            }
        }

        if(aSql.lastIndexOf(',') == (aSql.getLength()-1))
            aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
        else
            aSql += ::rtl::OUString::createFromAscii(")");

        OSL_ENSURE(m_xConnection.is(),"Connection is null!");
        Reference< XStatement > xStmt = m_xConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
    }


    // create a new config entry
    if(m_aTablesConfig.isValid())
    {
        ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;
        descriptor->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
        descriptor->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
        descriptor->getPropertyValue(PROPERTY_NAME)         >>= sTable;

        ::dbtools::composeTableName(m_xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_False);

        OConfigurationNode aTableConfig;
        if(m_aTablesConfig.hasByName(sComposedName))
            aTableConfig = m_aTablesConfig.openNode(sComposedName);
        else
        {
            aTableConfig = m_aTablesConfig.createNode(sComposedName);
            m_aCommitLocation.commit();
        }
        // here I know that the table is saved
        // to get a table with a valid config node we have to recreate the table
        Reference<XPropertySet> xNewTable(createObject(sComposedName),UNO_QUERY);
        OCollection::appendByDescriptor(xNewTable);
    }
    else
        OCollection::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OTableContainer::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    Reference< XDrop > xDrop(m_xMasterTables,UNO_QUERY);
    if(xDrop.is())
        xDrop->dropByName(elementName);
    else
    {
        ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;

        Reference<XPropertySet> xTable(aIter->second.get(),UNO_QUERY);
        if(xTable.is())
        {
            if(m_xMetaData->supportsCatalogsInTableDefinitions())
                xTable->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
            if(m_xMetaData->supportsSchemasInTableDefinitions())
                xTable->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
            xTable->getPropertyValue(PROPERTY_NAME)         >>= sTable;

            ::dbtools::composeTableName(m_xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_True);
        }

        if(!sComposedName.getLength())
            throw ::dbtools::FunctionSequenceException(*this);

        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP TABLE ");
        aSql += sComposedName;
        Reference< XStatement > xStmt = m_xConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
    }
    OCollection::dropByName(elementName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableContainer::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException();

    dropByName((*m_aElements[index]).first);
}
// -------------------------------------------------------------------------

