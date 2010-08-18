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

package com.sun.star.lib.util;

/** jurt.jar internal string helper methods.
 */
public final class StringHelper
{
    private StringHelper() {} // do not instantiate

    public static String replace(String str, char from, String to) {
        StringBuffer b = new StringBuffer();
        for (int i = 0;;) {
            int j = str.indexOf(from, i);
            if (j == -1) {
                b.append(str.substring(i));
                break;
            } else {
                b.append(str.substring(i, j));
                b.append(to);
                i = j + 1;
            }
        }
        return b.toString();
    }
}
