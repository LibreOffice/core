/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChooseUninstallationTypeCtrl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:51:10 $
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
