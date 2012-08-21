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
import com.sun.star.accessibility.XAccessibleStateSet;
import tools.NameProvider;

class AccessibleContextHandler
    extends NodeHandler
{
    protected int nChildrenCount;

    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        if (xContext != null)
            return new AccessibleContextHandler (xContext);
        else
            return null;
    }

    public AccessibleContextHandler ()
    {
        super ();
    }

    public AccessibleContextHandler (XAccessibleContext xContext)
    {
        super();
        if (xContext != null)
            maChildList.setSize (4);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        XAccessibleContext xContext = null;
        if (aParent instanceof AccTreeNode)
            xContext = ((AccTreeNode)aParent).getContext();

        String sChild = new String();
        if (xContext != null)
        {
            switch( nIndex )
            {
                case 0:
                    sChild = "Description: " +
                        xContext.getAccessibleDescription();
                    break;
                case 1:
                    int nRole = xContext.getAccessibleRole();
                    sChild = "Role: " + nRole + " (" + NameProvider.getRoleName(nRole) + ")";
                    break;
                case 2:
                    XAccessible xParent = xContext.getAccessibleParent();
                    sChild = "Has parent: " + (xParent!=null ? "yes" : "no");
                    /*                    if (xParent != ((AccTreeNode)aParent).getAccessible())
                    {
                        sChild += " but that is inconsistent"
                            + "#" + xParent + " # " + ((AccTreeNode)aParent).getAccessible();
                    }
                    */
                    break;
                case 3:
                    sChild = "";
                    XAccessibleStateSet xStateSet =
                        xContext.getAccessibleStateSet();
                    if (xStateSet != null)
                    {
                        for (short i=0; i<=30; i++)
                        {
                            if (xStateSet.contains (i))
                            {
                                if (sChild.compareTo ("") != 0)
                                    sChild += ", ";
                                sChild += NameProvider.getStateName(i);
                            }
                        }
                    }
                    else
                        sChild += "no state set";
                    sChild = "State set: " + sChild;

                    /*                case 3:
                    sChild = "Child count: " + xContext.getAccessibleChildCount();
                    break;*/
            }
        }
        return new StringNode (sChild, aParent);
    }
}
