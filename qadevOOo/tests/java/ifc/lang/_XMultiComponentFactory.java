/*************************************************************************
 *
 *  $RCSfile: _XMultiComponentFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:45:32 $
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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.lang.XMultiComponentFactory</code>
* interface methods :
* <ul>
*  <li><code> createInstanceWithContext()</code></li>
*  <li><code> createInstanceWithArgumentsAndContext()</code></li>
*  <li><code> getAvailableServiceNames()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.lang.XMultiComponentFactory
*/
public class _XMultiComponentFactory extends MultiMethodTest {
    public XMultiComponentFactory oObj = null;

    public XComponentContext xContext = null;

    public void before(){
        xContext = (XComponentContext)tEnv.getObjRelation("DC");
    }

    /**
    * Calls the method with one of the available service names
    * obtained by method getAvailableServiceNames. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is not null.
    */
    public void _createInstanceWithContext() {
        requiredMethod("getAvailableServiceNames()");
        boolean result = true;

        try {
            XInterface component = (XInterface)
                oObj.createInstanceWithContext(
                    availableServiceNames[0], xContext);
            result = (component != null);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance " + availableServiceNames[0]);
            result = false;
        }

        tRes.tested("createInstanceWithContext()", result);
    }

    /**
    * Calls the method with one of the available service names
    * obtained by method getAvailableServiceNames. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is not null.
    */
    public void _createInstanceWithArgumentsAndContext() {
        requiredMethod("getAvailableServiceNames()");
        boolean result = true;

        try {
            XInterface component = (XInterface)
                oObj.createInstanceWithArgumentsAndContext(
                    availableServiceNames[0], new Object[0], xContext);
            result = (component != null);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance " + availableServiceNames[0]);
            result = false;
        }

        tRes.tested("createInstanceWithArgumentsAndContext()", result);
    }

    String[] availableServiceNames;

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is not null.
    */
    public void _getAvailableServiceNames() {
        availableServiceNames = oObj.getAvailableServiceNames();

        log.println("Available service names:");
        for(int i = 0; i < availableServiceNames.length; i++) {
            log.println("   " + availableServiceNames[i]);
        }

        tRes.tested("getAvailableServiceNames()", availableServiceNames != null);
    }
}

