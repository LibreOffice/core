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

import org.openoffice.accessibility.misc.NameProvider;

import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultMutableTreeNode;

import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;

/**
 *
 */
public class AccessibilityTree extends javax.swing.JTree {

    /** Creates a new instance of AccessibilityTree */
    public AccessibilityTree(javax.swing.tree.TreeModel model) {
        super(model);
        // always show handles to indicate expandable / collapsable
        showsRootHandles = true;
    }

    public void setToolkit(XExtendedToolkit xToolkit) {
        AccessibilityModel model = (AccessibilityModel) getModel();
        if (model != null) {
            // hide the root node when connected
            setRootVisible(xToolkit == null);
            // update the root node
            model.setRoot(xToolkit);
            model.reload();
        }
    }

     public String convertValueToText(Object value, boolean selected,
            boolean expanded, boolean leaf, int row, boolean hasFocus) {

        if (value instanceof DefaultMutableTreeNode) {
            DefaultMutableTreeNode node = (DefaultMutableTreeNode) value;

            Object userObject = node.getUserObject();
            if (userObject != null && userObject instanceof XAccessible) {
                XAccessible xAccessible = (XAccessible) userObject;
                try {
                    XAccessibleContext xAC = xAccessible.getAccessibleContext();
                    if (xAC != null) {
                        String name = xAC.getAccessibleName();
                        if (name.length() == 0) {
                            name = new String ("<no name>");
                        }
                        value = name + " / " + NameProvider.getRoleName(xAC.getAccessibleRole());
                    }
                } catch (com.sun.star.uno.RuntimeException e) {
                    value = "???";
                }
            }
        }

        return super.convertValueToText(value, selected, expanded, leaf, row, hasFocus);
     }

}
