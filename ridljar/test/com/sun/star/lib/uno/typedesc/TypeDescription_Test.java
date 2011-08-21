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
import complexlib.ComplexTestCase;

public final class TypeDescription_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test", "testUnsigned",
                              "testGetMethodDescription", "testSequence" };
    }

    public void test() throws Exception {
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

    public void testUnsigned() throws ClassNotFoundException {
        assure("TypeDescription for UNSIGNED LONG",
               TypeDescription.getTypeDescription(Type.UNSIGNED_LONG).
               getTypeName().equals("unsigned long"));
    }

    public void testGetMethodDescription() {
        TypeDescription td = TypeDescription.getTypeDescription(XDerived.class);
        td.getMethodDescription("fn");
    }

    public void testSequence() throws ClassNotFoundException {
        assure(
            TypeDescription.getTypeDescription("[]unsigned short").
            getComponentType().getTypeName().equals("unsigned short"));
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
            assure(prefix + "; getIndex", description.getIndex() == index);
            assure(prefix + "; getMethod",
                   (description.getMethod() == null) == buildIn);
            assure(prefix + "; isOneway", description.isOneway() == oneWay);
            ITypeDescription[] in = description.getInSignature();
            assure(prefix + "; getInSignature",
                   in.length == inParameters.length);
            for (int i = 0; i < in.length; ++i) {
                assure(prefix + "; getInSignature " + i,
                       in[i].equals(inParameters[i]));
            }
            ITypeDescription[] out = description.getOutSignature();
            assure(prefix + "; getOutSignature",
                   out.length == outParameters.length);
            for (int i = 0; i < out.length; ++i) {
                assure(prefix + "; getOutSignature " + i,
                       out[i] == null ? outParameters[i] == null
                       : out[i].equals(outParameters[i]));
            }
            assure(prefix + "; getReturnSignature",
                   description.getReturnSignature().equals(returnValue));
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
            assure(prefix + "; getTypeName",
                   description.getTypeName().equals(data[0]));
            assure(prefix + "; equals",
                   description.equals(TypeDescription.getTypeDescription(
                                          (String)data[0])));
            assure(prefix + "; getArrayTypeName",
                   description.getArrayTypeName().equals(data[1]));
            assure(prefix + "; getZClass", description.getZClass() == data[2]);
            assure(prefix + "; getTypeClass",
                   description.getTypeClass() == data[3]);
            assure(prefix + "; getComponentType",
                   description.getComponentType() == null);

            IMethodDescription[] mds = description.getMethodDescriptions();
            assure(
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
                assure(prefix + "; getMethodDescription " + (i + methodOffset),
                       md != null);
                methodSignatures[i].test(
                    prefix + "; getMethodDescription " + (i + methodOffset),
                    i + methodOffset, md);
            }
            for (int i = 0; i < methodNames.length; ++i) {
                IMethodDescription md = description.getMethodDescription(
                    methodNames[i]);
                assure(prefix + "; getMethodDescription " + methodNames[i],
                       md != null);
                methodSignatures[i].test(
                    prefix + "; getMethodDescription " + methodNames[i],
                    i + methodOffset, md);
            }

            IFieldDescription[] fds = description.getFieldDescriptions();
            assure(
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
            assure(prefix + "; getSuperType",
                   (supert == null) == (data.length < 6));
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
