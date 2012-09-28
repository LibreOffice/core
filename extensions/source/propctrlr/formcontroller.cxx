/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "formcontroller.hxx"
#include "pcrcommon.hxx"
#include "formstrings.hxx"
#include "defaultforminspection.hxx"
#include "propctrlr.hrc"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/vclunohelper.hxx>

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_FormController()
{
    ::pcr::OAutoRegistration< ::pcr::FormController > aFormControllerRegistration;
    ::pcr::OAutoRegistration< ::pcr::DialogController > aDialogControllerRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

    /** === begin UNO using === **/
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
    using ::com::sun::star::form::XGridColumnFactory;
    using ::com::sun::star::form::XForm;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::awt::XWindow;
    /** === end UNO using === **/

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //====================================================================
    //= FormController
    //====================================================================
    //--------------------------------------------------------------------
    FormController::FormController( const Reference< XComponentContext >& _rxContext, ServiceDescriptor _aServiceDescriptor,
            bool _bUseFormFormComponentHandlers )
        :OPropertyBrowserController( _rxContext )
        ,FormController_PropertyBase1( m_aBHelper )
        ,m_aServiceDescriptor( _aServiceDescriptor )
    {
        osl_atomic_increment( &m_refCount );
        {
            Reference< XObjectInspectorModel > xModel(
                *(new DefaultFormComponentInspectorModel( _rxContext, _bUseFormFormComponentHandlers )),
                UNO_QUERY_THROW
            );
            setInspectorModel( xModel );
        }
        osl_atomic_decrement( &m_refCount );
    }

    //------------------------------------------------------------------------
    FormController::~FormController()
    {
    }

    //------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( FormController, OPropertyBrowserController, FormController_PropertyBase1 )

    //------------------------------------------------------------------------
    Sequence< Type > SAL_CALL FormController::getTypes(  ) throw(RuntimeException)
    {
        ::cppu::OTypeCollection aTypes(
            ::getCppuType( static_cast< Reference< XPropertySet >* >(NULL) ),
            ::getCppuType( static_cast< Reference< XMultiPropertySet >* >(NULL) ),
            ::getCppuType( static_cast< Reference< XFastPropertySet >* >(NULL) ),
            OPropertyBrowserController::getTypes());
        return aTypes.getTypes();
    }

    //------------------------------------------------------------------------
    IMPLEMENT_GET_IMPLEMENTATION_ID( FormController )

    //------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL FormController::getImplementationName(  ) throw(RuntimeException)
    {
        return m_aServiceDescriptor.GetImplementationName();
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL FormController::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( m_aServiceDescriptor.GetSupportedServiceNames() );
        aSupported.realloc( aSupported.getLength() + 1 );
        aSupported[ aSupported.getLength() - 1 ] = ::rtl::OUString(  "com.sun.star.inspection.ObjectInspector"  );
        return aSupported;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString FormController::getImplementationName_static(  ) throw(RuntimeException)
    {
        return ::rtl::OUString("org.openoffice.comp.extensions.FormController");
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > FormController::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString("com.sun.star.form.PropertyBrowserController");
        return aSupported;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL FormController::Create(const Reference< XComponentContext >& _rxContext )
    {
        ServiceDescriptor aService;
        aService.GetImplementationName = &FormController::getImplementationName_static;
        aService.GetSupportedServiceNames = &FormController::getSupportedServiceNames_static;
        return *(new FormController( _rxContext, aService, true ) );
    }

    //------------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL FormController::getPropertySetInfo(  ) throw(RuntimeException)
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
    }

    //------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL FormController::getInfoHelper()
    {
        return *getArrayHelper();
    }

    //------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* FormController::createArrayHelper( ) const
    {
        Sequence< Property > aProps( 2 );
        aProps[0] = Property(
            PROPERTY_CURRENTPAGE,
            OWN_PROPERTY_ID_CURRENTPAGE,
            ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ),
            PropertyAttribute::TRANSIENT
        );
        aProps[1] = Property(
            PROPERTY_INTROSPECTEDOBJECT,
            OWN_PROPERTY_ID_INTROSPECTEDOBJECT,
            XPropertySet::static_type(),
            PropertyAttribute::TRANSIENT | PropertyAttribute::CONSTRAINED
        );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //------------------------------------------------------------------------
    sal_Bool SAL_CALL FormController::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue ) throw (IllegalArgumentException)
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
        return sal_True;
    }

    //------------------------------------------------------------------------
    void SAL_CALL FormController::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (Exception)
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

    //------------------------------------------------------------------------
    void SAL_CALL FormController::getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const
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

    //====================================================================
    //= DialogController
    //====================================================================
    //------------------------------------------------------------------------
    ::rtl::OUString DialogController::getImplementationName_static(  ) throw(RuntimeException)
    {
        return ::rtl::OUString("org.openoffice.comp.extensions.DialogController");
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > DialogController::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString("com.sun.star.awt.PropertyBrowserController");
        return aSupported;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL DialogController::Create(const Reference< XComponentContext >& _rxContext)
    {
        ServiceDescriptor aService;
        aService.GetImplementationName = &DialogController::getImplementationName_static;
        aService.GetSupportedServiceNames = &DialogController::getSupportedServiceNames_static;
        return *(new FormController( _rxContext, aService, false ) );
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
