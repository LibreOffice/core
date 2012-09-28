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

import com.sun.star.awt.tree.TreeDataModelEvent;
import com.sun.star.awt.tree.XTreeDataModel;
import com.sun.star.awt.tree.XTreeDataModelListener;
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
        oObj.getRoot();

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


