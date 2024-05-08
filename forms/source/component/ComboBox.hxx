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

#pragma once

#include <memory>
#include <FormComponent.hxx>
#include "errorbroadcaster.hxx"
#include "entrylisthelper.hxx"
#include "cachedrowset.hxx"

#include <com/sun/star/form/ListSourceType.hpp>

#include <connectivity/formattedcolumnvalue.hxx>


namespace frm
{

class OComboBoxModel final
            :public OBoundControlModel
            ,public OEntryListHelper
            ,public OErrorBroadcaster
{
    CachedRowSet               m_aListRowSet;          // the row set to fill the list
    css::uno::Any              m_aBoundColumn;         // obsolete
    OUString                   m_aListSource;
    OUString                   m_aDefaultText;         // DefaultText
    css::uno::Any              m_aLastKnownValue;

    css::uno::Sequence<OUString>                          m_aDesignModeStringItems;

    css::form::ListSourceType  m_eListSourceType;      // ListSource's type
    bool                       m_bEmptyIsNull;         // Empty string is interpreted as NULL

    ::std::unique_ptr< ::dbtools::FormattedColumnValue > m_pValueFormatter;

    virtual css::uno::Sequence< css::uno::Type>   _getTypes() override;

public:
    OComboBoxModel(
        const css::uno::Reference< css::uno::XComponentContext>& _rxFactory
    );
    OComboBoxModel(
        const OComboBoxModel* _pOriginal,
        const css::uno::Reference< css::uno::XComponentContext>& _rxFactory
    );
    virtual ~OComboBoxModel() override;

    virtual void SAL_CALL disposing() override;

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue ) override;

    // XLoadListener
    virtual void SAL_CALL reloaded( const css::lang::EventObject& aEvent ) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.form.OComboBoxModel"_ustr; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OComboBoxModel, OBoundControlModel)
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) override;

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() override;
    virtual void SAL_CALL
        write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) override;
    virtual void SAL_CALL
        read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;
    virtual void describeAggregateProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
    ) const override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    // prevent method hiding
    using OBoundControlModel::getFastPropertyValue;

private:
    // OBoundControlModel overridables
    virtual css::uno::Any   translateDbColumnToControlValue( ) override;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;

    virtual void            onConnectedDbColumn( const css::uno::Reference< css::uno::XInterface >& _rxForm ) override;
    virtual void            onDisconnectedDbColumn() override;

    virtual css::uno::Any   getDefaultForReset() const override;

    virtual void            resetNoBroadcast() override;

    // OEntryListHelper overridables
    virtual void    stringItemListChanged( ControlModelLock& _rInstanceLock ) override;
    virtual void    refreshInternalEntryList() override;

    void loadData( bool _bForce );

    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;
};

class OComboBoxControl : public OBoundControl
{
public:
    explicit OComboBoxControl(const css::uno::Reference< css::uno::XComponentContext>& _rxContext);

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.form.OComboBoxControl"_ustr; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
