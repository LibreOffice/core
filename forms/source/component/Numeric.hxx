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


namespace frm
{

class ONumericModel
                :public OEditBaseModel
{
private:
    css::uno::Any          m_aSaveValue;

public:
    DECLARE_DEFAULT_LEAF_XTOR( ONumericModel );

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.form.ONumericModel"; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // css::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

protected:
    // OBoundControlModel overridables
    virtual css::uno::Any
                            translateDbColumnToControlValue( ) override;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;

    virtual css::uno::Any
                            getDefaultForReset() const override;
    virtual void            resetNoBroadcast() override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;
};

class ONumericControl: public OBoundControl
{
public:
    explicit ONumericControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.form.ONumericControl"; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
