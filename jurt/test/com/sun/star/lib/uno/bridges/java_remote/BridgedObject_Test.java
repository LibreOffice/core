/*************************************************************************
 *
 *  $RCSfile: BridgedObject_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-03-25 11:04:23 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

public final class BridgedObject_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() {
        RequestHandler handler = new RequestHandler() {
                public Object sendRequest(
                    String oid, Type type, String operation, Object[] args)
                {
                    return null;
                }
            };
        XBridge bridge1 = new TestBridge();
        ProxyFactory factory1 = new ProxyFactory(handler, bridge1);
        XBridge bridge2 = new TestBridge();
        ProxyFactory factory2 = new ProxyFactory(handler, bridge2);
        Object object0 = new Object();
        Object object1 = factory1.create("", new Type(XInterface.class));
        Object object2 = factory2.create("", new Type(XInterface.class));
        assure(BridgedObject.getBridge(object0) == null);
        assure(BridgedObject.getBridge(object1) == bridge1);
        assure(BridgedObject.getBridge(object2) == bridge2);
    }

    private static final class TestBridge implements XBridge {
        public Object getInstance(String instanceName) {
            return null;
        }

        public String getName() {
            return null;
        }

        public String getDescription() {
            return null;
        }
    }
}
