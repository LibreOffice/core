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

package com.sun.star.lib.uno.typedesc;

import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.Any;
import com.sun.star.uno.IFieldDescription;
import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import org.junit.Test;
import static org.junit.Assert.*;

public final class TypeDescription_Test {
    @Test public void test() throws Exception {
        ITypeDescription voidTD = TypeDescription.getTypeDescription(
            void.class);
        ITypeDescription stringTD = TypeDescription.getTypeDescription(
            String.class);
        ITypeDescription typeTD = TypeDescription.getTypeDescription(
            Type.class);
        ITypeDescription anyTD = TypeDescription.getTypeDescription(Any.class);
        ITypeDescription interfaceTD = TypeDescription.getTypeDescription(
            XInterface.class);

        MethodSignature sigBuildinSyncTypeToAny = new MethodSignature(
            true, false, new ITypeDescription[] { typeTD },
            new ITypeDescription[1], anyTD);
        MethodSignature sigBuildinAsyncToVoid = new MethodSignature(
            true, true, new ITypeDescription[0], new ITypeDescription[0],
            voidTD);
        MethodSignature sigAddonSyncStringToVoid = new MethodSignature(
            false, false, new ITypeDescription[] { stringTD },
            new ITypeDescription[1], voidTD);
        MethodSignature sigAddonSyncStringInterfaceToVoid = new MethodSignature(
            false, false, new ITypeDescription[] { stringTD, interfaceTD },
            new ITypeDescription[2], voidTD);
        MethodSignature sigAddonSyncStringToInterface = new MethodSignature(
            false, false, new ITypeDescription[] { stringTD },
            new ITypeDescription[1], interfaceTD);

        TypeSignature emptyTypeSig = new TypeSignature(
            null, new String[0], null, new String[0], null);
        TypeSignature interfaceTypeSig = new TypeSignature(
            null, new String[] { "queryInterface", "acquire", "release" },
            new MethodSignature[] { sigBuildinSyncTypeToAny,
                                    sigBuildinAsyncToVoid,
                                    sigBuildinAsyncToVoid },
            new String[0], null);
        TypeSignature exceptionTypeSig = new TypeSignature(
            null, new String[0], null,
            new String[]{"Context"}, new TypeSignature[] { interfaceTypeSig });
            // com.sun.star.uno.Exception.idl says that Exception (a) has no
            // base exception, and (b) has two fields, Message and Context; the
            // generated com.sun.star.uno.Exception.java, however, (a) is
            // inherited from java.lang.Exception, and (b) has only one field,
            // Context, as Message is inherited from java.lang.Exception
        TypeSignature namingServiceTypeSig = new TypeSignature(
            interfaceTypeSig,
            new String[] { "getRegisteredObject", "registerObject",
                           "revokeObject" },
            new MethodSignature[] { sigAddonSyncStringToInterface,
                                    sigAddonSyncStringInterfaceToVoid,
                                    sigAddonSyncStringToVoid },
            new String[0], null);

        Object[] byteData = new Object[] {
            "byte", "[B", byte.class, TypeClass.BYTE };
        Object[] stringData = new Object[] {
            "string", "[Ljava.lang.String;", java.lang.String.class,
            TypeClass.STRING };
        Object[] typeClassData = new Object[] {
            "com.sun.star.uno.TypeClass", "[Lcom.sun.star.uno.TypeClass;",
            TypeClass.class, TypeClass.ENUM };
        Object[] interfaceData = new Object[] {
            "com.sun.star.uno.XInterface", "[Lcom.sun.star.uno.XInterface;",
            XInterface.class, TypeClass.INTERFACE };
        Object[] exceptionData = new Object [] {
            "com.sun.star.uno.Exception", "[Lcom.sun.star.uno.Exception;",
            com.sun.star.uno.Exception.class, TypeClass.EXCEPTION,
            new Object[] { interfaceData } };
        Object[] namingServiceData = new Object[] {
            "com.sun.star.uno.XNamingService",
            "[Lcom.sun.star.uno.XNamingService;", XNamingService.class,
            TypeClass.INTERFACE, null, interfaceData };

        emptyTypeSig.test("TypeSignature.test(byte)", byteData,
                          TypeDescription.getTypeDescription("byte"));
        emptyTypeSig.test("TypeSignature.test(string)", stringData,
                          TypeDescription.getTypeDescription("string"));
        emptyTypeSig.test("TypeSignature.test(TypeClass)", typeClassData,
                          TypeDescription.getTypeDescription(
                              "com.sun.star.uno.TypeClass"));
        exceptionTypeSig.test("TypeSignature.test(com.sun.star.uno.Exception)",
                              exceptionData,
                              TypeDescription.getTypeDescription(
                                  "com.sun.star.uno.Exception"));
        interfaceTypeSig.test("TypeSignature.test(XInterface)", interfaceData,
                              TypeDescription.getTypeDescription(
                                  "com.sun.star.uno.XInterface"));
        namingServiceTypeSig.test("TypeSignature.test(XNamingService)",
                                  namingServiceData,
                                  TypeDescription.getTypeDescription(
                                      "com.sun.star.uno.XNamingService"));
    }

    @Test public void testUnsigned() throws ClassNotFoundException {
        assertEquals(
            "TypeDescription for UNSIGNED LONG", "unsigned long",
            TypeDescription.getTypeDescription(Type.UNSIGNED_LONG).getTypeName());
    }

    @Test public void testGetMethodDescription() {
        TypeDescription td = TypeDescription.getTypeDescription(XDerived.class);
        td.getMethodDescription("fn");
    }

