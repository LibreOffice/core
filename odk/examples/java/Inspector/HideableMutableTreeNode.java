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

import javax.swing.tree.*;

/**
 * <code>HideableMutableTreeNode</code> is a <code>DefaultMutableTreeNode</code>
 * implementation that works with <code>HideableTreeModel</code>.
 */
public class HideableMutableTreeNode extends DefaultMutableTreeNode {
    /**
     * The node is visible flag.
     */
    public boolean bIsvisible = true;
        private static final String SDUMMY = "Dummy";


    /**
     * Creates a tree node that has no parent and no children, but which
     * allows children.
     */
    public HideableMutableTreeNode() {
            super();
    }

    /**
     * Creates a tree node with no parent, no children, but which allows
     * children, and initializes it with the specified user object.
     *
     * @param  userObject - an Object provided by the user that
     *                      constitutes the node's data
     */
    public HideableMutableTreeNode(Object _userObject) {
            super(_userObject);
    }

    /**
     * Creates a tree node with no parent, no children, initialized with the
     * specified user object, and that allows children only if specified.
     *
     * @param  _userObject     - an Object provided by the user that describes the node's data
     * @param  _ballowsChildren - if true, the node is allowed to have childnodes -- otherwise, it is always a leaf node
     */
    public HideableMutableTreeNode(Object _userObject, boolean _ballowsChildren) {
            super(_userObject, _ballowsChildren);
    }

    /**
     * Checks if the node is visible.
     *
     * @return  true if the node is visible, else false
     */
    public boolean isVisible() {
            return this.bIsvisible;
    }

    /**
     * Sets if the node is visible.
     *
     * @param  returns true if the node is visible, else false
     */
    public void setVisible(boolean _bIsVisible) {
            this.bIsvisible = _bIsVisible;
    }


        public void addDummyNode(){
            removeDummyNode();
            DefaultMutableTreeNode oDefaultMutableTreeNode = new DefaultMutableTreeNode(SDUMMY);
            add(oDefaultMutableTreeNode);

        }


        public boolean removeDummyNode(){
            boolean breturn = false;
            if (getChildCount() == 1){
                DefaultMutableTreeNode oDefaultMutableTreeNode = (DefaultMutableTreeNode) getChildAt(0);
                if (oDefaultMutableTreeNode != null){
                    if (oDefaultMutableTreeNode.getUserObject().equals(SDUMMY)){
                        remove(0);
                        breturn = true;
                    }
                }
            }
            return breturn;
        }

}