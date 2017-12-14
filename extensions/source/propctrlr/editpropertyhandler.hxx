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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EDITPROPERTYHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EDITPROPERTYHANDLER_HXX

#include "propertyhandler.hxx"


namespace pcr
{


    //= EditPropertyHandler

    class EditPropertyHandler;
    typedef HandlerComponentBase< EditPropertyHandler > EditPropertyHandler_Base;
    /** a property handler for any virtual string properties
    */
    class EditPropertyHandler : public EditPropertyHandler_Base
    {
    public:
        explicit EditPropertyHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_static(  );
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_static(  );

    protected:
        virtual ~EditPropertyHandler() override;

    protected:
        // XPropertyHandler overriables
        virtual css::uno::Any                   SAL_CALL getPropertyValue( const OUString& _rPropertyName ) override;
        virtual void                            SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) override;
        virtual css::uno::Sequence< OUString >  SAL_CALL getSupersededProperties( ) override;
        virtual css::uno::Sequence< OUString >  SAL_CALL getActuatingProperties( ) override;
        virtual void                            SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) override;

        // PropertyHandler overridables
        virtual css::uno::Sequence< css::beans::Property >
                                                doDescribeSupportedProperties() const override;
    private:
        bool    implHaveBothScrollBarProperties() const;
        bool    implHaveTextTypeProperty() const;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_EDITPROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
