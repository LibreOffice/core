/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


