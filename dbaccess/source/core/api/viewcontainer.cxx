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

#include "viewcontainer.hxx"
#include "dbastrings.hrc"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include "View.hxx"

#include <tools/debug.hxx>
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

// OViewContainer
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
    DBG_DTOR(OViewContainer, NULL);
}

// XServiceInfo
IMPLEMENT_SERVICE_INFO2(OViewContainer, "com.sun.star.sdb.dbaccess.OViewContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)

ObjectType OViewContainer::createObject(const OUString& _rName)
{
    ObjectType xProp;
    if ( m_xMasterContainer.is() && m_xMasterContainer->hasByName(_rName) )
        xProp.set(m_xMasterContainer->getByName(_rName),UNO_QUERY);

    if ( !xProp.is() )
    {
        OUString sCatalog,sSchema,sTable;
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
ObjectType OViewContainer::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    // append the new table with a create stmt
    OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));

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
        OUString sComposedName = ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::eInTableDefinitions, false, false, true );
        if(sComposedName.isEmpty())
            ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)));

        OUString sCommand;
        descriptor->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;

        OUStringBuffer aSQL;
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
void OViewContainer::dropObject(sal_Int32 _nPos,const OUString _sElementName)
{
    if ( !m_bInElementRemoved )
    {
        Reference< XDrop > xDrop(m_xMasterContainer,UNO_QUERY);
        if(xDrop.is())
            xDrop->dropByName(_sElementName);
        else
        {
            OUString sCatalog,sSchema,sTable,sComposedName;

            Reference<XPropertySet> xTable(getObject(_nPos),UNO_QUERY);
            if ( xTable.is() )
            {
                xTable->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
                xTable->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
                xTable->getPropertyValue(PROPERTY_NAME)         >>= sTable;

                sComposedName = ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_True, ::dbtools::eInTableDefinitions );
            }

            if(sComposedName.isEmpty())
                ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)));

            OUString aSql("DROP VIEW ");
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
    OUString sName;
    if  (   ( Event.Accessor >>= sName )
        &&  ( !m_nInAppend )
        &&  ( !hasByName( sName ) )
        )
    {
        Reference<XPropertySet> xProp(Event.Element,UNO_QUERY);
        OUString sType;
        xProp->getPropertyValue(PROPERTY_TYPE) >>= sType;
        if ( sType == OUString("VIEW") )
            insertElement(sName,createObject(sName));
    }
}

void SAL_CALL OViewContainer::elementRemoved( const ContainerEvent& Event ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    OUString sName;
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

OUString OViewContainer::getTableTypeRestriction() const
{
    // no restriction at all (other than the ones provided externally)
    return OUString( "VIEW"  );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
