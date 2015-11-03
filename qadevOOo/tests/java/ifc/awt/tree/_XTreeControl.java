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

package ifc.awt.tree;

import com.sun.star.awt.tree.ExpandVetoException;
import com.sun.star.awt.tree.TreeExpansionEvent;
import com.sun.star.awt.tree.XMutableTreeNode;
import com.sun.star.awt.tree.XTreeControl;
import com.sun.star.awt.tree.XTreeEditListener;
import com.sun.star.awt.tree.XTreeExpansionListener;
import com.sun.star.awt.tree.XTreeNode;
import com.sun.star.lang.EventObject;
import com.sun.star.util.VetoException;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 * Testing <code>com.sun.star.awt.tree.XTreeControl</code>
 * interface methods :
 * <ul>
 *  <li><code> isNodeExpanded()</code></li>
 *  <li><code> isNodeCollapsed()</code></li>
 *  <li><code> makeNodeVisible()</code></li>
 *  <li><code> isNodeVisible()</code></li>
 *  <li><code> expandNode()</code></li>
 *  <li><code> collapseNode()</code></li>
 *  <li><code> addTreeExpansionListener()</code></li>
 *  <li><code> removeTreeExpansionListener()</code></li>
 *  <li><code> getNodeForLocation()</code></li>
 *  <li><code> getClosestNodeForLocation()</code></li>
 *  <li><code> isEditing()</code></li>
 *  <li><code> stopEditing()</code></li>
 *  <li><code> cancelEditing()</code></li>
 *  <li><code> startEditingAtNode()</code></li>
 *  <li><code> addTreeEditListener()</code></li>
 *  <li><code> removeTreeEditListener()</code></li>
 *  <li><code> DefaultExpandedGraphicURL()</code></li>
 *  <li><code> DefaultCollapsedGraphicURL()</code></li>* </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 *
 * @see com.sun.star.awt.tree.XTreeControl
 */
public class _XTreeControl extends MultiMethodTest {

    public XTreeControl oObj = null;

    private XMutableTreeNode mXNode = null;

    XTreeExpansionListener mTreeExpansionListener1 = new TreeExpansionListenerImpl1();

    XTreeExpansionListener mTreeExpansionListener2 = new TreeExpansionListenerImpl2();

    XTreeEditListener mTreeEditListener1 = new TreeEditListenerImpl1();

    XTreeEditListener mTreeEditListener2 = new TreeEditListenerImpl2();

    boolean mTreeExpanded1 = false;
    boolean mTreeExpanding1 = false;
    boolean mTreeCollapsed1 = false;
    boolean mTreeCollapsing1 = false;

    boolean mTreeExpanded2 = false;
    boolean mTreeExpanding2 = false;
    boolean mTreeCollapsed2 = false;
    boolean mTreeCollapsing2 = false;

    boolean mTreeNodeEditing1 = false;
    boolean mTreeNodeEdit1 = false;

    public interface XTreeDataModelListenerEvent{
        void fireEvent();
    }

    public class TreeExpansionListenerImpl1 implements XTreeExpansionListener{
        public void requestChildNodes(TreeExpansionEvent treeExpansionEvent) {
            log.println("event at ExpansionListener 1: requestChildNodes");
        }

        public void treeExpanding(TreeExpansionEvent treeExpansionEvent) throws ExpandVetoException {
            log.println("event at ExpansionListener 1: treeExpanding");
            mTreeExpanding1 = true;
        }

        public void treeCollapsing(TreeExpansionEvent treeExpansionEvent) throws ExpandVetoException {
            log.println("event at ExpansionListener 1: treeCollapsing");
            mTreeCollapsing1 = true;
        }

        public void treeExpanded(TreeExpansionEvent treeExpansionEvent) {
            log.println("event at ExpansionListener 1: treeExpanded");
            mTreeExpanded1 = true;
        }

        public void treeCollapsed(TreeExpansionEvent treeExpansionEvent) {
            log.println("event at ExpansionListener 1: treeCollapsed");
            mTreeCollapsed1 = true;
        }

        public void disposing(EventObject eventObject) {
            log.println("event at ExpansionListener 1: disposing");
        }
    }

    public class TreeExpansionListenerImpl2 implements XTreeExpansionListener{
        public void requestChildNodes(TreeExpansionEvent treeExpansionEvent) {
            log.println("event at ExpansionListener 2: requestChildNodes");
        }

