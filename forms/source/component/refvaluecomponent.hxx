/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refvaluecomponent.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:51:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EFORMS2_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX
#define EFORMS2_FORMS_SOURCE_COMPONENT_REFVALUECOMPONENT_HXX

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace frm
{
//........................................................................

    enum state { STATE_NOCHECK, STATE_CHECK, STATE_DONTKNOW };

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
        sal_Int16           m_nDefaultChecked;          // the default check state
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

        sal_Int16              getDefaultChecked() const { return m_nDefaultChecked; }
        void                   setDefaultChecked( sal_Int16 _nChecked ) { m_nDefaultChecked = _nChecked; }

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
        virtual void fillProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;

        // OBoundControlModel overridables
        virtual void            onConnectedExternalValue( );
        virtual sal_Bool        approveValueBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XValueBinding >& _rxBinding );

        virtual ::com::sun::star::uno::Any
                                translateExternalValueToControlValue( ) const;
        virtual ::com::sun::star::uno::Any
                                translateControlValueToExternalValue( ) const;

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

