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

#include <MutexOwner.hxx>
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/compbase.hxx>

namespace com::sun::star::drawing::framework { class XConfiguration; }
namespace com::sun::star::drawing::framework { class XResourceId; }

namespace sd::framework {

typedef ::cppu::WeakComponentImplHelper <
      css::drawing::framework::XConfigurationChangeRequest,
      css::container::XNamed
    > GenericConfigurationChangeRequestInterfaceBase;

/** This implementation of the XConfigurationChangeRequest interface
    represents a single explicit request for a configuration change.  On its
    execution it may result in other, implicit, configuration changes.  For
    example this is the case when the deactivation of a unique resource is
    requested: the resources linked to it have to be deactivated as well.
*/
class GenericConfigurationChangeRequest
    : private MutexOwner,
      public GenericConfigurationChangeRequestInterfaceBase
{
public:
    /** This enum specified whether the activation or deactivation of a
        resource is requested.
    */
    enum Mode { Activation, Deactivation };

    /** Create a new object that represents the request for activation or
        deactivation of the specified resource.
        @param rxsResourceId
            Id of the resource that is to be activated or deactivated.
        @param eMode
            The mode specifies whether to activate or to deactivate the
            resource.
        @throws css::css::lang::IllegalArgumentException
    */
    GenericConfigurationChangeRequest (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId,
        const Mode eMode);

    virtual ~GenericConfigurationChangeRequest() noexcept override;

    // XConfigurationChangeOperation

    /** The requested configuration change is executed on the given
        configuration.  Additionally to the explicitly requested change
        other changes have to be made as well.  See class description for an
        example.
        @param rxConfiguration
            The configuration to which the requested change is made.
    */
    virtual void SAL_CALL execute (
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration) override;

    // XNamed

    /** Return a human readable string representation.  This is used for
        debugging purposes.
    */
    virtual OUString SAL_CALL getName() override;

    /** This call is ignored because the XNamed interface is (mis)used to
        give access to a human readable name for debugging purposes.
    */
    virtual void SAL_CALL setName (const OUString& rName) override;

private:
    const css::uno::Reference<css::drawing::framework::XResourceId> mxResourceId;
    const Mode meMode;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
