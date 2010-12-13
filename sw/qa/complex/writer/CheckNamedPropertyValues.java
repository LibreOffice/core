/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.writer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameContainer;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 *
 */
public class CheckNamedPropertyValues {
    @Test public void checkNamedPropertyValues()
        throws com.sun.star.uno.Exception
    {
        XNameContainer xCont = UnoRuntime.queryInterface(
            XNameContainer.class,
            (connection.getComponentContext().getServiceManager().
             createInstanceWithContext(
                 "com.sun.star.document.NamedPropertyValues",
                 connection.getComponentContext())));

        assertNotNull("XNameContainer was queried but returned null.", xCont);
        PropertyValue[] prop1 = new PropertyValue[1];
        prop1[0] = new PropertyValue();
        prop1[0].Name  = "Jupp";
        prop1[0].Value = "GoodGuy";

        PropertyValue[] prop2 = new PropertyValue[1];
        prop2[0] = new PropertyValue();
        prop2[0].Name  = "Horst";
        prop2[0].Value = "BadGuy";

        Type t = xCont.getElementType();
        assertFalse("Initial container is not empty.", xCont.hasElements());

        xCont.insertByName("prop1", prop1);
        PropertyValue[]ret = (PropertyValue[])xCont.getByName("prop1");
        assertEquals(prop1[0].Name, ret[0].Name);
        assertEquals(prop1[0].Value, ret[0].Value);
        xCont.replaceByName("prop1", prop2);
        ret = (PropertyValue[])xCont.getByName("prop1");
        assertEquals(prop2[0].Name, ret[0].Name);
        assertEquals(prop2[0].Value, ret[0].Value);
        xCont.removeByName("prop1");
        assertFalse("Could not remove PropertyValue.", xCont.hasElements());
        xCont.insertByName("prop1", prop1);
        xCont.insertByName("prop2", prop2);
        assertTrue("Did not insert PropertyValue.", xCont.hasElements());
        String[] names = xCont.getElementNames();
        assertEquals("Not all element names were returned.", 2, names.length);
        for (int i=0; i<names.length; i++) {
            assertTrue(
                "Got a wrong element name",
                names[i].equals("prop1") || names[i].equals("prop2"));
        }

        try {
            xCont.insertByName("prop2", prop1);
            fail("ElementExistException was not thrown.");
        }
        catch(com.sun.star.container.ElementExistException e) {
        }

        try {
            xCont.insertByName("prop3", "Example String");
            fail("IllegalArgumentException was not thrown.");
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
        }

        try {
            xCont.removeByName("prop3");
            fail("NoSuchElementException was not thrown.");
        }
        catch(com.sun.star.container.NoSuchElementException e) {
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
