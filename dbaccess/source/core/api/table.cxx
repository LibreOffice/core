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

#include <table.hxx>
#include <definitioncolumn.hxx>
#include <stringconstants.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include "CIndexes.hxx"

#include <osl/diagnose.h>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XRefreshListener.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdb/tools/XTableRename.hpp>
#include <com/sun/star/sdb/tools/XTableAlteration.hpp>

#include <connectivity/TKeys.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>

#include <sdbcoretools.hxx>
#include <ContainerMediator.hxx>

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

// ODBTable

ODBTable::ODBTable(connectivity::sdbcx::OCollection* _pTables
        ,const Reference< XConnection >& _rxConn
        ,const OUString& _rCatalog
        ,const OUString& _rSchema
        ,const OUString& _rName
        ,const OUString& _rType
        ,const OUString& _rDesc
        ,const Reference< XNameAccess >& _xColumnDefinitions)
    :OTable_Base(_pTables,_rxConn,_rxConn->getMetaData().is() && _rxConn->getMetaData()->supportsMixedCaseQuotedIdentifiers(), _rName, _rType, _rDesc, _rSchema, _rCatalog )
    ,m_xColumnDefinitions(_xColumnDefinitions)
    ,m_nPrivileges(0)
{
    OSL_ENSURE(getMetaData().is(), "ODBTable::ODBTable : invalid conn !");
    OSL_ENSURE(!_rName.isEmpty(), "ODBTable::ODBTable : name !");
    // TODO : think about collecting the privileges here, as we can't ensure that in getFastPropertyValue, where
    // we do this at the moment, the statement needed can be supplied by the connection (for example the SQL-Server
    // ODBC driver does not allow more than one statement per connection, and in getFastPropertyValue it's more
    // likely that it's already used up than it's here.)
}

ODBTable::ODBTable(connectivity::sdbcx::OCollection* _pTables
                   ,const Reference< XConnection >& _rxConn)
    :OTable_Base(_pTables,_rxConn, _rxConn->getMetaData().is() && _rxConn->getMetaData()->supportsMixedCaseQuotedIdentifiers())
    ,m_nPrivileges(-1)
{
}

ODBTable::~ODBTable()
{
}

OColumn* ODBTable::createColumn(const OUString& _rName) const
{
    OColumn* pReturn = nullptr;

    Reference<XPropertySet> xProp;
    if ( m_xDriverColumns.is() && m_xDriverColumns->hasByName(_rName) )
    {
        xProp.set(m_xDriverColumns->getByName(_rName),UNO_QUERY);
    }
    else
    {
        OColumns* pColumns = static_cast<OColumns*>(m_xColumns.get());
        xProp.set(pColumns->createBaseObject(_rName),UNO_QUERY);
    }

    Reference<XPropertySet> xColumnDefintion;
    if ( m_xColumnDefinitions.is() && m_xColumnDefinitions->hasByName(_rName) )
        xColumnDefintion.set(m_xColumnDefinitions->getByName(_rName),UNO_QUERY);
    pReturn = new OTableColumnWrapper( xProp, xColumnDefintion, false );

    return pReturn;
}

void ODBTable::columnAppended( const Reference< XPropertySet >& /*_rxSourceDescriptor*/ )
{
    // not interested in
}

void ODBTable::columnDropped(const OUString& _sName)
{
    Reference<XDrop> xDrop(m_xColumnDefinitions,UNO_QUERY);
    if ( xDrop.is() && m_xColumnDefinitions->hasByName(_sName) )
    {
        xDrop->dropByName(_sName);
    }
}

Sequence< sal_Int8 > ODBTable::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// OComponentHelper
void SAL_CALL ODBTable::disposing()
{
    OPropertySetHelper::disposing();
    OTable_Base::disposing();
    m_xColumnDefinitions = nullptr;
    m_xDriverColumns = nullptr;
    m_pColumnMediator = nullptr;
}

void ODBTable::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    if ((PROPERTY_ID_PRIVILEGES == _nHandle) && (-1 == m_nPrivileges))
    {   // somebody is asking for the privileges and we do not know them, yet
        const_cast<ODBTable*>(this)->m_nPrivileges = ::dbtools::getTablePrivileges(getMetaData(),m_CatalogName,m_SchemaName, m_Name);
    }

    OTable_Base::getFastPropertyValue(_rValue, _nHandle);
}

