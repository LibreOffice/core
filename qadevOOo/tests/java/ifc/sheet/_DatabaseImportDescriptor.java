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
import com.sun.star.util.XImportable;

/**
* Testing <code>com.sun.star.sheet.DatabaseImportDescriptor</code>
* service properties:
* <ul>
*   <li><code>DatabaseName</code></li>
*   <li><code>SourceObject</code></li>
*   <li><code>SourceType</code></li>
* </ul> <p>
* @see com.sun.star.sheet.DatabaseImportDescriptor
*/
public class _DatabaseImportDescriptor extends MultiMethodTest {

    public XPropertySet oObj = null;
    public XImportable xIMP = null;
    public PropertyValue[] props = null;

    public void _DatabaseName() {
        xIMP = (XImportable) tEnv.getObjRelation("xIMP");
        props = xIMP.createImportDescriptor(true);
        changeProp("DatabaseName",0);
    }

    public void _SourceObject() {
        changeProp("SourceObject",2);
    }

    public void _SourceType() {
        changeProp("SourceType",1);
    }

    public void changeProp(String name, int nr) {

        Object gValue = null;
        Object sValue = null;
        Object ValueToSet = null;

        if ( ! name.equals(props[nr].Name) ) {
            log.println("Property '"+props[nr].Name+"' is tested");
            log.println("But the status is for '"+name+"'");
        }


        try {
            //waitForAllThreads();
            gValue = props[nr].Value;
            //waitForAllThreads();
            ValueToSet = ValueChanger.changePValue(gValue);
            //waitForAllThreads();
            props[nr].Value=ValueToSet;
            sValue = props[nr].Value;

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


}  // finish class _DatabaseImportDescriptor


