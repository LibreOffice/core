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

package test.codemaker.javamaker.java15;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.DeploymentException;
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
                    return null;
                }
            };
        try {
            Service.create(context);
            failed();
        } catch (DeploymentException e) {}
        try {
            Service.create(
                context, false, (byte) 1, (short) 2, Integer.valueOf(4));
            failed();
        } catch (DeploymentException e) {}
    }

    private static final class Ifc implements XIfc {
        public void f1(PolyStruct<Integer, Integer> arg) {}

        public void f2(PolyStruct<Object, Object> arg) {}
    }
}
