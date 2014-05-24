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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_TIME_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_TIME_HXX

#include "EditBase.hxx"
#include "limitedformats.hxx"


namespace frm
{

class OTimeModel
                :public OEditBaseModel
                ,public OLimitedFormats
{
private:
    ::com::sun::star::uno::Any      m_aSaveValue;
    bool                        m_bDateTimeField;

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    DECLARE_DEFAULT_LEAF_XTOR( OTimeModel );

    // stario::XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OTimeModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
                            getSupportedBindingTypes() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const SAL_OVERRIDE;
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
    void                    impl_translateControlValueToUNOTime(
                                ::com::sun::star::uno::Any& _rUNOValue ) const;
};

class OTimeControl: public OBoundControl
{
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    OTimeControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);
    DECLARE_UNO3_AGG_DEFAULTS(OTimeControl, OBoundControl)

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OTimeControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_TIME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
