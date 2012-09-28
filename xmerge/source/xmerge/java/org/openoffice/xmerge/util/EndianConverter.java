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


/**
 * <p>Helper class providing static methods to convert data to/from
 *    Network Byte Order (Big Endian).</p>
 *
 * <p>With the introduction of <code>java.nio.ByteOrder</code> and
 *    <code>java.nio.ByteBuffer</code> in Java 1.4, it may no longer be
 *    necessary to use this class in the future.</p>
 *
 * @version 1.1
 */
public class EndianConverter {

    /**
     * <p>Convert a <code>short</code> to a Little Endian representation.</p>
     *
     * @param   value   The <code>short</code> to be converted.
     *
     * @return  Two element <code>byte</code> array containing the converted
     *          value.
     */
    public static byte[] writeShort (short value) {
        byte[] leShort = new byte[2];

        leShort[0] = (byte) value;
        leShort[1] = (byte) (value >>> 8);

        return leShort;
    }


    /**
     * <p>Convert an integer to a Little Endian representation.</p>
     *
     * @param   value   The <code>int</code> to be converted.
     *
     * @return  Four element <code>byte</code> array containing the converted
     *          value.
     */
    public static byte[] writeInt (int value) {
        byte[] leInt = new byte[4];

        leInt[0] = (byte) value;
        leInt[1] = (byte) (value >>> 8);
        leInt[2] = (byte) (value >>> 16);
        leInt[3] = (byte) (value >>> 24);

        return leInt;
    }

    /**
     * <p>Converts a <code>double</code> to a Little Endian representation
     * of a float in IEEE-754 format.
     *
     * <p>An array with more than eight elements can be used, but only the first
     * eight elements will be read.</p>
     *
     * @param   value <code>double</code> containing the value to be converted
     *
     * @return   <code>byte</code> array containing the LE representation of a IEEE-754 float
     */
    public static byte[] writeDouble(double value) {

        long myDouble = Double.doubleToLongBits(value);
        byte[] leDouble = new byte[8];

        leDouble[0] = (byte) (myDouble >>> 0);
        leDouble[1] = (byte) (myDouble >>> 8);
        leDouble[2] = (byte) (myDouble >>> 16);
        leDouble[3] = (byte) (myDouble >>> 24);
        leDouble[4] = (byte) (myDouble >>> 32);
        leDouble[5] = (byte) (myDouble >>> 40);
        leDouble[6] = (byte) (myDouble >>> 48);
        leDouble[7] = (byte) (myDouble >>> 56);

        return leDouble;
    }

    /**
     * <p>Convert a Little Endian representation of a short to a Java
     *    <code>short</code> (Network Byte Order).</p>
     *
     * <p>An array with more than two elements can be used, but only the first
     *    two elements will be read.</p>
     *
     * @param   value   <code>byte</code> array containing the LE representation
     *                  of the value.
     *
     * @return  <code>short</code> containing the converted value.
     */
    public static short readShort (byte[] value) {
        int high, low;

        high = value[1] & 0xFF;
        low  = value[0] & 0xFF;

        return (short)(high << 8 | low);
    }


    /**
     * <p>Convert a Little Endian representation of an integer to a Java
     *    <code>int</code> (Network Byte Order).</p>
     *
     * <p>An array with more than four elements can be used, but only the first
     * four elements will be read.</p>
     *
     * @param   value   <code>byte</code> array containing the LE representation
     *                  of the value.
     *
     * @return  <code>int</code> containing the converted value.
     */
    public static int readInt(byte[] value) {
        int number = 0;

        for (int i = 0; i < 4; i++) {
            number |= (value[i] & 0xFF) << ( i * 8);
        }

        return number;
    }

    /**
     * <p>Convert a Little Endian representation of a float in IEEE-754 Little
     * Endian to a Java <code>double</code> (Network Byte Order).</p>
     *
     * <p>An array with more than eight elements can be used, but only the first
     * eight elements will be read.</p>
     *
     * @param   value   <code>byte</code> array containing the LE representation
     *                  of a IEEE-754 float.
     *
     * @return  <code>double</code> containing the converted value.
     */
    public static double readDouble(byte[] value) {

        long lvalue = ( ((long)(value[7])       << 56)  +
                        ((long)(value[6]&0xFF)  << 48)  +
                        ((long)(value[5]&0xFF)  << 40)  +
                        ((long)(value[4]&0xFF)  << 32)  +
                        ((long)(value[3]&0xFF)  << 24)  +
                        ((long)(value[2]&0xFF)  << 16)  +
                        ((long)(value[1]&0xFF)  << 8)   +
                         (value[0]&0xFF));

        return Double.longBitsToDouble(lvalue);
    }
}
