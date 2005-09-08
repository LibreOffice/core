/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _SheetSortDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:38:48 $
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
             log.println("Exception occured while testing property '" +
                 name + "'");
             e.printStackTrace(log);
             tRes.tested(name, false);
        }


    }// end of changeProp


}  // finish class _SheetSortDescriptor


