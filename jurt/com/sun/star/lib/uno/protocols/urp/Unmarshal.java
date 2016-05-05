/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
package com.sun.star.lib.uno.protocols.urp;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;

import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.Enum;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.IFieldDescription;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;

final class Unmarshal {
    public Unmarshal(IBridge bridge, int cacheSize) {
        this.bridge = bridge;
        objectIdCache = new String[cacheSize];
        threadIdCache = new ThreadId[cacheSize];
        typeCache = new TypeDescription[cacheSize];
        reset(new byte[0]);
    }

    public int read8Bit() {
        try {
            return input.readUnsignedByte();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public int read16Bit() {
        try {
            return input.readUnsignedShort();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public String readObjectId() {
        try {
            String id = readStringValue();
            int index = read16Bit();
            if (index == 0xFFFF) {
                if (id.length() == 0) {
                    id = null;
                }
            } else {
                if (id.length() == 0) {
                    id = objectIdCache[index];
                } else {
                    objectIdCache[index] = id;
                }
            }
            return id;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public Object readInterface(Type type) {
        String id = readObjectId();
        return id == null ? null : bridge.mapInterfaceFrom(id, type);
    }

    public ThreadId readThreadId() {
        try {
            int len = readCompressedNumber();
            byte[] data  ;
            ThreadId id = null;
            if (len != 0) {
                data = new byte[len];
                readBytes(data);
                id = new ThreadId(data);
            }
            int index = read16Bit();
            if (index != 0xFFFF) {
                if (len == 0) {
                    id = threadIdCache[index];
                } else {
                    threadIdCache[index] = id;
                }
            }
            return id;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public TypeDescription readType() {
        int b = read8Bit();
        TypeClass typeClass = TypeClass.fromInt(b & 0x7F);
        if (typeClass == null) {
            throw new RuntimeException(
                "Reading TYPE with bad type class " + (b & 0x7F));
        }
        if (TypeDescription.isTypeClassSimple(typeClass)) {
            if ((b & 0x80) != 0) {
                throw new RuntimeException(
                    "Reading TYPE with bad type class/cache flag " + b);
            }
            return TypeDescription.getTypeDescription(typeClass);
        } else {
            int index = read16Bit();
            TypeDescription type;
            if ((b & 0x80) == 0) {
                if (index >= typeCache.length) {
                    throw new RuntimeException(
                        "Reading TYPE with bad cache index " + index);
                }
                type = typeCache[index];
                if (type == null) {
                    throw new RuntimeException(
                        "Reading TYPE with empty cache index " + index);
                }
            } else {
                try {
                    type = TypeDescription.getTypeDescription(
                        readStringValue());
                } catch (IOException e) {
                    throw new RuntimeException(e);
                } catch (ClassNotFoundException e) {
                    throw new RuntimeException(e);
                }
                if (index != 0xFFFF) {
                    if (index >= typeCache.length) {
                        throw new RuntimeException(
                            "Reading TYPE with bad cache index " + index);
                    }
                    typeCache[index] = type;
                }
            }
            return type;
        }
    }

    public Object readValue(TypeDescription type) {
        try {
            switch (type.getTypeClass().getValue()) {
            case TypeClass.VOID_value:
                return null;

            case TypeClass.BOOLEAN_value:
                return readBooleanValue();

            case TypeClass.BYTE_value:
                return readByteValue();

            case TypeClass.SHORT_value:
            case TypeClass.UNSIGNED_SHORT_value:
                return readShortValue();

            case TypeClass.LONG_value:
            case TypeClass.UNSIGNED_LONG_value:
                return readLongValue();

            case TypeClass.HYPER_value:
            case TypeClass.UNSIGNED_HYPER_value:
                return readHyperValue();

            case TypeClass.FLOAT_value:
                return readFloatValue();

            case TypeClass.DOUBLE_value:
                return readDoubleValue();

            case TypeClass.CHAR_value:
                return readCharValue();

            case TypeClass.STRING_value:
                return readStringValue();

            case TypeClass.TYPE_value:
                return readTypeValue();

            case TypeClass.ANY_value:
                return readAnyValue();

            case TypeClass.SEQUENCE_value:
                return readSequenceValue(type);

            case TypeClass.ENUM_value:
                return readEnumValue(type);

            case TypeClass.STRUCT_value:
                return readStructValue(type);

            case TypeClass.EXCEPTION_value:
                return readExceptionValue(type);

            case TypeClass.INTERFACE_value:
                return readInterfaceValue(type);

            default:
                throw new IllegalArgumentException("Bad type descriptor " + type);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public boolean hasMore() {
        try {
            return input.available() > 0;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void reset(byte[] data) {
        input = new DataInputStream(new ByteArrayInputStream(data));
    }

    private Boolean readBooleanValue() throws IOException {
       return input.readBoolean() ? Boolean.TRUE : Boolean.FALSE;
    }

    private Byte readByteValue() throws IOException {
       return Byte.valueOf(input.readByte());
    }

    private Short readShortValue() throws IOException {
        return Short.valueOf(input.readShort());
    }

    private Integer readLongValue() throws IOException {
        return Integer.valueOf(input.readInt());
    }

    private Long readHyperValue() throws IOException {
        return Long.valueOf(input.readLong());
    }

    private Float readFloatValue() throws IOException {
        return new Float(input.readFloat());
    }

    private Double readDoubleValue() throws IOException {
        return new Double(input.readDouble());
    }

    private Character readCharValue() throws IOException {
        return new Character(input.readChar());
    }

    private String readStringValue() throws IOException {
        int len = readCompressedNumber();
        byte[] data = new byte[len];
        readBytes(data);
        try {
            return new String(data, "UTF8");
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    private Type readTypeValue() {
        return new Type(readType());
    }

    private Object readAnyValue() throws IOException {
        TypeDescription type = readType();
        switch (type.getTypeClass().getValue()) {
        case TypeClass.VOID_value:
            return Any.VOID;

        case TypeClass.BOOLEAN_value:
            return readBooleanValue();

        case TypeClass.BYTE_value:
            return readByteValue();

        case TypeClass.SHORT_value:
            return readShortValue();

        case TypeClass.UNSIGNED_SHORT_value:
            return new Any(Type.UNSIGNED_SHORT, readShortValue());

        case TypeClass.LONG_value:
            return readLongValue();

        case TypeClass.UNSIGNED_LONG_value:
            return new Any(Type.UNSIGNED_LONG, readLongValue());

        case TypeClass.HYPER_value:
            return readHyperValue();

        case TypeClass.UNSIGNED_HYPER_value:
            return new Any(Type.UNSIGNED_HYPER, readHyperValue());

        case TypeClass.FLOAT_value:
            return readFloatValue();

        case TypeClass.DOUBLE_value:
            return readDoubleValue();

        case TypeClass.CHAR_value:
            return readCharValue();

        case TypeClass.STRING_value:
            return readStringValue();

        case TypeClass.TYPE_value:
            return readTypeValue();

        case TypeClass.SEQUENCE_value:
            {
                Object value = readSequenceValue(type);
                TypeDescription ctype = (TypeDescription)
                    type.getComponentType();
                while (ctype.getTypeClass() == TypeClass.SEQUENCE) {
                    ctype = (TypeDescription) ctype.getComponentType();
                }
                switch (ctype.getTypeClass().getValue()) {
                case TypeClass.UNSIGNED_SHORT_value:
                case TypeClass.UNSIGNED_LONG_value:
                case TypeClass.UNSIGNED_HYPER_value:
                    return new Any(new Type(type), value);

                case TypeClass.STRUCT_value:
                    if (ctype.hasTypeArguments()) {
                        return new Any(new Type(type), value);
                    }
                default:
                    return value;
                }
            }

        case TypeClass.ENUM_value:
            return readEnumValue(type);

        case TypeClass.STRUCT_value:
            {
                Object value = readStructValue(type);
                return type.hasTypeArguments()
                    ? new Any(new Type(type), value) : value;
            }

        case TypeClass.EXCEPTION_value:
            return readExceptionValue(type);

        case TypeClass.INTERFACE_value:
            {
                Object value = readInterfaceValue(type);
                return type.getZClass() == XInterface.class
                    ? value : new Any(new Type(type), value);
            }

        default:
            throw new RuntimeException(
                "Reading ANY with bad type " + type.getTypeClass());
        }
    }

    private Object readSequenceValue(TypeDescription type) throws IOException {
        int len = readCompressedNumber();
        TypeDescription ctype = (TypeDescription) type.getComponentType();
        if (ctype.getTypeClass() == TypeClass.BYTE) {
            byte[] data = new byte[len];
            readBytes(data);
            return data;
        } else {
            Object value = Array.newInstance(
                ctype.getTypeClass() == TypeClass.ANY
                ? Object.class : ctype.getZClass(), len);
            for (int i = 0; i < len; ++i) {
                Array.set(value, i, readValue(ctype));
            }
            return value;
        }
    }

    private Enum readEnumValue(TypeDescription type) throws IOException {
        try {
            return (Enum)
                type.getZClass().getMethod(
                    "fromInt", new Class[] { int.class }).
                invoke(null, new Object[] { readLongValue() });
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

    private Object readStructValue(TypeDescription type) {
        Object value;
        try {
            value = type.getZClass().newInstance();
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InstantiationException e) {
            throw new RuntimeException(e);
        }
        readFields(type, value);
        return value;
    }

    private Exception readExceptionValue(TypeDescription type) throws IOException {
        Exception value;
        try {
            value = (Exception)
                type.getZClass().getConstructor(new Class[] { String.class }).
                newInstance(new Object[] { readStringValue() });
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InstantiationException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
        readFields(type, value);
        return value;
    }

    private Object readInterfaceValue(TypeDescription type) {
        return readInterface(new Type(type));
    }

    private int readCompressedNumber() throws IOException {
        int number = read8Bit();
        return number < 0xFF ? number : input.readInt();
    }

    private void readBytes(byte[] data) throws IOException {
        input.readFully(data);
    }

    private void readFields(TypeDescription type, Object value) {
        IFieldDescription[] fields = type.getFieldDescriptions();
        for (int i = 0; i < fields.length; ++i) {
            try {
                fields[i].getField().set(
                    value,
                    readValue(
                        (TypeDescription) fields[i].getTypeDescription()));
            } catch (IllegalAccessException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private final IBridge bridge;
    private final String[] objectIdCache;
    private final ThreadId[] threadIdCache;
    private final TypeDescription[] typeCache;
    private DataInputStream input;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
