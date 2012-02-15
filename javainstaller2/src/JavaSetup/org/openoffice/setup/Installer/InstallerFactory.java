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
