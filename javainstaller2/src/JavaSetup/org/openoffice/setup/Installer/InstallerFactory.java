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
