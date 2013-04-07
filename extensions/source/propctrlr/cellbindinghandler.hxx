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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX

#include "propertyhandler.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ref.hxx>

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    class CellBindingHelper;
    class IPropertyEnumRepresentation;
    //====================================================================
    //= CellBindingPropertyHandler
    //====================================================================
    class CellBindingPropertyHandler;
    typedef HandlerComponentBase< CellBindingPropertyHandler > CellBindingPropertyHandler_Base;
    class CellBindingPropertyHandler : public CellBindingPropertyHandler_Base
    {
    private:
        ::std::auto_ptr< CellBindingHelper >            m_pHelper;
        ::rtl::Reference< IPropertyEnumRepresentation > m_pCellExchangeConverter;

    public:
        CellBindingPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        static OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~CellBindingPropertyHandler();

    protected:
        // XPropertyHandler overriables
        virtual ::com::sun::star::uno::Any      SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                            SAL_CALL setPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any      SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any      SAL_CALL convertToControlValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString >
                                                SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual void                            SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);

        // PropertyHandler overridables
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                                SAL_CALL doDescribeSupportedProperties() const;
        virtual void onNewComponent();

    private:
        /** updates a property (UI) whose state depends on more than one other property

            ->actuatingPropertyChanged is called for certain properties in whose changes
            we expressed interes (->getActuatingProperty). Now such a property change can
            result in simple UI updates, for instance another property being enabled or disabled.

            However, it can also result in a more complex change: The current (UI) state might
            depend on the value of more than one other property. Those dependent properties (their
            UI, more precisly) are updated in this method.

            @param _nPropid
                the ->PropertyId of the dependent property whose UI state is to be updated

            @param _rxInspectorUI
                provides access to the property browser UI. Must not be <NULL/>.
        */
        void impl_updateDependentProperty_nothrow( PropertyId _nPropId, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) const;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
