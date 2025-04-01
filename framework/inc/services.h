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

#include <rtl/ustring.hxx>

namespace framework{

//  used servicenames by framework

inline constexpr OUString SERVICENAME_FILTERFACTORY = u"com.sun.star.document.FilterFactory"_ustr;
inline constexpr OUString SERVICENAME_CFGUPDATEACCESS = u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr; // provides read/write access to the configuration
inline constexpr OUString SERVICENAME_CFGREADACCESS = u"com.sun.star.configuration.ConfigurationAccess"_ustr;  // provides readonly access to the configuration
inline constexpr OUString SERVICENAME_JOB = u"com.sun.star.task.Job"_ustr;
inline constexpr OUString SERVICENAME_PROTOCOLHANDLER = u"com.sun.star.frame.ProtocolHandler"_ustr;
inline constexpr OUString SERVICENAME_POPUPMENUCONTROLLER = u"com.sun.star.frame.PopupMenuController"_ustr;

//  used implementationnames by framework

                        "com.sun.star.comp.desktop.QuickstartWrapper"
inline constexpr OUString IMPLEMENTATIONNAME_FWK_TASKCREATOR = u"com.sun.star.comp.framework.TaskCreator"_ustr;

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
