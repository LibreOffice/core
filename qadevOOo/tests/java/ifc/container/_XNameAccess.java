/*************************************************************************
 *
 *  $RCSfile: _XNameAccess.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:23:28 $
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

package ifc.container;

import lib.MultiMethodTest;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameAccess;

/**
* Testing <code>com.sun.star.container.XNameAccess</code> interface methods. <p>
* Test is <b> NOT </b> multithread compilant. <p>
*/
public class _XNameAccess extends MultiMethodTest {
    public XNameAccess oObj = null;
    public String[] Names = null;

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method successfully returns
    * not null value and no exceptions were thrown. <p>
    */
    public void _getElementNames() {
        boolean result = true;
        log.println("getting elements names");
        Names = oObj.getElementNames();

        result = (Names != null);
        tRes.tested("getElementNames()", result);
        return;
    } // end getElementNames()

    /**
    * First test calls the method with existing element name,
    * then with non existing. <p>
    * Has <b> OK </b> status if in the first case the method returns
    * true and in the second - false. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getElementNames </code> : to retrieve at least one
    *    element name. </li>
    * </ul>
    */
    public void _hasByName() {
        requiredMethod("getElementNames()");
        log.println("testing hasByName() ...");

        boolean result = true;
        boolean loc_result = true;

        String name = null;

    if (Names.length != 0) {
            log.println("testing hasByName() with valid name");
            name = Names[0];
            loc_result = oObj.hasByName(name);
            log.println("hasByName with valid names: " + loc_result);
            result &= loc_result;
    }

    name = "non_existant_name__1234";
    log.println("testing hasByName() with invalid name");
        try {
            loc_result = !oObj.hasByName(name);
        } catch ( Exception nsee) {
            log.println("Expected exception was thrown");
        }
        log.println("hasByName with invalid names: " + loc_result);
    result &= loc_result;

    tRes.tested("hasByName()", result);

        return;
    } // end hasByName()


    /**
    * First test calls the method with existing element name,
    * then with non existing. <p>
    * Has <b> OK </b> status if in the first case the method returns
    * not null value and no exceptions were thrown,
    * and in the second case <code>NoSuchElementException</code> was
    * thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getElementNames </code> : to retrieve at least one
    *    element name. </li>
    * </ul>
    */
    public void _getByName() {
        log.println("reqiure getElementNames() ...");
        requiredMethod("getElementNames()");
        log.println("require getElementNames() ...OK");
        log.println("testing getByName() ...");

        boolean result = true;
        boolean loc_result = true;

        String name = null;

        if (Names.length != 0) {
            log.println("testing with valid name");
            name = Names[0];
            try {
                loc_result = (null != oObj.getByName(name));
            } catch (Exception e) {
                log.println("Exception! - FAILED");
                loc_result = false;
            }
            log.println("getByName with valid name: " + loc_result);
            result &= loc_result;
        }

        log.println("testing with non-existant name");
        name = "non_existant_name__1234";
        try {
            loc_result = (null != oObj.getByName(name));
            loc_result = false;
            log.println("getByName: Exception expected - FAILED");
        } catch (NoSuchElementException e) {
            log.println("getByName: Expected exception - OK");
            loc_result = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("getByName: Wrong exception - " + e + " - FAILED");
            loc_result = false;
        }

        result &= loc_result;
        tRes.tested("getByName()", result);

        return;

    } // end getByName()
} /// finished class _XNameAccess



