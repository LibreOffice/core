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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX

#include "propertyhandler.hxx"
#include "eformshelper.hxx"

#include <com/sun/star/xforms/XSubmission.hpp>
#include <comphelper/propmultiplex.hxx>

namespace comphelper
{
    class OPropertyChangeMultiplexer;
}


namespace pcr
{



    //= SubmissionHelper

    class SubmissionHelper : public EFormsHelper
    {
    public:
        SubmissionHelper(
            osl::Mutex& _rMutex,
            const css::uno::Reference< css::beans::XPropertySet >& _rxIntrospectee,
            const css::uno::Reference< css::frame::XModel >& _rxContextDocument
        );

        /** determines whether the given control model is able to trigger submissions

            Instances of the <type>SubmissionHelper</type> class should not be instantiated
            for components where this method returned <FALSE/>
        */
        static bool canTriggerSubmissions(
                    const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
                    const css::uno::Reference< css::frame::XModel >& _rxContextDocument
                );
    };


    //= SubmissionPropertyHandler

    class SubmissionPropertyHandler;
    typedef HandlerComponentBase< SubmissionPropertyHandler > EditPropertyHandler_Base;
    /** a property handler for any virtual string properties
    */
    class SubmissionPropertyHandler : public EditPropertyHandler_Base, public ::comphelper::OPropertyChangeListener
    {
    private:
        ::osl::Mutex                                m_aMutex;
        ::std::unique_ptr< SubmissionHelper >       m_pHelper;
        ::comphelper::OPropertyChangeMultiplexer*   m_pPropChangeMultiplexer;

    public:
        explicit SubmissionPropertyHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        static OUString SAL_CALL getImplementationName_static(  ) throw (css::uno::RuntimeException);
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (css::uno::RuntimeException);

        virtual ~SubmissionPropertyHandler();

    protected:
        // XPropertyHandler overriables
        virtual css::uno::Any               SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual void                        SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString >
                                            SAL_CALL getActuatingProperties( ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString >
                                            SAL_CALL getSupersededProperties( ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::inspection::LineDescriptor
                                            SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;
        virtual void                        SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any               SAL_CALL convertToPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rControlValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any               SAL_CALL convertToControlValue( const OUString& _rPropertyName, const css::uno::Any& _rPropertyValue, const css::uno::Type& _rControlValueType ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;

        // PropertyHandler overridables
        virtual css::uno::Sequence< css::beans::Property >
                                            SAL_CALL doDescribeSupportedProperties() const override;
        virtual void onNewComponent() override;

    private:
        // OPropertyChangeListener
        virtual void _propertyChanged(const css::beans::PropertyChangeEvent& _rEvent) throw( css::uno::RuntimeException) override;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
