/*************************************************************************
 *
 *  $RCSfile: BViews.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-29 07:03:17 $
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

#include "adabas/BViews.hxx"
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_CHECKOPTION_HPP_
#include <com/sun/star/sdbcx/CheckOption.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE adabas
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#include "connectivity/sdbcx/VView.hxx"
#endif

using namespace cppu;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Reference< XNamed > OViews::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aSchema;
    sal_Int32 nLen = _rName.indexOf('.');
    aSchema = _rName.copy(0,nLen);
    aName   = _rName.copy(nLen+1);

    ::rtl::OUString sStmt = ::rtl::OUString::createFromAscii("SELECT DISTINCT DOMAIN.VIEWDEFS.DEFINITION FROM DOMAIN.VIEWDEFS WHERE DOMAIN.VIEWDEFS.OWNER = '");
    sStmt += aSchema;
    sStmt += ::rtl::OUString::createFromAscii("' AND DOMAIN.VIEWDEFS.VIEWNAME = '");
    sStmt += aName;
    sStmt += ::rtl::OUString::createFromAscii("'");
    Reference< XStatement > xStmt = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->createStatement(  );
    Reference< XResultSet > xResult = xStmt->executeQuery(sStmt);

    Reference< XNamed > xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one table with this name
        {
            connectivity::sdbcx::OView* pRet = new connectivity::sdbcx::OView(sal_True,aName,CheckOption::NONE,xRow->getString(1),aSchema);
            xRet = pRet;
        }
    }
    ::comphelper::disposeComponent(xResult);
    ::comphelper::disposeComponent(xStmt);

    return xRet;
}
// -------------------------------------------------------------------------
void OViews::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OAdabasCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OViews::disposing(void)
{
    m_xMetaData = NULL;
    OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OViews::createEmptyObject()
{
    connectivity::sdbcx::OView* pNew = new connectivity::sdbcx::OView(sal_True);
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OViews::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    ObjectMap::iterator aIter = m_aNameMap.find(aName);
    if( aIter != m_aNameMap.end())
        throw ElementExistException(aName,*this);
    if(!aName.getLength())
        ::dbtools::FunctionSequenceException(*this);

    createView(descriptor);
    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OViews::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    if(!aIter->second.is()) // we want to drop a object which isn't loaded yet so we must load it
        aIter->second = createObject(elementName);
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(aIter->second.get(),UNO_QUERY);
    sal_Bool bIsNew = sal_False;
    if(xTunnel.is())
    {
        connectivity::sdbcx::ODescriptor* pTable = (connectivity::sdbcx::ODescriptor*)xTunnel->getSomething(connectivity::sdbcx::ODescriptor::getUnoTunnelImplementationId());
        if(pTable)
            bIsNew = pTable->isNew();
    }
    if (!bIsNew)
    {
        OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );

        ::rtl::OUString aName,aSchema;
        sal_Int32 nLen = elementName.indexOf('.');
        aSchema = elementName.copy(0,nLen);
        aName   = elementName.copy(nLen+1);
        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP VIEW");

        aSql = aSql + m_xMetaData->getIdentifierQuoteString(  ) + aSchema + m_xMetaData->getIdentifierQuoteString(  );
        aSql = aSql + ::rtl::OUString::createFromAscii(".");
        aSql = aSql + m_xMetaData->getIdentifierQuoteString(  ) + aName + m_xMetaData->getIdentifierQuoteString(  );
        xStmt->execute(aSql);
    }

    OCollection_TYPE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OViews::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException();

    dropByName((*m_aElements[index]).first);
}
// -----------------------------------------------------------------------------
void OViews::createView( const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE VIEW ");
    ::rtl::OUString aQuote  = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
    ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii("."),sSchema,sCommand;

    descriptor->getPropertyValue(PROPERTY_SCHEMANAME) >>= sSchema;
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + aDot;
    else
        descriptor->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getUserName()));

    aSql += ::dbtools::quoteName(aQuote, getString(descriptor->getPropertyValue(PROPERTY_NAME)))
                + ::rtl::OUString::createFromAscii(" AS ");
    descriptor->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
    aSql += sCommand;

    OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );
    xStmt->execute(aSql);

    // insert the new view also in the tables collection
    OViews* pTables = static_cast<OViews*>(static_cast<OAdabasCatalog&>(m_rParent).getPrivateTables());
    if(pTables)
    {
        ::rtl::OUString sName = sSchema;
        sName += aDot;
        sName += getString(descriptor->getPropertyValue(PROPERTY_NAME));
        pTables->appendNew(sName);
    }
}
// -----------------------------------------------------------------------------
void OViews::appendNew(const ::rtl::OUString& _rsNewTable)
{
    m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(_rsNewTable,WeakReference< XNamed >())));
        // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rsNewTable), Any(), Any());
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}
// -----------------------------------------------------------------------------


