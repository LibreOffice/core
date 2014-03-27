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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_DATE_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_DATE_HXX

#include "EditBase.hxx"
#include "limitedformats.hxx"


namespace frm
{



//= ODateModel

class ODateModel
                :public OEditBaseModel
                ,public OLimitedFormats
{
    ::com::sun::star::uno::Any      m_aSaveValue;
    sal_Bool                        m_bDateTimeField;

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    DECLARE_DEFAULT_LEAF_XTOR( ODateModel );

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // ::com::sun::star::beans::XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    IMPLEMENTATION_NAME(ODateModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // XPropertySet
    using OEditBaseModel::getFastPropertyValue;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
                            getSupportedBindingTypes() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any
                            translateExternalValueToControlValue( const ::com::sun::star::uno::Any& _rExternalValue ) const SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any
                            translateControlValueToValidatableValue( ) const SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const SAL_OVERRIDE;

    virtual void            resetNoBroadcast() SAL_OVERRIDE;

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm ) SAL_OVERRIDE;

protected:
    DECLARE_XCLONEABLE();

private:
    /** translates the control value (the VCL-internal integer representation of a date) into
        a UNO-Date.
    */
    void                    impl_translateControlValueToUNODate(
                                ::com::sun::star::uno::Any& _rUNOValue ) const;
};


//= ODateControl

class ODateControl: public OBoundControl
{
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    ODateControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);
    DECLARE_UNO3_AGG_DEFAULTS(ODateControl, OBoundControl);

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(ODateControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_DATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
