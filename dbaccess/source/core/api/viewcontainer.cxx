/*************************************************************************
 *
 *  $RCSfile: viewcontainer.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:05:28 $
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
#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#include <connectivity/sdbcx/VView.hxx>
#endif

using namespace dbaccess;
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

//==========================================================================
//= OViewContainer
//==========================================================================
DBG_NAME(OViewContainer)
//------------------------------------------------------------------------------
OViewContainer::OViewContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 sal_Bool _bCase,
                                 IRefreshListener*  _pRefreshListener,
                                 IWarningsContainer* _pWarningsContainer)
    :OFilteredContainer(_rParent,_rMutex,_xCon,_bCase,_pRefreshListener,_pWarningsContainer)
{
}

//------------------------------------------------------------------------------
OViewContainer::~OViewContainer()
{
    //  dispose();
    DBG_DTOR(OViewContainer, NULL);
}
//------------------------------------------------------------------------------
// XServiceInfo
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2(OViewContainer, "com.sun.star.sdb.dbaccess.OViewContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)
// -----------------------------------------------------------------------------
Reference< XNamed > OViewContainer::createObject(const ::rtl::OUString& _rName)
{
    Reference< XNamed > xProp;
    if ( m_xMasterContainer.is() && m_xMasterContainer->hasByName(_rName) )
        xProp.set(m_xMasterContainer->getByName(_rName),UNO_QUERY);

    if ( !xProp.is() )
    {
        ::rtl::OUString sCatalog,sSchema,sTable;
        ::dbtools::qualifiedNameComponents(m_xMetaData,
                                            _rName,
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::eInDataManipulation);
        return new ::connectivity::sdbcx::OView(isCaseSensitive(),
                                sTable,
                                m_xMetaData,
                                0,
                                ::rtl::OUString(),
                                sSchema,
                                sCatalog
                                );
    }

    return xProp;
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OViewContainer::createEmptyObject()
{
    Reference< XPropertySet > xRet;
    // frist we have to look if the master tables does support this
    // and if then create a table object as well with the master tables
    Reference<XColumnsSupplier > xMasterColumnsSup;
    Reference<XDataDescriptorFactory> xDataFactory(m_xMasterContainer,UNO_QUERY);
    if(xDataFactory.is())
        xRet = xDataFactory->createDataDescriptor();
    else
        xRet = new ::connectivity::sdbcx::OView(isCaseSensitive(),m_xMetaData);

    return xRet;
}
// -----------------------------------------------------------------------------
// XAppend
void OViewContainer::appendObject( const Reference< XPropertySet >& descriptor )
{
    // append the new table with a create stmt
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));

    Reference<XAppend> xAppend(m_xMasterContainer,UNO_QUERY);
    Reference< XPropertySet > xProp = descriptor;
    if(xAppend.is())
    {
        xAppend->appendByDescriptor(descriptor);
        if(m_xMasterContainer->hasByName(aName))
            xProp.set(m_xMasterContainer->getByName(aName),UNO_QUERY);
    }
    else
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE VIEW ");
        ::rtl::OUString sComposedName = ::dbtools::composeTableName(m_xMetaData,descriptor,sal_True,::dbtools::eInTableDefinitions);
        if(!sComposedName.getLength())
            ::dbtools::throwFunctionSequenceException(*this);

        aSql += sComposedName + ::rtl::OUString::createFromAscii(" AS ");
        ::rtl::OUString sCommand;
        descriptor->getPropertyValue(PROPERTY_COMMAND)          >>= sCommand;
        aSql += sCommand;


        Reference<XConnection> xCon = m_xConnection;
        OSL_ENSURE(xCon.is(),"Connection is null!");
        if ( xCon.is() )
        {
            Reference< XStatement > xStmt = xCon->createStatement(  );
            if ( xStmt.is() )
                xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }
}
// -------------------------------------------------------------------------
// XDrop
void OViewContainer::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< XDrop > xDrop(m_xMasterContainer,UNO_QUERY);
    if(xDrop.is())
        xDrop->dropByName(_sElementName);
    else
    {
        ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;

        Reference<XPropertySet> xTable(getObject(_nPos),UNO_QUERY);
        if ( xTable.is() )
        {
            xTable->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
            xTable->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
            xTable->getPropertyValue(PROPERTY_NAME)         >>= sTable;

            ::dbtools::composeTableName(m_xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_True,::dbtools::eInTableDefinitions);
        }

        if(!sComposedName.getLength())
            ::dbtools::throwFunctionSequenceException(*this);

        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP VIEW ");
        aSql += sComposedName;
        Reference<XConnection> xCon = m_xConnection;
        OSL_ENSURE(xCon.is(),"Connection is null!");
        if ( xCon.is() )
        {
            Reference< XStatement > xStmt = xCon->createStatement(  );
            if(xStmt.is())
                xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OViewContainer::elementInserted( const ContainerEvent& Event ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString sName;
    if ( (Event.Accessor >>= sName) && !hasByName(sName) )
    {
        Reference<XPropertySet> xProp(Event.Element,UNO_QUERY);
        ::rtl::OUString sType;
        xProp->getPropertyValue(PROPERTY_TYPE) >>= sType;
        if ( sType == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW")) )
            insertElement(sName,createObject(sName));
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OViewContainer::elementRemoved( const ContainerEvent& Event ) throw (RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OViewContainer::disposing( const ::com::sun::star::lang::EventObject& Source ) throw (RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OViewContainer::elementReplaced( const ContainerEvent& Event ) throw (RuntimeException)
{
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > OViewContainer::getTableTypeFilter(const Sequence< ::rtl::OUString >& _rTableTypeFilter) const
{
    static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));

    if(_rTableTypeFilter.getLength() != 0)
    {
        const ::rtl::OUString* pBegin = _rTableTypeFilter.getConstArray();
        const ::rtl::OUString* pEnd   = pBegin + _rTableTypeFilter.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if ( *pBegin == s_sTableTypeView )
                break;
        }
        if ( pBegin != pEnd )
        { // view are filtered out
            m_bConstructed = sal_True;
            return Sequence< ::rtl::OUString >();
        }
    }
    // we want all catalogues, all schemas, all tables
    Sequence< ::rtl::OUString > sTableTypes(1);
    sTableTypes[0] = s_sTableTypeView;
    return sTableTypes;
}
// -----------------------------------------------------------------------------


