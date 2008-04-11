/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormattedField.hxx,v $
 * $Revision: 1.18 $
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

#ifndef _FORMS_FORMATTEDFIELD_HXX_
#define _FORMS_FORMATTEDFIELD_HXX_

#include "EditBase.hxx"
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/implbase1.hxx>
#include "errorbroadcaster.hxx"

//.........................................................................
namespace frm
{

    //==================================================================
    //= OFormattedModel
    //==================================================================

    class OFormattedModel
                    :public OEditBaseModel
                    ,public OErrorBroadcaster
    {
        // das Original, falls ich die Format-Properties meines aggregierten Models gefaket, d.h. von dem Feld, an das
        // ich gebunden bin, weitergereicht habe (nur gueltig wenn loaded)
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>   m_xOriginalFormatter;
        ::com::sun::star::util::Date        m_aNullDate;
        ::com::sun::star::uno::Any          m_aSaveValue;

        sal_Int32                           m_nFieldType;
        sal_Int16                           m_nKeyType;
        sal_Bool                            m_bOriginalNumeric      : 1,
                                            m_bNumeric              : 1;    // analog fuer TreatAsNumeric-Property

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcDefaultFormatsSupplier() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormFormatsSupplier() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormatsSupplier() const;
        sal_Int32 calcFormatKey() const;

        DECLARE_DEFAULT_LEAF_XTOR( OFormattedModel );

        friend InterfaceRef SAL_CALL OFormattedModel_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
        friend class OFormattedFieldWrapper;

    protected:
    // XInterface
        DECLARE_UNO3_AGG_DEFAULTS( OFormattedModel, OEditBaseModel );

    // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

    // XAggregation
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
        virtual void SAL_CALL disposing();

    // XServiceInfo
        IMPLEMENTATION_NAME(OFormattedModel);
        virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // XPersistObject
        virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);

    // XPropertySet
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                              sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                            throw(::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    // XLoadListener
        virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& rEvent) throw ( ::com::sun::star::uno::RuntimeException);

    // XPropertyState
        void setPropertyToDefaultByHandle(sal_Int32 nHandle);
        ::com::sun::star::uno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const;

        void SAL_CALL setPropertyToDefault(const ::rtl::OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

        // OControlModel's property handling
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const;
        virtual void describeAggregateProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;

    // XPropertyChangeListener
        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    // prevent method hiding
        using OEditBaseModel::disposing;
        using OEditBaseModel::getFastPropertyValue;

    protected:
        virtual sal_uInt16 getPersistenceFlags() const;
            // as we have an own version handling for persistence

        // OBoundControlModel overridables
        virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
        virtual sal_Bool    commitControlValueToDbColumn( bool _bPostReset );

        virtual ::com::sun::star::uno::Any
                            translateExternalValueToControlValue( ) const;
        virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const;
        virtual void onConnectedExternalValue( );

        virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;

        virtual void        onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm );
        virtual void        onDisconnectedDbColumn();

        virtual sal_Bool    approveValueBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XValueBinding >& _rxBinding );

    protected:
        /** retrieves the type which should be used to communicate with the current
            external binding

            The type depends on the current number format, and the types which are supported
            by the current external binding. As a least fallback, |double|'s type is returned.
            (In approveValueBinding, we ensure that only bindings supporting |double|'s are
            accepted.)

            @precond hasExternalValueBinding returns <TRUE/>
        */
        ::com::sun::star::uno::Type
                            getExternalValueType() const;

    private:
        DECLARE_XCLONEABLE();

        void implConstruct();

        void    updateFormatterNullDate();
    };

    //==================================================================
    //= OFormattedControl
    //==================================================================
    typedef ::cppu::ImplHelper1< ::com::sun::star::awt::XKeyListener> OFormattedControl_BASE;
    class OFormattedControl :    public OBoundControl
                                ,public OFormattedControl_BASE
    {
        sal_uInt32              m_nKeyEvent;

    public:
        OFormattedControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
        virtual ~OFormattedControl();

        DECLARE_UNO3_AGG_DEFAULTS(OFormattedControl, OBoundControl);
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

    // ::com::sun::star::lang::XServiceInfo
        IMPLEMENTATION_NAME(OFormattedControl);
        virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XKeyListener
        virtual void SAL_CALL keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL keyReleased(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
        virtual void SAL_CALL setDesignMode(sal_Bool bOn) throw ( ::com::sun::star::uno::RuntimeException);

    // disambiguation
        using OBoundControl::disposing;

    private:
        DECL_LINK( OnKeyPressed, void* );
    };

//.........................................................................
}
//.........................................................................

#endif // _FORMS_FORMATTEDFIELD_HXX_

