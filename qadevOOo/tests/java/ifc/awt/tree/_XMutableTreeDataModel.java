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

import com.sun.star.awt.tree.XMutableTreeDataModel;
import com.sun.star.awt.tree.XMutableTreeNode;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.awt.tree.XMutableTreeDataModel</code>
* interface methods :
* <ul>
*  <li><code> createNode()</code></li>
*  <li><code> setRoot()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>

* @see com.sun.star.awt.tree.XMutableTreeDataModel
*/
public class _XMutableTreeDataModel extends MultiMethodTest {

    public XMutableTreeDataModel oObj = null;

    private XMutableTreeNode mNewNode = null;

    /**
    * Sets the title to some string. <p>
    * Has <b>OK</b> status if no runtime exceptions occurs.
    */
    public void _createNode() {

        mNewNode = oObj.createNode("Hallo Welt", true);

        tRes.tested("createNode()", true) ;
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
    public void _setRoot() {
        requiredMethod("createNode()") ;

        boolean bOK = true;
        try {

            oObj.setRoot(mNewNode);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            bOK = false;
            log.println("ERROR: while trying to set a new root an IllegalArgumentException was thrown:\n" + ex.toString());
        }

        try {

            oObj.setRoot(null);
            bOK = false;
            log.println("ERROR: while trying to set a null object as root expected IllegalArgumentException was not thrown.");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("expected IllegalArgumentException was thrown => ok");

        }

        tRes.tested("setRoot()", bOK);

    }

}


