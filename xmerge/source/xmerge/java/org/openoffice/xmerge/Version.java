/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Version.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:21:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

