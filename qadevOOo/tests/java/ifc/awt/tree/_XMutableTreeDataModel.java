/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XMutableTreeDataModel.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:50:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


