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

    public abstract void installPackage(PackageDescription packageData);

    public abstract void uninstallPackage(PackageDescription packageData);

    public abstract boolean isPackageInstalled(PackageDescription packageData, InstallData installData);

    public abstract boolean isInstalledPackageOlder(PackageDescription packageData, InstallData installData);

    public abstract boolean isInstallSetPackageOlder(PackageDescription packageData, InstallData installData);
}
