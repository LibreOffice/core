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
#include "formstrings.hxx"
#include "defaultforminspection.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <utility>


namespace pcr
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::TypeClass_INTERFACE;
    using ::com::sun::star::uno::TypeClass_STRING;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::inspection::XObjectInspectorModel;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::util::VetoException;
    using ::com::sun::star::beans::PropertyVetoException;
    using ::com::sun::star::uno::UNO_QUERY;

    namespace PropertyAttribute = css::beans::PropertyAttribute;


    //= FormController


    FormController::FormController( const Reference< XComponentContext >& _rxContext,
            OUString sImplementationName,
            const css::uno::Sequence<OUString>& aSupportedServiceNames,
            bool _bUseFormFormComponentHandlers )
        :OPropertyBrowserController( _rxContext )
        ,FormController_PropertyBase1( m_aBHelper )
        ,m_sImplementationName(std::move( sImplementationName ))
        ,m_aSupportedServiceNames( aSupportedServiceNames )
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
        return m_sImplementationName;
    }


    Sequence< OUString > SAL_CALL FormController::getSupportedServiceNames(  )
    {
        Sequence< OUString > aSupported( m_aSupportedServiceNames );
        aSupported.realloc( aSupported.getLength() + 1 );
        aSupported.getArray()[ aSupported.getLength() - 1 ] = "com.sun.star.inspection.ObjectInspector";
        return aSupported;
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
        Sequence< Property > aProps{
            Property(
                PROPERTY_CURRENTPAGE,
                static_cast<sal_Int32>(OwnPropertyId::CURRENTPAGE),
                ::cppu::UnoType<OUString>::get(),
                PropertyAttribute::TRANSIENT
            ),
            Property(
                PROPERTY_INTROSPECTEDOBJECT,
                static_cast<sal_Int32>(OwnPropertyId::INTROSPECTEDOBJECT),
                cppu::UnoType<XPropertySet>::get(),
                PropertyAttribute::TRANSIENT | PropertyAttribute::CONSTRAINED
            )
        };
        return new ::cppu::OPropertyArrayHelper( aProps );
    }


    sal_Bool SAL_CALL FormController::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue )
    {
        switch ( static_cast<OwnPropertyId>(nHandle) )
        {
        case OwnPropertyId::INTROSPECTEDOBJECT:
            if ( rValue.getValueTypeClass() != TypeClass_INTERFACE )
                throw IllegalArgumentException();
            break;
        case OwnPropertyId::CURRENTPAGE:
            if ( rValue.getValueTypeClass() != TypeClass_STRING )
                throw IllegalArgumentException();
            break;
        default:
            break;
        }

        getFastPropertyValue( rOldValue, nHandle );
        rConvertedValue = rValue;
        return true;
    }


    void SAL_CALL FormController::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
    {
        switch ( static_cast<OwnPropertyId>(_nHandle) )
        {
        case OwnPropertyId::INTROSPECTEDOBJECT:
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
                        aObjects = { m_xCurrentInspectee };
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
        case OwnPropertyId::CURRENTPAGE:
            restoreViewData( _rValue );
            break;
        default:
            break;
        }
    }


    void SAL_CALL FormController::getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const
    {
        switch ( static_cast<OwnPropertyId>(nHandle) )
        {
        case OwnPropertyId::INTROSPECTEDOBJECT:
            rValue <<= m_xCurrentInspectee;
            break;

        case OwnPropertyId::CURRENTPAGE:
            rValue = const_cast< FormController* >( this )->getViewData();
            break;
        default:
            break;
        }
    }



} // namespace pcr

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_propctrlr_FormController_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pcr::FormController( context,
                u"org.openoffice.comp.extensions.FormController"_ustr,
                { u"com.sun.star.form.PropertyBrowserController"_ustr },
                true ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_propctrlr_DialogController_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pcr::FormController( context,
            u"org.openoffice.comp.extensions.DialogController"_ustr,
            { u"com.sun.star.awt.PropertyBrowserController"_ustr },
            false ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
