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

package test.codemaker.javamaker.java15;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import complexlib.ComplexTestCase;

public final class Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] {
            "testPlainPolyStruct", "testBooleanPolyStruct", "testStruct",
            "testService" };
    }

    public void testPlainPolyStruct() {
        PolyStruct s = new PolyStruct();
        assure(s.member1 == null);
        assure(s.member2 == 0);
        s = new PolyStruct("ABC", 5);
        assure(s.member1.equals("ABC"));
        assure(s.member2 == 5);
    }

    public void testBooleanPolyStruct() {
        PolyStruct<Boolean,Object> s = new PolyStruct<Boolean,Object>();
        assure(s.member1 == null);
        assure(s.member2 == 0);
        s = new PolyStruct<Boolean,Object>(true, 5);
        assure(s.member1 == true);
        assure(s.member2 == 5);
    }

    public void testStruct() {
        Struct s = new Struct();
        assure(s.member.member1 == null);
        assure(s.member.member2 == 0);
        s = new Struct(
            new PolyStruct<PolyStruct<boolean[], Object>, Integer>(
                new PolyStruct<boolean[], Object>(new boolean[] { true }, 3),
                4));
        assure(s.member.member1.member1.length == 1);
        assure(s.member.member1.member1[0] == true);
        assure(s.member.member1.member2 == 3);
        assure(s.member.member2 == 4);
    }

    public void testService() {
        XComponentContext context = new XComponentContext() {
                public Object getValueByName(String name) {
                    return null;
                }

                public XMultiComponentFactory getServiceManager() {
                    throw new com.sun.star.uno.RuntimeException();
                }
            };
        try {
            Service.create(context);
            failed();
        } catch (com.sun.star.uno.RuntimeException e) {}
        try {
            Service.create(
                context, false, (byte) 1, (short) 2, Integer.valueOf(4));
            failed();
        } catch (com.sun.star.uno.RuntimeException e) {}
    }

    private static final class Ifc implements XIfc {
        public void f1(PolyStruct<Integer, Integer> arg) {}

        public void f2(PolyStruct<Object, Object> arg) {}
    }
}