void ODBTable::construct()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // we don't collect the privileges here, this is potentially expensive. Instead we determine them on request.
    // (see getFastPropertyValue)
    m_nPrivileges = -1;

    OTable_Base::construct();

    registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
                    &m_sFilter, cppu::UnoType<OUString>::get());

    registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
                    &m_sOrder, cppu::UnoType<OUString>::get());

    registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
                    &m_bApplyFilter, cppu::UnoType<bool>::get());

    registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
                    &m_aFont, cppu::UnoType<css::awt::FontDescriptor>::get());

    registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aRowHeight, cppu::UnoType<sal_Int32>::get());

    registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aTextColor, cppu::UnoType<sal_Int32>::get());

    registerProperty(PROPERTY_PRIVILEGES, PROPERTY_ID_PRIVILEGES, PropertyAttribute::BOUND | PropertyAttribute::READONLY,
                    &m_nPrivileges, cppu::UnoType<sal_Int32>::get());

    registerMayBeVoidProperty(PROPERTY_TEXTLINECOLOR, PROPERTY_ID_TEXTLINECOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aTextLineColor, cppu::UnoType<sal_Int32>::get());

    registerProperty(PROPERTY_TEXTEMPHASIS, PROPERTY_ID_TEXTEMPHASIS, PropertyAttribute::BOUND,
                    &m_nFontEmphasis, cppu::UnoType<sal_Int16>::get());

    registerProperty(PROPERTY_TEXTRELIEF, PROPERTY_ID_TEXTRELIEF, PropertyAttribute::BOUND,
                    &m_nFontRelief, cppu::UnoType<sal_Int16>::get());

    registerProperty(PROPERTY_FONTNAME,         PROPERTY_ID_FONTNAME,        PropertyAttribute::BOUND,&m_aFont.Name, cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_FONTHEIGHT,       PROPERTY_ID_FONTHEIGHT,      PropertyAttribute::BOUND,&m_aFont.Height, cppu::UnoType<sal_Int16>::get());
    registerProperty(PROPERTY_FONTWIDTH,        PROPERTY_ID_FONTWIDTH,       PropertyAttribute::BOUND,&m_aFont.Width, cppu::UnoType<sal_Int16>::get());
    registerProperty(PROPERTY_FONTSTYLENAME,    PROPERTY_ID_FONTSTYLENAME,   PropertyAttribute::BOUND,&m_aFont.StyleName, cppu::UnoType<OUString>::get());
    registerProperty(PROPERTY_FONTFAMILY,       PROPERTY_ID_FONTFAMILY,      PropertyAttribute::BOUND,&m_aFont.Family, cppu::UnoType<sal_Int16>::get());
    registerProperty(PROPERTY_FONTCHARSET,      PROPERTY_ID_FONTCHARSET,     PropertyAttribute::BOUND,&m_aFont.CharSet, cppu::UnoType<sal_Int16>::get());
    registerProperty(PROPERTY_FONTPITCH,        PROPERTY_ID_FONTPITCH,       PropertyAttribute::BOUND,&m_aFont.Pitch, cppu::UnoType<sal_Int16>::get());
    registerProperty(PROPERTY_FONTCHARWIDTH,    PROPERTY_ID_FONTCHARWIDTH,   PropertyAttribute::BOUND,&m_aFont.CharacterWidth, cppu::UnoType<float>::get());
    registerProperty(PROPERTY_FONTWEIGHT,       PROPERTY_ID_FONTWEIGHT,      PropertyAttribute::BOUND,&m_aFont.Weight, cppu::UnoType<float>::get());
    registerProperty(PROPERTY_FONTSLANT,        PROPERTY_ID_FONTSLANT,       PropertyAttribute::BOUND,&m_aFont.Slant, cppu::UnoType<css::awt::FontSlant>::get());
    registerProperty(PROPERTY_FONTUNDERLINE,    PROPERTY_ID_FONTUNDERLINE,   PropertyAttribute::BOUND,&m_aFont.Underline, cppu::UnoType<sal_Int16>::get());
    registerProperty(PROPERTY_FONTSTRIKEOUT,    PROPERTY_ID_FONTSTRIKEOUT,   PropertyAttribute::BOUND,&m_aFont.Strikeout, cppu::UnoType<sal_Int16>::get());
    registerProperty(PROPERTY_FONTORIENTATION,  PROPERTY_ID_FONTORIENTATION, PropertyAttribute::BOUND,&m_aFont.Orientation, cppu::UnoType<float>::get());
    registerProperty(PROPERTY_FONTKERNING,      PROPERTY_ID_FONTKERNING,     PropertyAttribute::BOUND,&m_aFont.Kerning, cppu::UnoType<sal_Bool>::get());
    registerProperty(PROPERTY_FONTWORDLINEMODE, PROPERTY_ID_FONTWORDLINEMODE,PropertyAttribute::BOUND,&m_aFont.WordLineMode, cppu::UnoType<sal_Bool>::get());
    registerProperty(PROPERTY_FONTTYPE,         PROPERTY_ID_FONTTYPE,        PropertyAttribute::BOUND,&m_aFont.Type, cppu::UnoType<sal_Int16>::get());

    refreshColumns();
}

