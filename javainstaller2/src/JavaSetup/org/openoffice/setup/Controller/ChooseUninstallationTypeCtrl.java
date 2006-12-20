package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.ChooseUninstallationType;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.ModuleCtrl;

public class ChooseUninstallationTypeCtrl extends PanelController implements ActionListener {

    private String helpFile;

    public ChooseUninstallationTypeCtrl() {
        super("ChooseUninstallationType", new ChooseUninstallationType());
        helpFile = "String_Helpfile_ChooseUninstallationType";
    }

    public String getNext() {

        InstallData data = InstallData.getInstance();

        if ( data.getInstallationType().equals(data.getCustomActionCommand()) ) {
            return new String("ChooseUninstallationComponents");
        } else if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {
            return new String("UninstallationImminent");
        } else {
            System.err.println("Error: Unknown uninstallation type!" );
            return new String("Error");
        }
    }

    public String getPrevious() {
        return new String("UninstallationPrologue");
    }

    public void beforeShow() {

        InstallData data = InstallData.getInstance();

        ChooseUninstallationType panel = (ChooseUninstallationType)getPanel();
        panel.setActionListener((ChooseUninstallationTypeCtrl)this);
        panel.setCompleteActionCommand(data.getTypicalActionCommand());
        panel.setCustomActionCommand(data.getCustomActionCommand());
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;

        ChooseUninstallationType panel = (ChooseUninstallationType)getPanel();
        panel.removeActionListener((ChooseUninstallationTypeCtrl)this);

        if ( nextButtonPressed ) {

            InstallData data = InstallData.getInstance();
            PackageDescription packageData = SetupDataProvider.getPackageDescription();

            // Typical uninstallation type
            if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {
                // If typical selection state values have been saved before,
                // it is now time to restore them

                if ( data.typicalSelectionStateSaved()) {
                    ModuleCtrl.restoreTypicalSelectionStates(packageData);
                }
            }

            // Custom uninstallation type
            if ( data.getInstallationType().equals(data.getCustomActionCommand())) {
                // Saving typical selection state values, if they are not already saved.
                if ( ! data.typicalSelectionStateSaved()) {
                    ModuleCtrl.saveTypicalSelectionStates(packageData);
                    data.setTypicalSelectionStateSaved(true);
                }

                // Setting custom selection state values, if they have been saved before.
                if ( data.customSelectionStateSaved() ) {
                    ModuleCtrl.restoreCustomSelectionStates(packageData);
                }
            }
        }

        return repeatDialog;
    }

    public void actionPerformed(ActionEvent evt) {

        InstallData data = InstallData.getInstance();

        if (evt.getActionCommand().equals(data.getTypicalActionCommand())) {
            data.setInstallationType(data.getTypicalActionCommand());
            // System.err.println("Setting uninstallation type: " +  data.getTypicalActionCommand());
        } else if (evt.getActionCommand().equals(data.getCustomActionCommand())) {
            data.setInstallationType(data.getCustomActionCommand());
            // System.err.println("Setting uninstallation type: " +  data.getCustomActionCommand());
        }

    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

}
