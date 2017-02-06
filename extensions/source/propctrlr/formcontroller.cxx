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

#include "formcontroller.hxx"
#include "pcrcommon.hxx"
#include "pcrservices.hxx"
#include "formstrings.hxx"
#include "defaultforminspection.hxx"
#include "propctrlr.hrc"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/vclunohelper.hxx>


extern "C" void SAL_CALL createRegistryInfo_FormController()
{
    ::pcr::OAutoRegistration< ::pcr::FormController > aFormControllerRegistration;
    ::pcr::OAutoRegistration< ::pcr::DialogController > aDialogControllerRegistration;
}


namespace pcr
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::TypeClass_INTERFACE;
    using ::com::sun::star::uno::TypeClass_STRING;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::inspection::XObjectInspectorModel;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::util::VetoException;
    using ::com::sun::star::beans::PropertyVetoException;
    using ::com::sun::star::uno::UNO_QUERY;

    namespace PropertyAttribute = css::beans::PropertyAttribute;


    //= FormController


    FormController::FormController( const Reference< XComponentContext >& _rxContext, ServiceDescriptor _aServiceDescriptor,
            bool _bUseFormFormComponentHandlers )
        :OPropertyBrowserController( _rxContext )
        ,FormController_PropertyBase1( m_aBHelper )
        ,m_aServiceDescriptor( _aServiceDescriptor )
    {
        osl_atomic_increment( &m_refCount );
        {
            Reference< XObjectInspectorModel > xModel(
                *(new DefaultFormComponentInspectorModel( _bUseFormFormComponentHandlers )),
                UNO_QUERY_THROW
            );
            setInspectorModel( xModel );
        }
        osl_atomic_decrement( &m_refCount );
    }


    FormController::~FormController()
    {
    }


    IMPLEMENT_FORWARD_XINTERFACE2( FormController, OPropertyBrowserController, FormController_PropertyBase1 )


    Sequence< Type > SAL_CALL FormController::getTypes(  )
    {
        ::cppu::OTypeCollection aTypes(
            cppu::UnoType<XPropertySet>::get(),
            cppu::UnoType<XMultiPropertySet>::get(),
            cppu::UnoType<XFastPropertySet>::get(),
            OPropertyBrowserController::getTypes());
        return aTypes.getTypes();
    }


    IMPLEMENT_GET_IMPLEMENTATION_ID( FormController )


    OUString SAL_CALL FormController::getImplementationName(  )
    {
        return m_aServiceDescriptor.GetImplementationName();
    }


    Sequence< OUString > SAL_CALL FormController::getSupportedServiceNames(  )
    {
        Sequence< OUString > aSupported( m_aServiceDescriptor.GetSupportedServiceNames() );
        aSupported.realloc( aSupported.getLength() + 1 );
        aSupported[ aSupported.getLength() - 1 ] = "com.sun.star.inspection.ObjectInspector";
        return aSupported;
    }


    OUString FormController::getImplementationName_static(  )
    {
        return OUString("org.openoffice.comp.extensions.FormController");
    }


    Sequence< OUString > FormController::getSupportedServiceNames_static(  )
    {
        Sequence< OUString > aSupported { "com.sun.star.form.PropertyBrowserController" };
        return aSupported;
    }


    Reference< XInterface > SAL_CALL FormController::Create(const Reference< XComponentContext >& _rxContext )
    {
        ServiceDescriptor aService;
        aService.GetImplementationName = &FormController::getImplementationName_static;
        aService.GetSupportedServiceNames = &FormController::getSupportedServiceNames_static;
        return *(new FormController( _rxContext, aService, true ) );
    }


    Reference< XPropertySetInfo > SAL_CALL FormController::getPropertySetInfo(  )
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
    }


    ::cppu::IPropertyArrayHelper& SAL_CALL FormController::getInfoHelper()
    {
        return *getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* FormController::createArrayHelper( ) const
    {
        Sequence< Property > aProps( 2 );
        aProps[0] = Property(
            PROPERTY_CURRENTPAGE,
            OWN_PROPERTY_ID_CURRENTPAGE,
            ::cppu::UnoType<OUString>::get(),
            PropertyAttribute::TRANSIENT
        );
        aProps[1] = Property(
            PROPERTY_INTROSPECTEDOBJECT,
            OWN_PROPERTY_ID_INTROSPECTEDOBJECT,
            cppu::UnoType<XPropertySet>::get(),
            PropertyAttribute::TRANSIENT | PropertyAttribute::CONSTRAINED
        );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }


    sal_Bool SAL_CALL FormController::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue )
    {
        switch ( nHandle )
        {
        case OWN_PROPERTY_ID_INTROSPECTEDOBJECT:
            if ( rValue.getValueTypeClass() != TypeClass_INTERFACE )
                throw IllegalArgumentException();
            break;
        case OWN_PROPERTY_ID_CURRENTPAGE:
            if ( rValue.getValueTypeClass() != TypeClass_STRING )
                throw IllegalArgumentException();
            break;
        }

        getFastPropertyValue( rOldValue, nHandle );
        rConvertedValue = rValue;
        return true;
    }


    void SAL_CALL FormController::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
    {
        switch ( _nHandle )
        {
        case OWN_PROPERTY_ID_INTROSPECTEDOBJECT:
        {
            Reference< XObjectInspectorModel > xModel( getInspectorModel() );
            if ( xModel.is() )
            {
                try
                {
                    m_xCurrentInspectee.set( _rValue, UNO_QUERY );
                    Sequence< Reference< XInterface > > aObjects;
                    if ( m_xCurrentInspectee.is() )
                    {
                        aObjects.realloc( 1 );
                        aObjects[0] = m_xCurrentInspectee;
                    }

                    Reference< XObjectInspector > xInspector( *this, UNO_QUERY_THROW );
                    xInspector->inspect( aObjects );
                }
                catch( const VetoException& e )
                {
                    throw PropertyVetoException( e.Message, e.Context );
                }
            }
        }
        break;
        case OWN_PROPERTY_ID_CURRENTPAGE:
            restoreViewData( _rValue );
            break;
        }
    }


    void SAL_CALL FormController::getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const
    {
        switch ( nHandle )
        {
        case OWN_PROPERTY_ID_INTROSPECTEDOBJECT:
            rValue <<= m_xCurrentInspectee;
            break;

        case OWN_PROPERTY_ID_CURRENTPAGE:
            rValue = const_cast< FormController* >( this )->getViewData();
            break;
        }
    }


    //= DialogController


    OUString DialogController::getImplementationName_static(  )
    {
        return OUString("org.openoffice.comp.extensions.DialogController");
    }


    Sequence< OUString > DialogController::getSupportedServiceNames_static(  )
    {
        Sequence< OUString > aSupported { "com.sun.star.awt.PropertyBrowserController" };
        return aSupported;
    }


    Reference< XInterface > SAL_CALL DialogController::Create(const Reference< XComponentContext >& _rxContext)
    {
        ServiceDescriptor aService;
        aService.GetImplementationName = &DialogController::getImplementationName_static;
        aService.GetSupportedServiceNames = &DialogController::getSupportedServiceNames_static;
        return *(new FormController( _rxContext, aService, false ) );
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