::cppu::IPropertyArrayHelper* ODBTable::createArrayHelper( sal_Int32 _nId) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    if(!_nId)
    {
        for(Property & prop : aProps)
        {
            if (prop.Name == PROPERTY_CATALOGNAME)
                prop.Attributes = PropertyAttribute::READONLY;
            else if (prop.Name == PROPERTY_SCHEMANAME)
                prop.Attributes = PropertyAttribute::READONLY;
            else if (prop.Name == PROPERTY_DESCRIPTION)
                prop.Attributes = PropertyAttribute::READONLY;
            else if (prop.Name == PROPERTY_NAME)
                prop.Attributes = PropertyAttribute::READONLY;
        }
    }

    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper & SAL_CALL ODBTable::getInfoHelper()
{
    return *ODBTable_PROP::getArrayHelper(isNew() ? 1 : 0);
}

// XServiceInfo
IMPLEMENT_SERVICE_INFO1(ODBTable, "com.sun.star.sdb.dbaccess.ODBTable", SERVICE_SDBCX_TABLE)

Any SAL_CALL ODBTable::queryInterface( const Type & rType )
{
    if(rType == cppu::UnoType<XRename>::get()&& !getRenameService().is() )
        return Any();
    if(rType == cppu::UnoType<XAlterTable>::get()&& !getAlterService().is() )
        return Any();
    return OTable_Base::queryInterface( rType);
}

Sequence< Type > SAL_CALL ODBTable::getTypes(  )
{
    Type aRenameType = cppu::UnoType<XRename>::get();
    Type aAlterType = cppu::UnoType<XAlterTable>::get();

    Sequence< Type > aTypes(OTable_Base::getTypes());
    std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());

    const Type* pIter = aTypes.getConstArray();
    const Type* pEnd = pIter + aTypes.getLength();
    for(;pIter != pEnd ;++pIter)
    {
        if( (*pIter != aRenameType || getRenameService().is()) && (*pIter != aAlterType || getAlterService().is()))
            aOwnTypes.push_back(*pIter);
    }

    return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
}

// XRename,
void SAL_CALL ODBTable::rename( const OUString& _rNewName )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed);
    if ( !getRenameService().is() )
        throw SQLException(DBA_RES(RID_STR_NO_TABLE_RENAME),*this,SQLSTATE_GENERAL,1000,Any() );

    Reference<XPropertySet> xTable(this);
    getRenameService()->rename(xTable,_rNewName);
    ::connectivity::OTable_TYPEDEF::rename(_rNewName);
}

// XAlterTable,
void SAL_CALL ODBTable::alterColumnByName( const OUString& _rName, const Reference< XPropertySet >& _rxDescriptor )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed);
    if ( !getAlterService().is() )
        throw SQLException(DBA_RES(RID_STR_NO_TABLE_RENAME),*this,SQLSTATE_GENERAL,1000,Any() );

    if ( !m_xColumns->hasByName(_rName) )
        throw SQLException(DBA_RES(RID_STR_COLUMN_NOT_VALID),*this,SQLSTATE_GENERAL,1000,Any() );

    Reference<XPropertySet> xTable(this);
    getAlterService()->alterColumnByName(xTable,_rName,_rxDescriptor);
    m_xColumns->refresh();
}

sal_Int64 SAL_CALL ODBTable::getSomething( const Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet(0);
    if (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        nRet = reinterpret_cast<sal_Int64>(this);
    else
        nRet = OTable_Base::getSomething(rId);

    return nRet;
}

Sequence< sal_Int8 > ODBTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId s_Id;

    return s_Id.getImplementationId();
}

Reference< XPropertySet > ODBTable::createColumnDescriptor()
{
    return new OTableColumnDescriptor( true );
}

sdbcx::OCollection* ODBTable::createColumns(const ::std::vector< OUString>& _rNames)
{
    Reference<XDatabaseMetaData> xMeta = getMetaData();
    OColumns* pCol = new OColumns(*this, m_aMutex, nullptr, isCaseSensitive(), _rNames, this,this,
                                    getAlterService().is() || (xMeta.is() && xMeta->supportsAlterTableWithAddColumn()),
                                    getAlterService().is() || (xMeta.is() && xMeta->supportsAlterTableWithDropColumn()));
    static_cast<OColumnsHelper*>(pCol)->setParent(this);
    pCol->setParent(*this);
    m_pColumnMediator = new OContainerMediator( pCol, m_xColumnDefinitions );
    pCol->setMediator( m_pColumnMediator.get() );
    return pCol;
}

sdbcx::OCollection* ODBTable::createKeys(const ::std::vector< OUString>& _rNames)
{
    return new connectivity::OKeysHelper(this,m_aMutex,_rNames);
}

sdbcx::OCollection* ODBTable::createIndexes(const ::std::vector< OUString>& _rNames)
{
    return new OIndexes(this,m_aMutex,_rNames,nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
