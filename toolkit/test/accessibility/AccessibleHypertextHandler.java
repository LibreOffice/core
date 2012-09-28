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
import com.sun.star.accessibility.XAccessibleHypertext;


class AccessibleHypertextHandler extends AccessibleTreeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleHypertext xText =
            UnoRuntime.queryInterface (
            XAccessibleHypertext.class, xContext);
        if (xText != null)
            return new AccessibleHypertextHandler (xText);
        else
            return null;
    }

    public AccessibleHypertextHandler ()
    {
    }

    public AccessibleHypertextHandler (XAccessibleHypertext xText)
    {
        if (xText != null)
            maChildList.setSize (1);
    }

    protected static XAccessibleHypertext getHypertext (AccTreeNode aNode)
    {
        XAccessibleHypertext xHypertext =
            UnoRuntime.queryInterface (
             XAccessibleHypertext.class, aNode.getContext());
        return xHypertext;
    }

    public AccessibleTreeNode getChild (AccessibleTreeNode aParent, int nIndex)
    {
        return new StringNode ("interface XAccessibleHypertext is supported", aParent);
    }
}
