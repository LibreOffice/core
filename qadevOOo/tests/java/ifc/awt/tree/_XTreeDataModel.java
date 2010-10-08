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

package ifc.awt.tree;

import com.sun.star.awt.tree.TreeDataModelEvent;
import com.sun.star.awt.tree.XMutableTreeNode;
import com.sun.star.awt.tree.XTreeDataModel;
import com.sun.star.awt.tree.XTreeDataModelListener;
import com.sun.star.awt.tree.XTreeNode;
import com.sun.star.lang.EventObject;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 * Testing <code>com.sun.star.awt.tree.XTreeDataModel</code>
 * interface methods :
 * <ul>
 *  <li><code> createNode()</code></li>
 *  <li><code> setRoot()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 *
 * @see com.sun.star.awt.tree.XTreeDataModel
 */
public class _XTreeDataModel extends MultiMethodTest {

    public XTreeDataModel oObj = null;

    private XMutableTreeNode mNewNode = null;

    XTreeDataModelListener mListener1 = new myEventListener1();

    XTreeDataModelListener mListener2 = new myEventListener2();

    XTreeDataModelListenerEvent mListenerEvent = null;

    boolean mTreeStructureChanged1 = false;

    boolean mTreeStructureChanged2 = false;

    public static interface XTreeDataModelListenerEvent{
        public void fireEvent();
    }

    public class myEventListener1 implements XTreeDataModelListener{
        public void treeNodesChanged(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 1: treeNodesChanged");
        }

        public void treeNodesInserted(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 1: treeNodesInserted");
        }

        public void treeNodesRemoved(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 1: treeNodesRemoved");
        }

        public void treeStructureChanged(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 1: treeStructureChanged");
            mTreeStructureChanged1 = true;
        }

        public void disposing(EventObject eventObject) {
            log.println("Listener 1: disposing");
        }
    }

    public class myEventListener2 implements XTreeDataModelListener{
        public void treeNodesChanged(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 2: treeNodesChanged");
        }

        public void treeNodesInserted(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 2: treeNodesInserted");
        }

        public void treeNodesRemoved(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 2: treeNodesRemoved");
        }

        public void treeStructureChanged(TreeDataModelEvent treeDataModelEvent) {
            log.println("Listener 2: treeStructureChanged");
            mTreeStructureChanged2 = true;
        }

        public void disposing(EventObject eventObject) {
            log.println("Listener 2: disposing");
        }

    }

    public void before(){
        mListenerEvent = (XTreeDataModelListenerEvent) tEnv.getObjRelation("XTreeDataModelListenerEvent");
        if (mListenerEvent == null) {
            throw new StatusException(Status.failed("ERROR: could not get object relation 'XTreeDataModelListenerEvent'\n" +
                "This object relation must be kind of 'ifc.awt.tree._XTreeDataModel.XTreeDataModelListenerEvent'"));
        }
    }

    /**
     * Gets the title and compares it to the value set in
     * <code>setTitle</code> method test. <p>
     * Has <b>OK</b> status is set/get values are equal.
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> setTitle </code>  </li>
     * </ul>
     */
    public void _getRoot() {

        boolean bOK = true;
        XTreeNode root = oObj.getRoot();

        tRes.tested("getRoot()", bOK);

    }

    public void _addTreeDataModelListener(){

        boolean bOK = true;

        log.println("add listener 1 to object...");
        oObj.addTreeDataModelListener(mListener1);

        log.println("add listener 2 to object...");
        oObj.addTreeDataModelListener(mListener2);

        mTreeStructureChanged1 = false;
        mTreeStructureChanged2 = false;

        mListenerEvent.fireEvent();

        bOK = mTreeStructureChanged1 & mTreeStructureChanged2;

        tRes.tested("addTreeDataModelListener()", bOK);
    }

    public void _removeTreeDataModelListener(){

        boolean bOK = true;

        log.println("remove listener 2 from object...");
        oObj.removeTreeDataModelListener(mListener2);

        mTreeStructureChanged1 = false;
        mTreeStructureChanged2 = false;

        mListenerEvent.fireEvent();

        bOK = mTreeStructureChanged1 & ! mTreeStructureChanged2;

        tRes.tested("removeTreeDataModelListener()", bOK);

    }
}


