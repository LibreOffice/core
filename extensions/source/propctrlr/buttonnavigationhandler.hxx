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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX

#include "propertyhandler.hxx"


namespace pcr
{

    class ButtonNavigationHandler;
    typedef HandlerComponentBase< ButtonNavigationHandler > ButtonNavigationHandler_Base;
    /** a property handler for any virtual string properties
    */
    class ButtonNavigationHandler : public ButtonNavigationHandler_Base
    {
    private:
        css::uno::Reference< css::inspection::XPropertyHandler >
            m_xSlaveHandler;

    public:
        ButtonNavigationHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );
        static OUString SAL_CALL getImplementationName_static(  ) throw (css::uno::RuntimeException);
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (css::uno::RuntimeException);

    protected:
        virtual ~ButtonNavigationHandler();

        static bool    isNavigationCapableButton( const css::uno::Reference< css::beans::XPropertySet >& _rxComponent );

    protected:
        // XPropertyHandler overriables
        virtual void                                    SAL_CALL inspect( const css::uno::Reference< css::uno::XInterface >& _rxIntrospectee ) throw (css::uno::RuntimeException, css::lang::NullPointerException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any                           SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                    SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::beans::PropertyState               SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString >          SAL_CALL getActuatingProperties( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::InteractiveSelectionResult
                                                        SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, css::uno::Any& _rData, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                    SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::LineDescriptor         SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // PropertyHandler overridables
        virtual css::uno::Sequence< css::beans::Property >
                                                        SAL_CALL doDescribeSupportedProperties() const SAL_OVERRIDE;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
