/*************************************************************************
 *
 *  $RCSfile: _XChild.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:21:46 $
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
import lib.Status;

import com.sun.star.container.XChild;
import com.sun.star.container.XNamed;
import com.sun.star.uno.UnoRuntime;

/*
* Testing <code>com.sun.star.container.XChild</code>
* interface methods :
* <ul>
*  <li><code> getParent()</code></li>
*  <li><code> setParent()</code></li>
* </ul>
* @see com.sun.star.container.XChild
*/
public class _XChild extends MultiMethodTest {

    public XChild oObj = null;
    public Object gotten = null;


    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. Parent returned is stored.<p>
    * Has <b> OK </b> status if the method returns not null value
    * and no exceptions were thrown. <p>
    */
    public void _getParent() {
        gotten = oObj.getParent();
        XNamed the_name = (XNamed) UnoRuntime.queryInterface(XNamed.class,gotten);
        if (the_name != null)
            log.println("Parent:"+the_name.getName());
        tRes.tested("getParent()",gotten != null);
    }

    /**
    * Sets existing parent and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getParent() </code> : to get the parent. </li>
    * </ul>
    */
    public void _setParent() {
        requiredMethod("getParent()") ;

        String parentComment = (String) tEnv.getObjRelation("cannotSwitchParent");

        if (parentComment != null) {
            log.println(parentComment);
            tRes.tested("setParent()",Status.skipped(true));
            return;
        }

        try {
            oObj.setParent(gotten);
            tRes.tested("setParent()",true);
        }
        catch (com.sun.star.lang.NoSupportException ex) {
            log.println("Exception occured during setParent()");
            ex.printStackTrace(log);
            tRes.tested("setParent()",false);
        }

    }

}  // finish class _XChild


