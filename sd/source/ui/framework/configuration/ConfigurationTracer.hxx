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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONTRACER_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONTRACER_HXX

#include <com/sun/star/drawing/framework/XConfiguration.hpp>

namespace sd { namespace framework {

/** Print debug information about configurations to the standard error
    output channel.
*/
class ConfigurationTracer
{
public:
    static void TraceConfiguration (
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration,
        const char* pMessage);
#ifdef DEBUG
    static void TraceBoundResources (
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        const int nIndentation);
#endif
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
