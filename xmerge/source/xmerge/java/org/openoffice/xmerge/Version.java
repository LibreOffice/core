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

package org.openoffice.xmerge;

/**
 * This class provides a quick utility to check the version of the jar file.
 *
 * <p>It has a main method that prints out the version info.  It also provides
 * two static methods for runtime classes to query.</p>
 */
public final class Version {

    private static final Version version = new Version();

    private static final Package pkg = version.getClass().getPackage();

    /**
     * Private constructor to provide a singleton instance.
     */
    private Version() {
    }

    /**
     * Returns specification version.
     *
     * @return  The specification version.
     */
    public static String getSpecificationVersion() {
        return pkg.getSpecificationVersion();
    }

    /**
     * Returns implementation version.
     *
     * @return  The implementation version.
     */
    public static String getImplementationVersion() {
        return pkg.getImplementationVersion();
    }

    /**
     * Main method for printing out version info.
     *
     * @param  args  Array of arguments, not used.
     */
    public static void main(String args[]) {
        System.out.println("Specification-Title: " + pkg.getSpecificationTitle());
        System.out.println("Specification-Vendor: " + pkg.getSpecificationVendor());
        System.out.println("Specification-Version: " + pkg.getSpecificationVersion());
        System.out.println("Implementation-Version: " + pkg.getImplementationVersion());
    }
}