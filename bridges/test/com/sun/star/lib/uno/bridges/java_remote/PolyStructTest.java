/*************************************************************************
 *
 *  $RCSfile: PolyStructTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:04:49 $
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

package com.sun.star.lib.uno.bridges.javaremote;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.ParameterTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

public final class PolyStructTest extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        assure(
            "test",
            new TestBed().execute(new Provider(), false, Client.class, 0));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XBridge bridge) throws Throwable {
            TestTransport t = (TestTransport) UnoRuntime.queryInterface(
                TestTransport.class, bridge.getInstance(""));

            assertEquals(
                Boolean.FALSE, t.transportBoolean(new TestPolyStruct()).member);
            assertEquals(
                Boolean.FALSE,
                t.transportBoolean(new TestPolyStruct(Boolean.FALSE)).member);
            assertEquals(
                Boolean.TRUE,
                t.transportBoolean(new TestPolyStruct(Boolean.TRUE)).member);

            assertEquals(
                new Byte((byte) 0),
                t.transportByte(new TestPolyStruct()).member);
            assertEquals(
                new Byte(Byte.MIN_VALUE),
                t.transportByte(
                    new TestPolyStruct(new Byte(Byte.MIN_VALUE))).member);
            assertEquals(
                new Byte(Byte.MAX_VALUE),
                t.transportByte(
                    new TestPolyStruct(new Byte(Byte.MAX_VALUE))).member);

            assertEquals(
                new Short((short) 0),
                t.transportShort(new TestPolyStruct()).member);
            assertEquals(
                new Short(Short.MIN_VALUE),
                t.transportShort(
                    new TestPolyStruct(new Short(Short.MIN_VALUE))).member);
            assertEquals(
                new Short(Short.MAX_VALUE),
                t.transportShort(
                    new TestPolyStruct(new Short(Short.MAX_VALUE))).member);

            assertEquals(
                new Short((short) 0),
                t.transportUnsignedShort(new TestPolyStruct()).member);
            assertEquals(
                new Short((short) 0),
                t.transportUnsignedShort(
                    new TestPolyStruct(new Short((short) 0))).member);
            assertEquals(
                new Short((short) 0xFFFF),
                t.transportUnsignedShort(
                    new TestPolyStruct(new Short((short) 0xFFFF))).member);

            assertEquals(
                new Integer(0), t.transportLong(new TestPolyStruct()).member);
            assertEquals(
                new Integer(Integer.MIN_VALUE),
                t.transportLong(
                    new TestPolyStruct(new Integer(Integer.MIN_VALUE))).member);
            assertEquals(
                new Integer(Integer.MAX_VALUE),
                t.transportLong(
                    new TestPolyStruct(new Integer(Integer.MAX_VALUE))).member);

            assertEquals(
                new Integer(0),
                t.transportUnsignedLong(new TestPolyStruct()).member);
            assertEquals(
                new Integer(0),
                t.transportUnsignedLong(
                    new TestPolyStruct(new Integer(0))).member);
            assertEquals(
                new Integer(0xFFFFFFFF),
                t.transportUnsignedLong(
                    new TestPolyStruct(new Integer(0xFFFFFFFF))).member);

            assertEquals(
                new Long(0L), t.transportHyper(new TestPolyStruct()).member);
            assertEquals(
                new Long(Long.MIN_VALUE),
                t.transportHyper(
                    new TestPolyStruct(new Long(Long.MIN_VALUE))).member);
            assertEquals(
                new Long(Long.MAX_VALUE),
                t.transportHyper(
                    new TestPolyStruct(new Long(Long.MAX_VALUE))).member);

            assertEquals(
                new Long(0L),
                t.transportUnsignedHyper(new TestPolyStruct()).member);
            assertEquals(
                new Long(0),
                t.transportUnsignedHyper(
                    new TestPolyStruct(new Long(0))).member);
            assertEquals(
                new Long(0xFFFFFFFFFFFFFFFFL),
                t.transportUnsignedHyper(
                    new TestPolyStruct(
                        new Long(0xFFFFFFFFFFFFFFFFL))).member);

            assertEquals(
                new Float(0.0f), t.transportFloat(new TestPolyStruct()).member);
            assertEquals(
                new Float(Float.MIN_VALUE),
                t.transportFloat(
                    new TestPolyStruct(new Float(Float.MIN_VALUE))).member);
            assertEquals(
                new Float(Float.MAX_VALUE),
                t.transportFloat(
                    new TestPolyStruct(new Float(Float.MAX_VALUE))).member);

            assertEquals(
                new Double(0.0),
                t.transportDouble(new TestPolyStruct()).member);
            assertEquals(
                new Double(Double.MIN_VALUE),
                t.transportDouble(
                    new TestPolyStruct(new Double(Double.MIN_VALUE))).member);
            assertEquals(
                new Double(Double.MAX_VALUE),
                t.transportDouble(
                    new TestPolyStruct(new Double(Double.MAX_VALUE))).member);

            assertEquals(
                new Character(Character.MIN_VALUE),
                t.transportChar(new TestPolyStruct()).member);
            assertEquals(
                new Character(Character.MIN_VALUE),
                t.transportChar(
                    new TestPolyStruct(
                        new Character(Character.MIN_VALUE))).member);
            assertEquals(
                new Character(Character.MAX_VALUE),
                t.transportChar(
                    new TestPolyStruct(
                        new Character(Character.MAX_VALUE))).member);

            assertEquals("", t.transportString(new TestPolyStruct()).member);
            assertEquals(
                "ABC", t.transportString(new TestPolyStruct("ABC")).member);

            assertEquals(
                Type.VOID, t.transportType(new TestPolyStruct()).member);
            assertEquals(
                new Type(
                    "[]com.sun.star.lib.uno.bridges.javaremote.TestPolyStruct"
                    + "<long>"),
                t.transportType(
                    new TestPolyStruct(
                        new Type(
                            "[]com.sun.star.lib.uno.bridges.javaremote."
                            + "TestPolyStruct<long>"))).member);

            assertEquals(null, t.transportAny(new TestPolyStruct()).member);
            assertEquals(
                Any.VOID, t.transportAny(new TestPolyStruct(Any.VOID)).member);
            assertEquals(null, t.transportAny(new TestPolyStruct(null)).member);
            assertEquals(
                new Any(Type.UNSIGNED_LONG, new Integer(5)),
                t.transportAny(
                    new TestPolyStruct(
                        new Any(Type.UNSIGNED_LONG, new Integer(5)))).member);

            assertEquals(
                TestEnum.VALUE1, t.transportEnum(new TestPolyStruct()).member);
            assertEquals(
                TestEnum.VALUE1,
                t.transportEnum(new TestPolyStruct(TestEnum.VALUE1)).member);
            assertEquals(
                TestEnum.VALUE2,
                t.transportEnum(new TestPolyStruct(TestEnum.VALUE2)).member);

            return success;
        }

        private void assertEquals(Object expected, Object actual) {
            if (!(expected == null ? actual == null : expected.equals(actual)))
            {
                new RuntimeException(
                    "failed; expected " + expected + ", got " + actual).
                    printStackTrace();
                success = false;
            }
        }

        private boolean success = true;
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new TestTransport() {
                    public TestPolyStruct transportBoolean(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportByte(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportShort(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportUnsignedShort(
                        TestPolyStruct s)
                    {
                        return s;
                    }

                    public TestPolyStruct transportLong(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportUnsignedLong(
                        TestPolyStruct s)
                    {
                        return s;
                    }

                    public TestPolyStruct transportHyper(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportUnsignedHyper(
                        TestPolyStruct s)
                    {
                        return s;
                    }

                    public TestPolyStruct transportFloat(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportDouble(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportChar(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportString(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportType(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportAny(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportEnum(TestPolyStruct s) {
                        return s;
                    }
                };
        }
    }
}
