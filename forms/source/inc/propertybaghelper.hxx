/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertybaghelper.hxx,v $
 * $Revision: 1.3 $
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

#ifndef FORMS_PROPERTYBAGHELPER_HXX
#define FORMS_PROPERTYBAGHELPER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyValue.hpp>
/** === end UNO includes === **/

#include <comphelper/propertybag.hxx>
#include <comphelper/propagg.hxx>

#include <boost/noncopyable.hpp>

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= class IPropertyBagHelperContext
    //====================================================================
    class SAL_NO_VTABLE IPropertyBagHelperContext
    {
    public:
        virtual ::osl::Mutex&   getMutex() = 0;

        virtual void            describeFixedAndAggregateProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _out_rFixedProperties,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _out_rAggregateProperties
        ) const = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet >
                                getPropertiesInterface() = 0;
    };

    //====================================================================
    //= class PropertyBagHelper
    //====================================================================
    class PropertyBagHelper : public ::boost::noncopyable
    {
    private:
        IPropertyBagHelperContext&                      m_rContext;
        ::comphelper::OPropertyArrayAggregationHelper*  m_pPropertyArrayHelper;
        ::comphelper::PropertyBag                       m_aDynamicProperties;
        bool                                            m_bDisposed;

    public:
        PropertyBagHelper( IPropertyBagHelperContext& _rContext );
        ~PropertyBagHelper();

        // XComponent equivalent
        void    dispose();

        // OPropertySetHelper equivalent
        inline ::comphelper::OPropertyArrayAggregationHelper& getInfoHelper() const;

        // XPropertyContainer equivalent
        void addProperty( const ::rtl::OUString& _rName, ::sal_Int16 _nAttributes, const ::com::sun::star::uno::Any& _rInitialValue );
        void removeProperty( const ::rtl::OUString& _rName );

        // XPropertyAccess equivalent
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues();
        void setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rProps );

        // forwards to m_aDynamicProperties
        inline void getDynamicFastPropertyValue( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _out_rValue ) const;
        inline bool convertDynamicFastPropertyValue( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rNewValue, ::com::sun::star::uno::Any& _out_rConvertedValue, ::com::sun::star::uno::Any& _out_rCurrentValue ) const;
        inline void setDynamicFastPropertyValue( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue );
        inline void getDynamicPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _out_rValue ) const;
        inline bool hasDynamicPropertyByName( const ::rtl::OUString& _rName ) const;
        inline bool hasDynamicPropertyByHandle( sal_Int32 _nHandle ) const;

    private:
        void    impl_nts_checkDisposed_throw() const;

        /** invalidates our property set info, so subsequent calls to impl_ts_getArrayHelper and thus
            getInfoHelper will return a newly created instance
        */
        void    impl_nts_invalidatePropertySetInfo();

        /** returns the IPropertyArrayHelper instance used by |this|
        */
        ::comphelper::OPropertyArrayAggregationHelper& impl_ts_getArrayHelper() const;

        /** finds a free property handle
            @param _rPropertyName
                the name of the property to find a handle for. If possible, the handle as determined by
                our ConcreteInfoService instance will be used
        */
        sal_Int32   impl_findFreeHandle( const ::rtl::OUString& _rPropertyName );
    };

    //--------------------------------------------------------------------
    inline ::comphelper::OPropertyArrayAggregationHelper& PropertyBagHelper::getInfoHelper() const
    {
        return impl_ts_getArrayHelper();
    }

    //--------------------------------------------------------------------
    inline void PropertyBagHelper::getDynamicFastPropertyValue( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _out_rValue ) const
    {
        m_aDynamicProperties.getFastPropertyValue( _nHandle, _out_rValue );
    }

    //--------------------------------------------------------------------
    inline bool PropertyBagHelper::convertDynamicFastPropertyValue( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rNewValue, ::com::sun::star::uno::Any& _out_rConvertedValue, ::com::sun::star::uno::Any& _out_rCurrentValue ) const
    {
        return m_aDynamicProperties.convertFastPropertyValue( _nHandle, _rNewValue, _out_rConvertedValue, _out_rCurrentValue );
    }

    //--------------------------------------------------------------------
    inline void PropertyBagHelper::setDynamicFastPropertyValue( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
    {
        m_aDynamicProperties.setFastPropertyValue( _nHandle, _rValue );
    }

    //--------------------------------------------------------------------
    inline void PropertyBagHelper::getDynamicPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _out_rValue ) const
    {
        m_aDynamicProperties.getPropertyDefaultByHandle( _nHandle, _out_rValue );
    }

    //--------------------------------------------------------------------
    inline bool PropertyBagHelper::hasDynamicPropertyByName( const ::rtl::OUString& _rName ) const
    {
        return m_aDynamicProperties.hasPropertyByName( _rName );
    }

    //--------------------------------------------------------------------
    inline bool PropertyBagHelper::hasDynamicPropertyByHandle( sal_Int32 _nHandle ) const
    {
        return m_aDynamicProperties.hasPropertyByHandle( _nHandle );
    }

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_PROPERTYBAGHELPER_HXX

