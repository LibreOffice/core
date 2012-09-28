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

package test.java_remote;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import test.lib.TestBed;

/**
 * Test case for bug #111153#.
 *
 * <P>Bug #111153# "jni_uno bridge sometimes fails to map objects
 * correctly" describes that mapping a local object out with type XDerived and
 * then mapping it back in with type XBase produces a proxy, instead of
 * short-cutting to the local object.</P>
 */
public final class MethodIdTest extends ComplexTestCase {
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
            XTest t = UnoRuntime.queryInterface(
                XTest.class, getBridge(context).getInstance("Test"));
            return t.f129() == 129;
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTest() {
                    public int f0() { return 0; }
                    public int f1() { return 1; }
                    public int f2() { return 2; }
                    public int f3() { return 3; }
                    public int f4() { return 4; }
                    public int f5() { return 5; }
                    public int f6() { return 6; }
                    public int f7() { return 7; }
                    public int f8() { return 8; }
                    public int f9() { return 9; }
                    public int f10() { return 10; }
                    public int f11() { return 11; }
                    public int f12() { return 12; }
                    public int f13() { return 13; }
                    public int f14() { return 14; }
                    public int f15() { return 15; }
                    public int f16() { return 16; }
                    public int f17() { return 17; }
                    public int f18() { return 18; }
                    public int f19() { return 19; }
                    public int f20() { return 20; }
                    public int f21() { return 21; }
                    public int f22() { return 22; }
                    public int f23() { return 23; }
                    public int f24() { return 24; }
                    public int f25() { return 25; }
                    public int f26() { return 26; }
                    public int f27() { return 27; }
                    public int f28() { return 28; }
                    public int f29() { return 29; }
                    public int f30() { return 30; }
                    public int f31() { return 31; }
                    public int f32() { return 32; }
                    public int f33() { return 33; }
                    public int f34() { return 34; }
                    public int f35() { return 35; }
                    public int f36() { return 36; }
                    public int f37() { return 37; }
                    public int f38() { return 38; }
                    public int f39() { return 39; }
                    public int f40() { return 40; }
                    public int f41() { return 41; }
                    public int f42() { return 42; }
                    public int f43() { return 43; }
                    public int f44() { return 44; }
                    public int f45() { return 45; }
                    public int f46() { return 46; }
                    public int f47() { return 47; }
                    public int f48() { return 48; }
                    public int f49() { return 49; }
                    public int f50() { return 50; }
                    public int f51() { return 51; }
                    public int f52() { return 52; }
                    public int f53() { return 53; }
                    public int f54() { return 54; }
                    public int f55() { return 55; }
                    public int f56() { return 56; }
                    public int f57() { return 57; }
                    public int f58() { return 58; }
                    public int f59() { return 59; }
                    public int f60() { return 60; }
                    public int f61() { return 61; }
                    public int f62() { return 62; }
                    public int f63() { return 63; }
                    public int f64() { return 64; }
                    public int f65() { return 65; }
                    public int f66() { return 66; }
                    public int f67() { return 67; }
                    public int f68() { return 68; }
                    public int f69() { return 69; }
                    public int f70() { return 70; }
                    public int f71() { return 71; }
                    public int f72() { return 72; }
                    public int f73() { return 73; }
                    public int f74() { return 74; }
                    public int f75() { return 75; }
                    public int f76() { return 76; }
                    public int f77() { return 77; }
                    public int f78() { return 78; }
                    public int f79() { return 79; }
                    public int f80() { return 80; }
                    public int f81() { return 81; }
                    public int f82() { return 82; }
                    public int f83() { return 83; }
                    public int f84() { return 84; }
                    public int f85() { return 85; }
                    public int f86() { return 86; }
                    public int f87() { return 87; }
                    public int f88() { return 88; }
                    public int f89() { return 89; }
                    public int f90() { return 90; }
                    public int f91() { return 91; }
                    public int f92() { return 92; }
                    public int f93() { return 93; }
                    public int f94() { return 94; }
                    public int f95() { return 95; }
                    public int f96() { return 96; }
                    public int f97() { return 97; }
                    public int f98() { return 98; }
                    public int f99() { return 99; }
                    public int f100() { return 100; }
                    public int f101() { return 101; }
                    public int f102() { return 102; }
                    public int f103() { return 103; }
                    public int f104() { return 104; }
                    public int f105() { return 105; }
                    public int f106() { return 106; }
                    public int f107() { return 107; }
                    public int f108() { return 108; }
                    public int f109() { return 109; }
                    public int f110() { return 110; }
                    public int f111() { return 111; }
                    public int f112() { return 112; }
                    public int f113() { return 113; }
                    public int f114() { return 114; }
                    public int f115() { return 115; }
                    public int f116() { return 116; }
                    public int f117() { return 117; }
                    public int f118() { return 118; }
                    public int f119() { return 119; }
                    public int f120() { return 120; }
                    public int f121() { return 121; }
                    public int f122() { return 122; }
                    public int f123() { return 123; }
                    public int f124() { return 124; }
                    public int f125() { return 125; }
                    public int f126() { return 126; }
                    public int f127() { return 127; }
                    public int f128() { return 128; }
                    public int f129() { return 129; }
                    public int f130() { return 130; }
                };
        }
    }

    public interface XTest extends XInterface {
        int f0();
        int f1();
        int f2();
        int f3();
        int f4();
        int f5();
        int f6();
        int f7();
        int f8();
        int f9();
        int f10();
        int f11();
        int f12();
        int f13();
        int f14();
        int f15();
        int f16();
        int f17();
        int f18();
        int f19();
        int f20();
        int f21();
        int f22();
        int f23();
        int f24();
        int f25();
        int f26();
        int f27();
        int f28();
        int f29();
        int f30();
        int f31();
        int f32();
        int f33();
        int f34();
        int f35();
        int f36();
        int f37();
        int f38();
        int f39();
        int f40();
        int f41();
        int f42();
        int f43();
        int f44();
        int f45();
        int f46();
        int f47();
        int f48();
        int f49();
        int f50();
        int f51();
        int f52();
        int f53();
        int f54();
        int f55();
        int f56();
        int f57();
        int f58();
        int f59();
        int f60();
        int f61();
        int f62();
        int f63();
        int f64();
        int f65();
        int f66();
        int f67();
        int f68();
        int f69();
        int f70();
        int f71();
        int f72();
        int f73();
        int f74();
        int f75();
        int f76();
        int f77();
        int f78();
        int f79();
        int f80();
        int f81();
        int f82();
        int f83();
        int f84();
        int f85();
        int f86();
        int f87();
        int f88();
        int f89();
        int f90();
        int f91();
        int f92();
        int f93();
        int f94();
        int f95();
        int f96();
        int f97();
        int f98();
        int f99();
        int f100();
        int f101();
        int f102();
        int f103();
        int f104();
        int f105();
        int f106();
        int f107();
        int f108();
        int f109();
        int f110();
        int f111();
        int f112();
        int f113();
        int f114();
        int f115();
        int f116();
        int f117();
        int f118();
        int f119();
        int f120();
        int f121();
        int f122();
        int f123();
        int f124();
        int f125();
        int f126();
        int f127();
        int f128();
        int f129();
        int f130();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("f0", 0, 0),
                                   new MethodTypeInfo("f1", 1, 0),
                                   new MethodTypeInfo("f2", 2, 0),
                                   new MethodTypeInfo("f3", 3, 0),
                                   new MethodTypeInfo("f4", 4, 0),
                                   new MethodTypeInfo("f5", 5, 0),
                                   new MethodTypeInfo("f6", 6, 0),
                                   new MethodTypeInfo("f7", 7, 0),
                                   new MethodTypeInfo("f8", 8, 0),
                                   new MethodTypeInfo("f9", 9, 0),
                                   new MethodTypeInfo("f10", 10, 0),
                                   new MethodTypeInfo("f11", 11, 0),
                                   new MethodTypeInfo("f12", 12, 0),
                                   new MethodTypeInfo("f13", 13, 0),
                                   new MethodTypeInfo("f14", 14, 0),
                                   new MethodTypeInfo("f15", 15, 0),
                                   new MethodTypeInfo("f16", 16, 0),
                                   new MethodTypeInfo("f17", 17, 0),
                                   new MethodTypeInfo("f18", 18, 0),
                                   new MethodTypeInfo("f19", 19, 0),
                                   new MethodTypeInfo("f20", 20, 0),
                                   new MethodTypeInfo("f21", 21, 0),
                                   new MethodTypeInfo("f22", 22, 0),
                                   new MethodTypeInfo("f23", 23, 0),
                                   new MethodTypeInfo("f24", 24, 0),
                                   new MethodTypeInfo("f25", 25, 0),
                                   new MethodTypeInfo("f26", 26, 0),
                                   new MethodTypeInfo("f27", 27, 0),
                                   new MethodTypeInfo("f28", 28, 0),
                                   new MethodTypeInfo("f29", 29, 0),
                                   new MethodTypeInfo("f30", 30, 0),
                                   new MethodTypeInfo("f31", 31, 0),
                                   new MethodTypeInfo("f32", 32, 0),
                                   new MethodTypeInfo("f33", 33, 0),
                                   new MethodTypeInfo("f34", 34, 0),
                                   new MethodTypeInfo("f35", 35, 0),
                                   new MethodTypeInfo("f36", 36, 0),
                                   new MethodTypeInfo("f37", 37, 0),
                                   new MethodTypeInfo("f38", 38, 0),
                                   new MethodTypeInfo("f39", 39, 0),
                                   new MethodTypeInfo("f40", 40, 0),
                                   new MethodTypeInfo("f41", 41, 0),
                                   new MethodTypeInfo("f42", 42, 0),
                                   new MethodTypeInfo("f43", 43, 0),
                                   new MethodTypeInfo("f44", 44, 0),
                                   new MethodTypeInfo("f45", 45, 0),
                                   new MethodTypeInfo("f46", 46, 0),
                                   new MethodTypeInfo("f47", 47, 0),
                                   new MethodTypeInfo("f48", 48, 0),
                                   new MethodTypeInfo("f49", 49, 0),
                                   new MethodTypeInfo("f50", 50, 0),
                                   new MethodTypeInfo("f51", 51, 0),
                                   new MethodTypeInfo("f52", 52, 0),
                                   new MethodTypeInfo("f53", 53, 0),
                                   new MethodTypeInfo("f54", 54, 0),
                                   new MethodTypeInfo("f55", 55, 0),
                                   new MethodTypeInfo("f56", 56, 0),
                                   new MethodTypeInfo("f57", 57, 0),
                                   new MethodTypeInfo("f58", 58, 0),
                                   new MethodTypeInfo("f59", 59, 0),
                                   new MethodTypeInfo("f60", 60, 0),
                                   new MethodTypeInfo("f61", 61, 0),
                                   new MethodTypeInfo("f62", 62, 0),
                                   new MethodTypeInfo("f63", 63, 0),
                                   new MethodTypeInfo("f64", 64, 0),
                                   new MethodTypeInfo("f65", 65, 0),
                                   new MethodTypeInfo("f66", 66, 0),
                                   new MethodTypeInfo("f67", 67, 0),
                                   new MethodTypeInfo("f68", 68, 0),
                                   new MethodTypeInfo("f69", 69, 0),
                                   new MethodTypeInfo("f70", 70, 0),
                                   new MethodTypeInfo("f71", 71, 0),
                                   new MethodTypeInfo("f72", 72, 0),
                                   new MethodTypeInfo("f73", 73, 0),
                                   new MethodTypeInfo("f74", 74, 0),
                                   new MethodTypeInfo("f75", 75, 0),
                                   new MethodTypeInfo("f76", 76, 0),
                                   new MethodTypeInfo("f77", 77, 0),
                                   new MethodTypeInfo("f78", 78, 0),
                                   new MethodTypeInfo("f79", 79, 0),
                                   new MethodTypeInfo("f80", 80, 0),
                                   new MethodTypeInfo("f81", 81, 0),
                                   new MethodTypeInfo("f82", 82, 0),
                                   new MethodTypeInfo("f83", 83, 0),
                                   new MethodTypeInfo("f84", 84, 0),
                                   new MethodTypeInfo("f85", 85, 0),
                                   new MethodTypeInfo("f86", 86, 0),
                                   new MethodTypeInfo("f87", 87, 0),
                                   new MethodTypeInfo("f88", 88, 0),
                                   new MethodTypeInfo("f89", 89, 0),
                                   new MethodTypeInfo("f90", 90, 0),
                                   new MethodTypeInfo("f91", 91, 0),
                                   new MethodTypeInfo("f92", 92, 0),
                                   new MethodTypeInfo("f93", 93, 0),
                                   new MethodTypeInfo("f94", 94, 0),
                                   new MethodTypeInfo("f95", 95, 0),
                                   new MethodTypeInfo("f96", 96, 0),
                                   new MethodTypeInfo("f97", 97, 0),
                                   new MethodTypeInfo("f98", 98, 0),
                                   new MethodTypeInfo("f99", 99, 0),
                                   new MethodTypeInfo("f100", 100, 0),
                                   new MethodTypeInfo("f101", 101, 0),
                                   new MethodTypeInfo("f102", 102, 0),
                                   new MethodTypeInfo("f103", 103, 0),
                                   new MethodTypeInfo("f104", 104, 0),
                                   new MethodTypeInfo("f105", 105, 0),
                                   new MethodTypeInfo("f106", 106, 0),
                                   new MethodTypeInfo("f107", 107, 0),
                                   new MethodTypeInfo("f108", 108, 0),
                                   new MethodTypeInfo("f109", 109, 0),
                                   new MethodTypeInfo("f110", 110, 0),
                                   new MethodTypeInfo("f111", 111, 0),
                                   new MethodTypeInfo("f112", 112, 0),
                                   new MethodTypeInfo("f113", 113, 0),
                                   new MethodTypeInfo("f114", 114, 0),
                                   new MethodTypeInfo("f115", 115, 0),
                                   new MethodTypeInfo("f116", 116, 0),
                                   new MethodTypeInfo("f117", 117, 0),
                                   new MethodTypeInfo("f118", 118, 0),
                                   new MethodTypeInfo("f119", 119, 0),
                                   new MethodTypeInfo("f120", 120, 0),
                                   new MethodTypeInfo("f121", 121, 0),
                                   new MethodTypeInfo("f122", 122, 0),
                                   new MethodTypeInfo("f123", 123, 0),
                                   new MethodTypeInfo("f124", 124, 0),
                                   new MethodTypeInfo("f125", 125, 0),
                                   new MethodTypeInfo("f126", 126, 0),
                                   new MethodTypeInfo("f127", 127, 0),
                                   new MethodTypeInfo("f128", 128, 0),
                                   new MethodTypeInfo("f129", 129, 0),
                                   new MethodTypeInfo("f130", 130, 0) };
    }
}
