/*************************************************************************
 *
 *  $RCSfile: _XView.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:14:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.Size;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XView;

/**
* Testing <code>com.sun.star.awt.XView</code>
* interface methods:
* <ul>
*  <li><code> setGraphics() </code></li>
*  <li><code> getGraphics() </code></li>
*  <li><code> getSize() </code></li>
*  <li><code> draw() </code></li>
*  <li><code> setZoom() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'GRAPHICS'</code> (of type <code>XGraphics</code>):
*   used as a parameter to setGraphics() </li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XView
*/
public class _XView extends MultiMethodTest {
    public XView oObj = null;

    /**
    * After obtaining object relation 'GRAPHICS', test calls the method. <p>
    * Has <b> OK </b> status if the method returns true.
    */
    public void _setGraphics() {
        XGraphics graph = (XGraphics) tEnv.getObjRelation("GRAPHICS");
        boolean isSet = oObj.setGraphics(graph);
        if ( !isSet ) {
            log.println("setGraphics() returns false");
        }
        tRes.tested("setGraphics()", isSet);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method does not return null. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _getGraphics() {
        requiredMethod("setGraphics()");
        XGraphics graph = oObj.getGraphics();
        if (graph == null) {
            log.println("getGraphics() returns NULL");
        }
        tRes.tested("getGraphics()", graph != null);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method returns structure with fields that
    * are not equal to zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _getSize() {
        requiredMethod("setGraphics()");
        Size aSize = oObj.getSize();
        boolean res = (aSize.Height != 0) && (aSize.Width != 0);
        if ( !res ) {
            log.println("Height: " + aSize.Height);
            log.println("Width: " + aSize.Width);
        }
        tRes.tested("getSize()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _draw() {
        requiredMethod("setGraphics()");
        oObj.draw(20, 20);
        tRes.tested("draw()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setGraphics() </code> : sets the output device </li>
    * </ul>
    */
    public void _setZoom() {
        requiredMethod("setGraphics()");
        oObj.setZoom(2,2);
        tRes.tested("setZoom()", true);
    }

}

