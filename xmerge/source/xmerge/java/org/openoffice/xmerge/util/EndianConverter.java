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

package org.openoffice.xmerge.util;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Helper class providing static methods to convert data to/from Network Byte
 * Order (Big Endian).
 *
 * <p>With the introduction of {@code java.nio.ByteOrder} and
 * {@code java.nio.ByteBuffer} in Java 1.4, it may no longer be necessary to use
 * this class in the future.</p>
 *
 * @version 1.1
 */
public class EndianConverter {

    /**
     * Convert a {@code short} to a Little Endian representation.
     *
     * @param   value   The {@code short} to be converted.
     *
     * @return  Two element {@code byte} array containing the converted value.
     */
    public static byte[] writeShort (short value) {
        return ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)
            .putShort(value).array();
    }

    /**
     * Convert an integer to a Little Endian representation.
     *
     * @param   value  The {@code int} to be converted.
     *
     * @return  Four element {@code byte} array containing the converted value.
     */
    public static byte[] writeInt (int value) {
        return ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN)
            .putInt(value).array();
    }

    /**
     * Converts a {@code double} to a Little Endian representation of a float in
     * IEEE-754 format.
     *
     * <p>An array with more than eight elements can be used, but only the first
     * eight elements will be read.</p>
     *
     * @param   value {@code double} containing the value to be converted.
     *
     * @return  {@code byte} array containing the LE representation of a
     *          IEEE-754 float.
     */
    public static byte[] writeDouble(double value) {
        return ByteBuffer.allocate(8).order(ByteOrder.LITTLE_ENDIAN).putLong(
            Double.doubleToLongBits(value)).array();
    }

    /**
     * Convert a Little Endian representation of a short to a Java {@code short}
     * (Network Byte Order).
     *
     * <p>An array with more than two elements can be used, but only the first
     * two elements will be read.</p>
     *
     * @param   value  {@code byte} array containing the LE representation of
     *                 the value.
     *
     * @return  {@code short} containing the converted value.
     */
    public static short readShort (byte[] value) {
        return ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)
            .put(value).getShort(0);
    }

    /**
     * Convert a Little Endian representation of an integer to a Java {@code int}
     * (Network Byte Order).
     *
     * <p>An array with more than four elements can be used, but only the first
     * four elements will be read.</p>
     *
     * @param   value  {@code byte} array containing the LE representation of
     *                 the value.
     *
     * @return  {@code int} containing the converted value.
     */
    public static int readInt(byte[] value) {
        return ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN)
            .put(value).getInt(0);
    }

    /**
     * Convert a Little Endian representation of a float in IEEE-754 Little
     * Endian to a Java {@code double} (Network Byte Order).
     *
     * <p>An array with more than eight elements can be used, but only the first
     * eight elements will be read.</p>
     *
     * @param   value  {@code byte} array containing the LE representation of an
     *                 IEEE-754 float.
     *
     * @return  {@code double} containing the converted value.
     */
    public static double readDouble(byte[] value) {
        return Double.longBitsToDouble(
            ByteBuffer.allocate(8).order(ByteOrder.LITTLE_ENDIAN).put(value)
                .getLong(0));
    }
}