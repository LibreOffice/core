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

package org.openoffice.xmerge;

/**
 *  This class provides a quick utility to check the version of the
 *  jar file.  It has a main method that prints out the version
 *  info.  It also provides two static methods for runtime classes
 *  to query.
 *
 *  @author    Herbie Ong
 */
public final class Version {

    private static Package pkg;

    private static Version version;

    static {

        version = new Version();
        pkg = version.getClass().getPackage();
    }

    /**
     *  Private constructor to provide a singleton instance.
     */
    private Version() {
    }

    /**
     *  Returns specification version.
     *
     *  @return  The specification version.
     */
    public static String getSpecificationVersion() {
        return pkg.getSpecificationVersion();
    }

    /**
     *  Returns implementation version.
     *
     *  @return  The implementation version.
     */

    public static String getImplementationVersion() {
        return pkg.getImplementationVersion();
    }

    /**
     *  Main method for printing out version info.
     *
     *  @param  args  Array of arguments, not used.
     */
    public static void main(String args[]) {

        System.out.println("Specification-Title: " + pkg.getSpecificationTitle());
        System.out.println("Specification-Vendor: " + pkg.getSpecificationVendor());
        System.out.println("Specification-Version: " + pkg.getSpecificationVersion());
        System.out.println("Implementation-Version: " + pkg.getImplementationVersion());
    }
}

