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

#include <bitset>

#include "apitools.hxx"
#include "dbastrings.hrc"
#include "definitioncolumn.hxx"
#include "sdbcoretools.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>

#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::cppu;
using namespace ::comphelper;
using namespace ::osl;
using namespace dbaccess;

namespace
{
    const sal_Int32 HAS_DESCRIPTION            = 0x00000001;
    const sal_Int32 HAS_DEFAULTVALUE           = 0x00000002;
    const sal_Int32 HAS_ROWVERSION             = 0x00000004;
    const sal_Int32 HAS_AUTOINCREMENT_CREATION = 0x00000008;
    const sal_Int32 HAS_CATALOGNAME            = 0x00000010;
    const sal_Int32 HAS_SCHEMANAME             = 0x00000020;
    const sal_Int32 HAS_TABLENAME              = 0x00000040;
}

// OTableColumnDescriptor
IMPLEMENT_FORWARD_XINTERFACE2(OTableColumnDescriptor,OColumn,TXChild)

void OTableColumnDescriptor::impl_registerProperties()
{
    sal_Int32 nDefaultAttr = m_bActAsDescriptor ? 0 : PropertyAttribute::READONLY;

    registerProperty( PROPERTY_TYPENAME, PROPERTY_ID_TYPENAME, nDefaultAttr, &m_aTypeName, cppu::UnoType<decltype(m_aTypeName)>::get() );
    registerProperty( PROPERTY_DESCRIPTION, PROPERTY_ID_DESCRIPTION, nDefaultAttr, &m_aDescription, cppu::UnoType<decltype(m_aDescription)>::get() );
    registerProperty( PROPERTY_DEFAULTVALUE, PROPERTY_ID_DEFAULTVALUE, nDefaultAttr, &m_aDefaultValue, cppu::UnoType<decltype(m_aDefaultValue)>::get() );

    if ( m_bActAsDescriptor )
        registerProperty( PROPERTY_AUTOINCREMENTCREATION, PROPERTY_ID_AUTOINCREMENTCREATION, nDefaultAttr, &m_aAutoIncrementValue, cppu::UnoType<decltype(m_aAutoIncrementValue)>::get() );

    registerProperty( PROPERTY_TYPE, PROPERTY_ID_TYPE, nDefaultAttr, &m_nType, cppu::UnoType<decltype(m_nType)>::get() );
    registerProperty( PROPERTY_PRECISION, PROPERTY_ID_PRECISION, nDefaultAttr, &m_nPrecision, cppu::UnoType<decltype(m_nPrecision)>::get() );
    registerProperty( PROPERTY_SCALE, PROPERTY_ID_SCALE, nDefaultAttr, &m_nScale, cppu::UnoType<decltype(m_nScale)>::get() );
    registerProperty( PROPERTY_ISNULLABLE, PROPERTY_ID_ISNULLABLE, nDefaultAttr, &m_nIsNullable, cppu::UnoType<decltype(m_nIsNullable)>::get() );
    registerProperty( PROPERTY_ISAUTOINCREMENT, PROPERTY_ID_ISAUTOINCREMENT, nDefaultAttr, &m_bAutoIncrement, cppu::UnoType<decltype(m_bAutoIncrement)>::get() );
    registerProperty( PROPERTY_ISROWVERSION, PROPERTY_ID_ISROWVERSION, nDefaultAttr, &m_bRowVersion, cppu::UnoType<decltype(m_bRowVersion)>::get() );
    registerProperty( PROPERTY_ISCURRENCY, PROPERTY_ID_ISCURRENCY, nDefaultAttr, &m_bCurrency, cppu::UnoType<decltype(m_bCurrency)>::get() );

    OColumnSettings::registerProperties( *this );
}

IMPLEMENT_GET_IMPLEMENTATION_ID( OTableColumnDescriptor )

// css::lang::XServiceInfo
OUString OTableColumnDescriptor::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.sdb.OTableColumnDescriptor");
}

Sequence< OUString > OTableColumnDescriptor::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = m_bActAsDescriptor ? OUString(SERVICE_SDBCX_COLUMNDESCRIPTOR) : OUString(SERVICE_SDBCX_COLUMN);
    aSNS[1] = SERVICE_SDB_COLUMNSETTINGS;
    return aSNS;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* OTableColumnDescriptor::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& OTableColumnDescriptor::getInfoHelper()
{
    return *static_cast< ::comphelper::OPropertyArrayUsageHelper< OTableColumnDescriptor >* >(this)->getArrayHelper();
}

