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
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleTableModelChange;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;

class TableEventHandler
    extends EventHandler
{
    public TableEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        switch (mnEventId)
        {
            case AccessibleEventId.TABLE_MODEL_CHANGED:
                AccessibleTableModelChange aModelChange =
                    (AccessibleTableModelChange)maEvent.NewValue;
                out.println( "Range: StartRow " + aModelChange.FirstRow +
                    " StartColumn " + aModelChange.FirstColumn +
                    " EndRow " + aModelChange.LastRow +
                    " EndColumn " + aModelChange.LastColumn +
                    " Id " + aModelChange.Type);
                break;
            default:
                super.PrintOldAndNew (out);
        }
    }

    public void Process ()
    {
        maTreeModel.updateNode (mxEventSource, AccessibleTableHandler.class);
    }


    private XAccessible mxOldChild;
    private XAccessible mxNewChild;
}