        public void treeExpanding(TreeExpansionEvent treeExpansionEvent) throws ExpandVetoException {
            log.println("event at ExpansionListener 2: treeExpanding");
            mTreeExpanding2 = true;
        }

        public void treeCollapsing(TreeExpansionEvent treeExpansionEvent) throws ExpandVetoException {
            log.println("event at ExpansionListener 2: treeCollapsing");
            mTreeCollapsing2 = true;
        }

        public void treeExpanded(TreeExpansionEvent treeExpansionEvent) {
            log.println("event at ExpansionListener 2: treeExpanded");
            mTreeExpanded2 = true;
        }

        public void treeCollapsed(TreeExpansionEvent treeExpansionEvent) {
            log.println("event at ExpansionListener 2: treeCollapsed");
            mTreeCollapsed2 = true;
        }

        public void disposing(EventObject eventObject) {
            log.println("event at ExpansionListener 2: disposing");
        }

    }

    public class TreeEditListenerImpl1 implements XTreeEditListener{
        public void nodeEditing(XTreeNode xTreeNode) throws VetoException {
            log.println("event at EditListener 1: nodeEditing");
            mTreeNodeEditing1 = true;
        }

        public void nodeEdited(XTreeNode xTreeNode, String string) {
            log.println("event at EditListener 1: nodeEdited");
            mTreeNodeEdit1 = true;
        }

        public void disposing(EventObject eventObject) {
            log.println("event at EditListener 1: disposing");
        }
    }

    public class TreeEditListenerImpl2 implements XTreeEditListener{
        public void nodeEditing(XTreeNode xTreeNode) throws VetoException {
            log.println("event at EditListener 2: nodeEditing");
        }

        public void nodeEdited(XTreeNode xTreeNode, String string) {
            log.println("event at EditListener 2: nodeEdited");
        }

        public void disposing(EventObject eventObject) {
            log.println("event at EditListener 2: disposing");
        }
    }

    private void resetTreeExpandingListener(){
        mTreeExpanded1 = false;
        mTreeExpanded2 = false;
        mTreeCollapsed1 = false;
        mTreeCollapsed2 = false;
        mTreeExpanding1 = false;
        mTreeExpanding2 = false;
        mTreeCollapsing1 = false;
        mTreeCollapsing2 = false;
    }

    private void resetEditListener(){
        mTreeNodeEditing1 = false;
        mTreeNodeEdit1 = false;
    }

    @Override
    public void before(){
        mXNode = (XMutableTreeNode) tEnv.getObjRelation("XTreeControl_Node");
        if (mXNode == null) {
            throw new StatusException(Status.failed("ERROR: could not get object relation 'XTreeControl_Node'"));
        }
    }

