/************************************************************************
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


/**
 *  Contains common static methods and contants for use within the package.
 *
 *  @author    Herbie Ong
 */

public final class PDBUtil {

    /** difference in seconds from Jan 01, 1904 to Jan 01, 1970 */
    final static long TIME_DIFF = 2082844800;

    /** encoding scheme used */
    final static String ENCODING = "8859_1";

    /** size of a pdb header in bytes */
    final static int HEADER_SIZE = 78;

    /**
     *  This method converts a 4 letter string into the Palm ID integer.
     *
     *  It is normally used to convert the Palm creator ID string into
     *  the integer version of it.  Also use for data types, etc.
     *
     *  @param   s    4 character string.
     *  @return   int    Palm ID representing the string.
     *  @throws   ArrayIndexOutOfBoundsException    if string parameter
     *                contains less than 4 characters.
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
     *  This method converts an integer into a String given
     *  the Palm ID format.
     *
     *  @param  i   Palm id.
     *  @return  String   string representation.
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

