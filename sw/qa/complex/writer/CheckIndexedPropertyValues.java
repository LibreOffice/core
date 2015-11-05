/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package complex.writer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexContainer;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * Test the com.sun.star.document.IndexedPropertyValues service
 */
public class CheckIndexedPropertyValues {
    @Test public void checkIndexedPropertyValues()
        throws com.sun.star.uno.Exception
    {
        Object instance = connection.getComponentContext().getServiceManager().
                createInstanceWithContext(
                        "com.sun.star.document.IndexedPropertyValues",
                        connection.getComponentContext());
        XIndexContainer xCont = UnoRuntime.queryInterface(XIndexContainer.class, instance);

        assertNotNull("XIndexContainer was queried but returned null.", xCont);
        PropertyValue[] prop1 = new PropertyValue[1];
        prop1[0] = new PropertyValue();
        prop1[0].Name  = "Jupp";
        prop1[0].Value = "GoodGuy";

        PropertyValue[] prop2 = new PropertyValue[1];
        prop2[0] = new PropertyValue();
        prop2[0].Name  = "Horst";
        prop2[0].Value = "BadGuy";

        PropertyValue[] prop3 = new PropertyValue[1];
        prop3[0] = new PropertyValue();
        prop3[0].Name  = "Peter";
        prop3[0].Value = "FamilyGuy";

        Type t = xCont.getElementType();
        assertEquals("Initial container is not empty", 0, xCont.getCount());
        xCont.insertByIndex(0, prop1);
        PropertyValue[]ret = (PropertyValue[])xCont.getByIndex(0);
        assertEquals(prop1[0].Name, ret[0].Name);
        assertEquals(prop1[0].Value, ret[0].Value);
        xCont.replaceByIndex(0, prop2);
        ret = (PropertyValue[])xCont.getByIndex(0);
        assertEquals(prop2[0].Name, ret[0].Name);
        assertEquals(prop2[0].Value, ret[0].Value);
        xCont.removeByIndex(0);
        assertTrue("Could not remove PropertyValue.",
                   !xCont.hasElements() && xCont.getCount()==0);
        xCont.insertByIndex(0, prop1);
        xCont.insertByIndex(1, prop2);
        assertTrue("Did not insert PropertyValue.",
                   xCont.hasElements() && xCont.getCount()==2);
        try {
            xCont.removeByIndex(1);
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            fail("Could not remove last PropertyValue");
        }
        xCont.insertByIndex(1, prop2);
        xCont.insertByIndex(1, prop3);
        ret = (PropertyValue[])xCont.getByIndex(1);
        assertEquals(prop3[0].Name, ret[0].Name);
        assertEquals(prop3[0].Value, ret[0].Value);

        try {
            xCont.insertByIndex(25, prop2);
            fail("IllegalArgumentException was not thrown.");
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        try {
            xCont.removeByIndex(25);
            fail("IndexOutOfBoundsException was not thrown.");
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        try {
            xCont.insertByIndex(3, "Example String");
            fail("IllegalArgumentException was not thrown.");
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}
