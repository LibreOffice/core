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

#include "EditBase.hxx"

#include <connectivity/formattedcolumnvalue.hxx>

#include <memory>


namespace frm
{

class OPatternModel
                :public OEditBaseModel
{
private:
    css::uno::Any               m_aLastKnownValue;
    ::std::unique_ptr< ::dbtools::FormattedColumnValue >
                                m_pFormattedValue;

public:
    DECLARE_DEFAULT_LEAF_XTOR( OPatternModel );

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.form.OPatternModel"; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // css::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

protected:
    // OBoundControlModel overridables
    virtual css::uno::Any   translateDbColumnToControlValue( ) override;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;
    virtual void            onConnectedDbColumn( const css::uno::Reference< css::uno::XInterface >& _rxForm ) override;
    virtual void            onDisconnectedDbColumn() override;

    virtual css::uno::Any   getDefaultForReset() const override;
    virtual void            resetNoBroadcast() override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;
};

class OPatternControl: public OBoundControl
{
public:
    explicit OPatternControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.form.OPatternControl"; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
