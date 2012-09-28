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
import com.sun.star.accessibility.XAccessibleImage;


class AccessibleImageHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleImage xImage =
            UnoRuntime.queryInterface (
            XAccessibleImage.class, xContext);
        if (xImage != null)
            return new AccessibleImageHandler (xImage);
        else
            return null;
    }

    public AccessibleImageHandler ()
    {
    }

    public AccessibleImageHandler (XAccessibleImage xImage)
    {
        if (xImage != null)
            maChildList.setSize (1);
    }

    protected static XAccessibleImage getImage (AccTreeNode aNode)
    {
        return UnoRuntime.queryInterface (
            XAccessibleImage.class, aNode.getContext());
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        if (aParent instanceof AccTreeNode)
        {
            XAccessibleImage xImage = getImage ((AccTreeNode)aParent);
            if (xImage != null)
                return new StringNode (
                    "Image: " +
                    xImage.getAccessibleImageDescription() + " (" +
                    xImage.getAccessibleImageWidth() + "x" +
                    xImage.getAccessibleImageHeight() + ")",
                    aParent);
        }
        return null;
    }
}
