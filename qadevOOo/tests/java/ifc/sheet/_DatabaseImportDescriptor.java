/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _DatabaseImportDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:35:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
             log.println("Exception occured while testing property '" +
                 name + "'");
             e.printStackTrace(log);
             tRes.tested(name, false);
        }
    }// end of changeProp


}  // finish class _DatabaseImportDescriptor


