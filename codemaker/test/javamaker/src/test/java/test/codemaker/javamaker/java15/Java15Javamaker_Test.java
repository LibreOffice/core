/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package test.codemaker.javamaker.java15;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.DeploymentException;
import com.sun.star.uno.XComponentContext;
import org.junit.Test;

import static org.junit.Assert.*;

public final class Java15Javamaker_Test {
    @Test
    public void testPlainPolyStruct() {
        PolyStruct s = new PolyStruct();
        assertTrue(s.member1 == null);
        assertTrue(s.member2 == 0);
        s = new PolyStruct("ABC", 5);
        assertTrue(s.member1.equals("ABC"));
        assertTrue(s.member2 == 5);
    }

    @Test
    public void testBooleanPolyStruct() {
        PolyStruct<Boolean,Object> s = new PolyStruct<Boolean,Object>();
        assertTrue(s.member1 == null);
        assertTrue(s.member2 == 0);
        s = new PolyStruct<Boolean,Object>(true, 5);
        assertTrue(s.member1 == true);
        assertTrue(s.member2 == 5);
    }

    @Test
    public void testStruct() {
        Struct s = new Struct();
        assertTrue(s.member.member1 == null);
        assertTrue(s.member.member2 == 0);
        s = new Struct(
            new PolyStruct<PolyStruct<boolean[], Object>, Integer>(
                new PolyStruct<boolean[], Object>(new boolean[] { true }, 3),
                4));
        assertTrue(s.member.member1.member1.length == 1);
        assertTrue(s.member.member1.member1[0] == true);
        assertTrue(s.member.member1.member2 == 3);
        assertTrue(s.member.member2 == 4);
    }

    @Test
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
            fail();
        } catch (DeploymentException e) {}
        try {
            Service.create(
                context, false, (byte) 1, (short) 2, Integer.valueOf(4));
            fail();
        } catch (DeploymentException e) {}
    }

    private static final class Ifc implements XIfc {
        public void f1(PolyStruct<Integer, Integer> arg) {}

        public void f2(PolyStruct<Object, Object> arg) {}
    }
}