    @Test public void testSequence() throws ClassNotFoundException {
        assertEquals(
            "unsigned short",
            TypeDescription.getTypeDescription("[]unsigned short").getComponentType().getTypeName());
    }

    public interface XBase extends XInterface {
        void fn();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("fn", 0, 0) };
    }

    public interface XDerived extends XBase {
        TypeInfo[] UNOTYPEINFO = null;
    }

    private final class MethodSignature {
        public MethodSignature(
            boolean buildIn, boolean oneWay, ITypeDescription[] inParameters,
            ITypeDescription[] outParameters, ITypeDescription returnValue)
        {
            this.buildIn = buildIn;
            this.oneWay = oneWay;
            this.inParameters = inParameters;
            this.outParameters = outParameters;
            this.returnValue = returnValue;
        }

        public void test(String prefix, int index,
                         IMethodDescription description) {
            assertEquals(prefix + "; getIndex", index, description.getIndex());
            assertEquals(
                prefix + "; getMethod", buildIn,
                description.getMethod() == null);
            assertEquals(prefix + "; isOneway", oneWay, description.isOneway());
            ITypeDescription[] in = description.getInSignature();
            assertEquals(
                prefix + "; getInSignature", inParameters.length, in.length);
            for (int i = 0; i < in.length; ++i) {
                assertEquals(
                    prefix + "; getInSignature " + i, inParameters[i], in[i]);
            }
            ITypeDescription[] out = description.getOutSignature();
            assertEquals(
                prefix + "; getOutSignature", outParameters.length, out.length);
            for (int i = 0; i < out.length; ++i) {
                assertTrue(
                    prefix + "; getOutSignature " + i,
                    (out[i] == null
                     ? outParameters[i] == null
                     : out[i].equals(outParameters[i])));
            }
            assertEquals(
                prefix + "; getReturnSignature", returnValue,
                description.getReturnSignature());
        }

        private final boolean buildIn;
        private final boolean oneWay;
        private final ITypeDescription[] inParameters;
        private final ITypeDescription[] outParameters;
        private final ITypeDescription returnValue;
    }

    private final class TypeSignature {
        public TypeSignature(TypeSignature superType, String[] methodNames,
                             MethodSignature[] methodSignatures,
                             String[] fieldNames,
                             TypeSignature[] fieldSignatures) {
            this._superType = superType;
            this.methodNames = methodNames;
            this.methodSignatures = methodSignatures;
            methodOffset = superType == null ? 0
                : superType.methodOffset + superType.methodNames.length;
            this.fieldSignatures = fieldSignatures;
            this.fieldNames = fieldNames;
            fieldOffset = superType == null ? 0
                : superType.fieldOffset + superType.fieldNames.length;
        }

        public void test(String prefix, Object[] data,
                         ITypeDescription description) throws Exception {
            assertEquals(
                prefix + "; getTypeName", data[0], description.getTypeName());
            assertEquals(
                prefix + "; equals",
                TypeDescription.getTypeDescription((String)data[0]),
                description);
            assertEquals(
                prefix + "; getArrayTypeName", data[1],
                description.getArrayTypeName());
            assertSame(
                prefix + "; getZClass", data[2], description.getZClass());
            assertSame(
                prefix + "; getTypeClass", data[3], description.getTypeClass());
            assertNull(
                prefix + "; getComponentType", description.getComponentType());

            IMethodDescription[] mds = description.getMethodDescriptions();
            assertTrue(
                prefix + "; getMethodDescriptions",
                mds == null
                    ? methodSignatures == null
                    : mds.length == methodSignatures.length);
            if (methodSignatures != null) {
                for (int i = 0; i < methodSignatures.length; ++i) {
                    methodSignatures[i].test(
                        prefix + "; getMethodDescriptions " + i,
                        i + methodOffset, mds[i]);
                }
            }
            for (int i = 0; i < methodNames.length; ++i) {
                IMethodDescription md = description.getMethodDescription(
                    i + methodOffset);
                assertNotNull(
                    prefix + "; getMethodDescription " + (i + methodOffset),
                    md);
                methodSignatures[i].test(
                    prefix + "; getMethodDescription " + (i + methodOffset),
                    i + methodOffset, md);
            }
            for (int i = 0; i < methodNames.length; ++i) {
                IMethodDescription md = description.getMethodDescription(
                    methodNames[i]);
                assertNotNull(
                    prefix + "; getMethodDescription " + methodNames[i], md);
                methodSignatures[i].test(
                    prefix + "; getMethodDescription " + methodNames[i],
                    i + methodOffset, md);
            }

            IFieldDescription[] fds = description.getFieldDescriptions();
            assertTrue(
                prefix + "; getFieldDescriptions",
                fds == null
                    ? fieldSignatures == null
                    : fds.length == fieldSignatures.length);
            if (fieldSignatures != null) {
                for (int i = 0; i < fieldSignatures.length; ++i) {
                    fieldSignatures[i].test(
                        prefix + "; getFieldDescriptions " + i,
                        (Object[]) ((Object[]) data[4])[i],
                        fds[i].getTypeDescription());
                }
            }

            ITypeDescription supert = description.getSuperType();
            assertEquals(
                prefix + "; getSuperType", data.length < 6, supert == null);
            if (supert != null && data[5] != null) {
                _superType.test(prefix + "; getSuperType", (Object[]) data[5],
                                supert);
            }
        }

        private final TypeSignature _superType;
        private final MethodSignature[] methodSignatures;
        private final String[] methodNames;
        private final int methodOffset;
        private final TypeSignature[] fieldSignatures;
        private final String[] fieldNames;
        private final int fieldOffset;
    }
}
