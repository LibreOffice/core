/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            gValue = props[nr].Value;
            if ( name.equals("OutputPosition")) {
                gInt = ((CellAddress) gValue).Row;
            }
            ValueToSet = ValueChanger.changePValue(gValue);
            props[nr].Value=ValueToSet;
            sValue = props[nr].Value;
            if ( name.equals("OutputPosition")) {
                sInt = ((CellAddress) sValue).Row;
                gValue = Integer.valueOf(gInt);
                sValue = Integer.valueOf(sInt);
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


