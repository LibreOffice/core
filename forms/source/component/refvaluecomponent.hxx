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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX

#include "FormComponent.hxx"
#include "togglestate.hxx"

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OReferenceValueComponent
    //====================================================================
    /** a OBoundControlModel which features the exchange of a reference value
    */
    class OReferenceValueComponent : public OBoundControlModel
    {
    private:
        // <properties>
        OUString     m_sReferenceValue;          // the reference value to use for data exchange
        OUString     m_sNoCheckReferenceValue;   // the reference value to be exchanged when the control is not checked
        ToggleState         m_eDefaultChecked;          // the default check state
        // </properties>

        sal_Bool            m_bSupportSecondRefValue;       // do we support the SecondaryRefValue property?

    protected:
        const OUString& getReferenceValue() const { return m_sReferenceValue; }
        void                   setReferenceValue( const OUString& _rRefValue );

        const OUString& getNoCheckReferenceValue() const { return m_sNoCheckReferenceValue; }

        ToggleState            getDefaultChecked() const { return m_eDefaultChecked; }
        void                   setDefaultChecked( ToggleState _eChecked ) { m_eDefaultChecked = _eChecked; }

    protected:
        OReferenceValueComponent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory,
            const OUString& _rUnoControlModelTypeName,
            const OUString& _rDefault,
            sal_Bool _bSupportNoCheckRefValue = sal_False
        );
        DECLARE_DEFAULT_CLONE_CTOR( OReferenceValueComponent )
        DECLARE_DEFAULT_DTOR( OReferenceValueComponent );

        // OPropertySet and friends
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                    throw (::com::sun::star::uno::Exception, std::exception);
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                    ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                    throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const;
        using ::cppu::OPropertySetHelper::getFastPropertyValue;

        // OBoundControlModel overridables
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
                                getSupportedBindingTypes();
        virtual ::com::sun::star::uno::Any
                                translateExternalValueToControlValue( const ::com::sun::star::uno::Any& _rExternalValue ) const;
        virtual ::com::sun::star::uno::Any
                                translateControlValueToExternalValue( ) const;

        virtual ::com::sun::star::uno::Any
                                translateControlValueToValidatableValue( ) const;

        virtual ::com::sun::star::uno::Any
                                getDefaultForReset() const;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
