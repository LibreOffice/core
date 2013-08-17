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

#include <string.h>

#include "querydescriptor.hxx"
#include "apitools.hxx"
#include "dbastrings.hrc"
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include "definitioncolumn.hxx"
#include <tools/debug.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;

namespace dbaccess
{

// OQueryDescriptor
DBG_NAME(OQueryDescriptor)

OQueryDescriptor::OQueryDescriptor()
    :OQueryDescriptor_Base(m_aMutex,*this)
    ,ODataSettings(m_aBHelper,sal_True)
{
    DBG_CTOR(OQueryDescriptor,NULL);
    registerProperties();
    ODataSettings::registerPropertiesFor(this);
}

OQueryDescriptor::OQueryDescriptor(const OQueryDescriptor_Base& _rSource)
    :OQueryDescriptor_Base(_rSource,*this)
    ,ODataSettings(m_aBHelper,sal_True)
{
    DBG_CTOR(OQueryDescriptor,NULL);
    registerProperties();
    ODataSettings::registerPropertiesFor(this);
}

OQueryDescriptor::~OQueryDescriptor()
{
    DBG_DTOR(OQueryDescriptor,NULL);
}

IMPLEMENT_TYPEPROVIDER2(OQueryDescriptor,OQueryDescriptor_Base,ODataSettings);
IMPLEMENT_FORWARD_XINTERFACE3( OQueryDescriptor,OWeakObject,OQueryDescriptor_Base,ODataSettings)

void OQueryDescriptor::registerProperties()
{
    // the properties which OCommandBase supplies (it has no own registration, as it's not derived from
    // a OPropertyStateContainer)
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND|PropertyAttribute::CONSTRAINED,
                    &m_sElementName, ::getCppuType(&m_sElementName));

    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &m_sCommand, ::getCppuType(&m_sCommand));

    registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &m_bEscapeProcessing, ::getBooleanCppuType());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &m_sUpdateTableName, ::getCppuType(&m_sUpdateTableName));

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &m_sUpdateSchemaName, ::getCppuType(&m_sUpdateSchemaName));

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &m_sUpdateCatalogName, ::getCppuType(&m_sUpdateCatalogName));

    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &m_aLayoutInformation, ::getCppuType(&m_aLayoutInformation));
}

Reference< XPropertySetInfo > SAL_CALL OQueryDescriptor::getPropertySetInfo(  ) throw(RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

::cppu::IPropertyArrayHelper& OQueryDescriptor::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* OQueryDescriptor::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

DBG_NAME(OQueryDescriptor_Base);

OQueryDescriptor_Base::OQueryDescriptor_Base(::osl::Mutex&  _rMutex,::cppu::OWeakObject& _rMySelf)
    :m_bColumnsOutOfDate(sal_True)
    ,m_rMutex(_rMutex)
{
    DBG_CTOR(OQueryDescriptor_Base,NULL);
    m_pColumns = new OColumns(_rMySelf, m_rMutex, sal_True,::std::vector< OUString>(), this,this);
}

OQueryDescriptor_Base::OQueryDescriptor_Base(const OQueryDescriptor_Base& _rSource,::cppu::OWeakObject& _rMySelf)
    :m_bColumnsOutOfDate(sal_True)
    ,m_rMutex(_rSource.m_rMutex)
{
    DBG_CTOR(OQueryDescriptor_Base,NULL);
    m_pColumns = new OColumns(_rMySelf, m_rMutex, sal_True,::std::vector< OUString>(), this,this);

    m_sCommand = _rSource.m_sCommand;
    m_bEscapeProcessing = _rSource.m_bEscapeProcessing;
    m_sUpdateTableName = _rSource.m_sUpdateTableName;
    m_sUpdateSchemaName = _rSource.m_sUpdateSchemaName;
    m_sUpdateCatalogName = _rSource.m_sUpdateCatalogName;
    m_aLayoutInformation = _rSource.m_aLayoutInformation;
}

OQueryDescriptor_Base::~OQueryDescriptor_Base()
{
    m_pColumns->acquire();
    m_pColumns->disposing();
    delete m_pColumns;

    DBG_DTOR(OQueryDescriptor_Base,NULL);
}

sal_Int64 SAL_CALL OQueryDescriptor_Base::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException)
{
    if (_rIdentifier.getLength() != 16)
        return 0;

    if (0 == memcmp(getImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}

IMPLEMENT_IMPLEMENTATION_ID(OQueryDescriptor_Base)

void OQueryDescriptor_Base::setColumnsOutOfDate( sal_Bool _bOutOfDate )
{
    m_bColumnsOutOfDate = _bOutOfDate;
    if ( !m_bColumnsOutOfDate )
        m_pColumns->setInitialized();
}

void OQueryDescriptor_Base::implAppendColumn( const OUString& _rName, OColumn* _pColumn )
{
    m_pColumns->append( _rName, _pColumn );
}

void OQueryDescriptor_Base::clearColumns( )
{
    m_pColumns->clearColumns();

    setColumnsOutOfDate();
}

Reference< XNameAccess > SAL_CALL OQueryDescriptor_Base::getColumns( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);

    if ( isColumnsOutOfDate() )
    {
        // clear the current columns
        clearColumns();

        // do this before rebuildColumns. This prevents recursion, e.g. in the case where we
        // have queries with cyclic references:
        // foo := SELECT * FROM bar
        // bar := SELECT * FROM foo
        setColumnsOutOfDate( sal_False );

        // rebuild them
        try
        {
            rebuildColumns();
        }
        catch ( const Exception& )
        {
            setColumnsOutOfDate( sal_True );
            throw;
        }
    }

    return m_pColumns;
}

OUString SAL_CALL OQueryDescriptor_Base::getImplementationName(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.sdb.OQueryDescriptor");
}

sal_Bool SAL_CALL OQueryDescriptor_Base::supportsService( const OUString& _rServiceName ) throw(RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

Sequence< OUString > SAL_CALL OQueryDescriptor_Base::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< OUString > aSupported(2);
    aSupported.getArray()[0] = SERVICE_SDB_DATASETTINGS;
    aSupported.getArray()[1] = SERVICE_SDB_QUERYDESCRIPTOR;
    return aSupported;
}

void OQueryDescriptor_Base::disposeColumns()
{
    m_pColumns->disposing();
}

void OQueryDescriptor_Base::columnAppended( const Reference< XPropertySet >& /*_rxSourceDescriptor*/ )
{
    // not interested in
}

void OQueryDescriptor_Base::columnDropped(const OUString& /*_sName*/)
{
    // not interested in
}

Reference< XPropertySet > OQueryDescriptor_Base::createColumnDescriptor()
{
    OSL_FAIL( "OQueryDescriptor_Base::createColumnDescriptor: called why?" );
    return NULL;
}

void OQueryDescriptor_Base::rebuildColumns( )
{
}

// IRefreshableColumns
void OQueryDescriptor_Base::refreshColumns()
{
    MutexGuard aGuard( m_rMutex );

    clearColumns();
    rebuildColumns();
}

OColumn* OQueryDescriptor_Base::createColumn( const OUString& /*_rName*/ ) const
{
    // creating a column/descriptor for a query/descriptor does not make sense at all
    return NULL;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
