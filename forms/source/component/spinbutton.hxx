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
#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX

#include "FormComponent.hxx"


namespace frm
{

    class OSpinButtonModel   :public OBoundControlModel
    {
    private:
        // <properties>
        sal_Int32   m_nDefaultSpinValue;
        // </properties>

    public:
        DECLARE_DEFAULT_LEAF_XTOR( OSpinButtonModel );

    protected:
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::css::uno::RuntimeException, std::exception) override;
        virtual ::css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::css::uno::RuntimeException, std::exception) override;

        // XPersistObject
        DECLARE_XPERSISTOBJECT()

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XPropertyState
        virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const override;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
        ) const override;

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& _rValue, sal_Int32 _nHandle ) const override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const css::uno::Any& _rValue )
            throw ( css::uno::Exception, std::exception ) override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue )
            throw ( css::lang::IllegalArgumentException ) override;

        // OBoundControlModel
        virtual css::uno::Any   translateDbColumnToControlValue( ) override;
        virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;
        virtual css::uno::Any   getDefaultForReset() const override;

        virtual css::uno::Sequence< css::uno::Type >
                                getSupportedBindingTypes() override;
        virtual css::uno::Any   translateExternalValueToControlValue( const css::uno::Any& _rExternalValue ) const override;
        virtual css::uno::Any   translateControlValueToExternalValue( ) const override;

        // XCoponent and related helpers
        virtual void SAL_CALL disposing() override;

        // prevent method hiding
        using OBoundControlModel::disposing;
        using OBoundControlModel::getFastPropertyValue;

    };

} // namespacefrm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_SPINBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
