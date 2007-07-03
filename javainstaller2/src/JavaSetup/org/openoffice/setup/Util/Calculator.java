/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Calculator.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 12:00:02 $
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

package org.openoffice.setup.Util;

import org.openoffice.setup.SetupData.PackageDescription;
import java.util.Vector;
import org.openoffice.setup.InstallData;
import org.openoffice.setup.ResourceManager;

public class Calculator {

    private Calculator() {
    }

    static private int calculateInstallSize(Vector allPackages) {

        int value = 0;

        for (int i = 0; i < allPackages.size(); i++) {
            PackageDescription packageData = (PackageDescription)allPackages.get(i);
            int size = packageData.getSize();
            value = value + size;
        }

        return value;
    }

    static private boolean missingDiscSpace(int required, int available) {
        boolean missingDiscSpace = true;

        if ( required < available ) {
            missingDiscSpace = false;
        }

        // missingDiscSpace = true; // for testing reasons
        return missingDiscSpace;
    }

    static public boolean notEnoughDiscSpace(InstallData data) {

        Vector installPackages = data.getInstallPackages();
        // Calculate size of selected modules
        int installationSize = calculateInstallSize(installPackages);

        // Compare with available space
        int availableDiscSpace = data.getAvailableDiscSpace();

        // Show warning and repeat dialog, if not sufficient disc space available
        boolean insufficientDiscSpace = missingDiscSpace(installationSize, availableDiscSpace);

        if ( insufficientDiscSpace ) {
            String message = ResourceManager.getString("String_Discspace_Insufficient") + "\n" +
                             ResourceManager.getString("String_Discspace_Required") + ": " + installationSize + " kB." + "\n" +
                             ResourceManager.getString("String_Discspace_Available") + ": " + availableDiscSpace + " kB." + "\n" +
                             ResourceManager.getString("String_Discspace_Tip");
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
        }

        return insufficientDiscSpace;
    }

}
