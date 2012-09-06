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
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;


public class HideableTreeModel implements TreeModel {

    private Vector modelListeners = new Vector();
    private Object root = null;


        public HideableTreeModel(TreeNode _root) {
            super();
            setRoot(_root);
    }


    public Object getRoot() {
        return this.root;
    }


        protected void setRoot(Object r) {
            this.root = r;
    }


        public Object[] getPathToRoot(Object node) {
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
            modelListeners.addElement(l);
    }


        public void removeTreeModelListener(TreeModelListener l) {
            modelListeners.removeElement(l);
    }


    public void reload() {
            reload(getRoot());
    }


        public void reload(Object node) {
            if(node != null) {
                TreePath tp = new TreePath(getPathToRoot(node));
                fireTreeStructureChanged(new TreeModelEvent(this, tp));
            }
    }


        public void valueForPathChanged(TreePath path, Object newValue) {
            nodeChanged(path.getLastPathComponent());
    }

    public void nodeInserted(Object node, Object child) {
            nodeInserted(node, child, -1);
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


        public void nodeRemoved(Object node, Object child, int index) {
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


        protected void fireTreeNodesChanged(TreeModelEvent event) {
            for(int i = 0; i < modelListeners.size(); i++) {
                ((TreeModelListener)modelListeners.elementAt(i)).treeNodesChanged(event);
            }
    }


        protected void fireTreeNodesInserted(TreeModelEvent event) {
            for(int i = 0; i < modelListeners.size(); i++) {
                ((TreeModelListener)modelListeners.elementAt(i)).treeNodesInserted(event);
            }
    }


        protected void fireTreeNodesRemoved(TreeModelEvent event) {
            for(int i = 0; i < modelListeners.size(); i++) {
                ((TreeModelListener)modelListeners.elementAt(i)).treeNodesRemoved(event);
            }
    }

    protected void fireTreeStructureChanged(TreeModelEvent event) {
            for(int i = 0; i < modelListeners.size(); i++) {
                ((TreeModelListener)modelListeners.elementAt(i)).treeStructureChanged(event);
            }
    }


        public ArrayList getExpandedPaths(JTree tree) {
        ArrayList expandedPaths = new ArrayList();
        addExpandedPaths(tree, tree.getPathForRow(0), expandedPaths);
        return expandedPaths;
    }


        private void addExpandedPaths(JTree tree, TreePath path, ArrayList pathlist) {
            Enumeration aEnum = tree.getExpandedDescendants(path);
            while(aEnum.hasMoreElements()) {
                TreePath tp = (TreePath) aEnum.nextElement();
                pathlist.add(tp);
                addExpandedPaths(tree, tp, pathlist);
            }
    }


        public void expandPaths(JTree tree, ArrayList pathlist) {
            for(int i = 0; i < pathlist.size(); i++) {
                tree.expandPath((TreePath)pathlist.get(i));
            }
    }


        public boolean isLeaf(Object _oNode) {
            if(_oNode instanceof TreeNode) {
                return ((TreeNode) _oNode).isLeaf();
            }
            return true;
    }



    public Object getParent(Object node) {
            if(node != getRoot() && (node instanceof TreeNode)) {
                return ((TreeNode)node).getParent();
            }
            return null;
    }


        public boolean isNodeVisible(Object node) {
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


        public boolean isPathToNodeVisible(Object node) {
            Object[] path = getPathToRoot(node);
            for(int i = 0; i < path.length; i++) {
                if(!isNodeVisible(path[i])) {
                    return false;
                }
            }
            return true;
    }


        public void ensurePathToNodeVisible(Object node) {
            Object[] path = getPathToRoot(node);
            for(int i = 0; i < path.length; i++) {
                setNodeVisible(path[i], true);
            }
    }


        public Object getChild(Object parent, int index) {
            if(parent instanceof TreeNode) {
                TreeNode p = (TreeNode) parent;
                for(int i = 0, j = -1; i < p.getChildCount(); i++) {
                    TreeNode pc = (TreeNode)p.getChildAt(i);
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
                    TreeNode pc = (TreeNode)p.getChildAt(i);
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
                        TreeNode pc = (TreeNode)p.getChildAt(i);
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