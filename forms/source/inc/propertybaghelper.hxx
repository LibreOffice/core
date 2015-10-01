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

#ifndef INCLUDED_FORMS_SOURCE_INC_PROPERTYBAGHELPER_HXX
#define INCLUDED_FORMS_SOURCE_INC_PROPERTYBAGHELPER_HXX

#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/propertybag.hxx>
#include <comphelper/propagg.hxx>

#include <boost/noncopyable.hpp>


namespace frm
{



    //= class IPropertyBagHelperContext

    class SAL_NO_VTABLE IPropertyBagHelperContext
    {
    public:
        virtual ::osl::Mutex&   getMutex() = 0;

        virtual void            describeFixedAndAggregateProperties(
            css::uno::Sequence< css::beans::Property >& _out_rFixedProperties,
            css::uno::Sequence< css::beans::Property >& _out_rAggregateProperties
        ) const = 0;

        virtual css::uno::Reference< css::beans::XMultiPropertySet >
                                getPropertiesInterface() = 0;

    protected:
        ~IPropertyBagHelperContext() {}
    };

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
        void addProperty( const OUString& _rName, ::sal_Int16 _nAttributes, const css::uno::Any& _rInitialValue );
        void removeProperty( const OUString& _rName );

        // XPropertyAccess equivalent
        css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues();
        void setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& _rProps );

        // forwards to m_aDynamicProperties
        inline void getDynamicFastPropertyValue( sal_Int32 _nHandle, css::uno::Any& _out_rValue ) const;
        inline bool convertDynamicFastPropertyValue( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, css::uno::Any& _out_rConvertedValue, css::uno::Any& _out_rCurrentValue ) const;
        inline void setDynamicFastPropertyValue( sal_Int32 _nHandle, const css::uno::Any& _rValue );
        inline void getDynamicPropertyDefaultByHandle( sal_Int32 _nHandle, css::uno::Any& _out_rValue ) const;
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
        sal_Int32   impl_findFreeHandle( const OUString& _rPropertyName );
    };


    inline ::comphelper::OPropertyArrayAggregationHelper& PropertyBagHelper::getInfoHelper() const
    {
        return impl_ts_getArrayHelper();
    }


    inline void PropertyBagHelper::getDynamicFastPropertyValue( sal_Int32 _nHandle, css::uno::Any& _out_rValue ) const
    {
        m_aDynamicProperties.getFastPropertyValue( _nHandle, _out_rValue );
    }


    inline bool PropertyBagHelper::convertDynamicFastPropertyValue( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, css::uno::Any& _out_rConvertedValue, css::uno::Any& _out_rCurrentValue ) const
    {
        return m_aDynamicProperties.convertFastPropertyValue( _nHandle, _rNewValue, _out_rConvertedValue, _out_rCurrentValue );
    }


    inline void PropertyBagHelper::setDynamicFastPropertyValue( sal_Int32 _nHandle, const css::uno::Any& _rValue )
    {
        m_aDynamicProperties.setFastPropertyValue( _nHandle, _rValue );
    }


    inline void PropertyBagHelper::getDynamicPropertyDefaultByHandle( sal_Int32 _nHandle, css::uno::Any& _out_rValue ) const
    {
        m_aDynamicProperties.getPropertyDefaultByHandle( _nHandle, _out_rValue );
    }

    inline bool PropertyBagHelper::hasDynamicPropertyByHandle( sal_Int32 _nHandle ) const
    {
        return m_aDynamicProperties.hasPropertyByHandle( _nHandle );
    }


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_PROPERTYBAGHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
