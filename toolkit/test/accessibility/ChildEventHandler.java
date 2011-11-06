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
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;

class ChildEventHandler
    extends EventHandler
{
    public ChildEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
        mxOldChild = (XAccessible)UnoRuntime.queryInterface(
            XAccessible.class, aEvent.OldValue);
        mxNewChild = (XAccessible)UnoRuntime.queryInterface(
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
