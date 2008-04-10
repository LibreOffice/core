/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Calculator.java,v $
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
