/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PolyStructTest.java,v $
 * $Revision: 1.6 $
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

package com.sun.star.lib.uno.bridges.javaremote;

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
import com.sun.star.uno.XComponentContext;
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

        protected boolean run(XComponentContext context) throws Throwable {
            TestTransport t = UnoRuntime.queryInterface(
                TestTransport.class, getBridge(context).getInstance(""));

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

                    public TestPolyStruct transportLong(TestPolyStruct s) {
                        return s;
                    }

                    public TestPolyStruct transportHyper(TestPolyStruct s) {
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