    public void _isNodeExpanded() {

        boolean bOK = true;

        boolean isExpanded = false;

        try {
            isExpanded = oObj.isNodeExpanded(mXNode);
            log.println("node is expanded: " + isExpanded);

        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not query for 'isNodeExpanded()': " + ex.toString());
        }

        if (isExpanded)
            try {

                log.println("try to collapse node...");
                oObj.collapseNode(mXNode);

            } catch (ExpandVetoException ex) {
                bOK = false;
                log.println("ERROR: could not collapse node: " + ex.toString());
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                bOK = false;
                log.println("ERROR: could not collapse node: " + ex.toString());
            }
        else
            try {

                log.println("try to expand node...");
                oObj.expandNode(mXNode);

            } catch (ExpandVetoException ex) {
                bOK = false;
                log.println("ERROR: could not expand node: " + ex.toString());
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                bOK = false;
                log.println("ERROR: could not expand node: " + ex.toString());
            }

        try {

            boolean isAlsoExpanded = oObj.isNodeExpanded(mXNode);

            bOK &= (isExpanded != isAlsoExpanded);

            log.println("node is expanded: " + isAlsoExpanded + " => " + bOK);

        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not query for 'isNodeExpanded()': " + ex.toString());
        }

        tRes.tested("isNodeExpanded()", bOK);

    }
    public void _isNodeCollapsed(){

        boolean bOK = true;

        boolean isCollapsed = false;

        try {
            isCollapsed = oObj.isNodeCollapsed(mXNode);
            log.println("node is Collapsed: " + isCollapsed);

        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not query for 'isNodeCollapsed()': " + ex.toString());
        }

        if ( ! isCollapsed)
            try {

                log.println("try to collapse node...");
                oObj.collapseNode(mXNode);

            } catch (ExpandVetoException ex) {
                bOK = false;
                log.println("ERROR: could not collapse node: " + ex.toString());
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                bOK = false;
                log.println("ERROR: could not collapse node: " + ex.toString());
            }
        else
            try {

                log.println("try to expand node...");
                oObj.expandNode(mXNode);

            } catch (ExpandVetoException ex) {
                bOK = false;
                log.println("ERROR: could not expand node: " + ex.toString());
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                bOK = false;
                log.println("ERROR: could not expand node: " + ex.toString());
            }

        try {

            boolean isAlsoCollapsed = oObj.isNodeCollapsed(mXNode);

            bOK &= (isCollapsed != isAlsoCollapsed);

            log.println("node is Collapsed: " + isAlsoCollapsed + " => " + bOK);

        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not query for 'isNodeCollapsed()': " + ex.toString());
        }

        tRes.tested("isNodeCollapsed()", bOK);

    }
    public void _makeNodeVisible(){

        boolean bOK = true;
        try {

            oObj.makeNodeVisible(mXNode);
        } catch (ExpandVetoException ex) {
            bOK = false;
            log.println("ERROR: could not call method 'makeNodeVisible()' successfully: " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not call method 'makeNodeVisible()' successfully: " + ex.toString());
        }
        try {

            bOK &= oObj.isNodeVisible(mXNode);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not call method 'isNodeVisible()' successfully: " + ex.toString());
        }

        tRes.tested("makeNodeVisible()", bOK);

    }
    public void _isNodeVisible(){

        this.requiredMethod("makeNodeVisible()");

        boolean bOK = true;
        log.println("since required method 'makeNodeVisible()' uses method 'isNodeVisible()' this test is ok.");
        tRes.tested("isNodeVisible()", bOK);

    }

    public void _expandNode(){

        this.requiredMethod("isNodeExpanded()");

        boolean bOK = true;
        log.println("since required method 'isnodeExpanded()' uses method 'expandNode()' this test is ok.");
        tRes.tested("expandNode()", bOK);

    }

    public void _collapseNode(){

        this.requiredMethod("isNodeCollapsed()");

        boolean bOK = true;
        log.println("since required method 'isnodeCollapsed()' uses method 'expandNode()' this test is ok.");
        tRes.tested("collapseNode()", bOK);

    }

    public void _addTreeExpansionListener(){

        boolean bOK = true;

        log.println("collapse Node to get an initial status...");
        try {

            oObj.collapseNode(mXNode);
        } catch (ExpandVetoException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        }

        log.println("add mTreeExpansionListener1...");
        oObj.addTreeExpansionListener(mTreeExpansionListener1);

        log.println("add mTreeExpansionListener2");
        oObj.addTreeExpansionListener(mTreeExpansionListener2);

        resetTreeExpandingListener();

        log.println("expand Node...");
        try {

            oObj.expandNode(mXNode);

        } catch (ExpandVetoException ex) {
            bOK = false;
            log.println("ERROR: could not expand node: " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not expand node: " + ex.toString());
        }

        bOK &= mTreeExpanded1  & mTreeExpanded2  & ! mTreeCollapsed1  & ! mTreeCollapsed2
             & mTreeExpanding1 & mTreeExpanding2 & ! mTreeCollapsing1 & ! mTreeCollapsing2;

        resetTreeExpandingListener();

        log.println("collapse Node...");
        try {

            oObj.collapseNode(mXNode);
        } catch (ExpandVetoException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        }

        bOK &= ! mTreeExpanded1  & ! mTreeExpanded2  & mTreeCollapsed1  & mTreeCollapsed2
             & ! mTreeExpanding1 & ! mTreeExpanding2 & mTreeCollapsing1 & mTreeCollapsing2;

        tRes.tested("addTreeExpansionListener()", bOK);

    }

    public void _removeTreeExpansionListener(){

        this.requiredMethod("addTreeExpansionListener()");

        boolean bOK = true;

        log.println("collapse Node to get an initial status...");
        try {

            oObj.collapseNode(mXNode);
        } catch (ExpandVetoException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        }

        resetTreeExpandingListener();

        log.println("remove mTreeExpansionListener2 ...");
        oObj.removeTreeExpansionListener(mTreeExpansionListener2);

        log.println("expand Node...");
        try {

            oObj.expandNode(mXNode);

        } catch (ExpandVetoException ex) {
            bOK = false;
            log.println("ERROR: could not expand node: " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not expand node: " + ex.toString());
        }

        bOK &= mTreeExpanded1  & ! mTreeExpanded2  & ! mTreeCollapsed1  & ! mTreeCollapsed2
             & mTreeExpanding1 & ! mTreeExpanding2 & ! mTreeCollapsing1 & ! mTreeCollapsing2;

        resetTreeExpandingListener();

        log.println("collapse Node...");
        try {

            oObj.collapseNode(mXNode);
        } catch (ExpandVetoException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: could not collapse node: " + ex.toString());
        }

        bOK &= ! mTreeExpanded1  & ! mTreeExpanded2  & mTreeCollapsed1  & ! mTreeCollapsed2
             & ! mTreeExpanding1 & ! mTreeExpanding2 & mTreeCollapsing1 & ! mTreeCollapsing2;

        log.println("remove mTreeExpansionListener2 ...");
        oObj.removeTreeExpansionListener(mTreeExpansionListener2);

        tRes.tested("removeTreeExpansionListener()", bOK);

    }

    public void _getNodeForLocation(){

        boolean bOK = false;


        tRes.tested("getNodeForLocation()", bOK);

    }

    public void _getClosestNodeForLocation(){

        boolean bOK = false;

        log.println("try to get Node for location 100,100");
        XTreeNode myNode = oObj.getClosestNodeForLocation(100,100);

        bOK &= (myNode != null);

        tRes.tested("getClosestNodeForLocation()", bOK);

    }

    public void _isEditing(){

        boolean bOK = false;

        log.println("not ready because of i78701");

        tRes.tested("isEditing()", bOK);

    }

    public void _stopEditing(){

        boolean bOK = false;
        log.println("not ready because of i78701");


        tRes.tested("stopEditing()", bOK);

    }

    public void _cancelEditing(){

        boolean bOK = false;

        log.println("not ready because of i78701");

        tRes.tested("cancelEditing()", bOK);

    }

    public void _startEditingAtNode(){

        boolean bOK = false;

        log.println("not ready because of i78701");

        tRes.tested("startEditingAtNode()", bOK);

    }

    public void _addTreeEditListener(){

        boolean bOK = true;

        log.println("add EditListener 1....");
        oObj.addTreeEditListener(mTreeEditListener1);

        log.println("add EditListener 2....");
        oObj.addTreeEditListener(mTreeEditListener2);

        resetEditListener();

        try {

            oObj.startEditingAtNode(mXNode);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: could not start editing at node: " + ex.toString());
            bOK = false;
        }

        log.println("not ready because of i78701");
        bOK = false;


        tRes.tested("addTreeEditListener()", bOK);

    }

    public void _removeTreeEditListener(){

        boolean bOK = false;

        log.println("not ready because of i78701");

        tRes.tested("removeTreeEditListener()", bOK);

    }

    public void _DefaultExpandedGraphicURL(){

        boolean bOK = true;

        String current = oObj.getDefaultExpandedGraphicURL();
        log.println("current DefaultExpandedGraphicURL: " + current);

        String changeUrl = null;

        if (current.equals("private:graphicrepository/sd/res/triangle_right.png"))
             changeUrl = "private:graphicrepository/sd/res/triangle_down.png";
        else changeUrl = "private:graphicrepository/sd/res/triangle_right.png";

        log.println("try to set '" + changeUrl + "' as new DefaultExpandedGraphicURL");

        oObj.setDefaultExpandedGraphicURL(changeUrl);

        String newUrl = oObj.getDefaultExpandedGraphicURL();

        log.println("new DefaultExpandedGraphicURL: " + newUrl);

        bOK &= ! newUrl.equals(current);

        tRes.tested("DefaultExpandedGraphicURL()", bOK);

    }

    public void _DefaultCollapsedGraphicURL(){

        boolean bOK = true;

        String current = oObj.getDefaultCollapsedGraphicURL();
        log.println("current DefaultCollapsedGraphicURL: " + current);

        String changeUrl = null;

        if (current.equals("private:graphicrepository/sd/res/triangle_right.png"))
             changeUrl = "private:graphicrepository/sd/res/triangle_down.png";
        else changeUrl = "private:graphicrepository/sd/res/triangle_right.png";

        log.println("try to set '" + changeUrl + "' as new DefaultCollapsedGraphicURL");
        oObj.setDefaultCollapsedGraphicURL(changeUrl);

        String newUrl = oObj.getDefaultCollapsedGraphicURL();

        log.println("new DefaultCollapsedGraphicURL: " + newUrl);

        bOK &= ! newUrl.equals(current);

        tRes.tested("DefaultCollapsedGraphicURL()", bOK);
    }
}


