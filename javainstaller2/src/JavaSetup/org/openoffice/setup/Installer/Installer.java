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
import org.openoffice.setup.SetupData.PackageDescription;

public abstract class Installer {

    public Installer() {
    }

    public void preInstall(PackageDescription packageData) {
    }

    public void postInstall(PackageDescription packageData) {
    }

    public void preUninstall(PackageDescription packageData) {
    }

    public void postUninstall(PackageDescription packageData) {
    }

    public void preInstallationOngoing () {
    }

    public void postInstallationOngoing () {
    }

    public void postUninstallationOngoing () {
    }

    public void defineDatabasePath() {
    }

    public String getChangeInstallDir(PackageDescription packageData) {
        return null;
    }

    public boolean isPackageNameInstalled(String packageName, InstallData installData) {
        return false;
    }

    public abstract void installPackage(PackageDescription packageData);

    public abstract void uninstallPackage(PackageDescription packageData);

    public abstract boolean isPackageInstalled(PackageDescription packageData, InstallData installData);

    public abstract boolean isInstalledPackageOlder(PackageDescription packageData, InstallData installData);

    public abstract boolean isInstallSetPackageOlder(PackageDescription packageData, InstallData installData);
}
