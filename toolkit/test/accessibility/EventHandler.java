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

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;

import tools.NameProvider;

/** Base class for handling of accessibility events.
*/
class EventHandler
{
    public EventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        maEvent = aEvent;
        maTreeModel = aTreeModel;

        mnEventId = aEvent.EventId;

        mxEventSource = UnoRuntime.queryInterface(
            XAccessibleContext.class, aEvent.Source);
        if (mxEventSource == null)
        {
            XAccessible xAccessible = UnoRuntime.queryInterface(
                XAccessible.class, aEvent.Source);
            if (xAccessible != null)
                mxEventSource = xAccessible.getAccessibleContext();
        }
    }

    public void Print (PrintStream out)
    {
        out.println ("Event id is " + mnEventId
            + " (" + NameProvider.getEventName(mnEventId)+")"
            + " for " + mxEventSource.getAccessibleName() + " / "
            + NameProvider.getRoleName (mxEventSource.getAccessibleRole()));
        PrintOldAndNew (out);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        out.println ("    old value is " + maEvent.OldValue);
        out.println ("    new value is " + maEvent.NewValue);
    }

    public void Process ()
    {
        System.out.println ("processing of event " + maEvent + " not implemented");
    }

    protected AccessibleEventObject maEvent;
    protected AccessibilityTreeModel maTreeModel;

    protected int mnEventId;
    protected XAccessibleContext mxEventSource;
}
