/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
