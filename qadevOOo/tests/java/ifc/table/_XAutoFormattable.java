/*************************************************************************
 *
 *  $RCSfile: _XAutoFormattable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:09:49 $
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
package ifc.table;

import java.util.Random;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XAutoFormattable;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
* Testing <code>com.sun.star.table.XAutoFormattable</code>
* interface methods :
* <ul>
*  <li><code> autoFormat()</code></li>
* </ul> <p>
* The component tested <b>must implement</b> interface
* <code>com.sun.star.table.XCellRange</code>. <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.table.XAutoFormattable
*/
public class _XAutoFormattable extends MultiMethodTest {
    public XAutoFormattable oObj = null;

    /**
    * First 'Default' autoformat is set and a background of a cell
    * is obtained. Then any other autoformat is set and background
    * of a cell is obtained again.<p>
    * Has <b> OK </b> status if backgrounds with different autoformat
    * settings are differ. <p>
    */
    public void _autoFormat() {
        boolean bResult = true;
        XMultiServiceFactory oMSF = (XMultiServiceFactory) tParam.getMSF();
        String name = "Default";

        try {
            oObj.autoFormat(name); // applying default format

            // getting current background of the cell
            XCellRange cellRange = (XCellRange) UnoRuntime.queryInterface(
                                           XCellRange.class, oObj);
            XCell oCell = cellRange.getCellByPosition(0, 0);
            XPropertySet PS = (XPropertySet) UnoRuntime.queryInterface(
                                      XPropertySet.class, oCell);

            Integer bkgrnd1;
            try {
                bkgrnd1 = (Integer) PS.getPropertyValue("CellBackColor");
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                bkgrnd1 = (Integer) PS.getPropertyValue("BackColor");
            }

            // getting formats names.
            XInterface iFormats = (XInterface) oMSF.createInstance(
                                          "com.sun.star.sheet.TableAutoFormats");
            XNameAccess formats = (XNameAccess) UnoRuntime.queryInterface(
                                          XNameAccess.class, iFormats);
            String[] names = formats.getElementNames();

            // getting one random not default style name
            Random rnd = new Random();

            if (names.length > 1) {
                while (name.equals("Default")) {
                    name = names[rnd.nextInt(names.length)];
                }
            } else {
                name = names[0];
            }

            log.println("Applying style " + name);


            // applying style
            oObj.autoFormat(name);

            // getting new cell's backround.
            Integer bkgrnd2;
            try {
                bkgrnd2 = (Integer) PS.getPropertyValue("CellBackColor");
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                bkgrnd2 = (Integer) PS.getPropertyValue("BackColor");
            }

            bResult &= !bkgrnd1.equals(bkgrnd2);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured :");
            e.printStackTrace(log);
            bResult = false;
        }

        tRes.tested("autoFormat()", bResult);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }
}