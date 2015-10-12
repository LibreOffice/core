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

#ifndef INCLUDED_BASCTL_SOURCE_INC_BASIDECTRLR_HXX
#define INCLUDED_BASCTL_SOURCE_INC_BASIDECTRLR_HXX

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <sfx2/sfxbasecontroller.hxx>

namespace basctl
{

class Shell;

class Controller :
    public comphelper::OMutexAndBroadcastHelper,
    public comphelper::OPropertyContainer,
    public comphelper::OPropertyArrayUsageHelper<Controller>,
    public SfxBaseController
{
private:
    // properties
    sal_Int32   m_nIconId;

public:
    Controller (Shell* pViewShell);
    virtual ~Controller();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XTypeProvider ( ::SfxBaseController )
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw( css::uno::RuntimeException, std::exception ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

protected:
    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_BASIDECTRLR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
