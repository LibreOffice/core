/*************************************************************************
 *
 *  $RCSfile: _XViewDataSupplier.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:54:29 $
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
package ifc.document;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexContainer;
import com.sun.star.document.XViewDataSupplier;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;
import lib.Status;

/**
 * Check the XViewDataSupplier interface.
 * Test idea: take the property values from the index access, change one
 * property value, put this into the index access and write it back.
 * Get the property value again and check that the change made it.
 */
public class _XViewDataSupplier extends MultiMethodTest {
    public XViewDataSupplier oObj = null;
    XIndexAccess xAccess = null;
    PropertyValue[] newProps = null;
    PropertyValue[] oldProps = null;
    String myview = "myview1";

    public void _getViewData() {
        xAccess = oObj.getViewData();
//        util.dbg.printInterfaces(xAccess);
        if (xAccess != null) {
            setViewID(xAccess, myview);
        }
        tRes.tested("getViewData()", true);
    }

    public void _setViewData() {
        if (xAccess == null) {
            log.println("No view data to change available");
            tRes.tested("setViewData()", Status.skipped(true));
        }
        else {
            // 2do: provide an own implementation of the XIndexAccess to set.
            // this will work without "setViewData()", it just checks that a
            // setViewData can be done.
            oObj.setViewData(xAccess);
            XIndexAccess xAccess2 = oObj.getViewData();
            String newView = getViewID(xAccess2);
            tRes.tested("setViewData()", newView.equals(myview));
        }
    }

    private void setViewID(XIndexAccess xAccess, String value) {
        XIndexContainer xIndexContainer = (XIndexContainer)UnoRuntime.queryInterface(XIndexContainer.class, xAccess);
        int count = xAccess.getCount();
        try {
            if (count > 0) {
                oldProps = (PropertyValue[])xAccess.getByIndex(0);
                newProps = new PropertyValue[oldProps.length];
                for (int j=0; j<oldProps.length; j++) {
//                    log.println("Name: " + oldProps[j].Name);
//                    log.println("Value: " + oldProps[j].Value.toString());
                    newProps[j] = new PropertyValue();
                    newProps[j].Name = oldProps[j].Name;
                    newProps[j].Handle = oldProps[j].Handle;
                    newProps[j].State = oldProps[j].State;
                    if (oldProps[j].Name.equals("ViewId")) {
                        newProps[j].Value = value;
                    }

                }
                xIndexContainer.insertByIndex(0, newProps);
            }
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
        }
    }

    private String getViewID(XIndexAccess xAccess) {
        String retValue = null;
        int count = xAccess.getCount();
        try {
            if (count > 0) {
                oldProps = (PropertyValue[])xAccess.getByIndex(0);
                for (int j=0; j<oldProps.length; j++) {
//                    log.println("Name: " + oldProps[j].Name);
//                    log.println("Value: " + oldProps[j].Value.toString());
                    if (oldProps[j].Name.equals("ViewId")) {
                        retValue = (String)newProps[j].Value;
                    }

                }
            }
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
        }
        return retValue;
    }
}
