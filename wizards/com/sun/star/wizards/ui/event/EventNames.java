/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EventNames.java,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.wizards.ui.event;

/**
 *
 * @author  rpiterman
 */
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
