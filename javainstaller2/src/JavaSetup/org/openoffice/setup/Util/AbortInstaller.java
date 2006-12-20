package org.openoffice.setup.Util;

import org.openoffice.setup.InstallData;
import java.util.Vector;

public class AbortInstaller {

    private AbortInstaller() {
    }

    static public void abortInstallProcess() {
        InstallData installData = InstallData.getInstance();
        installData.setIsAbortedInstallation(true);

        if ( installData.isInstallationMode() ) {
            String log = "<b>Installation was aborted</b><br>";
            LogManager.addLogfileComment(log);
        } else {
            String log = "<b>Uninstallation was aborted</b><br>";
            LogManager.addLogfileComment(log);
        }

    }

}
