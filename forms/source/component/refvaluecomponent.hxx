/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: refvaluecomponent.hxx,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef EFORMS2_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX
#define EFORMS2_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX

#include "FormComponent.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace frm
{
//........................................................................

    enum CheckState { STATE_NOCHECK = 0, STATE_CHECK = 1, STATE_DONTKNOW = 2 };

    //====================================================================
    //= OReferenceValueComponent
    //====================================================================
    /** a OBoundControlModel which features the exchange of a reference value
    */
    class OReferenceValueComponent : public OBoundControlModel
    {
    private:
        // <properties>
        ::rtl::OUString     m_sReferenceValue;          // the reference value to use for data exchange
        ::rtl::OUString     m_sNoCheckReferenceValue;   // the reference value to be exchanged when the control is not checked
        CheckState          m_eDefaultChecked;          // the default check state
        // </properties>

        sal_Bool            m_bSupportSecondRefValue;       // do we support the SecondaryRefValue property?

        /** type how we should transfer our selection to external value bindings
        */
        enum ValueExchangeType
        {
            eString,
            eBoolean
        };
        ValueExchangeType   m_eValueExchangeType;

    protected:
        const ::rtl::OUString& getReferenceValue() const { return m_sReferenceValue; }
        void                   setReferenceValue( const ::rtl::OUString& _rRefValue );

        const ::rtl::OUString& getNoCheckReferenceValue() const { return m_sNoCheckReferenceValue; }
        void                   setNoCheckReferenceValue( const ::rtl::OUString& _rNoCheckRefValue );

        CheckState             getDefaultChecked() const { return m_eDefaultChecked; }
        void                   setDefaultChecked( CheckState _eChecked ) { m_eDefaultChecked = _eChecked; }

    protected:
        OReferenceValueComponent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
            const ::rtl::OUString& _rUnoControlModelTypeName,
            const ::rtl::OUString& _rDefault,
            sal_Bool _bSupportNoCheckRefValue = sal_False
        );
        DECLARE_DEFAULT_CLONE_CTOR( OReferenceValueComponent )
        DECLARE_DEFAULT_DTOR( OReferenceValueComponent );

        // OPropertySet and friends
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                    throw (::com::sun::star::uno::Exception);
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                    ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                    throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const;
        using ::cppu::OPropertySetHelper::getFastPropertyValue;

        // OBoundControlModel overridables
        virtual void            onConnectedExternalValue( );
        virtual sal_Bool        approveValueBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XValueBinding >& _rxBinding );

        virtual ::com::sun::star::uno::Any
                                translateExternalValueToControlValue( ) const;
        virtual ::com::sun::star::uno::Any
                                translateControlValueToExternalValue( ) const;

        virtual ::com::sun::star::uno::Any
                                translateControlValueToValidatableValue( ) const;

        virtual ::com::sun::star::uno::Any
                                getDefaultForReset() const;

    private:
        /** calculates the data type we need to use to exchange values with external bindings
        */
        void    calcValueExchangeType();
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // EFORMS2_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX

