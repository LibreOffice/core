/*************************************************************************
 *
 *  $RCSfile: _XShape.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:32:19 $
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

package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.drawing.XShape;

/**
* Testing <code>com.sun.star.drawing.XShape</code>
* interface methods :
* <ul>
*  <li><code> getPosition()</code></li>
*  <li><code> setPosition()</code></li>
*  <li><code> getSize()</code></li>
*  <li><code> setSize()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'NoPos'</code> <b>optional</b>
*  (of type <code>Object</code>):
*   if this relation exists then position setting is
*   not supported by the object.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XShape
*/
public class _XShape extends MultiMethodTest {

    public XShape oObj = null;        //oObj filled by MultiMethodTest

    Size sOld = new Size();
    Point pOld = new Point();
    Size sNew = new Size();
    Point pNew = new Point();

    /**
    * Gets the size and stores it. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _getSize(){

        boolean result = false;

        log.println("get the size");

        sOld = (Size) oObj.getSize();
        result = true;

        tRes.tested("getSize()", result);
    }

    /**
    * Gets the current position and stores it if the object
    * supports position setting. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown or object doesn't
    * support position setting. <p>
    */
    public void _getPosition(){
        boolean result = false;

        String obj = (String) tEnv.getObjRelation("NoPos");
        if (obj != null) {
            log.println("Can't be used with "+obj);
            result = true;
            tRes.tested("getPosition()", result);
            return;
        }

        log.println("get the position");
        pOld = (Point) oObj.getPosition();
        result = true;

        tRes.tested("getPosition()", result);
    }

    /**
    * Sets a new size different from the size get before. <p>
    * Has <b> OK </b> status if the size returned by <code>getSize()</code>
    * is equal to the size which was set. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getSize() </code> : to set the original size changed.</li>
    * </ul>
    */
    public void _setSize(){
        requiredMethod("getSize()");

        boolean result = true;

        String obj = (String) tEnv.getObjRelation("NoSetSize");
        if (obj != null) {
            log.println("Can't be used with " + obj);
            tRes.tested("setSize()", true);
            return;
        }
        // get the current thread's holder
        sNew = new Size(sOld.Height + 10, sOld.Width + 10) ;

        //set new size
        log.println("change the size");
        try {
            oObj.setSize(sNew);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception while calling the method :" + e);
            result = true ;
        }

        Size gSize = oObj.getSize() ;

        log.println("Previously: "+sOld.Height+";"+sOld.Width);
        log.println("Expected: "+sNew.Height+";"+sNew.Width);
        log.println("Getting: "+gSize.Height+";"+gSize.Width);

        //result &= util.ValueComparer.equalValue(sNew, gSize) ;
        //errors in calculation from points/twips less then 1 are acceptable
        result &= (sNew.Height-gSize.Height < 2) && (sNew.Width-gSize.Width < 2);
        tRes.tested("setSize()", result);
    }

    /**
    * If object doesn't support position setting the test does nothing.
    * Else a new position is created and set.<p>
    * Has <b> OK </b> status if get position is equal to set position or
    * if the position setting isn't supported. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getPosition() </code> : to change old position. </li>
    * </ul>
    */
    public void _setPosition(){
        requiredMethod("getPosition()");

        boolean result = true;

        String obj = (String) tEnv.getObjRelation("NoPos");
        if (obj != null) {
            log.println("Can't be used with " + obj);
            tRes.tested("setPosition()", true);
            return;
        }

        // get the current thread's holder
        pNew = new Point(pOld.X + 100, pOld.Y + 100) ;
        oObj.setPosition(pNew);

        Point gPos = oObj.getPosition() ;
        result = !util.ValueComparer.equalValue(pOld, gPos) ;

        tRes.tested("setPosition()", result);
    }


}  // finish class _XShape



