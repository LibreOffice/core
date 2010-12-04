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

package ifc.sheet;

import lib.MultiMethodTest;
import util.ValueChanger;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.table.CellAddress;
import com.sun.star.util.XSortable;

/**
* Testing <code>com.sun.star.sheet.SheetSortDescriptor</code>
* service properties: <p>
* <ul>
*   <li><code>BindFormatsToContent</code></li>
*   <li><code>CopyOutputData</code></li>
*   <li><code>IsCaseSensitive</code></li>
*   <li><code>IsUserListEnabled</code></li>
*   <li><code>OutputPosition</code></li>
*   <li><code>SortAscending</code></li>
*   <li><code>UserListIndex</code></li>
* </ul> <p>
* @see com.sun.star.sheet.SheetSortDescriptor
*/
public class _SheetSortDescriptor extends MultiMethodTest {

    public XPropertySet oObj = null;
    public XSortable xSORT = null;
    public PropertyValue[] props = null;

    public void _BindFormatsToContent() {
        xSORT = (XSortable) tEnv.getObjRelation("xSORT");
        props = xSORT.createSortDescriptor();
        changeProp("BindFormatsToContent",5);
    }

    public void _CopyOutputData() {
        changeProp("CopyOutputData",6);
    }

    public void _IsCaseSensitive() {
        changeProp("IsCaseSensitive",4);
    }

    public void _IsUserListEnabled() {
        changeProp("IsUserListEnabled",8);
    }

    public void _OutputPosition() {
        changeProp("OutputPosition",7);
    }

    public void _SortAscending() {
        //changeProp("SortAscending",3);
        log.println("Property 'SortAscending' is not part of the "+
            "property array");
        log.println("Available properties:");
        for (int i=0; i<props.length;i++) {
            log.println("\t"+props[i].Name);
        }
        tRes.tested("SortAscending",false);
    }

    public void _UserListIndex() {
        changeProp("UserListIndex",9);
    }

    public void changeProp(String name, int nr) {

        Object gValue = null;
        Object sValue = null;
        Object ValueToSet = null;
        int gInt = 0;
        int sInt = 0;

        if ( ! name.equals(props[nr].Name) ) {
            log.println("Property '"+props[nr].Name+"' is tested");
            log.println("But the status is for '"+name+"'");
        }


        try {
            //waitForAllThreads();
            gValue = props[nr].Value;
            if ( name.equals("OutputPosition")) {
                gInt = ((CellAddress) gValue).Row;
            }
            //waitForAllThreads();
            ValueToSet = ValueChanger.changePValue(gValue);
            //waitForAllThreads();
            props[nr].Value=ValueToSet;
            sValue = props[nr].Value;
            if ( name.equals("OutputPosition")) {
                sInt = ((CellAddress) sValue).Row;
                gValue = new Integer(gInt);
                sValue = new Integer(sInt);
            }

            //check get-set methods
            if ( (gValue.equals(sValue)) || (sValue == null) ) {
                log.println("Value for '"+name+"' hasn't changed");
                tRes.tested(name, false);
            }
            else {
                log.println("Property '"+name+"' OK");
                tRes.tested(name, true);
            }
        } catch (Exception e) {
             log.println("Exception occurred while testing property '" +
                 name + "'");
             e.printStackTrace(log);
             tRes.tested(name, false);
        }


    }// end of changeProp


}  // finish class _SheetSortDescriptor


