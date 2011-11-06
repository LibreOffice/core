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


import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleHyperlink;


class AccessibleHyperlinkHandler extends AccessibleTreeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleHyperlink xLink =
            (XAccessibleHyperlink) UnoRuntime.queryInterface (
                XAccessibleHyperlink.class, xContext);
        if (xLink != null)
            return new AccessibleHyperlinkHandler (xLink);
        else
            return null;
    }

    public AccessibleHyperlinkHandler ()
    {
    }

    public AccessibleHyperlinkHandler (XAccessibleHyperlink xLink)
    {
        if (xLink != null)
            maChildList.setSize (1);
    }

    protected XAccessibleHyperlink getHyperlink(Object aObject)
    {
        XAccessibleHyperlink xHyperlink =
            (XAccessibleHyperlink) UnoRuntime.queryInterface (
                 XAccessibleHyperlink.class, aObject);
        return xHyperlink;
    }

    public AccessibleTreeNode getChild (AccessibleTreeNode aParent, int nIndex)
    {
        return new StringNode ("interface XAccessibleHyperlink is supported", aParent);
    }
}
