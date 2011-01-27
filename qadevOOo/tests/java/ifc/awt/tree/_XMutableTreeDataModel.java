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
* Test is <b> NOT </b> multithread compilant. <p>

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


