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

#include "viewcontainer.hxx"
#include "dbastrings.hrc"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include "View.hxx"

#include <tools/debug.hxx>
#include <tools/wldcrd.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>

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

OViewContainer::OViewContainer(::cppu::OWeakObject& _rParent
                                 ,::osl::Mutex& _rMutex
                                 ,const Reference< XConnection >& _xCon
                                 ,sal_Bool _bCase
                                 ,IRefreshListener* _pRefreshListener
                                 ,::dbtools::IWarningsContainer* _pWarningsContainer
                                 ,oslInterlockedCount& _nInAppend)
    :OFilteredContainer(_rParent,_rMutex,_xCon,_bCase,_pRefreshListener,_pWarningsContainer,_nInAppend)
    ,m_bInElementRemoved(false)
{
    DBG_CTOR(OViewContainer, NULL);
}

OViewContainer::~OViewContainer()
{
    //  dispose();
    DBG_DTOR(OViewContainer, NULL);
}

// XServiceInfo
IMPLEMENT_SERVICE_INFO2(OViewContainer, "com.sun.star.sdb.dbaccess.OViewContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)

ObjectType OViewContainer::createObject(const ::rtl::OUString& _rName)
{
    ObjectType xProp;
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
        return new View(m_xConnection,
                        isCaseSensitive(),
                        sCatalog,
                        sSchema,
                        sTable
                        );
    }

    return xProp;
}

Reference< XPropertySet > OViewContainer::createDescriptor()
{
    Reference< XPropertySet > xRet;
    // first we have to look if the master tables support this
    // and if so then create a table object as well with the master tables
    Reference<XColumnsSupplier > xMasterColumnsSup;
    Reference<XDataDescriptorFactory> xDataFactory(m_xMasterContainer,UNO_QUERY);
    if(xDataFactory.is())
        xRet = xDataFactory->createDataDescriptor();
    else
        xRet = new ::connectivity::sdbcx::OView(isCaseSensitive(),m_xMetaData);

    return xRet;
}

// XAppend
ObjectType OViewContainer::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    // append the new table with a create stmt
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));

    Reference<XAppend> xAppend(m_xMasterContainer,UNO_QUERY);
    Reference< XPropertySet > xProp = descriptor;
    if(xAppend.is())
    {
        EnsureReset aReset(m_nInAppend);

        xAppend->appendByDescriptor(descriptor);
        if(m_xMasterContainer->hasByName(aName))
            xProp.set(m_xMasterContainer->getByName(aName),UNO_QUERY);
    }
    else
    {
        ::rtl::OUString sComposedName = ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::eInTableDefinitions, false, false, true );
        if(!sComposedName.getLength())
            ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)));

        ::rtl::OUString sCommand;
        descriptor->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;

        ::rtl::OUStringBuffer aSQL;
        aSQL.appendAscii( "CREATE VIEW " );
        aSQL.append     ( sComposedName );
        aSQL.appendAscii( " AS " );
        aSQL.append     ( sCommand );

        Reference<XConnection> xCon = m_xConnection;
        OSL_ENSURE(xCon.is(),"Connection is null!");
        if ( xCon.is() )
        {
            ::utl::SharedUNOComponent< XStatement > xStmt( xCon->createStatement() );
            if ( xStmt.is() )
                xStmt->execute( aSQL.makeStringAndClear() );
        }
    }

    return createObject( _rForName );
}

// XDrop
void OViewContainer::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if ( !m_bInElementRemoved )
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

                sComposedName = ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_True, ::dbtools::eInTableDefinitions );
            }

            if(!sComposedName.getLength())
                ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)));

            ::rtl::OUString aSql(RTL_CONSTASCII_USTRINGPARAM("DROP VIEW "));
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
}

void SAL_CALL OViewContainer::elementInserted( const ContainerEvent& Event ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString sName;
    if  (   ( Event.Accessor >>= sName )
        &&  ( !m_nInAppend )
        &&  ( !hasByName( sName ) )
        )
    {
        Reference<XPropertySet> xProp(Event.Element,UNO_QUERY);
        ::rtl::OUString sType;
        xProp->getPropertyValue(PROPERTY_TYPE) >>= sType;
        if ( sType == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW")) )
            insertElement(sName,createObject(sName));
    }
}

void SAL_CALL OViewContainer::elementRemoved( const ContainerEvent& Event ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString sName;
    if ( (Event.Accessor >>= sName) && hasByName(sName) )
    {
        m_bInElementRemoved = true;
        try
        {
            dropByName(sName);
        }
        catch(Exception&)
        {
            m_bInElementRemoved = sal_False;
            throw;
        }
        m_bInElementRemoved = false;
    }
}

void SAL_CALL OViewContainer::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw (RuntimeException)
{
}

void SAL_CALL OViewContainer::elementReplaced( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
{
}

::rtl::OUString OViewContainer::getTableTypeRestriction() const
{
    // no restriction at all (other than the ones provided externally)
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VIEW" ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
