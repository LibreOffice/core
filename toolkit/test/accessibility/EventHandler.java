/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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

        mxEventSource = (XAccessibleContext)UnoRuntime.queryInterface(
            XAccessibleContext.class, aEvent.Source);
        if (mxEventSource == null)
        {
            XAccessible xAccessible = (XAccessible)UnoRuntime.queryInterface(
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
