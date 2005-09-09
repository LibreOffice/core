/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EventNames.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:51:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.ui.event;

/**
 *
 * @author  rpiterman
 */
public interface EventNames {

    //common listener events
    public static final String EVENT_ACTION_PERFORMED = "APR";
    public static final String EVENT_ITEM_CHANGED = "ICH";
    public static final String EVENT_TEXT_CHANGED = "TCH";

    //window events (XWindow)
    public static final String EVENT_WINDOW_RESIZED = "WRE";
    public static final String EVENT_WINDOW_MOVED = "WMO";
    public static final String EVENT_WINDOW_SHOWN = "WSH";
    public static final String EVENT_WINDOW_HIDDEN = "WHI";

    //focus events (XWindow)
    public static final String EVENT_FOCUS_GAINED = "FGA";
    public static final String EVENT_FOCUS_LOST = "FLO";

    //keyboard events
    public static final String EVENT_KEY_PRESSED = "KPR";
    public static final String EVENT_KEY_RELEASED = "KRE";

    //mouse events
    public static final String EVENT_MOUSE_PRESSED = "MPR";
    public static final String EVENT_MOUSE_RELEASED = "MRE";
    public static final String EVENT_MOUSE_ENTERED = "MEN";
    public static final String EVENT_MOUSE_EXITED = "MEX";

    //other events

}
