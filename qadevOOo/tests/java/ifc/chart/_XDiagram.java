/*************************************************************************
 *
 *  $RCSfile: _XDiagram.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:20:20 $
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

package ifc.chart;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XDiagram;

/**
* Testing <code>com.sun.star.chart.XDiagram</code>
* interface methods :
* <ul>
*  <li><code> getDiagramType()</code></li>
*  <li><code> getDataRowProperties()</code></li>
*  <li><code> getDataPointProperties()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ROWAMOUNT'</code> (of type <code>Integer</code>):
*   to have amount of rows </li>
*  <li> <code>'COLAMOUNT'</code> (of type <code>Integer</code>):
*   to have amount of columns </li>
* <ul> <p>
* @see com.sun.star.chart.XDiagram
*/
public class _XDiagram extends MultiMethodTest {

    public XDiagram      oObj = null;
    boolean         result = true;
    Integer      rowamount = null;
    Integer      colamount = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        rowamount = (Integer)tEnv.getObjRelation("ROWAMOUNT");
        if (rowamount == null) throw new StatusException(Status.failed
            ("Relation 'ROWAMOUNT' not found"));

        colamount = (Integer)tEnv.getObjRelation("COLAMOUNT");
        if (colamount == null) throw new StatusException(Status.failed
            ("Relation 'COLAMOUNT' not found"));
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value start from 'com.sun.star.chart.' <p>
    */
    public void _getDiagramType() {
        result = true;

        String stype = oObj.getDiagramType();
        log.println("Current Diagram Type is " + stype);
        result = (stype.startsWith("com.sun.star.chart."));

        tRes.tested("getDiagramType()", result);
    }

    /**
    * Test calls the method for every row and checks returned value. <p>
    * Has <b> OK </b> status if returned value for every row isn't null and
    * no exceptions were thrown. <p>
    */
    public void _getDataRowProperties() {
        result = true;

        int rows = rowamount.intValue();
        rows -= 1;
        XPropertySet props = null;

        log.println("There are " + rows + " rows.");
        try {
            for (int i = 0; i < rows; i++) {
                props = oObj.getDataRowProperties(i);
                if (props != null) {
                    log.println("Row " + i + " - OK");
                } else {
                    log.println("Row " + i + " - FAILED");
                    result = false;
                }
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception while get data row properties");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("getDataRowProperties()", result);
    }

    /**
    * Test calls the method for every point and checks returned value. <p>
    * Has <b> OK </b> status if returned value for every point isn't null and
    * no exceptions were thrown. <p>
    */
    public void _getDataPointProperties() {
        result = true;

        int rows = rowamount.intValue();
        int cols = colamount.intValue();
        XPropertySet props = null;

        log.println("There are " + rows + " rows and " + cols + " cols.");

        try {
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++) {
                    props = oObj.getDataPointProperties(i, j);
                    if (props != null) {
                        log.println("Row " + i + " Col " + j + " - OK");
                    } else {
                        log.println("Row " + i + " Col " + j + " - FAILED");
                        result = false;
                    }
                }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception while get data point properties");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("getDataPointProperties()", result);
    }
}


