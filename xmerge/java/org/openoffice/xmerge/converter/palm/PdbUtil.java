/************************************************************************
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


package org.openoffice.xmerge.converter.palm;

/**
 *  Contains common static methods and constants for use within the package.
 *
 *  @author    Herbie Ong
 */
public final class PdbUtil {

    /**  Difference in seconds from Jan 01, 1904 to Jan 01, 1970. */
    final static long TIME_DIFF = 2082844800;

    /**  Encoding scheme used. */
    final static String ENCODING = "8859_1";

    /**  Size of a PDB header in bytes. */
    final static int HEADER_SIZE = 78;


    /**
     *  <p>This method converts a 4 letter string into the Palm ID
     *  integer.</p>
     *
     *  <p>It is normally used to convert the Palm creator ID string into
     *  the integer version of it.  Also use for data types, etc.</p>
     *
     *  @param  s  Four character <code>String</code>.
     *
     *  @return  Palm ID representing the <code>String</code>.
     *
     *  @throws  ArrayIndexOutOfBoundsException  If <code>String</code>
     *           parameter contains less than four characters.
     */
    public static int intID(String s) {

        int id = -1;
        int temp = 0;

        // grab the first char and put it in the high bits
        // note that we only want 8 lower bits of it.
        temp = (int) s.charAt(0);
        id = temp << 24;

        // grab the second char and add it in.
        temp = ((int) s.charAt(1)) & 0x00ff;
        id += temp << 16;

        // grab the second char and add it in.
        temp = ((int) s.charAt(2)) & 0x00ff;
        id += temp << 8;

        // grab the last char and add it in
        id += ((int) s.charAt(3)) & 0x00ff;

        return id;
    }


    /**
     *  This method converts an integer into a <code>String</code>
     *  given the Palm ID format.
     *
     *  @param  i  Palm ID.
     *
     *  @return  <code>String</code> representation.
     */
    public static String stringID(int i) {

        char ch[] = new char[4];
        ch[0] = (char) (i >>> 24);
        ch[1] = (char) ((i >> 16) & 0x00ff);
        ch[2] = (char) ((i >> 8) & 0x00ff);
        ch[3] = (char) (i & 0x00ff);

        return new String(ch);
    }
}

