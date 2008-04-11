/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnoUrlTest.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.lib.uno.helper;
public class UnoUrlTest  {

    private UnoUrlTest() {
    }


    private void fail(String msg) {
        System.err.println(msg);
        System.exit(1);
    }

    private static void log(String msg) {
        System.out.println(msg);
    }

    private void assertTrue(boolean b) {
        if (!b)
            fail("boolean assertion failed");
    }

    private void assertEquals(String expected, String actual) {
        if (!expected.equals(actual)) {
            fail("Expected: '"+ expected + "' but was: '"+actual+"'");
        }
    }

    private void assertEquals(int expected, int actual) {
        if (expected != actual) {
            fail("Expected: "+ expected + " but was: "+actual);
        }
    }

    public void testStart1() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x;y;z");
            assertTrue((url != null));
            assertEquals("x", url.getConnection());
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            fail("Caught exception:" + e.getMessage());
        }
    }

    public void testStart2() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno1:x;y;z");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testStart3() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("un:x;y;z");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testStart4() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("x;y;z");
            assertTrue((url != null));
            assertEquals("y", url.getProtocol());
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            fail("Caught exception:" + e.getMessage());
        }
    }

    public void testParam1() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testParam2() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:a;");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testPartName1() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:abc!abc;b;c");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testOID1() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x;y;ABC<ABC");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testOIDandParams1() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x,key9=val9;y;ABC");
            assertTrue((url != null));
            assertEquals("ABC", url.getRootOid());
            assertEquals(1, url.getConnectionParameters().size());
            assertEquals("val9", (String)url.getConnectionParameters().get("key9"));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        }
    }

    public void testOIDandParams2() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x,key1=val1,k2=v2;y,k3=v3;ABC()!/");
            assertTrue((url != null));
            assertEquals("ABC()!/", url.getRootOid());
            assertEquals(2, url.getConnectionParameters().size());
            assertEquals(1, url.getProtocolParameters().size());
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            fail("Caught exception:" + e.getMessage());
        }
    }

    public void testParams1() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x,abc!abc=val;y;ABC");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testParams2() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x,abc=val<val;y;ABC");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void testParams3() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x,abc=val!()val;y;ABC");
            assertTrue((url != null));
            assertEquals(1, url.getConnectionParameters().size());
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            fail("Caught exception:" + e.getMessage());
        }
    }

    public void testCommon() {
        try {
            UnoUrl url =
                UnoUrl.parseUnoUrl(
                    "socket,host=localhost,port=2002;urp;StarOffice.ServiceManager");
            assertTrue((url != null));
            assertEquals("StarOffice.ServiceManager", url.getRootOid());
            assertEquals("socket", url.getConnection());
            assertEquals("urp", url.getProtocol());
            assertEquals("2002", (String)url.getConnectionParameters().get("port"));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            fail("Caught exception:" + e.getMessage());
        }
    }

    public void testUTF() {
        try {
            UnoUrl url =
                UnoUrl.parseUnoUrl(
                    "socket,host=localhost,horst=abc%c3%9c%c3%a4ABC%41%2c%2C,port=2002;urp;StarOffice.ServiceManager");
            assertEquals("abcÜäABCA,,", (String)url.getConnectionParameters().get("horst"));
            assertEquals(
                "host=localhost,horst=abc%c3%9c%c3%a4ABC%41%2c%2C,port=2002",
                url.getConnectionParametersAsString());
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            fail("Caught exception:" + e.getMessage());
        }

    }

    public void testUTF1() {
        try {
            UnoUrl url = UnoUrl.parseUnoUrl("uno:x,abc=val%4t;y;ABC");
            fail("Should throw an exception");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }


    public static void main(String args[]) {
        UnoUrlTest t = new UnoUrlTest();

        log("Running test case 1");
        t.testStart1();
        log("Running test case 2");
        t.testStart2();
        log("Running test case 3");
        t.testStart3();
        log("Running test case 4");
        t.testStart4();

        log("Running test case 5");
        t.testParam1();
        log("Running test case 6");
        t.testParam2();

        log("Running test case 7");
        t.testPartName1();

        log("Running test case 8");
        t.testOID1();

        log("Running test case 9");
        t.testOIDandParams1();
        log("Running test case 10");
        t.testOIDandParams2();

        log("Running test case 11");
        t.testParams1();
        log("Running test case 12");
        t.testParams2();
        log("Running test case 13");
        t.testParams3();

        log("Running test case 14");
        t.testCommon();

        log("Running test case 15");
        t.testUTF();
        log("Running test case 16");
        t.testUTF1();
    }
}
