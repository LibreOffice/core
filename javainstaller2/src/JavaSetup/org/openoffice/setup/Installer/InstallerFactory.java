/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InstallerFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:54:01 $
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
