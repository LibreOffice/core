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

package test.java_uno.any;

import com.sun.star.uno.Any;
import com.sun.star.uno.Enum;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;
import java.lang.reflect.Array;
// "any" is a reserved word in IDL, so we need to use a different packagename
import test.java_uno.anytest.*;

final class TestAny {
    public static boolean test(XTransport transport, boolean createTypes) {
        boolean success = true;

        // Sanity check for com.sun.star.uno.Type:
        success &= testType(void.class, TypeClass.VOID, "void");
        success &= testType(boolean.class, TypeClass.BOOLEAN, "boolean");
        success &= testType(byte.class, TypeClass.BYTE, "byte");
        success &= testType(short.class, TypeClass.SHORT, "short");
        success &= testType(int.class, TypeClass.LONG, "long");
        success &= testType(long.class, TypeClass.HYPER, "hyper");
        success &= testType(float.class, TypeClass.FLOAT, "float");
        success &= testType(double.class, TypeClass.DOUBLE, "double");
        success &= testType(char.class, TypeClass.CHAR, "char");
        success &= testType(String.class, TypeClass.STRING, "string");
        success &= testType(Type.class, TypeClass.TYPE, "type");
        success &= testType(Any.class, TypeClass.ANY, "any");
        success &= testType(boolean[].class, TypeClass.SEQUENCE, "[]boolean");
        success &= testType(byte[].class, TypeClass.SEQUENCE, "[]byte");
        success &= testType(short[].class, TypeClass.SEQUENCE, "[]short");
        success &= testType(int[].class, TypeClass.SEQUENCE, "[]long");
        success &= testType(long[].class, TypeClass.SEQUENCE, "[]hyper");
        success &= testType(float[].class, TypeClass.SEQUENCE, "[]float");
        success &= testType(double[].class, TypeClass.SEQUENCE, "[]double");
        success &= testType(char[].class, TypeClass.SEQUENCE, "[]char");
        success &= testType(String[].class, TypeClass.SEQUENCE, "[]string");
        success &= testType(Type[].class, TypeClass.SEQUENCE, "[]type");
        success &= testType(Any[].class, TypeClass.SEQUENCE, "[]any");
        success &= testType(Enum1[].class, TypeClass.SEQUENCE,
                            "[]" + Enum1.class.getName());
        success &= testType(BaseStruct[].class, TypeClass.SEQUENCE,
                            "[]" + BaseStruct.class.getName());
        success &= testType(DerivedStruct[].class, TypeClass.SEQUENCE,
                            "[]" + DerivedStruct.class.getName());
        success &= testType(XInterface[].class, TypeClass.SEQUENCE,
                            "[]" + XInterface.class.getName());
        success &= testType(BaseInterface[].class, TypeClass.SEQUENCE,
                            "[]" + BaseInterface.class.getName());
        success &= testType(DerivedInterface[].class, TypeClass.SEQUENCE,
                            "[]" + DerivedInterface.class.getName());
        success &= testType(boolean[][].class, TypeClass.SEQUENCE,
                            "[][]boolean");
        success &= testType(byte[][].class, TypeClass.SEQUENCE, "[][]byte");
        success &= testType(short[][].class, TypeClass.SEQUENCE, "[][]short");
        success &= testType(int[][].class, TypeClass.SEQUENCE, "[][]long");
        success &= testType(long[][].class, TypeClass.SEQUENCE, "[][]hyper");
        success &= testType(float[][].class, TypeClass.SEQUENCE, "[][]float");
        success &= testType(double[][].class, TypeClass.SEQUENCE, "[][]double");
        success &= testType(char[][].class, TypeClass.SEQUENCE, "[][]char");
        success &= testType(String[][].class, TypeClass.SEQUENCE, "[][]string");
        success &= testType(Type[][].class, TypeClass.SEQUENCE, "[][]type");
        success &= testType(Any[][].class, TypeClass.SEQUENCE, "[][]any");
        success &= testType(Enum1[][].class, TypeClass.SEQUENCE,
                            "[][]" + Enum1.class.getName());
        success &= testType(BaseStruct[][].class, TypeClass.SEQUENCE,
                            "[][]" + BaseStruct.class.getName());
        success &= testType(DerivedStruct[][].class, TypeClass.SEQUENCE,
                            "[][]" + DerivedStruct.class.getName());
        success &= testType(XInterface[][].class, TypeClass.SEQUENCE,
                            "[][]" + XInterface.class.getName());
        success &= testType(BaseInterface[][].class, TypeClass.SEQUENCE,
                            "[][]" + BaseInterface.class.getName());
        success &= testType(DerivedInterface[][].class, TypeClass.SEQUENCE,
                            "[][]" + DerivedInterface.class.getName());
        success &= testType(Enum1.class, TypeClass.ENUM, Enum1.class.getName());
        success &= testType(BaseStruct.class, TypeClass.STRUCT,
                            BaseStruct.class.getName());
        success &= testType(DerivedStruct.class, TypeClass.STRUCT,
                            DerivedStruct.class.getName());
        success &= testType(com.sun.star.uno.Exception.class,
                            TypeClass.EXCEPTION,
                            com.sun.star.uno.Exception.class.getName());
        success &= testType(com.sun.star.uno.RuntimeException.class,
                            TypeClass.EXCEPTION,
                            com.sun.star.uno.RuntimeException.class.getName());
        success &= testType(XInterface.class, TypeClass.INTERFACE,
                            XInterface.class.getName());
        success &= testType(BaseInterface.class, TypeClass.INTERFACE,
                            BaseInterface.class.getName());
        success &= testType(DerivedInterface.class, TypeClass.INTERFACE,
                            DerivedInterface.class.getName());

        // VOID:
        success &= testMapAny(transport, Any.VOID, new CompareBoxed());

        // BOOLEAN:
        success &= testMapAny(transport, Boolean.FALSE, new CompareBoxed());
        success &= testMapAny(transport, Boolean.TRUE, new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.BOOLEAN, Boolean.FALSE),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.BOOLEAN, Boolean.TRUE),
                              new CompareUnboxed());

        // BYTE:
        success &= testMapAny(transport, new Byte((byte) -128),
                              new CompareBoxed());
        success &= testMapAny(transport, new Byte((byte) 0),
                              new CompareBoxed());
        success &= testMapAny(transport, new Byte((byte) 127),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.BYTE, new Byte((byte) -128)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.BYTE, new Byte((byte) 0)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.BYTE, new Byte((byte) 127)),
                              new CompareUnboxed());

        // SHORT:
        success &= testMapAny(transport, new Short((short) -32768),
                              new CompareBoxed());
        success &= testMapAny(transport, new Short((short) 0),
                              new CompareBoxed());
        success &= testMapAny(transport, new Short((short) 32767),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.SHORT,
                                      new Short((short) -32768)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.SHORT, new Short((short) 0)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.SHORT, new Short((short) 32767)),
                              new CompareUnboxed());

        // UNSIGNED SHORT:
        success &= testMapAny(transport,
                              new Any(Type.UNSIGNED_SHORT,
                                      new Short((short) 0)),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.UNSIGNED_SHORT,
                                      new Short((short) -32768)),
                              new CompareBoxed());

        // LONG:
        success &= testMapAny(transport, new Integer(-2147483648),
                              new CompareBoxed());
        success &= testMapAny(transport, new Integer(0),
                              new CompareBoxed());
        success &= testMapAny(transport, new Integer(2147483647),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.LONG, new Integer(-2147483648)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.LONG, new Integer(0)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.LONG, new Integer(2147483647)),
                              new CompareUnboxed());

        // UNSIGNED LONG:
        success &= testMapAny(transport,
                              new Any(Type.UNSIGNED_LONG, new Integer(0)),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.UNSIGNED_LONG,
                                      new Integer(-2147483648)),
                              new CompareBoxed());

        // HYPER:
        success &= testMapAny(transport, new Long(-9223372036854775808L),
                              new CompareBoxed());
        success &= testMapAny(transport, new Long(0L), new CompareBoxed());
        success &= testMapAny(transport, new Long(9223372036854775807L),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.HYPER,
                                      new Long(-9223372036854775808L)),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.HYPER, new Long(0L)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.HYPER,
                                      new Long(9223372036854775807L)),
                              new CompareUnboxed());

        // UNSIGNED HYPER:
        success &= testMapAny(transport,
                              new Any(Type.UNSIGNED_HYPER, new Long(0L)),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.UNSIGNED_HYPER,
                                      new Long(-9223372036854775808L)),
                              new CompareBoxed());

        // FLOAT:
        success &= testMapAny(transport, new Float(Float.NEGATIVE_INFINITY),
                              new CompareBoxed());
        success &= testMapAny(transport, new Float(Float.MIN_VALUE),
                              new CompareBoxed());
        success &= testMapAny(transport, new Float(-0.0f),
                              new CompareBoxed());
        success &= testMapAny(transport, new Float(0.0f),
                              new CompareBoxed());
        success &= testMapAny(transport, new Float(Float.MAX_VALUE),
                              new CompareBoxed());
        success &= testMapAny(transport, new Float(Float.POSITIVE_INFINITY),
                              new CompareBoxed());
        success &= testMapAny(transport, new Float(Float.NaN),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.FLOAT,
                                      new Float(Float.NEGATIVE_INFINITY)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.FLOAT,
                                      new Float(Float.MIN_VALUE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.FLOAT, new Float(-0.0f)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.FLOAT, new Float(0.0f)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.FLOAT,
                                      new Float(Float.MAX_VALUE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.FLOAT,
                                      new Float(Float.POSITIVE_INFINITY)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.FLOAT, new Float(Float.NaN)),
                              new CompareUnboxed());

        // DOUBLE:
        success &= testMapAny(transport,
                              new Double(Double.NEGATIVE_INFINITY),
                              new CompareBoxed());
        success &= testMapAny(transport, new Double(Double.MIN_VALUE),
                              new CompareBoxed());
        success &= testMapAny(transport, new Double(-0.0f),
                              new CompareBoxed());
        success &= testMapAny(transport, new Double(0.0f),
                              new CompareBoxed());
        success &= testMapAny(transport, new Double(Double.MAX_VALUE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Double(Double.POSITIVE_INFINITY),
                              new CompareBoxed());
        success &= testMapAny(transport, new Double(Double.NaN),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.DOUBLE,
                                      new Double(Double.NEGATIVE_INFINITY)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.DOUBLE,
                                      new Double(Double.MIN_VALUE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.DOUBLE, new Double(-0.0)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.DOUBLE, new Double(0.0)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.DOUBLE,
                                      new Double(Double.MAX_VALUE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.DOUBLE,
                                      new Double(Double.POSITIVE_INFINITY)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.DOUBLE, new Double(Double.NaN)),
                              new CompareUnboxed());

        // CHAR:
        success &= testMapAny(transport, new Character('\u0000'),
                              new CompareBoxed());
        success &= testMapAny(transport, new Character('\uDBFF'),
                              new CompareBoxed());
        success &= testMapAny(transport, new Character('\uFFFD'),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(Type.CHAR, new Character('\u0000')),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.CHAR, new Character('\uDBFF')),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.CHAR, new Character('\uFFFD')),
                              new CompareUnboxed());

        // STRING:
        success &= testMapAny(transport, "", new CompareBoxed());
        success &= testMapAny(transport, "\uD800\uDC00",
                              new CompareBoxed());
        success &= testMapAny(transport, "Test", new CompareBoxed());
        success &= testMapAny(transport, new Any(Type.STRING, ""),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.STRING, "\uD800\uDC00"),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.STRING, "Test"),
                              new CompareUnboxed());

        // TYPE:
        success &= testMapAny(transport, Type.VOID, new CompareBoxed());
        success &= testMapAny(transport, Type.BOOLEAN, new CompareBoxed());
        success &= testMapAny(transport, Type.BYTE, new CompareBoxed());
        success &= testMapAny(transport, Type.SHORT, new CompareBoxed());
        success &= testMapAny(transport, Type.UNSIGNED_SHORT,
                              new CompareBoxed());
        success &= testMapAny(transport, Type.LONG, new CompareBoxed());
        success &= testMapAny(transport, Type.UNSIGNED_LONG,
                              new CompareBoxed());
        success &= testMapAny(transport, Type.HYPER, new CompareBoxed());
        success &= testMapAny(transport, Type.UNSIGNED_HYPER,
                              new CompareBoxed());
        success &= testMapAny(transport, Type.FLOAT, new CompareBoxed());
        success &= testMapAny(transport, Type.DOUBLE, new CompareBoxed());
        success &= testMapAny(transport, Type.CHAR, new CompareBoxed());
        success &= testMapAny(transport, Type.STRING, new CompareBoxed());
        success &= testMapAny(transport, Type.TYPE, new CompareBoxed());
        success &= testMapAny(transport, Type.ANY, new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]boolean", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]byte", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]short", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]unsigned short",
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]long", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]unsigned long",
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]hyper", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]unsigned hyper",
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]float", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]double", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]char", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]string", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]type", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]any", TypeClass.SEQUENCE),
                              new CompareBoxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Type("[]" + Enum1.class.getName(),
                                           TypeClass.SEQUENCE),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type("[]" + BaseStruct.class.getName(),
                                           TypeClass.SEQUENCE),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type("[]" + DerivedStruct.class.getName(),
                                           TypeClass.SEQUENCE),
                                  new CompareBoxed());
        }
        success &= testMapAny(transport,
                              new Type("[]" + XInterface.class.getName(),
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]" + BaseInterface.class.getName(),
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[]"
                                       + DerivedInterface.class.getName(),
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]boolean", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]byte", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]short", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]unsigned short",
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]long", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]unsigned long",
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]hyper", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]unsigned hyper",
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]float", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]double", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]char", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]string", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]type", TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]any", TypeClass.SEQUENCE),
                              new CompareBoxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Type("[][]" + Enum1.class.getName(),
                                           TypeClass.SEQUENCE),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type("[][]" + BaseStruct.class.getName(),
                                           TypeClass.SEQUENCE),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type("[][]"
                                           + DerivedStruct.class.getName(),
                                           TypeClass.SEQUENCE),
                                  new CompareBoxed());
        }
        success &= testMapAny(transport,
                              new Type("[][]" + XInterface.class.getName(),
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]"
                                       + BaseInterface.class.getName(),
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type("[][]"
                                       + DerivedInterface.class.getName(),
                                       TypeClass.SEQUENCE),
                              new CompareBoxed());
        if (createTypes) {
            success &= testMapAny(transport, new Type(Enum1.class.getName(),
                                                      TypeClass.ENUM),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type(BaseStruct.class.getName(),
                                           TypeClass.STRUCT),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type(DerivedStruct.class.getName(),
                                           TypeClass.STRUCT),
                                  new CompareBoxed());
        }
        success &= testMapAny(transport,
                              new Type(
                                  com.sun.star.uno.Exception.class.
                                  getName(),
                                  TypeClass.EXCEPTION),
                              new CompareBoxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Type(BaseException.class.getName(),
                                           TypeClass.EXCEPTION),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type(DerivedException.class.getName(),
                                           TypeClass.EXCEPTION),
                                  new CompareBoxed());
        }
        success &= testMapAny(transport,
                              new Type(
                                  com.sun.star.uno.RuntimeException.class.
                                  getName(),
                                  TypeClass.EXCEPTION),
                              new CompareBoxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Type(
                                      BaseRuntimeException.class.getName(),
                                      TypeClass.EXCEPTION),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Type(
                                      DerivedRuntimeException.class.
                                      getName(),
                                      TypeClass.EXCEPTION),
                                  new CompareBoxed());
        }
        success &= testMapAny(transport,
                              new Type(XInterface.class.getName(),
                                       TypeClass.INTERFACE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type(BaseInterface.class.getName(),
                                       TypeClass.INTERFACE),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type(DerivedInterface.class.getName(),
                                       TypeClass.INTERFACE),
                              new CompareBoxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.VOID),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.BOOLEAN),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.BYTE),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.SHORT),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE, Type.UNSIGNED_SHORT),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.LONG),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE, Type.UNSIGNED_LONG),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.HYPER),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE, Type.UNSIGNED_HYPER),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.FLOAT),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.DOUBLE),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.CHAR),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.STRING),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.TYPE),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Any(Type.TYPE, Type.ANY),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]boolean",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]byte",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]short",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]unsigned short",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]long",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]unsigned long",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]hyper",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]unsigned hyper",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]float",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]double",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]char",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]string",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]type",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]any",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type("[]" + Enum1.class.getName(),
                                                   TypeClass.SEQUENCE)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type("[]"
                                                   + BaseStruct.class.getName(),
                                                   TypeClass.SEQUENCE)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type(
                                              "[]"
                                              + DerivedStruct.class.getName(),
                                              TypeClass.SEQUENCE)),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[]"
                                               + XInterface.class.getName(),
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type(
                                          "[]"
                                          + BaseInterface.class.getName(),
                                          TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  Type.TYPE,
                                  new Type(
                                      "[]"
                                      + DerivedInterface.class.getName(),
                                      TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]boolean",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]byte",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]short",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]unsigned short",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]long",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]unsigned long",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]hyper",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]unsigned hyper",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]float",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]double",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]char",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]string",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]type",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]any",
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type("[][]"
                                                   + Enum1.class.getName(),
                                                   TypeClass.SEQUENCE)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type("[][]"
                                                   + BaseStruct.class.getName(),
                                                   TypeClass.SEQUENCE)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type(
                                              "[][]"
                                              + DerivedStruct.class.getName(),
                                              TypeClass.SEQUENCE)),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type("[][]"
                                               + XInterface.class.getName(),
                                               TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type(
                                          "[][]"
                                          + BaseInterface.class.getName(),
                                          TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  Type.TYPE,
                                  new Type(
                                      "[][]"
                                      + DerivedInterface.class.getName(),
                                      TypeClass.SEQUENCE)),
                              new CompareUnboxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type(Enum1.class.getName(),
                                                   TypeClass.ENUM)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type(BaseStruct.class.getName(),
                                                   TypeClass.STRUCT)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type(
                                              DerivedStruct.class.getName(),
                                              TypeClass.STRUCT)),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport,
                              new Any(
                                  Type.TYPE,
                                  new Type(
                                      com.sun.star.uno.Exception.class.
                                      getName(),
                                      TypeClass.EXCEPTION)),
                              new CompareUnboxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Any(Type.TYPE,
                                          new Type(
                                              BaseException.class.getName(),
                                              TypeClass.EXCEPTION)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(
                                      Type.TYPE,
                                      new Type(
                                          DerivedException.class.getName(),
                                          TypeClass.EXCEPTION)),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport,
                              new Any(
                                  Type.TYPE,
                                  new Type(
                                      com.sun.star.uno.RuntimeException.
                                      class.getName(),
                                      TypeClass.EXCEPTION)),
                              new CompareUnboxed());
        if (createTypes) {
            success &= testMapAny(transport,
                                  new Any(
                                      Type.TYPE,
                                      new Type(
                                          BaseRuntimeException.class.
                                          getName(),
                                          TypeClass.EXCEPTION)),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(
                                      Type.TYPE,
                                      new Type(
                                          DerivedRuntimeException.class.
                                          getName(),
                                          TypeClass.EXCEPTION)),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type(XInterface.class.getName(),
                                               TypeClass.INTERFACE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type(
                                          BaseInterface.class.getName(),
                                          TypeClass.INTERFACE)),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(Type.TYPE,
                                      new Type(
                                          DerivedInterface.class.getName(),
                                          TypeClass.INTERFACE)),
                              new CompareUnboxed());

        // Sequence Types:
        success &= testMapAny(transport, new boolean[] {},
                              new CompareBoxed());
        success &= testMapAny(transport, new boolean[] { false, true },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(boolean[].class),
                                      new boolean[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(boolean[].class),
                                      new boolean[] { false, true }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new byte[] {},
                              new CompareBoxed());
        success &= testMapAny(transport, new byte[] { -128, 0, 127 },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(byte[].class),
                                      new byte[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(byte[].class),
                                      new byte[] { -128, 0, 127 }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new short[] {},
                              new CompareBoxed());
        success &= testMapAny(transport, new short[] { -32768, 0, 32767 },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(short[].class),
                                      new short[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(short[].class),
                                      new short[] { -32768, 0, 32767 }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type("[]unsigned short",
                                               TypeClass.SEQUENCE),
                                      new short[] {}),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type("[]unsigned short",
                                               TypeClass.SEQUENCE),
                                      new short[] { 0, -32768 }),
                              new CompareBoxed());
        success &= testMapAny(transport, new int[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new int[] { -2147483648, 0, 2147483647 },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(int[].class),
                                      new int[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(int[].class),
                                      new int[] { -2147483648, 0,
                                                  2147483647 }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type("[]unsigned long",
                                               TypeClass.SEQUENCE),
                                      new int[] {}),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type("[]unsigned long",
                                               TypeClass.SEQUENCE),
                                      new int[] { 0, -2147483648 }),
                              new CompareBoxed());
        success &= testMapAny(transport, new long[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new long[] { -9223372036854775808L, 0L,
                                           9223372036854775807L },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(long[].class),
                                      new long[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(long[].class),
                                      new long[] { -9223372036854775808L,
                                                   0L,
                                                   9223372036854775807L }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type("[]unsigned hyper",
                                               TypeClass.SEQUENCE),
                                      new long[] {}),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type("[]unsigned hyper",
                                               TypeClass.SEQUENCE),
                                      new long[] { 0L,
                                                   -9223372036854775808L }),
                              new CompareBoxed());
        success &= testMapAny(transport, new float[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new float[] { Float.NEGATIVE_INFINITY,
                                            Float.MIN_VALUE, -0.0f, 0.0f,
                                            Float.MAX_VALUE,
                                            Float.POSITIVE_INFINITY,
                                            Float.NaN },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(float[].class),
                                      new float[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(float[].class),
                                      new float[] { Float.NEGATIVE_INFINITY,
                                                    Float.MIN_VALUE, -0.0f,
                                                    0.0f, Float.MAX_VALUE,
                                                    Float.POSITIVE_INFINITY,
                                                    Float.NaN }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new double[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new double[] { Double.NEGATIVE_INFINITY,
                                             Double.MIN_VALUE, -0.0, 0.0,
                                             Double.MAX_VALUE,
                                             Double.POSITIVE_INFINITY,
                                             Double.NaN },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(double[].class),
                                      new double[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(double[].class),
                                      new double[] {
                                          Double.NEGATIVE_INFINITY,
                                          Double.MIN_VALUE, -0.0, 0.0,
                                          Double.MAX_VALUE,
                                          Double.POSITIVE_INFINITY,
                                          Double.NaN }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new char[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new char[] { '\u0000', '\uDBFF', '\uFFFD' },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(char[].class),
                                      new char[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  new Type(char[].class),
                                  new char[] { '\u0000', '\uDBFF',
                                               '\uFFFD' }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new String[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new String[] { "", "\uD800\uDC00", "Test" },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(String[].class),
                                      new String[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(String[].class),
                                      new String[] { "", "\uD800\uDC00",
                                                     "Test" }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Type[] {}, new CompareBoxed());
        success &= testMapAny(transport,
                              new Type[] {
                                  Type.VOID,
                                  new Type(DerivedInterface.class.getName(),
                                           TypeClass.INTERFACE) },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(Type[].class),
                                      new Type[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  new Type(Type[].class),
                                  new Type[] {
                                      Type.VOID,
                                      new Type(
                                          DerivedInterface.class.getName(),
                                          TypeClass.INTERFACE) }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Object[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Object[] { Any.VOID, Boolean.FALSE },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Object[] {
                                  Boolean.FALSE,
                                  new Any(Type.BOOLEAN, Boolean.TRUE) },
                              new CompareBoxed(true));
        success &= testMapAny(transport,
                              new Any(new Type(Any[].class),
                                      new Object[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(Any[].class),
                                      new Object[] { Any.VOID,
                                                     Boolean.FALSE }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(Any[].class),
                                      new Object[] {
                                          Boolean.FALSE,
                                          new Any(Type.BOOLEAN,
                                                  Boolean.TRUE) }),
                              new CompareUnboxed(true));
        success &= testMapAny(transport, new Any[] {},
                              new CompareSpecific(new Object[] {}));
        success &= testMapAny(transport,
                              new Any[] { Any.VOID,
                                          new Any(Type.BOOLEAN,
                                                  Boolean.TRUE) },
                              new CompareSpecific(
                                  new Object[] { Any.VOID, Boolean.TRUE }));
        success &= testMapAny(transport,
                              new Any(new Type(Any[].class), new Any[] {}),
                              new CompareSpecific(new Object[] {}));
        success &= testMapAny(transport,
                              new Any(new Type(Any[].class),
                                      new Any[] { Any.VOID,
                                                  new Any(Type.BOOLEAN,
                                                          Boolean.TRUE) }),
                              new CompareSpecific(
                                  new Object[] { Any.VOID, Boolean.TRUE }));
        success &= testMapAny(transport,
                              new Any(new Type(Any[].class),
                                      new Boolean[] {}),
                              new CompareSpecific(new Object[] {}));
        success &= testMapAny(transport,
                              new Any(new Type(Any[].class),
                                      new Boolean[] { Boolean.FALSE }),
                              new CompareSpecific(
                                  new Object[] { Boolean.FALSE }));
        if (createTypes) {
            success &= testMapAny(transport, new Enum1[] {},
                                  new CompareBoxed());
            success &= testMapAny(transport, new Enum1[] { new Enum1(),
                                                           new Enum2() },
                                  new CompareSpecific(
                                      new Enum1[] { new Enum1(),
                                                    new Enum1() }));
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[].class),
                                          new Enum1[] {}),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[].class),
                                          new Enum1[] { new Enum1(),
                                                        new Enum2() }),
                                  new CompareSpecific(
                                      new Enum1[] { new Enum1(),
                                                    new Enum1() }));
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[].class),
                                          new Enum2[] {}),
                                  new CompareSpecific(new Enum1[] {}));
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[].class),
                                          new Enum2[] { new Enum2() }),
                                  new CompareSpecific(
                                      new Enum1[] { new Enum1() }));
            success &= testMapAny(transport, new BaseStruct[] {},
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new BaseStruct[] { new BaseStruct(),
                                                     new DerivedStruct() },
                                  new CompareSpecific(
                                      new BaseStruct[] { new BaseStruct(),
                                                         new BaseStruct() }));
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[].class),
                                          new BaseStruct[] {}),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[].class),
                                          new BaseStruct[] {
                                              new BaseStruct(),
                                              new DerivedStruct() }),
                                  new CompareSpecific(
                                      new BaseStruct[] { new BaseStruct(),
                                                         new BaseStruct() }));
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[].class),
                                          new DerivedStruct[] {}),
                                  new CompareSpecific(new BaseStruct[] {}));
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[].class),
                                          new DerivedStruct[] {
                                              new DerivedStruct() }),
                                  new CompareSpecific(
                                      new BaseStruct[] { new BaseStruct() }));
            success &= testMapAny(transport, new DerivedStruct[] {},
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new DerivedStruct[] { new DerivedStruct() },
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(DerivedStruct[].class),
                                          new DerivedStruct[] {}),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(DerivedStruct[].class),
                                          new DerivedStruct[] {
                                              new DerivedStruct() }),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport, new XInterface[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new XInterface[] {
                                  null, new XInterface() {},
                                  new BaseInterface() {},
                                  new DerivedInterface() {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[].class),
                                      new XInterface[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[].class),
                                      new XInterface[] {
                                          null, new XInterface() {},
                                          new BaseInterface() {},
                                          new DerivedInterface() {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[].class),
                                      new Object[] {}),
                              new CompareSpecific(new XInterface[] {}));
        {
            XInterface if1 = new XInterface() {};
            XInterface if2 = new BaseInterface() {};
            XInterface if3 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(XInterface[].class),
                                          new Object[] { null, if1, if2,
                                                         if3 }),
                                  new CompareSpecific(
                                      new XInterface[] { null, if1, if2,
                                                         if3 }));
        }
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[].class),
                                      new BaseInterface[] {}),
                              new CompareSpecific(new XInterface[] {}));
        {
            BaseInterface if1 = new BaseInterface() {};
            BaseInterface if2 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(XInterface[].class),
                                          new BaseInterface[] { null, if1,
                                                                if2 }),
                                  new CompareSpecific(
                                      new XInterface[] { null, if1, if2 }));
        }
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[].class),
                                      new DerivedInterface[] {}),
                              new CompareSpecific(new XInterface[] {}));
        {
            DerivedInterface if1 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(XInterface[].class),
                                          new DerivedInterface[] { null,
                                                                   if1 }),
                                  new CompareSpecific(
                                      new XInterface[] { null, if1 }));
        }
        success &= testMapAny(transport, new BaseInterface[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new BaseInterface[] {
                                  null, new BaseInterface() {},
                                  new DerivedInterface() {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface[].class),
                                      new BaseInterface[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface[].class),
                                      new BaseInterface[] {
                                          null, new BaseInterface() {},
                                          new DerivedInterface() {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface[].class),
                                      new DerivedInterface[] {}),
                              new CompareSpecific(new BaseInterface[] {}));
        {
            DerivedInterface if1 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(BaseInterface[].class),
                                          new DerivedInterface[] { null,
                                                                   if1 }),
                                  new CompareSpecific(
                                      new BaseInterface[] { null, if1 }));
        }
        success &= testMapAny(transport, new DerivedInterface[] {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new DerivedInterface[] {
                                  null, new DerivedInterface() {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(DerivedInterface[].class),
                                      new DerivedInterface[] {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(DerivedInterface[].class),
                                      new DerivedInterface[] {
                                          null,
                                          new DerivedInterface() {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new boolean[][] { new boolean[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new boolean[][] {
                                  new boolean[] { false, true } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(boolean[][].class),
                                      new boolean[][] { new boolean[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(boolean[][].class),
                                      new boolean[][] {
                                          new boolean[] { false, true } }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new byte[][] { new byte[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new byte[][] { new byte[] { -128, 0, 127 } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(byte[][].class),
                                      new byte[][] { new byte[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(byte[][].class),
                                      new byte[][] {
                                          new byte[] { -128, 0, 127 } }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new short[][] { new short[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new short[][] {
                                  new short[] { -32768, 0, 32767 } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(short[][].class),
                                      new short[][] { new short[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(short[][].class),
                                      new short[][] {
                                          new short[] { -32768, 0,
                                                        32767 } }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type("[][]unsigned short",
                                               TypeClass.SEQUENCE),
                                      new short[][] { new short[] {} }),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type("[][]unsigned short",
                                               TypeClass.SEQUENCE),
                                      new short[][] {
                                          new short[] { 0, -32768 } }),
                              new CompareBoxed());
        success &= testMapAny(transport, new int[][] { new int[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new int[][] { new int[] { -2147483648, 0,
                                                        2147483647 } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(int[][].class),
                                      new int[][] { new int[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(int[][].class),
                                      new int[][] {
                                          new int[] { -2147483648, 0,
                                                      2147483647 } }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type("[][]unsigned long",
                                               TypeClass.SEQUENCE),
                                      new int[][] { new int[] {} }),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type("[][]unsigned long",
                                               TypeClass.SEQUENCE),
                                      new int[][] {
                                          new int[] { 0, -2147483648 } }),
                              new CompareBoxed());
        success &= testMapAny(transport, new long[][] { new long[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new long[][] {
                                  new long[] { -9223372036854775808L, 0L,
                                               9223372036854775807L } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(long[][].class),
                                      new long[][] { new long[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(long[][].class),
                                      new long[][] {
                                          new long[] {
                                              -9223372036854775808L, 0L,
                                              9223372036854775807L } }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type("[][]unsigned hyper",
                                               TypeClass.SEQUENCE),
                                      new long[][] { new long[] {} }),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type("[][]unsigned hyper",
                                               TypeClass.SEQUENCE),
                                      new long[][] {
                                          new long[] {
                                              0L,
                                              -9223372036854775808L } }),
                              new CompareBoxed());
        success &= testMapAny(transport, new float[][] { new float[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new float[][] {
                                  new float[] { Float.NEGATIVE_INFINITY,
                                                Float.MIN_VALUE, -0.0f,
                                                0.0f, Float.MAX_VALUE,
                                                Float.POSITIVE_INFINITY,
                                                Float.NaN } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(float[][].class),
                                      new float[][] { new float[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(float[][].class),
                                      new float[][] {
                                          new float[] {
                                              Float.NEGATIVE_INFINITY,
                                              Float.MIN_VALUE, -0.0f, 0.0f,
                                              Float.MAX_VALUE,
                                              Float.POSITIVE_INFINITY,
                                              Float.NaN } }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new double[][] { new double[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new double[][] {
                                  new double[] { Double.NEGATIVE_INFINITY,
                                                 Double.MIN_VALUE, -0.0,
                                                 0.0, Double.MAX_VALUE,
                                                 Double.POSITIVE_INFINITY,
                                                 Double.NaN } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(double[][].class),
                                      new double[][] { new double[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(double[][].class),
                                      new double[][] {
                                          new double[] {
                                              Double.NEGATIVE_INFINITY,
                                              Double.MIN_VALUE, -0.0, 0.0,
                                              Double.MAX_VALUE,
                                              Double.POSITIVE_INFINITY,
                                              Double.NaN } }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new char[][] { new char[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new char[][] {
                                  new char[] { '\u0000', '\uDBFF',
                                               '\uFFFD' } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(char[][].class),
                                      new char[][] { new char[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  new Type(char[][].class),
                                  new char[][] {
                                      new char[] { '\u0000', '\uDBFF',
                                                   '\uFFFD' } }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new String[][] { new String[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new String[][] {
                                  new String[] { "", "\uD800\uDC00",
                                                 "Test" } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(String[][].class),
                                      new String[][] { new String[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(String[][].class),
                                      new String[][] {
                                          new String[] { "", "\uD800\uDC00",
                                                         "Test" } }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Type[][] { new Type[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Type[][] {
                                  new Type[] {
                                      Type.VOID,
                                      new Type(
                                          DerivedInterface.class.getName(),
                                          TypeClass.INTERFACE) } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(Type[][].class),
                                      new Type[][] { new Type[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  new Type(Type[][].class),
                                  new Type[][] {
                                      new Type[] {
                                          Type.VOID,
                                          new Type(
                                              DerivedInterface.class.
                                              getName(),
                                              TypeClass.INTERFACE) } }),
                              new CompareUnboxed());
        success &= testMapAny(transport, new Object[][] { new Object[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Object[][] {
                                  new Object[] { Any.VOID,
                                                 Boolean.FALSE } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Object[][] {
                                  new Object[] {
                                      Boolean.FALSE,
                                      new Any(Type.BOOLEAN,
                                              Boolean.TRUE) } },
                              new CompareBoxed(true));
        success &= testMapAny(transport,
                              new Any(new Type(Any[][].class),
                                      new Object[][] { new Object[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(Any[][].class),
                                      new Object[][] {
                                          new Object[] { Any.VOID,
                                                         Boolean.FALSE } }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(Any[][].class),
                                      new Object[][] {
                                          new Object[] {
                                              Boolean.FALSE,
                                              new Any(Type.BOOLEAN,
                                                      Boolean.TRUE) } }),
                              new CompareUnboxed(true));
        success &= testMapAny(transport, new Any[][] { new Any[] {} },
                              new CompareSpecific(
                                  new Object[][] { new Object[] {} }));
        success &= testMapAny(transport,
                              new Any[][] {
                                  new Any[] { Any.VOID,
                                              new Any(Type.BOOLEAN,
                                                      Boolean.TRUE) } },
                              new CompareSpecific(
                                  new Object[][] {
                                      new Object[] { Any.VOID,
                                                     Boolean.TRUE } }));
        success &= testMapAny(transport,
                              new Any(new Type(Any[][].class),
                                      new Any[][] { new Any[] {} }),
                              new CompareSpecific(
                                  new Object[][] { new Object[] {} }));
        success &= testMapAny(transport,
                              new Any(new Type(Any[][].class),
                                      new Any[][] {
                                          new Any[] {
                                              Any.VOID,
                                              new Any(Type.BOOLEAN,
                                                      Boolean.TRUE) } }),
                              new CompareSpecific(
                                  new Object[][] {
                                      new Object[] { Any.VOID,
                                                     Boolean.TRUE } }));
        success &= testMapAny(transport,
                              new Any(new Type(Any[][].class),
                                      new Boolean[][] { new Boolean[] {} }),
                              new CompareSpecific(
                                  new Object[][] { new Object[] {} }));
        success &= testMapAny(transport,
                              new Any(new Type(Any[][].class),
                                      new Boolean[][] {
                                          new Boolean[] {
                                              Boolean.FALSE } }),
                              new CompareSpecific(
                                  new Object[][] {
                                      new Object[] { Boolean.FALSE } }));
        if (createTypes) {
            success &= testMapAny(transport, new Enum1[][] { new Enum1[] {} },
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Enum1[][] {
                                      new Enum1[] { new Enum1(),
                                                    new Enum2() } },
                                  new CompareSpecific(
                                      new Enum1[][] {
                                          new Enum1[] { new Enum1(),
                                                        new Enum1() } }));
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[][].class),
                                          new Enum1[][] { new Enum1[] {} }),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[][].class),
                                          new Enum1[][] {
                                              new Enum1[] { new Enum1(),
                                                            new Enum2() } }),
                                  new CompareSpecific(
                                      new Enum1[][] {
                                          new Enum1[] { new Enum1(),
                                                        new Enum1() } }));
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[][].class),
                                          new Enum2[][] { new Enum2[] {} }),
                                  new CompareSpecific(
                                      new Enum1[][] { new Enum1[] {} }));
            success &= testMapAny(transport,
                                  new Any(new Type(Enum1[][].class),
                                          new Enum2[][] {
                                              new Enum2[] { new Enum2() } }),
                                  new CompareSpecific(
                                      new Enum1[][] {
                                          new Enum1[] { new Enum1() } }));
            success &= testMapAny(transport,
                                  new BaseStruct[][] { new BaseStruct[] {} },
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new BaseStruct[][] {
                                      new BaseStruct[] {
                                          new BaseStruct(),
                                          new DerivedStruct() } },
                                  new CompareSpecific(
                                      new BaseStruct[][] {
                                          new BaseStruct[] {
                                              new BaseStruct(),
                                              new BaseStruct() } }));
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[][].class),
                                          new BaseStruct[][] {
                                              new BaseStruct[] {} }),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[][].class),
                                          new BaseStruct[][] {
                                              new BaseStruct[] {
                                                  new BaseStruct(),
                                                  new DerivedStruct() } }),
                                  new CompareSpecific(
                                      new BaseStruct[][] {
                                          new BaseStruct[] {
                                              new BaseStruct(),
                                              new BaseStruct() } }));
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[][].class),
                                          new DerivedStruct[][] {
                                              new DerivedStruct[] {} }),
                                  new CompareSpecific(
                                      new BaseStruct[][] {
                                          new BaseStruct[] {} }));
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct[][].class),
                                          new DerivedStruct[][] {
                                              new DerivedStruct[] {
                                                  new DerivedStruct() } }),
                                  new CompareSpecific(
                                      new BaseStruct[][] {
                                          new BaseStruct[] {
                                              new BaseStruct() } }));
            success &= testMapAny(transport,
                                  new DerivedStruct[][] {
                                      new DerivedStruct[] {} },
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new DerivedStruct[][] {
                                      new DerivedStruct[] {
                                          new DerivedStruct() } },
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(DerivedStruct[][].class),
                                          new DerivedStruct[][] {
                                              new DerivedStruct[] {} }),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(DerivedStruct[][].class),
                                          new DerivedStruct[][] {
                                              new DerivedStruct[] {
                                                  new DerivedStruct() } }),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport,
                              new XInterface[][] { new XInterface[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new XInterface[][] {
                                  new XInterface[] {
                                      null, new XInterface() {},
                                      new BaseInterface() {},
                                      new DerivedInterface() {} } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[][].class),
                                      new XInterface[][] {
                                          new XInterface[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  new Type(XInterface[][].class),
                                  new XInterface[][] {
                                      new XInterface[] {
                                          null, new XInterface() {},
                                          new BaseInterface() {},
                                          new DerivedInterface() {} } }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[][].class),
                                      new Object[][] { new Object[] {} }),
                              new CompareSpecific(
                                  new XInterface[][] {
                                      new XInterface[] {} }));
        {
            XInterface if1 = new XInterface() {};
            XInterface if2 = new BaseInterface() {};
            XInterface if3 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(XInterface[][].class),
                                          new Object[][] {
                                              new Object[] { null, if1, if2,
                                                             if3 } }),
                                  new CompareSpecific(
                                      new XInterface[][] {
                                          new XInterface[] { null, if1, if2,
                                                             if3 } }));
        }
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[][].class),
                                      new BaseInterface[][] {
                                          new BaseInterface[] {} }),
                              new CompareSpecific(
                                  new XInterface[][] {
                                      new XInterface[] {} }));
        {
            BaseInterface if1 = new BaseInterface() {};
            BaseInterface if2 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(XInterface[][].class),
                                          new BaseInterface[][] {
                                              new BaseInterface[] {
                                                  null, if1, if2 } }),
                                  new CompareSpecific(
                                      new XInterface[][] {
                                          new XInterface[] {
                                              null, if1, if2 } }));
        }
        success &= testMapAny(transport,
                              new Any(new Type(XInterface[][].class),
                                      new DerivedInterface[][] {
                                          new DerivedInterface[] {} }),
                              new CompareSpecific(
                                  new XInterface[][] {
                                      new XInterface[] {} }));
        {
            DerivedInterface if1 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(XInterface[][].class),
                                          new DerivedInterface[][] {
                                              new DerivedInterface[] {
                                                  null, if1 } }),
                                  new CompareSpecific(
                                      new XInterface[][] {
                                          new XInterface[] {
                                              null, if1 } }));
        }
        success &= testMapAny(transport,
                              new BaseInterface[][] {
                                  new BaseInterface[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new BaseInterface[][] {
                                  new BaseInterface[] {
                                      null, new BaseInterface() {},
                                      new DerivedInterface() {} } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface[][].class),
                                      new BaseInterface[][] {
                                          new BaseInterface[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  new Type(BaseInterface[][].class),
                                  new BaseInterface[][] {
                                      new BaseInterface[] {
                                          null, new BaseInterface() {},
                                          new DerivedInterface() {} } }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface[][].class),
                                      new DerivedInterface[][] {
                                          new DerivedInterface[] {} }),
                              new CompareSpecific(
                                  new BaseInterface[][] {
                                      new BaseInterface[] {} }));
        {
            DerivedInterface if1 = new DerivedInterface() {};
            success &= testMapAny(transport,
                                  new Any(new Type(BaseInterface[][].class),
                                          new DerivedInterface[][] {
                                              new DerivedInterface[] {
                                                  null, if1 } }),
                                  new CompareSpecific(
                                      new BaseInterface[][] {
                                          new BaseInterface[] {
                                              null, if1 } }));
        }
        success &= testMapAny(transport,
                              new DerivedInterface[][] {
                                  new DerivedInterface[] {} },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new DerivedInterface[][] {
                                  new DerivedInterface[] {
                                      null, new DerivedInterface() {} } },
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(DerivedInterface[][].class),
                                      new DerivedInterface[][] {
                                          new DerivedInterface[] {} }),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(
                                  new Type(DerivedInterface[][].class),
                                  new DerivedInterface[][] {
                                      new DerivedInterface[] {
                                          null,
                                          new DerivedInterface() {} } }),
                              new CompareUnboxed());

        // Enum Types:
        if (createTypes) {
            success &= testMapAny(transport, new Enum1(), new CompareBoxed());
            success &= testMapAny(transport, new Any(new Type(Enum1.class),
                                                     new Enum1()),
                                  new CompareUnboxed());
            success &= testMapAny(transport, new Any(new Type(Enum1.class),
                                                     new Enum2()),
                                  new CompareSpecific(new Enum1()));
        }

        // Struct Types:
        if (createTypes) {
            success &= testMapAny(transport, new BaseStruct(),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct.class),
                                          new BaseStruct()),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(BaseStruct.class),
                                          new DerivedStruct()),
                                  new CompareSpecific(new BaseStruct()));
            success &= testMapAny(transport, new DerivedStruct(),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(DerivedStruct.class),
                                          new DerivedStruct()),
                                  new CompareUnboxed());
        }

        // Exception Types:
        success &= testMapAny(transport, new com.sun.star.uno.Exception(),
                              new CompareClass(
                                  com.sun.star.uno.Exception.class));
        success &= testMapAny(transport,
                              new Any(new Type(
                                          com.sun.star.uno.Exception.class),
                                      new com.sun.star.uno.Exception()),
                              new CompareClass(
                                  com.sun.star.uno.Exception.class));
        success &= testMapAny(transport,
                              new Any(new Type(
                                          com.sun.star.uno.Exception.class),
                                      new BaseException()),
                              new CompareClass(
                                  com.sun.star.uno.Exception.class));
        success &= testMapAny(transport,
                              new Any(new Type(
                                          com.sun.star.uno.Exception.class),
                                      new DerivedException()),
                              new CompareClass(
                                  com.sun.star.uno.Exception.class));
        if (createTypes) {
            success &= testMapAny(transport, new BaseException(),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(BaseException.class),
                                          new BaseException()),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(BaseException.class),
                                          new DerivedException()),
                                  new CompareSpecific(new BaseException()));
            success &= testMapAny(transport, new DerivedException(),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(DerivedException.class),
                                          new DerivedException()),
                                  new CompareUnboxed());
        }
        success &= testMapAny(transport,
                              new com.sun.star.uno.RuntimeException(),
                              new CompareClass(
                                  com.sun.star.uno.RuntimeException.class));
        success &= testMapAny(transport,
                              new Any(
                                  new Type(
                                      com.sun.star.uno.RuntimeException.
                                      class),
                                  new com.sun.star.uno.RuntimeException()),
                              new CompareClass(
                                  com.sun.star.uno.RuntimeException.class));
        success &= testMapAny(transport,
                              new Any(
                                  new Type(
                                      com.sun.star.uno.RuntimeException.
                                      class),
                                  new BaseRuntimeException()),
                              new CompareClass(
                                  com.sun.star.uno.RuntimeException.class));
        success &= testMapAny(transport,
                              new Any(
                                  new Type(
                                      com.sun.star.uno.RuntimeException.
                                      class),
                                  new DerivedRuntimeException()),
                              new CompareClass(
                                  com.sun.star.uno.RuntimeException.class));
        if (createTypes) {
            success &= testMapAny(transport, new BaseRuntimeException(),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(
                                              BaseRuntimeException.class),
                                          new BaseRuntimeException()),
                                  new CompareUnboxed());
            success &= testMapAny(transport,
                                  new Any(new Type(
                                              BaseRuntimeException.class),
                                          new DerivedRuntimeException()),
                                  new CompareSpecific(
                                      new BaseRuntimeException()));
            success &= testMapAny(transport, new DerivedRuntimeException(),
                                  new CompareBoxed());
            success &= testMapAny(transport,
                                  new Any(new Type(
                                              DerivedRuntimeException.class),
                                          new DerivedRuntimeException()),
                                  new CompareUnboxed());
        }

        // Interface Types:
        success &= testMapAny(transport, null, new CompareBoxed());
        success &= testMapAny(transport, new XInterface() {},
                              new CompareBoxed());
        success &= testMapAny(transport, new BaseInterface() {},
                              new CompareBoxed());
        success &= testMapAny(transport, new DerivedInterface() {},
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface.class), null),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface.class),
                                      new XInterface() {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface.class),
                                      new BaseInterface() {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(XInterface.class),
                                      new DerivedInterface() {}),
                              new CompareUnboxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface.class), null),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface.class),
                                      new BaseInterface() {}),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(BaseInterface.class),
                                      new DerivedInterface() {}),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(DerivedInterface.class),
                                      null),
                              new CompareBoxed());
        success &= testMapAny(transport,
                              new Any(new Type(DerivedInterface.class),
                                      new DerivedInterface() {}),
                              new CompareBoxed());

        // Misc:
        try {
            transport.mapAny(new Object());
            System.out.println("BAD mapAny(Object), no exception");
            success = false;
        } catch (StackOverflowError e) {
            System.out.println("BAD mapAny(Object): " + e);
            success = false;
        } catch (RuntimeException e) {}

        return success;
    }

    private TestAny() {} // do not instantiate

    private static boolean testType(Class zclass, TypeClass tclass,
                                    String tname) {
        Type t1 = new Type(zclass);
        Type t2 = new Type(tname, tclass);
        boolean ok = true;
        if (t1.getTypeClass() != tclass) {
            ok = false;
            System.out.println("BAD Type(" + zclass + ").getTypeClass() = "
                               + t1.getTypeClass() + " != " + tclass);
        }
        if (!t1.getTypeName().equals(tname)) {
            ok = false;
            System.out.println("BAD Type(" + zclass + ").getTypeName() = "
                               + t1.getTypeName() + " != " + tname);
        }
        if (!t1.equals(t2)) {
            ok = false;
            System.out.println("BAD Type(" + zclass + ") != Type(" + tname
                               + ", " + tclass + ")");
        }
        return ok;
    }

    private static boolean testMapAny(XTransport transport, Object any,
                                      Compare compare) {
        Object any2 = transport.mapAny(any);
        boolean eq = compare.equal(any, any2);
        if (!eq) {
            System.out.println("BAD mapAny(" + any + ") -> " + any2);
        }
        return eq;
    }

    private static abstract class Compare {
        public abstract boolean equal(Object o1, Object o2);
    }

    private static final class CompareBoxed extends Compare {
        public CompareBoxed() {
            this(false);
        }

        public CompareBoxed(boolean unboxInner) {
            this.unboxInner = unboxInner;
        }

        public boolean equal(Object o1, Object o2) {
            if (o1 instanceof Any) {
                return o2 instanceof Any
                    && ((Any) o1).getType().equals(((Any) o2).getType())
                    && equalValues(((Any) o1).getObject(),
                                   ((Any) o2).getObject());
            } else {
                return equalValues(o1, o2);
            }
        }

        private boolean equalValues(Object o1, Object o2) {
            if (o1 == null) {
                return o2 == null;
            } else if (o1.getClass().isArray()) {
                if (!(o2 != null && o1.getClass() == o2.getClass()
                      && Array.getLength(o1) == Array.getLength(o2))) {
                    return false;
                }
                for (int i = 0; i < Array.getLength(o1); ++i) {
                    Object oo1 = Array.get(o1, i);
                    if (unboxInner && oo1 instanceof Any) {
                        oo1 = ((Any) oo1).getObject();
                    }
                    if (!equal(oo1, Array.get(o2, i))) {
                        return false;
                    }
                }
                return true;
            } else {
                return o1.equals(o2);
            }
        }

        private final boolean unboxInner;
    }

    private static final class CompareUnboxed extends Compare {
        public CompareUnboxed() {
            this(false);
        }

        public CompareUnboxed(boolean unboxInner) {
            this.unboxInner = unboxInner;
        }

        public boolean equal(Object o1, Object o2) {
            return new CompareBoxed(unboxInner).equal(((Any) o1).getObject(),
                                                      o2);
        }

        private final boolean unboxInner;
    }

    private static final class CompareSpecific extends Compare {
        public CompareSpecific(Object specific) {
            this.specific = specific;
        }

        public boolean equal(Object o1, Object o2) {
            return new CompareBoxed().equal(specific, o2);
        }

        private final Object specific;
    }

    private static final class CompareClass extends Compare {
        public CompareClass(Class clazz) {
            this.clazz = clazz;
        }

        public boolean equal(Object o1, Object o2) {
            return o2 != null && o2.getClass() == clazz;
        }

        private final Class clazz;
    }

    public static class Enum1 extends Enum {
        public Enum1() {
            super(0);
        }

        public static Enum1 fromInt(int value) {
            return new Enum1();
        }

        public boolean equals(Object obj) {
            return obj != null && obj.getClass() == Enum1.class;
        }
    }

    public static class Enum2 extends Enum1 {
        public boolean equals(Object obj) {
            return obj != null && obj.getClass() == Enum2.class;
        }
    }

    public static class BaseStruct {
        public boolean equals(Object obj) {
            return obj != null && obj.getClass() == BaseStruct.class;
        }
    }

    public static class DerivedStruct extends BaseStruct {
        public boolean equals(Object obj) {
            return obj != null && obj.getClass() == DerivedStruct.class;
        }
    }

    public static class BaseException extends com.sun.star.uno.Exception {
        public BaseException() {}

        public BaseException(String message) {
            super(message);
        }

        public boolean equals(Object obj) {
            return obj != null && obj.getClass() == BaseException.class;
        }
    }

    public static class DerivedException extends BaseException {
        public DerivedException() {}

        public DerivedException(String message) {
            super(message);
        }

        public boolean equals(Object obj) {
            return obj != null && obj.getClass() == DerivedException.class;
        }
    }

    public static class BaseRuntimeException
        extends com.sun.star.uno.RuntimeException
    {
        public BaseRuntimeException() {}

        public BaseRuntimeException(String message) {
            super(message);
        }

        public boolean equals(Object obj) {
            return obj != null
                && obj.getClass() == BaseRuntimeException.class;
        }
    }

    public static class DerivedRuntimeException extends BaseRuntimeException
    {
        public DerivedRuntimeException() {}

        public DerivedRuntimeException(String message) {
            super(message);
        }

        public boolean equals(Object obj) {
            return obj != null
                && obj.getClass() == DerivedRuntimeException.class;
        }
    }
}
