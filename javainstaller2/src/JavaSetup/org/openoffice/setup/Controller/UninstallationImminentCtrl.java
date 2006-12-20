package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.UninstallationImminent;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.ProductDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.InfoCtrl;
import org.openoffice.setup.Util.ModuleCtrl;

public class UninstallationImminentCtrl extends PanelController {

    private String helpFile;
    private String htmlInfoText = "";

    public UninstallationImminentCtrl() {
        super("UninstallationImminent", new UninstallationImminent());
        helpFile = "String_Helpfile_UninstallationImminent";
    }

    public String getNext() {
        return new String("UninstallationOngoing");
    }

    public String getPrevious() {

        InstallData data = InstallData.getInstance();

        if ( data.getInstallationType().equals(data.getCustomActionCommand()) ) {
            return new String("ChooseUninstallationComponents");
        } else if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {
            return new String("ChooseUninstallationType");
        } else {
            System.err.println("Error: Unknown uninstallation type!" );
            return new String("Error");
        }
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {
        String StringInstall = ResourceManager.getString("String_Uninstall");
        getSetupFrame().setButtonText(StringInstall, getSetupFrame().BUTTON_NEXT);

        ProductDescription productData = SetupDataProvider.getProductDescription();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();

        htmlInfoText = InfoCtrl.setHtmlFrame("header", htmlInfoText);
        htmlInfoText = InfoCtrl.setReadyToInstallInfoText(productData, htmlInfoText);
        htmlInfoText = InfoCtrl.setReadyToInstallInfoText(packageData, htmlInfoText);
        htmlInfoText = InfoCtrl.setHtmlFrame("end", htmlInfoText);

        UninstallationImminent panel = (UninstallationImminent)getPanel();
        panel.setInfoText(htmlInfoText);

        // System.err.println("\nUninstallation module state dump 3:");
        // Dumper.dumpModuleStates(packageData);
}

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;

        if ( nextButtonPressed ) {
            PackageDescription packageData = SetupDataProvider.getPackageDescription();
            ModuleCtrl.setHiddenModuleSettingsUninstall(packageData);
            // Dumper.dumpUninstallPackages(packageData);

            // System.err.println("\nUninstallation module state dump 4:");
            // Dumper.dumpModuleStates(packageData);
        }

        return repeatDialog;
    }

}
