/*************************************************************************
 *
 *  $RCSfile: _XNamed.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:23:58 $
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
import util.utils;

import com.sun.star.container.XNamed;

/**
* Testing <code>com.sun.star.container.XNamed</code>
* interface methods :
* <ul>
*  <li><code> getName()</code></li>
*  <li><code> setName()</code></li>
* </ul>
* This test need the following object relations :
* <ul>
*  <li> <code>'setName'</code> : of <code>Boolean</code>
*    type. If it exists then <code>setName</code> method
*    isn't to be tested and result of this test will be
*    equal to relation value.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.container.XNamed
*/
public class _XNamed extends MultiMethodTest {

    public XNamed oObj = null;    // oObj filled by MultiMethodTest

    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if the method returns non null value
    * and no exceptions were thrown. <p>
    */
    public void _getName() {

        // write to log what we try next
        log.println( "test for getName()" );

        boolean result = true;
        boolean loc_result = true;
        String name = null;

        loc_result = ((name = oObj.getName()) != null);
        log.println("getting the name \"" + name + "\"");

        if (loc_result) log.println("... getName() - OK");
        else log.println("... getName() - FAILED");
        result &= loc_result;
        tRes.tested("getName()", result);
    }

    /**
    * Sets a new name for object and checks if it was properly
    * set. Special cases for the following objects :
    * <ul>
    *  <li><code>ScSheetLinkObj</code> : name must be in form of URL.</li>
    *  <li><code>ScDDELinkObj</code> : name must contain link to cell in
    *     some external Sheet.</li>
    * </ul>
    * Has <b> OK </b> status if new name was successfully set, or if
    * object environment contains relation <code>'setName'</code> with
    * value <code>true</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getName() </code> : to be sure the method works</li>
    * </ul>
    */
    public void _setName(){
        String Oname = tEnv.getTestCase().getObjectName();
        String nsn = (String) tEnv.getObjRelation("NoSetName");
        if (nsn != null) {
            Oname = nsn;
        }
        if (Oname.indexOf("Exporter")>0 || nsn != null) {
            log.println("With "+Oname+" setName() doesn't work");
            log.println("see idl-file for further information");
            tRes.tested("setName()",true);
            return;
        }
        requiredMethod("getName()");
        log.println("testing setName() ... ");

        String oldName = oObj.getName();
        String NewName = oldName == null ? "XNamed" : oldName + "X" ;

        String testobjname = tEnv.getTestCase().getObjectName();
        if (testobjname.equals("ScSheetLinkObj")) {
            // special case, here name is equals to links URL.
            NewName = "file:///c:/somename/from/XNamed";
        } else if (testobjname.equals("ScDDELinkObj")) {
            String fileName = utils.getFullTestDocName("ScDDELinksObj.sdc");
            NewName = "soffice|" + fileName + "!Sheet1.A2";
        } else if (testobjname.equals("SwXAutoTextGroup")) {
            //This avoids a GPF
            NewName = "XNamed";
        }

        boolean result = true;
        boolean loc_result = true;
        Boolean sName = (Boolean) tEnv.getObjRelation("setName");
        if (sName == null) {
            log.println("set the name of object to \"" + NewName + "\"");
            oObj.setName(NewName);
            log.println("check that container has element with this name");

            String name = oObj.getName();
            log.println("getting the name \"" + name + "\"");
            loc_result = name.equals(NewName);

            if (loc_result) log.println("... setName() - OK");
                            else log.println("... setName() - FAILED");
            result &= loc_result;
        }
        else {
            log.println("The names for the object '"+testobjname+
                "' are fixed.");
            log.println("It is not possible to rename.");
            log.println("So 'setName()' is always OK");
            result = sName.booleanValue();
        }
        tRes.tested("setName()", result);
        oObj.setName(oldName);
        return;
    }

    /**
    * Does nothing.
    */
    protected void after() {
        //disposeEnvironment();
    }

}


