/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertybaghelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:35:14 $
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
#include "precompiled_forms.hxx"

#ifndef FORMS_PROPERTYBAGHELPER_HXX
#include "propertybaghelper.hxx"
#endif

#include "property.hxx"

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYEXISTEXCEPTION_HPP_
#include <com/sun/star/beans/PropertyExistException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NOTREMOVEABLEEXCEPTION_HPP_
#include <com/sun/star/beans/NotRemoveableException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

#include <comphelper/sequence.hxx>

#define NEW_HANDLE_BASE 10000

//........................................................................
namespace frm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::beans::PropertyExistException;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::beans::XMultiPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::beans::NotRemoveableException;
    using ::com::sun::star::beans::UnknownPropertyException;
    /** === end UNO using === **/
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        //----------------------------------------------------------------
        static ::comphelper::IPropertyInfoService& lcl_getPropertyInfos()
        {
            static ConcreteInfoService s_aPropInfos;
            return s_aPropInfos;
        }
    }

    //====================================================================
    //= PropertyBagHelper
    //====================================================================
    //--------------------------------------------------------------------
    PropertyBagHelper::PropertyBagHelper( IPropertyBagHelperContext& _rContext )
        :m_rContext( _rContext )
        ,m_pPropertyArrayHelper( NULL )
        ,m_bDisposed( false )
    {
    }

    //--------------------------------------------------------------------
    PropertyBagHelper::~PropertyBagHelper()
    {
        delete m_pPropertyArrayHelper, m_pPropertyArrayHelper = NULL;
    }

    //--------------------------------------------------------------------
    void PropertyBagHelper::dispose()
    {
        m_bDisposed = true;
    }

    //--------------------------------------------------------------------
    void PropertyBagHelper::impl_nts_checkDisposed_throw() const
    {
        if ( m_bDisposed )
            throw DisposedException();
    }

    //--------------------------------------------------------------------
    void PropertyBagHelper::impl_nts_invalidatePropertySetInfo()
    {
        delete m_pPropertyArrayHelper, m_pPropertyArrayHelper = NULL;
    }

    //--------------------------------------------------------------------
    sal_Int32 PropertyBagHelper::impl_findFreeHandle( const ::rtl::OUString& _rPropertyName )
    {
        ::comphelper::OPropertyArrayAggregationHelper& rPropInfo( impl_ts_getArrayHelper() );

        // check the preferred handle
        sal_Int32 nHandle = lcl_getPropertyInfos().getPreferedPropertyId( _rPropertyName );
        if ( ( nHandle != -1 ) && rPropInfo.fillPropertyMembersByHandle( NULL, NULL, nHandle ) )
            nHandle = -1;

        // seach a free handle in <math>F_1009</math>
        if ( nHandle == -1 )
        {
            sal_Int32 nPrime = 1009;
            sal_Int32 nFactor = 11;
            sal_Int32 nNum = nFactor;
            while ( nNum != 1 )
            {
                if ( !rPropInfo.fillPropertyMembersByHandle( NULL, NULL, nNum + NEW_HANDLE_BASE ) )
                {
                    // handle not used, yet
                    nHandle = nNum + NEW_HANDLE_BASE;
                    break;
                }
                nNum = ( nNum * nFactor ) % nPrime;
            }
        }

        // search a free handle greater NEW_HANDLE_BASE
        if ( nHandle == -1 )
        {
            nHandle = NEW_HANDLE_BASE + 1009;
            while ( rPropInfo.fillPropertyMembersByHandle( NULL, NULL, nHandle ) )
                ++nHandle;
        }

        return nHandle;
    }

    //--------------------------------------------------------------------
    ::comphelper::OPropertyArrayAggregationHelper& PropertyBagHelper::impl_ts_getArrayHelper() const
    {
        ::osl::MutexGuard aGuard( m_rContext.getMutex() );
        if ( !m_pPropertyArrayHelper )
        {
            // our own fixed and our aggregate's properties
            Sequence< Property > aFixedProps;
            Sequence< Property > aAggregateProps;
            m_rContext.describeFixedAndAggregateProperties( aFixedProps, aAggregateProps );

            // our dynamic properties
            Sequence< Property > aDynamicProps;
            m_aDynamicProperties.describeProperties( aDynamicProps );

            Sequence< Property > aOwnProps(
                ::comphelper::concatSequences( aFixedProps, aDynamicProps ) );

            const_cast< PropertyBagHelper* >( this )->m_pPropertyArrayHelper = new OPropertyArrayAggregationHelper( aOwnProps, aAggregateProps, &lcl_getPropertyInfos(), NEW_HANDLE_BASE );
        }
        return *m_pPropertyArrayHelper;
    }

    //--------------------------------------------------------------------
    void PropertyBagHelper::addProperty( const ::rtl::OUString& _rName, ::sal_Int16 _nAttributes, const Any& _rInitialValue )
    {
        ::osl::MutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();

        //----------------------------------------------
        // check name sanity
        ::comphelper::OPropertyArrayAggregationHelper& aPropInfo( impl_ts_getArrayHelper() );
        if ( aPropInfo.hasPropertyByName( _rName ) )
            throw PropertyExistException( _rName, m_rContext.getPropertiesInterface() );

        //----------------------------------------------
        // normalize the REMOVEABLE attribute - the FormComponent service
        // requires that all dynamic properties are REMOVEABLE
        _nAttributes |= PropertyAttribute::REMOVEABLE;

        //----------------------------------------------
        // find a free handle
        sal_Int32 nHandle = impl_findFreeHandle( _rName );

        //----------------------------------------------
        // register the property, and invalidate our property meta data
        m_aDynamicProperties.addProperty( _rName, nHandle, _nAttributes, _rInitialValue );
        impl_nts_invalidatePropertySetInfo();
    }

    //--------------------------------------------------------------------
    void PropertyBagHelper::removeProperty( const ::rtl::OUString& _rName )
    {
        ::osl::MutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();

        // check whether it's removeable at all
        Reference< XMultiPropertySet > xMe( m_rContext.getPropertiesInterface(), UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xPSI( xMe->getPropertySetInfo(), UNO_QUERY_THROW );
        Property aProperty( xPSI->getPropertyByName( _rName ) );
        if ( ( aProperty.Attributes & PropertyAttribute::REMOVEABLE ) == 0 )
            throw NotRemoveableException( _rName, xMe );

        m_aDynamicProperties.removeProperty( _rName );
        impl_nts_invalidatePropertySetInfo();
    }

    //--------------------------------------------------------------------
    namespace
    {
        //----------------------------------------------------------------
        struct SelectNameOfProperty : public ::std::unary_function< Property, ::rtl::OUString >
        {
            const ::rtl::OUString& operator()( const Property& _rProp ) const { return _rProp.Name; }
        };

        //----------------------------------------------------------------
        struct SelectNameOfPropertyValue : public ::std::unary_function< PropertyValue, ::rtl::OUString >
        {
            const ::rtl::OUString& operator()( const PropertyValue& _rProp ) const { return _rProp.Name; }
        };

        //----------------------------------------------------------------
        struct SelectValueOfPropertyValue : public ::std::unary_function< PropertyValue, Any >
        {
            const Any& operator()( const PropertyValue& _rProp ) const { return _rProp.Value; }
        };

        //----------------------------------------------------------------
        struct PropertyValueLessByName : public ::std::binary_function< PropertyValue, PropertyValue, bool >
        {
            bool operator()( const PropertyValue& _lhs, const PropertyValue _rhs ) const
            {
                return _lhs.Name < _rhs.Name;
            }
        };
    }

    //--------------------------------------------------------------------
    Sequence< PropertyValue > PropertyBagHelper::getPropertyValues()
    {
        ::osl::MutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();

        Reference< XMultiPropertySet > xMe( m_rContext.getPropertiesInterface(), UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xPSI( xMe->getPropertySetInfo(), UNO_QUERY_THROW );

        Sequence< Property > aProperties( xPSI->getProperties() );
        Sequence< ::rtl::OUString > aPropertyNames( aProperties.getLength() );
        ::std::transform( aProperties.getConstArray(), aProperties.getConstArray() + aProperties.getLength(),
            aPropertyNames.getArray(), SelectNameOfProperty() );

        Sequence< Any > aValues;
        try
        {
            aValues = xMe->getPropertyValues( aPropertyNames );

            if ( aValues.getLength() != aPropertyNames.getLength() )
                throw RuntimeException();
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        Sequence< PropertyValue > aPropertyValues( aValues.getLength() );
        PropertyValue* pPropertyValue = aPropertyValues.getArray();

        const ::rtl::OUString* pName = aPropertyNames.getConstArray();
        const ::rtl::OUString* pNameEnd = aPropertyNames.getConstArray() + aPropertyNames.getLength();
        const Any* pValue = aValues.getConstArray();
        for ( ; pName != pNameEnd; ++pName, ++pValue, ++pPropertyValue )
        {
            pPropertyValue->Name = *pName;
            pPropertyValue->Value = *pValue;
        }

        return aPropertyValues;
    }

    //--------------------------------------------------------------------
    void PropertyBagHelper::setPropertyValues( const Sequence< PropertyValue >& _rProps )
    {
        ::osl::ClearableMutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();

        sal_Int32 nPropertyValues = _rProps.getLength();

        // XMultiPropertySet::setPropertyValues expects its arguments to be sorted by name
        // while XPropertyAccess::setPropertyValues doesn't. So first of all, sort.
        Sequence< PropertyValue > aSortedProps( _rProps );
        ::std::sort( aSortedProps.getArray(), aSortedProps.getArray() + nPropertyValues, PropertyValueLessByName() );

        // also, XPropertyAccess::setPropertyValues is expected to throw an UnknownPropertyException
        // for unsupported properties, while XMultiPropertySet::setPropertyValues is expected to ignore
        // those. So, check for unsupported properties first.
        ::comphelper::OPropertyArrayAggregationHelper& rArrayHelper( impl_ts_getArrayHelper() );
        for (   const PropertyValue* pProperties = aSortedProps.getConstArray();
                pProperties != aSortedProps.getConstArray() + nPropertyValues;
                ++pProperties
            )
        {
            if ( !rArrayHelper.hasPropertyByName( pProperties->Name ) )
                throw UnknownPropertyException( pProperties->Name, m_rContext.getPropertiesInterface() );
        }

        // Now finally split into a Name and a Value sequence, and forward to
        // XMultiPropertySet::setPropertyValues
        Sequence< ::rtl::OUString > aNames( nPropertyValues );
        ::std::transform( aSortedProps.getConstArray(), aSortedProps.getConstArray() + nPropertyValues,
            aNames.getArray(), SelectNameOfPropertyValue() );

        Sequence< Any > aValues( nPropertyValues );
        ::std::transform( aSortedProps.getConstArray(), aSortedProps.getConstArray() + nPropertyValues,
            aValues.getArray(), SelectValueOfPropertyValue() );

        Reference< XMultiPropertySet > xMe( m_rContext.getPropertiesInterface(), UNO_QUERY_THROW );

        aGuard.clear();
        xMe->setPropertyValues( aNames, aValues );
    }

//........................................................................
} // namespace frm
//........................................................................

