package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.ChooseUninstallationComponents;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.ModuleCtrl;

public class ChooseUninstallationComponentsCtrl extends PanelController {

    private String helpFile;

    public ChooseUninstallationComponentsCtrl() {
        super("ChooseUninstallationComponents", new ChooseUninstallationComponents());
        helpFile = "String_Helpfile_ChooseUninstallationComponents";
    }

    public String getNext() {
        return new String("UninstallationImminent");
    }

    public String getPrevious() {
        return new String("ChooseUninstallationType");
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;

        InstallData data = InstallData.getInstance();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();

        if ( nextButtonPressed ) {

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "Choose UninstallationComponents: Before checkVisibleModulesUninstall");
            }

            // Check, if at least one visible module is selected for uninstallation
            data.setVisibleModulesChecked(false);
            ModuleCtrl.checkVisibleModulesUninstall(packageData, data);

            if ( ! data.visibleModulesChecked() ) {
                String message = ResourceManager.getString("String_No_Uninstallcomponents_Selected_1") + "\n" +
                                 ResourceManager.getString("String_No_Uninstallcomponents_Selected_2");
                String title = ResourceManager.getString("String_Nothing_To_Uninstall");
                Informer.showInfoMessage(message, title);
                repeatDialog = true;
            } else {
                // Check, if all visible modules are selected for uninstallation.
                // Then this shall be handled as complete uninstallation
                // -> The responsible value is InstallData.isMaskedCompleteUninstallation
                data.setMaskedCompleteUninstallation(true);
                ModuleCtrl.checkMaskedCompleteUninstallation(packageData, data);
            }
        } else {  // the back button was pressed
            // Saving typical selection state values (always if back button is pressed!).
            ModuleCtrl.saveCustomSelectionStates(packageData);
            data.setCustomSelectionStateSaved(true);
        }

        return repeatDialog;
    }

}
