/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
