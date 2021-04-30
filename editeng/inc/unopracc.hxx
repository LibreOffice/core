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

#include <editeng/unotext.hxx>

class SvxEditSource;

/** Wraps SvxUnoTextRangeBase and provides us with the text properties

    Inherits from SvxUnoTextRangeBase and provides XPropertySet and
    XMultiPropertySet interfaces. Just set the selection to the
    required text range and return a reference to a XPropertySet.
 */
class SvxAccessibleTextPropertySet final : public SvxUnoTextRangeBase,
                                           public css::lang::XTypeProvider,
                                           public ::cppu::OWeakObject
{
public:
    SvxAccessibleTextPropertySet(const SvxEditSource*, const SvxItemPropertySet*);
    virtual ~SvxAccessibleTextPropertySet() noexcept override;

    // XTextRange
    virtual css::uno::Reference<css::text::XText> SAL_CALL getText() override;

    virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& rType) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString&) override;

    // lang::XTypeProvider
    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
