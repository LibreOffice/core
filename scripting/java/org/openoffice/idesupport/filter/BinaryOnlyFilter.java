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

package org.openoffice.idesupport.filter;

public class BinaryOnlyFilter implements FileFilter {
    private static final String[] EXTENSIONS = {".class", ".jar", ".bsh"};
    private static final String DESCRIPTION = "Executable Files Only";
    private static final BinaryOnlyFilter filter = new BinaryOnlyFilter();

    private BinaryOnlyFilter() {
    }

    public static BinaryOnlyFilter getInstance() {
        return filter;
    }
    public boolean validate(String name) {
        for (int i = 0; i < EXTENSIONS.length; i++)
            if (name.endsWith(EXTENSIONS[i]))
                return true;
        return false;
    }

    public String toString() {
        /* StringBuffer buf = new StringBuffer(DESCRIPTION + ": ");

        for (int i = 0; i < EXTENSIONS.length - 1; i++)
            buf.append("<" + EXTENSIONS[i] + "> ");
        buf.append("<" + EXTENSIONS[EXTENSIONS.length - 1] + ">");

        return buf.toString(); */
        return DESCRIPTION;
    }
}
