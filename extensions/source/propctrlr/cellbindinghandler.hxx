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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX

#include "propertyhandler.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ref.hxx>

#include <memory>


namespace pcr
{


    class CellBindingHelper;
    class IPropertyEnumRepresentation;

    class CellBindingPropertyHandler;
    typedef HandlerComponentBase< CellBindingPropertyHandler > CellBindingPropertyHandler_Base;
    class CellBindingPropertyHandler : public CellBindingPropertyHandler_Base
    {
    private:
        ::std::unique_ptr< CellBindingHelper >          m_pHelper;
        ::rtl::Reference< IPropertyEnumRepresentation > m_pCellExchangeConverter;

    public:
        explicit CellBindingPropertyHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        static OUString SAL_CALL getImplementationName_static(  ) throw (css::uno::RuntimeException);
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (css::uno::RuntimeException);

    protected:
        virtual ~CellBindingPropertyHandler();

    protected:
        // XPropertyHandler overriables
        virtual css::uno::Any                   SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual void                            SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any                   SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rControlValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any                   SAL_CALL convertToControlValue( const OUString& _rPropertyName, const css::uno::Any& _rPropertyValue, const css::uno::Type& _rControlValueType ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString >  SAL_CALL getActuatingProperties( ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void                            SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;

        // PropertyHandler overridables
        virtual css::uno::Sequence< css::beans::Property >
                                                SAL_CALL doDescribeSupportedProperties() const override;
        virtual void onNewComponent() override;

    private:
        /** updates a property (UI) whose state depends on more than one other property

            ->actuatingPropertyChanged is called for certain properties in whose changes
            we expressed interes (->getActuatingProperty). Now such a property change can
            result in simple UI updates, for instance another property being enabled or disabled.

            However, it can also result in a more complex change: The current (UI) state might
            depend on the value of more than one other property. Those dependent properties (their
            UI, more precisely) are updated in this method.

            @param _nPropid
                the ->PropertyId of the dependent property whose UI state is to be updated

            @param _rxInspectorUI
                provides access to the property browser UI. Must not be <NULL/>.
        */
        void impl_updateDependentProperty_nothrow( PropertyId _nPropId, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) const;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_CELLBINDINGHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
