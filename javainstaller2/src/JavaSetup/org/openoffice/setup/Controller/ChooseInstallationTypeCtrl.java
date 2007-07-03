/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChooseInstallationTypeCtrl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:50:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.ChooseInstallationType;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Calculator;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.ModuleCtrl;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;
import org.openoffice.setup.Util.PackageCollector;

public class ChooseInstallationTypeCtrl extends PanelController implements ActionListener {

    private String helpFile;

    public ChooseInstallationTypeCtrl() {
        super("ChooseInstallationType", new ChooseInstallationType());
        helpFile = "String_Helpfile_ChooseInstallationType";
    }

    public String getNext() {

        InstallData data = InstallData.getInstance();

        if ( data.getInstallationType().equals(data.getCustomActionCommand()) ) {
            return new String("ChooseComponents");
        } else if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {
            return new String("InstallationImminent");
        } else {
            System.err.println("Error: Unknown installation type!" );
            return new String("Error");
        }
    }

    public String getPrevious() {
        return new String("ChooseDirectory");
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {

        InstallData data = InstallData.getInstance();

        ChooseInstallationType panel = (ChooseInstallationType)getPanel();
        panel.setActionListener((ChooseInstallationTypeCtrl)this);
        panel.setTypicalActionCommand(data.getTypicalActionCommand());
        panel.setCustomActionCommand(data.getCustomActionCommand());
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;
        ChooseInstallationType panel = (ChooseInstallationType)getPanel();
        panel.removeActionListener((ChooseInstallationTypeCtrl)this);

        if ( nextButtonPressed ) {

            InstallData data = InstallData.getInstance();
            PackageDescription packageData = SetupDataProvider.getPackageDescription();

            if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {

                // If typical selection state values have been saved before,
                // it is now time to restore them

                if ( data.typicalSelectionStateSaved()) {
                    // System.err.println("Restoring typical selection states");
                    ModuleCtrl.restoreTypicalSelectionStates(packageData);
                }

                if ( data.logModuleStates() ) {
                    Dumper.logModuleStates(packageData, "ChooseInstallationType: Before setHiddenModuleSettingsInstall");
                }

                // For standard installation type, the hidden modules have to be defined here.
                // Then it is possible to calculate the size of the installed product, to show a warning
                // and to set the repeatDialog value to true
                ModuleCtrl.setHiddenModuleSettingsInstall(packageData);
                // Dumper.dumpInstallPackages(packageData);

                if ( data.logModuleStates() ) {
                    Dumper.logModuleStates(packageData, "ChooseInstallationType: After setHiddenModuleSettingsInstall");
                }

                // Collecting packages to install
                Vector installPackages = new Vector();
                PackageCollector.collectInstallPackages(packageData, installPackages);
                data.setInstallPackages(installPackages);

                // Check disc space
                if ( Calculator.notEnoughDiscSpace(data) ) {
                    repeatDialog = true;
                }
            }

            // Custom installation type
            if (( data.getInstallationType().equals(data.getCustomActionCommand() ))) {
                // Saving typical selection state values, if they are not already saved.
                if ( ! data.typicalSelectionStateSaved()) {
                    // System.err.println("Saving typical selection states");
                    ModuleCtrl.saveTypicalSelectionStates(packageData);
                    data.setTypicalSelectionStateSaved(true);
                }

                // Setting custom selection state values, if they have been saved before.
                if ( data.customSelectionStateSaved() ) {
                    // System.err.println("Restoring custom selection states");
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
            // System.err.println("Setting installation type: " +  data.getTypicalActionCommand());
        } else if (evt.getActionCommand().equals(data.getCustomActionCommand())) {
            data.setInstallationType(data.getCustomActionCommand());
            // System.err.println("Setting installation type: " +  data.getCustomActionCommand());
        }

    }

}
