/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertybag.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-09 13:45:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#ifndef COMPHELPER_PROPERTYBAG_HXX
#include "comphelper/propertybag.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_ILLEGALTYPEEXCEPTION_HPP_
#include <com/sun/star/beans/IllegalTypeException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYEXISTEXCEPTION_HPP_
#include <com/sun/star/beans/PropertyExistException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NOTREMOVEABLEEXCEPTION_HPP_
#include <com/sun/star/beans/NotRemoveableException.hpp>
#endif
/** === end UNO includes === **/

#include <map>

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::TypeClass_VOID;
    using ::com::sun::star::beans::IllegalTypeException;
    using ::com::sun::star::beans::PropertyExistException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::beans::NotRemoveableException;
    using ::com::sun::star::beans::UnknownPropertyException;
    /** === end UNO using === **/
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //====================================================================
    //= PropertyBag_Impl
    //====================================================================
    typedef ::std::map< sal_Int32, Any >    MapInt2Any;
    struct PropertyBag_Impl
    {
        MapInt2Any  aDefaults;
    };

    //====================================================================
    //= PropertyBag
    //====================================================================
    //--------------------------------------------------------------------
    PropertyBag::PropertyBag()
        :m_pImpl( new PropertyBag_Impl )
    {
    }

    PropertyBag::~PropertyBag()
    {
    }

    //--------------------------------------------------------------------
    void PropertyBag::addProperty( const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, const Any& _rInitialValue )
    {
        // check type sanity
        Type aPropertyType = _rInitialValue.getValueType();
        if ( aPropertyType.getTypeClass() == TypeClass_VOID )
            throw IllegalTypeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The initial vallue must be non-NULL, to determine the property type." ) ),
                // TODO: resource
                NULL );

        // check name/handle sanity
        if ( !_rName.getLength() )
            throw IllegalArgumentException(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The property name must not be empty." ) ),
            // TODO: resource
            NULL,
            1 );
        if ( isRegisteredProperty( _rName ) || isRegisteredProperty( _nHandle ) )
            throw PropertyExistException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property name or handle already used." ) ),
                // TODO: resource
                NULL );

        // register the property
        registerPropertyNoMember( _rName, _nHandle, _nAttributes, aPropertyType,
            _rInitialValue.hasValue() ? _rInitialValue.getValue() : NULL );

        // remember the default
        m_pImpl->aDefaults.insert( MapInt2Any::value_type( _nHandle, _rInitialValue ) );
    }

    //--------------------------------------------------------------------
    void PropertyBag::removeProperty( const ::rtl::OUString& _rName )
    {
        const Property& rProp = getProperty( _rName );
            // will throw an UnknownPropertyException if necessary
        if ( ( rProp.Attributes & PropertyAttribute::REMOVEABLE ) == 0 )
            throw NotRemoveableException( ::rtl::OUString(), NULL );

        revokeProperty( rProp.Handle );
    }

    //--------------------------------------------------------------------
    void PropertyBag::getFastPropertyValue( sal_Int32 _nHandle, Any& _out_rValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        OPropertyContainerHelper::getFastPropertyValue( _out_rValue, _nHandle );
    }

    //--------------------------------------------------------------------
    bool PropertyBag::convertFastPropertyValue( sal_Int32 _nHandle, const Any& _rNewValue, Any& _out_rConvertedValue, Any& _out_rCurrentValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        return const_cast< PropertyBag*  >( this )->OPropertyContainerHelper::convertFastPropertyValue(
            _out_rConvertedValue, _out_rCurrentValue, _nHandle, _rNewValue );
    }

    //--------------------------------------------------------------------
    void PropertyBag::setFastPropertyValue( sal_Int32 _nHandle, const Any& _rValue )
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
    }

    //--------------------------------------------------------------------
    void PropertyBag::getPropertyDefaultByHandle( sal_Int32 _nHandle, Any& _out_rValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        MapInt2Any::const_iterator pos = m_pImpl->aDefaults.find( _nHandle );
        OSL_ENSURE( pos != m_pImpl->aDefaults.end(), "PropertyBag::getPropertyDefaultByHandle: inconsistency!" );
        if ( pos != m_pImpl->aDefaults.end() )
            _out_rValue = pos->second;
        else
            _out_rValue.clear();
    }


//........................................................................
} // namespace comphelper
//........................................................................

