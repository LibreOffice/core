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

import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.Enum;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.IFieldDescription;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;

import java.io.ByteArrayOutputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;

final class Marshal {
    public Marshal(IBridge bridge, short cacheSize) {
        this.bridge = bridge;
        objectIdCache = new Cache(cacheSize);
        threadIdCache = new Cache(cacheSize);
        typeCache = new Cache(cacheSize);
    }

    public void write8Bit(int value) {
        try {
            output.writeByte(value);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void write16Bit(int value) {
        try {
            output.writeShort(value);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void writeObjectId(String objectId) {
        try {
            if (objectId == null) {
                writeStringValue(null);
                write16Bit(0xFFFF);
            } else {
                boolean[] found = new boolean[1];
                int index = objectIdCache.add(found, objectId);
                writeStringValue(found[0] ? null : objectId);
                write16Bit(index);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void writeInterface(XInterface object, Type type) {
        writeObjectId((String) bridge.mapInterfaceTo(object, type));
    }

    public void writeThreadId(ThreadId threadId) {
        try {
            byte[] data = threadId.getBytes();
            boolean[] found = new boolean[1];
            int index = threadIdCache.add(found, data);
            if (found[0]) {
                writeCompressedNumber(0);
            } else {
                writeCompressedNumber(data.length);
                writeBytes(data);
            }
            write16Bit(index);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void writeType(TypeDescription type) {
        try {
            TypeClass typeClass = type.getTypeClass();
            if (TypeDescription.isTypeClassSimple(typeClass)) {
                write8Bit(typeClass.getValue());
            } else {
                boolean[] found = new boolean[1];
                int index = typeCache.add(found, type.getTypeName());
                write8Bit(typeClass.getValue() | (found[0] ? 0 : 0x80));
                write16Bit(index);
                if (!found[0]) {
                    writeStringValue(type.getTypeName());
                }
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void writeValue(TypeDescription type, Object value) {
        try {
            switch(type.getTypeClass().getValue()) {
            case TypeClass.VOID_value:
                break;

            case TypeClass.BOOLEAN_value:
                writeBooleanValue((Boolean) value);
                break;

            case TypeClass.BYTE_value:
                writeByteValue((Byte) value);
                break;

            case TypeClass.SHORT_value:
            case TypeClass.UNSIGNED_SHORT_value:
                writeShortValue((Short) value);
                break;

            case TypeClass.LONG_value:
            case TypeClass.UNSIGNED_LONG_value:
                writeLongValue((Integer) value);
                break;

            case TypeClass.HYPER_value:
            case TypeClass.UNSIGNED_HYPER_value:
                writeHyperValue((Long) value);
                break;

            case TypeClass.FLOAT_value:
                writeFloatValue((Float) value);
                break;

            case TypeClass.DOUBLE_value:
                writeDoubleValue((Double) value);
                break;

            case TypeClass.CHAR_value:
                writeCharValue((Character) value);
                break;

            case TypeClass.STRING_value:
                writeStringValue((String) value);
                break;

            case TypeClass.TYPE_value:
                writeTypeValue((Type) value);
                break;

            case TypeClass.ANY_value:
                writeAnyValue(value);
                break;

            case TypeClass.SEQUENCE_value:
                writeSequenceValue(type, value);
                break;

            case TypeClass.ENUM_value:
                writeEnumValue(type, (Enum) value);
                break;

            case TypeClass.STRUCT_value:
                writeStructValue(type, value);
                break;

            case TypeClass.EXCEPTION_value:
                writeExceptionValue(type, (Exception) value);
                break;

            case TypeClass.INTERFACE_value:
                writeInterfaceValue(type, (XInterface) value);
                break;

            default:
                throw new IllegalArgumentException("Bad type descriptor " + type);
            }
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

    public byte[] reset() {
        byte[] data = buffer.toByteArray();
        buffer.reset();
        return data;
    }

    private void writeBooleanValue(Boolean value) throws IOException {
        output.writeBoolean(value != null && value.booleanValue());
    }

    private void writeByteValue(Byte value) {
        write8Bit(value == null ? 0 : value.byteValue());
    }

    private void writeShortValue(Short value) {
        write16Bit(value == null ? 0 : value.shortValue());
    }

    private void writeLongValue(Integer value) throws IOException {
        write32Bit(value == null ? 0 : value.intValue());
    }

    private void writeHyperValue(Long value) throws IOException {
        output.writeLong(value == null ? 0 : value.longValue());
    }

    private void writeFloatValue(Float value) throws IOException {
        output.writeFloat(value == null ? 0 : value.floatValue());
    }

    private void writeDoubleValue(Double value) throws IOException {
        output.writeDouble(value == null ? 0 : value.doubleValue());
    }

    private void writeCharValue(Character value) throws IOException {
        output.writeChar(value == null ? 0 : value.charValue());
    }

    private void writeStringValue(String value) throws IOException {
        if (value == null) {
            writeCompressedNumber(0);
        } else {
            byte[] data = value.getBytes("UTF8");
            writeCompressedNumber(data.length);
            writeBytes(data);
        }
    }

    private void writeTypeValue(Type value) throws ClassNotFoundException {
        writeType(
            TypeDescription.getTypeDescription(
                value == null ? Type.VOID : value));
    }

    private void writeAnyValue(Object value) throws ClassNotFoundException {
        TypeDescription type;
        if (value == null || value instanceof XInterface) {
            type = TypeDescription.getTypeDescription(XInterface.class);
        } else if (value instanceof Any) {
            Any any = (Any) value;
            type = TypeDescription.getTypeDescription(any.getType());
            value = any.getObject();
        } else if (value.getClass() == Object.class) {
            // Avoid StackOverflowError:
            throw new IllegalArgumentException(
                "Object instance does not represent UNO value");
        } else {
            type = TypeDescription.getTypeDescription(value.getClass());
        }
        writeType(type);
        writeValue(type, value);
    }

    private void writeSequenceValue(TypeDescription type, Object value) throws IOException {
        if (value == null) {
            writeCompressedNumber(0);
        } else {
            TypeDescription ctype = (TypeDescription) type.getComponentType();
            if (ctype.getTypeClass() == TypeClass.BYTE) {
                byte[] data = (byte[]) value;
                writeCompressedNumber(data.length);
                writeBytes(data);
            } else {
                int len = Array.getLength(value);
                writeCompressedNumber(len);
                for (int i = 0; i < len; ++i) {
                    writeValue(ctype, Array.get(value, i));
                }
            }
        }
    }

    private void writeEnumValue(TypeDescription type, Enum value) throws IllegalAccessException, IOException, InvocationTargetException, NoSuchMethodException {
        int n;
        if (value == null) {
            n = ((Enum)
                 (type.getZClass().getMethod("getDefault", (Class[]) null).
                  invoke(null, (Object[]) null))).
                getValue();
        } else {
            n = value.getValue();
        }
        write32Bit(n);
    }

    private void writeStructValue(TypeDescription type, Object value) throws IllegalAccessException {
        IFieldDescription[] fields = type.getFieldDescriptions();
        for (int i = 0; i < fields.length; ++i) {
            writeValue(
                (TypeDescription) fields[i].getTypeDescription(),
                value == null ? null : fields[i].getField().get(value));
        }
    }

    private void writeExceptionValue(TypeDescription type, Exception value) throws IllegalAccessException, IOException {
        writeStringValue(value == null ? null : value.getMessage());
        writeStructValue(type, value);
    }

    private void writeInterfaceValue(TypeDescription type, XInterface value) {
        writeInterface(value, new Type(type));
    }

    private void write32Bit(int value) throws IOException {
        output.writeInt(value);
    }

    private void writeCompressedNumber(int number) throws IOException {
        if (number >= 0 && number < 0xFF) {
            write8Bit(number);
        } else {
            write8Bit(0xFF);
            write32Bit(number);
        }
    }

    private void writeBytes(byte[] data) throws IOException {
        output.write(data);
    }

    private final ByteArrayOutputStream buffer = new ByteArrayOutputStream();
    private final DataOutput output = new DataOutputStream(buffer);
    private final IBridge bridge;
    private final Cache objectIdCache;
    private final Cache threadIdCache;
    private final Cache typeCache;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
