/*************************************************************************
 *
 *  $RCSfile: _XProxyFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:49:11 $
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

package ifc.reflection;

import lib.MultiMethodTest;

import com.sun.star.lang.XInitialization;
import com.sun.star.reflection.XProxyFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XAggregation;

/**
/**
* Testing <code>com.sun.star.reflection.XProxyFactory</code>
* interface methods :
* <ul>
*  <li><code> createProxy()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.reflection.XProxyFactory
*/
public class _XProxyFactory extends MultiMethodTest {
    /** Is initialized in super class(using reflection API)
     * when instantiating the test.
     */
    public XProxyFactory oObj;

    /**
    * First an implementation of
    * <code>com.sun.star.lang.XInitialization</code> interface
    * is made which sets a flag when its <code>initialize()</code>
    * method is called. Then an instance of this implementation
    * is created and a proxy object is created for it. Proxy
    * object is tried to query for <code>XInitialization</code>
    * interface and it's <code>initialize</code> method is
    * called. The goal is to check if the real object method
    * was called throwgh it's proxy. <p>
    * Has <b>OK</b> status if the real object method was
    * called and paramters were passed correctly.
    */
    public void _createProxy() {
        class MyObject implements XInitialization {
            Object[] params;

            public void initialize(Object args[]) {
                params = args;
            }
        }

        MyObject obj = new MyObject();

        XAggregation xAggr = oObj.createProxy(obj);

        XInitialization xInit = (XInitialization)UnoRuntime.queryInterface(
                XInitialization.class, xAggr);

        Object params[] = new Object[0];

        try {
            xInit.initialize(params);
        } catch(com.sun.star.uno.Exception e) {
            log.println("Unexpected exception : " + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("createProxy()", false);
            return;
        }

        tRes.tested("createProxy()",
            util.ValueComparer.equalValue(params, obj.params));
    }
}