void OTableColumnDescriptor::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception, std::exception)
{
    OColumn::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    ::dbaccess::notifyDataSourceModified( m_xParent, true );
}

Reference< XInterface > SAL_CALL OTableColumnDescriptor::getParent(  ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xParent;
}

void SAL_CALL OTableColumnDescriptor::setParent( const Reference< XInterface >& _xParent ) throw (NoSupportException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xParent = _xParent;
}

// OTableColumn

OTableColumn::OTableColumn( const OUString& _rName )
    :OTableColumnDescriptor( false /* do not act as descriptor */ )
{
    m_sName = _rName;
}

OTableColumn::~OTableColumn()
{
}

IMPLEMENT_GET_IMPLEMENTATION_ID( OTableColumn )

OUString OTableColumn::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.sdb.OTableColumn");
}

::cppu::IPropertyArrayHelper& SAL_CALL OTableColumn::getInfoHelper()
{
    return *OTableColumn_PBase::getArrayHelper();
}

::cppu::IPropertyArrayHelper* OTableColumn::createArrayHelper( ) const
{
    return OTableColumnDescriptor::createArrayHelper();
}

// OQueryColumn

OQueryColumn::OQueryColumn( const Reference< XPropertySet >& _rxParserColumn, const Reference< XConnection >& _rxConnection, const OUString &i_sLabel )
    :OTableColumnDescriptor( false /* do not act as descriptor */ )
    ,m_sLabel(i_sLabel)
{
    const sal_Int32 nPropAttr = PropertyAttribute::READONLY;
    registerProperty( PROPERTY_CATALOGNAME, PROPERTY_ID_CATALOGNAME, nPropAttr, &m_sCatalogName, cppu::UnoType<decltype(m_sCatalogName)>::get() );
    registerProperty( PROPERTY_SCHEMANAME, PROPERTY_ID_SCHEMANAME, nPropAttr, &m_sSchemaName, cppu::UnoType<decltype(m_sSchemaName)>::get() );
    registerProperty( PROPERTY_TABLENAME, PROPERTY_ID_TABLENAME, nPropAttr, &m_sTableName, cppu::UnoType<decltype(m_sTableName)>::get() );
    registerProperty( PROPERTY_REALNAME, PROPERTY_ID_REALNAME, nPropAttr, &m_sRealName, cppu::UnoType<decltype(m_sRealName)>::get() );
    registerProperty( PROPERTY_LABEL, PROPERTY_ID_LABEL, nPropAttr, &m_sLabel, cppu::UnoType<decltype(m_sLabel)>::get() );


    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_TYPENAME ) >>= m_aTypeName );
    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_ISNULLABLE ) >>= m_nIsNullable );
    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_PRECISION ) >>= m_nPrecision );
    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_SCALE ) >>= m_nScale );
    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_TYPE ) >>= m_nType );
    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_ISAUTOINCREMENT ) >>= m_bAutoIncrement );
    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_ISCURRENCY ) >>= m_bCurrency );
    OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_NAME ) >>= m_sName );

    m_bRowVersion = false;

    Reference< XPropertySetInfo > xPSI( _rxParserColumn->getPropertySetInfo(), UNO_SET_THROW );
    if ( xPSI->hasPropertyByName( PROPERTY_DEFAULTVALUE ) )
        OSL_VERIFY( _rxParserColumn->getPropertyValue( PROPERTY_DEFAULTVALUE ) >>= m_aDefaultValue );

    // copy some optional properties from the parser column
    struct PropertyDescriptor
    {
        OUString sName;
        sal_Int32       nHandle;
    };
    const PropertyDescriptor aProps[] =
    {
        { OUString(PROPERTY_CATALOGNAME),      PROPERTY_ID_CATALOGNAME },
        { OUString(PROPERTY_SCHEMANAME),       PROPERTY_ID_SCHEMANAME },
        { OUString(PROPERTY_TABLENAME),        PROPERTY_ID_TABLENAME },
        { OUString(PROPERTY_REALNAME),         PROPERTY_ID_REALNAME }
    };
    for (const auto & aProp : aProps)
    {
        if ( xPSI->hasPropertyByName( aProp.sName ) )
            setFastPropertyValue_NoBroadcast( aProp.nHandle, _rxParserColumn->getPropertyValue( aProp.sName ) );
    }

    // determine the table column we're based on
    osl_atomic_increment( &m_refCount );
    {
        m_xOriginalTableColumn = impl_determineOriginalTableColumn( _rxConnection );
    }
    osl_atomic_decrement( &m_refCount );
}

