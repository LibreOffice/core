/*************************************************************************
 *
 *  $RCSfile: TestAcquire.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-09 09:20:40 $
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

package test.java_uno.acquire;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

public final class TestAcquire {
    // args[0] must be the UNO URL to connect to
    public static void main(String[] args) throws Exception {
        XComponentContext context
            = Bootstrap.createInitialComponentContext(null);
        XUnoUrlResolver resolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
            XUnoUrlResolver.class,
            context.getServiceManager().createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver", context));
        XTest test = (XTest) UnoRuntime.queryInterface(
            XTest.class, resolver.resolve(args[0]));

        // A minimal test for #110444#:
        test.setDerivedToDerived(new XDerived() {});
        test.getDerivedFromDerived();

/*
        // A test that crashes the server, because roundTripDerivedToInterface
        // sends back a "release" for the tuple (someDerived,XInterface) from
        // the client to the server, altough the server did not acquire this
        // tuple when sending it, since earlier the server received the tuple
        // (someDerived,XDerived) from the client (and XDerived is a subtype of
        // XInterface):
        XBase someBase = new XBase() {};
        XDerived someDerived = new XDerived() {};
        test.setAnyToBase(new Any(XBase.class, someBase));
        test.setAnyToBase(new Any(XBase.class, someDerived));
        test.getInterfaceFromBase();
        test.roundTripDerivedToInterface(someDerived);
*/

/*
        // The complete test suite:

        test.setInterfaceToInterface(new XInterface() {});
        test.getInterfaceFromInterface();
        test.setInterfaceToInterface(new XBase() {});
        test.getInterfaceFromInterface();
        test.setInterfaceToInterface(new XDerived() {});
        test.getInterfaceFromInterface();

        test.setBaseToInterface(new XBase() {});
        test.getInterfaceFromInterface();
        test.setBaseToInterface(new XDerived() {});
        test.getInterfaceFromInterface();

        test.setDerivedToInterface(new XDerived() {});
        test.getInterfaceFromInterface();

        test.setAnyToInterface(new XInterface() {});
        test.getInterfaceFromInterface();
        test.setAnyToInterface(new XBase() {});
        test.getInterfaceFromInterface();
        test.setAnyToInterface(new XDerived() {});
        test.getInterfaceFromInterface();
        test.setAnyToInterface(new Any(XBase.class, new XBase() {}));
        test.getInterfaceFromInterface();
        test.setAnyToInterface(new Any(XBase.class, new XDerived() {}));
        test.getInterfaceFromInterface();
        test.setAnyToInterface(new Any(XDerived.class, new XDerived() {}));
        test.getInterfaceFromInterface();

        test.setBaseToBase(new XBase() {});
        test.getInterfaceFromBase();
        test.getBaseFromBase();
        test.setBaseToBase(new XDerived() {});
        test.getInterfaceFromBase();
        test.getBaseFromBase();

        test.setDerivedToBase(new XDerived() {});
        test.getInterfaceFromBase();
        test.getBaseFromBase();

        test.setAnyToBase(new XBase() {});
        test.getInterfaceFromBase();
        test.getBaseFromBase();
        test.setAnyToBase(new XDerived() {});
        test.getInterfaceFromBase();
        test.getBaseFromBase();
        test.setAnyToBase(new Any(XBase.class, new XBase() {}));
        test.getInterfaceFromBase();
        test.getBaseFromBase();
        test.setAnyToBase(new Any(XBase.class, new XDerived() {}));
        test.getInterfaceFromBase();
        test.getBaseFromBase();
        test.setAnyToBase(new Any(XBase.class, new XDerived() {}));
        test.getInterfaceFromBase();
        test.getBaseFromBase();

        test.setDerivedToDerived(new XDerived() {});
        test.getInterfaceFromDerived();
        test.getBaseFromDerived();
        test.getDerivedFromDerived();

        test.setAnyToDerived(new XDerived() {});
        test.getInterfaceFromDerived();
        test.getBaseFromDerived();
        test.getDerivedFromDerived();
        test.setAnyToDerived(new Any(XDerived.class, new XDerived() {}));
        test.getInterfaceFromDerived();
        test.getBaseFromDerived();
        test.getDerivedFromDerived();

        test.setAnyToAny(new XInterface() {});
        test.getAnyFromAny();
        test.setAnyToAny(new XBase() {});
        test.getAnyFromAny();
        test.setAnyToAny(new XDerived() {});
        test.getAnyFromAny();
        test.setAnyToAny(new Any(XBase.class, new XBase() {}));
        test.getAnyFromAny();
        test.setAnyToAny(new Any(XBase.class, new XDerived() {}));
        test.getAnyFromAny();
        test.setAnyToAny(new Any(XDerived.class, new XDerived() {}));
        test.getAnyFromAny();

        test.roundTripInterfaceToInterface(new XInterface() {});
        test.roundTripInterfaceToInterface(new XBase() {});
        test.roundTripInterfaceToInterface(new XDerived() {});

        test.roundTripBaseToInterface(new XBase() {});
        test.roundTripBaseToInterface(new XDerived() {});

        test.roundTripDerivedToInterface(new XDerived() {});

        test.roundTripAnyToInterface(new XInterface() {});
        test.roundTripAnyToInterface(new XBase() {});
        test.roundTripAnyToInterface(new XDerived() {});
        test.roundTripAnyToInterface(new Any(XBase.class, new XBase() {}));
        test.roundTripAnyToInterface(new Any(XBase.class, new XDerived() {}));
        test.roundTripAnyToInterface(new Any(XDerived.class,
                                             new XDerived() {}));

        test.roundTripBaseToBase(new XBase() {});
        test.roundTripBaseToBase(new XDerived() {});

        test.roundTripDerivedToBase(new XDerived() {});

        test.roundTripAnyToBase(new XBase() {});
        test.roundTripAnyToBase(new XDerived() {});
        test.roundTripAnyToBase(new Any(XBase.class, new XBase() {}));
        test.roundTripAnyToBase(new Any(XBase.class, new XDerived() {}));
        test.roundTripAnyToBase(new Any(XDerived.class, new XDerived() {}));

        test.roundTripDerivedToDerived(new XDerived() {});

        test.roundTripAnyToDerived(new XDerived() {});
        test.roundTripAnyToDerived(new Any(XDerived.class, new XDerived() {}));

        test.roundTripAnyToAny(new XInterface() {});
        test.roundTripAnyToAny(new XBase() {});
        test.roundTripAnyToAny(new XDerived() {});
        test.roundTripAnyToAny(new Any(XBase.class, new XBase() {}));
        test.roundTripAnyToAny(new Any(XBase.class, new XDerived() {}));
        test.roundTripAnyToAny(new Any(XDerived.class, new XDerived() {}));
*/

        System.exit(0);
    }
}
