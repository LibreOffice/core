/*************************************************************************
 *
 *  $RCSfile: _XAutoTextContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:14:49 $
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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XAutoTextContainer;
import com.sun.star.text.XAutoTextGroup;

/**
 * Testing <code>com.sun.star.text.XAutoTextContainer</code>
 * interface methods :
 * <ul>
 *  <li><code> insertNewByName()</code></li>
 *  <li><code> removeByName()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XAutoTextContainer
 */
public class _XAutoTextContainer  extends MultiMethodTest {
    public XAutoTextContainer oObj = null;
    // every Thread must insert it's own AutoTextContainer:
    public String Name = "";

    /**
     * First removes old element from container with the specified name
     * if it exists. Then tries to add a new group with the specified
     * name. <p>
     *
     * Has <b>OK</b> status if not <code>null</code>
     * <code>AutoTextGroup</code> instance is returned.
     */
    public void _insertNewByName() {
        System.out.println("Starting: insertNewByName");
        boolean result = true;
        Name = "XAutoTextContainerx" + Thread.currentThread().getName();
        Name = Name.replace('-','x');
        Name = Name.replace(':','x');
        Name = Name.replace('.','x');
        XAutoTextGroup oGroup = null;
        //first clear the container
        log.println("remove old elements in container");
        System.out.println("remove old elements in container");
        try {
            oObj.removeByName(Name);
            log.println("old elements removed -> OK");
            System.out.println("old elements removed -> OK");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("no old elements available -> OK");
            System.out.println("no old elements available -> OK");
        }

        // insert an element
        log.println("insertNewByName");
        try {
            System.out.println("Inserting element with name '" + Name + "'");
            log.println("Inserting element with name '" + Name + "'");
            oGroup = oObj.insertNewByName(Name);
            System.out.println("done");
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("insertNewByName(): " + e);
            result &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("insertNewByName(): " + e);
            result &= false;
        }

        result &= ( oGroup != null );
        tRes.tested("insertNewByName()", result);
    } // end insertNewByName()

    /**
     * First removes element by name which was added before,
     * then tries to remove the element with the same name again. <p>
     *
     * Has <b> OK </b> status if in the first case no exceptions
     * were thrown, and in the second case
     * <code>NoSuchElementException</code> was thrown. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> insertNewByName() </code> : new element inserted here.</li>
     * </ul>
     */
    public void _removeByName() {
        requiredMethod("insertNewByName()");

        boolean result = true;
        // remove the element
        log.println("removeByName()");
        try {
            log.println("Removing element with name '" + Name + "'");
            oObj.removeByName(Name);
            result &= true;
        } catch (com.sun.star.container.NoSuchElementException e) {
            result = false;
            log.println("removeByName(): " + e + " -> FAILD");
        }

        log.println("2nd removeByName()");
        try {
            oObj.removeByName(Name);
            log.println("No exceptions were thrown -> FAILED");
            result = false ;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("2nd removeByName(): -> OK");
            result &= true;
        }

        tRes.tested("removeByName()", result);

    } // end removeByName()

}    /// finish class XAutoTextContainer


