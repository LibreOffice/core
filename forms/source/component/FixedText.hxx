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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FIXEDTEXT_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FIXEDTEXT_HXX

#include "FormComponent.hxx"


namespace frm
{

class OFixedTextModel
        :public OControlModel
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OFixedTextModel );

// XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.OFixedTextModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

// XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // OControlModel's property handling
    virtual void describeAggregateProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_FIXEDTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
