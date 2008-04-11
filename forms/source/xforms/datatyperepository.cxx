/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datatyperepository.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_forms.hxx"

#ifndef FORMS_SOURCE_XFORMS_DATATYPEREPOSITORY_HXX
#include "datatyperepository.hxx"
#endif
#include "datatypes.hxx"
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#include "frm_resource.hxx"
#include "frm_strings.hxx"
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <tools/debug.hxx>
#include <comphelper/enumhelper.hxx>

#include <functional>
#include <algorithm>

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
            throw VetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "This is a built-in type and cannot be removed." ) ), *this );
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
            ::std::select1st< Repository::value_type >()
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

