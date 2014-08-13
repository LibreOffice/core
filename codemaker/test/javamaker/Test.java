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

package test.codemaker.javamaker;

import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.lang.XEventListener;
import com.sun.star.uno.Any;
import com.sun.star.uno.DeploymentException;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XNamingService;
import complexlib.ComplexTestCase;
import java.util.EventListener;
import test.codemaker.javamaker.Enum1;
import test.codemaker.javamaker.Enum2;
import test.codemaker.javamaker.PolyStruct;
import test.codemaker.javamaker.S2;
import test.codemaker.javamaker.Struct2;
import test.codemaker.javamaker.services.service_abstract;
import test.codemaker.javamaker.services.service_assert;
import test.codemaker.javamaker.services.service_break;
import test.codemaker.javamaker.services.service_catch;
import test.codemaker.javamaker.services.service_class;
import test.codemaker.javamaker.services.service_continue;
import test.codemaker.javamaker.services.service_do;
import test.codemaker.javamaker.services.service_else;
import test.codemaker.javamaker.services.service_extends;
import test.codemaker.javamaker.services.service_final;
import test.codemaker.javamaker.services.service_finally;
import test.codemaker.javamaker.services.service_for;
import test.codemaker.javamaker.services.service_goto;
import test.codemaker.javamaker.services.service_if;
import test.codemaker.javamaker.services.service_implements;
import test.codemaker.javamaker.services.service_import;
import test.codemaker.javamaker.services.service_instanceof;
import test.codemaker.javamaker.services.service_int;
import test.codemaker.javamaker.services.service_native;
import test.codemaker.javamaker.services.service_new;
import test.codemaker.javamaker.services.service_package;
import test.codemaker.javamaker.services.service_private;
import test.codemaker.javamaker.services.service_protected;
import test.codemaker.javamaker.services.service_public;
import test.codemaker.javamaker.services.service_return;
import test.codemaker.javamaker.services.service_static;
import test.codemaker.javamaker.services.service_strictfp;
import test.codemaker.javamaker.services.service_super;
import test.codemaker.javamaker.services.service_synchronized;
import test.codemaker.javamaker.services.service_this;
import test.codemaker.javamaker.services.service_throw;
import test.codemaker.javamaker.services.service_throws;
import test.codemaker.javamaker.services.service_try;
import test.codemaker.javamaker.services.service_volatile;
import test.codemaker.javamaker.services.service_while;
import test.codemaker.javamaker.singleton_abstract;

