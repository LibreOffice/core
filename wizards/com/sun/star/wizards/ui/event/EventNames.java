/*
 * EventNames.java
 *
 * Created on 1. September 2003, 16:06
 */
package com.sun.star.wizards.ui.event;

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
