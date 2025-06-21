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

#include <framework/Configuration.hxx>
#include <rtl/ref.hxx>

namespace sd::framework
{
/** A single explicit request for a configuration change.

    <p>The requested change is committed to a configuration only when the
    execute() method is called.  Configuration change
    requests are executed asynchronously.  This is done to avoid reentrance
    problems with objects that are registered as
    XConfigurationChangeListener and at the same time make
    configuration change requests.  When the requests were executed
    synchronously then the listeners would be notified of the changes while
    their request call has not yet returned.</p>

    <p>This interface is typically used internally by the
    XConfigurationController</p> @see XConfigurationController
*/
class ConfigurationChangeRequest : public comphelper::WeakComponentImplHelper<>
{
public:
    /** Commit the configuration change request represented by the called
        object to the given configuration.
        @param xConfiguration
            This is the configuration to commit the requested change to.
    */
    virtual void execute(const rtl::Reference<sd::framework::Configuration>& xConfiguration) = 0;

protected:
    virtual ~ConfigurationChangeRequest() override;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
