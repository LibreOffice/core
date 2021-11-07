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


#include "datatyperepository.hxx"
#include "datatypes.hxx"
#include <strings.hrc>
#include <frm_resource.hxx>
#include <frm_strings.hxx>
#include <property.hxx>

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <comphelper/enumhelper.hxx>

namespace xforms
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::util::VetoException;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::container::ElementExistException;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::xsd::XDataType;
    using namespace frm;

    ODataTypeRepository::ODataTypeRepository( )
    {

        // insert some basic types
        OUString sName( ResourceManager::loadString(RID_STR_DATATYPE_STRING) );
        m_aRepository[ sName ] = new OStringType( sName, css::xsd::DataTypeClass::STRING );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_URL);
        m_aRepository[ sName ] = new OStringType( sName, css::xsd::DataTypeClass::anyURI );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_BOOLEAN);
        m_aRepository[ sName ] = new OBooleanType( sName );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_DECIMAL);
        m_aRepository[ sName ] = new ODecimalType( sName, css::xsd::DataTypeClass::DECIMAL );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_FLOAT);
        m_aRepository[ sName ] = new ODecimalType( sName, css::xsd::DataTypeClass::FLOAT );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_DOUBLE);
        m_aRepository[ sName ] = new ODecimalType( sName, css::xsd::DataTypeClass::DOUBLE );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_DATE);
        m_aRepository[ sName ] = new ODateType( sName );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_TIME);
        m_aRepository[ sName ] = new OTimeType( sName );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_DATETIME);
        m_aRepository[ sName ] = new ODateTimeType( sName );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_YEAR);
        m_aRepository[ sName ] = new OShortIntegerType( sName, css::xsd::DataTypeClass::gYear );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_MONTH);
        m_aRepository[ sName ] = new OShortIntegerType( sName, css::xsd::DataTypeClass::gMonth );

        sName = ResourceManager::loadString(RID_STR_DATATYPE_DAY);
        m_aRepository[ sName ] = new OShortIntegerType( sName, css::xsd::DataTypeClass::gDay );
    }


    ODataTypeRepository::~ODataTypeRepository( )
    {
    }


    ODataTypeRepository::Repository::iterator ODataTypeRepository::implLocate( const OUString& _rName, bool _bAllowMiss )
    {
        Repository::iterator aTypePos = m_aRepository.find( _rName );
        if ( aTypePos == m_aRepository.end() && !_bAllowMiss )
            throw NoSuchElementException( OUString(), *this );

        return aTypePos;
    }


    Reference< XDataType > SAL_CALL ODataTypeRepository::getBasicDataType( sal_Int16 dataTypeClass )
    {
        Reference< XDataType > xReturn;

        for ( Repository::const_iterator lookup = m_aRepository.begin();
              ( lookup != m_aRepository.end() ) && ! xReturn.is();
              ++lookup
            )
        {
            if ( lookup->second->getIsBasic() && ( lookup->second->getTypeClass() == dataTypeClass ) )
                xReturn = lookup->second.get();
        }

        if ( !xReturn.is() )
            throw NoSuchElementException( OUString(), *this );

        return xReturn;
    }


    Reference< XDataType > SAL_CALL ODataTypeRepository::cloneDataType( const OUString& sourceName, const OUString& newName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Repository::iterator aTypePos = implLocate( newName, true );
        if ( aTypePos != m_aRepository.end() )
            throw ElementExistException( OUString(), *this );

        aTypePos = implLocate( sourceName );
        rtl::Reference<OXSDDataType> pClone = aTypePos->second->clone( newName );
        m_aRepository[ newName ] = pClone;

        return pClone;
    }


    void SAL_CALL ODataTypeRepository::revokeDataType( const OUString& typeName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Repository::iterator aTypePos = implLocate( typeName );
        if ( aTypePos->second->getIsBasic() )
            // "This is a built-in type and cannot be removed."
            throw VetoException(ResourceManager::loadString(RID_STR_XFORMS_CANT_REMOVE_TYPE), *this );

        m_aRepository.erase( aTypePos );
    }


    Reference< XDataType > SAL_CALL ODataTypeRepository::getDataType( const OUString& typeName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return implLocate( typeName )->second;
    }


    Reference< XEnumeration > SAL_CALL ODataTypeRepository::createEnumeration(  )
    {
        return new ::comphelper::OEnumerationByName( this );
    }


    Any SAL_CALL ODataTypeRepository::getByName( const OUString& aName )
    {
        return makeAny( getDataType( aName ) );
    }


    Sequence< OUString > SAL_CALL ODataTypeRepository::getElementNames(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return comphelper::mapKeysToSequence( m_aRepository );
    }


    sal_Bool SAL_CALL ODataTypeRepository::hasByName( const OUString& aName )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_aRepository.find( aName ) != m_aRepository.end();
    }


    Type SAL_CALL ODataTypeRepository::getElementType(  )
    {
        return cppu::UnoType<XDataType>::get();
    }


    sal_Bool SAL_CALL ODataTypeRepository::hasElements(  )
    {
        return !m_aRepository.empty();
    }


    // type specific implementation of registerProperties, using explicit
    // template instantiations

    template<>
    void OValueLimitedType<css::util::Date>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_INCLUSIVE_DATE, PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_EXCLUSIVE_DATE, PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxExclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_INCLUSIVE_DATE, PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_EXCLUSIVE_DATE, PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinExclusive, cppu::UnoType<ValueType>::get() );
    }

    template<>
    void OValueLimitedType<css::util::Time>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_INCLUSIVE_TIME, PROPERTY_ID_XSD_MAX_INCLUSIVE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_EXCLUSIVE_TIME, PROPERTY_ID_XSD_MAX_EXCLUSIVE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxExclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_INCLUSIVE_TIME, PROPERTY_ID_XSD_MIN_INCLUSIVE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_EXCLUSIVE_TIME, PROPERTY_ID_XSD_MIN_EXCLUSIVE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinExclusive, cppu::UnoType<ValueType>::get() );
    }

    template<>
    void OValueLimitedType<css::util::DateTime>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_INCLUSIVE_DATE_TIME, PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_EXCLUSIVE_DATE_TIME, PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxExclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_INCLUSIVE_DATE_TIME, PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_EXCLUSIVE_DATE_TIME, PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE_TIME, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinExclusive, cppu::UnoType<ValueType>::get() );
    }

    template<>
    void OValueLimitedType<double>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_INCLUSIVE_DOUBLE, PROPERTY_ID_XSD_MAX_INCLUSIVE_DOUBLE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_EXCLUSIVE_DOUBLE, PROPERTY_ID_XSD_MAX_EXCLUSIVE_DOUBLE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxExclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_INCLUSIVE_DOUBLE, PROPERTY_ID_XSD_MIN_INCLUSIVE_DOUBLE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_EXCLUSIVE_DOUBLE, PROPERTY_ID_XSD_MIN_EXCLUSIVE_DOUBLE, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinExclusive, cppu::UnoType<ValueType>::get() );
    }

    template<>
    void OValueLimitedType<sal_Int16>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_INCLUSIVE_INT, PROPERTY_ID_XSD_MAX_INCLUSIVE_INT, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MAX_EXCLUSIVE_INT, PROPERTY_ID_XSD_MAX_EXCLUSIVE_INT, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMaxExclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_INCLUSIVE_INT, PROPERTY_ID_XSD_MIN_INCLUSIVE_INT, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinInclusive, cppu::UnoType<ValueType>::get() );

        registerMayBeVoidProperty( PROPERTY_XSD_MIN_EXCLUSIVE_INT, PROPERTY_ID_XSD_MIN_EXCLUSIVE_INT, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID,
            &m_aMinExclusive, cppu::UnoType<ValueType>::get() );
    }

} // namespace xforms


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
