/*************************************************************************
 *
 *  $RCSfile: Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:15:30 $
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

package test.codemaker.javamaker;

import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.uno.Any;
import com.sun.star.uno.DeploymentException;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import complexlib.ComplexTestCase;
import test.codemaker.javamaker.Enum1;
import test.codemaker.javamaker.Enum2;
import test.codemaker.javamaker.PolyStruct;
import test.codemaker.javamaker.S2;
import test.codemaker.javamaker.Struct2;
import test.codemaker.javamaker.singleton_abstract;

public final class Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] {
            "testEnum1", "testEnum2", "testPolyStruct", "testEmptyStruct2",
            "testFullStruct2", "testS1", "testS2", "testSingletons" };
    }

    public void before() throws Exception {
        context = Bootstrap.createInitialComponentContext(null);
    }

    public void testEnum1() {
        assure(Enum1.VALUE1.getValue() == -100);
        assure(Enum1.VALUE2.getValue() == 100);
        assure(Enum1.VALUE1_value == -100);
        assure(Enum1.VALUE2_value == 100);
        assure(Enum1.getDefault() == Enum1.VALUE1);
        assure(Enum1.fromInt(-101) == null);
        assure(Enum1.fromInt(-100) == Enum1.VALUE1);
        assure(Enum1.fromInt(-99) == null);
        assure(Enum1.fromInt(0) == null);
        assure(Enum1.fromInt(99) == null);
        assure(Enum1.fromInt(100) == Enum1.VALUE2);
        assure(Enum1.fromInt(101) == null);
    }

    public void testEnum2() {
        assure(Enum2.VALUE0.getValue() == 0);
        assure(Enum2.VALUE1.getValue() == 1);
        assure(Enum2.VALUE2.getValue() == 2);
        assure(Enum2.VALUE4.getValue() == 4);
        assure(Enum2.VALUE0_value == 0);
        assure(Enum2.VALUE1_value == 1);
        assure(Enum2.VALUE2_value == 2);
        assure(Enum2.VALUE4_value == 4);
        assure(Enum2.getDefault() == Enum2.VALUE0);
        assure(Enum2.fromInt(-1) == null);
        assure(Enum2.fromInt(0) == Enum2.VALUE0);
        assure(Enum2.fromInt(1) == Enum2.VALUE1);
        assure(Enum2.fromInt(2) == Enum2.VALUE2);
        assure(Enum2.fromInt(3) == null);
        assure(Enum2.fromInt(4) == Enum2.VALUE4);
        assure(Enum2.fromInt(5) == null);
    }

    public void testPolyStruct() {
        PolyStruct s = new PolyStruct();
        assure(s.member1 == null);
        assure(s.member2 == 0);
        s = new PolyStruct("ABC", 5);
        assure(s.member1.equals("ABC"));
        assure(s.member2 == 5);
    }

    public void testEmptyStruct2() {
        Struct2 s = new Struct2();
        assure(s.p1 == false);
        assure(s.p2 == 0);
        assure(s.p3 == 0);
        assure(s.p4 == 0);
        assure(s.p5 == 0);
        assure(s.p6 == 0);
        assure(s.p7 == 0L);
        assure(s.p8 == 0L);
        assure(s.p9 == 0.0f);
        assure(s.p10 == 0.0);
        assure(s.p11 == '\u0000');
        assure(s.p12.equals(""));
        assure(s.p13.equals(Type.VOID));
        assure(s.p14.equals(Any.VOID));
        assure(s.p15 == Enum2.VALUE0);
        assure(s.p16.member1 == 0);
        assure(s.p17 == null);
        assure(s.p18 == null);
        assure(s.t1 == false);
        assure(s.t2 == 0);
        assure(s.t3 == 0);
        assure(s.t4 == 0);
        assure(s.t5 == 0);
        assure(s.t6 == 0);
        assure(s.t7 == 0L);
        assure(s.t8 == 0L);
        assure(s.t9 == 0.0f);
        assure(s.t10 == 0.0);
        assure(s.t11 == '\u0000');
        assure(s.t12.equals(""));
        assure(s.t13.equals(Type.VOID));
        assure(s.t14.equals(Any.VOID));
        assure(s.t15 == Enum2.VALUE0);
        assure(s.t16.member1 == 0);
        assure(s.t17 == null);
        assure(s.t18 == null);
        assure(s.a1.length == 0);
        assure(s.a2.length == 0);
        assure(s.a3.length == 0);
        assure(s.a4.length == 0);
        assure(s.a5.length == 0);
        assure(s.a6.length == 0);
        assure(s.a7.length == 0);
        assure(s.a8.length == 0);
        assure(s.a9.length == 0);
        assure(s.a10.length == 0);
        assure(s.a11.length == 0);
        assure(s.a12.length == 0);
        assure(s.a13.length == 0);
        assure(s.a14.length == 0);
        assure(s.a15.length == 0);
        assure(s.a16.length == 0);
        assure(s.a17.length == 0);
        assure(s.a18.length == 0);
        assure(s.aa1.length == 0);
        assure(s.aa2.length == 0);
        assure(s.aa3.length == 0);
        assure(s.aa4.length == 0);
        assure(s.aa5.length == 0);
        assure(s.aa6.length == 0);
        assure(s.aa7.length == 0);
        assure(s.aa8.length == 0);
        assure(s.aa9.length == 0);
        assure(s.aa10.length == 0);
        assure(s.aa11.length == 0);
        assure(s.aa12.length == 0);
        assure(s.aa13.length == 0);
        assure(s.aa14.length == 0);
        assure(s.aa15.length == 0);
        assure(s.aa16.length == 0);
        assure(s.aa17.length == 0);
        assure(s.aa18.length == 0);
        assure(s.at1.length == 0);
        assure(s.at2.length == 0);
        assure(s.at3.length == 0);
        assure(s.at4.length == 0);
        assure(s.at5.length == 0);
        assure(s.at6.length == 0);
        assure(s.at7.length == 0);
        assure(s.at8.length == 0);
        assure(s.at9.length == 0);
        assure(s.at10.length == 0);
        assure(s.at11.length == 0);
        assure(s.at12.length == 0);
        assure(s.at13.length == 0);
        assure(s.at14.length == 0);
        assure(s.at15.length == 0);
        assure(s.at16.length == 0);
        assure(s.at17.length == 0);
        assure(s.at18.length == 0);
    }

    public void testFullStruct2() {
        //TODO:
        Struct2 s = new Struct2(
            true, (byte) 1, (short) 2, (short) 3, 4, 5, 6L, 7L, 0.8f, 0.9, 'A',
            "BCD", Type.UNSIGNED_HYPER, new Integer(22), Enum2.VALUE4,
            new Struct1(1), null, null, false, (byte) 0, (short) 0, (short) 0,
            0, 0, 0L, 0L, 0.0f, 0.0, '\u0000', "", Type.VOID, Any.VOID,
            Enum2.VALUE0, new Struct1(), null, null,
            new boolean[] { false, true }, new byte[] { (byte) 1, (byte) 2 },
            new short[0], new short[0], new int[0], new int[0],
            new long[0], new long[0], new float[0], new double[0], new char[0],
            new String[0], new Type[0], new Object[0], new Enum2[0],
            new Struct1[] { new Struct1(1), new Struct1(2) }, new Object[0],
            new XNamingService[0], new boolean[0][], new byte[0][],
            new short[0][], new short[0][], new int[0][], new int[0][],
            new long[0][], new long[0][], new float[0][], new double[0][],
            new char[0][], new String[0][], new Type[0][], new Object[0][],
            new Enum2[0][], new Struct1[0][], new Object[0][],
            new XNamingService[0][], new boolean[0][], new byte[0][],
            new short[0][], new short[0][], new int[0][], new int[0][],
            new long[0][], new long[0][], new float[0][], new double[0][],
            new char[0][], new String[0][], new Type[0][], new Object[0][],
            new Enum2[0][], new Struct1[0][], new Object[0][],
            new XNamingService[0][]);
        assure(s.p1 == true);
        assure(s.p2 == 1);
        assure(s.p3 == 2);
        assure(s.p4 == 3);
        assure(s.p5 == 4);
        assure(s.p6 == 5);
        assure(s.p7 == 6L);
        assure(s.p8 == 7L);
        assure(s.p9 == 0.8f);
        assure(s.p10 == 0.9);
        assure(s.p11 == 'A');
        assure(s.p12.equals("BCD"));
        assure(s.p13.equals(Type.UNSIGNED_HYPER));
        assure(s.p14.equals(new Integer(22)));
        assure(s.p15 == Enum2.VALUE4);
        assure(s.p16.member1 == 1);
        assure(s.p17 == null);
        assure(s.p18 == null);
        assure(s.t1 == false);
        assure(s.t2 == 0);
        assure(s.t3 == 0);
        assure(s.t4 == 0);
        assure(s.t5 == 0);
        assure(s.t6 == 0);
        assure(s.t7 == 0L);
        assure(s.t8 == 0L);
        assure(s.t9 == 0.0f);
        assure(s.t10 == 0.0);
        assure(s.t11 == '\u0000');
        assure(s.t12.equals(""));
        assure(s.t13.equals(Type.VOID));
        assure(s.t14.equals(Any.VOID));
        assure(s.t15 == Enum2.VALUE0);
        assure(s.t16.member1 == 0);
        assure(s.t17 == null);
        assure(s.t18 == null);
        assure(s.a1.length == 2);
        assure(s.a1[0] == false);
        assure(s.a1[1] == true);
        assure(s.a2.length == 2);
        assure(s.a2[0] == 1);
        assure(s.a2[1] == 2);
        assure(s.a3.length == 0);
        assure(s.a4.length == 0);
        assure(s.a5.length == 0);
        assure(s.a6.length == 0);
        assure(s.a7.length == 0);
        assure(s.a8.length == 0);
        assure(s.a9.length == 0);
        assure(s.a10.length == 0);
        assure(s.a11.length == 0);
        assure(s.a12.length == 0);
        assure(s.a13.length == 0);
        assure(s.a14.length == 0);
        assure(s.a15.length == 0);
        assure(s.a16.length == 2);
        assure(s.a16[0].member1 == 1);
        assure(s.a16[1].member1 == 2);
        assure(s.a17.length == 0);
        assure(s.a18.length == 0);
        assure(s.aa1.length == 0);
        assure(s.aa2.length == 0);
        assure(s.aa3.length == 0);
        assure(s.aa4.length == 0);
        assure(s.aa5.length == 0);
        assure(s.aa6.length == 0);
        assure(s.aa7.length == 0);
        assure(s.aa8.length == 0);
        assure(s.aa9.length == 0);
        assure(s.aa10.length == 0);
        assure(s.aa11.length == 0);
        assure(s.aa12.length == 0);
        assure(s.aa13.length == 0);
        assure(s.aa14.length == 0);
        assure(s.aa15.length == 0);
        assure(s.aa16.length == 0);
        assure(s.aa17.length == 0);
        assure(s.aa18.length == 0);
        assure(s.at1.length == 0);
        assure(s.at2.length == 0);
        assure(s.at3.length == 0);
        assure(s.at4.length == 0);
        assure(s.at5.length == 0);
        assure(s.at6.length == 0);
        assure(s.at7.length == 0);
        assure(s.at8.length == 0);
        assure(s.at9.length == 0);
        assure(s.at10.length == 0);
        assure(s.at11.length == 0);
        assure(s.at12.length == 0);
        assure(s.at13.length == 0);
        assure(s.at14.length == 0);
        assure(s.at15.length == 0);
        assure(s.at16.length == 0);
        assure(s.at17.length == 0);
        assure(s.at18.length == 0);
    }

    public void testS1() throws com.sun.star.uno.Exception {
        //TODO:
        try {
            S1.create1(context);
            failed("S1.create1");
        } catch (DeploymentException e) {}
        try {
            S1.create2(context, new Any[0]);
            failed("S1.create2");
        } catch (com.sun.star.uno.Exception e) {}
        try {
            S1.create3(context, new Any[0]);
            failed("S1.create3");
        } catch (DeploymentException e) {}
        try {
            S1.create4(context, 0, 0, 0);
            failed("S1.create4");
        } catch (DeploymentException e) {}
        try {
            S1.create5(
                context, false, (byte) 0, (short) 0, (short) 0, 0, 0, 0L, 0L,
                0.0f, 0.0, '\u0000', "", Type.VOID, Any.VOID, Enum2.VALUE0,
                new Struct1(), null, null, false, (byte) 0, (short) 0,
                (short) 0, 0, 0, 0L, 0L, 0.0f, 0.0, '\u0000', "", Type.VOID,
                Any.VOID, Enum2.VALUE0, new Struct1(), null, null,
                new boolean[0], new byte[0], new short[0], new short[0],
                new int[0], new int[0], new long[0], new long[0], new float[0],
                new double[0], new char[0], new String[0], new Type[0],
                new Object[0], new Enum2[0], new Struct1[0], new Object[0],
                new XNamingService[0], new boolean[0][], new byte[0][],
                new short[0][], new short[0][], new int[0][], new int[0][],
                new long[0][], new long[0][], new float[0][], new double[0][],
                new char[0][], new String[0][], new Type[0][], new Object[0][],
                new Enum2[0][], new Struct1[0][], new Object[0][],
                new XNamingService[0][], new boolean[0][], new byte[0][],
                new short[0][], new short[0][], new int[0][], new int[0][],
                new long[0][], new long[0][], new float[0][], new double[0][],
                new char[0][], new String[0][], new Type[0][], new Object[0][],
                new Enum2[0][], new Struct1[0][], new Object[0][],
                new XNamingService[0][]);
            failed("S1.create4");
        } catch (DeploymentException e) {}
        try {
            S1.method_abstract(context, 0);
            failed("S1.method_abstract");
        } catch (DeploymentException e) {}
        try {
            S1.method_assert(context, 0);
            failed("S1.method_assert");
        } catch (DeploymentException e) {}
    }

    public void testS2() {
        //TODO
    }

    public void testSingletons() {
        try {
            S4.get(context);
            failed("S4");
        } catch (DeploymentException e) {}
        try {
            singleton_abstract.get(context);
            failed("singleton_abstract");
        } catch (DeploymentException e) {}
    }

    private XComponentContext context;
}
