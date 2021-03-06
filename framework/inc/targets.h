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

namespace framework{

//  Values for special frame search ... sTargetFrameName of findFrame() or queryDispatch() or loadComponentFromURL()

#define SPECIALTARGET_SELF            u"_self"                             // The frame himself is searched.
#define SPECIALTARGET_PARENT          u"_parent"                           // The direct parent frame is searched.
#define SPECIALTARGET_TOP             u"_top"                              // Search at our parents for the first task (if any exist) or a frame without a parent.
#define SPECIALTARGET_BLANK           u"_blank"                            // Create a new task.
#define SPECIALTARGET_DEFAULT         u"_default"                          // Create a new task or recycle an existing one
#define SPECIALTARGET_BEAMER          u"_beamer"                           // special frame in hierarchy
#define SPECIALTARGET_HELPTASK        u"OFFICE_HELP_TASK"                  // special name for our help task

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
