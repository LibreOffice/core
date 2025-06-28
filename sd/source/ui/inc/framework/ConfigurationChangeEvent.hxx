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

#include <com/sun/star/lang/EventObject.hpp>
#include <ResourceId.hxx>
#include <rtl/ref.hxx>

namespace sd::framework
{
class Configuration;
class AbstractResource;

enum class ConfigurationChangeEventType
{
    ConfigurationUpdateStart,
    ConfigurationUpdateEnd,
    ResourceActivation,
    ResourceActivationRequest,
    ResourceDeactivation,
    ResourceDeactivationRequest
};

/** Objects of this class are used for notifying changes of the
    configuration.

    <p>They are broadcasted by the configuration controller
    which maintains the configuration.  The set of types of configuration
    changes is not fixed and is not maintained or documented in one
    place.</p>

    <p>The set of used members and the exact meaning of their values is not the
    same for all types.  Therefore, the descriptions of the members are just
    general guidelines.   See XConfigurationController for a
    list of event types used by the basic drawing framework.</p>
*/
struct ConfigurationChangeEvent final : public ::css::lang::EventObject
{
    /** This is the only member that is always set.  The values of the other members
        depend on the configuration change type and may or may not be set.
    */
    ConfigurationChangeEventType Type;
    /** The current configuration, depending on the event type, either
        before or after the change.  May be an empty reference.
    */
    rtl::Reference<::sd::framework::Configuration> Configuration;
    /** The resource id that is part of the configuration change.
    */
    rtl::Reference<sd::framework::ResourceId> ResourceId;
    /** The resource object that corresponds to the ResourceId.  May
        be an empty reference.
    */
    rtl::Reference<sd::framework::AbstractResource> ResourceObject;
    /** Each listener is called with exactly the #UserData
        that was given when the listener was registered.
    */
    ::css::uno::Any UserData;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
