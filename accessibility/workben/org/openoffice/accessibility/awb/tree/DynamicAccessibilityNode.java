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



package org.openoffice.accessibility.awb.tree;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;

/*
 * This class is dynamic in the way that it does not contain any children
 * until the node is going to be expanded. It also releases all children
 * as soon as the node is collapsed again.
 */
class DynamicAccessibilityNode extends AccessibilityNode {

    public DynamicAccessibilityNode(AccessibilityModel treeModel) {
        super(treeModel);
    }

    // Populates the child list. Called by AccessibilityMode.treeWillExpand().
    protected void populate() {
        try {
            XAccessibleContext xAC = getAccessibleContext();
            if (xAC != null) {
                int n = xAC.getAccessibleChildCount();
                for (int i=0; i<n; i++) {
                    XAccessible xAccessible = xAC.getAccessibleChild(i);
                    AccessibilityNode node = treeModel.findNode(xAccessible);
                    if (node == null) {
                        node = treeModel.createNode(xAccessible);
                    }
                    if (node != null) {
                        // NOTE: do not send any tree notifications here !
                        add(node);
                    }
                }
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            // This should never happen since we previously checked the child
            // count.
            // FIXME: error message
        } catch (com.sun.star.uno.RuntimeException e) {
            // FIXME: error message
        }
    }

    // Clears the child list. Called by AccessibilityModel.treeCollapsed().
    protected void clear() {
        removeAllChildren();
    }

    /* This is called whenever the node is painted, no matter if collapsed
     * or expanded. Making this a "life" value seems to be appropriate.
     */
    public boolean isLeaf() {
        try {
            XAccessibleContext xAC = getAccessibleContext();
            if (xAC != null) {
                return xAC.getAccessibleChildCount() == 0;
            }
            return true;
        } catch (com.sun.star.uno.RuntimeException e) {
            return true;
        }
    }

}
