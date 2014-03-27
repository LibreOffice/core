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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELD_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELD_HXX

#include "EditBase.hxx"
#include <tools/link.hxx>
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/implbase1.hxx>
#include "errorbroadcaster.hxx"

namespace frm
{
class OFormattedModel
                    :public OEditBaseModel
                    ,public OErrorBroadcaster
    {
        // the original, in case I faked the format properties of my aggregated model,
        // i.e. I just passed on the attributes of the field to which I am bound
        // (only valid if loaded)

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>   m_xOriginalFormatter;
        ::com::sun::star::util::Date        m_aNullDate;
        ::com::sun::star::uno::Any          m_aSaveValue;

        sal_Int32                           m_nFieldType;
        sal_Int16                           m_nKeyType;
        sal_Bool                            m_bOriginalNumeric      : 1,
                                            m_bNumeric              : 1;    // analogous for the TreatAsNumeric-property

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcDefaultFormatsSupplier() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormFormatsSupplier() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormatsSupplier() const;

        DECLARE_DEFAULT_LEAF_XTOR( OFormattedModel );

        friend class OFormattedFieldWrapper;

    protected:
        // XInterface
        DECLARE_UNO3_AGG_DEFAULTS( OFormattedModel, OEditBaseModel );

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

        // XAggregation
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // OComponentHelper
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // XServiceInfo
        IMPLEMENTATION_NAME(OFormattedModel);
        virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

        // XPersistObject
        virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertySet
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                              sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                            throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

        // XLoadListener
        virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& rEvent) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertyState
        void setPropertyToDefaultByHandle(sal_Int32 nHandle) SAL_OVERRIDE;
        ::com::sun::star::uno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const SAL_OVERRIDE;

        void SAL_CALL setPropertyToDefault(const OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const SAL_OVERRIDE;
        virtual void describeAggregateProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const SAL_OVERRIDE;

        // XPropertyChangeListener
        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

        // prevent method hiding
        using OEditBaseModel::disposing;
        using OEditBaseModel::getFastPropertyValue;

    protected:
        virtual sal_uInt16 getPersistenceFlags() const SAL_OVERRIDE;
        // as we have an own version handling for persistence

        // OBoundControlModel overridables
        virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( ) SAL_OVERRIDE;
        virtual sal_Bool    commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
                            getSupportedBindingTypes() SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any
                            translateExternalValueToControlValue( const ::com::sun::star::uno::Any& _rExternalValue ) const SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const SAL_OVERRIDE;
        virtual void onConnectedExternalValue( ) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const SAL_OVERRIDE;
        virtual void        resetNoBroadcast() SAL_OVERRIDE;

        virtual void        onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm ) SAL_OVERRIDE;
        virtual void        onDisconnectedDbColumn() SAL_OVERRIDE;

    private:
        DECLARE_XCLONEABLE();

        void implConstruct();

        void    updateFormatterNullDate();
    };

    // OFormattedControl
    typedef ::cppu::ImplHelper1< ::com::sun::star::awt::XKeyListener> OFormattedControl_BASE;
    class OFormattedControl :    public OBoundControl
                                ,public OFormattedControl_BASE
    {
        sal_uInt32              m_nKeyEvent;

    public:
        OFormattedControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext);
        virtual ~OFormattedControl();

        DECLARE_UNO3_AGG_DEFAULTS(OFormattedControl, OBoundControl);
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

        // ::com::sun::star::lang::XServiceInfo
        IMPLEMENTATION_NAME(OFormattedControl);
        virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::awt::XKeyListener
        virtual void SAL_CALL keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL keyReleased(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::awt::XControl
        virtual void SAL_CALL setDesignMode(sal_Bool bOn) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // disambiguation
        using OBoundControl::disposing;

    private:
        DECL_LINK( OnKeyPressed, void* );
    };
}
#endif // INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
