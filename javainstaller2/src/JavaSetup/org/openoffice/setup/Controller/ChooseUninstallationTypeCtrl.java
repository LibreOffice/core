/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChooseUninstallationTypeCtrl.java,v $
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
