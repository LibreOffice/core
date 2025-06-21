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

#include <com/sun/star/lang/XEventListener.hpp>
#include <comphelper/compbase.hxx>
#include <sddllapi.h>

namespace sd::framework
{
struct ConfigurationChangeEvent;

/** A listener for configuration changes is called when it has been
    registered at the configuration controller and a configuration change
    occurs.
*/
class SD_DLLPUBLIC ConfigurationChangeListener
    : public comphelper::WeakComponentImplHelper<::css::lang::XEventListener>
{
public:
    using XEventListener::disposing;
    using WeakComponentImplHelperBase::disposing;

    /** The exact time of when a listener is called (before the change takes
        place, during the change, or when the change has been made) depends
        on the change event.  The order in which listeners are called is the
        order in which they are registered (First registered, first called.)
    */
    virtual void notifyConfigurationChange(const sd::framework::ConfigurationChangeEvent& aEvent)
        = 0;

protected:
    virtual ~ConfigurationChangeListener() override;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
