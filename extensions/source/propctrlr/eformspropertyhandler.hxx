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

#pragma once

#include "propertyhandler.hxx"

#include <memory>


namespace pcr
{


    class EFormsHelper;

    //= EFormsPropertyHandler

    class EFormsPropertyHandler;
    class EFormsPropertyHandler : public PropertyHandlerComponent
    {
    private:
        std::unique_ptr< EFormsHelper > m_pHelper;
        /** current value of the Model property, if there is no binding, yet
        */
        OUString                 m_sBindingLessModelName;
        /** are we currently simulating a propertyChange event of the Model property?
        */
        bool                            m_bSimulatingModelChange;

    public:
        explicit EFormsPropertyHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

    protected:
        virtual ~EFormsPropertyHandler() override;

    protected:
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames () override;

        // XPropertyHandler overriables
        virtual css::uno::Any                       SAL_CALL getPropertyValue( const OUString& _rPropertyName ) override;
        virtual void                                SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) override;
        virtual css::uno::Sequence< OUString >
                                                    SAL_CALL getActuatingProperties( ) override;
        virtual css::uno::Sequence< OUString >
                                                    SAL_CALL getSupersededProperties( ) override;
        virtual css::inspection::LineDescriptor
                                                    SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) override;
        virtual css::inspection::InteractiveSelectionResult
                                                    SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, css::uno::Any& _rData, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) override;
        virtual void                                SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) override;
        virtual css::uno::Any                       SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rControlValue ) override;
        virtual css::uno::Any                       SAL_CALL convertToControlValue( const OUString& _rPropertyName, const css::uno::Any& _rPropertyValue, const css::uno::Type& _rControlValueType ) override;
        virtual void                                SAL_CALL addPropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) override;
        virtual void                                SAL_CALL removePropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) override;

        // PropertyHandler overridables
        virtual css::uno::Sequence< css::beans::Property >
                                                    doDescribeSupportedProperties() const override;
        virtual void onNewComponent() override;

    protected:
        /** returns the value of the PROPERTY_XML_DATA_MODEL property.

            An extra method is necessary here, which respects both the value set at our helper,
            and <member>m_sBindingLessModelName</member>
        */
        OUString getModelNamePropertyValue() const;
    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
