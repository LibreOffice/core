/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OSHelper.java,v $
 * $Revision: 1.6 $
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

package convwatch;

class OSHelper
{
    public static boolean isWindows()
        {
            String sOSName = System.getProperty("os.name");
            if (sOSName.toLowerCase().startsWith("windows"))
            {
                return true;
            }
            return false;
        }
    // not need
    // public static boolean isUnix()
    public static boolean isSolarisIntel()
        {
            if ( ( System.getProperty("os.name").toLowerCase().startsWith("solaris") ||
                   System.getProperty("os.name").toLowerCase().startsWith("sunos") ) &&
                 System.getProperty("os.arch").equals("x86"))
            {
                return true;
            }
            return false;
        }
    public static boolean isSolarisSparc()
        {
            if ( ( System.getProperty("os.name").toLowerCase().startsWith("solaris") ||
                   System.getProperty("os.name").toLowerCase().startsWith("sunos") ) &&
                 System.getProperty("os.arch").equals("sparc"))
            {
                return true;
            }
            return false;
        }
    public static boolean isLinuxIntel()
        {
            if (System.getProperty("os.name").toLowerCase().startsWith("linux") &&
                System.getProperty("os.arch").equals("i386"))
            {
                return true;
            }
            return false;
        }

    public static boolean isUnix()
        {
            if (isLinuxIntel() ||
                isSolarisIntel() ||
                isSolarisSparc())
            {
                return true;
            }
            return false;
        }

}
