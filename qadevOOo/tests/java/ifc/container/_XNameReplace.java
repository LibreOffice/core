/*************************************************************************
 *
 *  $RCSfile: _XNameReplace.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:23:48 $
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
import util.ValueComparer;

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameReplace;
import com.sun.star.uno.UnoRuntime;
/**
* Testing <code>com.sun.star.container.XNameReplace</code>
* interface methods :
* <ul>
*  <li><code> replaceByName()</code></li>
* </ul>
* This test needs the following object relations :
* <ul>
*  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> : N relations
*   which represents objects to be replaced with. See below
*   for more information.</li>
*  <li> <code>'NAMEREPLACE'</code> <b>optional</b>: <code>String</code>
*    relation which represents element name to be replaced.
*    Some Objects can't replace the firsr that comes along, i.e.
*    SwXStyleFamily. It have some pool styles which can't be replaced.
*    So the test need a special object to replace it by name. </li>
*  <li> <code>'XNameReplaceINDEX'</code> : For internal test
*   usage. Contains current thread number. </li>
*  <li> Test environment variable <code>'THRCNT'</code> : number
*   of interface threads running concurently. </li>
* <ul> <p>
* XNameReplace needs n ObjectRelations "INSTANCEn" , where n = 1, ..., THRCNT.
* <p>
* When this interface tested by different threads, it must use different instances
* to replace - one for each thread.
* <p>
* That's why we use objRelation "XNameReplaceINDEX" to store the number of last
* taken instance. If there is no such relation, it initialize with 1.
* <p>
* In one of the last steps the replaced object will be compared with the old
* object. For that it is necessary that every thread replace it's own object.
* INSTANCEn are n Objectrelations so that every thread can isert it's own
* object. n depends on the variable THRCNT which and comes from API.INI
* Some Object-Container can't replace the first that comes belong. So in
* NAMEREPLACE you can determine a containerobject, which is replaceable. <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.container.XNameReplace
*/
public class _XNameReplace extends MultiMethodTest {

    public XNameReplace oObj = null;

    /**
    * First test retrieves instance to be replaced with for each interface thread.
    * Then list of element names is retrieved, the first of them will
    * be replaced. In special case when <code>'NAMEREPLACE'</code> relation
    * exists, element with the specified name is replaced.
    * Test replaces element and checks values of element with the
    * specified name before and after replacement. <p>
    * Has <b>OK</b> status if values before and after replacement are
    * different.
    */
    public void _replaceByName(){
        boolean result = true;
        String[] oNames = null;

        int Index = 0;

        //get for every thread its own Object to insert it
        log.println("get ObjRelation(\"XNameReplaceINDEX\")");
        String sIndex = (String)tEnv.getObjRelation("XNameReplaceINDEX");
        System.out.println("Index: "+sIndex);
        if (sIndex == null) {
            log.println("No XNameReplaceINDEX - so set it to 1.");
            tEnv.addObjRelation("XNameReplaceINDEX", Integer.toString(1));
            Index = 1;
        } else {
            Index = Integer.parseInt(sIndex);
            Index++;
            tEnv.addObjRelation("XNameReplaceINDEX", Integer.toString(Index));
        }

        log.println("get ObjRelation(\"INSTANCE" + Index +"\")");
        Object oInstance = tEnv.getObjRelation("INSTANCE"+ Index);
        if (oInstance == null) {
            log.println("ObjRelation(\"INSTANCE" + Index +"\") Object n.a.");
        }

        log.println("getting the existant object's name");
        XNameAccess oNameAccess = (XNameAccess)UnoRuntime.queryInterface(
                                                       XNameAccess.class, oObj);
        oNames = oNameAccess.getElementNames();
        /* Some Objects can't replace the firsr that comes along, i.e.
           SwXStyleFamily. It have some pool styles which can't be replaced.
           So the test need a special object to replace it by name.
        */
        log.println("get ObjRelation(\"NAMEREPLACE\")");
        Object oNameReplace = tEnv.getObjRelation("NAMEREPLACE");
        if (oNameReplace != null) {
            oNames[0] = oNameReplace.toString();
        }


       log.println("replaceByName()");
        try {
            Object old = oObj.getByName(oNames[0]) ;
            oObj.replaceByName(oNames[0],oInstance);
            Object newEl = oObj.getByName(oNames[0]) ;
            result &= ! ValueComparer.equalValue(old, newEl);
        } catch (com.sun.star.lang.IllegalArgumentException e ) {
            result = false;
            e.printStackTrace(log) ;
        } catch (com.sun.star.container.NoSuchElementException e ) {
            result = false;
            e.printStackTrace(log) ;
        } catch (com.sun.star.lang.WrappedTargetException e ) {
            result = false;
            e.printStackTrace(log) ;
        }

        tRes.tested("replaceByName()", result);

    } // end replaceByName()

    /**
    * Forces object environment recreation.
    */
    public void after() {
        disposeEnvironment() ;
    }

}


