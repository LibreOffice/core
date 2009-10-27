/*************************************************************************
 *
 *  $RCSfile: DynamicAccessibilityNode.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2003/09/19 09:21:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
