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
package com.sun.star.wizards.ui.event;

public interface EventNames
{

    //common listener events
    public static final String EVENT_ACTION_PERFORMED = "APR";
    public static final String EVENT_ITEM_CHANGED = "ICH";
    public static final String EVENT_TEXT_CHANGED = "TCH";    //window events (XWindow)
    public static final String EVENT_WINDOW_RESIZED = "WRE";
    public static final String EVENT_WINDOW_MOVED = "WMO";
    public static final String EVENT_WINDOW_SHOWN = "WSH";
    public static final String EVENT_WINDOW_HIDDEN = "WHI";    //focus events (XWindow)
    public static final String EVENT_FOCUS_GAINED = "FGA";
    public static final String EVENT_FOCUS_LOST = "FLO";    //keyboard events
    public static final String EVENT_KEY_PRESSED = "KPR";
    public static final String EVENT_KEY_RELEASED = "KRE";    //mouse events
    public static final String EVENT_MOUSE_PRESSED = "MPR";
    public static final String EVENT_MOUSE_RELEASED = "MRE";
    public static final String EVENT_MOUSE_ENTERED = "MEN";
    public static final String EVENT_MOUSE_EXITED = "MEX";    //other events
}
