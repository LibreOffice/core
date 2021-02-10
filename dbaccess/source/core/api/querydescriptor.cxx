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

#include "querydescriptor.hxx"
#include <apitools.hxx>
#include <stringconstants.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::cppu;

namespace dbaccess
{

// OQueryDescriptor

OQueryDescriptor::OQueryDescriptor()
    :OQueryDescriptor_Base(m_aMutex,*this)
    ,ODataSettings(m_aBHelper,true)
{
    registerProperties();
    ODataSettings::registerPropertiesFor(this);
}

OQueryDescriptor::OQueryDescriptor(const OQueryDescriptor_Base& _rSource)
    :OQueryDescriptor_Base(_rSource,*this)
    ,ODataSettings(m_aBHelper,true)
{
    registerProperties();
    ODataSettings::registerPropertiesFor(this);
}

OQueryDescriptor::~OQueryDescriptor()
{
}

css::uno::Sequence<sal_Int8> OQueryDescriptor::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

IMPLEMENT_GETTYPES2(OQueryDescriptor,OQueryDescriptor_Base,ODataSettings);
IMPLEMENT_FORWARD_XINTERFACE3( OQueryDescriptor,OWeakObject,OQueryDescriptor_Base,ODataSettings)

void OQueryDescriptor::registerProperties()
{
    // the properties which OCommandBase supplies (it has no own registration, as it's not derived from
    // an OPropertyStateContainer)
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND|PropertyAttribute::CONSTRAINED,
                    &m_sElementName, cppu::UnoType<decltype(m_sElementName)>::get());

    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &m_sCommand, cppu::UnoType<decltype(m_sCommand)>::get());

    registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &m_bEscapeProcessing, cppu::UnoType<bool>::get());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &m_sUpdateTableName, cppu::UnoType<decltype(m_sUpdateTableName)>::get());

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &m_sUpdateSchemaName, cppu::UnoType<decltype(m_sUpdateSchemaName)>::get());

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &m_sUpdateCatalogName, cppu::UnoType<decltype(m_sUpdateCatalogName)>::get());

    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &m_aLayoutInformation, cppu::UnoType<decltype(m_aLayoutInformation)>::get());
}

Reference< XPropertySetInfo > SAL_CALL OQueryDescriptor::getPropertySetInfo(  )
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


OQueryDescriptor_Base::OQueryDescriptor_Base(::osl::Mutex&  _rMutex,::cppu::OWeakObject& _rMySelf)
    :m_bColumnsOutOfDate(true)
    ,m_rMutex(_rMutex)
{
    m_pColumns.reset( new OColumns(_rMySelf, m_rMutex, true,std::vector< OUString>(), this,this) );
}

OQueryDescriptor_Base::OQueryDescriptor_Base(const OQueryDescriptor_Base& _rSource,::cppu::OWeakObject& _rMySelf)
    :m_bColumnsOutOfDate(true)
    ,m_rMutex(_rSource.m_rMutex)
{
    m_pColumns.reset( new OColumns(_rMySelf, m_rMutex, true,std::vector< OUString>(), this,this) );

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
}

sal_Int64 SAL_CALL OQueryDescriptor_Base::getSomething( const Sequence< sal_Int8 >& _rIdentifier )
{
    if (isUnoTunnelId<OQueryDescriptor_Base>(_rIdentifier))
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}

css::uno::Sequence<sal_Int8> OQueryDescriptor_Base::getUnoTunnelId()
{
    static cppu::OImplementationId aId;
    return aId.getImplementationId();
}

css::uno::Sequence<sal_Int8> OQueryDescriptor_Base::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void OQueryDescriptor_Base::setColumnsOutOfDate( bool _bOutOfDate )
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

Reference< XNameAccess > SAL_CALL OQueryDescriptor_Base::getColumns( )
{
    MutexGuard aGuard(m_rMutex);

    if ( m_bColumnsOutOfDate )
    {
        // clear the current columns
        clearColumns();

        // do this before rebuildColumns. This prevents recursion, e.g. in the case where we
        // have queries with cyclic references:
        // foo := SELECT * FROM bar
        // bar := SELECT * FROM foo
        setColumnsOutOfDate( false );

        // rebuild them
        try
        {
            rebuildColumns();
        }
        catch ( const Exception& )
        {
            setColumnsOutOfDate();
            throw;
        }
    }

    return m_pColumns.get();
}

OUString SAL_CALL OQueryDescriptor_Base::getImplementationName(  )
{
    return "com.sun.star.sdb.OQueryDescriptor";
}

sal_Bool SAL_CALL OQueryDescriptor_Base::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL OQueryDescriptor_Base::getSupportedServiceNames(  )
{
    return { SERVICE_SDB_DATASETTINGS, SERVICE_SDB_QUERYDESCRIPTOR };
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
    return nullptr;
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

rtl::Reference<OColumn> OQueryDescriptor_Base::createColumn( const OUString& /*_rName*/ ) const
{
    // creating a column/descriptor for a query/descriptor does not make sense at all
    return nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
