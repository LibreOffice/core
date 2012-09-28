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
import com.sun.star.accessibility.XAccessibleComponent;


class AccessibleComponentHandler
    extends NodeHandler
{

    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleComponent xComponent =
            UnoRuntime.queryInterface (
            XAccessibleComponent.class, xContext);
        if (xComponent != null)
            return new AccessibleComponentHandler (xComponent);
        else
            return null;

    }

    public AccessibleComponentHandler ()
    {
    }

    public AccessibleComponentHandler (XAccessibleComponent xComponent)
    {
        if (xComponent != null)
            maChildList.setSize (6);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        if (aParent instanceof AccTreeNode)
        {
            XAccessibleComponent xComponent =
                ((AccTreeNode)aParent).getComponent();

            if (xComponent != null)
            {
                int nColor;
                switch (nIndex)
                {
                    case 0:
                        com.sun.star.awt.Point aLocation = xComponent.getLocation();
                        aChild = new StringNode (
                            "Location: " + aLocation.X + ", " + aLocation.Y,
                            aParent);
                        break;
                    case 1:
                        com.sun.star.awt.Point aScreenLocation = xComponent.getLocationOnScreen();
                        aChild = new StringNode (
                            "Location on Screen: " + aScreenLocation.X + ", " + aScreenLocation.Y,
                            aParent);
                        break;
                    case 2:
                        com.sun.star.awt.Size aSize = xComponent.getSize();
                        aChild = new StringNode (
                            "Size: "+ aSize.Width + ", " + aSize.Height,
                            aParent);
                        break;
                    case 3:
                        com.sun.star.awt.Rectangle aBBox = xComponent.getBounds();
                        aChild = new StringNode (
                            "Bounding Box: "+ aBBox.X + ", " + aBBox.Y + ","
                            + aBBox.Width + ", " + aBBox.Height,
                            aParent);
                        break;
                    case 4:
                        nColor = xComponent.getForeground();
                        aChild = new StringNode ("Foreground color: R"
                            +       (nColor>>16&0xff)
                            + "G" + (nColor>>8&0xff)
                            + "B" + (nColor>>0&0xff)
                            + "A" + (nColor>>24&0xff),
                            aParent);
                        break;
                    case 5:
                        nColor = xComponent.getBackground();
                        aChild = new StringNode ("Background color: R"
                            +       (nColor>>16&0xff)
                            + "G" + (nColor>>8&0xff)
                            + "B" + (nColor>>0&0xff)
                            + "A" + (nColor>>24&0xff),
                            aParent);
                        break;
                }
            }
        }
        return aChild;
    }

    public void update (AccessibleTreeNode aNode)
    {
        maChildList.clear();
        if (aNode instanceof AccTreeNode)
            if (((AccTreeNode)aNode).getComponent() != null)
                maChildList.setSize (4);
    }
}
