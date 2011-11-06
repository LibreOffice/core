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
import com.sun.star.accessibility.XAccessibleImage;


class AccessibleImageHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleImage xImage =
            (XAccessibleImage) UnoRuntime.queryInterface (
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
        return (XAccessibleImage) UnoRuntime.queryInterface (
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