OQueryColumn::~OQueryColumn()
{
}

Reference< XPropertySet > OQueryColumn::impl_determineOriginalTableColumn( const Reference< XConnection >& _rxConnection )
{
    OSL_PRECOND( _rxConnection.is(), "OQueryColumn::impl_determineOriginalTableColumn: illegal connection!" );
    if ( !_rxConnection.is() )
        return nullptr;

    Reference< XPropertySet > xOriginalTableColumn;
    try
    {
        // determine the composed table name, plus the column name, as indicated by the
        // respective properties
        OUString sCatalog, sSchema, sTable;
        OSL_VERIFY( getPropertyValue( PROPERTY_CATALOGNAME ) >>= sCatalog );
        OSL_VERIFY( getPropertyValue( PROPERTY_SCHEMANAME ) >>= sSchema );
        OSL_VERIFY( getPropertyValue( PROPERTY_TABLENAME ) >>= sTable );
        if ( sCatalog.isEmpty() && sSchema.isEmpty() && sTable.isEmpty() )
            return nullptr;

        OUString sComposedTableName = ::dbtools::composeTableName(
            _rxConnection->getMetaData(), sCatalog, sSchema, sTable, false, ::dbtools::EComposeRule::Complete );

        // retrieve the table in question
        Reference< XTablesSupplier > xSuppTables( _rxConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xTables( xSuppTables->getTables(), UNO_QUERY_THROW );
        if ( !xTables->hasByName( sComposedTableName ) )
            return nullptr;

        Reference< XColumnsSupplier > xSuppCols( xTables->getByName( sComposedTableName ), UNO_QUERY_THROW );
        Reference< XNameAccess > xColumns( xSuppCols->getColumns(), UNO_QUERY_THROW );

        OUString sColumn;
        OSL_VERIFY( getPropertyValue( PROPERTY_REALNAME ) >>= sColumn );
        if ( !xColumns->hasByName( sColumn ) )
            return nullptr;

        xOriginalTableColumn.set( xColumns->getByName( sColumn ), UNO_QUERY );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xOriginalTableColumn;
}

IMPLEMENT_GET_IMPLEMENTATION_ID( OQueryColumn )

OUString SAL_CALL OQueryColumn::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return OUString( "org.openoffice.comp.dbaccess.OQueryColumn"  );
}

::cppu::IPropertyArrayHelper& SAL_CALL OQueryColumn::getInfoHelper()
{
    return *OQueryColumn_PBase::getArrayHelper();
}

::cppu::IPropertyArrayHelper* OQueryColumn::createArrayHelper() const
{
    return OTableColumnDescriptor::createArrayHelper();
}

void SAL_CALL OQueryColumn::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
{
    OTableColumnDescriptor::getFastPropertyValue( _rValue, _nHandle );

    // special treatment for column settings:
    if ( !OColumnSettings::isColumnSettingProperty( _nHandle ) )
        return;

    // If the setting has its default value, then try to obtain the value from the table column which
    // this query column is based on
    if ( !OColumnSettings::isDefaulted( _nHandle, _rValue ) )
        return;

    if ( !m_xOriginalTableColumn.is() )
        return;

    try
    {
        // determine original property name
        OUString sPropName;
        sal_Int16 nAttributes( 0 );
        const_cast< OQueryColumn* >( this )->getInfoHelper().fillPropertyMembersByHandle( &sPropName, &nAttributes, _nHandle );
        OSL_ENSURE( !sPropName.isEmpty(), "OColumnWrapper::impl_getPropertyNameFromHandle: property not found!" );

        _rValue = m_xOriginalTableColumn->getPropertyValue( sPropName );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// OColumnWrapper

OColumnWrapper::OColumnWrapper( const Reference< XPropertySet > & rCol, const bool _bNameIsReadOnly )
    :OColumn( _bNameIsReadOnly )
    ,m_xAggregate(rCol)
    ,m_nColTypeID(-1)
{
    // which type of aggregate property do we have?
    // we distinguish the properties by the containment of optional properties
    m_nColTypeID = 0;
    if ( m_xAggregate.is() )
    {
        Reference <XPropertySetInfo > xInfo(m_xAggregate->getPropertySetInfo());
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_DESCRIPTION) ? HAS_DESCRIPTION : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_DEFAULTVALUE) ? HAS_DEFAULTVALUE : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_ISROWVERSION) ? HAS_ROWVERSION : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_AUTOINCREMENTCREATION) ? HAS_AUTOINCREMENT_CREATION : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_CATALOGNAME) ? HAS_CATALOGNAME : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_SCHEMANAME) ? HAS_SCHEMANAME : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_TABLENAME) ? HAS_TABLENAME : 0;

        m_xAggregate->getPropertyValue(PROPERTY_NAME) >>= m_sName;
    }
}

