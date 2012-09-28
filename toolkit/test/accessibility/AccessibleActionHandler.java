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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.lang.IndexOutOfBoundsException;

class AccessibleActionHandler
    extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleAction xEComponent =
            UnoRuntime.queryInterface (
            XAccessibleAction.class, xContext);
        if (xEComponent != null)
            return new AccessibleActionHandler (xEComponent);
        else
            return null;
    }

    public AccessibleActionHandler ()
    {
    }

    public AccessibleActionHandler (XAccessibleAction xAction)
    {
        if (xAction != null)
            maChildList.setSize (1 + xAction.getAccessibleActionCount());
    }

    protected static XAccessibleAction getAction (AccTreeNode aParent)
    {
        return UnoRuntime.queryInterface (
            XAccessibleAction.class, aParent.getContext());
    }

    public AccessibleTreeNode createChild (
        AccessibleTreeNode aParent,
        int nIndex)
    {
        AccessibleTreeNode aChild = null;

        if (aParent instanceof AccTreeNode)
        {
            XAccessibleAction xAction = getAction ((AccTreeNode)aParent);
            if( xAction != null )
            {
                if (nIndex == 0)
                    aChild = new StringNode ("Number of actions: " + xAction.getAccessibleActionCount(),
                        aParent);
                else
                {
                    nIndex -= 1;
                    try
                    {
                        aChild = new AccessibleActionNode (
                            "Action " + nIndex + " : "
                            + xAction.getAccessibleActionDescription (nIndex),
                            aParent,
                            nIndex);
                    }
                    catch( IndexOutOfBoundsException e )
                    {
                        aChild = new StringNode ("ERROR", aParent);
                    }
                }
            }
        }

        return aChild;
    }
}
