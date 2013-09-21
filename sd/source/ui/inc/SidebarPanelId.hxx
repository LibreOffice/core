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
#ifndef SD_SIDEBAR_PANEL_ID_HXX
#define SD_SIDEBAR_PANEL_ID_HXX

namespace rtl
{
    class OUString;
}


namespace sd { namespace sidebar {

/** List of top level panels that can be shown in the task pane.
*/
enum PanelId
{
    PID__START = 0,
    PID_UNKNOWN = PID__START,
    PID_MASTER_PAGES_ALL,
    PID_MASTER_PAGES_RECENT,
    PID_MASTER_PAGES_USED,
    PID_LAYOUT,
    PID_TABLE_DESIGN,
    PID_ANIMATION_SCHEMES,
    PID_CUSTOM_ANIMATION,
    PID_SLIDE_TRANSITION,
    PID__END = PID_SLIDE_TRANSITION
};

} } // namespace sd::sidebar


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