OColumnWrapper::~OColumnWrapper()
{
}

OUString OColumnWrapper::impl_getPropertyNameFromHandle( const sal_Int32 _nHandle ) const
{
    OUString sPropName;
    sal_Int16 nAttributes( 0 );
    const_cast< OColumnWrapper* >( this )->getInfoHelper().fillPropertyMembersByHandle( &sPropName, &nAttributes, _nHandle );
    OSL_ENSURE( !sPropName.isEmpty(), "OColumnWrapper::impl_getPropertyNameFromHandle: property not found!" );
    return sPropName;
}

void OColumnWrapper::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    // derived classes are free to either use the OPropertyContainer(Helper) mechanisms for properties,
    // or to declare additional properties which are to be forwarded to the wrapped object. So we need
    // to distinguish those cases.
    if ( OColumn::isRegisteredProperty( nHandle ) )
    {
        OColumn::getFastPropertyValue( rValue, nHandle );
    }
    else
    {
        rValue = m_xAggregate->getPropertyValue( impl_getPropertyNameFromHandle( nHandle ) );
    }
}

sal_Bool OColumnWrapper::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle,
            const Any& rValue ) throw (IllegalArgumentException)
{
    bool bModified( false );
    if ( OColumn::isRegisteredProperty( nHandle ) )
    {
        bModified = OColumn::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
    }
    else
    {
        getFastPropertyValue( rOldValue, nHandle );
        if ( rOldValue != rValue )
        {
            rConvertedValue = rValue;
            bModified = true;
        }
    }
    return bModified;
}

void OColumnWrapper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception, std::exception)
{
    if ( OColumn::isRegisteredProperty( nHandle ) )
    {
        OColumn::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    }
    else
    {
        m_xAggregate->setPropertyValue( impl_getPropertyNameFromHandle( nHandle ), rValue );
    }
}

// OTableColumnDescriptorWrapper
OTableColumnDescriptorWrapper::OTableColumnDescriptorWrapper( const Reference< XPropertySet >& _rCol, const bool _bPureWrap, const bool _bIsDescriptor )
    :OColumnWrapper( _rCol, !_bIsDescriptor )
    ,m_bPureWrap( _bPureWrap )
    ,m_bIsDescriptor( _bIsDescriptor )
{
    // let the ColumnSettings register its properties
    OColumnSettings::registerProperties( *this );
}

// css::lang::XTypeProvider
IMPLEMENT_GET_IMPLEMENTATION_ID( OTableColumnDescriptorWrapper )

// css::lang::XServiceInfo
OUString OTableColumnDescriptorWrapper::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.sdb.OTableColumnDescriptorWrapper");
}

