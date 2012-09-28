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
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleTableModelChange;
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
