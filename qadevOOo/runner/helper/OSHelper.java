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

package helper;

/**
 * Helper object, to identify the current Operation System.
 */
public class OSHelper
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