Sequence< OUString > OTableColumnDescriptorWrapper::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMNDESCRIPTOR;
    aSNS[1] = SERVICE_SDB_COLUMNSETTINGS;
    return aSNS;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* OTableColumnDescriptorWrapper::createArrayHelper( sal_Int32 nId ) const
{
    const sal_Int32 nHaveAlways = 7;

    // Which optional properties are contained?
    const sal_Int32 nHaveOptionally (::std::bitset<7>(nId).count());

    BEGIN_PROPERTY_SEQUENCE( nHaveAlways + nHaveOptionally )

    DECL_PROP0_BOOL( ISAUTOINCREMENT                );
    DECL_PROP0_BOOL( ISCURRENCY                     );
    DECL_PROP0( ISNULLABLE,         sal_Int32       );
    DECL_PROP0( PRECISION,          sal_Int32       );
    DECL_PROP0( SCALE,              sal_Int32       );
    DECL_PROP0( TYPE,               sal_Int32       );
    DECL_PROP0( TYPENAME,           OUString );

    if ( nId & HAS_AUTOINCREMENT_CREATION )
    {
        DECL_PROP1( AUTOINCREMENTCREATION, OUString, MAYBEVOID );
    }
    if ( nId & HAS_DEFAULTVALUE )
    {
        DECL_PROP0( DEFAULTVALUE, OUString );
    }
    if ( nId & HAS_DESCRIPTION )
    {
        DECL_PROP0( DESCRIPTION, OUString );
    }
    if ( nId & HAS_ROWVERSION )
    {
        DECL_PROP0_BOOL( ISROWVERSION );
    }
    if ( nId & HAS_CATALOGNAME )
    {
        DECL_PROP0( CATALOGNAME, OUString );
    }
    if ( nId & HAS_SCHEMANAME )
    {
        DECL_PROP0( SCHEMANAME, OUString );
    }
    if ( nId & HAS_TABLENAME )
    {
        DECL_PROP0( TABLENAME, OUString );
    }

    END_PROPERTY_SEQUENCE()

    if ( !m_bIsDescriptor )
    {
        for (   Property* prop = aDescriptor.getArray();
                prop != aDescriptor.getArray() + aDescriptor.getLength();
                ++prop
            )
        {
            prop->Attributes |= PropertyAttribute::READONLY;
        }
    }

    // finally also describe the properties which are maintained by our base class, in particular the OPropertyContainerHelper
    Sequence< Property > aBaseProperties;
    describeProperties( aBaseProperties );

    Sequence< Property > aAllProperties( ::comphelper::concatSequences( aDescriptor, aBaseProperties ) );
    return new ::cppu::OPropertyArrayHelper( aAllProperties, false );
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& OTableColumnDescriptorWrapper::getInfoHelper()
{
    return *static_cast< OIdPropertyArrayUsageHelper< OTableColumnDescriptorWrapper >* >(this)->getArrayHelper(m_nColTypeID);
}

void OTableColumnDescriptorWrapper::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    if ( m_bPureWrap )
    {
        rValue = m_xAggregate->getPropertyValue( impl_getPropertyNameFromHandle( nHandle ) );
    }
    else
    {
        OColumnWrapper::getFastPropertyValue( rValue, nHandle );
    }
}

sal_Bool OTableColumnDescriptorWrapper::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue ) throw (IllegalArgumentException)
{
    bool bModified(false);
    if ( m_bPureWrap )
    {
        // do not delegate to OColumnWrapper: It would, for the properties which were registered with registerProperty,
        // ask the OPropertyContainer base class, which is not what we want here.
        // TODO: the whole "m_bPureWrap"-thingie is strange. We should have a dedicated class doing this wrapping,
        // not a class which normally serves other purposes, and only sometimes does a "pure wrap". It makes the
        // code unnecessarily hard to maintain, and error prone.
        rOldValue = m_xAggregate->getPropertyValue( impl_getPropertyNameFromHandle( nHandle ) );
        if ( rOldValue != rValue )
        {
            rConvertedValue = rValue;
            bModified = true;
        }
    }
    else
    {
        bModified = OColumnWrapper::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
    }
    return bModified;
}

void OTableColumnDescriptorWrapper::setFastPropertyValue_NoBroadcast(
                                                sal_Int32 nHandle,
                                                const Any& rValue
                                                 )
                                                 throw (Exception, std::exception)
{
    if ( m_bPureWrap )
    {
        m_xAggregate->setPropertyValue( impl_getPropertyNameFromHandle( nHandle ), rValue );
    }
    else
    {
        OColumnWrapper::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    }
}

// OTableColumnWrapper
OTableColumnWrapper::OTableColumnWrapper( const Reference< XPropertySet >& rCol, const Reference< XPropertySet >& _xColDefintion,
            const bool _bPureWrap )
    :OTableColumnDescriptorWrapper( rCol, _bPureWrap, false )
{
    osl_atomic_increment( &m_refCount );
    if ( _xColDefintion.is() )
    {
        try
        {
            ::comphelper::copyProperties( _xColDefintion, this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    osl_atomic_decrement( &m_refCount );
}

OTableColumnWrapper::~OTableColumnWrapper()
{
}

IMPLEMENT_GET_IMPLEMENTATION_ID( OTableColumnWrapper )

OUString OTableColumnWrapper::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.sdb.OTableColumnWrapper" );
}

Sequence< OUString > OTableColumnWrapper::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    aSNS[1] = SERVICE_SDB_COLUMNSETTINGS;
    return aSNS;
}

::cppu::IPropertyArrayHelper& OTableColumnWrapper::getInfoHelper()
{
    return *static_cast< OIdPropertyArrayUsageHelper< OTableColumnWrapper >* >(this)->getArrayHelper(m_nColTypeID);
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* OTableColumnWrapper::createArrayHelper( sal_Int32 nId ) const
{
    return OTableColumnDescriptorWrapper::createArrayHelper( nId );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
