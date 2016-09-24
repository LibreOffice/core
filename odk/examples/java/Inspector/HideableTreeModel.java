/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import java.util.ArrayList;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;


public class HideableTreeModel implements TreeModel {

    private ArrayList<TreeModelListener> modelListeners = new ArrayList<TreeModelListener>();
    private Object root = null;


        public HideableTreeModel(TreeNode _root) {
            super();
            setRoot(_root);
    }


    public Object getRoot() {
        return this.root;
    }


        private void setRoot(Object r) {
            this.root = r;
    }


        private Object[] getPathToRoot(Object node) {
            return getPathToRoot(node, 0);
    }


        private Object[] getPathToRoot(Object node, int i) {
            Object anode[];
            if(node == null) {
                if(i == 0) {
                    return null;
                }
                anode = new Object[i];
            } else {
                i++;
                if(node == getRoot()) {
                    anode = new Object[i];
                } else {
                    anode = getPathToRoot(getParent(node), i);
                }
                anode[anode.length - i] = node;
            }
            return anode;
    }


        public void addTreeModelListener(TreeModelListener l) {
            modelListeners.add(l);
    }


        public void removeTreeModelListener(TreeModelListener l) {
            modelListeners.remove(l);
    }





        public void valueForPathChanged(TreePath path, Object newValue) {
            nodeChanged(path.getLastPathComponent());
    }




        public void nodeInserted(Object node, Object child, int index) {
            if(index < 0) {
                index = getIndexOfChild(node, child);
            }
            if(node != null && child != null && index >= 0) {
                TreePath tp = new TreePath(getPathToRoot(node));
                int[] ai = { index };
                Object[] ac = { child };
                fireTreeNodesInserted(new TreeModelEvent(this, tp, ai, ac));
            }
    }


        private void nodeRemoved(Object node, Object child, int index) {
            if(node != null && child != null && index >= 0) {
                TreePath tp = new TreePath(getPathToRoot(node));
                int[] ai = { index };
                Object[] ac = { child };
                fireTreeNodesRemoved(new TreeModelEvent(this, tp, ai, ac));
            }
    }


        public void nodeChanged(Object node) {
            if(node != null) {
                TreePath tp = new TreePath(getPathToRoot(node));
                fireTreeNodesChanged(new TreeModelEvent(this, tp, null, null));
            }
    }


        private void fireTreeNodesChanged(TreeModelEvent event) {
            for(TreeModelListener l : modelListeners) {
                l.treeNodesChanged(event);
            }
    }


        private void fireTreeNodesInserted(TreeModelEvent event) {
            for(TreeModelListener l : modelListeners) {
                l.treeNodesInserted(event);
            }
    }


        private void fireTreeNodesRemoved(TreeModelEvent event) {
            for(TreeModelListener l : modelListeners) {
                l.treeNodesRemoved(event);
            }
    }

    public boolean isLeaf(Object _oNode) {
            if(_oNode instanceof TreeNode) {
                return ((TreeNode) _oNode).isLeaf();
            }
            return true;
    }



    private Object getParent(Object node) {
            if(node != getRoot() && (node instanceof TreeNode)) {
                return ((TreeNode)node).getParent();
            }
            return null;
    }


        private boolean isNodeVisible(Object node) {
            if(node != getRoot()) {
                if(node instanceof HideableMutableTreeNode) {
                        return ((HideableMutableTreeNode)node).isVisible();
                }
            }
            return true;
    }


        public boolean setNodeVisible(Object node, boolean v) {
            // can't hide root
            if(node != getRoot()) {
                if(node instanceof HideableMutableTreeNode) {
                    HideableMutableTreeNode n = (HideableMutableTreeNode)node;
                    if(v != n.isVisible()) {
                        TreeNode parent = n.getParent();
                        if(v) {
                            // need to get index after showing...
                            n.setVisible(v);
                            int index = getIndexOfChild(parent, n);
                            nodeInserted(parent, n, index);
                        } else {
                            // need to get index before hiding...
                            int index = getIndexOfChild(parent, n);
                            n.setVisible(v);
                            nodeRemoved(parent, n, index);
                        }
                    }
                    return true;
                }
            }
            return false;
    }








        public Object getChild(Object parent, int index) {
            if(parent instanceof TreeNode) {
                TreeNode p = (TreeNode) parent;
                for(int i = 0, j = -1; i < p.getChildCount(); i++) {
                    TreeNode pc = p.getChildAt(i);
                    if(isNodeVisible(pc)) {
                        j++;
                    }
                    if(j == index) {
                        return pc;
                    }
                }
            }
            return null;
    }


        public int getChildCount(Object parent) {
            int count = 0;
            if(parent instanceof TreeNode) {
                TreeNode p = (TreeNode) parent;
                for(int i = 0; i < p.getChildCount(); i++) {
                    TreeNode pc = p.getChildAt(i);
                    if(isNodeVisible(pc)) {
                        count++;
                    }
                }
            }
            return count;
    }


        public int getIndexOfChild(Object parent, Object child) {
            int index = -1;
            if(parent instanceof TreeNode && child instanceof TreeNode) {
                TreeNode p = (TreeNode)parent;
                TreeNode c = (TreeNode)child;
                if(isNodeVisible(c)) {
                    index = 0;
                    for(int i = 0; i < p.getChildCount(); i++) {
                        TreeNode pc = p.getChildAt(i);
                        if(pc.equals(c)) {
                            return index;
                        }
                        if(isNodeVisible(pc)) {
                            index++;
                        }
                    }
                }
            }
            return index;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
