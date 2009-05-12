/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Main.java,v $
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

package org.openoffice.setup;

import org.openoffice.setup.Controller.AcceptLicenseCtrl;
import org.openoffice.setup.Controller.ChooseComponentsCtrl;
import org.openoffice.setup.Controller.ChooseUninstallationComponentsCtrl;
import org.openoffice.setup.Controller.ChooseUninstallationTypeCtrl;
import org.openoffice.setup.Controller.ChooseDirectoryCtrl;
import org.openoffice.setup.Controller.ChooseInstallationTypeCtrl;
import org.openoffice.setup.Controller.UninstallationCompletedCtrl;
import org.openoffice.setup.Controller.UninstallationImminentCtrl;
import org.openoffice.setup.Controller.UninstallationOngoingCtrl;
import org.openoffice.setup.Controller.UninstallationPrologueCtrl;
import org.openoffice.setup.Controller.InstallationCompletedCtrl;
import org.openoffice.setup.Controller.InstallationImminentCtrl;
import org.openoffice.setup.Controller.InstallationOngoingCtrl;
import org.openoffice.setup.Controller.PrologueCtrl;
// import org.openoffice.setup.Util.Dumper;

public class Main {

    public static void main(String[] args) {
        // try {
        //    UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
        // } catch (Exception e) { }

        // Dumper.dumpAllProperties();
        SetupFrame frame = new SetupFrame();
        InstallData data = InstallData.getInstance();
        if ( data.isInstallationMode() ) {
            PanelController controller1 = new PrologueCtrl();
            frame.addPanel(controller1, controller1.getName());
            PanelController controller2 = new AcceptLicenseCtrl();
            frame.addPanel(controller2, controller2.getName());
            PanelController controller3 = new ChooseDirectoryCtrl();
            frame.addPanel(controller3, controller3.getName());
            PanelController controller4 = new ChooseInstallationTypeCtrl();
            frame.addPanel(controller4, controller4.getName());
            PanelController controller5 = new ChooseComponentsCtrl();
            frame.addPanel(controller5, controller5.getName());
            PanelController controller6 = new InstallationImminentCtrl();
            frame.addPanel(controller6, controller6.getName());
            PanelController controller7 = new InstallationOngoingCtrl();
            frame.addPanel(controller7, controller7.getName());
            PanelController controller8 = new InstallationCompletedCtrl();
            frame.addPanel(controller8, controller8.getName());
            frame.setCurrentPanel(controller1.getName(), false, true);
        } else {
            PanelController controller1 = new UninstallationPrologueCtrl();
            frame.addPanel(controller1, controller1.getName());
            PanelController controller2 = new ChooseUninstallationTypeCtrl();
            frame.addPanel(controller2, controller2.getName());
            PanelController controller3 = new ChooseUninstallationComponentsCtrl();
            frame.addPanel(controller3, controller3.getName());
            PanelController controller4 = new UninstallationImminentCtrl();
            frame.addPanel(controller4, controller4.getName());
            PanelController controller5 = new UninstallationOngoingCtrl();
            frame.addPanel(controller5, controller5.getName());
            PanelController controller6 = new UninstallationCompletedCtrl();
            frame.addPanel(controller6, controller6.getName());
            frame.setCurrentPanel(controller1.getName(), false, true);
        }

        int ret = frame.showFrame();

        while ( data.stillRunning() ) {
            for (int i = 0; i < 100; i++) {}    // why?
        }

        System.exit(0);
    }
}
