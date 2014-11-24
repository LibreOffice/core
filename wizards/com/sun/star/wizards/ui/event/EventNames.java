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

public enum EventNames
{
    //common listener events
    ACTION_PERFORMED,
    ITEM_CHANGED,
    TEXT_CHANGED,    //window events (XWindow)
    WINDOW_RESIZED,
    WINDOW_MOVED,
    WINDOW_SHOWN,
    WINDOW_HIDDEN,    //focus events (XWindow)
    FOCUS_GAINED,
    FOCUS_LOST,    //keyboard events
    KEY_PRESSED,
    KEY_RELEASED,    //mouse events
    MOUSE_PRESSED,
    MOUSE_RELEASED,
    MOUSE_ENTERED,
    MOUSE_EXITED    //other events
}
