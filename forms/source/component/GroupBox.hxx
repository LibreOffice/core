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

#include <FormComponent.hxx>


namespace frm
{

class OGroupBoxModel
        :public OControlModel
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OGroupBoxModel );

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.form.OGroupBoxModel"; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() override;
    virtual void SAL_CALL
        write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) override;
    virtual void SAL_CALL
        read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) override;

    // OControlModel's property handling
    virtual void describeAggregateProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
    ) const override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;
};


// OGroupBoxControl (only for compatibility for 5.0)

class OGroupBoxControl : public OControl
{
public:
    explicit OGroupBoxControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.form.OGroupBoxControl"; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
