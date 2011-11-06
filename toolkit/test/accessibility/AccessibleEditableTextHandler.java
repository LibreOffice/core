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
import com.sun.star.accessibility.XAccessibleEditableText;


class AccessibleEditableTextHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleEditableText xText =
            (XAccessibleEditableText) UnoRuntime.queryInterface (
                XAccessibleEditableText.class, xContext);
        if (xText != null)
            return new AccessibleEditableTextHandler (xText);
        else
            return null;
    }

    public AccessibleEditableTextHandler ()
    {
    }

    public AccessibleEditableTextHandler (XAccessibleEditableText xText)
    {
        if (xText != null)
            maChildList.setSize (1);
    }

    protected static XAccessibleEditableText getEText (AccTreeNode aNode)
    {
        return (XAccessibleEditableText) UnoRuntime.queryInterface (
            XAccessibleEditableText.class, aNode.getContext());
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        return new StringNode ("XAccessibleEditableText is supported", aParent);
    }
}
