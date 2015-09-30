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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX

#include "propertyhandler.hxx"

#include <memory>


namespace pcr
{


    class XSDValidationHelper;

    //= XSDValidationPropertyHandler

    class XSDValidationPropertyHandler;
    typedef HandlerComponentBase< XSDValidationPropertyHandler > XSDValidationPropertyHandler_Base;
    class XSDValidationPropertyHandler : public XSDValidationPropertyHandler_Base
    {
    private:
        ::std::unique_ptr< XSDValidationHelper >  m_pHelper;

    public:
        XSDValidationPropertyHandler(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        static OUString SAL_CALL getImplementationName_static(  ) throw (css::uno::RuntimeException);
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (css::uno::RuntimeException);

    protected:
        virtual ~XSDValidationPropertyHandler();

    protected:
        // XPropertyHandler overriables
        virtual css::uno::Any               SAL_CALL getPropertyValue( const OUString& _rPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                        SAL_CALL setPropertyValue( const OUString& _rPropertyName, const css::uno::Any& _rValue ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString >
                                            SAL_CALL getSupersededProperties( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString >
                                            SAL_CALL getActuatingProperties( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::LineDescriptor
                                            SAL_CALL describePropertyLine( const OUString& _rPropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::inspection::InteractiveSelectionResult
                                            SAL_CALL onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, css::uno::Any& _rData, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                        SAL_CALL actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const css::uno::Any& _rNewValue, const css::uno::Any& _rOldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                        SAL_CALL addPropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::lang::NullPointerException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void                        SAL_CALL removePropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // PropertyHandler overridables
        virtual css::uno::Sequence< css::beans::Property >
                                            SAL_CALL doDescribeSupportedProperties() const SAL_OVERRIDE;
        virtual void onNewComponent() SAL_OVERRIDE;

    private:
        bool    implPrepareRemoveCurrentDataType();
        bool    implDoRemoveCurrentDataType();

        bool    implPrepareCloneDataCurrentType( OUString& _rNewName );
        bool    implDoCloneCurrentDataType( const OUString& _rNewName );

        /** retrieves the names of the data types which our introspectee can be validated against
        */
        void    implGetAvailableDataTypeNames( ::std::vector< OUString >& /* [out] */ _rNames ) const;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONPROPERTYHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
