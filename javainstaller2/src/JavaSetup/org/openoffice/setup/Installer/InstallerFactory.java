/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InstallerFactory.java,v $
 * $Revision: 1.3 $
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

package org.openoffice.setup.Installer;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.Util.Informer;

public class InstallerFactory
{
    private static Installer instance = null;

    private InstallerFactory() {
    }

    public static Installer getInstance()
    {
      if (instance == null) {
          instance = createInstaller();
      }
      return instance;
    }

    private static Installer createInstaller() {

        InstallData data = InstallData.getInstance();
        if ( data.getOSType().equalsIgnoreCase("Linux")) {
            instance = new LinuxInstaller();
        } else if ( data.getOSType().equalsIgnoreCase("SunOS")) {
            instance = new SolarisInstaller();
        } else {
            System.err.println("Error: No installer for this OS defined!");
            String message = ResourceManager.getString("String_InstallerFactory_Os_Not_Supported");
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
            System.exit(1);
        }

        return instance;
    }

}
