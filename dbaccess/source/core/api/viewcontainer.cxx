/*************************************************************************
 *
 *  $RCSfile: viewcontainer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-29 07:06:28 $
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

#ifndef _DBA_CORE_VIEWCONTAINER_HXX_
#include "viewcontainer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
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
//= OViewContainer
//==========================================================================
DBG_NAME(OViewContainer)
//------------------------------------------------------------------------------
OViewContainer::OViewContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 IWarningsContainer* _pWarningsContainer)
    :OCollection(_rParent,_xCon->getMetaData()->storesMixedCaseQuotedIdentifiers(),_rMutex,::std::vector< ::rtl::OUString>())
    ,m_bConstructed(sal_False)
    ,m_xConnection(_xCon)
    ,m_xMetaData(_xCon->getMetaData())
    ,m_pWarningsContainer(_pWarningsContainer)
{
    DBG_CTOR(OViewContainer, NULL);
}

//------------------------------------------------------------------------------
OViewContainer::~OViewContainer()
{
    //  dispose();
    DBG_DTOR(OViewContainer, NULL);
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
NameCompare( const void* pFirst, const void* pSecond);
// -------------------------------------------------------------------------
void OViewContainer::construct(const Reference< XNameAccess >& _rxMasterContainer,
                                const Sequence< ::rtl::OUString >& _rTableFilter,
                                const Sequence< ::rtl::OUString >& _rTableTypeFilter)
{
    m_xMasterViews = _rxMasterContainer;

    if(m_xMasterViews.is())
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

            Sequence< ::rtl::OUString> aNames = m_xMasterViews->getElementNames();
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
            Sequence< ::rtl::OUString> aNames = m_xMasterViews->getElementNames();
            const ::rtl::OUString* pBegin   = aNames.getConstArray();
            const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
            for(;pBegin != pEnd;++pBegin)
                m_aElements.push_back(m_aNameMap.insert(ObjectMap::value_type(*pBegin, NULL)).first);
        }
        m_bConstructed = sal_True;
    }
}
//------------------------------------------------------------------------------
void OViewContainer::disposing()
{
    MutexGuard aGuard(m_rMutex);
    OCollection::disposing();

    m_xMasterViews = NULL;
    m_xMetaData     = NULL;
    m_bConstructed  = sal_False;
}
// XServiceInfo
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2(OViewContainer, "com.sun.star.sdb.dbaccess.OViewContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)
// -------------------------------------------------------------------------
sal_Bool OViewContainer::isNameValid(   const ::rtl::OUString& _rName,
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
        ::cppu::extractInterface(xTable,m_xMasterViews->getByName(_rName));
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
void OViewContainer::impl_refresh() throw(RuntimeException)
{
}
// -----------------------------------------------------------------------------
Reference< XNamed > OViewContainer::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(m_xMasterViews.is(),"getByName: m_xMasterViews must be set!");
    Reference< XNamed > xProp;
    if(m_xMasterViews.is() && m_xMasterViews->hasByName(_rName))
        m_xMasterViews->getByName(_rName) >>= xProp;

    return xProp;
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OViewContainer::createEmptyObject()
{
    Reference< XPropertySet > xRet;
    // frist we have to look if the master tables does support this
    // and if then create a table object as well with the master tables
    Reference<XColumnsSupplier > xMasterColumnsSup;
    Reference<XDataDescriptorFactory> xDataFactory(m_xMasterViews,UNO_QUERY);
    if(xDataFactory.is())
        return xDataFactory->createDataDescriptor();

    return xRet;
}
// -----------------------------------------------------------------------------
// XAppend
void SAL_CALL OViewContainer::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    // append the new table with a create stmt
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    ObjectMap::iterator aIter = m_aNameMap.find(aName);
    if( aIter != m_aNameMap.end() || (m_xMasterViews.is() && m_xMasterViews->hasByName(aName)))
        throw ElementExistException(aName,*this);

    Reference<XAppend> xAppend(m_xMasterViews,UNO_QUERY);
    Reference< XPropertySet > xProp = descriptor;
    if(xAppend.is())
    {
        xAppend->appendByDescriptor(descriptor);
        if(m_xMasterViews->hasByName(aName))
            m_xMasterViews->getByName(aName) >>= xProp;
    }
    else
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE VIEW ");
        ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;

        if(m_xMetaData->supportsCatalogsInTableDefinitions())
            descriptor->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
        if(m_xMetaData->supportsSchemasInTableDefinitions())
            descriptor->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;

        descriptor->getPropertyValue(PROPERTY_NAME)         >>= sTable;

        ::dbtools::composeTableName(m_xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_True);
        if(!sComposedName.getLength())
            throw ::dbtools::FunctionSequenceException(*this);

        aSql += sComposedName + ::rtl::OUString::createFromAscii(" AS ");
        ::rtl::OUString sCommand;
        descriptor->getPropertyValue(PROPERTY_NAME)         >>= sCommand;
        aSql += sCommand;


        OSL_ENSURE(m_xConnection.is(),"Connection is null!");
        Reference< XStatement > xStmt = m_xConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
    }

    OCollection::appendByDescriptor(xProp);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OViewContainer::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    Reference< XDrop > xDrop(m_xMasterViews,UNO_QUERY);
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

        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP VIEW ");
        aSql += sComposedName;
        Reference< XStatement > xStmt = m_xConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
    }
    OCollection::dropByName(elementName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OViewContainer::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException();

    dropByName((*m_aElements[index]).first);
}
// -------------------------------------------------------------------------

