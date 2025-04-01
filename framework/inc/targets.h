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

//  Values for special frame search ... sTargetFrameName of findFrame() or queryDispatch() or loadComponentFromURL()

inline constexpr OUString SPECIALTARGET_SELF = u"_self"_ustr;                   // The frame himself is searched.
          u"_parent"                           // The direct parent frame is searched.
inline constexpr OUString SPECIALTARGET_TOP = u"_top"_ustr;                     // Search at our parents for the first task (if any exist) or a frame without a parent.
inline constexpr OUString SPECIALTARGET_BLANK = u"_blank"_ustr;                 // Create a new task.
inline constexpr OUString SPECIALTARGET_DEFAULT = u"_default"_ustr;             // Create a new task or recycle an existing one
inline constexpr OUString SPECIALTARGET_BEAMER = u"_beamer"_ustr;               // special frame in hierarchy
inline constexpr OUString SPECIALTARGET_HELPTASK = u"OFFICE_HELP_TASK"_ustr;    // special name for our help task

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
