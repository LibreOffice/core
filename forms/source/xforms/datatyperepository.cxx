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
#include "frm_resource.hrc"
#include "frm_resource.hxx"
#include "frm_strings.hxx"
#include "property.hrc"

#include <tools/debug.hxx>
#include <comphelper/enumhelper.hxx>

#include <functional>
#include <algorithm>
#include <o3tl/compat_functional.hxx>

//........................................................................
namespace xforms
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::util::VetoException;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::container::ElementExistException;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::xsd::XDataType;
    using namespace frm;

    //====================================================================
    //= ODataTypeRepository
    //====================================================================
    DBG_NAME( ODataTypeRepository )
    //--------------------------------------------------------------------
    ODataTypeRepository::ODataTypeRepository( )
    {
        DBG_CTOR( ODataTypeRepository, NULL );

        // insert some basic types
        ::rtl::OUString sName( FRM_RES_STRING( RID_STR_DATATYPE_STRING ) );
        m_aRepository[ sName ] = new OStringType( sName, ::com::sun::star::xsd::DataTypeClass::STRING );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_URL );
        m_aRepository[ sName ] = new OStringType( sName, ::com::sun::star::xsd::DataTypeClass::anyURI );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_BOOLEAN );
        m_aRepository[ sName ] = new OBooleanType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DECIMAL );
        m_aRepository[ sName ] = new ODecimalType( sName, ::com::sun::star::xsd::DataTypeClass::DECIMAL );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_FLOAT );
        m_aRepository[ sName ] = new ODecimalType( sName, ::com::sun::star::xsd::DataTypeClass::FLOAT );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DOUBLE );
        m_aRepository[ sName ] = new ODecimalType( sName, ::com::sun::star::xsd::DataTypeClass::DOUBLE );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DATE );
        m_aRepository[ sName ] = new ODateType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_TIME );
        m_aRepository[ sName ] = new OTimeType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DATETIME );
        m_aRepository[ sName ] = new ODateTimeType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_YEAR );
        m_aRepository[ sName ] = new OShortIntegerType( sName, ::com::sun::star::xsd::DataTypeClass::gYear );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_MONTH );
        m_aRepository[ sName ] = new OShortIntegerType( sName, ::com::sun::star::xsd::DataTypeClass::gMonth );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DAY );
        m_aRepository[ sName ] = new OShortIntegerType( sName, ::com::sun::star::xsd::DataTypeClass::gDay );
    }

    //--------------------------------------------------------------------
    ODataTypeRepository::~ODataTypeRepository( )
    {
        DBG_DTOR( ODataTypeRepository, NULL );
    }

    //--------------------------------------------------------------------
    ODataTypeRepository::Repository::iterator ODataTypeRepository::implLocate( const ::rtl::OUString& _rName, bool _bAllowMiss ) SAL_THROW( ( NoSuchElementException ) )
    {
        Repository::iterator aTypePos = m_aRepository.find( _rName );
        if ( aTypePos == m_aRepository.end() && !_bAllowMiss )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        return aTypePos;
    }

    //--------------------------------------------------------------------
    Reference< XDataType > SAL_CALL ODataTypeRepository::getBasicDataType( sal_Int16 dataTypeClass ) throw (NoSuchElementException, RuntimeException)
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
            throw NoSuchElementException( ::rtl::OUString(), *this );

        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< XDataType > SAL_CALL ODataTypeRepository::cloneDataType( const ::rtl::OUString& sourceName, const ::rtl::OUString& newName ) throw (NoSuchElementException, ElementExistException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Repository::iterator aTypePos = implLocate( newName, true );
        if ( aTypePos != m_aRepository.end() )
            throw ElementExistException( ::rtl::OUString(), *this );

        aTypePos = implLocate( sourceName );
        OXSDDataType* pClone = aTypePos->second->clone( newName );
        m_aRepository[ newName ] = pClone;

        return pClone;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ODataTypeRepository::revokeDataType( const ::rtl::OUString& typeName ) throw (NoSuchElementException, VetoException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Repository::iterator aTypePos = implLocate( typeName );
        if ( aTypePos->second->getIsBasic() )
            throw VetoException( ::rtl::OUString( "This is a built-in type and cannot be removed." ), *this );
            // TODO: localize this error message

        m_aRepository.erase( aTypePos );
    }

    //--------------------------------------------------------------------
    Reference< XDataType > SAL_CALL ODataTypeRepository::getDataType( const ::rtl::OUString& typeName ) throw (NoSuchElementException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return implLocate( typeName, false )->second.get();
    }


    //--------------------------------------------------------------------
    Reference< XEnumeration > SAL_CALL ODataTypeRepository::createEnumeration(  ) throw (RuntimeException)
    {
        return new ::comphelper::OEnumerationByName( this );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL ODataTypeRepository::getByName( const ::rtl::OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        return makeAny( getDataType( aName ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ODataTypeRepository::getElementNames(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Sequence< ::rtl::OUString > aNames( m_aRepository.size() );
        ::std::transform(
            m_aRepository.begin(),
            m_aRepository.end(),
            aNames.getArray(),
            ::o3tl::select1st< Repository::value_type >()
        );
        return aNames;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODataTypeRepository::hasByName( const ::rtl::OUString& aName ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_aRepository.find( aName ) != m_aRepository.end();
    }

    //--------------------------------------------------------------------
    Type SAL_CALL ODataTypeRepository::getElementType(  ) throw (RuntimeException)
    {
        return ::getCppuType( static_cast< Reference< XDataType >* >( NULL ) );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODataTypeRepository::hasElements(  ) throw (RuntimeException)
    {
        return !m_aRepository.empty();
    }

    //--------------------------------------------------------------------
    // type specific implementation of registerProperties, using explicit
    // template instantiations

    template<>
    void OValueLimitedType<com::sun::star::util::Date>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_MAX_INCLUSIVE_DATE, m_aMaxInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MAX_EXCLUSIVE_DATE, m_aMaxExclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_INCLUSIVE_DATE, m_aMinInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_EXCLUSIVE_DATE, m_aMinExclusive, ValueType );
    }

    template<>
    void OValueLimitedType<com::sun::star::util::Time>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_MAX_INCLUSIVE_TIME, m_aMaxInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MAX_EXCLUSIVE_TIME, m_aMaxExclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_INCLUSIVE_TIME, m_aMinInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_EXCLUSIVE_TIME, m_aMinExclusive, ValueType );
    }

    template<>
    void OValueLimitedType<com::sun::star::util::DateTime>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_MAX_INCLUSIVE_DATE_TIME, m_aMaxInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MAX_EXCLUSIVE_DATE_TIME, m_aMaxExclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_INCLUSIVE_DATE_TIME, m_aMinInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_EXCLUSIVE_DATE_TIME, m_aMinExclusive, ValueType );
    }

    template<>
    void OValueLimitedType<double>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_MAX_INCLUSIVE_DOUBLE, m_aMaxInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MAX_EXCLUSIVE_DOUBLE, m_aMaxExclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_INCLUSIVE_DOUBLE, m_aMinInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_EXCLUSIVE_DOUBLE, m_aMinExclusive, ValueType );
    }

    template<>
    void OValueLimitedType<sal_Int16>::registerProperties()
    {
        OValueLimitedType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_MAX_INCLUSIVE_INT, m_aMaxInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MAX_EXCLUSIVE_INT, m_aMaxExclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_INCLUSIVE_INT, m_aMinInclusive, ValueType );
        REGISTER_VOID_PROP( XSD_MIN_EXCLUSIVE_INT, m_aMinExclusive, ValueType );
    }
//........................................................................
} // namespace xforms
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
