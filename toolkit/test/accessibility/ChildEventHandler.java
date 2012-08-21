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
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;

class ChildEventHandler
    extends EventHandler
{
    public ChildEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
        mxOldChild = UnoRuntime.queryInterface(
            XAccessible.class, aEvent.OldValue);
        mxNewChild = UnoRuntime.queryInterface(
            XAccessible.class, aEvent.NewValue);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        if (mxOldChild != null)
            out.println ("   removing child " + mxOldChild);
        if (mxNewChild != null)
            out.println ("   adding child " + mxNewChild);
    }

    public void Process ()
    {
        // Insertion and removal of children should be mutually exclusive.
        // But this is a test tool and should take everything into account.
        if (mxOldChild != null)
        {
            maTreeModel.removeNode (mxOldChild.getAccessibleContext());
            maTreeModel.updateNode (mxEventSource, AccessibleTreeHandler.class);
        }

        if (mxNewChild != null)
        {
            maTreeModel.addChild (mxEventSource, mxNewChild);
        }
    }


    private XAccessible mxOldChild;
    private XAccessible mxNewChild;
}
