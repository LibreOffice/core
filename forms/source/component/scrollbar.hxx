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
#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX

#include "FormComponent.hxx"

namespace frm
{

    class OScrollBarModel   :public OBoundControlModel
    {
    private:
        // <properties>
        sal_Int32   m_nDefaultScrollValue;
        // </properties>

    public:
        DECLARE_DEFAULT_LEAF_XTOR( OScrollBarModel );

    protected:
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw (::css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPersistObject
        DECLARE_XPERSISTOBJECT()

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertyState
        virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const SAL_OVERRIDE;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
        ) const SAL_OVERRIDE;

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& _rValue, sal_Int32 _nHandle ) const SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const css::uno::Any& _rValue )
            throw ( css::uno::Exception, std::exception ) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue )
            throw ( css::lang::IllegalArgumentException ) SAL_OVERRIDE;

        // OBoundControlModel
        virtual css::uno::Any   translateDbColumnToControlValue( ) SAL_OVERRIDE;
        virtual bool            commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;
        virtual css::uno::Any   getDefaultForReset() const SAL_OVERRIDE;

        virtual css::uno::Sequence< css::uno::Type >
                                getSupportedBindingTypes() SAL_OVERRIDE;
        virtual css::uno::Any   translateExternalValueToControlValue( const css::uno::Any& _rExternalValue ) const SAL_OVERRIDE;
        virtual css::uno::Any   translateControlValueToExternalValue( ) const SAL_OVERRIDE;

        // XCoponent and related helpers
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // prevent method hiding
        using OBoundControlModel::disposing;
        using OBoundControlModel::getFastPropertyValue;

    };

    css::uno::Any translateExternalDoubleToControlIntValue(
        const css::uno::Any& _rExternalValue, const css::uno::Reference< css::beans::XPropertySet >& _rxProperties,
        const OUString& _rMinValueName, const OUString& _rMaxValueName );

    css::uno::Any translateControlIntToExternalDoubleValue( const css::uno::Any& _rControlIntValue );

} // namespacefrm

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_SCROLLBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
