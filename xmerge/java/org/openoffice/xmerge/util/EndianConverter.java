/************************************************************************
 *
 *  EndianConverter.java
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


package org.openoffice.xmerge.util;


/**
 * <p>Helper class providing static methods to convert data to/from
 *    Network Byte Order (Big Endian).</p>
 *
 * <p>With the introduction of <code>java.nio.ByteOrder</code> and
 *    <code>java.nio.ByteBuffer</code> in Java 1.4, it may no longer be
 *    necessary to use this class in the future.</p>
 *
 * @author  Mark Murnane
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
        short high, low;

        high = value[1];
        low  = value[0];

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
            number |= value[i] << ( i* 8);
        }

        return number;
    }
}