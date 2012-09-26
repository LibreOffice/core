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

package org.openoffice.xmerge.converter.palm;

/**
 *  Contains common static methods and constants for use within the package.
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
        temp = s.charAt(0);
        id = temp << 24;

        // grab the second char and add it in.
        temp = s.charAt(1) & 0x00ff;
        id += temp << 16;

        // grab the second char and add it in.
        temp = s.charAt(2) & 0x00ff;
        id += temp << 8;

        // grab the last char and add it in
        id += s.charAt(3) & 0x00ff;

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

