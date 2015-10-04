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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELDWRAPPER_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELDWRAPPER_HXX

#include "FormComponent.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>

namespace frm
{

class OEditModel;

//= OFormattedFieldWrapper

typedef ::cppu::WeakAggImplHelper <   css::io::XPersistObject
                                  ,   css::lang::XServiceInfo
                                  ,   css::util::XCloneable
                                  >   OFormattedFieldWrapper_Base;

class OFormattedFieldWrapper : public OFormattedFieldWrapper_Base
{
    css::uno::Reference< css::uno::XComponentContext> m_xContext;

protected:
    css::uno::Reference< css::uno::XAggregation>      m_xAggregate;

    rtl::Reference< OEditModel > m_pEditPart;
    // if we act as formatted this is used to write the EditModel part
    css::uno::Reference< css::io::XPersistObject>     m_xFormattedPart;

private:
    OFormattedFieldWrapper(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);

protected:
    virtual ~OFormattedFieldWrapper();

public:
    // if we act as formatted, this is the PersistObject interface of our aggregate, used
    // to read and write the FormattedModel part
    // if bActAsFormatted is false, the state is undetermined until somebody calls
    // ::read or does anything which requires a living aggregate
    static css::uno::Reference<css::uno::XInterface> createFormattedFieldWrapper(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory, bool bActAsFormatted);

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OFormattedFieldWrapper, OWeakAggObject)
    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

protected:
    /// ensure we're in a defined state, which means a FormattedModel _OR_ an EditModel
    void ensureAggregate();
};

}

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_FORMATTEDFIELDWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
