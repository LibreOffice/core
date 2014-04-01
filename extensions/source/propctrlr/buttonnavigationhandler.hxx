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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX

#include "propertyhandler.hxx"


namespace pcr
{



    //= ButtonNavigationHandler

    class ButtonNavigationHandler;
    typedef HandlerComponentBase< ButtonNavigationHandler > ButtonNavigationHandler_Base;
    /** a property handler for any virtual string properties
    */
    class ButtonNavigationHandler : public ButtonNavigationHandler_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >
            m_xSlaveHandler;

    public:
        ButtonNavigationHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );
        static OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~ButtonNavigationHandler();

        static bool    isNavigationCapableButton( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxComponent );

    protected:
        // XPropertyHandler overriables
        virtual void                                    SAL_CALL inspect( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIntrospectee ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any              SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                    SAL_CALL setPropertyValue( const OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::beans::PropertyState  SAL_CALL getPropertyState( const OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString >
                                                        SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::inspection::InteractiveSelectionResult
                                                        SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                                    SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::inspection::LineDescriptor
                                                        SAL_CALL describePropertyLine( const OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // PropertyHandler overridables
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                                        SAL_CALL doDescribeSupportedProperties() const SAL_OVERRIDE;
    };


} // namespace pcr


#endif // EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
