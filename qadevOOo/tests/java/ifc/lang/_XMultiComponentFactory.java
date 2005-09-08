/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XMultiComponentFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:15:56 $
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

package ifc.lang;

import com.sun.star.beans.PropertyValue;
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
    private String[] availableServiceNames = null;

    public void before(){
        xContext = (XComponentContext)tEnv.getObjRelation("DC");
        availableServiceNames = (String[])tEnv.getObjRelation("XMultiComponentFactory.ServiceNames");
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
        XInterface component = null;

        try {
            component = (XInterface)oObj.createInstanceWithArgumentsAndContext(
                    availableServiceNames[0], new Object[0], xContext);
            result = (component != null);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance " + availableServiceNames[0]);
            result = false;
        }

        tRes.tested("createInstanceWithArgumentsAndContext()", result);
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is not null.
    */
    public void _getAvailableServiceNames() {
        boolean result = true;
        if (availableServiceNames == null) {
            availableServiceNames = oObj.getAvailableServiceNames();
            result = (availableServiceNames != null);
        }
        else { // if service names are given, ignore result
            String[]erg = oObj.getAvailableServiceNames();
            result = (erg != null);
        }

        log.println("Available service names:");
        for(int i = 0; i < availableServiceNames.length; i++) {
            log.println("   " + availableServiceNames[i]);
        }

        tRes.tested("getAvailableServiceNames()", result);
    }
}

