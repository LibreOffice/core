/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/compbase.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace ucb_cmdenv {

using UcbCommandEnvironment_Base = comphelper::WeakComponentImplHelper< css::lang::XInitialization,
                                      css::lang::XServiceInfo,
                                      css::ucb::XCommandEnvironment >;

class UcbCommandEnvironment : public UcbCommandEnvironment_Base
{
    cpo::uno::Reference< css::task::XInteractionHandler > m_xIH;
    cpo::uno::Reference< css::ucb::XProgressHandler >     m_xPH;

public:
    explicit UcbCommandEnvironment();
    virtual ~UcbCommandEnvironment() override;

    // XInitialization
    virtual void
    initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;

    virtual bool
    supportsService( const OUString& ServiceName ) override;

    virtual cpo::uno::Sequence< OUString >
    getSupportedServiceNames() override;

    // XCommandEnvironment
    virtual cpo::uno::Reference< css::task::XInteractionHandler >
    getInteractionHandler() override;
    virtual cpo::uno::Reference< css::ucb::XProgressHandler >
    getProgressHandler() override;
};

} // namespace ucb_cmdenv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