public final class Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] {
            "testEnum1", "testEnum2", "testPolyStruct", "testEmptyStruct2",
            "testFullStruct2", "testXEventListener", "testS1", "testS2",
            "testKeywordServices", "testSingletons" };
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
        assure(!s.p1);
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
        assure(!s.t1);
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
            "BCD", Type.UNSIGNED_HYPER, Integer.valueOf(22), Enum2.VALUE4,
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
        assure(s.p1);
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
        assure(s.p14.equals(Integer.valueOf(22)));
        assure(s.p15 == Enum2.VALUE4);
        assure(s.p16.member1 == 1);
        assure(s.p17 == null);
        assure(s.p18 == null);
        assure(!s.t1);
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
        assure(!s.a1[0]);
        assure(s.a1[1]);
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

    public void testXEventListener() {
        assure(EventListener.class.isAssignableFrom(XEventListener.class));
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
    }

    public void testS2() {
        //TODO
    }

    public void testKeywordServices() {
        try {
            service_abstract.method_abstract(context, 0);
            failed("service_abstract.method_abstract");
        } catch (DeploymentException e) {}
        try {
            service_assert.method_assert(context, 0);
            failed("service_assert.method_assert");
        } catch (DeploymentException e) {}
        try {
            service_break.method_break(context, 0);
            failed("service_break.method_break");
        } catch (DeploymentException e) {}
        try {
            service_catch.method_catch(context, 0);
            failed("service_catch.method_catch");
        } catch (DeploymentException e) {}
        try {
            service_class.method_class(context, 0);
            failed("service_class.method_class");
        } catch (DeploymentException e) {}
        try {
            service_continue.method_continue(context, 0);
            failed("service_continue.method_continue");
        } catch (DeploymentException e) {}
        try {
            service_do.method_do(context, 0);
            failed("service_do.method_do");
        } catch (DeploymentException e) {}
        try {
            service_else.method_else(context, 0);
            failed("service_else.method_else");
        } catch (DeploymentException e) {}
        try {
            service_extends.method_extends(context, 0);
            failed("service_extends.method_extends");
        } catch (DeploymentException e) {}
        try {
            service_final.method_final(context, 0);
            failed("service_final.method_final");
        } catch (DeploymentException e) {}
        try {
            service_finally.method_finally(context, 0);
            failed("service_finally.method_finally");
        } catch (DeploymentException e) {}
        try {
            service_for.method_for(context, 0);
            failed("service_for.method_for");
        } catch (DeploymentException e) {}
        try {
            service_goto.method_goto(context, 0);
            failed("service_goto.method_goto");
        } catch (DeploymentException e) {}
        try {
            service_if.method_if(context, 0);
            failed("service_if.method_if");
        } catch (DeploymentException e) {}
        try {
            service_implements.method_implements(context, 0);
            failed("service_implements.method_implements");
        } catch (DeploymentException e) {}
        try {
            service_import.method_import(context, 0);
            failed("service_import.method_import");
        } catch (DeploymentException e) {}
        try {
            service_instanceof.method_instanceof(context, 0);
            failed("service_instanceof.method_instanceof");
        } catch (DeploymentException e) {}
        try {
            service_int.method_int(context, 0);
            failed("service_int.method_int");
        } catch (DeploymentException e) {}
        try {
            service_native.method_native(context, 0);
            failed("service_native.method_native");
        } catch (DeploymentException e) {}
        try {
            service_new.method_new(context, 0);
            failed("service_new.method_new");
        } catch (DeploymentException e) {}
        try {
            service_package.method_package(context, 0);
            failed("service_package.method_package");
        } catch (DeploymentException e) {}
        try {
            service_private.method_private(context, 0);
            failed("service_private.method_private");
        } catch (DeploymentException e) {}
        try {
            service_protected.method_protected(context, 0);
            failed("service_protected.method_protected");
        } catch (DeploymentException e) {}
        try {
            service_public.method_public(context, 0);
            failed("service_public.method_public");
        } catch (DeploymentException e) {}
        try {
            service_return.method_return(context, 0);
            failed("service_return.method_return");
        } catch (DeploymentException e) {}
        try {
            service_static.method_static(context, 0);
            failed("service_static.method_static");
        } catch (DeploymentException e) {}
        try {
            service_strictfp.method_strictfp(context, 0);
            failed("service_strictfp.method_strictfp");
        } catch (DeploymentException e) {}
        try {
            service_super.method_super(context, 0);
            failed("service_super.method_super");
        } catch (DeploymentException e) {}
        try {
            service_synchronized.method_synchronized(context, 0);
            failed("service_synchronized.method_synchronized");
        } catch (DeploymentException e) {}
        try {
            service_this.method_this(context, 0);
            failed("service_this.method_this");
        } catch (DeploymentException e) {}
        try {
            service_throw.method_throw(context, 0);
            failed("service_throw.method_throw");
        } catch (DeploymentException e) {}
        try {
            service_throws.method_throws(context, 0);
            failed("service_throws.method_throws");
        } catch (DeploymentException e) {}
        try {
            service_try.method_try(context, 0);
            failed("service_try.method_try");
        } catch (DeploymentException e) {}
        try {
            service_volatile.method_volatile(context, 0);
            failed("service_volatile.method_volatile");
        } catch (DeploymentException e) {}
        try {
            service_while.method_while(context, 0);
            failed("service_while.method_while");
        } catch (DeploymentException e) {}
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
